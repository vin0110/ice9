#include "stdio.h"
char *FileName;

void Fatal(int l, char *s)
{
  fprintf(stderr, "%s: line %d: %s\n", FileName, l, s);
  exit(-1);
}

