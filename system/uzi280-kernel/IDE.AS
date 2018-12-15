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
;	title	'Harddisk Driver Module of UZI280 with DMA3'

; 930414 Tilmann Reh
; 930723 Stefan Nitschke	Version for UZI280
;
; should be replaced by a interrupt driven routine
;

*Include	macro.lib

Stand_Alone	equ	0	; not tested yet
CPM_Boot	equ	1	; means UZI is booted from CPM

	global	_WD_Cyls,_WD_Heads,_WD_Sector ; defined in ideconf.c

dal1	equ	018h	; this will use DMA3 for data transfer
des1	equ	dal1+5h	;

; translation: log. tracks = cylinder, log. Sectors = sectors * heads.

;***********************************************************************
;**           IDE - Treiber (REH IDE-Interface)                       **
;***********************************************************************


         psect      text

         global     _wdsec,_wdtrk,_dptrlo,_wd_rdwr,_wd_io,_hdinit   
	 global	    _dptrhi,_iopg00,_iopgff

	 global	    _wd_error,_rw_flag
		
; write/read entry from devwd.c 
_wd_io:
          ld   hl,(_wd_rdwr)
          ld   a,l
          or   h
          jp   z,hdread
          jp   hdwrite


; Task File Register Definition:

IDEBase	equ	80h		; basis adress IDE-Interface
IdeDOR	equ	IDEBase+6	; Digital Output Register
IdeDat	equ	IDEBase+8	; Data Register
IdeErr	equ	IDEBase+9	; Error Register
IdeSCnt	equ	IDEBase+0Ah	; Sector Count
IdeSNum	equ	IDEBase+0Bh	; Sector Number
IdeCLo	equ	IDEBase+0Ch	; Cylinder Low
IdeCHi	equ	IDEBase+0Dh	; Cylinder High
IdeSDH	equ	IDEBase+0Eh	; Drive and Head
IdeCmd	equ	IDEBase+0Fh	; Command / Status

; IDE Festplatten Kommandos:

CmdHome	equ	10h		; Recalibrate
CmdRd	equ	20h		; Read Sector
CmdWr	equ	30h		; Write Sector
CmdInit	equ	91h		; Initialize Drive Params

; number of read/write retries
Retrys	equ	10
 
; Makros: wait, for harddisk ready (Non-Busy) or DRQ active.
; assumption: Bus-Page active.

macro	WaitRdy	
01:	in	a,(IdeCmd)
	rla
	jr	c,01b
	endm

macro	WaitDrq	
02:	in	a,(IdeCmd)
	bit	3,a
	jr	z,02b
	endm

COND	CPM_Boot
; since UZI is booted from CPM no initialisation is necessary.  
_hdinit:
	ld	hl,0
	ret
;	nop	; for alignment
ENDC

COND	Stand_Alone
; Initialisierung von IDE-Festplatten (z.B. Translation Mode aktivieren).
; Wird beim System-Boot einmal aufgerufen. Wenn die Festplatte nicht
; reagiert, werden die DPH's H: bis K: in der Drive Table gel|scht.
_hdinit:
	ld	a,6
	out	(IdeDOR),a	; IDE Festplatte: Software Reset
	ld	b,0
	djnz	$		; (min. 3 us warten)
	ld	a,2
	out	(IdeDOR),a	; Software Reset aufheben.
	ld	hl,Counter
	ld	(hl),0		; Z{hler 

HDI_1:	bit	7,(hl)		; Z{hler abgelaufen ?
	jr	nz,HDI_Ex	; ja: Festplatten-DPH's l|schen
	ld	hl,(Counter)
	inc	hl
	ld	(Counter),hl
	in	a,(IdeCmd)
	bit	7,a
	jr	nz,HDI_1	; sonst weiter warten auf Non-Busy
	and	11111001B
	cp	01010000B	; Status: Ready & NoError ?
	jr	nz,HDI_Ex	; Fehler x--> Rausschmi~
	ld	a,(_WD_Sector)
	out	(IdeSCnt),a	; Sector Count: Anzahl Sektoren pro Spur
	ld	hl,(_WD_Cyls)
	ld	a,l
	out	(IdeCLo),
	ld	a,h
	out	(IdeCHi),a	; Anzahl Zylinder angeben
	ld	a,(_WD_Heads)
	dec	a
	or	0a0h
;	ld	a,0A0h .or. (Heads-1)
	out	(IdeSDH),a	; SDH: Anzahl K|pfe -1
	ld	a,CmdInit	; Befehl : Initialisierung
	out	(IdeCmd),a	; (warten auf Non-Busy beim n{chsten Befehl)
	ld	hl,1
	ret			; ret true

HDI_Ex:
	ld	hl,0
	ret			; ret false
ENDC

; read data from hard disk
hdread:
01:	call	HdPrep		; Task File Register setzen

	ld	hl,IdeDat	; Data port
	xor	a			
	ld	(hdsource),hl		
	ld	(hdsource+2),a		; Source = Ide data port
	ld      (hdsource+3),a 
	ld      hl,(_dptrlo)		; set dest. address
	ld	(hddest),hl
	ld      hl,(_dptrhi) 
	ld	(hddest+2),hl

	ld	a,CmdRd
	out	(IdeCmd),a	; Befehl an Festplatte: read sector 
	WaitDrq
	in	a,(IdeDat)	; IDE-Interface: set LH-Flipflop

	ld	hl,1110000100000000B	; Continuous  Mode
;	ld	hl,1110000000000000B	; single Mode
	call	hddmaio			; Start DMA transfer

	WaitRdy
	in	a,(IdeCmd)
	and	10001001B	; Busy, DRQ, or Error?
	ret	z		; no: all ok
	call	_wd_error	; print warning
	ret

; write data to hard disk
hdwrite:
01:	call	HdPrep		; set Task File Register

	ld	hl,IdeDat		; Ide data port
	xor	a		
	ld	(hddest),hl	
	ld	(hddest+2),a		; Destination = IDE data port
        ld      (hddest+3),a
        ld      hl,(_dptrlo)		; set source address
	ld	(hdsource),hl
        ld      hl,(_dptrhi)
	ld	(hdsource+2),hl

	ld	a,CmdWr
	out	(IdeCmd),a	; Befehl an Festplatte: write sector
	WaitRdy
	WaitDrq

	ld	hl,1000000100001100B	; Continuous  Mode
;	ld	hl,1000000000001100B	; single Mode
	call	hddmaio			; Start DMA transfer

	WaitRdy
	in	a,(IdeCmd)
	and	10001001B	; Busy, DRQ, oder Error?
	ret	z		; nein: alles ok
	call	_wd_error	; print warning
	ret

; Lese- bzw. Schreiboperation vorbereiten (Register setzen).
HdPrep:	
	WaitRdy			; wait for hard disc ready
	ld	a,1
	out	(IdeSCnt),a	; Sector count always 1
	ld	hl,(_wdsec)	; log. Sector (from 0)
;	divu	hl,(WD_Sector)	; Auft. in Head (Quotient) and Sector (Rest)
	defb	0ddh,0edh,0fdh
	defw	_WD_Sector
	or	0A0h		; (Head in A) Drive 0 dazu
	out	(IdeSDH),a	; SDH Register ausgeben
	ld	a,l		; phys. Sector (from 0)
	inc	a
	out	(IdeSNum),a	; Sector number (from 1)
	ld	hl,(_wdtrk)
	ld	a,l
	out	(IdeCLo),a
	ld	a,h
	out	(IdeCHi),a	; Cylinder ausgeben
	in	a,(IdeCmd)	; because of Z280 Chip bug
	ret



;----------------------------------
; Setup DMA3 for Harddisk Transfer
;----------------------------------
hddmaio:			
	ld	(hddescr),hl	
;DMA 1 init --> transfer
        call    _iopgff
	ld	c,dal1		; Adress of DMA-Register
	ld      hl,hddest
        ld      b,6		
	;outiw			
loop00:	defb	0EDH,83h
	inc	c		
        inc     b
        dec     b
        jr      nz,loop00
;	ld	c,des1
;loop001:
;	;inw	hl,(c)		; get descriptor
;	defb	0edh,0b7h
;	ld	a,l
;	and	10h
;	jr	z,loop001	; wait for transfer finish
        call    _iopg00		; set bus page
	ret			
	
; dummy routine
	global	_dma3_int
_dma3_int:	inc	sp
		inc	sp
		RETIL
;------------------------------
; Variable Field for DMA3
;------------------------------ 
	psect	data

hddest:		defw	0,0	
hdsource:	defw	0,0
dmacnt:		defw	512	; length: 512 Byte (physic. sector size)
hddescr:	defw	0	

Counter:	defw	0	; retry counter
_rw_flag:	defw	0	; reserved for interrupt driven hard disk io 

