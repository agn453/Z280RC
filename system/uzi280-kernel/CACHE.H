
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


/* 	CACHE.H		*/


typedef struct ca_buf
{
	char	  dirty;	/* must be written to disk */
	char	  dev;		/* device number */
	unsigned  blkno;	/* device block number */
/*	struct ca_buf *ca_next;	/* pointer to next buffer in list */
/*	unsigned  buf_no;	/* number of buffer 0..max_cache */
} ca_buf;

#define HASH	100
#define M_DEV	2
/*
struct ca_buf	*free;
struct ca_buf 	*read [HASH][M_DEV];
struct ca_buf 	*dirty[HASH][M_DEV];
static		n_free,n_read,n_dirty;
*/

#define CACHE

#include "config.h"    /* config.h defines cach_buf[max_cache] */


/* use the next shorter prime number for max_cache */
/*#define max_cache 1013	/* old value 1016 calculated in config.h */
