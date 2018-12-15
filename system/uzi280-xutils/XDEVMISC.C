
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

kmem_read(minor, rawflag)
int minor;
int rawflag;
{
    uint16  addr;

    addr = 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    /* This test should be based on a check of the page tables */
    if (addr < 0x1000)
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
    uint16 addr;

    addr = 512*udata.u_offset.o_blkno+udata.u_offset.o_offset;

    if (addr < 0x1000)
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
   uint16 n;

    n = udata.u_count;
    while (n--)
	lpout(*udata.u_base++);
    return(udata.u_count);
}


lpout(c)
char c;
{
/*
    while(in(0x84)&02)
	;

    out(c,0x85);
    out(0xfe,0x86);
    out(0xff,0x86);
    out(0xff,0x85);
*/
}


#include "xdevmt.c"
