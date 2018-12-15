
/*-
 * All UZI280 source code is 
 * (c) Copyright (1990-95) by Stefan Nitschke and Doug Braun.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, (2) it is not used for military purpose in
 * any form, (3) it is not used for any commercial purpose, and (4) 
 * distributions including binaries display the following acknowledgement:
 * ``This product includes software developed by Stefan Nitschke 
 *   and his contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the author nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */


/*LINTLIBRARY*/
#include "unix.h"
#include "extern.h"

char *bread();


xfs_init(bootdev)
int bootdev;
{
    register char *j;
    inoptr i_open();

    fs_init();
    bufinit();

    /* User's file table */
    for (j=udata.u_files; j < (udata.u_files+UFTSIZE); ++j)
	*j = -1;

    /* Open the console tty device */
    if (d_open(TTYDEV) != 0)
	panic("no tty");

    ROOTDEV = bootdev;

    /* Mount the root device */
    if (fmount(ROOTDEV,NULLINODE))
	panic("no filesys");

    ifnot (root = i_open(ROOTDEV,ROOTINODE))
	panic("no root");
    i_ref(udata.u_cwd = root);
    rdtime(&udata.u_time);
}


xfs_end()
{
    register int16 j;

    for (j=0; j < UFTSIZE; ++j)
    {
	ifnot (udata.u_files[j] & 0x80)  /* Portable equivalent of == -1 */
	    doclose(j);
    }

    _sync();  /* Not necessary, but a good idea. */

}



_open(name, flag)
char *name;
register int16 flag;
{
    int16 uindex;
    register int16 oftindex;
    register inoptr ino;
    register int16 perm;
    inoptr n_open();

    udata.u_error = 0;

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

    return (uindex);

cantopen:
    oft_deref(oftindex);  /* This will call i_deref() */
nooft:
    udata.u_files[uindex] = -1;
    return (-1);
}




_close(uindex)
int16 uindex;
{
    udata.u_error = 0;
    return(doclose(uindex));
}



doclose(uindex)
int16 uindex;
{
    register int16 oftindex;
    inoptr ino;
    inoptr getinode();

    udata.u_error = 0;
    ifnot(ino = getinode(uindex))
	return(-1);
    oftindex = udata.u_files[uindex];

    if (isdevice(ino)
	/* && ino->c_refs == 1 && of_tab[oftindex].o_refs == 1 */ )
	d_close((int)(ino->c_node.i_addr[0]));

    udata.u_files[uindex] = -1;
    oft_deref(oftindex);

    return(0);
}



_creat(name, mode)
char *name;
int16 mode;
{
    register inoptr ino;
    register int16 uindex;
    register int16 oftindex;
    inoptr parent;
    register int16 j;
    inoptr n_open();
    inoptr newfile();

    udata.u_error = 0;
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
	if (parent && (ino = newfile(parent,name)))
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
	    if (parent)
		i_deref(parent);
	    goto nogood;
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




_link(name1, name2)
char *name1;
char *name2;
{
    register inoptr ino;
    register inoptr ino2;
    inoptr parent2;
    char *filename();
    inoptr n_open();

    udata.u_error = 0;
    ifnot (ino = n_open(name1,NULLINOPTR))
	return(-1);

    if (getmode(ino) == F_DIR && !super())
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

    if (ch_link(parent2,"",filename(name2),ino) == 0)
	goto nogood;


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



_unlink(path)
char *path;
{
    register inoptr ino;
    inoptr pino;
    char *filename();
    inoptr i_open();
    inoptr n_open();

    udata.u_error = 0;
    ino = n_open(path,&pino);

    ifnot (pino && ino)
    {
	udata.u_error = ENOENT;
	return (-1);
    }

    if (getmode(ino) == F_DIR && !super())
    {
	udata.u_error = EPERM;
	goto nogood;
    }

    /* Remove the directory entry */

    if (ch_link(pino,filename(path),"",NULLINODE) == 0)
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




_read(d, buf, nbytes)
int16 d;
char *buf;
uint16 nbytes;
{
    register inoptr ino;
    inoptr rwsetup();

    udata.u_error = 0;
    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(1, d, buf, nbytes)) == NULLINODE)
	return (-1);   /* bomb out if error */

    readi(ino);
    updoff(d);

    return (udata.u_count);
}



_write(d, buf, nbytes)
int16 d;
char *buf;
uint16 nbytes;
{
    register inoptr ino;
    off_t *offp;
    inoptr rwsetup();

    udata.u_error = 0;
    /* Set up u_base, u_offset, ino; check permissions, file num. */
    if ((ino = rwsetup(0, d, buf, nbytes)) == NULLINODE)
	return (-1);   /* bomb out if error */

    writei(ino);
    updoff(d);

    return (udata.u_count);
}



inoptr
rwsetup(rwflag, d, buf, nbytes)
int rwflag;
int d;
char *buf;
int nbytes;
{
    register inoptr ino;
    register struct oft *oftp;
    inoptr getinode();

    udata.u_base = buf;
    udata.u_count = nbytes;

    if ((ino = getinode(d)) == NULLINODE)
	return (NULLINODE);

    oftp = of_tab + udata.u_files[d];
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

	    bcopy(bp+udata.u_offset.o_offset, udata.u_base,
		    (amount = min(toread, 512 - udata.u_offset.o_offset)));
	    brelse(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
	    if (ispipe && udata.u_offset.o_blkno >= 18)
		udata.u_offset.o_blkno = 0;
	    toread -= amount;
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
    int created;	/* Set by bmap if newly allocated block used */
    int dev;
    char *zerobuf();
    char *bread();
    blkno_t bmap();

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

    loop:

	while (towrite)
	{
	    amount = min(towrite, 512 - udata.u_offset.o_offset);


	    if ((pblk = bmap(ino, udata.u_offset.o_blkno, 0)) == NULLBLK)
		break;    /* No space to make more blocks */

	    /* If we are writing an entire block, we don't care
	    about its previous contents */
	    bp = bread(dev, pblk, (amount == 512));

	    bcopy(udata.u_base, bp+udata.u_offset.o_offset, amount);
	    bawrite(bp);

	    udata.u_base += amount;
	    addoff(&udata.u_offset, amount);
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
    if (amount >= 0)
    {
    ofptr->o_offset = ofptr->o_offset + amount % 512;
    if (ofptr->o_offset >= 512)
    {
	ofptr->o_offset = ofptr->o_offset - 512;
	++ofptr->o_blkno;
    }
    ofptr->o_blkno = ofptr->o_blkno + amount/512;
    }
    else
    {
        ofptr->o_offset = ofptr->o_offset - (-amount) % 512;
        if (ofptr->o_offset < 0)
        {
	    ofptr->o_offset += 512;
	    --ofptr->o_blkno;
        }
        ofptr->o_blkno = ofptr->o_blkno - (-amount)/512;
    }
}


updoff(d)
int d;
{
    register off_t *offp;

    /* Update current file pointer */
    offp = &of_tab[udata.u_files[d]].o_ptr;
    offp->o_blkno = udata.u_offset.o_blkno;
    offp->o_offset = udata.u_offset.o_offset;
}



_seek(file,offset,flag)
int16 file;
uint16 offset;
int16 flag;
{
    register inoptr ino;
    register int16 oftno;
    register uint16 retval;
    inoptr getinode();

    udata.u_error = 0;
    if ((ino = getinode(file)) == NULLINODE)
	return(-1);

    oftno = udata.u_files[file];


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

    while ((unsigned)of_tab[oftno].o_ptr.o_offset >= 512)
    {
	of_tab[oftno].o_ptr.o_offset -= 512;
	++of_tab[oftno].o_ptr.o_blkno;
    }

    return((int16)retval);
}




_chdir(dir)
char *dir;
{
    register inoptr newcwd;
    inoptr n_open();

    udata.u_error = 0;
    ifnot (newcwd = n_open(dir,NULLINOPTR))
	return(-1);

    if (getmode(newcwd) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	i_deref(newcwd);
	return(-1);
    }
    i_deref(udata.u_cwd);
    udata.u_cwd = newcwd;
    return(0);
}




_mknod(name,mode,dev)
char *name;
int16 mode;
int16 dev;
{
    register inoptr ino;
    inoptr parent;
    inoptr n_open();
    inoptr newfile();

    udata.u_error = 0;
    ifnot (super())
    {
	udata.u_error = EPERM;
	return(-1);
    }

    if (ino = n_open(name,&parent))
    {
	udata.u_error = EEXIST;
	goto nogood;
    }
    
    ifnot (parent)
    {
	udata.u_error = ENOENT;
	return(-1);
    }

    ifnot (ino = newfile(parent,name))
	goto nogood2;

    /* Initialize mode and dev */
    ino->c_node.i_mode = mode & ~udata.u_mask;
    ino->c_node.i_addr[0] = isdevice(ino) ? dev : 0;
    setftime(ino, A_TIME|M_TIME|C_TIME);
    wr_inode(ino);

    i_deref(ino);
    return (0);

nogood:
    i_deref(ino);
nogood2:
    i_deref(parent);
    return (-1);
}




_sync()
{
    register j;
    register inoptr ino;
    register char *buf;
    char *bread();

    /* Write out modified inodes */

    udata.u_error = 0;
    for (ino=i_tab; ino < i_tab+ITABSIZE; ++ino)
	if ((ino->c_refs) > 0 && ino->c_dirty != 0)
	{
	    wr_inode(ino);
	    ino->c_dirty = 0;
	}

    /* Write out modified super blocks */
    /* This fills the rest of the super block with garbage. */

    for (j=0; j < NDEVS; ++j)
    {
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_fmod)
	{
	    fs_tab[j].s_fmod = 0;
	    buf = bread(j, 1, 1);
	    bcopy((char *)&fs_tab[j], buf, 512);
	    bfree(buf, 2);
	}
    }

    bufsync();   /* Clear buffer pool */
}


_access(path,mode)
char *path;
int16 mode;
{
    register inoptr ino;
    register int16 euid;
    register int16 egid;
    register int16 retval;
    inoptr n_open();

    udata.u_error = 0;
    if ((mode & 07) && !*(path))
    {
	udata.u_error = ENOENT;
	return (-1);
    }

    /* Temporarily make eff. id real id. */
    euid = udata.u_euid;
    egid = udata.u_egid;
    udata.u_euid = udata.u_ptab->p_uid;
    udata.u_egid = udata.u_gid;

    ifnot (ino = n_open(path,NULLINOPTR))
    {
	retval = -1;
	goto nogood;
    }
    
    retval = 0;
    if (~getperm(ino) & (mode&07))
    {
	udata.u_error = EPERM;
	retval = -1;
    }

    i_deref(ino);
nogood:
    udata.u_euid = euid;
    udata.u_egid = egid;

    return(retval);
}


_chmod(path,mode)
char *path;
int16 mode;
{

    inoptr ino;
    inoptr n_open();

    udata.u_error = 0;
    ifnot (ino = n_open(path,NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref(ino);
	udata.u_error = EPERM;
	return(-1);
    }

    ino->c_node.i_mode = (mode & MODE_MASK) | (ino->c_node.i_mode & F_MASK);
    setftime(ino, C_TIME);
    i_deref(ino);
    return(0);
}




_chown(path, owner, group)
char *path;
int owner;
int group;
{
    register inoptr ino;
    inoptr n_open();

    udata.u_error = 0;
    ifnot (ino = n_open(path,NULLINOPTR))
	return (-1);

    if (ino->c_node.i_uid != udata.u_euid && !super())
    {
	i_deref(ino);
	udata.u_error = EPERM;
	return(-1);
    }

    ino->c_node.i_uid = owner;
    ino->c_node.i_gid = group;
    setftime(ino, C_TIME);
    i_deref(ino);
    return(0);
}




_stat(path,buf)
char *path;
char *buf;
{

    register inoptr ino;
    inoptr n_open();

    udata.u_error = 0;
    ifnot (valadr(buf,sizeof(struct stat)) && (ino = n_open(path,NULLINOPTR)))
    {
	return (-1);
    }

    stcpy(ino,buf);
    i_deref(ino);
    return(0);
}




_fstat(fd, buf)
int16 fd;
char *buf;
{
    register inoptr ino;
    inoptr getinode();

    udata.u_error = 0;
    ifnot (valadr(buf,sizeof(struct stat)))
	return(-1);

    if ((ino = getinode(fd)) == NULLINODE)
	return(-1);

    stcpy(ino,buf);
    return(0);
}



/* Utility for stat and fstat */
stcpy(ino, buf)
inoptr ino;
char *buf;
{
    /* violently system-dependent */
    bcopy((char *)&(ino->c_dev), buf, 12);
    bcopy((char *)&(ino->c_node.i_addr[0]), buf+12, 2);
    bcopy((char *)&(ino->c_node.i_size), buf+14, 16);
}



_dup(oldd)
int16 oldd;
{
    register int newd;
    inoptr getinode();

    udata.u_error = 0;
    if (getinode(oldd) == NULLINODE)
	return(-1);

    if ((newd = uf_alloc()) == -1)
	return (-1);
    
    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return(newd);
}




_dup2(oldd, newd)
int16 oldd;
int16 newd;
{
    inoptr getinode();

    udata.u_error = 0;
    if (getinode(oldd) == NULLINODE)
	return(-1);

    if (newd < 0 || newd >= UFTSIZE)
    {
	udata.u_error = EBADF;
	return (-1);
    }
    
    ifnot (udata.u_files[newd] & 0x80)
	doclose(newd);
    
    udata.u_files[newd] = udata.u_files[oldd];
    ++of_tab[udata.u_files[oldd]].o_refs;

    return(0);
}




_umask(mask)
int mask;
{
    register int omask;

    udata.u_error = 0;
    omask = udata.u_mask;
    udata.u_mask = mask & 0777;
    return(omask);
}




/* Special system call returns super-block of given
filesystem for users to determine free space, etc.
Should be replaced with a sync() followed by a read
of block 1 of the device.  */

_getfsys(dev,buf)
{
    udata.u_error = 0;
   if (dev < 0 || dev >= NDEVS || fs_tab[dev].s_mounted != SMOUNTED)
   {
       udata.u_error = ENXIO;
       return(-1);
    }

    bcopy((char *)&fs_tab[dev],(char *)buf,sizeof(struct filesys));
    return(0);
}



_ioctl(fd, request, data)
int fd;
int request;
char *data;
{

    register inoptr ino;
    register int dev;
    inoptr getinode();

    udata.u_error = 0;
    if ((ino = getinode(fd)) == NULLINODE)
	return(-1);

    ifnot (isdevice(ino))
    {
	udata.u_error = ENOTTY;
	return(-1);
    }

    ifnot (getperm(ino) & OTH_WR)
    {
	udata.u_error = EPERM;
	return(-1);
    }

    dev = ino->c_node.i_addr[0];

    if (d_ioctl(dev, request,data))
	return(-1);
    return(0);
}




_mount(spec, dir, rwflag)
char *spec;
char *dir;
int rwflag;
{
    register inoptr sino, dino;
    register int dev;
    inoptr n_open();

    udata.u_error = 0;
    ifnot(super())
    {
	udata.u_error = EPERM;
	return (-1);
    }

    ifnot (sino = n_open(spec,NULLINOPTR))
	return (-1);

    ifnot (dino = n_open(dir,NULLINOPTR))
    {
	i_deref(sino);
	return (-1);
    }

    if (getmode(sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }

    if (getmode(dino) != F_DIR)
    {
	udata.u_error = ENOTDIR;
	goto nogood;
    }

    dev = (int)sino->c_node.i_addr[0];

    if ( dev >= NDEVS || d_open(dev))
    {
	udata.u_error = ENXIO;
	goto nogood;
    }

    if (fs_tab[dev].s_mounted || dino->c_refs != 1 || dino->c_num == ROOTINODE)
    {
       udata.u_error = EBUSY;
       goto nogood;
    }

    _sync();

    if (fmount(dev,dino))
    {
       udata.u_error = EBUSY;
       goto nogood;
    }

    i_deref(dino);
    i_deref(sino);
    return(0);

nogood:
    i_deref(dino);
    i_deref(sino);
    return (-1);
}




_umount(spec)
char *spec;
{
    register inoptr sino;
    register int dev;
    register inoptr ptr;
    inoptr n_open();

    udata.u_error = 0;
    ifnot(super())
    {
	udata.u_error = EPERM;
	return (-1);
    }

    ifnot (sino = n_open(spec,NULLINOPTR))
	return (-1);

    if (getmode(sino) != F_BDEV)
    {
	udata.u_error = ENOTBLK;
	goto nogood;
    }

    dev = (int)sino->c_node.i_addr[0];
    ifnot (validdev(dev))
    {
	udata.u_error = ENXIO;
	goto nogood;
    }

    if (!fs_tab[dev].s_mounted)
    {
	udata.u_error = EINVAL;
	goto nogood;
    }

    for (ptr = i_tab; ptr < i_tab+ITABSIZE; ++ptr)
	if (ptr->c_refs > 0 && ptr->c_dev == dev)
	{
	    udata.u_error = EBUSY;
	    goto nogood;
	}

    _sync();
    fs_tab[dev].s_mounted = 0;
    i_deref(fs_tab[dev].s_mntpt);

    i_deref(sino);
    return(0);

nogood:
    i_deref(sino);
    return (-1);
}





_time(tvec)
int tvec[];
{
    udata.u_error = 0;
    rdtime(tvec);  /* In machdep.c */
    return(0);
}












/* N_open is given a string containing a path name,
  and returns a inode table pointer.  If it returns NULL,
  the file did not exist.  If the parent existed,
  and parent is not null, parent will be filled in with
  the parents inoptr. Otherwise, parent will be set to NULL. */

inoptr
n_open(name,parent)
register char *name;
register inoptr *parent;
{

    register inoptr wd;  /* the directory we are currently searching. */
    register inoptr ninode;
    register inoptr temp;
    inoptr srch_dir();
    inoptr srch_mt();

    if (*name == '/')
	wd = root;
    else
	wd = udata.u_cwd;

    i_ref(ninode = wd);
    i_ref(ninode);

    for(;;)
    {
	if (ninode)
	    magic(ninode);

	/* See if we are at a mount point */
	if (ninode)
	    ninode = srch_mt(ninode);

	while (*name == '/')	/* Skip (possibly repeated) slashes */
	    ++name;
	ifnot (*name)		/* No more components of path? */
	    break;
	ifnot (ninode)
	{
	    udata.u_error = ENOENT;
	    goto nodir;
	}
	i_deref(wd);
	wd = ninode;
	if (getmode(wd) != F_DIR)
	{
	    udata.u_error = ENOTDIR;
	    goto nodir;
	}
	ifnot (getperm(wd) & OTH_EX)
	{
	    udata.u_error = EPERM;
	    goto nodir;
	}

	/* See if we are going up through a mount point */
	if ( wd->c_num == ROOTINODE && wd->c_dev != ROOTDEV && name[1] == '.')
	{
	   temp = fs_tab[wd->c_dev].s_mntpt;
	   ++temp->c_refs;
	   i_deref(wd);
	   wd = temp;
	}

	ninode = srch_dir(wd,name);

	while (*name != '/' && *name )
	    ++name;
    }

    if (parent)
	*parent = wd;
    else
	i_deref(wd);
    ifnot (parent || ninode)
	udata.u_error = ENOENT;
    return (ninode);

nodir:
    if (parent)
	*parent = NULLINODE;
    i_deref(wd);
    return(NULLINODE);

}



/* Srch_dir is given a inode pointer of an open directory
and a string containing a filename, and searches the directory
for the file.  If it exists, it opens it and returns the inode pointer,
otherwise NULL. This depends on the fact that ba_read will return unallocated
blocks as zero-filled, and a partially allocated block will be padded with
zeroes.  */

inoptr
srch_dir(wd,compname)
register inoptr wd;
register char *compname;
{
    register int curentry;
    register blkno_t curblock;
    register struct direct *buf;
    register int nblocks;
    unsigned inum;
    inoptr i_open();
    blkno_t bmap();

    nblocks = wd->c_node.i_size.o_blkno;
    if (wd->c_node.i_size.o_offset)
	++nblocks;

    for (curblock=0; curblock < nblocks; ++curblock)
    {
	buf = (struct direct *)bread( wd->c_dev, bmap(wd, curblock, 1), 0);
	for (curentry = 0; curentry < 32; ++curentry)
	{
	    if (namecomp(compname,buf[curentry].d_name))
	    {
		inum = buf[curentry&0x1f].d_ino;
		brelse(buf);
		return(i_open(wd->c_dev, inum));
	    }
	}
	brelse(buf);
    }
    return(NULLINODE);
}



/* Srch_mt sees if the given inode is a mount point. If
so it dereferences it, and references and returns a pointer
to the root of the mounted filesystem. */

inoptr
srch_mt(ino)
register inoptr ino;
{
    register int j;
    inoptr i_open();

    for (j=0; j < NDEVS; ++j)
	if (fs_tab[j].s_mounted == SMOUNTED && fs_tab[j].s_mntpt == ino)
	{
	    i_deref(ino);
	    return(i_open(j,ROOTINODE));
	}
    
    return(ino);
}


/* I_open is given an inode number and a device number,
and makes an entry in the inode table for them, or
increases it reference count if it is already there.
An inode # of zero means a newly allocated inode */

inoptr
i_open(dev,ino)
register int dev;
register unsigned ino;
{

    struct dinode *buf;
    register inoptr nindex;
    int i;
    register inoptr j;
    int new;
    static nexti = i_tab;
    unsigned i_alloc();

    if (dev<0 || dev>=NDEVS)
	panic("i_open: Bad dev");

    new = 0;
    ifnot (ino)  /* Want a new one */
    {
	new = 1;
        ifnot (ino = i_alloc(dev))
        {
	    udata.u_error = ENOSPC;
	    return (NULLINODE);
        }
    }

    if (ino < ROOTINODE || ino >= (fs_tab[dev].s_isize-2)*8)
    {
	warning("i_open: bad inode number");
	return (NULLINODE);
    }


    nindex = NULLINODE;
    j = nexti;
    for (i=0; i < ITABSIZE; ++i)
    {
        nexti =j;
        if (++j >= i_tab+ITABSIZE)
	    j = i_tab;

        ifnot (j->c_refs)
	   nindex = j;

	if (j->c_dev == dev && j->c_num == ino)
	{
	    nindex = j;
	    goto found;
	}
    }

    /* Not already in table. */

    ifnot (nindex)  /* No unrefed slots in inode table */
    {
	udata.u_error = ENFILE;
	return(NULLINODE);
    }

    buf = (struct dinode *)bread(dev, (ino>>3)+2, 0);
    bcopy((char *)&(buf[ino & 0x07]), (char *)&(nindex->c_node), 64);
    brelse(buf);

    nindex->c_dev = dev;
    nindex->c_num = ino;
    nindex->c_magic = CMAGIC;

found:
    if (new)
    {
        if (nindex->c_node.i_nlink || nindex->c_node.i_mode & F_MASK)
	    goto badino;
    }
    else
    {
        ifnot (nindex->c_node.i_nlink && nindex->c_node.i_mode & F_MASK)
	    goto badino;
    }

    ++nindex->c_refs;
    return(nindex);

badino:
    warning("i_open: bad disk inode");
    return (NULLINODE);
}



/* Ch_link modifies or makes a new entry in the directory for the name
and inode pointer given. The directory is searched for oldname.
When found, it is changed to newname, and it inode # is that of
*nindex.  A oldname of "" matches a unused slot, and a nindex
of NULLINODE means an inode # of 0.  A return status of 0 means there
was no space left in the filesystem, or a non-empty oldname was not found,
or the user did not have write permission. */

ch_link(wd,oldname,newname,nindex)
register inoptr wd;
char *oldname;
char *newname;
inoptr nindex;
{
    struct direct curentry;

    ifnot (getperm(wd) & OTH_WR)
    {
	udata.u_error = EPERM;
	return (0);
    }

    /* Search the directory for the desired slot. */

    udata.u_offset.o_blkno = 0;
    udata.u_offset.o_offset = 0;

    for (;;)
    {
	udata.u_count = 16;
	udata.u_base = (char *)&curentry;
	readi(wd);

	/* Read until EOF or name is found */
	/* readi() advances udata.u_offset */
	if (udata.u_count == 0 || namecomp(oldname, curentry.d_name))
	    break;
    }

    if (udata.u_count == 0 && *oldname)
	return (0);   /* Entry not found */

    bcopy(newname, curentry.d_name, 14);
    if (nindex)
	curentry.d_ino = nindex->c_num;
    else
	curentry.d_ino = 0;

    /* If an existing slot is being used, we must back up the file offset */
    if (udata.u_count)
    {
	ifnot (udata.u_offset.o_offset)
	{
	    --udata.u_offset.o_blkno;
	    udata.u_offset.o_offset = 512;
	}
	udata.u_offset.o_offset -= 16;
    }

    udata.u_count = 16;
    udata.u_base = (char *)&curentry;
    writei(wd);

    if (udata.u_error)
	return (0);

    setftime(wd, A_TIME|M_TIME|C_TIME);  /* Sets c_dirty */

    /* Update file length to next block */
    if (wd->c_node.i_size.o_offset)
    {
	wd->c_node.i_size.o_offset = 0;
	++wd->c_node.i_size.o_blkno;
    }

    return (1);
}



/* Filename is given a path name, and returns a pointer
to the final component of it. */

char *
filename(path)
char *path;
{
    register char *ptr;

    ptr = path;
    while (*ptr)
	++ptr;
    while (*ptr != '/' && ptr-- > path)
	;
    return (ptr+1);
}


/* Namecomp compares two strings to see if they are the same file name.
It stops at 14 chars or a null or a slash. It returns 0 for difference. */

namecomp(n1,n2)
register char *n1;
register char *n2;
{
    register int n;

    n = 14;
    while (*n1 && *n1 != '/')
    {
	if (*n1++ != *n2++)
	    return(0);
	ifnot (--n)
	    return(-1);
    }
    return(*n2 == '\0' || *n2 == '/');
}



/* Newfile is given a pointer to a directory and a name, and
   creates an entry in the directory for the name, dereferences
   the parent, and returns a pointer to the new inode.
   It allocates an inode number,
   and creates a new entry in the inode table for the new file,
   and initializes the inode table entry for the new file.  The new file
   will have one reference, and 0 links to it. 
   Better make sure there isn't already an entry with the same name. */

inoptr
newfile(pino, name)
register inoptr pino;
register char *name;
{

    register inoptr nindex;
    register int j;
    inoptr i_open();

    ifnot (nindex = i_open(pino->c_dev, 0))
	goto nogood;

    nindex->c_node.i_mode = F_REG;   /* For the time being */
    nindex->c_node.i_nlink = 1;
    nindex->c_node.i_size.o_offset = 0;
    nindex->c_node.i_size.o_blkno = 0;
    for (j=0; j <20; j++)
        nindex->c_node.i_addr[j] = 0;
    wr_inode(nindex);

    ifnot (ch_link(pino,"",filename(name),nindex))
    {
	i_deref(nindex);
	goto nogood;
    }

    i_deref (pino);
    return(nindex);

nogood:
    i_deref (pino);
    return (NULLINODE);
}



/* Check the given device number, and return its address in the mount table.
Also time-stamp the superblock of dev, and mark it modified.
Used when freeing and allocating blocks and inodes. */

fsptr
getdev(devno)
register int devno;
{
    register fsptr dev;

    dev = fs_tab + devno;
    if (devno < 0 || devno >= NDEVS || !dev->s_mounted)
	panic("getdev: bad dev");
    rdtime(&(dev->s_time));
    dev->s_fmod = 1;
    return (dev);
}


/* Returns true if the magic number of a superblock is corrupt */

baddev(dev)
fsptr dev;
{
    return (dev->s_mounted != SMOUNTED);
}


/* I_alloc finds an unused inode number, and returns it,
or 0 if there are no more inodes available. */

unsigned 
i_alloc(devno)
int devno;
{
    fsptr dev;
    blkno_t blk;
    struct dinode *buf;
    register int j;
    register int k;
    unsigned ino;

    if (baddev(dev = getdev(devno)))
	goto corrupt;

tryagain:
    if (dev->s_ninode)
    {
	ifnot (dev->s_tinode)
	    goto corrupt;
	ino = dev->s_inode[--dev->s_ninode];
	if (ino < 2 || ino >= (dev->s_isize-2)*8)
	    goto corrupt;
	--dev->s_tinode;
	return(ino);
    }

    /* We must scan the inodes, and fill up the table */

    _sync();  /* Make on-disk inodes consistent */
    k = 0;
    for (blk = 2; blk < dev->s_isize; blk++)
    {
        buf = (struct dinode *)bread(devno, blk, 0);
	for (j=0; j < 8; j++)
	{
	    ifnot (buf[j].i_mode || buf[j].i_nlink)
		dev->s_inode[k++] = 8*(blk-2) + j;
	    if (k==50)
	    {
	        brelse(buf);
		goto done;
	    }
	}
        brelse(buf);
    }

done:
    ifnot (k)
    {
	if (dev->s_tinode)
	    goto corrupt;
	udata.u_error = ENOSPC;
	return(0);
    }

    dev->s_ninode = k;
    goto tryagain;

corrupt:
    warning("i_alloc: corrupt superblock");
    dev->s_mounted = 1;
    udata.u_error = ENOSPC;
    return(0);
}


/* I_free is given a device and inode number,
and frees the inode.  It is assumed that there
are no references to the inode in the inode table or
in the filesystem. */

i_free(devno, ino)
register int devno;
register unsigned ino;
{
    register fsptr dev;

    if (baddev(dev = getdev(devno)))
	return;

    if (ino < 2 || ino >= (dev->s_isize-2)*8)
	panic("i_free: bad ino");

    ++dev->s_tinode;
    if (dev->s_ninode < 50)
	dev->s_inode[dev->s_ninode++] = ino;
}


/* Blk_alloc is given a device number, and allocates an unused block
from it. A returned block number of zero means no more blocks. */

blkno_t
blk_alloc(devno)
register int devno;
{

    register fsptr dev;
    register blkno_t newno;
    blkno_t *buf;
    register int j;

    if (baddev(dev = getdev(devno)))
	goto corrupt2;

    if (dev->s_nfree <= 0 || dev->s_nfree > 50)
	goto corrupt;

    newno = dev->s_free[--dev->s_nfree];
    ifnot (newno)
    {
	if (dev->s_tfree != 0)
	    goto corrupt;
	udata.u_error = ENOSPC;
	++dev->s_nfree;
	return(0);
    }

    /* See if we must refill the s_free array */

    ifnot (dev->s_nfree)
    {
	buf = (blkno_t *)bread(devno,newno, 0);
	dev->s_nfree = buf[0];
	for (j=0; j < 50; j++)
	{
	    dev->s_free[j] = buf[j+1];
	}
	brelse((char *)buf);
    }

    validblk(devno, newno);

    ifnot (dev->s_tfree)
	goto corrupt;
    --dev->s_tfree;

    /* Zero out the new block */
    buf = bread(devno, newno, 2);
    bzero(buf, 512);
    bawrite(buf);
    return(newno);

corrupt:
    warning("blk_alloc: corrupt");
    dev->s_mounted = 1;
corrupt2:
    udata.u_error = ENOSPC;
    return(0);
}


/* Blk_free is given a device number and a block number,
and frees the block. */

blk_free(devno,blk)
register int devno;
register blkno_t blk;
{
    register fsptr dev;
    register char *buf;

    ifnot (blk)
	return;

    if (baddev(dev = getdev(devno)))
        return;

    validblk(devno, blk);

    if (dev->s_nfree == 50)
    {
	buf = bread(devno, blk, 1);
	bcopy((char *)&(dev->s_nfree), buf, 512);
	bawrite(buf);
	dev->s_nfree = 0;
    }

    ++dev->s_tfree;
    dev->s_free[(dev->s_nfree)++] = blk;

}




/* Oft_alloc and oft_deref allocate and dereference (and possibly free)
entries in the open file table. */

oft_alloc()
{
    register int j;

    for (j=0; j < OFTSIZE ; ++j)
    {
	ifnot (of_tab[j].o_refs)
	{
	    of_tab[j].o_refs = 1;
	    of_tab[j].o_inode = NULLINODE;
	    return (j);
	}
    }
    udata.u_error = ENFILE;
    return(-1);
}

oft_deref(of)
register int of;
{
    register struct oft *ofptr;

    ofptr = of_tab + of;

    if (!(--ofptr->o_refs) && ofptr->o_inode)
    {
        i_deref(ofptr->o_inode);
	ofptr->o_inode = NULLINODE;
    }
}



/* Uf_alloc finds an unused slot in the user file table. */

uf_alloc()
{
    register int j;

    for (j=0; j < UFTSIZE ; ++j)
    {
	if (udata.u_files[j] & 0x80)  /* Portable, unlike  == -1 */
	{
	    return (j);
	}
    }
    udata.u_error = ENFILE;
    return(-1);
}



/* I_ref increases the reference count of the given inode table entry. */

i_ref(ino)
inoptr ino;
{
    if (++(ino->c_refs) == 2*ITABSIZE)  /* Arbitrary limit. */
	panic("too many i-refs");
}


/* I_deref decreases the reference count of an inode, and frees it
from the table if there are no more references to it.  If it also
has no links, the inode itself and its blocks (if not a device) is freed. */

i_deref(ino)
register inoptr ino;
{
    magic(ino);

    ifnot (ino->c_refs)
	panic("inode freed.");

    /* If the inode has no links and no refs, it must have
    its blocks freed. */

    ifnot (--ino->c_refs || ino->c_node.i_nlink)
	    f_trunc(ino);

    /* If the inode was modified, we must write it to disk. */
    if (!(ino->c_refs) && ino->c_dirty)
    {
	ifnot (ino->c_node.i_nlink)
	{
	    ino->c_node.i_mode = 0;
	    i_free(ino->c_dev, ino->c_num);
	}
	wr_inode(ino);
    }
}


/* Wr_inode writes out the given inode in the inode table out to disk,
and resets its dirty bit. */

wr_inode(ino)
register inoptr ino;
{
    struct dinode *buf;
    register blkno_t blkno;

    magic(ino);

    blkno = (ino->c_num >> 3) + 2;
    buf = (struct dinode *)bread(ino->c_dev, blkno,0);
    bcopy((char *)(&ino->c_node),
	(char *)((char **)&buf[ino->c_num & 0x07]), 64);
    bfree(buf, 2);
    ino->c_dirty = 0;
}


/* isdevice(ino) returns true if ino points to a device */
isdevice(ino)
inoptr ino;
{
    return (ino->c_node.i_mode & 020000);
}


/* This returns the device number of an inode representing a device */
devnum(ino)
inoptr ino;
{
    return (*(ino->c_node.i_addr));
}


/* F_trunc frees all the blocks associated with the file,
if it is a disk file. */

f_trunc(ino)
register inoptr ino;
{
    int dev;
    int j;

    dev = ino->c_dev;

    /* First deallocate the double indirect blocks */
    freeblk(dev, ino->c_node.i_addr[19], 2);

    /* Also deallocate the indirect blocks */
    freeblk(dev, ino->c_node.i_addr[18], 1);

    /* Finally, free the direct blocks */
    for (j=17; j >= 0; --j)
	freeblk(dev, ino->c_node.i_addr[j], 0);

    bzero((char *)ino->c_node.i_addr, sizeof(ino->c_node.i_addr));

    ino->c_dirty = 1;
    ino->c_node.i_size.o_blkno = 0;
    ino->c_node.i_size.o_offset = 0;
}


/* Companion function to f_trunc(). */
freeblk(dev, blk, level)
int dev;
blkno_t blk;
int level;
{
    blkno_t *buf;
    int j;

    ifnot (blk)
	return;

    if (level)
    {
	buf = (blkno_t *)bread(dev, blk, 0);
	for (j=255; j >= 0; --j)
	    freeblk(dev, buf[j], level-1);
	brelse((char *)buf);
    }

    blk_free(dev,blk);
}



/* Changes: blk_alloc zeroes block it allocates */

/*
 * Bmap defines the structure of file system storage
 * by returning the physical block number on a device given the
 * inode and the logical block number in a file.
 * The block is zeroed if created.
 */
blkno_t
bmap(ip, bn, rwflg)
inoptr ip;
register blkno_t bn;
int rwflg;
{
	register int i;
	register bufptr bp;
	register int j;
	register blkno_t nb;
	int sh;
	int dev;

	blkno_t blk_alloc();

	if (getmode(ip) == F_BDEV)
	    return (bn);

	dev = ip->c_dev;

	/*
	 * blocks 0..17 are direct blocks
	 */
	if(bn < 18) {
		nb = ip->c_node.i_addr[bn];
		if(nb == 0) {
			if(rwflg || (nb = blk_alloc(dev))==0)
				return(NULLBLK);
			ip->c_node.i_addr[bn] = nb;
			ip->c_dirty = 1;
		}
		return(nb);
	}

	/*
	 * addresses 18 and 19
	 * have single and double indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */
	bn -= 18;
	sh = 0;
	j = 2;
	if (bn & 0xff00)   /* bn > 255  so double indirect */
	{
	    sh = 8;
	    bn -= 256;
	    j = 1;
	}

	/*
	 * fetch the address from the inode
	 * Create the first indirect block if needed.
	 */
	ifnot (nb = ip->c_node.i_addr[20-j])
	{
		if(rwflg || !(nb = blk_alloc(dev)))
			return(NULLBLK);
		ip->c_node.i_addr[20-j] = nb;
		ip->c_dirty = 1;
	}

	/*
	 * fetch through the indirect blocks
	 */
	for(; j<=2; j++) {
		bp = (bufptr)bread(dev, nb, 0);
		/******
		if(bp->bf_error) {
			brelse(bp);
			return((blkno_t)0);
		}
		******/
		i = (bn>>sh) & 0xff;
		if (nb = ((blkno_t *)bp)[i])
		    brelse(bp);
		else
		{
			if(rwflg || !(nb = blk_alloc(dev))) {
				brelse(bp);
				return(NULLBLK);
			}
			((blkno_t *)bp)[i] = nb;
			bawrite(bp);
		}
		sh -= 8;
	}

	return(nb);
}



/* Validblk panics if the given block number is not a valid data block
for the given device. */

validblk(dev, num)
int dev;
blkno_t num;
{
    register fsptr devptr;

    devptr = fs_tab + dev;

    if (devptr->s_mounted == 0)
	panic("validblk: not mounted");
    
    if (num < devptr->s_isize || num >= devptr->s_fsize)
	panic("validblk: invalid blk");
}



/* This returns the inode pointer associated with a user's
file descriptor, checking for valid data structures */

inoptr
getinode(uindex)
register int uindex;
{
    register int oftindex;
    register inoptr inoindex;

    if (uindex < 0 || uindex >= UFTSIZE || udata.u_files[uindex] & 0x80 )
    {
	udata.u_error = EBADF;
	return (NULLINODE);
    }

    if ((oftindex = udata.u_files[uindex]) < 0 || oftindex >= OFTSIZE)
	panic("Getinode: bad desc table");

    if ((inoindex = of_tab[oftindex].o_inode) < i_tab ||
    		inoindex >= i_tab+ITABSIZE)
	panic("Getinode: bad OFT");

    magic(inoindex);

    return(inoindex);
}

/* Super returns true if we are the superuser */
super()
{
    return(udata.u_euid == 0);
}

/* Getperm looks at the given inode and the effective user/group ids, and
returns the effective permissions in the low-order 3 bits. */

getperm(ino)
inoptr ino;
{
    int mode;

    if (super())
	return(07);

    mode = ino->c_node.i_mode;
    if (ino->c_node.i_uid == udata.u_euid)
	mode >>= 6;
    else if (ino->c_node.i_gid == udata.u_egid)
	mode >>= 3;

    return(mode & 07);
}


/* This sets the times of the given inode, according to the flags */

setftime(ino, flag)
register inoptr ino;
register int flag;
{
    ino->c_dirty = 1;

    if (flag & A_TIME)
	rdtime(&(ino->c_node.i_atime));
    if (flag & M_TIME)
	rdtime(&(ino->c_node.i_mtime));
    if (flag & C_TIME)
	rdtime(&(ino->c_node.i_ctime));
}


getmode(ino)
inoptr ino;
{
    return( ino->c_node.i_mode & F_MASK);
}


/* Fmount places the given device in the mount table with
mount point ino */

fmount(dev,ino)
register int dev;
register inoptr ino;
{
    char *buf;
    register struct filesys *fp;

    if (d_open(dev) != 0)
	panic("fmount: Cant open filesystem");
    /* Dev 0 blk 1 */
    fp = fs_tab + dev;
    buf = bread(dev, 1, 0);
    bcopy(buf, (char *)fp, sizeof(struct filesys));
    brelse(buf);

    /* See if there really is a filesystem on the device */
    if (fp->s_mounted != SMOUNTED ||
         fp->s_isize >= fp->s_fsize)
	return (-1);

    fp->s_mntpt = ino;
    if (ino)
	++ino->c_refs;

    return (0);
}


magic(ino)
inoptr ino;
{
    if (ino->c_magic != CMAGIC)
	panic("Corrupt inode");
}
