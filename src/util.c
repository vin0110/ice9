#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ice9.h"

char *FileName;
static int Errors=0;
int ErrorLimit=1;
int VerboseLevel=0;

#define FIRST_PART(l,n) \
  do{if (l<=0) l = LINE;fprintf(stderr,"%sline %d: ",n,l);} while (0)

void Warning(int l, char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  FIRST_PART(l,"warning: ");
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

void Fatal(int l, char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  FIRST_PART(l,"FATAL ERROR: ");
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  if (++Errors == ErrorLimit)
    exit(-1);
}
void FatalS(int l, char *fmt, ...)
{
  if (DoSemantic) {
    va_list ap;
    va_start(ap,fmt);
    FIRST_PART(l,"FATAL ERROR: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (++Errors == ErrorLimit)
      exit(-1);
  }
}

void CompilerError(int l, char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  FIRST_PART(l,"COMPILER ERROR: ");
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  exit(-2);
}

int Verbose(int level, const char *fmt, ...) 
{
  int rc;

  if (VerboseLevel < level) return 0;
  va_list ap;
  va_start(ap,fmt);
  rc = printf(fmt,ap);
  va_end(ap);
  return rc;
}
