
/*-
 * All UZI280 source code is 
 * (c) Copyright (1990-95) by Stefan Nitschke and Doug Braun.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, (2) it is not used for military purpose in
 * any form, and (3) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by Stefan Nitschke and his contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the author nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* These are the global data structures */

#ifdef MAINpro
#define extern  
#endif


/* This is accessed by the macro udata which is really ub.u_d */


extern struct u_block ub;
 
#define page16 0xe000
#define udata ub.u_d

extern struct p_tab ptab[PTABSIZE];

extern inoptr root;	/* Address of root dir in inode table */
extern int16 ROOTDEV;	/* Device number of root filesystem. */

extern struct cinode i_tab[ITABSIZE];	/* In-core inode table */
extern struct oft of_tab[OFTSIZE];	/* Open File Table */

extern struct filesys fs_tab[NDEVS];	/* Table entry for each
					   device with a filesystem. */
extern struct blkbuf bufpool[NBUFS];

extern ptptr initproc;	/* The process table address of the first process. */
extern int16 inint;	/* flag is set whenever interrupts are being serviced */

extern int16 sec;	/* Tick counter for counting off one second */
extern int16 runticks;	/* Number of ticks current process has been
			 swapped in */

extern time_t tod;	/* Time of day */
extern time_t ticks;	/* Cumulative tick counter, in minutes and ticks  */

extern char *swapbase;	/* Used by device driver for swapping */
extern unsigned swapcnt;
extern blkno_t swapblk;

extern uint16 waitno;	/* Serial number of processes entering wait state */

#ifdef MAIN
#undef extern
#endif

