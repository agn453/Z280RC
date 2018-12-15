
/*-
; * All UZI280 source code is  
; * Copyright (c) (1990-95) by Stefan Nitschke and Doug Braun
; *
; * Permission is hereby granted, free of charge, to any person
; * obtaining a copy of this software and associated documentation
; * files (the "Software"), to deal in the Software without
; * restriction, including without limitation the rights to use,
; * copy, modify, merge, publish, distribute, sublicense, and/or
; * sell copies of the Software, and to permit persons to whom
; * the Software is furnished to do so, subject to the following
; * conditions:
; * 
; * The above copyright notice and this permission notice shall be
; * included in all copies or substantial portions of the Software.
; * 
; * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
; * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
; * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
; * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
; * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
; * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; * DEALINGS IN THE SOFTWARE.
; */


/*
	assembler related file of process.c
*/

#define MACHDEP

#include "unix.h"	/* also pulls in config.h */
#include "extern.h"

#undef  MEMDEBUG

/* Temp storage for swapout() */
int *stkptr;


/* Swapout swaps out the current process, finds another that is READY,
   possibly the same process, and swaps it in.
   When a process is restarted after calling swapout,
   it thinks it has just returned from swapout(). */

/* This function can have no arguments or auto variables */
swapout()
{
    static ptptr newp;
    ptptr getproc();

    /* See if any signals are pending */
    chksigs();

    /* Get a new process */
    newp = getproc();

    /* If there is nothing else to run, just return */
    if (newp == udata.u_ptab)
    {
	udata.u_ptab->p_status = P_RUNNING;
	return (runticks = 0);
    }

	
#ifdef MEMDEBUG
    kprintf("Swapping out process %d\n", udata.u_ptab->p_pid);
#endif

    /* Read the MMU registers back into udata.u_page */
    rdmmu(udata.u_page);

    /* Save the stack pointer and critical registers */
#asm
	ld	hl,01	;this will return 1 if swapped.
	push	hl	;will be return value
	push	ix
	push	iy		/* SN */

; save info of auxillary reg set in use
	ld	a,1	; preset TRUE
	;jar	1
				defb	0ddh,020h
				defb	1	; displacement 
	dec	a	; set FALSE if not in use
	push	af	; save the info on stack ( for swapin )
	
	ld	hl,0
	add	hl,sp	;get sp into hl
	ld	(_stkptr),hl
#endasm
    udata.u_ssp = stkptr;
    udata.u_usp = usp();

    swrite();
    /* Read the new process in, and return into its context. */
    swapin(newp);

    /* We should never get here. */
    panic("swapin failed");
}



/* No automatics can be used past tempstack(); */
swapin(pp)
ptptr pp;
{
    static ptptr newp;

    newp = pp;

    di();

#ifndef _WIN_SYS_
/* look if process has ignored signals. If not ignored 0ch
   increase process priority (maybe an interactive task) */   
    if (newp->p_ignored != 0xC)
      newp->p_priority = MAXINTER;   
    else
      newp->p_priority = MAXBACK2;
#else
 /*   if (newp->p_pending) /*if signals pending increase priority */
 /*     newp->p_priority = MAXINTER; */
#endif
    tempstack();

    /* Reload the user area */
    bcopy(newp->p_ublk, &udata, 512);

    /* Reload the MMU */
    ldmmu(udata.u_page);

    ei();
    if (newp != udata.u_ptab)
	panic("mangled swapin");

#ifdef MEMDEBUG
    kprintf("Swapped in process %d\n", newp->p_pid);
#endif

    di();
    newp->p_status = P_RUNNING;
    runticks = 0;
    ei();	
    /* Restore the registers */
    stkptr = udata.u_usp;	
#asm
	ld	hl,(_stkptr)
	;ldctl  usp,hl
				defb	0edh,08fh
#endasm

    stkptr = udata.u_ssp;

#asm
	ld	hl,(_stkptr)
	ld	sp,hl
; get info of auxillary reg set in use
	exx
	;jar	-4	; reexecute the exx op
				defb	0ddh,020h
				defb	-4	; select prim reg set
	pop	af	; get info flag
	or	a	; test it
	jp	z,0f
	exx		; select aux. reg set
0:	 

	pop	iy		/* SN */ 
	pop	ix
	pop	hl
	ld	a,h
	or	l
	jp	cret 	;return into the context of the swapped-in process 
#endasm

}



/* Temp storage for dofork */
uint16 newid;

/* dofork implements forking.  */
/* This function can have no arguments or auto variables */
dofork()
{
    static ptptr p;
    ptptr ptab_alloc();
    static int j;

    ifnot (p = ptab_alloc())
    {
	udata.u_error = EAGAIN;
	return(-1);
    }
    di();

#ifdef MEMDEBUG
    kprintf("forking process %d\n", udata.u_ptab->p_pid);
#endif

    /* Read the MMU registers back into udata.u_page */
    rdmmu(udata.u_page);

    /* Mark all pages as copy-on-write (read-only bit), and increment
	reference count in master page table */
    for (j=0; j<16; ++j)
    {
	/* To deal with swapped out pages, swap them in first.
	   Do this by touching the page. */
	if (udata.u_page[j] == OUTPAGE)
	{
#ifdef MEMDEBUG
	    kprintf("Dofork touching swapped out page %d\n", j);
#endif
#ifndef _ID_
	    ugetc(0x1000 * j);
#else
/*	    ugetc(0x2000 * j); /* this won't work SN */
	    warning("swapping not implemented");
#endif

	}
	if (realpage(udata.u_page[j])) {
	    pg_realloc(udata.u_page[j]);
	    udata.u_page[j]->pg_addr |= 0x05;  /* Mark as modified */
	}
    }

    /* Reload the MMU with the modified entries */
    ldmmu(udata.u_page);

    udata.u_ptab->p_status = P_READY; /* Parent is READY */
    newid = p->p_pid;
    ei();

    /* Save the stack pointer and critical registers */
    /* When the process is swapped back in, it will be as if
    it returns with the value of the childs pid. */

#asm
	LD	HL,(_newid)
	push	hl
	push	ix
	push	iy		/* SN */

; save info of auxillary reg set in use
	ld	a,1	; preset TRUE
	;jar	1
				defb	0ddh,020h
				defb	1	; displacement 
	dec	a	; set FALSE if not in use
	push	af	; save the info on stack ( for child )

	ld	hl,0
	add	hl,sp	;get sp into hl
	ld	(_stkptr),hl
#endasm

    udata.u_ssp = stkptr;
    udata.u_usp = usp();

    swrite();

#asm
	pop	hl	; reg set flag not needed
	pop	hl	/* SN */
	pop	hl		;repair stack pointer
	pop	hl
#endasm

    /* Make a new process table entry, etc. */
    newproc(p);

    di();
    runticks = 0;
    p->p_status = P_RUNNING;
/* added for correct execve() of type 1 binaries */
    p->p_fork_inf = 1; /* mark as child of fork */
    ei();

#ifdef MEMDEBUG
    kprintf("resuming child process %d\n", udata.u_ptab->p_pid);
#endif

    return (0);  /* Return to child */

}




/* unix system call trap routine */ 
#asm
*Include	macro.lib
	psect	text
	global _unix,_unix2
_unix:
	ex	af,af'
	push	af
	ex	af,af'
	exx
	push	bc
	push	de
	push	hl
	exx
	push	af
	push	bc
	push	de
	push	hl
	push	ix
	push	iy
	;call unix2()
	LDHLSX	12+8
	LD	(_ub + ?OCALL),hl	;Store in udata.u_callno
	CALL	_unix2

	;shove the return value where it will eventually get popped
	;into HL
	STHLSX	4
 
	PUSH	AF
	POP	HL	;get flags
	STHLSX	10	;shove in right place for return

	GLOBAL	_IRET

	JP	_IRET  ;jump to common interrupt return routine
#endasm


#asm
	psect	text
	global	_ret_error
_ret_error:
	LD	HL, (_ub + ?OERR)
	LD	A,H
	OR	L
	nop		; make system stack pointer aligned
	SCF
	jp	cret
#endasm

#ifndef C_WAKEUP
#asm
;
; ASM version of wakeup(int event)
;
	psect 	text
	global	_ptab, _wakeup, _ei

_wakeup:	; wakeup(int event)
	push	bc
	push	iy
	LDHLSX	6		; get argument
	ex	de,hl		; event into de
	ld	iy,_ptab	; pointer to ptab
	ld	b,30		; number of process table entrys PTABSIZE
w_loop:	ld	a,(iy)		; get p_status
	cp	2		; <= P_RUNNING
	jr	c,w_next	
	cp	8		; < P_ZOMBIE2
	jr	nc,w_next
is_ok:
;	ldw 	hl,(iy+12)	; p_wait
				defb	0edh,034h
				defw	12
;	cpw	hl,de
				defb	0edh,0d7h
	jr	nz,w_next
	ld	hl,0
;	ldw	(iy+12),hl	; clear p_wait
				defb	0edh,035h
				defw	12
	ld	a,2		; P_READY
	ld	(iy),a		; set p_ready
w_next:	ld	a,40		; sizeof( struct ptab )  
;	add	iy,a		; point to next entry
				defb	0fdh,0edh,06dh
	djnz	w_loop		; check for finish
w_end:	pop	iy
	pop	bc
	ret
; end of wakeup() 
#endasm
#endif
