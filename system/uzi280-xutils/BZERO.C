
bzero(ptr,count)
char     *ptr;
unsigned   count;
{
/**
  memset(ptr,0,count);
**/
  *ptr = 0;
  bcopy(ptr,ptr+1,count-1);
}

