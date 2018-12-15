
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



#define NUMTRKS 76
#define NUMSECS 26


fd_read(minor, rawflag)
int16 minor;
int rawflag;
{
    return (0);
}

fd_write(minor, rawflag)
int16 minor;
int rawflag;
{
    return (0);
}

#ifdef FLOPPY
static
fd(rwflag, minor, rawflag)
int rwflag;
int minor;
int rawflag;
{
    register unsigned nblocks;
    register unsigned firstblk;

    if (rawflag)
    {
	panic("Raw floppy access");

 	if (rawflag == 2)
	{
            nblocks = swapcnt >> 7;
	    fbuf = swapbase;
   	    firstblk = 4*swapblk;
	}
	else
	{
            nblocks = udata.u_count >> 7;
  	    fbuf = udata.u_base;
	    firstblk = udata.u_offset.o_blkno * 4;
	}	
    }
    else
    {
	nblocks = 4;
	fbuf = udata.u_buf->bf_data;
	firstblk = udata.u_buf->bf_blk * 4;
    }

    ftrack = firstblk / 26; 
    fsector = firstblk % 26 + 1;
    ferror = 0;

    for (;;)
    {
        if (rwflag)
	  /*  read()  */ ;
	else
	  /*  write() */ ;

	ifnot (--nblocks)
  	    break;

        if (++fsector == 27)
	{
	    fsector = 1;
	    ++ftrack;
	}
	fbuf += 128;
    }

    if (ferror)
    {
	kprintf("fd_%s: error %d track %d sector %d\n",
		    rwflag?"read":"write", ferror, ftrack, fsector);
	panic("");
    }

    return(nblocks);
}
#endif

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

#ifdef FLOPPY
/****************** not implemented on cpu280 SN ********** 

#asm 8080
; ALL THE FUNCTIONS IN HERE ARE STATIC TO THIS PACKAGE

;
;THESE ARE 1771 FLOPPY DISK CONTROLLER COMMANDS,
;I/O PORT ADDRESSES, AND FLAG MASKS:
;
RESTOR EQU 	08H	;6MS STEP,LOAD HEAD
SEEK EQU 	18H	;6MS STEP,LOAD HEAD
READC EQU 	88H	;NO HEAD LOAD
WRITEC EQU 	0A8H	;NO HEAD LOAD
RESET EQU 	0D0H	;RESET STATUS COMMAND
STATUS EQU 	80H
COMAND EQU 	80H
TRACK EQU 	81H
SECTOR EQU 	82H
DATA EQU 	83H
BUSY EQU 	01
RDMASK EQU 	10011111B
WRMASK EQU 	0FFH
;
;
;THIS FLOPPY READ ROUTINE CALLS FREAD2. IF THE
;READ IS UNSUCCESSFUL, THE DRIVE IS HOMED,
;AND THE READ IS RETRIED.
;

read?:	PUSH	B
	CALL	FREAD2
	LDA	ferror?
	ANA	A	;SET FLAGS
	JZ	FRDONE	;READ WAS OK
	CALL	FHOME
	CALL	FREAD2
FRDONE:
	POP	B
	RET

;
FREAD2:	CALL	FWAIT
	LDA	fsector?
	OUT	SECTOR
	CALL	FTKSET
	MVI	A,11
FREAD3:	STA 	TRYNUM
	CALL	FWAIT
	LHLD	fbuf?
	MVI	C,DATA
	MVI	B,128
	MVI	D,03
	MVI	A,READC
	DI		;ENTERING CRITICAL SECTION
	OUT	COMAND
	XTHL	;SHORT DELAY
	XTHL
	XTHL
	XTHL	
	XTHL
	XTHL
	XTHL
	XTHL
?1LOOP:	IN	STATUS
	ANA	D
	DCR	A
	JZ	?1LOOP
.Z80
	INI
.8080
	JNZ	?1LOOP
	EI		;LEAVING CRITICAL SECTION
	CALL	FWAIT
	IN	STATUS
	ANI	RDMASK
	JZ	FR1END
	LDA	TRYNUM
	DCR	A
	JNZ	FREAD3
	MVI	A,1
FR1END:	STA	ferror?
	RET
;
;THIS IS THE FLOPPY WRITE ROUTINE. IT CALLS
;THE ACTUAL WRITING SUBROUTINE, AND IF IT
;WAS UNSUCCESSFUL, RESETS THE HEAD AND
;TRIES AGAIN.
;

write?: PUSH	B
	CALL	FWR2
	LDA	ferror?
	ANA	A
	JZ	FWDONE
	CALL	FHOME
	CALL	FWR2
	LDA	ferror?
FWDONE:	POP	B
	RET



;
FWR2:	CALL	FWAIT
	LDA	fsector?
	OUT	SECTOR
	CALL    FTKSET
FWR4:	MVI	A,11
FWR3:	STA	TRYNUM
	CALL	FWAIT
	LHLD	fbuf?
	MVI	C,DATA
	MVI	B,128
	MVI	D,01
	MVI	A,WRITEC
	DI		;ENTERING CRITICAL SECTION
	OUT	COMAND
?3A:	IN	STATUS
	ANI	01
	JZ	?3A
?3B:	IN 	STATUS
	ANI	02
	JNZ	?2LOOP
	IN	STATUS
	ANI	01
	JZ	??ERROR
	JMP	?3B
?2LOOP:	IN	STATUS
	XRA	D
	JZ	?2LOOP
.Z80
	OUTI
.8080
	JNZ	?2LOOP
	EI		;LEAVING CRITICAL SECTION
	CALL	FWAIT
	IN	STATUS
	ANI	WRMASK
	JZ	FW2END
??ERROR: LDA	TRYNUM
	DCR	A
	JNZ	FWR3
	MVI	A,1
FW2END:	STA	ferror?
	RET
;
;THESE 3 SUBROUTINES ARE USED BY THE FREAD2 AND
;FWR2 ROUTINES:
;
FTKSET:	CALL	FWAIT
	LDA	ftrack?
	OUT	DATA
	XTHL	
	XTHL
	XTHL
	XTHL
	MVI	A,SEEK
	OUT	COMAND
	XTHL	
	XTHL
	XTHL
	XTHL
	XTHL	
	XTHL
	XTHL
	XTHL
	RET
;
FWAIT:	IN	STATUS
	XTHL	
	XTHL
	XTHL
	XTHL
	ANI	10000001B
	JNZ	FWAIT
	RET
;
reset?:
FHOME:	CALL	FWAIT
	MVI	A,RESTOR
	OUT	COMAND
	XTHL	
	XTHL
	XTHL
	XTHL
	RET
;

        

TRYNUM: DS	1
#endasm

#endif

