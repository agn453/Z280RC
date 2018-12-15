
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

/* These are the global data structures */

#ifdef MAIN
#define extern  
#endif


/* This is accessed by the macro udata which is really ub.u_d */


extern struct u_block ub;
 
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

