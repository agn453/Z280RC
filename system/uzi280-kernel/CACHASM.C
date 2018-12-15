
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

#include	"config.h"

#asm 

*Include	z280rc.inc

 	psect	text

 	global	_dma_write, _dma_read	
 	global	_lowadr,_highadr,_dptrlo,_dptrhi
 	global	_iopgff,_iopg00,_rw_flag
  
 	psect	data
_highadr:	defw	0
_lowadr:	defw	0
	psect	text

_dma_write:
	push	bc
 	ld	hl,(_lowadr)
 	ld	(hddest),hl	
 	ld	hl,(_highadr)
 	ld	(hddest+2),hl		; Destination Adr
        ld      hl,(_dptrlo)
 	ld	(hdsource),hl
        ld      hl,(_dptrhi)
 	ld	(hdsource+2),hl		; Source Adr

01:	ld	a,(_rw_flag)
	or	a
	jr	nz,01b			; wait for last hard disc write ready
 	jp	dma0		        ; Start DMA Transfer
 
_dma_read:
	push	bc
        ld      hl,(_dptrlo)
 	ld	(hddest),hl
        ld      hl,(_dptrhi)
 	ld	(hddest+2),hl		; Destination Adr
 	ld	hl,(_lowadr)
 	ld	(hdsource),hl	
 	ld	hl,(_highadr)
 	ld	(hdsource+2),hl		; Source Adr

 ;	jp	dma0		        ; Start DMA Transfer
 
;----------------------------------
; Setup DMA0 for mem copy
;----------------------------------
dma0:			
;DMA0 init --> transfer
        call    _iopgff
 	ld	c,dal0		; Adresse DMA-Register (DMA0)
 	ld      hl,hddest
        ld      b,6		
 	;outiw			
1:	defb	0edh,83h
 	inc	c		
        inc     b
        dec     b
        jr      nz,1b
        call    _iopg00
	pop	bc
 	ret			
 
;------------------------------
; Variablen Feld f}r die DMA0
;------------------------------ 
 	psect	data
 
hddest:		defw	0
 		defw	0	
hdsource:	defw	0
 		defw	0	
dmacnt:		defw	512	; length: 512 Byte 
hddescr:	defw	8100h	;continuous, memory++, 8 bit
		;defw	8000h	;single, memory++, 8 bit
	psect	text
#endasm 
