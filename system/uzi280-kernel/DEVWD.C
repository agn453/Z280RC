
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
	23/07/94	SN	
		IDE-Support

	05/07/94	SN
		cache support
		
	this seems do work perfekt, but rawflag == 2 (paging) not 
	correct implemented. (better never use)  SN
*/

#include "unix.h"
#include "extern.h"

/* Platten spezifische Daten */
#include "ide.h"


unsigned dptrlo;	/* lo memory address in DMA format */
pageaddr dptrhi;	/* hi   "        "    "  "    "    */  
unsigned hdblk;
unsigned wdsec,wdtrk;   /* hard disk sector andd track */
unsigned wd_rdwr;	/* global io flag  0 = reading, 1 = writing */
extern wd_io();		/* low level device driver */
extern wd_init();	/* hard disc init */	
static setup();
static int  hd_flag;  	/* flag for hdinit */ 


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


static
setup(minor, rawflag)
unsigned minor;
int rawflag;
{
	pageaddr padr();
	int	ca_read();
	static  unsigned SecHead;


    SecHead = WD_Heads*WD_Sector;	
    if (rawflag)
    {

	if (rawflag == 2)
        {
	    /* Paging */
/*	    hdblk   = swapcnt >>9;
	    dptrhi  = swapbase;
	    dptrlo  = 0;
            block   = swapblk;
*/
	    kprintf("no swap\n");
	    return(2);	
        }
	else
        {
	    /* Raw I/O (broken) */
	    hdblk   = udata.u_count >>9;
  	    dptrlo  = (unsigned)udata.u_base;
	    dptrhi  = padr(1, dptrlo);
            block   = udata.u_offset.o_blkno;
	 }
    }
    else
    {
        hdblk   = 1;
        dptrlo  = (unsigned)udata.u_buf->bf_data;
        dptrhi  = padr(0, dptrlo);
        block   = udata.u_buf->bf_blk;
    }
    dptrlo &= 0x0fff;	
    wdsec = block % SecHead;
	/*
		track offsets 
	*/
    wdtrk = (block / SecHead) + TrackOffset;

    if (minor == 2)
      wdtrk += WD2_Offset; 
    
    if (minor == 1)
      wdtrk += WD1_Offset; 

    for (;;)
    {
    	if (wd_rdwr == 1) { /* writing */

		ca_write(minor,block);  /* write incore buffer into cache, 
					   ca_write does the necessary dev 
					   IO on dirty cache buffers */
	    }
 	else	     /* reading */
	    {
	    /* try to read from cache */
	   	if( ! ca_read(minor,block) ) {
		   	/* reading block not in cache */
	       		wd_io();   /* read data from device into incore 
				      buffer */
	       		ca_write(minor,block); /* write into cache */
	       	}
	    }
       	if (! --hdblk)
         break;
      
       if (++wdsec == SecHead)
       {
          wdsec = 0;
          ++wdtrk;
       }
       dptrlo += 512; /* next 512 bytes */

       if (dptrlo >= 4096) {
		dptrlo -= 4096;
		dptrhi += 0x010;
       }

     }
     return(hdblk);
}


wd_open(minor)
int minor;
{
  if (hd_flag == 0)
  {
    wd_init(); 
    hd_flag = 1;
  }
  return(0);
}


wd_init()
{
        hdinit();
}
/*
	called bei "IDE.AS"
*/

wd_error()
{
	warning("HD IO err");
}

