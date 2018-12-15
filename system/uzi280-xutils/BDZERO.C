#include <stdio.h>
#include "unix.h"
#include "extern.h"

/* zero out hard disk block.

Usage:  bd dev blkno

************************************************** */


main(argc,argv)
int argc;
char *argv[];
{
    unsigned blkno;
    int dev,i,isdigit();
    char *buf,*bread(); 

    if (argc != 3 || !isdigit(argv[1][0]))
    {
	fprintf(stderr,"Usage: bd device blkno\n");
	exit(1);
    }

    bufinit();
    dev = atoi(argv[1]);
    blkno = atoi(argv[2]);
    d_open(dev);
  
    buf = bread(dev,blkno,1);
    for (i=0;i<512;i++)
	buf[i]=0;	
    bfree(buf, 2);
    exit(0);
}

