;
;/*-
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
;
;	title	'Harddisk Driver Module of Z280RC with DMA3'
;
; 930414 Tilmann Reh
; 930723 Stefan Nitschke	Version for UZI280
; 20181212 Tony Nicholson	Version for Z280RC IDE CompactFlash
;

*Include	z280rc.inc	; Z280 I/O port symbols

*Include	config.inc	; System configuration

*Include	macro.lib

Stand_Alone	equ	0	; not tested yet
CPM_Boot	equ	1	; means UZI is booted from CP/M

	global	_WD_Cyls,_WD_Heads,_WD_Sector ; defined in ideconf.c

	psect      text

	global     _wdsec,_wdtrk,_dptrlo,_wd_rdwr,_wd_io,_hdinit

COND XUTIL .eq. 0	; XUTIL programs don't use these under CP/M

	global	_dptrhi,_iopg00,_iopgff

	global	_wd_error,_rw_flag
ENDC
		
COND	CPM_Boot
_hdinit:
	ld	hl,0	; Started from CP/M - no initialisation required
	ret
ENDC

COND	Stand_Alone
_hdinit:
	ret		; Stand-alone init **not yet implemented**
ENDC

; write/read entry from devwd.c

_wd_io:

COND	XUTIL
	call	_iopg00		; Select CF I/O page
ENDC

loop01:
	in	a,(cfstat)	; Spin until not busy (bit 7 clear)
	and	080h
	jr	nz,loop01

	ld	a,0E0h		; Logical addressing mode (bit 6 high)
	out	(cf2427),a	; Unit number 0 (bit 4), bits 7&5 high

	ld	hl,(_wdtrk)	; Track number maps to bits 23..8
	ld	a,h		;  of logical block number
	out	(cf1623),a
	ld	a,l
	out	(cf815),a

	ld	a,(_wdsec)	; Sector is 0..255 and maps to
	out	(cf07),a	;  bits 7..0 of logical block number

	ld	a,1		; One sector
	out	(cfsectcnt),a

loop02:	in	a,(cfstat)	; Spin until not BUSY (bit 7 off) and
	and	0c0h		;  Drive Ready (bit 6 on)
	xor	040h
	jr	nz,loop02

	ld	hl,(_wd_rdwr)	; Is is a Read or Write?
	ld	a,l
	or	h
	jr	nz,write	; Write

	ld	a,020h		; Read command
	out	(cfcmd),a

loop04:	in	a,(cfstat)	; Spin until not BUSY (bit 7 off) and
	and	088h		;  DRQ (bit 3 on)
	xor	008h
	jr	nz,loop04

	in	a,(cfstat)	; Error (bit 0 on)?
	and	001h
	jr	z,contrd	; No, continue read

error:

COND	XUTIL
	call	_iopgfe
ENDC
	call	_wd_error
	ret

contrd:

COND XUTIL .eq. 0

	; Read from cfdata I/O port using DMA3
	; This must be to a buffer with an *even* address
	; since the data is read in 16-bit mode from
	; the CompactFlash drive

	ld	hl,cfdata	; Low 12-bit I/O address
	ld	(hdsrc),hl
	ld	l,h
	ld	h,cfp		; High 12-bit I/O page
	ld	(hdsrc+2),hl
	ld	hl,idebuf	; make sure buffer address is *EVEN*
	ld	a,l
	and	01H
	jr	z,even0
	inc	hl
even0:	ld	(hddest),hl	; dest is bounce buffer
	ld	l,h
	ld	h,01h		; System page
	ld	(hddest+2),hl
	ld	hl,256		; WORD count
	ld	(count),hl
	ld	hl,1110001100000000B
		; EN (bit 15)=1
		; Source is I/O address unmodified (bits 14..12)=110b
		; No interrupt (bit 11)=0
		; Word size (bits 10..9)=01b
		; Continuous mode (bits 8..7)=10b
		; Flowthrough (bits 6..5)=00b
		; Transfer complete (bit 4)
		; Destination Auto-increment memory (bits 3..1)=000b
		; End-of-process Signalled
	call	hddmaio		; Start DMA

	; Now use DMA3 in byte mode to copy the buffer
	; to the _dptr{hi,lo} address (which can be an odd or even
	; address).

	ld	hl,idebuf	; make sure buffer address is even
	ld	a,l
	and	01H
	jr	z,even1
	inc	hl
even1:	ld	(hdsrc),hl	; source is bounce buffer
	ld	l,h
	ld	h,01h		; System page
	ld	(hdsrc+2),hl
	ld	hl,(_dptrlo)
	ld	(hddest),hl
	ld	hl,(_dptrhi)
	ld	(hddest+2),hl
	ld	hl,512		; BYTE count
	ld	(count),hl
	ld	hl,1000000100000000B
		; EN (bit 15)=1
		; Source is Auto-increment memory (bits 14..12)=000b
		; No interrupt (bit 11)=0
		; BYTE size (bits 10..9)=00b
		; Continuous mode (bits 8..7)=10b
		; Flowthrough (bits 6..5)=00b
		; Transfer complete (bit 4)
		; Destination Auto-increment memory (bits 3..1)=000b
		; End-of-process Signalled
	call	hddmaio		; Start DMA

ENDC

COND XUTIL

	ld	hl,(_dptrlo)	; XUTIL reads into TPA
	ld	bc,cfdata
	INIRW

ENDC
	jr	done

write:	ld	a,030h		; CF  Write command
	out	(cfcmd),a

loop07:	in	a,(cfstat)	; Spin until not BUSY (bit 7 off) and
	and	088h		;  DRQ (bit 3 on)
	xor	008h
	jr	nz,loop07

COND XUTIL .eq. 0

	; Write the block to the cfdata I/O port using DMA3
	; must first copy it to the bounce buffer at an *even*
	; address

	ld	hl,(_dptrlo)
	ld	(hdsrc),hl
	ld	hl,(_dptrhi)
	ld	(hdsrc+2),hl
	ld	hl,idebuf	; make sure buffer address is even
	ld	a,l
	and	01H
	jr	z,even2
	inc	hl
even2:	ld	(hddest),hl	; dest is bounce buffer
	ld	l,h
	ld	h,01h		; System page
	ld	(hddest+2),hl
	ld	hl,512		; BYTE count
	ld	(count),hl
	ld	hl,1000000100000000B
		; EN (bit 15)=1
		; Source is Auto-increment memory (bits 14..12)=000b
		; No interrupt (bit 11)=0
		; Byte size (bits 10..9)=00b
		; Continuous mode (bits 8..7)=10b
		; Flowthrough (bits 6..5)=00b
		; Transfer complete (bit 4)
		; Destination Auto-increment memory (bits 3..1)=000b
		; End-of-process Signalled
	call	hddmaio		; Start DMA

	; Now ready to write it out using WORD addresses
	; to the IDE CompactFlash drive

	ld	hl,idebuf	; make sure buffer address is even
	ld	a,l
	and	01H
	jr	z,even3
	inc	hl
even3:	ld	(hdsrc),hl	; source is bounce buffer
	ld	l,h
	ld	h,01h		; System page
	ld	(hdsrc+2),hl
	ld	hl,cfdata	; Destination I/O port
	ld	(hddest),hl
	ld	l,h
	ld	h,cfp
	ld	(hddest+2),hl
	ld	hl,256		; WORD count
	ld	(count),hl
	ld	hl,1000001100001100B
		; EN (bit 15)=1
		; Source Auto-increment memory (bits 14..12)=000b
		; No interrupt (bit 11)=0
		; Word size (bits 10..9)=01b
		; Continuous mode (bits 8..7)=10b
		; Flowthrough (bits 6..5)=00b
		; Transfer complete (bit 4)
		; Destination I/O address unmodified (bits 3..1)=110b
		; End-of-process Signalled
	call	hddmaio		; Start DMA

ENDC

COND XUTIL

	ld	hl,(_dptrlo)	; XUTIL writes into TPA
	ld	bc,cfdata
	OTIRW

ENDC

loop08:	in	a,(cfstat)	; Spin until not BUSY (bit 7 off)
	and	080h
	jr	nz,loop08

	in	a,(cfstat)	; Error (bit 0 on)
	and	001h
	jp	nz,error

done:
COND	XUTIL
	call	_iopgfe		; Done, restore CP/M I/O page
ENDC

	ret


hddmaio:
	ld	(descr),hl	; Save descriptor

	call	_iopgff		; DMA I/O page

	ld	c,dal3
	ld	hl,hddest
	ld	b,6
loop00:	OUTIW
	inc	c
	inc	b
	dec	b
	jr	nz,loop00

	call	_iopg00
	ret

COND XUTIL

	psect	text

_wd_error:
	ret

_iopgff:
	push	bc
	push	hl
	ld	l,0FFh
	jr	selpg

_iopgfe:
	push	bc
	push	hl
	ld	l,uartp
	jr	selpg

_iopg00:
	push	bc
	push	hl
	ld	l,0

selpg:	ld	c,iop
	LDCTL
	nop
	nop
	nop
	nop
	pop	hl
	pop	bc
	ret

ENDC

; dummy routine

	global	_dma3_int

	psect	text

_dma3_int:
	inc	sp	;Discard reason code
	inc	sp
	RETIL
	nop

;------------------------------
; Variable Field for DMA3
;------------------------------

	psect	data

hddest:	defw	0,0	
hdsrc:	defw	0,0
count:	defw	256	; length: 256 words
descr:	defw	0	; descriptor

Counter:	defw	0	; retry counter
_rw_flag:	defw	0	; reserved for interrupt driven hard disk io

COND XUTIL .eq. 0

	psect	bss

	; Sector bounce buffer to overcome Odd address
	; restrictions for Word DMA transfers

idebuf:	defs	512+2		; We adjust this to use an
				; *EVEN* address for word transfers
ENDC

