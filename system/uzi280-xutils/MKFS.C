#include <stdio.h>
#include "unix.h"
#define MAINpro
#include "extern.h"
#undef MAINpro

/* This makes a filesystem */

#ifndef CPM
#define CPM
#endif

int dev;


static char _zerobuf[512];


struct direct dirbuf[32] = { ROOTINODE,".",ROOTINODE,".." };
static struct dinode inode[8];
static struct filesys filsys;



main(argc,argv)
int argc;
char *argv[];
{
    uint16 fsize, isize;

    if (argc != 4)
    {
	printf("Usage: mkfs device isize fsize\n");
	exit(-1);
    }

    dev = atoi(argv[1]);
    isize = (uint16)atoi(argv[2]);
    fsize = (uint16)atoi(argv[3]);

    if (dev == 0 && argv[1][0] != '0')
    {
	printf("Invalid device\n");
	exit(-1);
    }
    if (dev < 0 || dev >= NDEVS || (dev == 1))	/*AGN Disallow floppy dev 1 */
    {
	printf("Invalid device\n");
	exit(-1);
    }

    if (fsize < 3 || isize < 2 || isize >= fsize)
    {
	printf("Bad parameter values\n");
	exit(-1);
    }


    printf("Making filesystem on device %d with isize %u fsize %u. Confirm? ",
	    dev,isize,fsize);
    if (!yes())
	exit(-1);

    bufinit();
    if (d_open(dev))
    {
	printf("Can't open device");
	exit(-1);
    }

    mkfs(fsize,isize);
    bufsync();
    exit(0);
}




mkfs(fsize, isize)
uint16 fsize, isize;
{
    uint16 j;

    /* Zero out the blocks */

#ifdef _FULL_
    for (j=0; j < fsize; ++j)
	dwrite(j,(char *)_zerobuf);
#endif

    for (j=0; j < isize; ++j)    /*Don't waste time in CPM*/
	dwrite(j,(char *)_zerobuf);

    /* Initialize the super-block */
    filsys.s_mounted = SMOUNTED; /* Magic number */
    filsys.s_isize =  isize;
    filsys.s_fsize =  fsize;
    filsys.s_nfree =  1;
    filsys.s_free[0] =  0;
    filsys.s_tfree =  0;
    filsys.s_ninode = 0;
    filsys.s_tinode = (8 * (isize-2)) - 2;

    /* Free each block, building the free list */

    for (j= fsize-1; j > isize; j--)
    {

	if (filsys.s_nfree == 50)
	{
	    dwrite(j, (char *)&filsys.s_nfree);
	    filsys.s_nfree = 0;
	}
	++filsys.s_tfree;
	filsys.s_free[(filsys.s_nfree)++] = j;
    }


    /* The inodes are already zeroed out */
    /* create the root dir */

    inode[ROOTINODE].i_mode = F_DIR | (0777 & MODE_MASK);
    inode[ROOTINODE].i_nlink = 3;
    inode[ROOTINODE].i_size.o_blkno = 0;
    inode[ROOTINODE].i_size.o_offset = 32;
    inode[ROOTINODE].i_addr[0] = isize;

    /* Reserve reserved inode */
    inode[0].i_nlink = 1;
    inode[0].i_mode = ~0;

    dwrite(2,(char *)inode);

    dwrite(isize,(char *)dirbuf);

    /* Write out super block */
    dwrite(1,(char *)&filsys);
    
    bufsync();
}



dwrite(blk, addr)
uint16 blk;
char *addr;
{
    char *buf;
    char *bread();

    buf = bread(dev, blk, 1);
    bcopy(addr, buf, 512);
    bfree(buf, 1);
}
    

int yes()
{
    char line[20];

    if (!fgets(line,sizeof(line),stdin) || (*line != 'y' && *line != 'Y'))
	return(0);

    return(1);
}

