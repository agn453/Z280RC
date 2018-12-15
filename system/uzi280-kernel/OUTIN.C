/* outin.c */
/*
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

*Include	z280rc.inc

*Include	macro.lib

	psect	text
	global	_out, _in

; out(val,port) 
; int port,val; 
_out:
	push	bc
     LDHLSX    6   ; get port into hl port
     ld   c,l
     LDHLSX    4    ; get val into hl
     ld   a,l
     out  (c),a
     nop	; z280 bug !!
     nop
     nop
     nop
     nop
	pop	bc	
     ret

; int in(port)  
; int port;     
_in:
	push	bc
     LDHLSX    4   ; get port into hl
     ld   c,l
     in   a,(c)
     ld   l,a
     xor  a
     ld   h,a
     ld   a,h
     or   l
	pop	bc
     ret
	nop
#endasm
