
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


static int ok(), nogood();
#define DEVIO

#include "unix.h"
#include "extern.h"

/* Buffer pool management */

/********************************************************
 The high-level interface is through bread() and bfree().
 Bread() is given a device and block number, and a rewrite flag.
 If rewrite is 0, the block is actually read if it is not already
 in the buffer pool. If rewrite is set, it is assumed that the caller
 plans to rewrite the entire contents of the block, and it will
 not be read in, but only have a buffer named after it.

 Bfree() is given a buffer pointer and a dirty flag.
 If the dirty flag is 0, the buffer is made available for further 
 use.  If the flag is 1, the buffer is marked "dirty", and
 it will eventually be written out to disk.  If the flag is 2,
 it will be immediately written out.

 Zerobuf() returns a buffer of zeroes not belonging to any
 device.  It must be bfree'd after use, and must not be
 dirty. It is used when a read() wants to read an unallocated
 block of a file.

 Bufsync() write outs all dirty blocks.

 Note that a pointer to a buffer structure is the same as a
 pointer to the data.  This is very important.

 ********************************************************/

/*static*/ uint16 bufclock = 0;  /* Time-stamp counter for LRU */

char *
bread( dev, blk, rewrite)
int dev;
blkno_t blk;
int rewrite;
{
    register bufptr bp;
    bufptr bfind();
    bufptr freebuf();

    if (bp = bfind(dev, blk))
    {
	if (bp->bf_busy)
	    panic("want busy block");
	goto done;
    }
    bp = freebuf();

    bp->bf_dev = dev;
    bp->bf_blk = blk;

    /* If rewrite is set, we are about to write over the entire block,
       so we don't need the previous contents */

    ifnot (rewrite)
	if (bdread(bp) == -1)
	{
	    udata.u_error = EIO;
	    return (NULL);
	}

done:
    bp->bf_busy = 1;
    bp->bf_time = ++bufclock;  /* Time stamp it */
    return (bp->bf_data);
}



brelse(bp)
bufptr bp;
{
    bfree(bp, 0);
}

bawrite(bp)
bufptr bp;
{
    bfree(bp, 1);
}



bfree(bp, dirty)
register bufptr bp;
int dirty;
{

    bp->bf_dirty |= dirty;
    bp->bf_busy = 0;

    if (dirty == 2)   /* Extra dirty */
    {
	if (bdwrite(bp) == -1)
	    udata.u_error = EIO;
	bp->bf_dirty = 0;
	return (-1);
    }
    return (0);
}


/* This returns a busy block not belonging to any device, with
 garbage contents.  It is essentially a malloc for the kernel.
 Free it with brelse()! */

char *
tmpbuf()
{
    bufptr bp;
    bufptr freebuf();

    bp = freebuf();
    bp->bf_dev = -1;
    bp->bf_busy = 1;
    bp->bf_time = ++bufclock;  /* Time stamp it */
    return(bp->bf_data);
}

char *
zerobuf()
{
    char *b;

    b = tmpbuf();
    bzero(b,512);
    return(b);
}


bufsync()
{
    register bufptr bp;

    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
	if (bp->bf_dev != -1 && bp->bf_dirty)
          {
	    bdwrite(bp);
            if (!bp->bf_busy)
               bp->bf_dirty = 0;
          }
    }
}

#ifndef ASM_BUFIO

bufptr
bfind(dev, blk)
int dev;
blkno_t blk;
{
    register bufptr bp;

    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
	if (bp->bf_dev == dev && bp->bf_blk == blk)
	    return (bp);
    }

    return (NULL);
}


bufptr
freebuf()
{
    register bufptr bp;
    register bufptr oldest;
    register uint16 oldtime;
    int i;	
    /* Try to find a non-busy buffer 
    and write out the data if it is dirty */

    oldest = NULL;
    oldtime = 0;
    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
 	if ((bufclock - bp->bf_time) >= oldtime && !bp->bf_busy)
	{
	    oldest = bp;
	    oldtime = bufclock - bp->bf_time;
	}
    }
    ifnot (oldest)
        panic("no free buffers");
    
    if (oldest->bf_dirty)
    {
/*	bufsync(); /* SN Testing */
	if (bdwrite(oldest) == -1)
	    udata.u_error = EIO;
	oldest->bf_dirty = 0;
    }
    return (oldest);
}
#endif	

bufinit()
{
    register bufptr bp;

    for (bp=bufpool; bp < bufpool+NBUFS; ++bp)
    {
	bp->bf_dev = -1;
    }
    init_cache();	
}


bufdump()
{
    register bufptr j;

    kprintf("\ndev\tblock\tdirty\tbusy\ttime clock %d\n", bufclock);
    for (j=bufpool; j < bufpool+NBUFS; ++j)
	kprintf("%d\t%u\t%d\t%d\t%u\n",
	    j->bf_dev,j->bf_blk,j->bf_dirty,j->bf_busy,j->bf_time);
}


/***************************************************
 Bdread() and bdwrite() are the block device interface routines.
 they are given a buffer pointer, which contains the device, block number,
 and data location.
 They basically validate the device and vector the call.

 Cdread() and cdwrite are the same for character (or "raw") devices,
 and are handed a device number.
 Udata.u_base, count, and offset have the rest of the data.
 ****************************************************/

bdread(bp)
bufptr bp;
{
    ifnot (validdev(bp->bf_dev))
	panic("bdread: invalid dev");

    udata.u_buf = bp;
    return((*dev_tab[bp->bf_dev].dev_read)(dev_tab[bp->bf_dev].minor, 0));
}


bdwrite(bp)
bufptr bp;
{
   ifnot (validdev(bp->bf_dev))
	panic("bdwrite: invalid dev");

    udata.u_buf = bp;
    return((*dev_tab[bp->bf_dev].dev_write)(dev_tab[bp->bf_dev].minor, 0));
}


cdread(dev)
int dev;
{
    ifnot (validdev(dev))
	panic("cdread: invalid dev");
    return ((*dev_tab[dev].dev_read)(dev_tab[dev].minor, 1));
}

cdwrite(dev)
int dev;
{
    ifnot (validdev(dev))
	panic("cdwrite: invalid dev");
    return ((*dev_tab[dev].dev_write)(dev_tab[dev].minor, 1));
}


pageread(dev, blkno, page)
int dev;
blkno_t blkno;
pageaddr page;
{
    swapbase = page;
    swapblk = blkno;
#ifndef _ID_
    swapcnt = 4096;
#else
    swapcnt = 8192;
#endif
    return ((*dev_tab[dev].dev_read)(dev_tab[dev].minor, 2));
}


pagewrite(dev, blkno, page)
int dev;
blkno_t blkno;
pageaddr page;
{
    swapbase = page;
    swapblk = blkno;
#ifndef _ID_
    swapcnt = 4096;
#else
    swapcnt = 8192;
#endif
    return ((*dev_tab[dev].dev_write)(dev_tab[dev].minor, 2));
}


/**************************************************
The device driver read and write routines now have
only two arguments, minor and rawflag.  If rawflag is
zero, a single block is desired, and the necessary data
can be found in udata.u_buf.
Otherwise, a "raw" or character read is desired, and
udata.u_offset, udata.u_count, and udata.u_base
should be consulted instead.
Any device other than a disk will have only raw access.
*****************************************************/



d_open(dev)
int dev;
{
    ifnot (validdev(dev))
	return(-1);
    return ((*dev_tab[dev].dev_open)(dev_tab[dev].minor));
}


d_close(dev)
int dev;
{
    ifnot (validdev(dev))
	panic("d_close: bad device");
    (*dev_tab[dev].dev_close)(dev_tab[dev].minor);
}



d_ioctl(dev,request,data)
int dev;
int request;
char *data;
{
    ifnot (validdev(dev))
    {
	udata.u_error = ENXIO;
	return(-1);
    }
    if((*dev_tab[dev].dev_ioctl)(dev_tab[dev].minor,request,data))
    {
	udata.u_error = EINVAL;
	return(-1);
    }
	return(0);
}


static 
ok()
{
    return(0);
}


static 
nogood()
{
    return(-1);
}


validdev(dev)
{
    return(dev >= 0 && dev < (sizeof(dev_tab)/sizeof(struct devsw)));
}


/*************************************************************
Character queue management routines
************************************************************/


/* add something to the tail */
insq(q,c)
register struct s_queue *q;
char c;
{
    int oldspl;

    oldspl = spl(0);

/* don't do count check -> count any chars regardless if fit into buffer or not
*/
    if (q->q_count == q->q_size)
    {
	spl(oldspl);
	return(0);
    }

    *(q->q_tail) = c;
    ++q->q_count;
    if (++q->q_tail >= q->q_base + q->q_size)
	q->q_tail = q->q_base;
    spl(oldspl);
    return(1);
}

/* Remove something from the head. */
remq(q,cp)
register struct s_queue *q;
char *cp;
{
    int oldspl;

    oldspl = spl(0);

    ifnot (q->q_count)
    {
	spl(oldspl);
	return(0);
    }
    *cp = *(q->q_head);
    --q->q_count;
    if (++q->q_head >= q->q_base + q->q_size)
	q->q_head = q->q_base;
    spl(oldspl); 
    return(1);
}


clrq(q)
register struct s_queue *q;
{
    int oldspl;

    oldspl = spl(0);

    q->q_head = q->q_tail = q->q_base;
    q->q_count = 0;
    spl(oldspl); 
}


/* Remove something from the tail; the most recently added char. */
uninsq(q,cp)
register struct s_queue *q;
char *cp;
{
    int oldspl;

    oldspl = spl(0);
    ifnot (q->q_count)
    {
	spl(oldspl); 
	return(0);
    }
    --q->q_count;
    if (--q->q_tail < q->q_base)
	q->q_tail = q->q_base + q->q_size - 1;
    *cp = *(q->q_tail);
    spl(oldspl); 
    return(1);
}


/* Returns true if the queue has more characters than its wakeup number */
fullq(q)
struct s_queue *q;
{
    int oldspl;

    oldspl = spl(8);

    if (q->q_count > q->q_wakeup)
    {
	spl(oldspl);
	return (1);
    }
    spl(oldspl);
    return (0);
}
