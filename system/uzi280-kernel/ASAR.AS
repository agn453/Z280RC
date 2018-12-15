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

;	Shift operations - the count is always in B,
;	the quantity to be shifted is in HL, except for the assignment
;	type operations, when it is in the memory location pointed to by
;	HL

;	ATTENTION: this module will ONLY work with a Z280 processor !

	global	asar	;assign shift arithmetic right
	global	shar
	psect	text

*include	macs.lib

	nop		; this makes alt following code aligned 

asar:
	ld	d,h		;save the pointer
	ld	e,l
	ldhlhl			;ld	hl,(hl)
	call	shar		;shift
	ex	de,hl		;restore the pointer in hl
	ldhlde			;ld	(hl),de
	ex	de,hl		;return value in hl
	ret

