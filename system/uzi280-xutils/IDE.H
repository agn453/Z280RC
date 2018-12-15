
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

/* UZI280	IDE.H

	Partitions und Plattendaten externals (ideconf.c)
*/ 


extern unsigned WD_Heads;
extern unsigned WD_Sector;
extern unsigned SecHead;

/* Partitions Daten */
extern unsigned  TrackOffset;	/* 80 MB CPM-Partition, Offset der ersten
				   UZI280 Partition */
extern unsigned WD1_Offset;	/* WD0 32 MB */	
extern unsigned WD2_Offset;	/* WD1 32 MB */
