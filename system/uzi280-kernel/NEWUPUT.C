
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
	this file is for sep I/D pages

	new uget(), uput() uputp() that use DMA0 for data transfer
	between system and user data/prog space if more than 100 bytes
	must be transfered.

	access validation is tested for every page that
	is accessed during dma-copy. 

	Maximum of 8K bytes can be transfered (kernel only transfers
	max. 512 bytes !). System pages must have contiguous physical
	address space, no test is done in system space ! 
		
	10/06/93 SN
*/						
#asm
p_len	equ	2000h	/* length of page */
p_msk0	equ	0e0h	/* mask to get page no. */
p_msk1	equ	01fh	/* mask to get offset within page */
#endasm



#asm 
*Include	z280rc.inc

*Include	macro.lib

	psect	text
	global	_uput,_spadr,_upadr,dodma,_c_mode
	global	usflt,ulflt,usfltp

_uput:  ;/* uput(char *sptr, char *uptr, unsigned nbytes) */
	push	bc
	LDHLSX  8	;get count
	;cpw	hl,50
				defb	0fdh,0edh,0f7h
				defw	50
			; if more then 50 bytes to transfer
	jp	nc,nuput ; use DMA0
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX	4	;get source
	ex	de,hl	;put into de
	LDHLSX  6	;get dest into hl

uputlp:
	ld	a,(de)	;get byte from system area
	STUD		;store byte into (hl) in user data space
	call	c,usflt	;call if access violation occurred
	inc	de
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz,uputlp
	ld	hl,0000
	xor	a	;return 0 if OK
	pop	bc
	ret
;use DMA0 for data transfer
nuput:
	ld	(dmacnt),hl	;save count
	LDHLSX	4	 	;get source
	ld	(dmasrc),hl	; src-addr low
	push	hl
	call	_spadr		
	pop	de
	ld	(dmasrc+2),hl	; src-addr high
	LDHLSX  6		;get dest into hl

;calculate dest pages for dma-transfer
	ld	(dmadest),hl
	STUD			;store byte into (hl) in user data space
	call	c,usflt		;call if access violation occurred and
				;give a new page if possible
	push	hl
	call	_upadr
	pop	de
	ld	(dmadest+2),hl	; save Hi_dest_addr
; test for page boundary 
	ld	hl,(dmadest)	; get dest addr
	ld	b,h
	ld	de,(dmacnt)
;	dec	de
	add	hl,de		; add count-1
	ld	a,h
	and	p_msk0
	ld	c,a
	ld	a,b
	and	p_msk0
	cp	c
	jr	nz,pg_b0	; more than one page
dma_0:	ld	hl,movblk
	ld	a,dal0
	call 	dodma		; start DMA0
	ld	hl,0
	xor	a	;return 0 OK
	pop	bc
	ret		
; we have to copy to more than one page
pg_b0:
	ld	hl,(dmadest)
	ld	a,h		; hl = dest addr
	and	p_msk1		; offset within page
	ld	h,a
	ld	de,p_len	; page length
	ex	de,hl
	SUBDE		
	ld	(dmacnt),hl	; rest length of first page
	ex	de,hl
	LDHLSX	8		; get old count
	SUBDE			; length for next page		
	ld	(pg_len),hl	; save it
	ld	hl,movblk
	ld	a,dal0
	call	dodma		; copy into first page
	ld	hl,(dmadest)	; get old dest
	ld	de,(dmacnt)	; add bytes transfered
	add	hl,de
	ld	(dmadest),hl
	STUD			;store byte into (hl) in user data space
	call	c,usflt		;call if access violation occurred and
				;give a new page if possible
	push	hl
	call	_upadr
	pop	de
	ld	(dmadest+2),hl	; save Hi_dest_addr
	ld	hl,(dmasrc)	; get old src
	ld	de,(dmacnt)
	add	hl,de		; new src
	ld	(dmasrc),hl
	push	hl
	call	_spadr
	pop	de
	ld	(dmasrc+2),hl
	ld	hl,(pg_len)	; get count for next page
	ld	(dmacnt),hl	; save new count
	jp	dma_0		; copy next page
#endasm


#asm 
	psect	text
	global	_uget

	nop	
_uget:	;  /* uget(char *uptr, char *sptr, unsigned nbytes) */
	push	bc
	LDHLSX  8	;get count
	;cpw	hl,50
				defb	0fdh,0edh,0f7h
				defw	50
			; if more then 50 bytes to transfer
	jp	nc,nuget ; use DMA0
	ld	b,h
	ld	c,l	;put into bc
	LDHLSX	6	;get dest
	ex	de,hl	;put into de
	LDHLSX  4	;get src into hl
ugetlp:
	LDUD		;get byte from (hl) in user space
	call	c,ulflt	;call if access violation occurred
	ld	(de),a
	inc	de
	inc	hl
	dec	bc
	ld	a,b
	or	c
	jr	nz,ugetlp
	ld	hl,0000
	xor	a	;return 0 if OK
	pop	bc
	ret
;use DMA0 for data transfer
nuget:
	ld	(dmacnt),hl	;save count
	LDHLSX	6	 	;get dest
	ld	(dmadest),hl	;dest-addr low
	push	hl
	call	_spadr
	pop	de
	ld	(dmadest+2),hl	; dest-addr high
	LDHLSX  4		;get src into hl

;calculate src pages for dma-transfer
	ld	(dmasrc),hl
	LDUD			;get byte from (hl) in user data space
	call	c,ulflt		;call if access violation occurred and
				;give a page if possible
	push	hl
	call	_upadr
	pop	de
	ld	(dmasrc+2),hl	; save Hi_src_addr
; test for page boundary 
	ld	hl,(dmasrc)	; get src addr
	ld	b,h
	ld	de,(dmacnt)
;	dec	de
	add	hl,de		; add count-1
	ld	a,h
	and	p_msk0
	ld	c,a
	ld	a,b
	and	p_msk0
	cp	c
	jr	nz,pg_b1
dma_1:	ld	hl,movblk
	ld	a,dal0
	call 	dodma		; start DMA0
	ld	hl,0000
	xor	a	;return 0 OK
	pop	bc
	ret		
pg_b1:
	ld	hl,(dmasrc)
	ld	a,h		; hl = src addr
	and	p_msk1		; offset within page
	ld	h,a
	ld	de,p_len	; page length
	ex	de,hl
	SUBDE
	ld	(dmacnt),hl	; rest length of first page
	ex	de,hl
	LDHLSX	8		; get old count
	SUBDE			; length for next page		
	ld	(pg_len),hl	; save it
	ld	hl,movblk
	ld	a,dal0
	call	dodma		; copy into first page
	ld	hl,(dmasrc)	; get old src
	ld	de,(dmacnt)	; add bytes transfered
	add	hl,de
	ld	(dmasrc),hl	; save new src 
	LDUD			;get byte from (hl) in user data space
	call	c,ulflt		;call if access violation occurred and
				;give a page if possible
	push	hl
	call	_upadr
	pop	de
	ld	(dmasrc+2),hl	; save Hi_src_addr
	ld	hl,(dmadest)	; get old dest
	ld	de,(dmacnt)
	add	hl,de		; new dest
	ld	(dmadest),hl
	push	hl
	call	_spadr
	pop	de
	ld	(dmadest+2),hl
	ld	hl,(pg_len)
	ld	(dmacnt),hl	; new count
	jp	dma_1
#endasm




#asm 
	psect	text
	global	_uputp

_uputp:	  ; /* uputp(char *sptr, char *uptr, unsigned nbytes) */
	push	bc
	LDHLSX  8		;get count
	ld	(dmacnt),hl	;save count
	LDHLSX	4	 	;get source
	ld	(dmasrc),hl	; src-addr low
	push	hl
	call	_spadr
	pop	de
	ld	(dmasrc+2),hl	; src-addr high
	LDHLSX  6		;get dest into hl
;calculate dest pages for dma-transfer
	ld	(dmadest),hl
	STUP			;store byte into (hl) in user proc space
	call	c,usfltp	;call if access violation occurred and
				;give a new page if possible
	;ldw	(c@mode?),8	; offset for program page
				defb	0ddh,011h
				defw	_c_mode
				defw	8
	push	hl
	call	_upadr
	pop	de
	ld	(dmadest+2),hl	; save Hi_dest_addr
; test for page boundary 
	ld	hl,(dmadest)	; get dest addr
	ld	b,h
	ld	de,(dmacnt)
;	dec	de
	add	hl,de		; add count-1
	ld	a,h
	and	p_msk0
	ld	c,a
	ld	a,b
	and	p_msk0
	cp	c
	jr	nz,pg_b2	; more than one page
dma_2:	ld	hl,movblk
	ld	a,dal0
	call 	dodma		; start DMA0
	;ldw	(c@mode?),0	; reset offset 
				defb	0ddh,011h
				defw	_c_mode
				defw	0
	ld	hl,0
	xor	a	;return 0 OK
	pop	bc
	ret		
; we have to copy to more than one page
pg_b2:
	ld	hl,(dmadest)
	ld	a,h		; hl = dest addr
	and	p_msk1		; offset within page
	ld	h,a
	ld	de,p_len	; page length
	ex	de,hl
	SUBDE		
	ld	(dmacnt),hl	; rest length of first page
	ex	de,hl
	LDHLSX	8		; get old count
	SUBDE			; length for next page		
	ld	(pg_len),hl	; save it
	ld	hl,movblk
	ld	a,0h
	call	dodma		; copy into first page
	ld	hl,(dmadest)	; get old dest
	ld	de,(dmacnt)	; add bytes transfered
	add	hl,de
	ld	(dmadest),hl
	STUP			;store byte into (hl) in user prog space
	call	c,usfltp	;call if access violation occurred and
				;give a new page if possible
	;ldw	(c@mode?),8	; offset for program page
				defb	0ddh,011h
				defw	_c_mode
				defw	8
	push	hl
	call	_upadr
	pop	de
	ld	(dmadest+2),hl	; save Hi_dest_addr
	ld	hl,(dmasrc)	; get old src
	ld	de,(dmacnt)
	add	hl,de		; new src
	ld	(dmasrc),hl
	push	hl
	call	_spadr
	pop	de
	ld	(dmasrc+2),hl
	ld	hl,(pg_len)	; get count for next page
	ld	(dmacnt),hl	; save new count
	jp	dma_2
#endasm


#asm
		psect	data
movblk:
dmadest:	defw	0
		defw	0	
dmasrc:		defw	0
		defw	0
dmacnt:		defw	0	;length
dmadescr:	Defw	08100H	;continous, memory++ -> memory++, 8-bit
;dmadescr:	Defw	08000H	;single, memory++ -> memory++, 8-bit

; var for page boundary check
pg_len:		defw	0	;
		psect	text
#endasm

