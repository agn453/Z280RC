
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
	UZI280 devtty.c  vers 1.02a
*/
/* This one supports the Club80-Terminal at I/O Address 80/81h   SN */

/*  06.04.95 SN
    Bugfix for the window system. Now the background output works
    correct even during keyboard input.
*/

/* added support for REH-HGT terminal. 16.2.95 SN */

   
/* special for club80 terminal	SN
	
	you have 4 virtuell consoles on club80 terminal
	standard is tty1
	also reachable tty2,tty3,tty4

	tty5 is connected to the UART port
	
	tty6 is a tektonix like grafik terminal

	05/07/94	SN
			added TIOCGETC,TIOCSETC for ctrl character setting
			read now returns after one gotten char in RAW and
			CBREAK mode.
*/
#include "unix.h"
#include "extern.h"


#define TIOCGETP 0
#define TIOCSETP 1

#define TIOCSETN  2
/** currently not implemented  SN
#define TIOCEXCL  3
**/
#define UARTSLOW  4	/* normal interrupt routine */
#define UARTFAST  5	/* fast interrupt routine for modem usage */
#define TIOCFLUSH 6
#define TIOCGETC  7
#define TIOCSETC  8
/* 
	the following is not used under UNIX 
	special for patched cpm programms 
*/

#define CPM_CTRL
#ifdef CPM_CTRL
#define TIOCTLSET 9	/* don't parse ctrl-chars */
#define TIOCTLRES 10	/* normal parse */
#endif

#ifdef CLUB80_Terminal
/* some fuctions for window handling and graphic terminal emulation */
define_window(minor,number,xsize,ysize) 
{
                _putc(minor,0x1b);   	_putc(minor,'X');
    		_putc(minor,number+32); 
    		_putc(minor,' ');	_putc(minor,' '); 
    		_putc(minor,xsize+' ');	_putc(minor,ysize+' '); 
    		_putc(minor,' ');  
}
open_window(minor,number) 
{
	_putc(minor,0x1b); _putc(minor,'x'); _putc(minor,number+31);
}
open_background_window(minor,number) 
{
	_putc(minor,0x1b); _putc(minor,'z'); _putc(minor,number+31);
}
open_graphic(minor) 
{
	_putc(minor,0x1b); _putc(minor,0x1b); _putc(minor,'T');
}
close_graphic(minor) 
{
	_putc(minor,0x1b); _putc(minor,0x1b); _putc(minor,'A');
}
#endif


#define XTABS	0006000
#define RAW	0000040
#define CRMOD   0000020
#define ECHO	0000010
#define LCASE	0000004
#define CBREAK	0000002
#define COOKED  0000000

struct tty_data {
    char t_ispeed;
    char t_ospeed;
    char t_erase;
    char t_kill;
    int  t_flags;

    char t_intr;
    char t_quit;
    char t_start;
    char t_stop;
    char t_eof;

    char ctl_char;	
};


struct tty_data ttydata[7];   /* ttydata[0] is not used */

#define CTRL(c)  (c & 0x1f)

struct tty_data ttydflt = {
    0, 0, '\b', CTRL('x'), XTABS|CRMOD|ECHO|COOKED,
    CTRL('c'), CTRL('\\'), CTRL('q'), CTRL('s'), CTRL('d'),0
};

char *dflt_mode = XTABS|CRMOD|ECHO|COOKED;

int cur_win = 1;	/* global var holding input console window number */
int last_win =1;	/*    "       last output window */  
int  fast_mode=0;	/* flag for UART interrupt routine handling */
	

/* character input queue size */
#define TTYSIZ 40 /*132*/
#define BIGSIZ (1024+512) 	/* big buffer for UART serial input line */

char tbuf1[TTYSIZ];
char tbuf2[TTYSIZ];
char tbuf3[TTYSIZ];
char tbuf4[TTYSIZ];
char tbuf5[BIGSIZ];	/* UART line /dev/tty5 */
char tbuf6[TTYSIZ];

struct s_queue ttyinq[7] = {
{
    NULL,	/* ttyinq[0] is never used */
    NULL,
    NULL,
    0,
    0,
    0
},
{
    tbuf1,
    tbuf1,
    tbuf1,
    TTYSIZ,
    0,
    TTYSIZ/2
},
{
    tbuf2,
    tbuf2,
    tbuf2,
    TTYSIZ,
    0,
    TTYSIZ/2
},
{
    tbuf3,
    tbuf3,
    tbuf3,
    TTYSIZ,
    0,
    TTYSIZ/2
},
{
    tbuf4,
    tbuf4,
    tbuf4,
    TTYSIZ,
    0,
    TTYSIZ/2
},
{
    tbuf5,
    tbuf5,
    tbuf5,
    BIGSIZ,
    0,
    BIGSIZ/2
},
{
    tbuf6,
    tbuf6,
    tbuf6,
    TTYSIZ,
    0,
    TTYSIZ/2
}
};

int stopflag[7];  /* Flag for ^S/^Q */
int flshflag[7];   /* Flag for ^O */

tty_read(minor, rawflag)
int16 minor;
int16 rawflag;
{
    int nread;
    char c;

ifnot (minor==5 && fast_mode) {
    /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
	minor = udata.u_ptab->p_tty;
    ifnot (udata.u_ptab->p_tty)
	udata.u_ptab->p_tty = minor;

    if (minor < 1 || minor > 6)
    {
	udata.u_error = ENODEV;
	return (-1);
    }
#ifdef _WIN_SYS_
    if (minor == cur_win && minor < 5)	{
	open_window(minor,minor);
	last_win = minor;
    }
#endif
} /* end of not uart fast mode */

    nread = 0;
    while (nread < udata.u_count)
    {
	for (;;)
	{
	    if (remq(&ttyinq[minor],&c))
		break;
	    psleep(&ttyinq[minor]);
   	    if (udata.u_cursig || udata.u_ptab->p_pending)  /* messy */
	    {
	        udata.u_error = EINTR;
	        return(-1);
	    }
	}

	if (udata.u_sysio)
	    *udata.u_base = c;
	else
	    uputc(c, udata.u_base);

	if (nread++ == 0 && c == ttydata[minor].t_eof)   /* ^D */
	    return(0);

      /* fast mode on dev/tty5 (UART) disables cbreak */ 

      if (minor==5 && fast_mode) {
      /* don't wait for udata.u_count chars just get as much as possible. */
	if (  ttyinq[5].q_count==0 ) break;
      } else {
	 /* in raw or cbreak mode return after one char */
	 if (ttydata[minor].t_flags & (RAW|CBREAK))
		break;
	 if (c == '\n')
	    	break;
      } 
	++udata.u_base;
    } 
    return(nread);
}

tty_write(minor, rawflag)
int16 minor;
int16 rawflag;
{
    int towrite, c;

    /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
	minor = udata.u_ptab->p_tty;
    ifnot (udata.u_ptab->p_tty)
	udata.u_ptab->p_tty = minor;

    if (minor < 1 || minor > 6)
    {
	udata.u_error = ENODEV;
	return (-1);
    }
#ifdef _WIN_SYS_
   if (minor < 5 && minor != last_win)	{
    	if (minor != cur_win)	{
	  open_background_window(cur_win,minor);
    	}
	else {
	  open_window(cur_win,minor);
    	}
	last_win = minor;
    }	
#endif
    towrite = udata.u_count;

    while (udata.u_count-- != 0)
    {
	for (;;)	/* Wait on the ^S/^Q flag */
	{
	    ifnot (stopflag[minor])	
		break;
	    psleep(&stopflag[minor]);
   	    if (udata.u_cursig || udata.u_ptab->p_pending)  /* messy */
	    {
	        udata.u_error = EINTR;
	        return(-1);
	    }
	}
	
	ifnot (flshflag[minor])
	{
	    if (udata.u_sysio)
		c = *udata.u_base;
	    else
		c = ugetc(udata.u_base);

   	    if (c == '\n' && (ttydata[minor].t_flags & CRMOD))
	        _putc(minor, '\r');
	    _putc(minor, c);
	}
	++udata.u_base;
    }
#ifdef _WIN_SYS_
    if (minor < 5 && minor != last_win && minor == cur_win) {
	open_window(minor,minor);
	last_win = cur_win;
    }
#endif
    return(towrite);
}

tty_open(minor)
int minor;
{
	static first = 0;
	register j;

#ifdef _WIN_SYS_
	/*
	on first open initialize the club80 term window system  SN
	*/
    if (first == 0) {
    	for (j=0;j<5;++j) {	
		define_window(20,j,79,24);
    	}
    	first = 1;
	cur_win = 1;
	last_win = cur_win;
	open_window(20,1);	/* open window 1 by default */
    }
#else /* no terminal with window handling */
    	first = 1;
	cur_win = 1;
	last_win = cur_win;
#endif
    /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
	minor = udata.u_ptab->p_tty;

    /* If there is no controlling tty for the process, establish it */
    ifnot (udata.u_ptab->p_tty)
	udata.u_ptab->p_tty = minor;

#ifdef _WIN_SYS_
    if (minor < 1 || minor > 6) {
	udata.u_error = ENODEV;
	return (-1);
    }
#ifdef CLUB80_Terminal
    if (minor == 6) {
    	/* open grafik screen */ 
	open_graphic(minor);
    	/* send stop to text tty1-4 */
    	for (j=1;j<5;j++)
    		stopflag[j] = 1;
    }
#endif
#else

#ifdef REH_HGT
    if (minor != 5 && minor != 1)
#else
    if (minor != 5)
#endif
    {
	udata.u_error = ENODEV;
        return (-1);
    }
#endif	 	
    /* Initialize the ttydata */
    bcopy(&ttydflt, &ttydata[minor], sizeof(struct tty_data));
    return(0);
}


tty_close(minor)
int minor;
{
	register j;

    /* If we are closing the controlling tty, make note */
    if (minor == udata.u_ptab->p_tty)
	udata.u_ptab->p_tty = 0;
#ifdef CLUB80_Terminal
    if (minor == 6) {
    	/* close graphic screen */ 
	close_graphic(minor);
    	/* reset stop on tty1-4 */
    	for (j=1;j<5;j++) {
    		stopflag[j]=0;
    		wakeup(&stopflag[j]);
    	}
    }
#endif
    return(0);
}

tty_ioctl(minor, request, data)
int minor;
int request;
char *data;   /* In user space */
{
   /* Minor == 0 means that it is the controlling tty of the process */
    ifnot (minor)
	minor = udata.u_ptab->p_tty;

    if (minor < 1 || minor > 5)
    {
	udata.u_error = ENODEV;
	return (-1);
    }

    switch (request)
    {
	case TIOCGETP:
	    uput(&ttydata[minor], data, 6);
	    break;
	case TIOCSETP:
	    uget(data, &ttydata[minor], 6);
	    break;
	case TIOCGETC:
	    uput(&ttydata[minor].t_intr, data, 5);
	    break;
	case TIOCSETC:
	    uget(data, &ttydata[minor].t_intr, 5);
	    break;
	case TIOCSETN:
	    uput(&(ttyinq[minor].q_count), data, 2);
	    break;
	case TIOCFLUSH:
	    clrq(&ttydata[minor]);
	    break;
#ifdef CPM_CTRL
	case TIOCTLSET:
	    ttydata[minor].ctl_char = 1;
	    break;	
	case TIOCTLRES:
	    ttydata[minor].ctl_char = 0;
            break;
#endif
	case UARTSLOW:
	     if (minor!=5) return -1;
	     fast_mode=0;
	     break;		
	case UARTFAST:
	     if (minor!=5) return -1;
	     fast_mode=1;
	     break;		
	    
	default:
	    udata.u_error = EINVAL; /* ??? */
	    return(-1);
	    break;
    }
    return (0);
}

/* 
    Here the Club80_terminal is supported
    UART is tty5  SN 
*/
/* This tty interrupt routine checks to see if the uart receiver actually
caused the interrupt.  If so it adds the character to the tty input
queue, echoing and processing backspace and carriage return.  If the queue 
contains a full line, it wakes up anything waiting on it.  If it is totally
full, it beeps at the user. */

tinproc(minor, c)
int minor;
int c;
{
    register int temp_win;
    char oc;
    struct tty_data *td;
    int mode;

    td = &ttydata[minor];	
    mode = td->t_flags & (RAW|CBREAK);

  if(minor!=5) {
    if (c == 0x1b && td->t_flags == dflt_mode)
		return; 	/* my terminal hate it SN */ 

    if (mode != RAW)
    {
	c &= 0x7f;   /* Strip off parity */
	if (!c)
	    return;
    }
#ifdef _WIN_SYS_
    if (minor < 5) {
	open_window(cur_win,cur_win);
    }
#endif
    temp_win = last_win;
    last_win = cur_win;

/**    if (c==0x18) /* ^X */
/**	idump();	/* For debugging */

  } /* not minor 5 */
 
    if (mode == COOKED)
    {
#ifdef CPM_CTRL
    if (ttydata[minor].ctl_char ==0 ) /* don't parse ctl chars */
    {
#endif
    	if (c == td->t_erase)
    	{
	    if (uninsq(&ttyinq[minor],&oc))
	    {
		if (oc == '\n')
		    insq(&ttyinq[minor],oc);  /* Don't erase past nl */
	    	else
	     	{
		    echo(minor, '\b');
		    echo(minor, ' ');
		    echo(minor, '\b');
	    	}
	    }
	    return;
	}

	if (c == td->t_kill)
    	{
	    while (uninsq(&ttyinq[minor],&oc))
	    {
		if (oc == '\n')
		{
		    insq(&ttyinq[minor],oc);  /* Don't erase past nl */
		    break;
		}
		echo(minor, '\b');
		echo(minor, ' ');
		echo(minor, '\b');
	    }
	    return;
	}
#ifdef CPM_CTRL
    }
#endif
    }
#ifdef CPM_CTRL
    if (ttydata[minor].ctl_char==0) /* don't parse ctl chars */
    {
#endif
    if (mode == COOKED || mode == CBREAK)
    {

	if (c == '\r' && (td->t_flags & CRMOD))
	    c = '\n';

	if (c == td->t_intr)  /* ^C */
	{
	    sgrpsig(minor, SIGQUIT);
  	    clrq(&ttyinq[minor]);
	    stopflag[minor] = 
	    flshflag[minor] = 0;
 	    return;
	}

	if (c == td->t_quit)/* ^\ */
	{
	    sgrpsig(minor, SIGINT);
  	    clrq(&ttyinq[minor]);
	    stopflag[minor] = 
	    flshflag[minor] = 0;
	    return;
	}

	if (c == '\017' )  /* ^O */
	{
	    flshflag[minor] = !flshflag[minor];
	    return;
	}

	if (c == td->t_stop )   /* ^S */
	{
	    stopflag[minor] = 1;
	    return;
	}

	if (c == td->t_start)  /* ^Q */
	{
	    stopflag[minor] = 0;
	    wakeup(&stopflag[minor]);
	    return;
	}
    }
#ifdef CPM_CTRL
    }
#endif
    /* All modes come here */

    if (c == '\n' && (td->t_flags & CRMOD))
	echo(minor, '\r');

    if (insq(&ttyinq[minor],c))
        echo(minor, c);
    else
	_putc(minor, '\007');	/* Beep if no more room */

 if(minor!=5) {
    last_win = temp_win;

#ifdef _WIN_SYS_
    if (cur_win != last_win) {
	open_background_window(cur_win,last_win);
    }    
#endif
  } /* not minor 5 */

    if ((mode != COOKED || c == '\n' || c == td->t_eof) 
#ifdef CPM_CTRL
   	                && ttydata[minor].ctl_char ==0
#endif
			) /* ^D */
	wakeup(&ttyinq[minor]);
}

echo(minor, c)
int minor;
int c;
{
    if (ttydata[minor].t_flags & ECHO)
	_putc(minor, c);
}

_putc(minor, c)
char c;
{
    if (minor == 5 ) {
	/* Z280 internal UART */
	putuart(c);
	return(0);
    } 

#ifdef CLUB80_Terminal
    else {
        /* Club80 Terminal */
	while((in(0x80)&0x01) ==  0x01);
	out(c,0x81);
	return(0);
    }
#endif

#ifdef REH_HGT
    else {
	while((in(0x61)&0x04) == 0) ; 
	out(c,0x60);
	return(0);
    } 
#endif
}
