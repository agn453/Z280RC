
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


#asm

*Include	macro.lib
		
	global	_bzero

	psect	text
_bzero:
	LDHLSX	4 	; count
;	;cpw	127
;			defb	0fdh,0edh,0f7h
;			defw	127	
;	jr	nc,dma	; more than 127 bytes -> use dma0
	dec	hl
	ld	b,h
	ld	c,l
	LDHLSX	2	
	ld	d,h	
	ld	e,l	; ptr hl -> de 
	inc	de
	ld	(hl),0
	ldir
	ret
#endasm

/*AGN - DMA_bcopy is not defined and disabled (see commented CPW above) */

#ifdef DMA_bzero
#asm

	cond	z280rc
sysmhi	defl	001h
	else
sysmhi	defl	081h
	endc

	global	dodma
dma:
	ld	(length),hl	; save length
	LDHLSX	2
	ld	(dest),hl
	ld	l,h		; get bit 15-12 into l
	ld	h,sysmhi	; system memory high bits (23-16)
	ld	(dest+2),hl
	ld	hl,zero
	ld	(source),hl
	ld	l,h
	ld	h,sysmhi
	ld	(source+2),hl
	ld	A,0h	;use DMA0
	LD	HL,dest
	call	dodma
	ret

	psect	data

dest:	DEFW	0000
	DEFW	0000
source:	DEFW	0000	
	DEFW	0000	
length:	DEFW	0000	 	;length in 8-bit bytes
desc:	DEFW	0A100H		;continous, memory -> memory++, 8-bit
zero:	defb	0

	psect	text
#endasm
#endif
