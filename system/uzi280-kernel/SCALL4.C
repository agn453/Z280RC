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

/*
	this file is for sep I/D pages
*/
#define FALSE 	0
#define TRUE	1

#include "unix.h"
#include "extern.h"

#undef MEMDEBUG

/* User's execve() call. All other flavors are library routines. */

/*****************************************
execve(name, argv, envp)
char *name;
char *argv[];
char *envp[];
*****************************************/

#define name (char *)udata.u_argn2
#define argv (char **)udata.u_argn1
#define envp (char **)udata.u_argn

_execve()
{
    register inoptr ino;
    char *buf;
    inoptr n_open();
    char *bread();
    blkno_t bmap();
    blkno_t blk;
    char **nargv;   /* In user space */
    char **nenvp;   /* In user space */
    struct s_argblk *abuf, *ebuf;
    int (**sigp)();
    int argc;
    char **rargs();
    char *progptr,*dataptr;
    unsigned proglen,datalen;
    int j;
    unsigned  magic,c_blk;
    unsigned *pbuf;	
    ptptr p; 	/* used for shared code algo. */
/*    pg_descr *ptmp; 	*/
    int getperm(),wargs();
    unsigned tmpbuf();
 	
    /* Bit of code that is poked into user address space to help signal
	catchers return correctly.  It pops HL, then all regs, and returns */

	/*
	pop hl, ex af,af', pop af, ex af,af', 
	exx, pop bc, pop de, pop hl, exx,
	pop af, pop bc, pop de, pop hl, pop ix, pop iy, ret 
	*/

    static char sigprog[] = { 0xe1, 0x08, 0xf1, 0x08,
			      0xd9, 0xc1, 0xd1, 0xe1, 0xd9,
		 	      0xf1, 0xc1, 0xd1, 0xe1, 0xdd,
		 	      0xe1, 0xfd, 0xe1, 0xc9 };

    if (! (ino = n_open(name,NULLINOPTR)) )
	return(-1);

    if (!( (getperm(ino) & OTH_EX) 	&&
	   (ino->c_node.i_mode & F_REG) &&
           (ino->c_node.i_mode & (OWN_EX | OTH_EX | GRP_EX)) 
       ) ) 
    {
	udata.u_error = EACCES;
	goto nogood;
    }
    setftime(ino, A_TIME);

    /* Read in the first block of the new program */
    buf = bread( ino->c_dev, bmap(ino, 0, 1), 0);
    pbuf=(unsigned *)buf;

	/**************************************
	get magic number into var magic

	C3	: executable file no C/D sep.
	00FF 	:     "	      "   with C/D sep.
	other	: maybe shell script (nogood2)
	**************************************/

    if ( (*buf & 0xff) != EMAGIC &&
         (magic = (unsigned)*pbuf) != ECDMAGIC) {
	udata.u_error = ENOEXEC;
	goto nogood2;
    }

    /* Gather the arguments, and put them in temporary buffers. */
    /* Put environment in another buffer. */
    abuf = (struct s_argblk *)tmpbuf();
    ebuf = (struct s_argblk *)tmpbuf();


    if (wargs(argv, abuf) || wargs(envp, ebuf))
	goto nogood3; /* SN */

    di();
    udata.u_ptab->p_inode = ino->c_num;
    udata.u_ptab->p_idev  = ino->c_dev;
    udata.u_ptab->p_idate.t_time = ino->c_node.i_ctime.t_time;
    udata.u_ptab->p_idate.t_date = ino->c_node.i_ctime.t_date;

    p = find_zombie2(); 

    /* Here, check the setuid stuff. No other changes need be made in
       the user data */
    if (ino->c_node.i_mode & SET_UID)
	udata.u_euid = ino->c_node.i_uid;

    if (ino->c_node.i_mode & SET_GID)
	udata.u_egid = ino->c_node.i_gid;

    /* At this point, we are definitely going to succeed with the exec,
       so we can start writing over the old program */


/* bug fix for type 1 binaries: 
   Now works without a previous fork() call!
*/ 	
    /* Release all pages and mark them as VIRGIN.
       After loading the program, and setting the stack,
       mark all still-virgin pages as illegal. */

    for (j=15; j>=0; j--) {
      if (realpage(udata.u_page[j])) {
	if(!udata.u_ptab->p_fork_inf) {
	  /* not child of fork. Special case for type 1 binaries */
	  if(j>7 && udata.u_page[j]->pg_addr!=udata.u_page[j-8]->pg_addr)
	    pg_free(udata.u_page[j]);
	  if(j<8) /* handle code pages normal */	
	    pg_free(udata.u_page[j]);
	} else /* child of fork. Normal case */ 
 	  if(udata.u_page[j]->pg_refs)
	    pg_free(udata.u_page[j]);	
      }
      udata.u_page[j] = VIRGPAGE;
    }
    udata.u_ptab->p_fork_inf = 0; /* reset child of fork */
    give_pages(p);	


    ei();	
#ifdef MEMDEBUG
    kprintf("Execing process %d\n", udata.u_ptab->p_pid);
#endif
/* 
	handle magic C3 and 00FF different
*/
if( (*buf & 0xff) == EMAGIC )
{	 	   /*======*/
    if (ino->c_node.i_size.o_blkno > ((uint16)PROGTOP-((uint16)PROGBASE))/512)
    {
	udata.u_error = ENOMEM;
	/* release pages on error (proc to long) */
	if(p)	
    	  for (j=0; j<16; ++j) 
		if (realpage(udata.u_page[j])) 
			pg_free(udata.u_page[j]);
	goto nogood3;
    }

    /* set info about binary type */
    udata.u_ptab->p_bintype = 1;
	
    ldmmu(udata.u_page);   

    ifnot(p) 
    	uputp(buf,PROGBASE,512);

    brelse((bufptr)buf);

    /* Read in the rest of the program */
    dataptr = PROGBASE+512;
    for (blk = 1; blk <= ino->c_node.i_size.o_blkno; ++blk)
    {
        ifnot(p) {
            buf = bread(ino->c_dev, bmap(ino, blk, 1), 0);
	    uputp(buf, dataptr, 512);
	    brelse((bufptr)buf);
        }
	dataptr += 512;
    }
    datalen = dataptr;
    proglen = datalen;	

    /* set data page = code page and mark all data pages COW */
    ifnot(p)
	setup_pg_type1();

}/* end of EMAGIC */
else  
	/********
	 ECDMAGIC goes here 
	
	the first block only contains information of the file:
	Data and code section starts on new block.
	byte 0,1	: magic number 
	byte 2,3	: data length in bytes
	byte 4,5	: code length in bytes
	rest		: not used 

	the following blocks contain data and code section in that order
	********/ 
{
    /* set info about binary type */
    udata.u_ptab->p_bintype = 0;

    ldmmu(udata.u_page);
	/* 
	get progam and datasize 
	*/
    datalen = pbuf[1];
    proglen = pbuf[2];
    /*
	first get the data section.  
    */	
    brelse((bufptr)buf);
    c_blk = (datalen / 512);
    if (datalen % 512) ++c_blk;
    dataptr = PROGBASE;
    ifnot(p) {
     for (blk = 1; blk <= c_blk; ++blk)
     {
            buf = bread(ino->c_dev, bmap(ino, blk, 1), 0);
            if ((unsigned)dataptr < (unsigned)65024)
		    uput(buf, dataptr, 512);
	    else
	    	    uput(buf, dataptr, 
			 (unsigned)65535 - (unsigned)dataptr);
	    brelse((bufptr)buf);
        dataptr += 512;
     }
    /*
	now get the code section.
    */
     c_blk  += (proglen / 512);
     if (proglen % 512) ++c_blk;
     progptr = PROGBASE;
    	for ( ; blk <= c_blk; ++blk)
    	{
            		buf = bread(ino->c_dev, bmap(ino, blk, 1), 0);
            		if ((unsigned)progptr < (unsigned)65024)
		    		uputp(buf, progptr, 512);
		    	else
		    		uputp(buf, progptr, 
				      (unsigned)65535-(unsigned)progptr);
			brelse((bufptr)buf);
                progptr += 512;
    	}
    } 
}
	/***** 
	all magic files continue here
	******/

    i_deref(ino);
    /* 
	set brk for program 
    */
    udata.u_break  = datalen;	
    udata.u_pbreak = proglen;   /* only for ps */

    /* Poke in the special signal catcher program */
/* we put the signal catcher to address 0x040. This saves 8K bytes
   of prog space in most cases. This memory region is under CP/M
   reseved, so the should be no problem with patched CP/M programms 
   	SN */
    ifnot(p) 
    	uputp(sigprog, 0x0040, sizeof(sigprog));       

    /* Turn off caught signals */
    for (sigp= udata.u_sigvec; sigp < (udata.u_sigvec+NSIGS); ++sigp)
	if (*sigp != SIG_IGN) 
		*sigp = SIG_DFL;

    ifnot(p) 
        if (udata.u_ptab->p_pid != 1) /* init proc has not zombie2 ! */
    	        /* create zombie2 process */	  
		p=make_zombie();

    /* Read back the arguments and the environment */
    nargv = rargs(PROGTOP-2, abuf, &argc);
    nenvp = rargs((char *)(nargv), ebuf, NULL);

    /* Fill in udata.u_name */
    uget(ugetw(nargv),udata.u_name,8);

    brelse((char *)abuf);
    brelse((char *)ebuf);

    /* Shove argc and the address of nargv just below nenvp */
    uputw(argc, nenvp - 1);
    uputw(nargv, nenvp - 2);

    di();	
    /* Mark pages above program and below stack as illegal */
/** 
    This can't be done because the Z280 has no sep stack segment.
    So leave all untouched data pages VIRGPAGE.   SN
**/
    rdmmu(udata.u_page);
    for (j=8; j<16; ++j)
	if (udata.u_page[j] == VIRGPAGE)
	    udata.u_page[j] = NOPAGE;
    ldmmu(udata.u_page);

    /* copy name to zombie2 */
    if(p) {
    	bcopy(udata.u_name,p->p_ublk->u_d.u_name,8);	
    	p->p_status = P_ZOMBIE2; /* set back to zombie2 state */
    }	
    /* Go jump into the program, first setting the stack and
       copy the udata block */
    bcopy(&udata,udata.u_ptab->p_ublk, 512);
    ei();
    doexec((int16 *)(udata.u_isp = nenvp - 2));

nogood3:
    brelse((char *)abuf);
    brelse((char *)ebuf);
nogood2:
    brelse((char *)buf);	
nogood:
    i_deref(ino);
    return(-1);
}

#undef name
#undef argv
#undef envp

/* SN   TODO
	                 max (1024) 512 bytes for argv
		     and max  512 bytes for environ
*/ 
