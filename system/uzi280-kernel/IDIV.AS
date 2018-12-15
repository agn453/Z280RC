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

;	16 bit divide and modulus routines
;
;	called with dividend in hl and divisor in de
;
;	returns with result in hl.
;
;	adiv (amod) is signed divide (modulus), ldiv (lmod) is unsigned
;
;	ATTENTION: this module will ONLY work with a Z280 processor !

	global	adiv,ldiv,amod,lmod
	psect	text
*include	macs.lib

amod:
;	call	adiv
	ld	b,d
	ld	c,e
       ;exts     hl
        defb   0edh,06ch
       ;divw     dehl,bc
        defb   0edh,0cah
	ex	de,hl		;put modulus in hl
	ret

lmod:
;	call	ldiv
	ld	b,d
	ld	c,e
	ld	de,0
	divuwdehlbc		;DIVUW DEHL,BC
	ex	de,hl
	ret
	nop

ldiv:
	ld	a,d
	or	e
	ret	z
	ld	b,d
	ld	c,e
	ld	de,0
	divuwdehlbc		;DIVUW DEHL,BC
	ret
	nop

adiv:
	ld	a,d
	or	e
	ret	z
	ld	b,d
	ld	c,e
       ;exts     hl
        defb   0edh,06ch
       ;divw     dehl,bc
        defb   0edh,0cah
	ret



