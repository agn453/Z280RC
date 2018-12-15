
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

/*
	UZI280 devttyas.c vers 1.02a
*/

/* driver for cpu uart and bus-terminal with interrupt or pulling */

#include "config.h"

#define FAST_UART


#asm 
*Include	z280rc.inc

*Include	macro.lib


;
; Local bus iopage routines for uart transmitter interrupt
;
	psect	bss

temp_ut: defs	2

	psect	text
UTiopgfe:
	ld	c,iop
	RDCTL
	ld	(temp_ut),hl	;save iopage for case of uart receiver interrupt.
	ld	hl,uartp
	LDCTL    
	ret

UTiopgOLD:
	ld	c,iop
	ld	hl,(temp_ut)
	LDCTL			
	ret
;
;
 

; Z280 UART register addresses (in page FE)
CFREG	equ	ucr
TCREG	equ	tcs
RCREG	equ	rcs
RDREG	equ	rdr
TDREG	equ	tdr

#endasm

#ifdef POLLED_UART_TX

#asm 

	global	_iopg00,_iopgfe,_putuart,_tinproc
	global	_last_win,_cur_win,__putc

	psect	text

	global	_uart_T_int		

_putuart:
	push	bc
	call	_iopgfe
1:	in	a,(TCREG)
	and	01h 		;bit to test for tx ready
	jp	z,1b
	LDHLSX	4		;get arg
	LD	A,L
	out	(TDREG),a
	call	_iopg00
	pop	bc
	ret

; dummy interrupt routine

_uart_T_int:
	inc	sp		
	inc	sp		
	RETIL			

#endasm

#else
/*
	putuart(char c)

	Routine changed in Version 1.12.

	UART output is interrupt driven now. The output buffer holds
	up to "SIZE" output chars.
*/	 
#asm 

	global	_iopg00,_iopgfe,_putuart,_tinproc
	global	_last_win,_cur_win,__putc

	psect	text
		
	global	_uart_T_int		
	
SIZE	equ	400	; size of output queue
		
	psect	data				
out_buf:	defs	SIZE+1			
inp:		defw	out_buf			
outp:		defw	out_buf			
count:		defw	0			

	psect	text				
;		
; put something into the uart transmitter queue. The real output
; through the UART is done by the uart transmitter interrupt routine.  
;
_putuart:   ; putuart(char c)
	ld	hl,(count)
	;cpw	SIZE
				defb	0fdh,0edh,0f7h
				defw	SIZE
	jr	nc,_putuart	; wait if output buffer is full
	call	_iopgfe
	;incw	(count)	
				DEFB	0DDH
				inc	DE
				DEFW	count
	LDHLSX	2	; get argument char
	ld	a,l	; into a
	ld	hl,(inp) 
	ld	(hl),a	; put char into the queue
	inc	hl	
	ld	(inp),hl	; inp++			
	ex	de,hl		
	ld	hl,out_buf	
	;addw	hl,SIZE		
				DEFB	0FDH
				DEFB	0EDH,0F6h
				DEFW	SIZE                                    
	ex	de,hl		
	;cpw	hl,de
				DEFB	0edh,0d7h
	jp	c,ok_T1		
	;ldw	(inp),out_buf	
				DEFB	0DDh,11h
				DEFW	inp
				DEFW	out_buf   
ok_T1:			; make sure transmitter interrupts are enabled
	in	a,(TCREG) 
	or	0c0h	; set transmitter enable, interrupts on
	out	(TCREG),a
	call	_iopg00	
	ret		

;
; Uart transmitter ready interrupt routine.
;
;  Put out any pending characters that are still in the output queue.
;
_uart_T_int:					
	push	af	
	push	bc	
	push	de	
	push	hl	
	call	UTiopgfe
	in	a,(TCREG) 
	and	01h	
	jp	z,ok_TI	
	ld	hl,(count)	; interrupt but nothing in the 
	;cpw	HL,0		; output queue ?
				DEFB	0FDH		; -> disable transmitter interrupts.
				DEFB	0EDH,0F7h
				DEFW	0                                       
	jp	z,T_int_off 	
	;decw	(count)		; count--
				DEFB	0DDH
				dec	DE
				DEFW	count
	ld	hl,(outp)	
	ld	a,(hl)		
	out	(TDREG),a	
	inc	hl		
	ld	(outp),hl	
	ex	de,hl		
	ld	hl,out_buf	
	;addw	hl,SIZE		
				DEFB	0FDH
				DEFB	0EDH,0F6h
				DEFW	SIZE                                    
	ex	de,hl		
	;cpw	hl,de
				DEFB	0edh,0d7h
	jp	c,ok_TI		
	;ldw	(outp),out_buf		
				DEFB	0DDh,11h
				DEFW	outp
				DEFW	out_buf                                 
	jp	ok_TI		
T_int_off:					
	in	a,(TCREG)	
	and	0bfh		; disable transmitter interrputs	
	out	(TCREG),a	
ok_TI:	call	UTiopgOLD
	pop	hl		
	pop	de		
	pop	bc		
	pop	af		
	inc	sp		
	inc	sp		
	RETIL			
#endasm

#endif

/* interrupt routine for the Z280 UART receiver */

/*
   Fast interrupt service routine for the internal Z280-uart receiver.
   Works with a baud rate of up to 38400 baud. 
   Striped down version without control key management.
   Just puts the input char into the char input queue and wakes up	
   any process waiting for the input event.	   	
*/
#asm
	psect	bss

ur_page: defs   2

	global	_uart_int,_tinproc,_iopg00,_iopgfe,_IRET
	global	_ttyinq, _insq, _ttydata, _echo, _wakeup, _fullq
	psect	text

_uart_int:
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY

	;get char from uart here
	ld	c,iop
	RDCTL
	ld	(ur_page),hl
	ld	hl,uartp
	LDCTL    		; io-page 0feh

uart_loop:
	in	a,(RCREG)
	and	10H 		;bit to test for rcv ready
	jp	z,nochar	; no char available
	in	a,(RDREG)
	ld	l,a
	ld	h,0
	push	hl

	global	_fast_mode
	ld	a,(_fast_mode)	; check uart interrupt mode
	or	a
	jr	nz,1f		; fast mode 
	ld	hl,5	
	push	hl
	call	_tinproc	; tinproc(5, c);
	pop	hl
	pop	hl
	jp	uart_loop

; Fast Mode: insert char into queue and wakeup any process waiting on
; the input event. No ctrl char processing.
1:
	pop	hl
	ld	a,l	; the char

	ld	iy,_ttyinq+60

;	push	hl
;	call	_insq			; insq(&ttyinq[5],char)
;	pop	de
;	pop	de

; insq()  
	;ldw  hl,(iy+4)
				defb	0fdh,0edh,26h,+4
	ld	(hl),a
	ex	de,hl	; save the pointer
	;incw  (iy+8)
				defb	0fdh,013h
				defw	8
	;ldw  hl,(iy+6)
				defb	0fdh,0edh,26h,+6
	;addw	hl,(iy+0)
				defb	0fdh,0edh,0d6h
				defw	0
	ex	de,hl	; get back the pointer into hl
	inc	hl
	;ldw  (iy+4),hl
				defb	0fdh,0edh,2eh,+4
	;cpw	hl,de
				defb	0edh,0d7h
	jp	c,l20
	;->ldw  hl,(iy+0)
				defb	0fdh,0edh,26h,+0
	;->ldw  (iy+4),hl
				defb	0fdh,0edh,2eh,+4
l20:
; insq() end.
;
	ld	hl,_ttyinq+60		; event 

; wakeup() is called in the timer interrupt routine to
; speed up uart receiver interrupt handling.

	push	hl
	call	_wakeup			; wake up any process waiting for the 
	pop	hl			; input event
	jp	uart_loop

nochar:
	ld	c,iop
	ld	hl,(ur_page)	; get old io page
	LDCTL
	
	pop	iy
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
	inc	sp
	inc	sp
	RETIL
#endasm


#ifdef no_tty_interrupt
/* 
  no terminal interrupt only dummy routine for interrupt handler
  and pulling routine for char input.
*/
#asm
	psect	text
	global	_acia_int,_IRET
_acia_int:
	ex	af,af'
	push	af
	ex	af,af'
	exx
	push	bc
	push	de
	push	hl
	exx
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	jp	_IRET
#endasm

/* 
	terminal input pulling routines 
*/
#ifdef no_bus_tty  /* only dummy routine */
#asm
	psect	text
	global	_pull_tty
_pull_tty:
	ret
#endasm
#endif

#ifdef REH_HGT
#asm
	psect	text
	global	_pull_tty

_pull_tty:
	in	a,(61h)	; HGT status port
	bit	0,a	; char available ?
	ret	z	; no
	in	a,(60h)	; HGT data port, get char

	ld	l,a
	ld	h,0
	push	hl
	ld	hl,1	; tty minor number
	push	hl
	call	_tinproc	; tinproc(1,HGT_data)
	pop	hl
	pop	hl 
	ret

	psect	data
acminor:	defw	0
	psect	text
#endasm
#endif

#ifdef CLUB80_Terminal

#asm
	psect	text
	global	_pull_tty

_pull_tty:
	in	a,(080h)	; status port
	rla
	ret	c
;	acminor = in(0x81);
	in	a,(081h)	; data port
	ld	l,a
	ld	h,0
	ld	(acminor),hl
;	if (acminor>199 && acminor<205) 
	cp	200
	jp	c,normal
	cp	205
	jp	nc,normal
;	/* char 200-204 changes window */
;	{
;	    cur_win = acminor - 199;	/* cur_win 1-5 */
		ld	de,199
		or	a
		sbc	hl,de
		ld	(_cur_win),hl	
		ex	de,hl	; argument for _putc
		ld	a,e	; needed for compare
;	    last_win = -1;
		ld	hl,-1
		ld	(_last_win),hl
;	    if (cur_win != 5) { /* never open window on uart device */
		cp	5
		jp	z,1f
;	    	_putc(cur_win,0x1b); 
		ld	hl,01bh
		push	hl
		push	de
		call	__putc
;	    	_putc(cur_win,'x');
		pop	de
		pop	hl
		ld	hl,'x'
		push	hl
		push	de	
		call	__putc
;	    	_putc(cur_win,cur_win+31);
		pop	de
		pop	hl
		ld	hl,(_cur_win)
		ld	de,31
		add	hl,de
		push	hl
		push	de	
		call	__putc
		pop	de
		pop	de
;	    }
1:
;		;
	ret
;		;
;	} 
;	/* normal character */
normal:
;	tinproc(cur_win, acminor);
	ld	hl,(acminor)
	push	hl
	ld	hl,(_cur_win)
	push	hl
	call	_tinproc
	pop	de
	pop	de
	ret

	psect	data
acminor:	defw	0
	psect 	text
#endasm

#endif

#else /* no_tty_interrupt */
/* 
	interrupt routine for terminal input char I/O 
*/
/*
	Routine for Club80 Terminal.
*/
#asm
	psect	text
	global	_acia_int,_IRET
_acia_int:
	ex	af,af'
	push	af
	ex	af,af'
	exx
	push	bc
	push	de
	push	hl
	exx
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY

	in	a,(080h)	; status port
	rla
	jp	c,_IRET
;	acminor = in(0x81);
	in	a,(081h)	; data port
	ld	l,a
	ld	h,0
	ld	(acminor),hl
;	if (acminor>199 && acminor<205) 
	cp	200
	jp	c,normal
	cp	205
	jp	nc,normal
;	/* char 200-204 changes window */
;	{
;	    cur_win = acminor - 199;	/* cur_win 1-5 */
		ld	de,199
		or	a
		sbc	hl,de
		ld	(_cur_win),hl	
		ex	de,hl	; argument for _putc
		ld	a,e	; needed for compare
;	    last_win = -1;
		ld	hl,-1
		ld	(_last_win),hl
;	    if (cur_win != 5) { /* never open window on uart device */
		cp	5
		jp	z,1f
;	    	_putc(cur_win,0x1b); 
		ld	hl,01bh
		push	hl
		push	de
		call	__putc
;	    	_putc(cur_win,'x');
		pop	de
		pop	hl
		ld	hl,'x'
		push	hl
		push	de	
		call	__putc
;	    	_putc(cur_win,cur_win+31);
		pop	de
		pop	hl
		ld	hl,(_cur_win)
		ld	de,31
		add	hl,de
		push	hl
		push	de	
		call	__putc
		pop	de
		pop	de
;	    }
1:
;		;
	jp	_IRET
;		;
;	} 
;	/* normal character */
normal:
;	tinproc(cur_win, acminor);
	ld	hl,(acminor)
	push	hl
	ld	hl,(_cur_win)
	push	hl
	call	_tinproc
	pop	de
	pop	de
	jp	_IRET

	psect	data
acminor:	defw	0
	psect 	text
#endasm

#endif /*no_tty_interrupt*/
