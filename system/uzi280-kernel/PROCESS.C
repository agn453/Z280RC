
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
	process.c:	uses cur_win defined in devtty to change 
			process priority.	SN 
*/


#include "unix.h"	/* also pulls in config.h */
#include "extern.h"

/*#define MEMDEBUG*/

/*#define _WIN_SYS_*/

/* int dummy;  /* For testing Hiesenbug */

#ifdef _WIN_SYS_
extern int cur_win;
#endif


static sec_counter=0; 	/* counts up to 30 secounds */

init2()
{
    register char *j;
    pageaddr *k;
    static char bootline[2];
    static char arg[] =
	 { '/', 'i', 'n', 'i', 't', 0, 0, 0x01, 0x01, 0, 0, 0, 0};
    inoptr i_open(), n_open();
    ptptr ptab_alloc();

    bufinit();
		
	
    /* Create the context for the first process */
    newproc(udata.u_ptab = initproc = ptab_alloc());
    initproc->p_status = P_RUNNING;

    /* User's file table */
    for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
	*j = -1;

    /* Turn on the clock */
/*    out(02,0xf1); */

    RTclock();    /* in machdep.c  */

    ei();

    /* Wait until the clock has interrupted, to set tod */
         while (!tod.t_date) ;  /* Loop */ 

    /* Open the console tty device (establishing the init process'
	controlling tty */

    if (d_open(TTYDEV) != 0)
	panic("no tty");

    kprintf("\nUZI280 is (c) Copyright (1990-96) by Stefan Nitschke and Doug Braun\n\nboot: ");
    udata.u_base = bootline;
    udata.u_sysio = 1;
    udata.u_count = 2;
    cdread(TTYDEV);
    ROOTDEV = *bootline - '0';

    /* Mount the root device */
    if (fmount(ROOTDEV,NULLINODE))
	panic("no filesys");
    ifnot (root = i_open(ROOTDEV,ROOTINODE))
	panic("no root");
    i_ref(udata.u_cwd = root);
    rdtime(&udata.u_time);

    /* This embrionic process has only its first page allocated */
    for (k=udata.u_page; k < (udata.u_page+16); ++k)
	*k = NOPAGE;
    udata.u_page[0] = VIRGPAGE;
    ldmmu(udata.u_page);

    /* Poke the execve arguments into user data space */
    uput(arg, PROGBASE, sizeof(arg));

    udata.u_argn2 = (int16)PROGBASE;
    udata.u_argn1 = 0x107;    /* Arguments (just "init") */
    udata.u_argn = 0x10b;     /* Environment (none) */

    _execve();

    panic("no /init");

}


/* psleep() puts a process to sleep on the given event.
   If another process is runnable, it swaps out the current one
   and starts the new one. 
   Normally when psleep is called, the interrupts have already been
   disabled.   An event of 0 means a pause(), while an event equal
   to the process's own ptab address is a wait().   */

psleep(event)
char *event;
{
    register dummy;  /* Force saving of registers */

    di();
    if (udata.u_ptab->p_status != P_RUNNING)
	panic("psleep: voodoo");
    if (!event)
	udata.u_ptab->p_status = P_PAUSE;
    else if (event == (char *)udata.u_ptab)
	udata.u_ptab->p_status = P_WAIT;
    else
	udata.u_ptab->p_status = P_SLEEP;

    udata.u_ptab->p_wait = event;
    udata.u_ptab->p_waitno = ++waitno;

    ei();

    swapout();		/* Swap us out, and start another process */

    /* Swapout doesn't return until we have been swapped back in */
}

#ifdef C_WAKEUP

/* wakeup() looks for any process waiting on the event,
and make them runnable */

wakeup(event)
char *event;
{
    register ptptr p;
    unsigned temp;
	
    di();
    for(p=ptab;p < ptab+PTABSIZE; ++p)
    {
	temp = p->p_status;
	if ( p->p_wait == event && temp != P_ZOMBIE2
	    && temp > P_RUNNING)
	{
	    p->p_status = P_READY;
	    p->p_wait = (char *)NULL;
	}
    }
    ei();
}
#endif

/* Getproc returns the process table pointer of a runnable process.
It is actually the scheduler.
If there are none, it loops.  This is the only time-wasting loop in the 
system. */

ptptr
getproc()
{
    register status;
    static ptptr pp = ptab;    /* Pointer for round-robin scheduling */
    static unsigned lcnt=0;
    extern int in_casync,in_cawrite,rw_flag;
		
    for (;;)
    {
	if (++pp >= ptab + PTABSIZE)
	    pp = ptab;

	status = pp->p_status;

	if (status == P_RUNNING)
	    panic("getproc: extra running");
	if (status == P_READY)
	    return(pp);

    ++lcnt;
    /* nothing to do, write out one dirty cache buffer every 80th loop 
   	after at least 30 secounds of last write */
    if ( in_casync==1 && in_cawrite==0 && (lcnt%80 == 0) ) 
    /* this test should not be necessary because the interrupt to
	this function was during user mode operation */   
	if (rw_flag == 0)  /* only if last write finished */
		int_ca_sync();	/* write out next dirty buffer */

    }
}



/* This saves the user area */
swrite()
{
    di();
    bcopy(&udata, udata.u_ptab->p_ublk, 512);
    ei();
}


/* Newproc fixes up the tables for the child of a fork */
/* The u_page array and the MMU are unchanged */

newproc(p)
ptptr p;    /* New process table entry */
{
    register char *j;

    /* Note that ptab_alloc clears most of the entry */
    di();
    p->p_swap = (p - ptab) * 128  + 1;  /* Allow 128 blocks per process */
    p->p_status = /*P_WAIT;*/ P_RUNNING;
    p->p_pptr = udata.u_ptab;
    p->p_ignored = udata.u_ptab->p_ignored;
    p->p_tty = udata.u_ptab->p_tty;
    ifnot(p->p_tty) /* if no tty try tty of parents parent */
	p->p_tty = udata.u_ptab->p_pptr->p_tty;	
    p->p_uid = udata.u_ptab->p_uid;
    udata.u_ptab = p;

    bzero(&udata.u_utime,4*sizeof(time_t)); /* Clear tick counters */

    rdtime(&udata.u_time);	/* set start time */

    if (udata.u_cwd)
	i_ref(udata.u_cwd);
    udata.u_cursig = udata.u_error = 0;
     
/* set default priority */
    p->p_priority = MAXTICKS; 

    for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
	if (*j >= 0)
	   ++of_tab[*j].o_refs; 
    ei(); 
}



/* This allocates a new process table slot, and fills
in its p_pid field with a unique number.  */

ptptr
ptab_alloc()
{
    register ptptr p;
    register ptptr pp;
    int	i,free_zombie();
    static int nextpid = 0;

    di();
loop:
    for(p=ptab;p < ptab+PTABSIZE; ++p)
    {
	if (p->p_status == P_EMPTY)
	    goto found;
    }
    /* not found any emty ptab. look for zombie2 process */
    if (free_zombie())
	goto loop;	/* retry */

    ei();
    return(NULL);

found:

    /* See if next pid number is unique */
nogood:
    if (nextpid++ > 32000)
	nextpid = 1;
    for (pp=ptab; pp < ptab+PTABSIZE; ++pp)
    {
	if (pp->p_status != P_EMPTY && pp->p_pid == nextpid)
	    goto nogood;
    }

    bzero(p,sizeof(struct p_tab));
    p->p_pid = nextpid;
    p->p_status = P_FORKING;
    ei();

    /* Allocate storage for udata when not running */
    ifnot (p->p_ublk = zerobuf())
	return (NULL);

    return (p);
}



/* This is the clock interrupt routine.   Its job is to
   increment the clock counters, increment the tick count of the
   running process, and either swap it out if it has been in long enough
   and is in user space or mark it to be swapped out if in system space.
   Also it decrements the alarm clock of processes.
   The user arg is true if we are in user mode */

clkint2(user)
int user;
{
    ptptr p;
    extern int in_casync,in_cawrite; /* cache.c */

    inint=1;

    RDclock();  /* read clock reg. C to clear interrupt */

    /* Increment processes and global tick counters */
    if (udata.u_ptab->p_status == P_RUNNING)
        incrtick(user ? &udata.u_utime : &udata.u_stime);

    incrtick(&ticks);

    /* Do once-per-second things */

    if (++sec == TICKSPERSEC)
    {
	/* Update global time counters */
	sec = 0;

	rdtod();  /* Update time-of-day */

	/* Update process alarm clocks */
	for (p=ptab; p < ptab+PTABSIZE; ++p)
	{
	    if (p->p_alarm)
		ifnot(--p->p_alarm)
		    ssig(p,SIGALRM);
	}
   
	/* all 120 seconds */
	if ( ++sec_counter > 120 ) {
		sec_counter = 0;
		if (in_casync==0)
			in_casync = 1; /* set flag */
	}
    }

    /* Check run time of current process */
/*    if (++runticks >= MAXTICKS && user)    /* Time to swap out */
	
/* 
check if process is interactive and connected to current  
   console. If so increment priority else decrement.
*/
#ifdef _WIN_SYS_
  if (udata.u_ptab->p_priority != MAXBACK2) {
    if (udata.u_ptab->p_tty == cur_win)
	udata.u_ptab->p_priority =  MAXINTER;
    else
	udata.u_ptab->p_priority =  MAXBACK;
    if (udata.u_ptab->p_tty == 5) /* serial uart */
	udata.u_ptab->p_priority =  MAXTICKS;
  }
#endif
    if (++runticks >= udata.u_ptab->p_priority 
	&& user && inint==1)  /* Time to swap out */
    {
	udata.u_insys = 1;
	inint = 0;
	udata.u_ptab->p_status = P_READY;
	swapout();
	udata.u_insys = 0;	/* We have swapped back in */ 
    }

    inint=0;
}


/* This sees if the current process has any signals set, and deals with them */
chksigs()
{
    register j;

    di();
    ifnot (udata.u_ptab->p_pending)
    {
	ei();
	return;
    }

    for (j=1; j < NSIGS; ++j)
    {
	ifnot (sigmask(j) & udata.u_ptab->p_pending)
	    continue;
	if (udata.u_sigvec[j] == SIG_DFL)
	{
	    ei();
#ifdef MEMDEBUG
	    kprintf("process terminated by signal: ");
#endif
	    doexit(0,j);
	}

	if (udata.u_sigvec[j] != SIG_IGN)
	{
	    /* Arrange to call the user routine at return */
	    udata.u_ptab->p_pending &= ~sigmask(j);
	    udata.u_cursig = j;
	}
    }
    ei();
}


sgrpsig(tty,sig)
int tty;
int16 sig;
{
    register ptptr p;
	
    for (p=ptab; p < ptab+PTABSIZE; ++p) {
	if (p->p_status && p->p_status != P_ZOMBIE2 && p->p_tty == tty)
            ssig(p,sig);
    }
}

ssig(proc,sig)
register ptptr proc;
int16 sig;
{    
    register stat;

    di();
    ifnot(proc->p_status)
	goto done;		/* Presumably was killed just now */

    if (proc->p_status == P_ZOMBIE2)
	goto done;

    if (proc->p_ignored & sigmask(sig))
	goto done;
   
    stat = proc->p_status;
    if (stat == P_PAUSE || stat == P_WAIT || stat == P_SLEEP)
        proc->p_status = P_READY;

    proc->p_wait = (char *)NULL;
    proc->p_pending |= sigmask(sig);
done:
    ei();
}


extern int stopflag;

/* In data.c */
extern int (*disp_tab[])();
extern int ncalls;

/* No auto vars here, so carry flag will be preserved */
unix2()
{
    udata.u_insys = 1;
    udata.u_usp = usp();
    udata.u_error = 0;

    /* Copy args from user data space, checking for access violation */
    udata.u_argn3 = ugetw(udata.u_usp+4);
    udata.u_argn2 = ugetw(udata.u_usp+3);
    udata.u_argn1 = ugetw(udata.u_usp+2);
    udata.u_argn = ugetw(udata.u_usp+1);

    udata.u_retloc = ugetw(udata.u_usp);

    if (udata.u_callno >= ncalls)
	udata.u_error = EINVAL;

    ei();

#ifdef DEBUG
    while (stopflag)
	;
    kprintf ("\t\t\t\t\tcall %d (%x, %x, %x)\n", udata.u_callno,
	udata.u_argn2, udata.u_argn1, udata.u_argn);
#endif

    /* Branch to correct routine */

    ifnot (udata.u_error)  /* Could be set by ugetw() */
	udata.u_retval = (*disp_tab[udata.u_callno])();

#ifdef DEBUG
    kprintf("\t\t\t\t\t\tcall %d ret %x err %d\n",
	udata.u_callno,udata.u_retval, udata.u_error);
#endif

    chksigs();

    di();	
    if (runticks >= udata.u_ptab->p_priority)  /* Time to swap out */
    {
	udata.u_ptab->p_status = P_READY;
	swapout();
    }
    ei();	
    udata.u_insys = 0;

    /* If an error, return errno with carry set */

    if (udata.u_error)
    {
	ret_error();
    }
    return(udata.u_retval);
}

