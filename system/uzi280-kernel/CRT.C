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

#asm
; startup file for UZI280

*Include	z280rc.inc

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text
	defs	100h		;Base of UZI280 program and data space

	global	start,_main,_exit,__brk,__Hbss,__Lbss,_environ

; on return from the uzi kernel the stack looks like the following:
;
;	stack+4 ->	start of environ
;	stack+2 ->	argc
;sp->	stack	->	pointer	to argv
;
start:  jp	1f		; jp to indicate uzi binary type 1

1:	ld	hl,__Hbss
	push	hl
	call	__brk		; do system call to precisely set break 
	pop	hl
; zero out bss segment 
	or	a
	ld	de,__Lbss
	sbc	hl,de	; calc length
	ld	b,h
	ld	c,l
	dec	bc
	ld	hl,__Lbss
	ld	(hl),0	; zero out __Lbss
	inc	de
	ldir		; do it
 
	ld	hl,4
	add	hl,sp		; pointer to first element of enviroment
	ld	(_environ),hl	; save the pointer
	pop	hl
	pop	de
	push	hl
	push	de		; shuffle argv pointer and argc 
	call	_main		; main(argc,argv)
	push	hl
02:	call	_exit		; give return val of main to exit
; should never be reached
	jr	02b

#endasm
