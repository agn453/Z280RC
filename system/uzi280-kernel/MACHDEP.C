/* Z280RC modifications by Tony Nicholson 12-Dec-2018 */

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
	This file is for sep. I/D pages
*/
 
/* File for the Reh-Design CPU280 */

#undef MACHDEP
#define MACHDEP2

#undef MEMDEBUG

#include "unix.h"	/* also pulls in config.h */
#include "extern.h"


int c_mode = 0;	 /* this holds 0 Data Page, 8 Program Page */

main()
{

    /* Initialize the interrupt vector */

    initvec();
    initmmu();

    inint = 0;
    udata.u_insys = 1;

    spl(0x7f);  /*enable all interrupts*/

    ptab_pointer    = ptab;
    number_of_pages = npages;		
    init2();   /* in process.c */
}


/* This checks to see if a user-suppled address is legitimate */
valadr(base,size)
char *base;
uint16 size;
{
 	unsigned	usp();

    if (base >= (char *)udata.u_break && (uint16)base+size < (uint16)usp())
    {
	udata.u_error = EFAULT;
	return(0);
    }
    return(1);
}


/* This checks to see if a user-suppled program address is legitimate */
/**
	no test is done. u_pbreak holds not always a correct number
**/
pvaladr(base,size)
char *base;
uint16 size;
{
/**
    if (base >= udata.u_pbreak)
    {
	udata.u_error = EFAULT;
	return(0);
    }
**/
    return(1);
}


/* This adds two tick counts together.
The t_time field holds up to one second of ticks,
while the t_date field counts minutes */

addtick(t1,t2)
time_t *t1, *t2;
{

    t1->t_time += t2->t_time;
    t1->t_date += t2->t_date;
    if (t1->t_time >= 60*TICKSPERSEC)
    {
	t1->t_time -= 60*TICKSPERSEC;
	++t1->t_date;
    }
}

incrtick(t)
time_t *t;
{
    if (++t->t_time == 60*TICKSPERSEC)
    {
	t->t_time = 0;
	++t->t_date;
    }
}

bad_int()
{
    panic("Bad intrpt");
}

stk_trap()
{
    trappc();  /* Args are already on stack */
    panic("Sys-stack overflow");
}


/*
trap2(pc, msr, reason, psw, bc, de, hl, ix, iy)
*/
trap2(iy, ix, hl, de, bc, psw, reason, msr, pc)
char *pc;
unsigned msr;
unsigned reason, psw, bc, de, hl, ix, iy;
{
    trappc(pc, msr);
}


trappc(pc, msr)
char *pc;
unsigned msr;
{
    kprintf("%s pc: %x msr: %x\n", (msr&0x4000) ? "user" : "system", pc, msr);
}

/* This returns the physical address of the base of the page
containing the given user address.  It is in a format that
will allow it to be sent directly to the DMA controller.
A value of -1 means there is no valid address */

padr(user, ladr)
int user;
char *ladr;
{
	int	upadr(),spadr();

    if (user)
	return (upadr(ladr));
    else
	return (spadr(ladr));
}

gettrap()
{
    int (*curvec)();

    /* Get the vector for a pending caught signal, if any. */

    if (udata.u_cursig)
    {
        curvec = udata.u_sigvec[udata.u_cursig];
	udata.u_sigvec[udata.u_cursig] = SIG_DFL;   /* Reset to default */
	return ((unsigned)curvec);
    } 
    return (NULL);
}


#ifndef Z280RC

/* Port addresses of clock chip registers. */
/* On Reh-Design CPU280 this is on I/O-Page 40h    SN */

#define SECS 0x00
#define MINS 0x02
#define HRS 0x04
#define DAY 0x07
#define MON 0x08
#define YEAR 0x09

#endif

sttime()
{
    panic("Calling sttime");
}


rdtime(tloc)
time_t *tloc;
{
    register unsigned is,spl();

    is = spl(0x048 /*0*/);	/* 0x048 is DMA3 and UART Rx */

    tloc->t_time = tod.t_time;
    tloc->t_date = tod.t_date;
    spl(is);
}

#ifndef Z280RC

/* Update global time of day */
/* This is for the CPU280 SN */
rdtod()
{
    tod.t_time = (tread(SECS)>>1) | (tread(MINS)<<5) | (tread(HRS)<<11);
    tod.t_date = tread(DAY) | (tread(MON)<<5) | (tread(YEAR)<<9);
}
#endif

/* This is called to service a user segv trap */
usegv2()
{
    unsigned pageno, badpage();
    int getpag();
    	
    pageno = badpage();  /* Get the page number from the MMU (0 to 15) */

    if (pageno >= 16)
	panic("User segv of system page");
    c_mode = 0;
#ifdef MEMDEBUG
    kprintf("Proc %d has user-induced page %d fault, cmode %u : ",
	 udata.u_ptab->p_pid, pageno, c_mode);
#endif

    return(getpag(pageno));
}


/* This tries to load the page for the given user address.
If this is possible (or it is already loaded), it returns zero.
Otherwise, the address must be illegal, and nonzero is returned */

upage(uptr)
char  *uptr;
{
    int getpag();
#ifdef MEMDEBUG
    kprintf("Proc %d has system-induced page %d fault: ",
	 udata.u_ptab->p_pid, ((unsigned)uptr>>13)+c_mode);
#endif

/*    return (getpag( (unsigned)uptr >> 12)); */
	/* getpag handles c_mode internal */  
    return (getpag( (unsigned)uptr >> 13));
}


/* Used for current "date" for setting pg_age. */
unsigned age; 


/* This gets a free page, sets its reference count to 1, and returns
its descriptor address */
pg_descr *
pg_alloc()
{

    unsigned bestage,free_zombie();
    pg_descr *pg, *bestpg;

loop:
    bestpg = NULL;
    bestage = 0;

    /* Try to find an unused page we can just take */
    for (pg=pgtab; pg < pgtab+npages; ++pg)
    {
	if (pg->pg_refs == 0)
	{
	    bestpg = pg;
	    goto found;
	}
    }
    /* try to free a process page from a zombie2 */ 
    ifnot(free_zombie())	/* release p_zombie to get pages */
	panic("no pages");
    goto loop;	/* retry to get a page */

#ifdef VERS12 /* swapping not implemented */
    /* Try to find an page with only one ref */
    bestpg = NULL;
    bestage = 0;
    for (pg=pgtab; pg < pgtab+npages; ++pg)
    {
	if (pg->pg_refs == 1 && (age - pg->pg_age ) >= bestage)
	{
	    bestage = age - pg->pg_age;
	    bestpg = pg;
	}
    }
    if (bestpg)
	goto found;

    /* Try to find any page */
    bestpg = NULL;
    bestage = 0;
    for (pg=pgtab; pg < pgtab+npages; ++pg)
    {
	if ((age - pg->pg_age ) >= bestage)
	{
	    bestage = age - pg->pg_age;
	    bestpg = pg;
	}
    }
    ifnot(bestpg)
	panic("No pages");
#endif

found:

    bestpg->pg_addr &= ~0x0010;
    pg_out(bestpg);

#ifdef MEMDEBUG
    kprintf("pg_alloc returning %d-refed page %x\n", bestpg->pg_refs, bestpg->pg_addr);
#endif

    bestpg->pg_refs = 1;
    pg_zero(bestpg->pg_addr);
    bestpg->pg_age = ++age;
    bestpg->pg_addr &= ~0x000F;
    bestpg->pg_addr |= 0x0a;   /* Set various page status bits */

    return (bestpg);

}



/* This looks through all the processes' page tables,
and swaps out every instance of the given page. */

pg_out(pg)
pg_descr *pg;
{
#ifdef _SWAP_OUT_	/* not imlemented SN */ 

    ptptr p;
    pg_descr **pages;
    int pnum;

    for(p=ptab;p < ptab+PTABSIZE; ++p)
    {
	if (p->p_status == P_EMPTY || p->p_status == P_ZOMBIE)
	    continue;

	if (p == udata.u_ptab)
	    pages = udata.u_page;   /* Current process */
	else
	    pages = p->p_ublk->u_d.u_page;

	for (pnum=0; pnum < 16; ++pnum)
	{
	    if (pg == pages[pnum])
	    {
#ifdef MEMDEBUG
    kprintf("Process %d has page %d addr %x taken\n", p->p_pid, pnum, pg->pg_addr);
#endif
		/* Swap out if modified */
/* TEMP!!!		if (pages[pnum] & 0x01)   /* Modified bit */
		    pagewrite(SWAPDEV, p->p_swap + (16*pnum), pg->pg_addr);

		pages[pnum] = OUTPAGE;
		if (p == udata.u_ptab)
		    setmmu(pnum, 0);  /* Current process */
	    }
	}
    }
#endif
}



/* This increments the reference count of a page */
pg_realloc(pg)
pg_descr *pg;
{
    ++pg->pg_refs;
}



/* This decreases the reference count of a page,
possibly freeing it. */

pg_free(pg)
pg_descr *pg;
{
    if (!pg->pg_refs)
	panic("pg_free");
    return(--(pg->pg_refs));
}

/* This tries to load the page for the given user page number (0 to 15).
   If this is possible (or it is already loaded), it returns zero.
   Otherwise, the address must be illegal, and nonzero is returned */

int getpag(pnum)
int pnum;
{
    pg_descr *pg, *newpg, *pg_alloc();

    pnum += c_mode; 	/* add offset of 8 if program page wanted */	
    pg = udata.u_page[pnum];
    if (pg == NOPAGE)
    {
#ifdef MEMDEBUG
	kprintf("Page %d not in address space, cmode %u\n", pnum,c_mode);
#endif
	return (-1);

    }
    else if (pg == OUTPAGE)
    {
	pg = pg_alloc();
#ifdef MEMDEBUG
	kprintf("Swapped out page read into address %x\n", pg->pg_addr);
#endif
#ifdef _SWAP_OUT_
	pageread(SWAPDEV, udata.u_ptab->p_swap + (16*pnum), pg->pg_addr);
	udata.u_page[pnum] = pg;
	pg->pg_addr &= 0xffef;
	setmmu(pnum, pg->pg_addr | 0x0a);
#else
	warning("no swap");
#endif
    }
    else if (pg == VIRGPAGE)
    {
	pg = pg_alloc();
	udata.u_page[pnum] = pg;
	pg->pg_addr &= 0xffef;

#ifdef MEMDEBUG
	kprintf("Virgin page given address %x\n", pg->pg_addr);
#endif
	/* look for binary type 1 (no sep I&D pages) */
	if (udata.u_ptab->p_bintype) {
		/* data page must always be equal to instr page */
		if (pnum<8) { /* fault was on data page */
			udata.u_page[pnum+8] = pg;
			setmmu(pnum+8, pg->pg_addr | 0x0b);
		} else {      /* fault was on code page */
			udata.u_page[pnum-8] = pg;
			setmmu(pnum-8, pg->pg_addr | 0x0b);
		}
	}
	setmmu(pnum, pg->pg_addr | 0x0b);  /* Page is pre-marked as modified */
    }
    else
    {
	/* See if the page is write-protected, which implies
	copy-on-write (after forking) */
 	if (pg->pg_addr & 0x04)
	{
	    /* If the page is copy-on-write, but there are no other refs
		to it, panic.  (changed SN) */
	    if (pg->pg_refs == 1) {
		pg->pg_addr &= ~0x04;   /* make normal R/W  SN */
		pg->pg_addr &=0xffef;
		if (udata.u_ptab->p_bintype) {
		/* data page must always be equal to instr page */
			if (pnum<8) { /* fault was on data page */
				udata.u_page[pnum+8] = pg;
				setmmu(pnum+8, pg->pg_addr);
			} else {      /* fault was on code page */
				udata.u_page[pnum-8] = pg;
				setmmu(pnum-8, pg->pg_addr);
			}
		}
		setmmu(pnum, pg->pg_addr);
	    }	
	    else
	    {
		pg->pg_addr &= 0xffef;
		newpg = pg_alloc();
		newpg->pg_addr &= 0xffef;	
		pg_copy(pg->pg_addr, newpg->pg_addr);
#ifdef MEMDEBUG
		kprintf("COW page %x copied to %x\n", pg->pg_addr, newpg->pg_addr);
#endif
		/* TODO: This marks the copied page as modified, but it should
		   inherit the modified bit from the old page */
		/* look for binary type 1 (no sep I&D pages) */
		if (udata.u_ptab->p_bintype) {
			/* data page must always be equal to instr page */
			if (pnum<8) { /* fault was on data page */
				udata.u_page[pnum+8] = newpg;
				setmmu(pnum+8, newpg->pg_addr | 0x0b);
			} else {      /* fault was on code page */
				udata.u_page[pnum-8] = newpg;
				setmmu(pnum-8, newpg->pg_addr | 0x0b);
			}
		}	
		/* common for all binary types */
		udata.u_page[pnum] = newpg;
		setmmu(pnum, newpg->pg_addr | 0x0b);
 
		/* If there is only one more reference to this COW page,
		    make it normal (R/W).  SN */
		if (--pg->pg_refs==1) 
		    	pg->pg_addr &= ~0x04;
	    }
	}
	else
	{
#ifdef MEMDEBUG
	    kprintf("Valid page with address %x reloaded into MMU\n",
		     pg->pg_addr);
#endif
	    pg->pg_addr &= 0xffef;
	    setmmu(pnum, pg->pg_addr);
		/* look for binary type 1 (no sep I&D pages) */
	    if (udata.u_ptab->p_bintype) {
		if (pnum<8) { /* fault was on data page */
			setmmu(pnum+8, pg->pg_addr);
		} else {      /* fault was on code page */
			setmmu(pnum-8, pg->pg_addr);
		}
	    }
	}
    }

    return (0);
}


/* This reads all 16 user MMU entries to the given addresses */
rdmmu(addr)
pg_descr *addr[];
{
    register j;
    pageaddr	getmmu();

    for (j=0; j<16; ++j)
	if (realpage(addr[j]))
	    addr[j]->pg_addr = getmmu(j);
}


/* This loads all 16 user MMU entries from the given addresses */
ldmmu(addr)
pg_descr *addr[];
{
    register j;

    for (j=0; j<16; ++j)
	if (realpage(addr[j]))
	    setmmu(j, addr[j]->pg_addr);
	else
	    setmmu(j, 0);
}

ufault()
{
    udata.u_error = EFAULT;
}


/* These read and write data from/to the user address space */

ugetw(uptr)
char  *uptr;
{
    int w;

    uget(uptr, &w, sizeof(int));
    return (w);
}

uputw(w, uptr)
int w;
char  *uptr;
{
    uput(&w, uptr, sizeof(int));
}

uputc(c, uptr)
int c;
char  *uptr;
{
    uput(&c, uptr, 1);
}


ugetc(uptr)
char  *uptr;
{
    char c;

    uget(uptr, &c, 1);
    return (c);
}

#ifndef Z280RC

/* Read BCD clock register, convert to binary. */
tread(port)
uint16 port;
{
    uint16 n, in();

    iopg40();
    n = in(port);  
    iopg00();
    return ( 10*((n>>4)&0x0f) + (n&0x0f) );
}

#endif

trap_3()
{
    ssig(udata.u_ptab, SIGILL);   /* Send the SIGILL signal to ourself */
    chksigs();		/* This will exit if appropriate */
}

segv_trap_2()
{
	ssig(udata.u_ptab, SIGILL);   /* Send the SIGILL signal to ourself */
}


abort()
{
	di();
	for (;;) ;
}


/* This prints an error message and dies. */
panic(s)
char *s;
{
    static first = 0;
    		
#ifdef CLUB80_Terminal
	_putc(1,033); _putc(1,033); _putc(1,'A');
#endif
    if (first == 0)
    {
	first = 1;	/* not recursiv */
	_sync();	/* try to update drives before system halted */
    }
    ca_sync();		/* allways write out cache buffer */	
    di();
    kprintf("PANIC: %s\n",s);
/*
    idump();
*/
    abort();
}


warning(s)
char *s;
{
    kprintf("WARNING: %s\n",s);
}


puts(s)
char *s;
{
    while (*s)
	kputchar(*(s++));
}
#ifndef no_bus_tty /* bus tty is minor 1 by default */
kputchar(c)
int c;
{
    if (c == '\n')
	_putc(1, '\r');
    _putc(1, c);
    if (c == '\t')
       _putc(1,  '\6');
}
#else	/* UART is used here (minor 5) */
kputchar(c)
int c;
{
    if (c == '\n')
	_putc(5, '\r');
    _putc(5, c);
    if (c == '\t')
       _putc(5,  '\6');
}
#endif
idump()
{
#ifdef IDUMP
    inoptr ip;
    ptptr pp;
    extern struct cinode i_tab[];

/***
    kprintf(
        "\tMAGIC\tDEV\tNUM\tMODE\tNLINK\t(DEV)\tREFS\tDIRTY root %d\n",
            root - i_tab);

    for (ip=i_tab; ip < i_tab+ITABSIZE; ++ip)
    {
	ifnot (ip->c_magic)	
	    continue;

	kprintf("%d\t%d\t%d\t%u\t0%o\t%d\t%d\t%d\t%d\n",
	       ip-i_tab, ip->c_magic,ip->c_dev, ip->c_num,
	       ip->c_node.i_mode,ip->c_node.i_nlink,ip->c_node.i_addr[0],
	       ip->c_refs,ip->c_dirty);
    }
***/

    kprintf("\n\tSTAT\tWAIT\tPID\tPPTR\tALARM\tPENDING\tIGNORED\n");
    for (pp=ptab; pp < ptab+PTABSIZE; ++pp)
    {
	kprintf("%d\t%d\t0x%x\t%d\t%d\t%d\t0x%x\t0x%x\n",
	       pp-ptab, pp->p_status, pp->p_wait,  pp->p_pid,
	       pp->p_pptr-ptab, pp->p_alarm, pp->p_pending,
		pp->p_ignored);
        ifnot(pp->p_pptr)
	    break;
    }	
/**    
    bufdump();
**/
    kprintf("\ninsys %d ptab %d call %d cwd %d sp 0x%x inint %d\n",
	udata.u_insys,udata.u_ptab-ptab, udata.u_callno, udata.u_cwd-i_tab,
       udata.u_ssp, inint);
#endif
}
