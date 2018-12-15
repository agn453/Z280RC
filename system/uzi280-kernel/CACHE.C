
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
 
	UZI280_kernel :  CACHE.C
 
  Very simple read cache for block device wd0-2
  DMA0 is used for mem<->mem copy

  10.11.92 St. Nitschke 

  05/07/94	SN
  		added read/write cache support. still very simple
  		implementation with hash function 
*/

#define FALSE 0
#define TRUE 1

#include "config.h"
#include "ide.h"	/* hard disk parameter */
#include "cache.h"	/* memory configuration */ 

extern dma_write(),dma_read();
extern	highadr,lowadr;

/* from devwd.c */
extern unsigned dptrlo,dptrhi,wdtrk,wdsec,wd_rdwr;
extern wd_io();

static unsigned  key;
int in_casync = 0;	/* 1 : in ca_sync, 2 : ca_sync time out */
int in_cawrite =0;
  	
static
in_cache(dev,blkno)
char	 dev;
unsigned blkno;
{
	register struct ca_buf *match;   
/*
 very simple hash function 
*/
	key     = (100*(unsigned)dev+(unsigned)blkno) % (unsigned)max_cache;

	match   = cach_buf + key;
/*
   calculate the physical address of the buffer in dma format
*/
	highadr =  (unsigned)C_Start + (key/8)*0x0010;
	lowadr  = (key%8)*512; 
/*
  check if block and device number are identical
*/	
	if(match->blkno != blkno)
		return(FALSE);
	if(match->dev != dev)
		return(FALSE);
	return(TRUE);
}
/*
	try to read block from hard disc cache
	
	return	:	TRUE  found and read
			FALSE not found
*/
ca_read(dev,blkno)
char	 dev;
unsigned blkno;
{
	if(in_cache(dev,blkno)==TRUE)
	{
		dma_read();
	  	return(TRUE);
	}
	return(FALSE);
}


ca_write(dev,blkno)
char	 dev;
unsigned blkno;
{
	in_cawrite = 1;
	if (!in_cache(dev,blkno)) { /* if already in cache nothing to write */
          /* else we must look if the new buffer is dirty */
	  if (cach_buf[key].dirty == TRUE)
			cache_write(key);	/* write out dirty block */
	  cach_buf[key].dev  =dev;
          cach_buf[key].blkno=blkno;
        }
	if (wd_rdwr)	/* if we are writing the dirty flag must be set */
 		cach_buf[key].dirty = TRUE;
	dma_write(); /* copy the data from incore to cache buffer */
	in_cawrite = 0;
	return(0);
}
	
init_cache()
{
	register i;
	for(i=0;i<max_cache;i++) {
		cach_buf[i].dev = -2;
		cach_buf[i].dirty = FALSE;
	}
}
/*
	write all dirty cache buffer to device
*/
ca_sync()
{
	register i;

	in_casync = 2;
	for(i=0;i<max_cache;i++) 
		if (cach_buf[i].dirty == TRUE) {
			cache_write(i);
		}
	in_casync = 0;
}
/*
	interrupt routine to clear cache 
*/
int_ca_sync()
{
	static int i=0;

	in_casync = 2;	
	for(;i<max_cache;i++) 
		if (cach_buf[i].dirty == TRUE) {
			cache_write(i);
			in_casync = 1;
			return;
		}
	in_casync = 0;  /* reset flag */
	i = 0;
}

static unsigned s_ptrlo,s_ptrhi,s_rdwr,s_sec,s_trk;

/*
	write dirty buffer to disk
*/
cache_write(key)
{
	register struct ca_buf *use;
	static unsigned SecHead;

	SecHead = WD_Heads*WD_Sector;
	use = cach_buf + key;
	/* save ext. vars */
	s_ptrlo = dptrlo;
	s_ptrhi = dptrhi;
	s_rdwr  = wd_rdwr;
	s_trk	= wdtrk;
	s_sec	= wdsec;
	/* set temp values    */
	dptrhi  = (unsigned)C_Start + (key/8)*0x0010;
	dptrlo  = (key%8)*512;
	wdtrk	= (use->blkno / SecHead) + TrackOffset;
	wdsec   = use->blkno % SecHead;
	if (use->dev == 2)
		wdtrk += WD2_Offset;
	if (use->dev == 1)
		wdtrk += WD1_Offset;

	wd_rdwr = 1;		/* set write */
	wd_io();		/* write to disk */
	use->dirty = FALSE;	/* reset dirty flag */

	/* restore old settings */
	dptrlo  = s_ptrlo;
	dptrhi  = s_ptrhi;
	wd_rdwr = s_rdwr;
	wdtrk   = s_trk;
	wdsec   = s_sec; 
}
