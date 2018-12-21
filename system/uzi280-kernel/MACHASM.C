/* Z280RC Modifications by Tony Nicholson 12-Dec-2018  */
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

/* UZI280 machasm.c  vers. 1.02a

   with support of non interrupt driven bus terminals like the 
   REH-HGT terminal  ( changed: clk_int)  SN 
*/
 
#define MACHDEP
#undef	MACHDEP2

#include	"unix.h"
#include	"extern.h"

#define NEWUPUT		/* define this for fast DMA routines for
			   user<->system data copy */

/* This is called at the very beginning to initialize everything. */
/* It is the equivalent of (startup) */
#asm 
*Include	z280rc.inc

*Include	macro.lib

	psect	text
	global	_fs_init, _main
	global	_c_mode,_upage,_ufault
	global	_trap2,_panic,_kprintf,_usegv2
	global	_chksigs,_ub

_fs_init:
	DI
	LD	SP, _ub + 512
#endasm

#ifdef Z280RC

#asm
	; When we were loaded by BOOTUZI, only the first fourteen
	; System-mode PDRs were set-up.  Now do the remaining two
	; now we're not executing code in the physical 01E000..01FFFF
	; address space.

	call	_iopgff		; I/O page to MMUP
	ld	a,1Eh		; Select Sys-PDR 14
	out	(pdr),a		;  in the PDR pointer

	ld	c,bmp		; point C to the MMU BMP
	ld	hl,01EAh	; Sys-PDR 14 to 01E000
	OUTW
	ld	l,0FAh		; Sys-PDR 15 to 01F000
	OUTW
	call	_iopg00		; Back to default I/O page (and interrupts)
#endasm

#endif

#asm
				; init psect bss
	global	__Hbss, __Lbss
	global	_initvec,_initmmu,_main,_gettrap

	ld	de,__Lbss
;	ld	hl,__Hbss
	ld	hl,0ffffh
	SUBDE
	ld	c,l
	ld	b,h
	dec	bc
	ld	l,e
	ld	h,d
	inc	de
	ld	(hl),0
	ldir			; clear bss memory
	jp	_main
	jp	_fs_init
#endasm

#asm
	psect	text
	global	_tempstack
_tempstack:
	pop	hl
	ld	sp, tstk+100
	jp      (hl)
	nop
	psect	data
  
tstk:	defs 100	;temporary system stack to use during context switches

#endasm


#asm
	psect	text
	global	_initvec

_initvec:
	push	bc
	LD	HL, 00100H	;Trap table address 0010000H physical
				;System logical address 0000H
	ld	C,itv
	LDCTL

	ld	c,isr		;interrupt status register
	ld	hl,00
	LDCTL			; no interrupt vector

	IM3

	ld	hl, _ub + 200
	ld	a,l
	and	0f0h
	ld	l,a	;Calculate lower stack limit AND FFF0
	ld	c,slr
	LDCTL		; Load stack limit register

	ld	hl,0001h
	ld	c,tcr
	LDCTL		; Turn on stack warning trap

; enable all interrupts
	ld	c,msr
	RDCTL
	ld	l,07fh
	LDCTL
	call	_iopg00

#endasm

#ifdef Z280RC

#asm

; Time constant for 128Hz is 3686400 / 128 = 28800

timer$constant equ 28800

	call	_iopgfe		; Counter/Timer I/O page

	ld	a,10000000B
		; Continuous (bit 7)=1
		; Non-retriggerable (bit 6)=0
		; Interrupt disabled (bit 5)=0
		; Not cascaded (bit 4)=0
		; Counter output pin disabled (bit 3)=0
		; Timer mode (bit 2)=0
		; No gate output (bit 1)=0
		; No external trigger (bit 0)=0
	out	(cr0),a
	ld	c,tc0
	ld	hl,timer$constant
	OUTW
	ld	a,11100000B
		; Enabled (bit 7)=1
		; Software gate ON (bit 6)=1
		; Trigger gate ON (bit 5)=1
		; Other bits are ignored (bits 4..0)=00000
	out	(cs0),a

	call	_iopg00

#endasm

#else

#asm
	; Initialisieren der RTC fuer periodischen Interrupt   SN
	call    _iopg40
;	ld      a,00101011B ; 32 interrupts per secound from RTC
;	ld	a,00101010B	; 64   "
	ld	a,00101001B	; 128   "
	out     (10),a
	call    _iopg00

#endasm

#endif

#asm

	call	_iopgfe
	ld	a,0c0h
	out	(rcs),a	; Turn on UART receiver interrupt
	call	_iopg00

	; Set up DMA0 for software ready (for pg_copy)
	call	_iopgff
	ld	c,dmcr
	ld	hl,0020h
	OUTW
	call	_iopg00

;	; set external io waits to 1
;	ld	c,btc
;	ld	hl,01
;	LDCTL 

	pop	bc
	ret
#endasm

#asm
	psect	text
	global	_initmmu
_initmmu:
	push	bc
; Turn on the cache
	PCACHE
	ld	hl,08h	;Turn on cache for code and data with burst

	ld	c,ccr
	LDCTL
; This loads the system portion of the MMU 
; Set the I/O page register to ffh
	call	_iopgff
	ld	hl, 0C800H  ;Set MMU for user and system translation
			   ; Program/Data separation for user mode enable	
	ld	c,mmcr
	OUTW
; Load the MMU PDR pointer (at I/O address FFxxF1h) to 10h
	ld	a,10h
	out	(pdr),a
	ld	hl,ptab	;LOAD HL WITH ADDRESS OF PAGE TABLE 
; Do block output to port FFxxF4h to fill descriptors
	ld	c,bmp
	ld	b,16
	OTIRW
; invalidate the user portion of the MMU
	ld	a,0ch
	out	(ip),a
; Set the I/O page register back to 00h
	CALL	_iopg00
	pop	bc
	RET

; this table holds physical page address off the system mode
; memory pages.
	psect	data
ptab:
;The system part of the MMU:
	DEFW	0010ah		
	DEFW	0011ah
	DEFW	0012ah
	DEFW	0013ah
	DEFW	0014ah
	DEFW	0015ah
	DEFW	0016ah
	DEFW	0017ah
	DEFW	0018ah
	DEFW	0019ah
	DEFW	001aah
	DEFW	001bah
	DEFW	001cah
	DEFW	001dah
	DEFW	001eah
	DEFW	001fah
#endasm

#asm 

; io-page setting routines. The UART receiver interrupts are
; NOT disabled. 

	psect	data

iopei:	defw	00	;storage for interrupt enable for iopg routines

	psect	bss
iopage: defs	2	;io page before last iopgXX call

	psect	text

	global	_iopg00,_iopg40,_iopgfe,_iopgff
	global	_upush,_uput,_ugets

_iopg40:
	push	bc
	ld	l,i$rx	;disable interrupts, and store previous status
	push	hl
	call	_spl
	pop	de
	ld	(iopei),hl
	ld	c,iop
	RDCTL
	ld	(iopage),hl
	ld	l,040h
	LDCTL    
	pop	bc
	ret

_iopgfe:
	push	bc
	ld	l,i$rx		;disable interrupts, and store previous status
	push	hl
	call	_spl
	pop	de
	ld	(iopei),hl
	ld	c,iop
	RDCTL
	ld	(iopage),hl
	ld	l,0feh		;ctp, uartp
	LDCTL    
	pop	bc
	ret

_iopgff:
	push	bc
	ld	l,i$none	;disable interrupts, and store previous status
	push	hl
	call	_spl
	pop	de
	ld	(iopei),hl
	ld	c,iop
	RDCTL
	ld	(iopage),hl
	ld	l,0ffh		; mmup, rrrp, dmap
	LDCTL
	pop	bc    
	RET

_iopg00:
	push	bc
	ld	hl,(iopage)
	ld	c,iop
	LDCTL

	ld	hl,(iopei)	;restore interrupts to saved status
	push	hl
	call	_spl
	pop	de
	pop	bc
	ret

; This is the common interrupt-scall-trap return routine.
; It restores the registers, and will set up a return to a
; signal handler if necessary.
	
	global	_IRET
_IRET:
	LDHLSX	14+8	;Grab MSR we are returning to
	ld	a,h
	and	40h	;Mask off U/S bit
	jp	nz,USRRET  ;Handle case of returning
			; into user context different
SYSRET:
	pop	iy
	pop	ix
	pop	hl
	pop	de
	pop	bc
	pop	af
	exx
	pop	hl
	pop	de
	pop	bc
	exx	
	ex	af,af'
	pop	af
	ex	af,af'
	inc	sp
	inc	sp	;Effectively push off reason code
	RETIL

; For returning into user context, we must check to see if we want
; to call a signal catcher.

USRRET:
	call	_gettrap
	ld	a,h
	or	l
	jp	z,SYSRET	;If no signal handler, proceed normally
	ex	de,hl		;get handler address in DE
	LDHLSX	16+8		;Get user return address
	ex	de,hl		;orig user addr in DE, handler addr in HL
	STHLSX	16+8		;put handler address in stack over old user
	ex	de,hl		;get orig user address back in HL
	push	hl
	call	_upush		;shove on user stack
	pop	hl
	call	_upush		;shove IY on user stack
	pop	hl
	call	_upush		;IX
	pop	hl
	call	_upush		;HL
	pop	hl
	call	_upush		;DE
	pop	hl
	call	_upush		;BC
	pop	hl
	call	_upush		;AF

	pop	hl		
	call	_upush		; hl'
	pop	hl		
	call	_upush		; de'
	pop	hl
	call	_upush		; bc'	
	pop	hl
	call	_upush		; af'

	pop	hl
	ld	hl,(_ub + ?OSIG) ;get udata.u_cursig
	push	hl
	call	_upush		;shove on user stack as arg to handler
	pop	hl
	ld	hl,0000
	ld	(_ub + ?OSIG),hl  ;zero out udata.u_cursig
;	LXI	H, 0FFF6H	;get address of special handler return routine
	ld	hl, 0040H	;address changed. SN 
	push	hl
	call	_upush
	pop	hl
	inc	sp
	inc	sp		;pull reason code off stack
	RETIL			;return back into user context to signal
				;handler routine that returns to special
				;user code at 040h (that restores registers)
				;that returns to original return address.
#endasm

/* real time clock interrupt handling routine */

#ifdef no_tty_interrupt

#asm
;   
	psect	text
	global	_clk_int
_clk_int:
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
	LDHLSX	14+8	;Grab MSR we are coming from
	LD	A,H
	AND	40H	;Mask off U/S bit
	LD	H,A
	LD	L,0
	PUSH	HL	; save user/sys arg for clkint2

	global	_pull_tty	; terminal input polling 
	global	_inint

	ld	a,(_inint)
	or	a
	call	z,_pull_tty	; no args

	global	_clkint2

	CALL	_clkint2	;Call clkint2() with User/Sys arg
	POP	DE
	JP	_IRET
#endasm

#else	/* not no_tty_interrupt */

/* terminal has its own interrupt routine -> no polling */ 
#asm
;   
	psect	text
	global	_clk_int
_clk_int:
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
	LDHLSX	14+8	;Grab MSR we are coming from
	LD	A,H
	AND	40H	;Mask off U/S bit
	LD	H,A
	LD	L,0
	PUSH	HL	; save user/sys arg

	global	_clkint2

	CALL	_clkint2	;Call clkint2() with User/Sys arg
	POP	DE
	JP	_IRET
#endasm

#endif /* no_tty_interrupt */

#ifdef Z280RC

/* turn on the Counter/Timer 0 interrupts (128 Hz) AGN */

#asm
	psect	text
	global	_RTclock
_RTclock:
	push	bc

	call	_iopgfe		; Counter/Timer I/O page

	ld	a,10100000B
		; Continuous (bit 7)=1
		; Non-retriggerable (bit 6)=0
		; Interrupt enable (bit 5)=1
		; Not cascaded (bit 4)=0
		; Counter output pin disabled (bit 3)=0
		; Timer mode (bit 2)=0
		; No gate output (bit 1)=0
		; No external trigger (bit 0)=0
	out	(cr0),a
	ld	c,tc0
	ld	hl,timer$constant
	OUTW
	ld	a,11100000B
		; Enabled (bit 7)=1
		; Software gate ON (bit 6)=1
		; Trigger gate ON (bit 5)=1
		; Other bits are ignored (bits 4..0)=00000
	out	(cs0),a

	call	_iopg00

	pop	bc
	ret

#endasm

/* clear C/T 0 interrupt and retrigger next interrupt */

#asm
	psect	text
	global	_RDclock
_RDclock:
	push	bc

	call	_iopgfe

	ld	a,11100000B
	out	(cs0),a ; Interrupt acknowledge
			;  and retrigger next interrupt

	call	_iopg00

	pop	bc
	ret
#endasm

/* rdtod()  Read timekeeper time-of-day */

#asm
	global	_rdtod
	global	_tod
	global	shll
	global	_spl

	psect	text
_rdtod:
	push	bc
	call	getclk
	ld	b,11
	ld	a,(_hour)
	ld	l,a
	ld	h,0
	call	shll
	ex	de,hl
	push	de
	ld	b,5
	ld	a,(_min)
	ld	l,a
	ld	h,0
	call	shll
	ex	de,hl
	ld	a,(_sec)
	ld	l,a
	ld	h,0
	srl	h
	rr	l
	ld	a,l
	or	e
	ld	l,a
	ld	a,h
	or	d
	ld	h,a
	pop	de
	ld	a,l
	or	e
	ld	l,a
	ld	a,h
	or	d
	ld	h,a
	ld	(_time),hl
	ld	b,9
	ld	a,(_yy)
	ld	e,a
	ld	d,0
	ld	hl,100		; Adjust for year >2000
	add	hl,de
	call	shll
	ex	de,hl
	push	de
	ld	b,5
	ld	a,(_mm)
	ld	l,a
	ld	h,0
	call	shll
	ex	de,hl
	ld	a,(_dd)
	dec	a		; (day of month seems to be one off)
	ld	h,0
	or	e
	ld	l,a
	ld	a,h
	or	d
	ld	h,a
	pop	de
	ld	a,l
	or	e
	ld	l,a
	ld	a,h
	or	d
	ld	h,a
	ld	(_date),hl
	pop	bc
	ret	

	psect	bss
timeblk:		; Time values from DS1302 timekeeper chip
			; (the order must match that in watrdc)
_hour:	defs	1
_min:	defs	1
_sec:	defs	1
_dd:	defs	1
_mm:	defs	1
_yy:	defs	1

;todei:	defs	2	; Store for saved interrupt enable

_time	equ	_tod	; Time of Day pointers
_date	equ	_tod+2

	psect	text

getclk:

	; watchp is at the default I/O page 0

;	ld	l,i$none	;disable interrupts, and store previous status
;	push	hl
;	call	_spl
;	pop	de
;	ld	(todei),hl

	ld	hl,watrdc	; Point to Read Command bytes
	ld	de,timeblk	; Time values from clock go here
gclklp:	ld	a,(hl)
	or	a
	jr	z,gclkdn	; Zero byte means done reading clock
	push	de		; Save pointer to clock value
	ld	d,a		; command byte to D
	inc	hl
	ld	a,00000010B	; enable DS1302 (nRST 1 SCLK 0)
	out	(watch),a
	ld	b,8		; counter to write 8 bits
gclko:	ld	a,00000100B	; (nRST 1 SCLK 0) shifted left
	srl	d		; move LSB into carry
	rra			;  then into MSB (data bit)
	out	(watch),a	; output bit (nRST 1 SCLK 0, data in bit 7)
	or	00000001B	; raise SCLK to 1 to latch data output bit
	out	(watch),a	; output bit (nRST 1 SCLK 1, data in bit 7)
	nop	;outjmp
	nop
	nop
	nop
	djnz	gclko		; repeat for each bit of command byte
	push	hl
	push	bc
	ld	d,0		; initialise result
	ld	c,watch
	ld	b,8		; counter to read 8 bits
gclki:	ld	a,10000010B
	out	(watch),a	; output (nRST 1 SCLK 0) to latch input bit
;	inw	hl,(c)		; read bit into L (bit 7)
	INW
	ld	a,10000011B
	out	(watch),a	; output (nRST 1 SCLK 1)
	rl	l		; shift data bit into CY
	rr	d		; then into result
	djnz	gclki		; repeat for each bit of read data
	ld	a,d		; Result to A
	pop	bc
	pop	hl
	pop	de		; Restore time value pointer
	ld	(de),a		; Save the result byte
	inc	de		; Point to next value to be read
	ld	a,00000010B
	out	(watch),a	; output (nRST 1 SCLK 0)
	nop			; delay about a microsecond
	nop
	nop
	ld	a,00000000B
	out	(watch),a	; output (nRST 0 SCLK 0)
	nop			; delay about a microsecond
	nop
	nop
	jr	gclklp		; get next time value

gclkdn:
;	ld	hl,(todei)	;restore interrupts to saved status
;	push	hl
;	call	_spl
;	pop	de

	ld	hl,timeblk	; convert bcd values to binary
	ld	b,6
cvtval:	call	bcdbin
	inc	hl
	djnz	cvtval

clkdone:
	ret

bcdbin:	push	bc		; Convert byte pointed to by HL
	ld	a,(hl)		;  from BCD to binary and replace it
	ld	c,a
	and	0fh
	ld	b,a
	ld	a,c
	and	0f0h
	rrca
	rrca
	rrca
	ld	c,a
	rlca
	rlca
	add	a,c
	add	a,b
	ld	(hl),a
	pop	bc
	ret

	psect	data
watrdc:
	defb	10000101B	; hours
	defb	10000011B	; minutes
	defb	10000001B	; seconds
	defb	10000111B	; day
	defb	10001001B	; month
	defb	10001101B	; year
	defw	0 ; end of watch read command table

#endasm

#else

/* turn the RTC interrupts on  (Reh-Design CPU280 only)  SN  */
#asm 
	psect	text
	global	_RTclock
_RTclock: 
	push	bc
	call _iopg40
	ld   a,01000011B  ; 24 hrs, BCD format, per. interrupts  
	out  (11),a       ; write into register b of RTC
	call _iopg00
	pop	bc
	ret
	nop
#endasm                 

/* clear RTC interrupt (Reh-Design CPU280 only)   SN */
#asm
	psect	text
	global	_RDclock
_RDclock:
	push	bc
	call	_iopg40
	in   a,(12)    ; read register c of RTC to clear interrupt
	call	_iopg00
	pop	bc
	ret
#endasm

#endif /*Z280RC*/

#asm 	
	psect	text
	global _ill_trap, _trap_3
_ill_trap:

	PUSH	HL	;Dummy push because there is no reason code 

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
	LDHLSX	14+8	;Grab MSR we are coming from
	LD	A,H
	AND	40H	;Mask off U/S bit
	JP	NZ,USRILL  ;Handle case of user mode illegal instruction

;    trap2();  /* Args are already on stack */
	call	_trap2
;    panic("Illegal instruction");
	ld	hl,trap_msg
	push	hl
	call	_panic

	psect	data
trap_msg:	defm	'Illegal instruction'
	psect	text
USRILL:
	call	_trap_3
	JP	_IRET	;The IRET routine will call the handler if necessary
#endasm

#asm
	psect	text
	global	_segv_trap,_segv_trap_2

	nop
_segv_trap:

	PUSH	HL	;Dummy push because there is no reason code 

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
	LDHLSX	14+8	;Grab MSR we are coming from
	LD	A,H
	AND	40H	;Mask off U/S bit
	JP	NZ,USRSEG  ;Handle case of user mode illegal segv

;    trap2();  /* Args are already on stack */
	call	_trap2
;    kprintf("page %d  ", badpage());
	call	_badpage
	push	hl
	ld	hl,fmt_1
	push	hl
	call	_kprintf
	pop	hl
	pop	hl	
;    panic("System segv");
	push	hl,fmt_2
	call	_panic
	psect	data
fmt_1:	defm	'page %d '
fmt_2:	defm	'System segv'
	psect	text
USRSEG:
;    if (usegv2())	/* Try to read in the page */
	call	_usegv2
	ld	a,h
	or	l
	jp	z,1f
;    {
	call	_segv_trap_2
;	chksigs();		/* This will exit if appropriate */
	call	_chksigs
;    }
1:
	JP	_IRET	;The IRET routine will call the handler if necessary
#endasm

/* This returns the faulted page number */

#asm
	psect	text
	global	_badpage
_badpage:
	push	bc
	call	_iopgff
	ld	c,mmcr
	INW
	push	hl
	call	_iopg00
	pop	hl
	ld	a,l
	and	01fh
	ld	l,a
	ld	h,0
	pop	bc
	ret
	nop
#endasm

#asm
	psect	text
	global	_upadr
_upadr: ;  /*  upadr(char *uladr) */
	push	bc
	call	_iopgff
	LDHLSX	4	;get arg
	ld	a,h
	ld	b,a	; save hibyte addr
	srl	a
	srl	a
	srl	a
	srl	a
	srl	a
	;add	a,(_c_mode)	; get offset for program page
				defb	0ddh,087h
				defw	_c_mode	

	and	0fh    ;get high 4 bits of HL into a
	out	(pdr),a   ;load page descripter point
	ld	c,dsp
	INW		;read page descriptor
	push	hl
	call	_iopg00
	pop	hl
	ld	a,l
	and	08h	;test valid bit
	jp	z,0f   ;jump if not valid
	ld	a,l
	and	0E0h   ;clear off status bits and lsb
	ld	l,a
	ld	a,b	; get old hibyte addr
	and	010h	; extract bit 12
	or	l
	ld	l,a	; set addr bit 12 /* not stored in mmu */
	or	h	;set flags
	pop	bc
	ret

0:
	ld	hl,-1
	ld	a,h
	or	l
	pop	bc
	ret
#endasm

/* Same as above, except for system */
#asm 
	psect	text
	global	_spadr
_spadr: ;  /* spadr(char *sladr) */
	push	bc
	call	_iopgff
	LDHLSX	4	;get arg
	ld	a,h
	rrca
	rrca
	rrca
	rrca
	and	0fh    ;get high 4 bits of HL into a
	or	10h	;adjust value to point into system registers
			;(Only difference from above)
	out	(pdr),a   ;load page descripter point
	ld	c,dsp
	INW		;read page descriptor
	push	hl
	call	_iopg00
	pop	hl
	ld	a,l
	and	08h	;test valid bit
	jp	z,0f	;jump if not valid
	ld	a,l
	and	0f0h   ;clear off status bits		
	ld	l,a
	or	h	;set flags
	pop	bc
	ret

0:
	ld	hl,-1
	ld	a,h
	or	l
	pop	bc
	ret
#endasm


#asm 
	psect	text
	global	_doexec,_di
_doexec:
	call	_di
    	POP	HL
	POP	HL	;get argument
	LDUSP		;set user stack pointer
	LD	SP,_ub + 512  ;reset system stack pointer

	XOR	A
	ld	(_ub + ?OSYS),a

	LD	HL, 100H		;program entry point is at 100h
	PUSH	HL
	LD	HL, 0507FH	;MSR value: user mode, break on halt,
				;interrupts on
	PUSH	HL
	RETIL			;"return" into user process at 100H
#endasm


#asm 
	psect	text
	global	_usp
_usp:
	RDUSP  ;Get user stack pointer into HL
	LD	A,H
	OR	L
	ret
	nop
#endasm

#asm
	psect	text
	global	_msr
_msr:
	push	bc
	ld	C,00
	RDCTL
	pop	bc
	ld	A,H
	OR	L
	ret
	nop

intonmask	equ	07Fh	; Unmask all interrupts
intoffmask	equ	i$dma3+i$rx	; Leave DMA3 and UART Rx enabled

	psect	text
	global	_di
_di:  ; /* Same as spl(48h) */
;	DI
; version 1.12
	push	bc
	push	hl
	ld	c,msr
	RDCTL
	ld	l,intoffmask
	LDCTL
	pop	hl
	pop	bc
	ret
	nop
#endasm

#asm
	psect	text
	global	_ei
_ei:    ; /* Same as spl(0x7f) */
;	EI
	push	bc
	push	hl
	ld	c,msr
	RDCTL
	ld	l,intonmask
	LDCTL
	pop	hl
	pop	bc
	ret
	nop
#endasm

/* This sets the interrupt enable bits to the new value, and returns the old*/
#asm 
	psect	text
	global	_spl
_spl:  ; /* oldbits = spl(newbits) */
	push	bc
	LDHLSX  4	;get bits
	ld	e,l	;save new bits in e
	ld	c,msr
	RDCTL
	ld	d,l	;save original bits in d
	ld	a,e
	and	7fh
	ld	l,a	;replace interrupt bits with new ones in hl
	ld	h,0
	ld	c,msr	;reload MSR
	LDCTL	
	ld	a,d	;return original bits
	and	7fh
	ld	l,a
	pop	bc
	ret
	nop
#endasm

/* This copies the contents of one page to another, using the DMA */
#asm
	psect	text
	global	_pg_copy
_pg_copy:	; /* pg_copy(pageaddr src, pageaddr dest)  */
	push	bc
	LDHLSX	6		;get dest
	ld	a,l
	and	0e0h
	ld	l,a
	ld	(DSTPG),hl	;put into dma program
	LDHLSX	4		;get source
	ld	a,l
	and	0e0h
	ld	l,a
	ld	(SRCPG),hl	;put into dma program
	ld	A,0h		;use DMA0
	LD	HL,CPBLK
	;SEND PROGRAM TO DMA CONTROLLER
	call	dodma
	pop	bc
	RET
	nop
	psect	data

CPBLK:	defw	0000
DSTPG:	defw	0000
	defw	0000	
SRCPG:	defw	0000	
	defw	1000H 	 	;length in 16-bit words 8Kbyte
	defw	08300H		;continous, memory++ -> memory++, 16-bit
;	defw	08280H		;burst, memory++ -> memory++, 16-bit
;	defw	08200H		;single , memory++ -> memory++, 16-bit
	
	psect	text
#endasm


#asm
	psect	text
	global	_pg_zero
_pg_zero:	; /* pg_zero(pageaddr pg)  */
	push	bc
	LDHLSX	4		;get page addr
	ld	a,l
	and	0e0h
	ld	l,a
	ld	(ZERPG),hl	;put into dma program
	ld	hl,ZEROA
	inc	hl
	ld	a,l
	and	0feh
	ld	l,a		;generate aligned word
	ld	(ZRSRC),hl	; address into zeroa
	push	hl
	call	_spadr
	pop	de
	ld	(ZRSRC+2),hl	;generate and store high
				; order part of zero address
	ld	A,0h		;use DMA0
	LD	HL,ZRBLK
	;SEND PROGRAM TO DMA CONTROLLER
	call	dodma
	pop	bc
	RET
	nop
	psect	data

ZRBLK:	DEFW	0000
ZERPG:	DEFW	0000
ZRSRC:	DEFW	0000	
	DEFW	0000	
	DEFW	1000H	 	;length in 16-bit words
	DEFW	0A300H		;continous, memory -> memory++, 16-bit
;	DEFW	0A280H		;burst, memory -> memory++, 16-bit
;	DEFW	0A200H		;single , memory -> memory++, 16-bit

ZEROA:	DEFB	0,0,0		;memory location containing zero

	psect	text
#endasm


#asm 
; This loads dma program in (hl) into dma controller in a:
	psect	text
	global	dodma
dodma:
	PUSH	HL
	PUSH	AF
	CALL	_iopgff    ;This disables interrupts
	POP	AF
	POP	HL
	LD	C,A
	LD	B,6
1:	OUTIW
	INC	C
	INC	B
	DEC	B
	JP	NZ,1b
;	add	a,5	; point to dma descriptor register 
;	ld	c,a
;loop1:
;	;inw	hl,(c)
;	defb	0edh,0b7h
;	ld	a,l
;	and	10h
;	jr	z,loop1		; wait until dma EOT
	CALL	_iopg00		; This restores interrupts
	RET

#endasm

/* This sets the user mmu entry pgnum to value */
#asm 
	psect	text
	global	_setmmu
_setmmu: ; /* setmmu(int pgnum, unsigned value) */
	push	bc
	call _iopgff
	LDHLSX	4	;get pgnum
	ld	a,l
	out	(pdr),a	;set the PDR pointer in the MMU to the page number
	LDHLSX  6	;get value
	ld	c,dsp
	OUTW		;write value to I/O address f5 to put it into the MMU
	call	_iopg00
	pop	bc
	ret
#endasm


/* This gets the user mmu entry pgnum */
#asm
	psect	text
	global	_getmmu
_getmmu: ; /* getmmu(int pgnum) */
	push	bc
	call _iopgff
	LDHLSX	4	;get pgnum
	ld	a,l
	out	(pdr),a	;set the PDR pointer in the MMU to the page number
	ld	c,dsp
	INW		;read value from I/O address f5 
	push	hl
	call	_iopg00
	pop	hl
	ld	a,l
	and	0efh
	ld	l,a
	ld	a,h
	or	l
	pop	bc
	ret
#endasm

/* This invalidates the user part of the MMU */
#asm
	psect	text
	global	_clrmmu
_clrmmu:
	push	bc
	call	_iopgff
	ld	a,0ch
	out	(ip),a
        nop
        nop
        nop
        nop
	call	_iopg00
	pop	bc
	ret
	nop
#endasm

#ifndef NEWUPUT

#asm
	psect	text
	global	_uget
_uget: ; /* uget(char *uptr, char *sptr, unsigned nbytes) */
	push	bc
	LDHLSX  8	;get count
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX	6	;get dest
	ex	de,hl	;put into de
	LDHLSX 	4	;get source into hl
0:
	LDUD		;load a with (hl) from user data space
	call	c,ulflt	;call if access violation occurred
	ld	(de),a	;store in system space
	inc	de
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jp	nz,0b
	ld	hl,0000
	xor	a	;return 0 if OK
	pop	bc
	ret
#endasm

#endif	/* NEWUPUT */

/* See if a portion of user space is accessable */
#asm
	psect	text
	global	_utest
_utest:  ;  /* utest(char *uptr, unsigned nbytes) */
	push	bc
	LDHLSX  6	;get count
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX  4	;get source into hl
0:
	LDUD		;load a with (hl) from user data space
	call	c,ulflt	;call if access violation occurred
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jp	nz,0b
	ld	hl,0000
	xor	a	;return 0 if OK
	pop	bc
	ret
	nop
#endasm


/* This will not set EFAULT if there is a fault */
#asm
	psect	text
	global	_uzero
_uzero:  ; /* uzero(char *uptr, unsigned count) */
	push	bc
	LDHLSX  6	;get count
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX	4	;get dest
0:
	xor	a
	STUD		;load (hl) in user data space with 0
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jp	nz,0b
	pop	bc
	ret
	nop
#endasm

/* Get a string upto count */
#asm
	psect	text
	global	_ugets
_ugets: ; /* ugets(char *uptr, char *sptr, unsigned count) */
	push	bc
	LDHLSX  8	;get count
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX	6	;get dest
	ex	de,hl	;put into de
	LDHLSX  4	;get source into hl

0:
	LDUD		;load a with (hl) from user data space
	call	c,ulflt	;call if access violation occurred
	ld	(de),a	;store in system space
	and	a
	jp	z,1f   ;exit if hit null
	inc	de
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jp	nz,0b
1:	ld	hl,0000
	xor	a	;return 0 if OK
	pop	bc
	ret
	
#endasm

#ifndef	NEWUPUT

#asm
	psect	text
	global	_uput
_uput:  ; /* uput(char *sptr, char *uptr, unsigned nbytes) */
	push	bc
	LDHLSX  8	;get count
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX	4	;get source
	EX	de,hl	;put into de
	LDHLSX  6	;get dest into hl

0:
	ld	a,(de)	;get byte from system area
	STUD		;store byte into (hl) in user data space
	call	c,usflt	;call if access violation occurred
	inc	de
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jp	nz,0b
	ld	hl,0000
	xor	a	;return 0 if OK
	pop	bc
	ret
#endasm


#asm
	psect	text
	global	_uputp
_uputp:  ; /* uput(char *sptr, char *uptr, unsigned nbytes) */
	push	bc
	LDHLSX  8	;get count
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX	4	;get source
	ex	de,hl	;put into de
	LDHLSX  6	;get dest into hl
0:
	ld	a,(de)	;get byte from system area
	STUP		;store byte into (hl) in user data space
	call	c,usfltp	;call if access violation occurred
	inc	de
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz,0b

	ld	hl,0000
	xor	a	;return 0 if OK
	pop	bc
	ret
#endasm

#endif	/* NEWUPUT */

/* Push something onto user stack */
#asm
	psect	text
	global	_upush
_upush:  ;  /* upush(int val); */
	push	bc
	LDHLSX	4	;get arg
	ex	de,hl	;into DE
	RDUSP  		;Get user stack pointer into HL
	DEC	HL
	ld	A,D
	STUD
	call	c,usflt	;call if access violation occurred
	DEC	HL
	ld	A,E	;shove DE onto user stack
	STUD
	call	c,usflt	;call if access violation occurred
	LDUSP		;update user SP
	pop	bc
	RET
#endasm


/* Ulflt or usflt are called when one of the routines above has an
access violation. They try to read in the faulted page.  If this works,
a stud or ldud are re-excuted, and they return.  Otherwise they return
to the next higher level calling routine with a failure code. */

#asm 
ulflt:
	psect	text
	global	ulflt,usflt,usfltp

	push	bc
	push	de
	push	hl
	call	_upage	;Call upage() with HL as argument
	ld	a,h
	or	l
	pop	hl
	pop	de
	pop	bc
	jp	z,0f
	;this was an illegal address
	pop	hl	;pop off this level's return address
	call	_ufault
	ld	hl,0001
	ld	a,h
	or	l	;return 1 if fail
	pop	bc
	ret
0:
	LDUD		;re-execute the faulted ldud
	ret

; this is for data page

usflt:
	;ldw	(_c_mode),0	; reset offset
				defb	0ddh,011h
				defw	_c_mode
				defw	0
	push	af
	push	bc
	push	de
	push	hl
	call	_upage	;Call upage() with HL as argument
	ld	a,h
	or	l
	pop	hl
	pop	de
	pop	bc
	jp	z,0f
	pop	af	;pop after conditional jump because of flags
	;this was an illegal address
	pop	hl	;pop off this level's return address
	call	_ufault
	ld	hl,0001
	ld	a,h
	or	l	;return 1 if fail
	pop	bc
	ret
0:
	pop	af	;get back accumulator
	STUD		;re-execute the faulted stud
	ret
	nop
; this is for program page

usfltp:
	;ldw	(_c_mode),8	; offset for program page
				defb	0ddh,011h
				defw	_c_mode
				defw	8
	push	af
	push	bc
	push	de
	push	hl
	call	_upage	;Call upage() with HL as argument
	ld	a,h
	or	l
	;ldw	(_c_mode),0	; reset offset
				defb	0ddh,011h
				defw	_c_mode
				defw	0
	pop	hl
	pop	de
	pop	bc
	jp	z,0f
	pop	af	;pop after conditional jump because of flags
	;this was an illegal address
	pop	hl	;pop off this level's return address
	call	_ufault
	ld	hl,0001
	ld	a,h
	or	l	;return 1 if fail
	pop	bc
	ret
0:
	pop	af	;get back accumulator
	STUP		;re-execute the faulted stup
	ret
	nop
#endasm
