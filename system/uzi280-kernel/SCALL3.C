
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
 25.2.96	bug fix kill(), permisson test is now done
*/ 
/*
	this file is for sep I/D pages
*/
#define FALSE 	0
#define TRUE	1

#include "unix.h"
#include "extern.h"

#undef MEMDEBUG


/* Getpid() */
_getpid()
{
    return(udata.u_ptab->p_pid);
}

/* Getppid() */

_getppid()
{
    return(udata.u_ptab->p_pptr->p_pid);
}


/* Getuid() */

_getuid()
{
    return(udata.u_ptab->p_uid);
}


_getgid()
{
    return(udata.u_gid);
}


/*********************************
setuid(uid)
int uid;
***********************************/

#define uid (int)udata.u_argn

_setuid()
{
    int  super();
	
    if (super() || udata.u_ptab->p_uid == uid)
    {
	udata.u_ptab->p_uid = uid;
	udata.u_euid = uid;
	return(0);
    }
    udata.u_error = EPERM;
    return(-1);
}

#undef uid



/*****************************************
setgid(gid)
int gid;
****************************************/

#define gid (int16)udata.u_argn

_setgid()
{
    int  super();
	
    if (super() || udata.u_gid == gid)
    {
	udata.u_gid = gid;
	udata.u_egid = gid;
	return(0);
    }
    udata.u_error = EPERM;
    return(-1);
}

#undef gid;



/***********************************
time(tvec)
int tvec[];
**************************************/

#define tvec (int *)udata.u_argn

_time()
{
    time_t svec;

    rdtime(&svec);  /* In machdep.c */
    uput(&svec, tvec, 2*sizeof(int));
    return(0);
}

#undef tvec


/**************************************    
stime(tvec)
int tvec[];
**********************************/

#define tvec (int *)udata.u_argn

_stime()
{
/*
    int svec[2];

    ifnot (super())
    {
	udata.u_error = EPERM;
	return(-1);
    }

    uget(tvec, svec, 2*sizeof(int));
    if (udata.u_error)
	return (-1);

    sttime(tvec);
    return(0);
*/

    udata.u_error = EPERM;
    return(-1);
}

#undef tvec



/********************************************
times(buf)
char *buf;
**********************************************/

#define buf (char *)udata.u_argn

_times()
{
    int  valadr();

    ifnot (valadr(buf,6*sizeof(time_t)))
	return(-1);

    di();
    uput(&udata.u_utime, buf, 4*sizeof(time_t));
    uput(&ticks, buf + 4*sizeof(time_t), sizeof(time_t));
    ei();
    return(0);
}

#undef buf


/*************************************
signal(sig, func)
int16 sig;
int16 (*func)();
***************************************/

#define sig (int16)udata.u_argn1
#define func (int (*)())udata.u_argn

_signal()
{
    int retval,pvaladr();

    di();
/*    if (sig < 1 || sig == SIGKILL || sig >= NSIGS) */
    if (sig < 1 || sig >= NSIGS)
    {
	udata.u_error = EINVAL;
	goto nogood;
    }

    if (func == SIG_IGN)
	udata.u_ptab->p_ignored |= sigmask(sig);
    else
    {
	if (func != SIG_DFL && !pvaladr((char *)func, 1))
	{
	    udata.u_error = EFAULT;
	    goto nogood;
	}
	udata.u_ptab->p_ignored &= ~sigmask(sig);
    }
    retval = udata.u_sigvec[sig];
    udata.u_sigvec[sig] = func;
    ei();
    return(retval);

nogood:
    ei();
    return(-1);
}

#undef sig
#undef func


    
/**************************************
kill(pid, sig)
int16 pid;
int16 sig;
*****************************************/

#define pid (int16)udata.u_argn1
#define sig (int16)udata.u_argn

_kill()
{
    ptptr p;
    
    if (sig <= 0 || sig > 15)
	goto nogood;

/*
    ifnot (pid)
    {
	sgrpsig(udata.u_ptab->p_tty, sig);
	return (0);
    }
*/

    for (p=ptab; p < ptab+PTABSIZE; ++p)
    {
/* 
	SN wcy permission check 
*/
	if (p->p_pid == pid)
	{
		if (udata.u_ptab->p_uid == p->p_uid || super() ) 
		{
	    		ssig(p,sig);
	    		return(0);
		}
		else
		{
			udata.u_error = EPERM;
			return (-1);
		}
 	}
    }

nogood:
	udata.u_error = EINVAL;
	return(-1);
}

#undef pid
#undef sig



/********************************
alarm(secs)
uint16 secs;
*********************************/

#define secs (int16)udata.u_argn

_alarm()
{
    int retval;

    di();
    retval = udata.u_ptab->p_alarm;
    udata.u_ptab->p_alarm = secs;
    ei();
    return(retval);
}

#undef secs

/* 
	Called from getpage and pg_alloc. 
	Free pages looked by oldest zombie process. 
*/
free_zombie()
{
	register unsigned found,j,bestage;
	ptptr    p,bestprog;
	pg_descr *page;

	found   = FALSE;
	bestage = 32100;  /* greatest pid is 32000 */
        
	for (p=ptab;p<ptab+PTABSIZE;++p) 
		if (p->p_status==P_ZOMBIE2) {
		        j = p->p_uid; /* LRU counter */
			if ( j < bestage ) {
				bestage  = j;
				bestprog = p;
				found 	 = TRUE; 
			}
		}
		
	if(!found)
		return(FALSE);

	/* free the pages */
	if (bestprog->p_bintype) 
		found = 8;
	else
		found = 16;

	for (j=0;j<found;j++)
		if (realpage( (page=bestprog->p_ublk->u_d.u_page[j]) )) 
			pg_free(page);

        /* release udata storage block */
	brelse(bestprog->p_ublk);
	bestprog->p_status = P_EMPTY;
	return(TRUE); /* return success */
}
	