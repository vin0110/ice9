/******************************************************************************
 *
 *  File Name........: emit.c
 *
 *  Description......:
 *
 *  Created by vin on 03/08/10
 *
 *  Revision History.:
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "emit.h"
#include "ice9.h"

static int dataOffset=1;
static FILE *TMF=NULL;
static int outLine=0;

char *opCodeTab[] = {
    "HALT", "IN", "INB", "OUT", "OUTB", "OUTNL", "ADD", "SUB", "MUL", "DIV", "????",
    /* RR opcodes */
    "LD", "ST", "????",		/* RM opcodes */
    "LDA", "LDC", "JLT", "JLE", "JGT", "JGE", "JEQ", "JNE", "????"
    /* RA opcodes */
};

void emitFile(FILE *f)
{
  TMF = f;
}

int emitSkip(int insts) {
  // use insts = to get current line number
  int r = outLine;
  outLine += insts;
  return r;
}

int emitBackPatch(int line, Inst in, int a, int b, int c, char *note) 
{
  int lineno;

  switch (in) {
  case RRLim:
  case RMLim:
  case RALim:
    CompilerError(-1, "invalid inst opcode %d\n", (int)in);
    return -1;
  default:
    break;
  }

  if (line < 0)
    lineno = outLine++;
  else
    lineno = line;

  fprintf(TMF, "%4d: %7s ", lineno, opCodeTab[in]);
  if (in < RRLim)
    fprintf(TMF, "%d,%d,%d", a, b, c);
  else
    fprintf(TMF, "%d,%d(%d)", a, b, c);

  if (note)
    fprintf(TMF, "\t%s\n", note);
  else
    fputc('\n', TMF);
  return lineno;
}

int emit(Inst in, int a, int b, int c, char *note) 
{
  return emitBackPatch(-1, in, a, b, c, note);
}

int emitComment(char *comment)
{
  fprintf(TMF, "* %s\n", comment);
  return 0;
}

int emitData(int d, char *note)
{
  fprintf(TMF, ".DATA %4d\t%s\n", d, note);
  return dataOffset++;
}

int emitSData(char *s, char *note)
{
  int r = dataOffset;
  dataOffset += strlen(s);
  fprintf(TMF, ".SDATA \"%s\"\t%s\n", s, note);
  return r;
}

int emitDataOffset() {
  return dataOffset;
}
/*........................ end of emit.c ....................................*/
