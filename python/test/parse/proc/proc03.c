/* generated by ice9 */
/* runtime */
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
typedef int T_int;
typedef char * T_string;
int __read(){
char _readbuf[512]; int i;
if (!fgets(_readbuf, 512, stdin)) { puts("READ ERROR"); exit(-1);}
errno = 0;
 i = strtol(_readbuf, NULL, 10);if (errno) perror("CONVERSION ERROR");
return i;
}
T_int P_int(char *s) { return strtol(s, NULL, 10); }
T_string P_str(int n) {
char *bp, buf[512];
snprintf(buf, 511, "%d", n);
bp = malloc(strlen(buf));
if (!bp) {fprintf(stderr, "RUNTIME ERROR: malloc failed\n");exit(-1);}
strncpy(bp, buf, 511);
return bp;
};
#define _WRITEI(i)	printf("%d\n", i)
#define _WRITESI(i)	printf("%d", i)
#define _WRITES(s)	printf("%s\n", s)
#define _WRITESS(s)	printf("%s", s)

/* global declarations */
int
/* main program */
main()
{
}