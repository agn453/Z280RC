
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


#include "unix.h"
#define MAIN

/* This data will be at 0000H */

/* Trap/Interrupt vector table */

/* This must be at a multiple of 4K  */

extern int  	
	bad_int(),
	acia_int(),
	uart_int(),
	clk_int(),
	ill_trap(),
	segv_trap(),
	stk_trap(),
 	unix(),
	uart_T_int(),
	dma3_int();
	fs_init();
	
struct vect vectable[26] = {
#ifdef Z280RC
      /* MSR , service routine. MSR: 0x08 uart receiver, 0x20 uart transmitter,
				     0x40 dma3 (ide EOT), 0x02 clk_int
      */
	{0   , bad_int},
	{0   , fs_init},		/* nmi */
	{0   , bad_int},		/* interrupt line a */
	{0   , bad_int},		/* interrupt line b */
	{0   , bad_int},		/* interrupt line c */
	{0x68, clk_int},	        /* counter/timer 0 vector */ 
	{0   , bad_int},		/*   "    /  "   1   "    */
	{0   , bad_int},		/* reserved */
	{0   , bad_int},		/*   "    /   "  2 vector */
	{0   , bad_int},		/* dma 0  */
	{0   , bad_int},		/* dma 1  */
	{0   , bad_int},		/* dma 2   */
	{0x08, dma3_int},		/* dma 3  */
	{0   , uart_int},		/* uart receiver interrupt */
	{0x08, uart_T_int},		/* uart transmitter interrupt */
	{0   , bad_int},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , stk_trap},
	{0x08, segv_trap},
	{0x08, unix},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , ill_trap}
#else
      /* MSR , service routine. MSR: 0x08 uart receiver, 0x20 uart transmitter,
				     0x40 dma3 (ide EOT), 0x04 clk_int
      */
	{0   , bad_int},
	{0   , fs_init},
	{0x68, acia_int},		/* ECB-bus interrupt line a */
	{0x68, clk_int},		/* clk_int   line b */
	{0   , bad_int},		/* interrupt line c */
	{0   , bad_int},	        /* counter/timer 0 vector */ 
	{0   , bad_int},		/*   "    /  "   1   "    */
	{0   , bad_int},		/* reserved */
	{0   , bad_int},		/*   "    /   "  2 vector */
	{0   , bad_int},		/* dma 0  */
	{0   , bad_int},		/* dma 1  */
	{0   , bad_int},		/* dma 2   */
	{0x08, dma3_int},		/* dma 3  */
	{0   , uart_int},		/* uart receiver interrupt */
	{0x08, uart_T_int},		/* uart transmitter interrupt */
	{0   , bad_int},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , stk_trap},
	{0x08, segv_trap},
	{0x08, unix},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , ill_trap},
	{0   , ill_trap}
#endif
};

#include "extern.h"  /* This declares all the tables, etc */

int stdin, stdout, stderr;  /* Necessary for library, but never referenced */


/* Dispatch table for system calls */
extern int
 	__exit(),
	_open(),
	_close(),
	_creat(),
	_mknod(),
	_link(),
	_unlink(),
	_read(),
	_write(),
	_seek(),
	_chdir(),
	_sync(),
	_access(),
	_chmod(),
	_chown(),
	_stat(),
	_fstat(),
	_dup(),
	_getpid(),
	_getppid(),
	_getuid(),
	_umask(),
	_getfsys(),
	_execve(),
	_wait(),
	_setuid(),
	_setgid(),
	_time(),
	_stime(),
	_ioctl(),
	_brk(),
	_sbrk(),
	_fork(),
	_mount(),
	_umount(),
	_signal(),
	_dup2(),
	_pause(),
	_alarm(),
	_kill(),
	_pipe(),
	_getgid(),
	_times(),
	_utime();
	
int (*disp_tab[])() =
{	__exit,
	_open,
	_close,
	_creat,
	_mknod,
	_link,
	_unlink,
	_read,
	_write,
	_seek,
	_chdir,
	_sync,
	_access,
	_chmod,
	_chown,
	_stat,
	_fstat,
	_dup,
	_getpid,
	_getppid,
	_getuid,
	_umask,
	_getfsys,
	_execve,
	_wait,
	_setuid,
	_setgid,
	_time,
	_stime,
	_ioctl,
	_brk,
	_sbrk,
	_fork,
	_mount,
	_umount,
	_signal,
	_dup2,
	_pause,
	_alarm,
	_kill,
	_pipe,
	_getgid,
	_times,
	_utime
 };

int ncalls = sizeof(disp_tab) / sizeof(int(*)());

