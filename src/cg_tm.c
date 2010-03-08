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
#include <string.h>
#include "ice9.h"
#include "ast.h"
#include "type.h"
#include "yhelp.h"

static void cg(FILE *, Node);
extern Sig Gint, Gbool;
extern int GlobalAR;
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

static int procOffset=0;
static int dataOffset=1;
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

static FILE *TMF=NULL;

// registers

static int AC=0, AC2=1, AC3=2, AC4=3, ZERO=4, GP=5, FP=6, PC=7;

static int outLine=0;

static int emitSkip() {
    return outLine++;
}

static int emitBackPatch(int line, Inst in, int a, int b, int c, char *note) 
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

static int emit(Inst in, int a, int b, int c, char *note) 
{
  return emitBackPatch(-1, in, a, b, c, note);
}

static int emitComment(char *comment)
{
  fprintf(TMF, "* %s\n", comment);
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

static void cgDecls(FILE *out, Node n)
{
  if (!n) return;
  switch (n->oper) {
  case O_SEQ:
    cgDecls(out, n->n_l);
    cgDecls(out, n->n_r);
    break;
  case O_ID:
    break;
  default:
    cg(out, n);
    break;
  }
}

static int cgFormals(Node n)
{
  if (!n) return 0;

  assert(n->oper == O_SYM);
  printf("cgFormals: %s\n", n->n_sym->name);
  n->cg_value = procOffset++;
  return 1 + cgFormals(n->n_r);
}

static int cgLocals(Node n)
{
  if (!n) return 0;

  assert(n->oper == O_VAR);
  printf("cgLocals: %s\n", n->n_l->n_sym->name);
  n->cg_value = procOffset++;
  return 1 + cgLocals(n->n_r);
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
    emitComment("SEQ");
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
    emitComment("O_ID");
    CompilerError(n->n_loc, "shouldn't have an ID in CG\n");
    break;
  case O_SYM:
    emitComment("O_SYM");
    { int reg = FP;
      assert(!n->n_r);
      emitComment("O_SYM");
      printf("sym %s level %d loc %d\n", n->n_sym->name, n->n_sym->level, n->n_sym->location);
      if (n->n_sym->level == 1)	/* global variable */
	reg = GP;
      emit(LD, AC, n->cg_value, reg, n->n_sym->name);
      //fprintf(out, "%s", n->n_sym->name);
    }
    break;
  case O_ARR:
    emitComment("O_ARR");
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
    emitComment("O_I/BLIT");
    assert(!n->n_r);
    emit(LDC, AC, n->n_int, 0, 
	 n->oper==O_ILIT ? "integer literal" : "boolean literal");
    //fprintf(out, "%dL", n->n_int);
    break;
  case O_SLIT:
    emitComment("O_SLIT");
    assert(!n->n_r);
    fprintf(out, ".DATA %d\n", strlen(n->n_str));
    fprintf(out, ".SDATA \"%s\"\n", n->n_str);
    emit(LDC, AC, dataOffset, 0, "load string address");
    dataOffset += strlen(n->n_str);
    break;
  case O_PROC:
    emitComment("O_PROC");
    /*
      proc node n
      n->n_l->n_l :: formal param list (walk the n_r kids)
      n->n_l->n_r :: return type
      n->n_r->n_l :: local param list (walk the n_r kids)
      n->n_r->n_r :: body of proc
     */
    emitComment("procedure declaration");
    emitComment(n->n_str);
    procOffset = cgFormals(n->n_l->n_l) + cgLocals(n->n_r->n_l);
    // output body
    cg(out, n->n_r->n_r);
    n->cg_value = procOffset;
    //fprintf(out,"return %s;}\n", ReturnArg);
    break;
  case O_IF:
    emitComment("O_IF");
    cg(out, n->n_l);
    int elseJump = emitSkip();
    emitComment("then clause");
    cg(out, n->n_r->n_l);
    emitBackPatch(elseJump, JEQ, AC, outLine - elseJump, PC, 
		  "cond jump over then");
    if (n->n_r->n_r) {
      elseJump = emitSkip();
      emitComment("else clause");
      cg(out, n->n_r->n_r);
      emitBackPatch(elseJump, LDA, PC, outLine - elseJump, PC, 
		    "abs jump over else");
    }
    break;
  case O_DO:
    emitComment("O_DO");
    //    fputs("while (", out);
    cg(out, n->n_l);
    //fputs(") {\n", out);
    cg(out, n->n_r);
    //fputs("}\n", out);
    break;
  case O_FA:
    emitComment("O_FA");
    //fputs("{ int ", out);
    //cg(out, n->n_l);
    n->n_l->cg_value = procOffset++;
    //    fputs(";\nfor (", out);
    //cg(out, n->n_l);
    //fputs(" = ", out);
    emitComment("lower bound");
    cg(out, n->n_r->n_l->n_l);
    int top = emit(ST, AC, n->n_l->cg_value, FP, "'i' <- lb");
    //fputs("; ", out);
    //cg(out, n->n_l);
    //fputs(" <= ", out);
    //cg(out, n->n_r->n_l->n_r); // ub
    emit(LDC, AC2, n->n_r->n_l->n_r->n_int, 0, "get ub");
    emit(SUB, AC, AC, AC2, "'i' - ub");
    int jump = emitSkip(1);
    //fputs("; ++", out);
    //cg(out, n->n_l);
    //fputs(") {\n", out);
    emitComment("fa loop body");
    cg(out, n->n_r->n_r);
    //fputs("}\n}\n", out);
    emitComment("fa loop tail");
    emit(LD, AC, n->n_l->cg_value, FP, "get 'i'");
    emit(LDA, AC, 1, AC, "'i'++");
    emit(LDA, PC, top - outLine - 1, PC, "loop back to top");
    emitBackPatch(jump, JGT, AC, outLine, ZERO, "jump out of fa loop");
    break;
  case O_EXIT:
    emitComment("O_EXIT");
    //fputs("exit(0);\n", out);
    break;
  case O_WRITE:
    emitComment("O_WRITE");
    cg(out, n->n_r);
    if (!sigCmp(n->n_r->sig, Gint))
      emit(OUT, AC, AC, AC, "write statment");
    else
      CompilerError(n->n_loc, "strings not supported");
    if (n->n_int) 
      emit(OUTNL, 0, 0, 0, "newline for write statment");
    break;
  case O_READ:
    emitComment("O_READ");
    emit(IN, AC, AC, AC, 0);
    break;
  case O_BREAK:
    emitComment("O_BREAK");
    //fputs("break;\n", out);
    break;
  case O_RETURN:
    emitComment("O_RETURN");
    //fprintf(out, "return ");
    //fprintf(out, "%s;\n", ReturnArg);
    break;
  case O_ASSIGN:
    emitComment("O_ASSIGN");
    cg(out, n->n_l);
    //fputs(" = ", out);
    cg(out, n->n_r);
    //fputs(";\n", out);
    break;
  case O_UNIOP:
    emitComment("O_UNIOP");
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
    emitComment("O_BINOP");
    if (n->n_l->sig == Gbool) {
      CompilerError(n->n_loc, "Boolean short circuit not implemented");
    }
    else {
      // integer binop
      cg(out, n->n_l);
      n->n_l->cg_value = getLocal(1);
      emit(ST, AC, n->n_l->cg_value, ZERO, "move lhs to temp");
      cg(out, n->n_r);
      emit(LD, AC2, n->n_l->cg_value, ZERO, "move lhs from temp");
      switch (n->n_binop) {
      case B_ADD:
	emit(ADD, AC, AC, AC2, "add");
	break;
      case B_SUB:
	emit(SUB, AC, AC, AC2, "sub");
	break;
      case B_MUL:
	emit(MUL, AC, AC, AC2, "mul");
	break;
      case B_DIV:
	emit(DIV, AC, AC, AC2, "div");
	break;
      default:
	CompilerError(n->n_loc, "invalid integer binop");
      }
    }
    break;
  case O_CALL:
    emitComment("O_CALL");
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
    emitComment("O_DECL");
    break;
  case O_VAR:
    emitComment("O_VAR");
    break;
  case O_ERR:
    CompilerError(n->n_loc, "invalid node in cg, oper=%d\n", n->oper);
    break;
  }
}

void emitIntProc()
{
  return;
}

/*
  frame design

	--------
	 ret value
  fp ->	--------
 	 a1
	--------
	 ...
	--------
 	 an
	--------
         temp1
	--------
         ...
	--------
         tempn
	--------

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
  TMF = stdout; // @@@ for debugging
  TMF = out;

  // preamble

  int initJump = emitSkip(1);
  emitComment("BEGIN builtins");
  emitIntProc();
  emitComment("END builtins");
  
  if (n->n_l)
    emitComment("local procs");
  cgDecls(out, n->n_l);

  emitComment("BEGIN preamble");
  emitBackPatch(initJump, LDA, PC, outLine, PC, "jump over local procs and builtins");
  emit(LD, FP, 0, 0, "load top of physical memory into fp");
  emit(LDC, 0, dataOffset, 0, "load top of heap");
  emit(ST, 0, 0, ZERO, "store top of heap in memory");
  emit(LDC, FP, GlobalAR, 0, "increment FP to accommodate global AR");
  emitComment("END preamble");
  

  cg(out, n->n_r);
  
  emit(HALT, 0, 0, 0, "end of file");
  fflush(out);
}
/*........................ end of cg.c ......................................*/
