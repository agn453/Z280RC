
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


extern BIOS(char, unsigned);


#define NUMTRKS 80  /* 0..79 */
#define NUMSECS 15  /* 1..15 */

static uint16 ftrack, fsector, ferror;
static char *fbuf;



fd_read(minor, rawflag)
int16 minor;
int rawflag;
{
    return (fd(1, minor, rawflag));
}

fd_write(minor, rawflag)
int minor;
int rawflag;
{
    return (fd(0, minor, rawflag));
}



fd(rwfl, minor, rawflag)
int rwfl;
int minor;
int rawflag;
{
    register uint16 nblocks;
    register uint16 firstblk;

    if (rawflag)
    {
	panic("Raw floppy access");

 	if (rawflag == 2)
	{
            nblocks = swapcnt >> 9; /*7;*/
	    fbuf = swapbase;
   	    firstblk = swapblk;
	}
	else
	{
            nblocks = udata.u_count >> 9;
  	    fbuf = udata.u_base;
	    firstblk = udata.u_offset.o_blkno;
	}	
    }
    else
    {
	nblocks = 1;
	fbuf = udata.u_buf->bf_data;
	firstblk = udata.u_buf->bf_blk;
    }

    ftrack  = (firstblk / NUMSECS) + 2;  /* Track 0-1 reserviert */
    fsector = (firstblk % NUMSECS) + 1;
    ferror = 0;

    for (;;)
    {
        if (rwfl)
	    _1read();
	else
	    _1write();

	ifnot (--nblocks)
  	    break;

        if (++fsector == NUMSECS + 1 )
	{
	    fsector = 1;
	    ++ftrack;
	}
	fbuf += 512;
    }
    if (ferror)
    {
	printf("fd_%s: error %d track %d sector %d\n",
		    rwfl?"read":"write", ferror, ftrack, fsector);
	panic("");
    }

    return(nblocks);
}


fd_open(minor)
int minor;
{
    return(0);
}


fd_close(minor)
int minor;
{
    return(0);
}


fd_ioctl(minor)
int minor;
{
    return(-1);
}


_1read()
{
  char temp;
  temp = (char)bdos(25);
  bdos(14,0);
  BIOS(12,(unsigned)fbuf);    /* set DMA addr    */
  BIOS(10,ftrack);  /* set Track       */
  BIOS(11,fsector); /* set sector      */
  BIOS(13,0);       /* read            */
  bdos(14,temp);
}


_1write()
{
  char temp;
  temp = (char)bdos(25);
  bdos(14,0);
  BIOS(12,(unsigned)fbuf);    /* set DMA addr    */
  BIOS(10,ftrack);  /* set Track       */
  BIOS(11,fsector); /* set sector      */
  BIOS(14,0);       /* write           */
  bdos(14,temp);
}

