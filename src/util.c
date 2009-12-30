#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "ice9.h"

char *FileName;
static int Errors=0;
int ErrorLimit=1;
int VerboseLevel=0;

static void warning(int l, char *note, char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  if (l <= 0)
    l = LINE;
  fprintf(stderr, "%sline %d: ", note, l);
  fprintf(stderr, fmt, ap);
}

void Warning(int l, char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  warning(l, "warning: ", fmt, ap);
  va_end(ap);
}

void Fatal(int l, char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  warning(l, "FATAL ERROR: ", fmt, ap);
  va_end(ap);
  if (++Errors == ErrorLimit)
    exit(-1);
}
void FatalS(int l, char *fmt, ...)
{
  if (DoSemantic) {
    va_list ap;
    va_start(ap,fmt);
    Fatal(l, fmt, ap);
    va_end(ap);
  }
}

void CompilerError(int l, char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  warning(l, "COMPILER ERROR: ", fmt, ap);
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
