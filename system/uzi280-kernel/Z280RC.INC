;
; Z280RC.INC  -  Z280RC Hardware I/O Port equates for HiTech ZAS assembler
;
; This file contains Z280 definitions taken from the July 1987
; "Zilog Z280(TM) MPU Microprocessing Unit Preliminary Technical Manual"
; for internal control registers and on-chip peripherals.
;
; It also contains Z280RC device specific I/O port definitions
; for the IDE CompactFlash interface and DS1203 timekeeper chip.
;
;
; Modification History
;
; 27-Jul-2018	Tony Nicholson	Initial version from CPU280 BIOS sources
;				file Z280EQU.MAC by Daniel A. Jacobs
;				dated 13-Mar-1988.
; 05-Dec-2018	Tony Nicholson	Added Interrupt and Trap Vector
;				Table related equates and System-Call
;				function definitions.
;

;---------------------------------
;
; Z280 MPU internal devices
;

; Master Control Register (MCR) equates:	 Z280 manual	Page 5-77

msr	equ	0	; Master Status Register	Page 3-4
isr	equ	16h	; Interrupt Status Reg.		Page 3-4 & 3-5
itv	equ	6	; Interrupt/Trap Vector
			;   table pointer		Page 3-5 & 6-11
iop	equ	8	; I/O Page register	(8bit)	Page 3-5
bti	equ	0ffh	; Bus Timing and
			;   Initialization reg.	(8bit)	Page 3-1 & 3-2
btc	equ	2	; Bus Timing and
			;   Control register	(8bit)	Page 3-2
slr	equ	4	; Stack Limit Register		Page 3-6
tcr	equ	10h	; Trap Control Reg.	(8bit)	Page 3-5 & 3-6
ccr	equ	12h	; Cache Control Reg.	(8bit)	Page 3-3 & 3-4
lar	equ	14h	; Local (global)
			;   Address Register	(8bit)	Page 3-3

;---------------------------------

; Memory Management Unit (MMU) equates:	 Z280 manual	Page 7-6

mmup	equ	0ffh	; MMU Page I/O
mmcr	equ	0f0h	; MMU Master Control Reg.	Page 7-5
pdr	equ	0f1h	; Page Descripter Reg. pointer	Page 7-2 & 7-5
dsp	equ	0f5h	; Descriptor Select Port	Page 7-6
bmp	equ	0f4h	; Block Move Port		Page 7-6
ip	equ	0f2h	; Invalidation Port		Page 7-6

;---------------------------------

; Refresh Rate Register (RRR) equates:	Z280 manual	Page 9-1

rrrp	equ	0ffh	; RRR Page I/O
rrr	equ	0e8h	; Refresh Rate Register		Page 9-1

;---------------------------------

; Counter Timers (C/T) equates:		Z280 manual	Page 9-7

ctp	equ	0feh	; Counter/Timer Page I/O

cr0	equ	0e0h	; Configuration reg. for Counter/Timer 0	Page 9-5
cr1	equ	0e8h	; Configuration reg. for Counter/Timer 1	Page 9-5
cr2	equ	0f8h	; Configuration reg. for Counter/Timer 2	Page 9-5

cs0	equ	0e1h	; Command Status reg for Counter/Timer 0	Page 9-6
cs1	equ	0e9h	; Command Status reg for Counter/Timer 1	Page 9-6
cs2	equ	0f9h	; Command Status reg for Counter/Timer 2	Page 9-6

tc0	equ	0e2h	; Time Constant reg. for Counter/Timer 0	Page 9-6
tc1	equ	0eah	; Time Constant reg. for Counter/Timer 1	Page 9-6
tc2	equ	0fah	; Time Constant reg. for Counter/Timer 2	Page 9-6

ct0	equ	0e3h	; Count Time reg.    for Counter/Timer 0	Page 9-6
ct1	equ	0ebh	; Count Time reg.    for Counter/Timer 1	Page 9-6
ct2	equ	0fbh	; Count Time reg.    for Counter/Timer 2	Page 9-6

;---------------------------------

; Direct Memory Access (DMA) equates:	Z280 manual	Page 9-15

dmap	equ	0ffh	; DMA Page I/O
dmcr	equ	1fh	; DMA Master Control Register	Page 9-13 & 9-14

dal0	equ	0	; Destination Address Low DMA0	Page 9-14 & 9-15
dal1	equ	8	; Destination Address Low DMA1	Page 9-14 & 9-15
dal2	equ	10h	; Destination Address Low DMA2	Page 9-14 & 9-15
dal3	equ	18h	; Destination Address Low DMA3	Page 9-14 & 9-15

dah0	equ	1	; Destination Address High DMA0	Page 9-14 & 9-15
dah1	equ	9	; Destination Address High DMA1	Page 9-14 & 9-15
dah2	equ	11h	; Destination Address High DMA2	Page 9-14 & 9-15
dah3	equ	19h	; Destination Address High DMA3	Page 9-14 & 9-15

sal0	equ	2	; Source Address Low DMA0	Page 9-14 & 9-15
sal1	equ	0ah	; Source Address Low DMA1	Page 9-14 & 9-15
sal2	equ	12h	; Source Address Low DMA2	Page 9-14 & 9-15
sal3	equ	1ah	; Source Address Low DMA3	Page 9-14 & 9-15

sah0	equ	3	; Source Address High DMA0	Page 9-14 & 9-15
sah1	equ	0bh	; Source Address High DMA1	Page 9-14 & 9-15
sah2	equ	13h	; Source Address High DMA2	Page 9-14 & 9-15
sah3	equ	1bh	; Source Address High DMA3	Page 9-14 & 9-15

dcr0	equ	4	; Counter Register for DMA0	Page 9-14
dcr1	equ	0ch	; Counter Register for DMA1	Page 9-14
dcr2	equ	14h	; Counter Register for DMA2	Page 9-14
dcr3	equ	1ch	; Counter Register for DMA3	Page 9-14

tdr0	equ	5	; Transaction Descriptor DMA0	Page 9-13 & 9-14
tdr1	equ	0dh	; Transaction Descriptor DMA1	Page 9-13 & 9-14
tdr2	equ	15h	; Transaction Descriptor DMA2	Page 9-13 & 9-14
tdr3	equ	1dh	; Transaction Descriptor DMA3	Page 9-13 & 9-14

;---------------------------------

; Universal Asynchronous Receiver/Transmitter (UART) equates:   Page 9-20

uartp	equ	0feh	; UART Page I/O
ucr	equ	10h	; UART Configuration Reg.	Page 9-18
tcs	equ	12h	; Transmitter Control/Status	Page 9-19
rcs	equ	14h	; Receiver Control/Status reg.	Page 9-20
rdr	equ	16h	; Receiver Data Register
tdr	equ	18h	; Transmitter Data Register

uart$en	equ	10000000B ; Enable bit for TCS and RCS
tcstbe	equ	00000001B ; Transmit Buffer Empty bit
rcsdav	equ	00010000B ; Receive Data Available bit

; UART Configuration Register (UCR) bits

ucr$bcm	equ	11000000B ; BC field of UART control register
ucr$5bv	equ	00000000B	; 5-bits per character
ucr$6bv	equ	01000000B	; 6-bits
ucr$7bv	equ	10000000B	; 7-bits
ucr$8bv	equ	11000000B	; 8-bits
ucr$pm	equ	00100000B ; P - Parity bit enable
ucr$pv	equ	00100000B	; extra bit with parity
ucr$eom	equ	00010000B ; E/O - parity even / odd
ucr$ev	equ	00010000B	; Even Parity
ucr$ov	equ	00000000B	; Odd Parity
ucr$csm	equ	00001000B ; CS - Clock select
ucr$ct1v equ	00001000B	; Use Counter/Timer 1 output for clock
ucr$cexv equ	00000000B	; External clock input from Counter/Timer 1 input
ucr$crm	equ	00000110B ; CR - Clock Rate multiplier
ucr$x1v	equ	00000000B	; x1
ucr$x16v equ	00000010B	; x16
ucr$x32v equ	00000100B	; x32
ucr$x64v equ	00000110B	; x64
ucr$lbm	equ	00000001B ; LB - Loop Back Enable
ucr$lbv	equ	00000001B	; Local Loop back mode

;---------------------------------

; Bit Masks for Z280 Master Status Register and Interrupt Status Register

i$none	equ	00000000B ; No Interrupts
i$a	equ	00000001B ; Bit Value INTA Interrupt
i$ct0	equ	00000010B ; Bit Value Counter/Timer 0 & DMA0 Interrupt
i$dma0	equ	i$ct0
i$b	equ	00000100B ; Bit Value INTB Interrupt
i$ct1	equ	00001000B ; Bit Value Counter/Timer 1, UART Rx & DMA1 Interrupt
i$rx	equ	i$ct1
i$dma1	equ	i$ct1
i$c	equ	00010000B ; Bit Value INTC Interrupt
i$tx	equ	00100000B ; Bit Value UART Tx & DMA2 Interrupt
i$dma2	equ	i$tx
i$ct2	equ	01000000B ; Bit Value Counter/Timer 2 & DMA3 Interrupt
i$dma3	equ	i$ct2

;---------------------------------

; IDE CompactFlash Interface in 16-bit mode

cfp	equ	00h	; CF I/O page
cfdata	equ	0c0h	; CF data register (R/W)
cferr	equ	0c2h	; CF error register (R)
cffeat	equ	0c2h	; CF features register (W)
cfsectcnt equ	0c5h	; CF sector count
cf07	equ	0c7h	; CF LA0-7
cf815	equ	0c9h	; CF LA8-15
cf1623	equ	0cbh	; CF LA16..23
cf2427	equ	0cdh	; CF LA24..27 and LBA mode (bit 6 high)
cfstat	equ	0cfh	; CF status register (R)
cfcmd	equ	0cfh	; CF command register (W)

;---------------------------------

; DS1302 timekeeper clock chip

watchp	equ	00h	; DS1302 I/O page
watch	equ	0a2h	; DS1302 3-wire interface
w$sclk	equ	00000001B	; SCLK bit
w$nrst	equ	00000010B	; nRST bit
w$data	equ	10000000B	; data I/O bit

;---------------------------------
;
; Interrupt and Trap Vector Table offsets (Page 6-11)
;
itvtoff$nmi	equ	04h	; NMI vector
itvtoff$inta	equ	08h	; Interrupt line A vector
itvtoff$intb	equ	0ch	; Interrupt line B vector
itvtoff$intc	equ	10h	; Interrupt line C vector
itvtoff$tc0	equ	14h	; Counter/Timer 0 vector
itvtoff$tc1	equ	18h	; Counter/Timer 1 vector
itvtoff$tc2	equ	20h	; Counter/Timer 2 vector
itvtoff$dma0	equ	24h	; DMA channel 0 vector
itvtoff$dma1	equ	28h	; DMA channel 1 vector
itvtoff$dma2	equ	2ch	; DMA channel 2 vector
itvtoff$dma3	equ	30h	; DMA channel 3 vector
itvtoff$uar	equ	34h	; UART receiver vector
itvtoff$uat	equ	38h	; UART transmitter vector
itvtoff$ss	equ	3ch	; Single-Step trap vector
itvtoff$boh	equ	40h	; Breakpoint-on-Halt trap vector
itvtoff$dive	equ	44h	; Division Exception trap vector
itvtoff$stkovf equ	48h	; Stack Overflow Warning trap vector
itvtoff$accv	equ	4ch	; Access Violation trap vector
itvtoff$sc	equ	50h	; System Call trap vector
itvtoff$priv	equ	54h	; Privileged Instruction trap vector
itvtoff$mem2epu equ	58h	; EPU <- Memory Extended Instruction trap vector
itvtoff$epu2mem equ	5ch	; Memory <- EPU Extended Instruction trap vector
itvtoff$epu2a	equ	60h	; A <- EPU Extended Instruction trap vector
itvtoff$epuintop equ	64h	; EPU Internal Operation Extended Instr. trap vector
itvtoff$nmiav	equ	70h	; NMI Int A vectors (128 PC values)
itvtoff$ibv	equ	170h	; Int B vectors (128 PC values)
itvtoff$icv	equ	270h	; Int C vectors (128 PC values)

