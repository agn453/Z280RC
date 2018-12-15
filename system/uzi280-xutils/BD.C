#include <stdio.h>
#include "unix.h"
#include "extern.h"

/* Block dump: to examine hard disk.

Usage:  bd dev blkno

************************************************** */

char buf[512];

main(argc,argv)
int argc;
char *argv[];
{
    int i,j;
    unsigned blkno;
    int dev;

    if (argc != 3 || !isdigit(argv[1][0]))
    {
	fprintf(stderr,"Usage: bd device blkno\n");
	exit(1);
    }

    bufinit();
    dev = atoi(argv[1]);
    blkno = atoi(argv[2]);
    d_open(dev);

    dread(dev,blkno,buf);

    for (i=0; i < 512/24; ++i)
    {
	printf("%4x  ",24*i);
	for (j=0; j < 24; ++j)
	{
	    if (( buf[24*i+j]&0x00ff) < 16)
	        printf("0%1x ",buf[24*i + j] & 0x00ff);
	    else
	        printf("%2x ",buf[24*i + j] & 0x00ff);
	}
	printf("\n");
    }  
    
    exit(0);
}



dread(dev, blk, addr)
int dev;
uint16 blk;
char *addr;
{
    char *buf;
    char *bread();

    buf = bread(dev, blk, 0);
    bcopy(buf, addr, 512);
    bfree(buf, 0);
}
