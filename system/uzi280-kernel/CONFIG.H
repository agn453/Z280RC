/* enable Z280RC modifications - Tony Nicholson 12-Dec-2018 */

#define Z280RC 

#ifdef Z280RC
#define POLLED_UART_TX		/* problems with UART Tx interrupts */
#endif

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
	UZI280 config.h  vers 1.02a
*/

#ifdef Z280RC

#define MB2	/* Use all 2MB RAM on Z280RC */

/* Also no CLUB80_Terminal or REH_HGT devices - only the console UART */

#else

/* Config for REH-CPU280 with 1 or 2Mbyte RAM 

/* choose your memory configuration */
#define MB2		/* more than 2 MB RAM not used in this verion */
/*#define MB1*/

2 MB :
   has   1.5M byte 	process space (64Kbyte used by kernel) 
   and   0.5M byte      read/write cache
1 MB :
   has   768 Kbyte 	process space (64Kbyte used by kernel) 
   and   232 Kbyte      read/write cache
*/	

/* 
  Terminal setting (ECB-bus terminals). Undef both for general serial
  terminal I/O only. 
  Note: 
  	With a bus terminal connected the UART serial line
	drives a second terminal.

  For adaption of other bus terminals take a look at
   _putc() in devtty.c and _pull_tty in devttyas.c.
*/ 

#define CLUB80_Terminal*/	/* this Terminal has virtual console
				   and tektonix graphic capacity */
#define REH_HGT 		/* For RehDesign HGT terminal    */

#endif /* Z280RC */

/* 
  Define if terminal can't generate an interrupt on char input I/O.
  If undefined take a look into the file devttyas.c for interrupt
  handler routine.
*/  

#define no_tty_interrupt


#ifdef CLUB80_Terminal
#undef  no_bus_tty 
#undef no_tty_interrupt
#define BOOT_TTY 6		/* 4 text terminals and one graphic screen */
#define _WIN_SYS_		/* on the club80 terminal */	
#else
#undef _WIN_SYS_
#ifdef REH_HGT			
#undef  no_bus_tty
#define BOOT_TTY 6
#else /* no bus terminal */  
#define no_bus_tty
#define BOOT_TTY 13		/* only one serial TTY controlled by */
#endif /*REH_HGT*/				/* the cpu uart */
#endif /*CLUB80_Terminal*/


/* Remake devio.c when this is changed */
#ifdef STEFAN

extern dev_tab[];

#endif

#ifdef DEVIO

extern wd_open(), wd_read(),wd_write();
extern fd_open(), fd_read(),fd_write();
extern tty_open(), tty_close(), tty_read(),tty_write(), tty_ioctl();
extern lpr_open(), lpr_close(), lpr_write();
extern kmem_read(),kmem_write();
extern mt_read(), mt_write(), mt_open(), mt_close();
extern null_write();

static struct devsw dev_tab[] =  /* The device driver switch table */
{
    { 0, wd_open, ok,       wd_read, wd_write, nogood },	/* 0 disk0 */	
    { 0, fd_open, ok,       fd_read, fd_write, nogood },	/* 1 floppy */
    { 1, wd_open, ok,       wd_read, wd_write, nogood },	/* 2 disk1 */
    { 2, wd_open, ok,       wd_read, wd_write, nogood },	/* 3 Swap */
    { 0, lpr_open, lpr_close, nogood, lpr_write, nogood},	/* 4 printer */
    { 0, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 5 tty */
    { 1, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 6 tty1 */
    { 2, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 7 tty2 */
    { 0, ok, ok, ok, null_write, nogood },			/* 8 null */
    { 0, ok, ok, kmem_read, kmem_write, nogood },		/* 9 kmem */
    { 0, mt_open, mt_close, mt_read, mt_write, nogood },	/* 10 mt */
    { 3, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 11 tty3 */
    { 4, tty_open, tty_close, tty_read, tty_write, tty_ioctl }, /* 12 tty4 */
    { 5, tty_open, tty_close, tty_read, tty_write, tty_ioctl },	/* 13 tty5(UART)*/
    { 6, tty_open, tty_close, tty_read, tty_write, tty_ioctl }	/* 14 tty6 */
};

#endif

#ifndef CACHE

#define NDEVS   4    /* Devices 0..NDEVS-1 are capable of being mounted */

#define TTYDEV  BOOT_TTY    /* Device used by kernel for messages, panics */
		     /* Also initial process controlling tty */

#define SWAPDEV 3    /* Device for swapping. (not implemented) */

#define NBUFS  38    /* Number of block buffers ( 20KByte ). Must be
			at least 8 more than number of process tables (30) */
#endif

#ifdef MACHDEP2

/* some system variables for command ps. */

unsigned char word_alignment=0;	/*AGN*/
unsigned ptab_pointer=0;
unsigned number_of_pages=0;
unsigned proc_tab_size=PTABSIZE;
unsigned reserved_1=0;
unsigned reserved_2=0;

/* address tables for virtual user-ram management */ 

struct pg_descr pgtab[] = {

#ifdef Z280RC
	/* 2MB RAM contiguous from 0x000000..0x1FFFF */
	0x0000, 0, 0,
	0x0020, 0, 0,
	0x0040, 0, 0,
	0x0060, 0, 0,
	0x0080, 0, 0,
	0x00a0, 0, 0,
	0x00c0, 0, 0,
	0x00e0, 0, 0,
	/* System uses 0x010000..0x01FFFF */
        0x0200, 0, 0,   
	0x0220, 0, 0,
	0x0240, 0, 0,
	0x0260, 0, 0,
	0x0280, 0, 0,
	0x02a0, 0, 0,
	0x02c0, 0, 0,
	0x02e0, 0, 0,
	0x0300, 0, 0,
	0x0320, 0, 0,
	0x0340, 0, 0,
	0x0360, 0, 0,
	0x0380, 0, 0,
	0x03a0, 0, 0,
	0x03c0, 0, 0,
	0x03e0, 0, 0,
	0x0400, 0, 0,
	0x0420, 0, 0,
	0x0440, 0, 0,
	0x0460, 0, 0,
	0x0480, 0, 0,
	0x04a0, 0, 0,
	0x04c0, 0, 0,
	0x04e0, 0, 0,
	0x0500, 0, 0,
	0x0520, 0, 0,
	0x0540, 0, 0,
	0x0560, 0, 0,
	0x0580, 0, 0,
	0x05a0, 0, 0,
	0x05c0, 0, 0,
	0x05e0, 0, 0,
	0x0600, 0, 0,
	0x0620, 0, 0,
	0x0640, 0, 0,
	0x0660, 0, 0,
	0x0680, 0, 0,
	0x06a0, 0, 0,
	0x06c0, 0, 0,
	0x06e0, 0, 0,
	0x0700, 0, 0,
	0x0720, 0, 0,
	0x0740, 0, 0,
	0x0760, 0, 0,
	0x0780, 0, 0,
	0x07a0, 0, 0,
	0x07c0, 0, 0,
	0x07e0, 0, 0,
	0x0800, 0, 0,
	0x0820, 0, 0,
	0x0840, 0, 0,
	0x0860, 0, 0,
	0x0880, 0, 0,
	0x08a0, 0, 0,
	0x08c0, 0, 0,
	0x08e0, 0, 0,
	0x0900, 0, 0,
	0x0920, 0, 0,
	0x0940, 0, 0,
	0x0960, 0, 0,
	0x0980, 0, 0,
	0x09a0, 0, 0,
	0x09c0, 0, 0,
	0x09e0, 0, 0,
	0x0a00, 0, 0,
	0x0a20, 0, 0,
	0x0a40, 0, 0,
	0x0a60, 0, 0,
	0x0a80, 0, 0,
	0x0aa0, 0, 0,
	0x0ac0, 0, 0,
	0x0ae0, 0, 0,
	0x0b00, 0, 0,
	0x0b20, 0, 0,
	0x0b40, 0, 0,
	0x0b60, 0, 0,
	0x0b80, 0, 0,
	0x0ba0, 0, 0,
	0x0bc0, 0, 0,
#else
	/* REH280 system */
	0x8000, 0, 0,
	0x8020, 0, 0,
	0x8040, 0, 0,
	0x8060, 0, 0,
	0x8080, 0, 0,
	0x80a0, 0, 0,
	0x80c0, 0, 0,
	0x80e0, 0, 0,
        0x8200, 0, 0,   
	0x8220, 0, 0,
	0x8240, 0, 0,
	0x8260, 0, 0,
	0x8280, 0, 0,
	0x82a0, 0, 0,
	0x82c0, 0, 0,
	0x82e0, 0, 0,
	0x8300, 0, 0,
	0x8320, 0, 0,
	0x8340, 0, 0,
	0x8360, 0, 0,
	0x8380, 0, 0,
	0x83a0, 0, 0,
	0x83c0, 0, 0,
	0x83e0, 0, 0,
	0x8400, 0, 0,
	0x8420, 0, 0,
	0x8440, 0, 0,
	0x8460, 0, 0,
	0x8480, 0, 0,
	0x84a0, 0, 0,
	0x84c0, 0, 0,
	0x84e0, 0, 0,
	0x8500, 0, 0,
	0x8520, 0, 0,
	0x8540, 0, 0,
	0x8560, 0, 0,
	0x8580, 0, 0,
	0x85a0, 0, 0,
	0x85c0, 0, 0,
	0x85e0, 0, 0,
	0x8600, 0, 0,
	0x8620, 0, 0,
	0x8640, 0, 0,
	0x8660, 0, 0,
	0x8680, 0, 0,
	0x86a0, 0, 0,
	0x86c0, 0, 0,
	0x86e0, 0, 0,
	0x8700, 0, 0,
	0x8720, 0, 0,
	0x8740, 0, 0,
	0x8760, 0, 0,
	0x8780, 0, 0,
	0x87a0, 0, 0,
	0x87c0, 0, 0,
	0x87e0, 0, 0,
	0x8800, 0, 0,
	0x8820, 0, 0,
	0x8840, 0, 0,
	0x8860, 0, 0,
	0x8880, 0, 0,
	0x88a0, 0, 0,
	0x88c0, 0, 0,
	0x88e0, 0, 0,
	0x8900, 0, 0,
	0x8920, 0, 0,
	0x8940, 0, 0,
	0x8960, 0, 0,
	0x8980, 0, 0,
	0x89a0, 0, 0,
	0x89c0, 0, 0,
	0x89e0, 0, 0,
	0x8a00, 0, 0,
	0x8a20, 0, 0,
	0x8a40, 0, 0,
	0x8a60, 0, 0,
	0x8a80, 0, 0,
	0x8aa0, 0, 0,
	0x8ac0, 0, 0,
	0x8ae0, 0, 0,
	0x8b00, 0, 0,
	0x8b20, 0, 0,
	0x8b40, 0, 0,
	0x8b60, 0, 0,
	0x8b80, 0, 0,
	0x8ba0, 0, 0,
	0x8bc0, 0, 0,
#endif

#ifdef MB1
#ifdef Z280RC
	0x0be0, 0, 0
#else
	0x8be0, 0, 0
#endif
};
#endif

#ifdef MB2
#ifdef Z280RC
	0x0be0, 0, 0,
	0x0c00, 0, 0,
	0x0c20, 0, 0,
	0x0c40, 0, 0,
	0x0c60, 0, 0,
	0x0c80, 0, 0,
	0x0ca0, 0, 0,
	0x0cc0, 0, 0,
	0x0ce0, 0, 0,
	0x0d00, 0, 0,
	0x0d20, 0, 0,
	0x0d40, 0, 0,
	0x0d60, 0, 0,
	0x0d80, 0, 0,
	0x0da0, 0, 0,
	0x0dc0, 0, 0,
	0x0de0, 0, 0,
	0x0e00, 0, 0,
	0x0e20, 0, 0,
	0x0e40, 0, 0,
	0x0e60, 0, 0,
	0x0e80, 0, 0,
	0x0ea0, 0, 0,
	0x0ec0, 0, 0,
	0x0ee0, 0, 0,
	0x0f00, 0, 0,
	0x0f20, 0, 0,
	0x0f40, 0, 0,
	0x0f60, 0, 0,
	0x0f80, 0, 0,
	0x0fa0, 0, 0,
	0x0fc0, 0, 0,
	0x0fe0, 0, 0,
	0x1000, 0, 0,
	0x1020, 0, 0,
	0x1040, 0, 0,
	0x1060, 0, 0,
	0x1080, 0, 0,
	0x10a0, 0, 0,
	0x10c0, 0, 0,
	0x10e0, 0, 0,
	0x1100, 0, 0,
	0x1120, 0, 0,
	0x1140, 0, 0,
	0x1160, 0, 0,
	0x1180, 0, 0,
	0x11a0, 0, 0,
	0x11c0, 0, 0,
	0x11e0, 0, 0,
        0x1200, 0, 0,   
	0x1220, 0, 0,
	0x1240, 0, 0,
	0x1260, 0, 0,
	0x1280, 0, 0,
	0x12a0, 0, 0,
	0x12c0, 0, 0,
	0x12e0, 0, 0,
	0x1300, 0, 0,
	0x1320, 0, 0,
	0x1340, 0, 0,
	0x1360, 0, 0,
	0x1380, 0, 0,
	0x13a0, 0, 0,
	0x13c0, 0, 0,
	0x13e0, 0, 0,
	0x1400, 0, 0,
	0x1420, 0, 0,
	0x1440, 0, 0,
	0x1460, 0, 0,
	0x1480, 0, 0,
	0x14a0, 0, 0,
	0x14c0, 0, 0,
	0x14e0, 0, 0,
	0x1500, 0, 0,
	0x1520, 0, 0,
	0x1540, 0, 0,
	0x1560, 0, 0,
	0x1580, 0, 0,
	0x15a0, 0, 0,
	0x15c0, 0, 0,
	0x15e0, 0, 0,
	0x1600, 0, 0,
	0x1620, 0, 0,
	0x1640, 0, 0,
	0x1660, 0, 0,
	0x1680, 0, 0,
	0x16a0, 0, 0,
	0x16c0, 0, 0,
	0x16e0, 0, 0,
	0x1700, 0, 0,
	0x1720, 0, 0,
	0x1740, 0, 0,
	0x1760, 0, 0,
	0x1780, 0, 0,
	0x17a0, 0, 0,
	0x17c0, 0, 0,
	0x17e0, 0, 0
	/* Cache uses 0x18000..0x1FFFFF */
#else
	0x8be0, 0, 0,
	0x8c00, 0, 0,
	0x8c20, 0, 0,
	0x8c40, 0, 0,
	0x8c60, 0, 0,
	0x8c80, 0, 0,
	0x8ca0, 0, 0,
	0x8cc0, 0, 0,
	0x8ce0, 0, 0,
	0x8d00, 0, 0,
	0x8d20, 0, 0,
	0x8d40, 0, 0,
	0x8d60, 0, 0,
	0x8d80, 0, 0,
	0x8da0, 0, 0,
	0x8dc0, 0, 0,
	0x8de0, 0, 0,
	0x8e00, 0, 0,
	0x8e20, 0, 0,
	0x8e40, 0, 0,
	0x8e60, 0, 0,
	0x8e80, 0, 0,
	0x8ea0, 0, 0,
	0x8ec0, 0, 0,
	0x8ee0, 0, 0,
	0x8f00, 0, 0,
	0x8f20, 0, 0,
	0x8f40, 0, 0,
	0x8f60, 0, 0,
	0x8f80, 0, 0,
	0x8fa0, 0, 0,
	0x8fc0, 0, 0,
	0x8fe0, 0, 0,
	0x9000, 0, 0,
	0x9020, 0, 0,
	0x9040, 0, 0,
	0x9060, 0, 0,
	0x9080, 0, 0,
	0x90a0, 0, 0,
	0x90c0, 0, 0,
	0x90e0, 0, 0,
	0x9100, 0, 0,
	0x9120, 0, 0,
	0x9140, 0, 0,
	0x9160, 0, 0,
	0x9180, 0, 0,
	0x91a0, 0, 0,
	0x91c0, 0, 0,
	0x91e0, 0, 0,
        0x9200, 0, 0,   
	0x9220, 0, 0,
	0x9240, 0, 0,
	0x9260, 0, 0,
	0x9280, 0, 0,
	0x92a0, 0, 0,
	0x92c0, 0, 0,
	0x92e0, 0, 0,
	0x9300, 0, 0,
	0x9320, 0, 0,
	0x9340, 0, 0,
	0x9360, 0, 0,
	0x9380, 0, 0,
	0x93a0, 0, 0,
	0x93c0, 0, 0,
	0x93e0, 0, 0,
	0x9400, 0, 0,
	0x9420, 0, 0,
	0x9440, 0, 0,
	0x9460, 0, 0,
	0x9480, 0, 0,
	0x94a0, 0, 0,
	0x94c0, 0, 0,
	0x94e0, 0, 0,
	0x9500, 0, 0,
	0x9520, 0, 0,
	0x9540, 0, 0,
	0x9560, 0, 0,
	0x9580, 0, 0,
	0x95a0, 0, 0,
	0x95c0, 0, 0,
	0x95e0, 0, 0,
	0x9600, 0, 0,
	0x9620, 0, 0,
	0x9640, 0, 0,
	0x9660, 0, 0,
	0x9680, 0, 0,
	0x96a0, 0, 0,
	0x96c0, 0, 0,
	0x96e0, 0, 0,
	0x9700, 0, 0,
	0x9720, 0, 0,
	0x9740, 0, 0,
	0x9760, 0, 0,
	0x9780, 0, 0,
	0x97a0, 0, 0,
	0x97c0, 0, 0,
	0x97e0, 0, 0
#endif
};
#endif /* 2MB */

int npages = sizeof(pgtab) / sizeof(struct pg_descr);

#endif

#ifdef CACHE

#ifdef Z280RC
/* 
   read cache from mem. adr. 180000h to 1fffffh (model 2MB)
*/
#else
/* 
   read cache from mem. adr. 980000h to 9fffffh (model 2MB)
   or 			     8c0000h to 8fffffh (  "   1MB)
*/
#endif

#ifdef MB1
#ifdef Z280RC
#define C_Start (0x0C00)
#define max_cache  ((0x0ff-0x0C0)*8) 
#else
#define C_Start (0x8C00)
#define max_cache  ((0x8ff-0x8C0)*8) 
#endif
#endif

#ifdef MB2
#ifdef Z280RC
#define C_Start (0x1800)
#define max_cache  ((0x1ff-0x180)*8) /* 1016 */
#else
#define C_Start (0x9800)
#define max_cache  ((0x9ff-0x980)*8) /* 1016 */
#endif
#endif

struct ca_buf cach_buf[max_cache];

#endif /* cache */
