#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int read(){
char _readbuf[512]; int i;
if (!fgets(_readbuf, 512, stdin)) { puts("READ ERROR"); exit(-1);}
errno = 0;
 i = strtol(_readbuf, NULL, 10);if (errno) perror("CONVERSION ERROR");
return i;
}

main()
{
read();
}
