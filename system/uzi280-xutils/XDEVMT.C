
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


#define MTLUN 0
#define MTBUSID 2

/*
static char cmdblk[6] = { 0, (MTLUN<<5)|01, 0, 0, 0, 0 }; 

extern char *dptrlo;
extern pageaddr dptrhi;
extern int dlen;
extern char *cptr;
extern int busid;
extern scsiop();
*/

mt_read(minor, rawflag)
int minor;
int rawflag;
{
    return(0);
}


mt_write(minor, rawflag)
int minor;
int rawflag;
{
    return(0);
}


mt_open()
{
    return(0);
}



mt_close()
{
    return(0);
}

