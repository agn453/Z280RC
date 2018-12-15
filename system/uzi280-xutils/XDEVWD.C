
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
#include "ide.h"


/* Die folgenden Variablen werden im Assembler-Module verwendet */
unsigned dptrlo;
unsigned wdsec,wdtrk;
unsigned wd_rdwr;


static unsigned nblocks;

wd_read(minor, rawflag)
unsigned minor;
int rawflag;
{
    wd_rdwr = 0;
    return(setup(minor, rawflag));
}

wd_write(minor, rawflag)
unsigned minor;
int rawflag;
{
    wd_rdwr = 1;
    return(setup(minor, rawflag));
}

static blkno_t block;


setup(minor, rawflag)
int minor;
int rawflag;
{
    static unsigned SecHead;

    SecHead = WD_Sector*WD_Heads;		
    if (rawflag)
    {

	if (rawflag == 2)
        {
	    /* Paging */
	    nblocks = swapcnt >>9;
	    dptrlo  = swapbase;
            block   = swapblk;
        }
	else
        {
	    /* Raw I/O (broken) */
	    nblocks = udata.u_count >>9;
  	    dptrlo  = udata.u_base;
            block   = udata.u_offset.o_blkno;
	 }
    }
    else
    {
        nblocks = 1;
        dptrlo  = udata.u_buf->bf_data;
        block   = udata.u_buf->bf_blk;
    }

    wdtrk = (block / SecHead) + TrackOffset;
    wdsec = block % SecHead;
    
    if (minor == 1)
      wdtrk += WD1_Offset;

    if (minor == 2)
      wdtrk += WD2_Offset; 

    for (;;)
    {
       wd_io();
   
       ifnot (--nblocks)
         break;
      
       if (++wdsec == SecHead)
       {
          wdsec = 0;
          ++wdtrk;
       }
       dptrlo += 512;
    }
 
    return(nblocks);
}


wd_open(minor)
int minor;
{
    hdinit(); 
    return(0);
}

	
 
 
