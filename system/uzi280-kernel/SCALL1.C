
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


/*LINTLIBRARY*/
#include "unix.h"
#include "extern.h"



/*******************************************
open(name, flag)
char *name;
register int16 flag;
*********************************************/

#define name (char *)udata.u_argn1
#define flag (int16)udata.u_argn

_open()
{
    int16 uindex;
    register int16 oftindex;
    register inoptr ino;
    register int16 perm;
    inoptr n_open();
    int uf_alloc(),oft_alloc(),getperm(),isdevice(),d_open();
    unsigned getmode();	
	
    if (flag < 0 || flag > 2)
    {
	udata.u_error = EINVAL;
	return (-1);
    }
    if ((uindex = uf_alloc()) == -1)
	return (-1);

    if ((oftindex = oft_alloc()) == -1)
	goto nooft;

    ifnot (ino = n_open(name,NULLINOPTR))
	goto cantopen;

    of_tab[oftindex].o_inode = ino;

    perm = getperm(ino);
    if (((flag == O_RDONLY || flag == O_RDWR) && !(perm & OTH_RD)) ||
        ((flag == O_WRONLY || flag == O_RDWR) && !(perm & OTH_WR)))
    {
	udata.u_error = EPERM;
	goto cantopen;
    }

    if (getmode(ino) == F_DIR &&
	(flag == O_WRONLY || flag == O_RDWR))
    {
	udata.u_error = EISDIR;
	goto cantopen;
    }

    if (isdevice(ino) && d_open((int)ino->c_node.i_addr[0]) != 0)
    {
	udata.u_error = ENXIO;
	goto cantopen;
    }

    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr.o_offset = 0;
    of_tab[oftindex].o_ptr.o_blkno = 0;
    of_tab[oftindex].o_access = flag;

/*
	sleep process if no writer or reader
*/

    if (getmode(ino) == F_PIPE && of_tab[oftindex].o_refs == 1 )
    	psleep(ino);

    return (uindex);

cantopen:
    oft_deref(oftindex);  /* This will call i_deref() */
nooft:
    udata.u_files[uindex] = -1;
    return (-1);
}

#undef name
#undef flag



/*********************************************
close(uindex)
int16 uindex;
**********************************************/

#define uindex (int16)udata.u_argn

_close()
{
	int doclose();

    return(doclose(uindex));
}

#undef uindex


doclose(uindex)
int16 uindex;
{
    register int16 oftindex;
    inoptr ino;
    inoptr getinode();
    int isdevice();
 
    ifnot(ino = getinode(uindex))
	return(-1);
    oftindex = udata.u_files[uindex];

    if ( isdevice(ino)
/*	 && ino->c_refs == 1 && of_tab[oftindex].o_refs == 1 */ )
	d_close((int)(ino->c_node.i_addr[0]));

    udata.u_files[uindex] = -1;
    oft_deref(oftindex);

    return(0);
}



/****************************************
creat(name, mode)
char *name;
int16 mode;
*****************************************/

#define name (char *)udata.u_argn1
#define mode (int16)udata.u_argn

_creat()
{
    register inoptr ino;
    register int16 uindex;
    register int16 oftindex;
    inoptr parent;
    register int16 j;
    inoptr n_open();
    inoptr newfile();
    int    uf_alloc(),oft_alloc(),getperm();
    unsigned getmode();

    char fname[15];

    parent = NULLINODE;

    if ((uindex = uf_alloc()) == -1)
	return (-1);
    if ((oftindex = oft_alloc()) == -1)
	return (-1);

    if (ino = n_open(name,&parent))  /* The file exists */
    {
	i_deref(parent);

	if (getmode(ino) == F_DIR)
	{
	    i_deref(ino);
	    udata.u_error = EISDIR;
	    goto nogood;
	}
  	ifnot (getperm(ino) & OTH_WR)
  	{
  	    i_deref(ino);
  	    udata.u_error = EACCES;
  	    goto nogood;
  	}
  	if (getmode(ino) == F_REG)
	{
	    /* Truncate the file to zero length */
	    f_trunc(ino);
	    /* Reset any oft pointers */
	    for (j=0; j < OFTSIZE; ++j)
		if (of_tab[j].o_inode == ino)
		    of_tab[j].o_ptr.o_blkno = of_tab[j].o_ptr.o_offset = 0;
	}

    }
    else
    {
	/* Get trailing part of name into fname */
	filename(name, fname);
	if ( parent && *fname && (ino = newfile(parent,fname)))
		 /* Parent was derefed in newfile */
	{
	    ino->c_node.i_mode = (F_REG | (mode & MODE_MASK & ~udata.u_mask));
	    setftime(ino, A_TIME|M_TIME|C_TIME);
	    /* The rest of the inode is initialized in newfile() */
	    wr_inode(ino);
	}
	else
	{
	    /* Doesn't exist and can't make it */
/*	    if (ino)
		i_deref(ino);
*/	    goto nogood;
	}
    }

    udata.u_files[uindex] = oftindex;

    of_tab[oftindex].o_ptr.o_offset = 0;
    of_tab[oftindex].o_ptr.o_blkno = 0;
    of_tab[oftindex].o_inode = ino;
    of_tab[oftindex].o_access = O_WRONLY;

    return (uindex);

nogood:
    oft_deref(oftindex);
    return (-1);

}

#undef name
#undef mode



/********************************************
pipe(fildes)
int fildes[];
*******************************************/

#define fildes (int *)udata.u_argn

_pipe()
{
    register int16 u1, u2, oft1, oft2;
    register inoptr ino;
    inoptr i_open();
    int    uf_alloc(),oft_alloc();

/* buf fix SN */
    if ((u1 = uf_alloc()) == -1)
	goto nogood;
    if ((oft1 = oft_alloc()) == -1)
	goto nogood;
    udata.u_files[u1] = oft1;

    if ((u2 = uf_alloc()) == -1)
	goto nogood2;
    if ((oft2 = oft_alloc()) == -1)
	goto nogood2;

    ifnot (ino = i_open(ROOTDEV, 0))
    {
	oft_deref(oft2);
	goto nogood2;
    }

    udata.u_files[u2] = oft2;

    of_tab[oft1].o_ptr.o_offset = 0;
    of_tab[oft1].o_ptr.o_blkno = 0;
    of_tab[oft1].o_inode = ino;
    of_tab[oft1].o_access = O_RDONLY;

    of_tab[oft2].o_ptr.o_offset = 0;
    of_tab[oft2].o_ptr.o_blkno = 0;
    of_tab[oft2].o_inode = ino;
    of_tab[oft2].o_access = O_WRONLY;

    ++ino->c_refs;
    ino->c_node.i_mode = F_PIPE | 0777; /* No permissions necessary on pipes */
    ino->c_node.i_nlink = 0;		/* a pipe is not in any directory */

    uputw(u1, fildes);
    uputw(u2, fildes+1);
    return (0);

nogood2:
    oft_deref(oft1);
    udata.u_files[u1] = -1;
nogood:
    return(-1);

}

#undef fildes



/********************************************
link(name1, name2)
char *name1;
char *name2;
*********************************************/

#define name1 (char *)udata.u_argn1
#define name2 (char *)udata.u_argn

_link()
{
    register inoptr ino;
    register inoptr ino2;
    inoptr parent2;
    inoptr n_open();
    int	   ch_link(),super();
    unsigned getmode();
    char fname[15];
	
    ifnot (ino = n_open(name1,NULLINOPTR))
	return(-1);

/*  if (getmode(ino) == F_DIR && !super()) */
    if(!(getperm(ino) & OTH_WR) && !super())
    {
	udata.u_error = EPERM;
	goto nogood;
    }

    /* Make sure file2 doesn't exist, and get its parent */
    if (ino2 = n_open(name2,&parent2))
    {
	i_deref(ino2);
	i_deref(parent2);
	udata.u_error = EEXIST;
	goto nogood;
    }
    
    ifnot (parent2)
	goto nogood;

    if (ino->c_dev != parent2->c_dev)
    {
	i_deref(parent2);
	udata.u_error = EXDEV;
	goto nogood;
    }

    filename(name2, fname);
    ifnot (*fname && ch_link(parent2,"",fname,ino))
    {
/* BUG SN*/
	i_deref(parent2);
/* !!! */
	goto nogood;
    }

    /* Update the link count. */
    ++ino->c_node.i_nlink;
    wr_inode(ino);
    setftime(ino, C_TIME);

    i_deref(parent2);
    i_deref(ino);
    return(0);

nogood:
    i_deref(ino);
    return(-1);

}

#undef name1
#undef name2



/**********************************************************
unlink(path)
char *path;
**************************************************/

#define path (char *)udata.u_argn

_unlink()
{
    register inoptr ino;
    inoptr pino;
    inoptr n_open();
    int    super(),ch_link();
    unsigned getmode();
    char fname[15];

    ino = n_open(path,&pino);

/* BUG !! 
  resulting in to many i-refs to pino for non existing files.
Fixed SN */
    ifnot (pino && ino)
    {
/* FIX ... */
	if (pino) /* parent exist */
		i_deref(pino);
/* ... FIX end */
	udata.u_error = ENOENT;
	return (-1);
    }


/*  if (getmode(ino) == F_DIR && !super()) */

/* owner of file can overwrite R/O mode
   root has no restrictions
*/ 
    if( ino->c_node.i_uid!=udata.u_euid && !(getperm(ino)&OTH_WR) && !super())
    {
	udata.u_error = EPERM;
	goto nogood;
    }

    /* Remove the directory entry */
    filename(path, fname);
    ifnot (*fname && ch_link(pino,fname,"",NULLINODE))
        goto nogood;

    /* Decrease the link count of the inode */

    ifnot (ino->c_node.i_nlink--)
    {
        ino->c_node.i_nlink += 2;
	warning("_unlink: bad nlink");
    }
    setftime(ino, C_TIME);
    i_deref(pino);
    i_deref(ino);
    return(0);

nogood:
    i_deref(pino);
    i_deref(ino);
    return(-1);
}

#undef path    



/*****************************************************
read(d, buf, nbytes)
int16 d;
char *buf;
uint16 nbytes;
**********************************************/

#define d (int16)udata.u_argn2
#define buf (char *)udata.u_argn1
#define nbytes (uint16)udata.u_argn

_read()
{
    register inoptr ino;
    inoptr rwsetup();

    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(1)) == NULLINODE)
	return (-1);   /* bomb out if error */

    readi(ino);
    updoff();

    return (udata.u_count);
}

#undef d
#undef buf
#undef nbytes


/***********************************
write(d, buf, nbytes)
int16 d;
char *buf;
uint16 nbytes;
***********************************/

#define d (int16)udata.u_argn2
#define buf (char *)udata.u_argn1
#define nbytes (uint16)udata.u_argn

_write()
{
    register inoptr ino;
    inoptr rwsetup();

    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(0)) == NULLINODE)
	return (-1);   /* bomb out if error */

    writei(ino);
    updoff();

    return (udata.u_count);
}

#undef d
#undef buf
#undef nbytes



inoptr
rwsetup(rwflag)
int rwflag;
{
    register inoptr ino;
    register struct oft *oftp;
    inoptr getinode();

    udata.u_sysio = 0;  /* I/O to user data space */
    udata.u_base = (char *)udata.u_argn1;  /* buf */
    udata.u_count = (uint16)udata.u_argn;  /* nbytes */

    if ((ino = getinode(udata.u_argn2)) == NULLINODE)
	return (NULLINODE);

    oftp = of_tab + udata.u_files[udata.u_argn2];
    if (oftp->o_access == (rwflag ? O_WRONLY : O_RDONLY))
    {
	udata.u_error = EBADF;
	return (NULLINODE);
    }

    setftime(ino, rwflag ? A_TIME : (A_TIME | M_TIME | C_TIME));

    /* Initialize u_offset from file pointer */
    udata.u_offset.o_blkno = oftp->o_ptr.o_blkno;
    udata.u_offset.o_offset = oftp->o_ptr.o_offset;

    return (ino);
}



readi(ino)
register inoptr ino;
{
    register uint16 amount;
    register uint16 toread;
    register blkno_t pblk;
    register char *bp;
    int dev;
    int ispipe;
    char *bread();
    char *zerobuf();
    blkno_t bmap();
    unsigned psize(),cdread();

    dev = ino->c_dev;
    ispipe = 0;
    switch (getmode(ino))
    {

    case F_DIR:
    case F_REG:

	/* See of end of file will limit read */
	toread = udata.u_count =
	    ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno >= 64 ?
		udata.u_count :
		min(udata.u_count,
		 512*(ino->c_node.i_size.o_blkno-udata.u_offset.o_blkno) +
		 (ino->c_node.i_size.o_offset-udata.u_offset.o_offset));
	goto loop;

    case F_PIPE:
	ispipe = 1;
	while (psize(ino) == 0)
	{
	    if (ino->c_refs == 1) /* No writers */
		break;
	    /* Sleep if empty pipe */
	    psleep(ino);
	}
	toread = udata.u_count = min(udata.u_count, psize(ino));
	goto loop;

    case F_BDEV:
	toread = udata.u_count;
        dev = *(ino->c_node.i_addr);

    loop:
	while (toread)
	{
	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 1)) != NULLBLK)
		bp = bread(dev, pblk, 0);
	    else
		bp = zerobuf();

	    amount = min(toread, 512 - udata.u_offset.o_offset);

	    if (udata.u_sysio)
		bcopy(bp+udata.u_offset.o_offset, udata.u_base, amount);
	    else
		uput(bp+udata.u_offset.o_offset, udata.u_base, amount);

	    brelse(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if (ispipe && udata.u_offset.o_blkno >= 18)
		udata.u_offset.o_blkno = 0;
	    toread -= amount;
	    if (ispipe)
	    {
		addoff(&(ino->c_node.i_size), -amount);
		wakeup(ino);
	    }
	}

	break;

    case F_CDEV:
	udata.u_count = cdread(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
        break;

    default:
	udata.u_error = ENODEV;
    }
}



/* Writei (and readi) need more i/o error handling */

writei(ino)
register inoptr ino;
{
    register uint16 amount;
    register uint16 towrite;
    register char *bp;
    int ispipe;
    blkno_t pblk;
    int dev;
    char *bread();
    blkno_t bmap();
    unsigned cdwrite(),psize();

    dev = ino->c_dev;

    switch (getmode(ino))
    {

    case F_BDEV:
        dev = *(ino->c_node.i_addr);
    case F_DIR:
    case F_REG:
  	ispipe = 0;
	towrite = udata.u_count;
	goto loop;

    case F_PIPE:
	ispipe = 1;
	while ((towrite = udata.u_count) > (16*512) - psize(ino))
	{
	    if (ino->c_refs == 1) /* No readers */
	    {
		udata.u_count = -1;
		udata.u_error = EPIPE;
		ssig(udata.u_ptab, SIGPIPE);
	        return;
	    }
	    /* Sleep if empty pipe */
	    psleep(ino);
	}

	/* Sleep if empty pipe */
	goto loop;

    loop:

	while (towrite)
	{
	    amount = min(towrite, 512 - udata.u_offset.o_offset);


	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 0)) == NULLBLK)
		break;    /* No space to make more blocks */

	    /* If we are writing an entire block, we don't care
	    about its previous contents */
	    bp = bread(dev, pblk, (amount == 512));

	    if (udata.u_sysio)
		bcopy(udata.u_base, bp+udata.u_offset.o_offset, amount);
	    else
		uget(udata.u_base, bp+udata.u_offset.o_offset, amount);

	    bawrite(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if(ispipe)
	    {
		if (udata.u_offset.o_blkno >= 18) 
		    	udata.u_offset.o_blkno = 0;
		addoff(&(ino->c_node.i_size), amount);
	        /* Wake up any readers */
		wakeup(ino);

	    }
	    towrite -= amount;
	}

	/* Update size if file grew */
	ifnot (ispipe)
	{
	    if ( udata.u_offset.o_blkno > ino->c_node.i_size.o_blkno ||
	        (udata.u_offset.o_blkno == ino->c_node.i_size.o_blkno &&
		    udata.u_offset.o_offset > ino->c_node.i_size.o_offset))
	    {
	        ino->c_node.i_size.o_blkno = udata.u_offset.o_blkno;
	        ino->c_node.i_size.o_offset = udata.u_offset.o_offset;
	        ino->c_dirty = 1;
	    }    
	}

	break;

    case F_CDEV:
	udata.u_count = cdwrite(ino->c_node.i_addr[0]);

	if (udata.u_count != -1)
	    addoff(&udata.u_offset, udata.u_count);
	break;

    default:
	udata.u_error = ENODEV;
    }

}

min(a, b)
int a, b;
{
    return ( a < b ? a : b);
}

psize(ino)
inoptr ino;
{
    return (512*ino->c_node.i_size.o_blkno+ino->c_node.i_size.o_offset);
}



addoff(ofptr, amount)
off_t *ofptr;
int amount;
{
	register int temp;

    if (amount >= 0)
    {
    	temp = amount % 512;
    	ofptr->o_offset += temp;
    	if (ofptr->o_offset >= 512)
    	{
		ofptr->o_offset -= 512;
		++ofptr->o_blkno;
    	}
    	temp = amount / 512;
    	ofptr->o_blkno += temp;
    }
    else
    {
	temp = (-amount) % 512;
        ofptr->o_offset -= temp;
        if (ofptr->o_offset < 0)
        {
	    ofptr->o_offset += 512;
	    --ofptr->o_blkno;
        }
	temp = (-amount) / 512;
        ofptr->o_blkno -= temp;
    }
}


updoff()
{
    register off_t *offp;

    /* Update current file pointer */
    offp = &of_tab[udata.u_files[udata.u_argn2]].o_ptr;
    offp->o_blkno = udata.u_offset.o_blkno;
    offp->o_offset = udata.u_offset.o_offset;
}



/****************************************
seek(file,offset,flag)
int16 file;
int16 offset;
int16 flag;
*****************************************/

#define file (int16)udata.u_argn2
#ifdef _signed_seek_
#define offset (int16)udata.u_argn1
#else
#define offset (uint16)udata.u_argn1
#endif
#define flag (int16)udata.u_argn

_seek()
{
    register inoptr ino;
    register int16 oftno;
    register uint16 retval;
    inoptr getinode();
    unsigned getmode();

    if ((ino = getinode(file)) == NULLINODE)
	return(-1);

    if (getmode(ino) == F_PIPE)
    {
	udata.u_error = ESPIPE;
	return(-1);
    }

    oftno = udata.u_files[file];

/* REMEMBER:
   flag  0-2 return only offset within current block (512 bytes)
	 3-5 return only current block number
*/

    if (flag <= 2)
	retval = of_tab[oftno].o_ptr.o_offset;
    else
	retval = of_tab[oftno].o_ptr.o_blkno;

    switch(flag)
    {
    case 0:
	of_tab[oftno].o_ptr.o_blkno = 0;
	of_tab[oftno].o_ptr.o_offset = offset;
	break;
    case 1:
	of_tab[oftno].o_ptr.o_offset += offset;
	break;
    case 2:
	of_tab[oftno].o_ptr.o_blkno = ino->c_node.i_size.o_blkno;
	of_tab[oftno].o_ptr.o_offset = ino->c_node.i_size.o_offset + offset;
	break;
    case 3:
	of_tab[oftno].o_ptr.o_blkno = offset;
	break;
    case 4:
	of_tab[oftno].o_ptr.o_blkno += offset;
	break;
    case 5:
	of_tab[oftno].o_ptr.o_blkno = ino->c_node.i_size.o_blkno + offset;
	break;
    default:
	udata.u_error = EINVAL;
	return(-1);
    }
    while ((unsigned)of_tab[oftno].o_ptr.o_offset >= 512) {
	of_tab[oftno].o_ptr.o_offset -= 512;
	++of_tab[oftno].o_ptr.o_blkno;
    }


    return((int16)retval);
}

#undef file
#undef offset
#undef flag

#include "scall1b.c"
