
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

#define LINESIZ 128

static char line[LINESIZ];

tty_read(minor, rawflag)
int16 minor;
int rawflag;
{
    int nread;

    line[0] = udata.u_count;
    line[1] = 0;
    bdos(10,line);  /* Read console buffer */
    bdos(2,'\n');
    nread = line[1];
    line[nread+2] = '\n';
    bcopy(line+2,udata.u_base,nread+1);
    return(nread+1);
}


tty_write(minor, rawflag)
int16 minor;
int rawflag;
{
    while (udata.u_count-- != 0)
    {
        if (*udata.u_base=='\n')
	    bdos(2,'\r');
        bdos(2,*udata.u_base);
	++udata.u_base;
    }
}


_putc(c)
int c;
{
    bdos(2,c);
}


tty_open(minor)
int minor;
{
    return(0);
}


tty_close(minor)
int minor;
{
    return(0);
}


tty_ioctl(minor)
int minor;
{
    return(-1);
}
