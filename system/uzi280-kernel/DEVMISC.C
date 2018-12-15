
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


#include "unix.h"
#include "extern.h"

kmem_read(minor, rawflag)
int minor;
int rawflag;
{
    unsigned addr;

    addr = 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    /* This test should be based on a check of the page tables */
    if (addr < 0x006C)
	return (1);

    if (udata.u_sysio)
	bcopy((char *)addr, udata.u_base, udata.u_count);
    else
	uput((char *)addr, udata.u_base, udata.u_count);

    return(udata.u_count);
}

kmem_write(minor, rawflag)
int minor;
int rawflag;
{
    unsigned addr;

    addr = 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    if (addr < 0x006C)
	return (1);

    if (udata.u_sysio)
	bcopy(udata.u_base, (char *)addr, udata.u_count);
    else
	uget(udata.u_base, (char *)addr, udata.u_count);

    return(udata.u_count);
}



null_write(minor, rawflag)
int minor;
int rawflag;
{
    return(udata.u_count);
}



static char lop = 0;

lpr_open()
{
    lop = 1;
    return(0);
}

lpr_close()
{
    if (lop)
    {
	lop  = 0;
        lpout('\f');
        lpout('\f');
    }
    return(0);
}



lpr_write(minor, rawflag)
int minor;
int rawflag;
{
    unsigned n;

    n = udata.u_count;
    while (n--)
	lpout(*udata.u_base++);
    return(udata.u_count);
}


lpout(c)
char c;
{
/*
	unsigned in();

    while( (in(0x85)&0x80) == 0x80) /* wait for device ready 
	;
    out(c,0x85);
*/
}


#include "devmt.c"

