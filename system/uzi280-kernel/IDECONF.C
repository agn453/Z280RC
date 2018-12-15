
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

/* UZI280	IDECONF.C

	Partition and hard disc data
 */ 

#include "config.h"

#ifdef Z280RC

/* drive specific data (128MB CompactFlash) */
unsigned WD_Cyls   = 1024;
unsigned WD_Heads  = 1;		/* 256 sectors per track */
unsigned WD_Sector = 256;

/* Partition Data */
unsigned TrackOffset = 512;	/* 8x 8MB CP/M partitions. Track offset to 
				   first UZI280 Partition */
/* Following UZI partitions track offsets */
unsigned WD1_Offset = 256;	/* WD0 32 MB */	
unsigned WD2_Offset = (2*256);	/* WD1 32 MB, WD2 32 MB */

#else

/* drive specific data (ST-3243A) */
unsigned WD_Cyls   = 1024;
unsigned WD_Heads  = 12;
unsigned WD_Sector = 34;

/* Partition Data */
unsigned TrackOffset = 400;	/* 80 MB CPM partition. Track offset to 
				   first UZI280 Partition */
/* Following UZI partitions track offsets */
unsigned WD1_Offset = 161;	/* WD0 32 MB */	
unsigned WD2_Offset = (2*161);	/* WD1 32 MB, WD2 32 MB */

#endif

