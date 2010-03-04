/******************************************************************************
 *
 *  File Name........: cg.c
 *
 *  Description......:
 *
 *  Created by vin on 01/16/02
 *
 *  Revision History.:
 *
 *  $Log: cg.c,v $
 *  Revision 1.1  2002/01/18 20:51:55  vin
 *  Also needed files.
 *
 *
 *  $Id: cg.c,v 1.1 2002/01/18 20:51:55 vin Exp $
 *
 *****************************************************************************/

#include <assert.h>
#include "ice9.h"
#include "ast.h"
#include "type.h"
#include "yhelp.h"

static void cg(FILE *, Node);
extern Sig Gint, Gbool;
static char *ReturnArg="";

typedef enum
{
    /* RR instructions */
    HALT,		/* RR     halt, operands are ignored */
    IN,		      /* RR     read integer into reg(r); s and t are ignored */
    INB,		/* RR     read bool into reg(r); s and t are ignored */
    OUT,	/* RR     write integer from reg(r), s and t are ignored */
    OUTB,		/* RR     write bool from reg(r), s and t are ignored */
    OUTNL,		/* RR     write newline regs r, s and t are ignored */
    ADD,		/* RR     reg(r) = reg(s)+reg(t) */
    SUB,		/* RR     reg(r) = reg(s)-reg(t) */
    MUL,		/* RR     reg(r) = reg(s)*reg(t) */
    DIV,		/* RR     reg(r) = reg(s)/reg(t) */
    RRLim,		/* limit of RR opcodes */

    /* RM instructions */
    LD,			/* RM     reg(r) = mem(d+reg(s)) */
    ST,			/* RM     mem(d+reg(s)) = reg(r) */
    RMLim,		/* Limit of RM opcodes */

    /* RA instructions */
    LDA,		/* RA     reg(r) = d+reg(s) */
    LDC,		/* RA     reg(r) = d ; reg(s) is ignored */
    JLT,		/* RA     if reg(r)<0 then reg(7) = d+reg(s) */
    JLE,		/* RA     if reg(r)<=0 then reg(7) = d+reg(s) */
    JGT,		/* RA     if reg(r)>0 then reg(7) = d+reg(s) */
    JGE,		/* RA     if reg(r)>=0 then reg(7) = d+reg(s) */
    JEQ,		/* RA     if reg(r)==0 then reg(7) = d+reg(s) */
    JNE,		/* RA     if reg(r)!=0 then reg(7) = d+reg(s) */
    RALim,		/* Limit of RA opcodes */

} Inst;

char *opCodeTab[] = {
    "HALT", "IN", "INB", "OUT", "OUTB", "OUTNL", "ADD", "SUB", "MUL", "DIV", "????",
    /* RR opcodes */
    "LD", "ST", "????",		/* RM opcodes */
    "LDA", "LDC", "JLT", "JLE", "JGT", "JGE", "JEQ", "JNE", "????"
    /* RA opcodes */
};

static int localOffset=0;
static void resetLocals()
{
  localOffset = 0;
}

static int getLocal(int size)
{
  int prev = localOffset;
  localOffset += size;
  return prev;
}

static FILE *TM=NULL;

// registers
static int AC=3, AC2=4, ZERO=0, FP=6, PC=7;

static int outLine=1;

static int emitSkip() {
    return outLine++;
}

static int emit(int line, Inst in, int a, int b, int c, char *note)
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

  if (line)
    lineno = line;
  else
    lineno = outLine++;

  fprintf(TM, "%4d: %7s ", lineno, opCodeTab[in]);
  if (in < RRLim)
    fprintf(TM, "%d,%d,%d", a, b, c);
  else
    fprintf(TM, "%d,%d(%d)", a, b, c);

  if (note)
    fprintf(TM, "\t%s\n", note);
  else
    fputc('\n', TM);
  return lineno;
}

static int emitComment(char *comment)
{
  fprintf(TM, "* %s\n", comment);
  return 0;
}

static void cgType(FILE *out, Sig g)
{
  while (g->type == T_ARRAY) {
    g = g->under;
    assert(g);
  }
  switch (g->type) {
  case T_INT:
    //    fputs("T_int ", out);
    break;
  case T_BOOL:
    //    fputs("int ", out);
    break;
  case T_STR:
    //    fputs("char *", out);
    break;
  case T_NAME:
    //    fprintf(out, "T_%s ", g->sym->name);
    break;
  default:
    CompilerError(LINE, "invalid type: %d\n", g->type);
  }
}

static void cgArrayDecl(FILE *out, Sig g)
{
  assert(g);
  while (g->type == T_ARRAY) {
    //    fprintf(out, "[%d]", g->size);
    g = g->under;
  }
}

static void cgVarDecls(FILE *out, Node n)
{
  if (!n) return;
  Sig g = n->sig;
  for ( ; n != NULL; n = n->n_r) {
    cgType(out, g);
    //fprintf(out, "%s", n->n_str);
    cgArrayDecl(out, g);
    //    fputs(";\n", out);
  }
}

static void cgDecls(FILE *out, Node n)
{
  if (!n) return;
  switch (n->oper) {
  case O_SEQ:
    cgDecls(out, n->n_l);
    cgDecls(out, n->n_r);
    break;
  case O_ID:
    cgVarDecls(out, n);
    break;
  default:
    cg(out, n);
    break;
  }
}

static void cgArray(FILE *out, Sig g)
{
  assert(g);
  if (g->type == T_ARRAY) {
    //fprintf(out, "[%d]", g->size);
    cgArray(out, g->under);
  }
}

static void cg(FILE *out, Node n)
{
  if (!n) return;

  switch (n->oper) {
  case O_SEQ:
    if (n->n_l) {
      cg(out, n->n_l);
    }
    if (n->n_r) {
      cg(out, n->n_r);
    }
    break;
  case O_EXP:
    emitComment("O_EXP");
    cg(out, n->n_r);
    break;
  case O_ID:
    CompilerError(n->n_loc, "shouldn't have an ID in CG");
    break;
  case O_SYM:
    assert(!n->n_r);
    emitComment("O_SYM");
    emit(0, LD, AC, n->cg_value, FP, n->n_sym->name);
    //fprintf(out, "%s", n->n_sym->name);
    break;
  case O_ARR:
    {
      Node u;
      u = n->n_l;
      while (u->oper == O_ARR) {
	u = u->n_l;
      }
      cg(out, u);
      while (n->oper == O_ARR) {
	//fputs("[__bounds(", out);
	cg(out, n->n_r);
	//fprintf(out, ",%d,%d)]", n->n_l->sig->size, n->n_loc);
	n = n->n_l;
      }
    }
    break;
  case O_ILIT:
  case O_BLIT:
    assert(!n->n_r);
    emit(0, LDC, AC, n->n_int, 0, 
	 n->oper==O_ILIT ? "integer literal" : "boolean literal");
    //fprintf(out, "%dL", n->n_int);
    break;
  case O_SLIT:
    assert(!n->n_r);
    CompilerError(n->n_loc, "Strings not supported");
    //fprintf(out, "\"%s\"", n->n_str);
    break;
  case O_PROC:
    {
      Node p;
      //fputc('\n', out);
      // output type of function or void and set return arg
      if (n->n_l->n_r) {
	cgType(out, n->n_l->n_r->sig);
	ReturnArg = n->n_str;
      }
      else {
	//fputs("void ", out);
	ReturnArg = "";
      }
      // output name and arglist
      //      fprintf(out, "P_%s(", n->n_str);
      p = n->n_l->n_l;
      while (p) {
	Symbol m = p->n_sym;
	cgType(out, m->info);
	//fprintf(out, "%s", m->name);
	p = p->n_r;
	if (p)
	  //fputs(", ", out);
	  emitComment("...");
      }
      fputs(")\n{\n", out);
      if (n->n_l->n_r) {
	// output return variable declaration
	cgType(out, n->n_l->n_r->sig);
	//fprintf(out, "%s;\n", n->n_str);
      }
      // output local variables
      cgVarDecls(out, n->n_r->n_l);
      // output body
      cg(out, n->n_r->n_r);
      //fprintf(out,"return %s;}\n", ReturnArg);
    }
    break;
  case O_IF:
    cg(out, n->n_l);
    int elseJump = emitSkip();
    emitComment("then clause");
    cg(out, n->n_r->n_l);
    emit(elseJump, JEQ, AC, outLine - elseJump, PC, "cond jump over then");
    if (n->n_r->n_r) {
      elseJump = emitSkip();
      emitComment("else clause");
      cg(out, n->n_r->n_r);
      emit(elseJump, LDA, PC, outLine - elseJump, PC, "abs jump over else");
    }
    break;
  case O_DO:
    //    fputs("while (", out);
    cg(out, n->n_l);
    //fputs(") {\n", out);
    cg(out, n->n_r);
    //fputs("}\n", out);
    break;
  case O_FA:
    //fputs("{ int ", out);
    cg(out, n->n_l);
    //    fputs(";\nfor (", out);
    cg(out, n->n_l);
    //fputs(" = ", out);
    cg(out, n->n_r->n_l->n_l); // lb
    //fputs("; ", out);
    cg(out, n->n_l);
    //fputs(" <= ", out);
    cg(out, n->n_r->n_l->n_r); // lb
    //fputs("; ++", out);
    cg(out, n->n_l);
    //fputs(") {\n", out);
    cg(out, n->n_r->n_r);
    //fputs("}\n}\n", out);
    break;
  case O_EXIT:
    //fputs("exit(0);\n", out);
    break;
  case O_WRITE:
    cg(out, n->n_r);
    if (!sigCmp(n->n_r->sig, Gint))
      emit(0, OUT, AC, AC, AC, "write statment");
    else
      CompilerError(n->n_loc, "strings not supported");
    if (n->n_int) 
      emit(0, OUTNL, 0, 0, 0, "newline for write statment");
    break;
  case O_READ:
    emit(0, IN, AC, AC, AC, 0);
    break;
  case O_BREAK:
    //fputs("break;\n", out);
    break;
  case O_RETURN:
    //fprintf(out, "return ");
    //fprintf(out, "%s;\n", ReturnArg);
    break;
  case O_ASSIGN:
    cg(out, n->n_l);
    //fputs(" = ", out);
    cg(out, n->n_r);
    //fputs(";\n", out);
    break;
  case O_UNIOP:
    if (n->n_binop == B_SUB) {
      //fprintf(out, "( 0 -");
      cg(out, n->n_r);
      //fputc(')', out);
    }
    else if (n->n_binop == B_QUEST) {
      //fputc('(', out);
      cg(out, n->n_r);
      //fprintf(out, " == 1)");
    }
    else
      CompilerError(n->n_loc, "Invalid uniary op: %d\n", n->n_binop);
    break;
  case O_BINOP:
    if (n->n_l->sig == Gbool) {
      CompilerError(n->n_loc, "Boolean short circuit not implemented");
    }
    else {
      // integer binop
      cg(out, n->n_l);
      n->n_l->cg_value = getLocal(1);
      emit(0, ST, AC, n->n_l->cg_value, ZERO, "move lhs to temp");
      cg(out, n->n_r);
      emit(0, LD, AC2, n->n_l->cg_value, ZERO, "move lhs from temp");
      switch (n->n_binop) {
      case B_ADD:
	emit(0, ADD, AC, AC, AC2, "add");
	break;
      case B_SUB:
	emit(0, SUB, AC, AC, AC2, "sub");
	break;
      case B_MUL:
	emit(0, MUL, AC, AC, AC2, "mul");
	break;
      case B_DIV:
	emit(0, DIV, AC, AC, AC2, "div");
	break;
      default:
	CompilerError(n->n_loc, "invalid integer binop");
      }
    }
    break;
  case O_CALL:
    {
      Node p;
      //fputs("P_", out);
      cg(out, n->n_l);
      //fputc('(', out);
      p = n->n_r;
      if (p) {
	while (p->oper == O_SEQ) {
	  cg(out, p->n_l);
	  p = p->n_r;
	  if (p)
	    emitComment("...");
	}
	cg(out, p);
      }
      //fputs(")", out);
    }
    break;
  case O_FORWARD:
  case O_TYPE:
  case O_VAR:
    if (n->sig->type == T_ARRAY) {
      CompilerError(n->n_loc, "Arrays not supported");
    }
    emitComment("setting var");
    n->cg_value = getLocal(1);
  case O_ERR:
    CompilerError(n->n_loc, "invalid node in cg, oper=%d\n", n->oper);
    break;
  }
}

/*
  frame design

	-----------
	 param n
	-----------
	 ...
	-----------
	 param 2
	-----------
	 param 1
	-----------
	 prev fp
	-----------
	 ret value
 fp -> 	-----------
 */

void cgGen(FILE *out, Node n)
{
  if (!n)
    return;

#if 0
  fprintf(out, "/* generated by ice9 */\n");
  fprintf(out, "#include <errno.h>\n#include <stdlib.h>\n#include <stdio.h>\n#include <string.h>\n"
	  "typedef long T_int;\ntypedef int T_bool;\ntypedef char * T_string;\n");
  fprintf(out, "/* runtime */\n");
  fprintf(out, "T_int __bounds(T_int i, int m, int l) {\n"
	  "if (i < 0 || i >= m) {\n"
	  "fprintf(stderr, \"RUNTIME ERROR: line %%d: array bounds violation\\n\", l);exit(-1);}\n"
	  "return i;}\n");
  fprintf(out, "T_int __read(){\n");
  fprintf(out, "T_int val;\n");
  fprintf(out, "if (1 == scanf(\"%%ld\", &val)) return val; else exit(-1);\n}\n");
  fprintf(out, "T_int P_int(char *s) { return strtol(s, NULL, 10); }\n");
  fprintf(out, "T_string P_str(T_int n) {\nchar *bp, buf[512];\n"
   	  "snprintf(buf, 511, \"%%ld\", n);\n"
	  "bp = malloc(strlen(buf));\n"
	  "if (!bp) {fprintf(stderr, \"RUNTIME ERROR: malloc failed\\n\");exit(-1);}\n"
	  "strncpy(bp, buf, 511);\n"
	  "return bp;\n};\n");
  fprintf(out, "#define _WRITEI(i)\tprintf(\"%%ld\\n\", i)\n");
  fprintf(out, "#define _WRITESI(i)\tprintf(\"%%ld\", i)\n");
  fprintf(out, "#define _WRITES(s)\tprintf(\"%%s\\n\", s)\n");
  fprintf(out, "#define _WRITESS(s)\tprintf(\"%%s\", s)\n");
  fprintf(out, "\n");
  if (n->n_l) {
    fprintf(out, "/* global declarations */\n");
    cgDecls(out, n->n_l);
  }

  fprintf(out, "\n/* main program */\n");
  fprintf(out, "main()\n{\n");
  cg(out, n->n_r);
  fprintf(out, "}\n");
#endif  
  TM = stdout; // @@@ for debugging
  TM = out;

  cg(out, n->n_r);
  
  emit(0, HALT, 0, 0, 0, "end of file");
  fflush(out);
}
/*........................ end of cg.c ......................................*/
