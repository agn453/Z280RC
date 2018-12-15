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

/*LINTLIBRARY*/

/*
	this file is for sep I/D pages
*/
#define FALSE 	0
#define TRUE	1

#include "unix.h"
#include "extern.h"

#undef MEMDEBUG

/*
int
equal(j)
int j;
{
	return(udata.u_page[j]->pg_addr!=udata.u_page[j-8]->pg_addr);
}
*/
void
give_pages(p)
ptptr p;
{
	pg_descr *ptmp;
	int j;

    if (!p) return;	/* give process pages of zombie2 process and mark COW */
    for (j=0; j<16; ++j) {
	ptmp=p->p_ublk->u_d.u_page[j];
	if (realpage(ptmp)) {
	  if (p->p_bintype) {
	    if (j < 8) 
		pg_realloc(ptmp);
	  } else
		pg_realloc(ptmp);
	  ptmp->pg_addr |= 0x05; 

	}
	udata.u_page[j] = ptmp;
    }
}

 
/* set data page = code page */
/* and mark all data pages COW */
void
setup_pg_type1()
{
	pg_descr *ptmp;
	unsigned j;

    	rdmmu(udata.u_page);
    	for (j=8; j<16; ++j) 
	  if (realpage((ptmp=udata.u_page[j]))) {
/*		ptmp->pg_addr |= 0x05; /* Mark as modified */ 
/*		pg_realloc(ptmp);*/
		/* set data page = code page */
		udata.u_page[j-8] = ptmp; 
	  }
        ldmmu(udata.u_page);
}

unsigned
eq_proc(p)
ptptr p;
{ 
	if( p->p_inode 	      == udata.u_ptab->p_inode &&
	    p->p_idev         == udata.u_ptab->p_idev &&
	    p->p_idate.t_time == udata.u_ptab->p_idate.t_time &&
	    p->p_idate.t_date == udata.u_ptab->p_idate.t_date )
	 return(1);
	else
	 return(0);
}

/*
	make a zombie2 process entry
*/
ptptr
make_zombie()
{
	ptptr z_proc,ptab_alloc();
	unsigned	j;

	di();	
    	rdmmu(udata.u_page);
	z_proc = ptab_alloc();
	z_proc->p_bintype = udata.u_ptab->p_bintype;
	z_proc->p_uid = udata.u_ptab->p_pid;
	z_proc->p_status = P_FORKING;
	/* set zombie2 id data */
	z_proc->p_inode = udata.u_ptab->p_inode;
	z_proc->p_idev  = udata.u_ptab->p_idev;
	z_proc->p_idate.t_time = udata.u_ptab->p_idate.t_time;
	z_proc->p_idate.t_date = udata.u_ptab->p_idate.t_date;

	/* one more page refs. set all pages on COW (zombie never changes) */
	for (j=0;j<16;j++)
		if (realpage(udata.u_page[j])) {
		  if (z_proc->p_bintype) {
		    if (j<8)
			pg_realloc(udata.u_page[j]);
		  } else
			pg_realloc(udata.u_page[j]);
		    udata.u_page[j]->pg_addr |= 0x05;

		}
	ldmmu(udata.u_page);
    	/* copy udata block */
	bcopy(&udata,z_proc->p_ublk,512); 
	z_proc->p_ublk->u_d.u_ptab = z_proc; /* set ptab pointer */
	ei();
	return(z_proc);
}

ptptr
find_zombie2()
{	
    ptptr    p;

    /* find out if zombie2 process with same inode is loaded */ 
    for (p=ptab;p < ptab+PTABSIZE; ++p)  
      if (p->p_status == P_ZOMBIE2) /* only zombie2 */
	if( eq_proc(p) )
        {
		p->p_uid=udata.u_ptab->p_pid; 
			/* update zombie count for later LRU */
    		p->p_status= P_FORKING;	  
			/* temorary set zombie to FORKING status */
		return(p);
	}
    return(0);
}

wargs(argv,argbuf)   /* argv in user space */
char **argv;
struct s_argblk *argbuf;
{
    register char *ptr;    /* Address of base of arg strings in user space */
    int c;
    register char *bufp;
    unsigned ugetw();
    char     ugetc();

    argbuf->a_argc = 0;  /* Store argc in argbuf. */
    bufp = argbuf->a_buf;

    while (ptr = ugetw(argv++))
    {
	++(argbuf->a_argc);  /* Store argc in argbuf. */
	do
	{
	    *bufp++ = c = ugetc(ptr++);
	    if (bufp > argbuf->a_buf+500)
		udata.u_error = E2BIG;
	    if (udata.u_error)
	    {
		return (1);
	    }
	}
	while (c);
    }

    argbuf->a_arglen = bufp - argbuf->a_buf;  /*Store total string size. */

    return (0);
}




char **
rargs(ptr,argbuf,cnt)
register char *ptr;
struct s_argblk *argbuf;
int *cnt;
{
    char **argv;  /* Address of users argv[], just below ptr */
    int argc, arglen;
    char **argbase;
    char *sptr;

    sptr = argbuf->a_buf;

    /* Move them into the users address space, at the very top */
    ptr -= (arglen = argbuf->a_arglen);

    if ((unsigned)ptr%2)  ptr--;	/* make aligned address */	

    if (arglen)
        uput(sptr, ptr, arglen);

    /* Set argv to point below the argument strings */
    argc = argbuf->a_argc;
    argbase = argv = (char**)ptr - (argc + 1);

    if (cnt)
  	*cnt = argc;

    /* Set each element of argv[] to point to its argument string */

    while (argc--)
    {
        uputw(ptr, argv++);
	if (argc)
	{
	    do
		++ptr;
	    while(*sptr++);
        }
    }
/*    uputw(argv, NULL); */

    return (argbase);
}



/**********************************
brk(addr)
char *addr;
************************************/

#define addr (unsigned)udata.u_argn

_brk()
{
    register j;

    /* Don't allow break to be set past user's stack pointer */
    /*** St. Nitschke allow min. of 512 bytes for Stack ***/
    if (addr >= (unsigned)(udata.u_usp)-512)
    {
	udata.u_error = ENOMEM;
	return (-1);
    }

    udata.u_break = addr;

    /* Make all unallocated pages up to and including the page with
    the break address virgin pages */

    for (j=0; j <= addrpage(udata.u_break); ++j)
    {
	if (udata.u_page[j] == NOPAGE)
	    udata.u_page[j] = VIRGPAGE;
    }

    /* TODO: If the break is reduced, free the pages above the break */

    return(0);
}

#undef addr



/************************************
sbrk(incr)
uint16 incr;
***************************************/

#define incr (uint16)udata.u_argn

_sbrk()
{
    register unsigned oldbrk;

    udata.u_argn += (oldbrk = udata.u_break);
    if ((unsigned)udata.u_argn < oldbrk)
	return(-1); 
    if (_brk()) /* brk(udata.u_argn) */
	return(-1);

    return((unsigned)oldbrk);
}

#undef incr



/**************************************
wait(statloc)
int *statloc;
****************************************/

#define statloc (int *)udata.u_argn

_wait()
{
    ptptr p;
    int retval,valadr();
    unsigned temp;
	     
    if (statloc && !valadr(statloc, sizeof(int)))
    {
	udata.u_error = EFAULT;
	return(-1);
    }

    di();
    /* See if we have any children. */
    for (p=ptab;p < ptab+PTABSIZE; ++p)
    {
	temp = p->p_status;
	if (temp && temp != P_ZOMBIE2 
	    && p->p_pptr == udata.u_ptab && p != udata.u_ptab)
	    goto ok;
    }
    udata.u_error = ECHILD;
    ei();
    return (-1);

ok:
    /* Search for an exited child; */
    ei();
    for (;;)
    {
	chksigs();
        if (udata.u_cursig)
	{
	    udata.u_error = EINTR;
	    return(-1);
	}
	di();
	for(p=ptab;p < ptab+PTABSIZE; ++p)
	{
	    if (p->p_status == P_ZOMBIE && p->p_pptr == udata.u_ptab)
	    {
		if (statloc)
		    uputw(p->p_exitval, statloc);
		p->p_status = P_EMPTY;
		retval = p->p_pid;
	        /* Add in child's time info */
		/* It was stored on top of p_wait in the childs process
		   table entry */
		addtick(&udata.u_cutime, p->p_wait);
	 	addtick(&udata.u_cstime, p->p_wait +
					 sizeof(time_t));
		ei();
		return(retval);
     	    }
	}
	ei();
	/* Nothing yet, so wait */
	psleep(udata.u_ptab);
    }
}

#undef statloc



/**************************************
_exit(val)
int16 val;
**************************************/

#define val (int16)udata.u_argn

__exit()
{
    doexit(val,0);
}

#undef val


/* subject of change in version 1.3h */
doexit(val,val2)
int16 val;
int16 val2;
{
    register int16 j;
    register ptptr p;
    unsigned	temp;	

#ifdef MEMDEBUG
    kprintf("process %d exiting\n", udata.u_ptab->p_pid);
#endif

    di();
 
    for (j=0; j < UFTSIZE; ++j)
    {
	if (udata.u_files[j] >= 0)  /* Portable equivalent of == -1 */
	    doclose(j);
    }

/*    _sync();  /* Not necessary, but a good idea. */

    udata.u_ptab->p_exitval = (val<<8) | (val2 & 0xff);
/*
    if (udata.u_cwd)
*/
      i_deref(udata.u_cwd);
    
    /* Stash away child's execution tick counts in process table,
       overlaying some no longer necessary stuff. */
    addtick(&udata.u_utime,&udata.u_cutime);
    addtick(&udata.u_stime,&udata.u_cstime);
    bcopy(&udata.u_utime, udata.u_ptab->p_wait, 2 * sizeof(time_t));

    /* code pages are still correct */
    udata.u_ptab->p_status = P_ZOMBIE;

    /* See if we have any children. Set childs parent to our parent */
    for (p=ptab;p < ptab+PTABSIZE; ++p)
    {
	temp = p->p_status;
	if (temp && temp != P_ZOMBIE2 && p->p_pptr == udata.u_ptab && 
	    p != udata.u_ptab) 
            p->p_pptr = udata.u_ptab->p_pptr;
    }
    ei();
 
    /* Wake up a waiting parent, if any. */
    wakeup((char *)udata.u_ptab->p_pptr);

    di();
    if (udata.u_ptab->p_bintype)
	temp = 8;
    else
	temp = 16;		
    /* Free all pages */
    for (j=0; j < temp; ++j)
	if (realpage(udata.u_page[j]))
	    	pg_free(udata.u_page[j]);

    /* Release udata storage block */ 
    brelse(udata.u_ptab->p_ublk);  
    ei();

    swapin(getproc());
    panic("doexit:won't exit");
}


_fork()
{
   int dofork();
	
   return (dofork());
}



_pause()
{
    psleep(0);
    udata.u_error = EINTR; 
    return(-1);
}

