; this is not yet working - use the idecf.as source instead
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

; 930414 Tilmann Reh
; 930723 Stefan Nitschke	Version for UZI280
; 20181211 Tony Nicholson	Version for Z280RC IDE CompactFlash
;
; Interrupt driven routine
;

*Include	config.inc	; System configuration

*Include	z280rc.inc	; Z280RC I/O port definitions

*Include	macro.lib	; Z280 instruction macros for HiTech ZAS

Stand_Alone	equ	0	; not tested yet
CPM_Boot	equ	1	; means UZI is booted from CPM

	global	_WD_Cyls,_WD_Heads,_WD_Sector ; defined in ideconf.c

	psect	text

	global	_wdsec,_wdtrk,_dptrlo,_wd_rdwr,_wd_io,_hdinit
	global	_dptrhi,_iopg00,_iopgff,_dma3_int

	global	_wd_error,_rw_flag
		
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
	call	_iopg00		; Select CF I/O page

wait:	ld	a,(_rw_flag)	; Wait for previous operation to complete
	or	a
	jr	nz,wait
	ld	a,1		; Set busy
	ld	(_rw_flag),a

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

error:	call	_wd_error
	ret

contrd:	ld	hl,cfdata	; Low 12-bit I/O address
	ld	(hdsrc),hl
	ld	hl,cfp*256	; High 12-bit I/O page
	ld	(hdsrc+2),hl
	ld	hl,(_dptrlo)
	ld	(hddest),hl
	ld	hl,(_dptrhi)
	ld	(hddest+2),hl
	ld	hl,1110100000000000B
		; EN (bit 15)=1
		; Source is I/O address unmodified (bits 14..12)=110b
		; Interrupt on completion (bit 11)=1
		; Byte size (bits 10..9)=00b
		; Single mode (bits 8..7)=00b
		; Flowthrough (bits 6..5)=00b
		; Transfer complete (bit 4)
		; Destination Auto-increment memory (bits 3..1)=000b
		; End-of-process Signalled
	call	hddmaio		; Start DMA

	; Read in-progress.  DMA will interrupt on completion

	jr	done

write:	ld	a,030h		; CF Write command
	out	(cfcmd),a

loop07:	in	a,(cfstat)	; Spin until not BUSY (bit 7 off) and
	and	088h		;  DRQ (bit 3 on)
	xor	008h
	jr	nz,loop07

	ld	hl,(_dptrlo)	; Copy block to local buffer
	ld	(hdsrc),hl
	ld	hl,(_dptrhi)
	ld	(hdsrc+2),hl
	ld	hl,cfdata
	ld	(hddest),hl
	ld	hl,cfp*256
	ld	(hddest+2),hl
	ld	hl,1000100000001100B
		; EN (bit 15)=1
		; Source Auto-increment memory (bits 14..12)=000b
		; Interrupt on completion (bit 11)=1
		; Byte size (bits 10..9)=00b
		; Single mode (bits 8..7)=00b
		; Flowthrough (bits 6..5)=00b
		; Transfer complete (bit 4)
		; Destination I/O address unmodified (bits 3..1)=110b
		; End-of-process Signalled
	call	hddmaio		; Start DMA

	; Write in progress.  DMA will interrupt on completion.

;loop08:	in	a,(cfstat)	; Spin until not BUSY (bit 7 off)
;	and	080h
;	jr	nz,loop08
;
;	in	a,(cfstat)	; Error (bit 0 on)
;	and	001h
;	jr	nz,error

done:	call	_iopg00		; Done, restore I/O page

	xor	a
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

; Interrupt handler for DMA3

	global	_dma3_int

_dma3_int:
	ex	(sp),hl		; Get reason code, save HL
	push	bc
	push	af
;	cpw	hl,itvtoff$dma3	; Is reason code = 30h?
				defb	0fdh,0edh,0f7h,030h,000h
	jr	nz,dma3done	; No

	ld	c,iop
	RDCTL			; Push old I/O page
	push	hl
	ld	l,dmap		; Select DMA I/O Page
	LDCTL

	ld	c,tdr3		; Point to DMA3 descriptor register
	INW			; Fetch it and reset interrupt bit
	ld	a,h		;  and enable bit
	and	01110111B
	ld	h,a
	OUTW

	ld	c,iop		; Point to CF I/O page
	ld	l,cfp
	LDCTL

	in	a,(cfstat)	; Was there an error? (bit 0 on)
	and	001h

	push	de		; Save registers in case we
	push	ix		;  need to print warning
	push	iy
	call	nz,_wd_error	; Yes, print warning
	pop	iy
	pop	ix
	pop	de

	xor	a
	ld	(_rw_flag),a	; Reset busy flag

	pop	hl		; Restore I/O page
	ld	c,iop
	LDCTL	

dma3done:
	pop	af
	pop	bc
	pop	hl
	RETIL

;------------------------------
; Variable Field for DMA3
;------------------------------ 

	psect	data

hddest:	defw	0,0	
hdsrc:	defw	0,0
	defw	512	; length: 512 bytes
descr:	defw	0	; descriptor

Counter:	defw	0	; retry counter
_rw_flag:	defw	0	; reserved for interrupt driven hard disk io 

