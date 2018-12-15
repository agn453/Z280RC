
/*-
 * All UZI280 source code is 
 * (c) Copyright (1990-95) by Stefan Nitschke and Doug Braun.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, (2) it is not used for military purpose in
 * any form, (3) it is not used for any commercial purpose, and (4) 
 * distributions including binaries display the following acknowledgement:
 * ``This product includes software developed by Stefan Nitschke 
 *   and his contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the author nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "unix.h"
#include "extern.h"


/* This is called at the very beginning to initialize everything. */
/* It is the equivalent of main() */

fs_init()
{
    inint = 0;
    udata.u_euid = 0;
    udata.u_insys = 1;
}


/* This checks to see if a user-suppled address is legitimate */
valadr(base,size)
char *base;
uint16 size;
{
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



sttime()
{
    panic("Calling sttime");
}


rdtime(tloc)
time_t *tloc;
{
    tloc->t_time = (20>>1) | (20<<5) | (12<<11);
    tloc->t_date = 19 | (04<<5) | (92<<9);
}


/* Update global time of day */
rdtod()
{
    tod.t_time = (20>>1) | (20<<5) | (12<<11);
    tod.t_date = 19 | (04<<5) | (92<<9);
}



/* This prints an error message and dies. */

panic(s)
char *s;
{
    inint = 1;
    printf("PANIC: %s\n",s);
    idump();
    _exit();
}


warning(s)
char *s;
{
    printf("WARNING: %s\n",s);
}





idump()
{
    inoptr ip;
    ptptr pp;
    extern struct cinode i_tab[];
    bufptr j;

    printf(
        "   MAGIC  DEV  NUM  MODE  NLINK (DEV) REFS DIRTY err %d root %d\n",
            udata.u_error, root - i_tab);

    for (ip=i_tab; ip < i_tab+ITABSIZE; ++ip)
    {
	printf("%d %d %d %u 0%o %d %d %d %d\n",
	       ip-i_tab, ip->c_magic,ip->c_dev, ip->c_num,
	       ip->c_node.i_mode,ip->c_node.i_nlink,ip->c_node.i_addr[0],
	       ip->c_refs,ip->c_dirty);
	ifnot (ip->c_magic)	
	    break;
    }

    printf("\n   STAT WAIT   PID PPTR  ALARM  PENDING  IGNORED\n");
    for (pp=ptab; pp < ptab+PTABSIZE; ++pp)
    {
	printf("%d %d    0x%x %d %d  %d 0x%x 0x%x\n",
	       pp-ptab, pp->p_status, pp->p_wait,  pp->p_pid,
	       pp->p_pptr-ptab, pp->p_alarm, pp->p_pending,
		pp->p_ignored);
        ifnot(pp->p_pptr)
	    break;
    }	
    
    printf("\ndev blk drty bsy\n");
    for (j=bufpool; j < bufpool+NBUFS; ++j)
	printf("%d %u %d %d\n",j->bf_dev,j->bf_blk,j->bf_dirty,j->bf_busy);

    printf("\ninsys %d ptab %d call %d cwd %d sp 0x%x\n",
	udata.u_insys,udata.u_ptab-ptab, udata.u_callno, udata.u_cwd-i_tab,
       udata.u_ssp);
}



