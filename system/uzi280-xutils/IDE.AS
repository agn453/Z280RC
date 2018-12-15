
;	title	'Harddisk Driver Module of UZI280'

; 930414 Tilmann Reh
; 930723 Stefan Nitschke	Version for UZI280


;Cyls	equ	1024
;Heads	equ	12
;Secs	equ	34


; ]bersetzung: log. Tracks = Zylinder, log. Sektoren = Sektoren * Heads.

;***********************************************************************
;**           IDE - Treiber (REH IDE-Interface)                       **
;***********************************************************************


         psect      text

         global     _wdsec,_wdtrk,_dptrlo,_wd_rdwr,_wd_io,_hdinit   


	global	_WD_Sector  ; from ideconf.obj 
		
; Write/Read entry from devwd.c 
_wd_io:
          ld   hl,(_wd_rdwr)
          ld   a,l
          or   h
          jp   z,hdread
          jp   hdwrite


; Task File Register Definitionen:

IDEBase	equ	80h		; Basisadresse IDE-Interface
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
 
; Makros: warten, bis Festplatte bereit (Non-Busy) oder DRQ aktiv.
; Voraussetzung: Bus-Page bereits aktiv.

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

; not used
_hdinit:
	ld	hl,0
	ret			; ret false

; read data from hard disc
hdread:
01:	call	HdPrep		; Task File Register setzen, DMA-Adr. rechnen
	ld	a,CmdRd
	out	(IdeCmd),a	; Befehl an Festplatte: Sektor lesen
	WaitDrq
	in	a,(IdeDat)	; IDE-Interface: LH-Flipflop setzen
	ld	hl,(_dptrlo)
	ld	c,IdeDat
	ld	b,0
	inir
	inir
	WaitRdy
	in	a,(IdeCmd)
	and	10001001B	; Busy, DRQ, oder Error?
	ret	z		; nein: alles ok
	ret

; Daten auf Festplatte schreiben:
hdwrite:
01:	call	HdPrep		; Task File Register setzen, DMA-Adr. rechnen
	ld	a,CmdWr
	out	(IdeCmd),a	; Befehl an Festplatte: Sektor schreiben
	WaitRdy
	WaitDrq
	ld	hl,(_dptrlo)
	ld	c,IdeDat
	ld	b,0
	otir
	otir
	WaitRdy
	in	a,(IdeCmd)
	and	10001001B	; Busy, DRQ, oder Error?
	ret	z		; nein: alles ok
	ret

; Lese- bzw. Schreiboperation vorbereiten (Register setzen).
; R}ckkehr mit phys. DMA-Adresse in AHL.

HdPrep:	
	WaitRdy			; warten bis Platte bereit
	ld	a,1
	out	(IdeSCnt),a	; Sektoranzahl immer 1
	ld	hl,(_wdsec)	; log. Sektor (ab 0)
;	divu	hl,(WD_Sector)	; Auft. in Head (Quotient) und Sektor (Rest)
	defb	0ddh,0edh,0fdh
	defw	_WD_Sector
	or	0A0h		; (Head in A) Drive 0 dazu
	out	(IdeSDH),a	; SDH Register ausgeben
	ld	a,l		; phys. Sektor (ab 0)
	inc	a
	out	(IdeSNum),a	; Sektornummer (ab 1)
	ld	hl,(_wdtrk)
	ld	a,l
	out	(IdeCLo),a
	ld	a,h
	out	(IdeCHi),a	; Cylinder ausgeben
	in	a,(IdeCmd)	; wegen Z280 Chipfehler
	ret

