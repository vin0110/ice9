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
#include "emit.h"

static void cg(Node);
extern Sig Gint, Gbool, Gstr;
extern int GlobalAR;
static char *ReturnArg="";

#define DEBUG 1

// registers

static int AC=0, AC0=0, AC1=1, AC2=2, AC3=3, ZERO=4, TP=5, FP=6, PC=7;

static int push(int r, char *note)
{
  emit(ST, r, 0, TP, note);
  emit(LDA, TP, 1, TP, "push");
  return 0;
}

static int pop(int r, char *note)
{
  emit(LDA, TP, -1, TP, "pop");
  emit(LD, r, 0, TP, note);
  return 0;
}

static void cgDecls(Node n)
{
  if (!n) return;
  switch (n->oper) {
  case O_SEQ:
    cgDecls(n->n_l);
    cgDecls(n->n_r);
    break;
  case O_SYM:
    if (sigCmp(n->sig, Gint) || sigCmp(n->sig, Gbool)) {
      int i;
      assert(n->sig->size > 0);
      n->n_sym->location = emitData(0, n->n_sym->name);
      for (i = 1; i < n->sig->size; i++) {
	emitData(0, "");
      }
    }
    else if (sigCmp(n->sig, Gstr)) {
      CompilerError(n->n_loc, "Not implemented\n");
    }
    break;
  default:
    cg(n);
    break;
  }
}
static void cgLval(Node n)
{
  Symbol sym;
  char buffer[512];
  assert(n->oper == O_SYM);
  assert(!n->n_r);
  sym = n->n_sym;
  sprintf(buffer, "lvalue: address of %s", sym->name);
  emit(LDA, AC, sym->location, sym->level == 1 ? ZERO : FP, buffer);
}

static void cgArray(Sig g)
{
  assert(g);
  if (g->type == T_ARRAY) {
    //fprintf(out, "[%d]", g->size);
    cgArray(g->under);
  }
}

static void cg(Node n)
{
  if (!n) return;

  switch (n->oper) {
  case O_SEQ:
    #if DEBUG
    emitComment("SEQ");
    #endif
    if (n->n_l) {
      cg(n->n_l);
    }
    if (n->n_r) {
      cg(n->n_r);
    }
    break;
  case O_EXP:
    #if DEBUG
    emitComment("O_EXP");
    #endif
    cg(n->n_r);
    break;
  case O_ID:
    #if DEBUG
    emitComment("O_ID");
    #endif
    CompilerError(n->n_loc, "shouldn't have an ID in CG\n");
    break;
  case O_SYM:
    #if DEBUG
    emitComment("O_SYM");
    #endif
    emit(LD, AC, n->n_sym->location, n->n_sym->level == 1 ? ZERO : FP, 
	 n->n_sym->name);
    break;
  case O_ARR:
    #if DEBUG
    emitComment("O_ARR");
    #endif
    {
      Node u;
      u = n->n_l;
      while (u->oper == O_ARR) {
	u = u->n_l;
      }
      cg(u);
      while (n->oper == O_ARR) {
	//fputs("[__bounds(", out);
	cg(n->n_r);
	//fprintf(out, ",%d,%d)]", n->n_l->sig->size, n->n_loc);
	n = n->n_l;
      }
    }
    CompilerError(n->n_loc, "Not implemented\n");
    break;
  case O_ILIT:
  case O_BLIT:
    #if DEBUG
    emitComment("O_I/BLIT");
    #endif
    assert(!n->n_r);
    emit(LDC, AC, n->n_int, 0, 
	 n->oper==O_ILIT ? "integer literal" : "boolean literal");
    break;
  case O_SLIT:
    #if DEBUG
    emitComment("O_SLIT");
    #endif
    assert(!n->n_r);
    n->cg_value = emitData(strlen(n->n_str), n->n_str);
    emitSData(n->n_str, n->n_str);
    emit(LDC, AC, emitDataOffset(), 0, "load string address");
    break;
  case O_PROC:
    #if DEBUG
    emitComment("O_PROC");
    #endif
    /*
      proc node n
      n->n_l->n_l :: formal param list (walk the n_r kids)
      n->n_l->n_r :: return type
      n->n_r->n_l :: local param list (walk the n_r kids)
      n->n_r->n_r :: body of proc
     */
    emitComment("procedure declaration");
    emitComment(n->n_str);
    // output body
    n->cg_loc = emitSkip(0);
    cg(n->n_r->n_r);
    //fprintf(out,"return %s;}\n", ReturnArg);
    CompilerError(n->n_loc, "Not implemented\n");
    break;
  case O_IF:
    #if DEBUG
    emitComment("O_IF");
    #endif
    cg(n->n_l);
    int elseJump = emitSkip(1);
    emitComment("then clause");
    cg(n->n_r->n_l);
    emitBackPatch(elseJump, JEQ, AC, emitSkip(0) - elseJump, PC, 
		  "cond jump over then");
    if (n->n_r->n_r) {
      elseJump = emitSkip(1);
      emitComment("else clause");
      cg(n->n_r->n_r);
      emitBackPatch(elseJump, LDA, PC, emitSkip(0) - elseJump, PC, 
		    "abs jump over else");
    }
    break;
  case O_DO:
    #if DEBUG
    emitComment("O_DO");
    #endif
    cg(n->n_l);
    cg(n->n_r);
    break;
  case O_FA:
    #if DEBUG
    emitComment("O_FA");
    #endif
    emitComment("lower bound");
    cg(n->n_r->n_l->n_l);
    int top = emit(ST, AC, n->n_l->n_sym->location, FP, "'i' <- lb");
    emit(LDC, AC1, n->n_r->n_l->n_r->n_int, 0, "get ub");
    emit(SUB, AC, AC, AC1, "'i' - ub");
    int jump = emitSkip(1);
    emitComment("fa loop body");
    cg(n->n_r->n_r);
    emitComment("fa loop tail");
    emit(LD, AC, n->n_l->n_sym->location, FP, "get 'i'");
    emit(LDA, AC, 1, AC, "'i'++");
    emit(LDA, PC, top - emitSkip(0) - 1, PC, "loop back to top");
    emitBackPatch(jump, JGT, AC, emitSkip(0), ZERO, "jump out of fa loop");
    break;
  case O_EXIT:
    #if DEBUG
    emitComment("O_EXIT");
    #endif
    emit(HALT, 0, 0, 0, "exit statement");
    break;
  case O_WRITE:
    #if DEBUG
    emitComment("O_WRITE");
    #endif
    cg(n->n_r);
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
    #if DEBUG
    emitComment("O_BREAK");
    #endif
    CompilerError(n->n_loc, "Not implemented\n");
    break;
  case O_RETURN:
    #if DEBUG
    emitComment("O_RETURN");
    #endif
    CompilerError(n->n_loc, "Not implemented\n");
    break;
  case O_ASSIGN:
    #if DEBUG
    emitComment("O_ASSIGN");
    #endif
    cgLval(n->n_l);
    push(AC, "lvalue");
    //fputs(" = ", out);
    cg(n->n_r);
    pop(AC1, "lvalue");
    emit(ST, AC, 0, AC1, "assign");
    break;
  case O_UNIOP:
    #if DEBUG
    emitComment("O_UNIOP");
    #endif
    if (n->n_binop == B_SUB) {
      //fprintf(out, "( 0 -");
      cg(n->n_r);
      emit(SUB, AC, ZERO, AC, "uniary minus");
    }
    else if (n->n_binop == B_QUEST) {
      cg(n->n_r);
    }
    else
      CompilerError(n->n_loc, "Invalid uniary op: %d\n", n->n_binop);
    break;
  case O_BINOP:
    #if DEBUG
    emitComment("O_BINOP");
    #endif
    if (n->n_l->sig == Gbool && n->n_binop == B_ADD) {
      CompilerError(n->n_loc, "Boolean short circuit not implemented\n");
    }
    else if (n->n_l->sig == Gbool && n->n_binop == B_MUL) {
      CompilerError(n->n_loc, "Boolean short circuit not implemented\n");
    }
    else {
      // integer binop
      cg(n->n_l);
      push(AC, "lhs of binop");
      cg(n->n_r);
      pop(AC1, "lhs of binop");
      switch (n->n_binop) {
      case B_ADD:
	emit(ADD, AC, AC, AC1, "add");
	break;
      case B_SUB:
	emit(SUB, AC, AC, AC1, "sub");
	break;
      case B_MUL:
	emit(MUL, AC, AC, AC1, "mul");
	break;
      case B_DIV:
	emit(DIV, AC, AC, AC1, "div");
	break;
      case B_MOD:
	CompilerError(n->n_loc, "Boolean short circuit not implemented\n");
	break;
      case B_NEQ:
	emit(SUB, AC, AC, AC1, "int EQ int");
	int f = emitSkip(1);
	emit(LDC, AC, 1, 0, "set to true");
	emitBackPatch(f, JEQ, AC, 1, PC, "leave false");
	break;
      case B_EQ:
      case B_GT:
      case B_LT:
      case B_GE:
      case B_LE:
	emit(SUB, AC, AC1, AC, "int EQ int");
	int t = emitSkip(1);
	emit(LDC, AC, 0, 0, "set to false");
	emit(LDA, PC, 1, PC, "jump over true");
	Inst op;
	switch (n->n_binop) {
	case B_EQ: op = JEQ; break;
	case B_GT: op = JGT; break;
	case B_LT: op = JLT; break;
	case B_GE: op = JGE; break;
	case B_LE: op = JLE; break;
	default:
	  CompilerError(n->n_loc, "actually a C compiler error\n");
	}
	emitBackPatch(t, op, AC, 2, PC, "go to true");
	emit(LDC, AC, 1, 0, "set to true");
	break;
      default:
	CompilerError(n->n_loc, "invalid integer binop\n");
      }
    }
    break;
  case O_CALL:
    #if DEBUG
    emitComment("O_CALL");
    #endif
    {
      Node p;
      //fputs("P_", out);
      //cg(n->n_l);
      //fputc('(', out);
      p = n->n_r;
      if (p) {
	while (p->oper == O_SEQ) {
	  cg(p->n_l);
	  p = p->n_r;
	  if (p)
	    emitComment("...");
	}
	cg(p);
      }
      //fputs(")", out);
    }
    break;
  case O_FORWARD:
  case O_TYPE:
    #if DEBUG
    emitComment("O_DECL");
    #endif
    break;
  case O_VAR:
    #if DEBUG
    emitComment("O_VAR");
    #endif
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
  cg(n->n_r);
  fprintf(out, "}\n");
#endif  
  emitFile(stdout); // @@@ for debugging
  emitFile(out);

  // preamble

  int initJump = emitSkip(1);
  #if DEBUG
  emitComment("BEGIN builtins");
  #endif
  emitIntProc();
  #if DEBUG
  emitComment("END builtins");
  #endif
  
  if (n->n_l)
    emitComment("local procs");
  cgDecls(n->n_l);

  emitComment("BEGIN preamble");
  emitBackPatch(initJump, LDA, PC, emitSkip(0), PC, "jump over local procs and builtins");
  emit(LD, FP, 0, 0, "load top of physical memory into fp");
  emit(LDC, 0, emitDataOffset(), 0, "load top of heap");
  emit(ST, 0, 0, ZERO, "store top of heap in memory");
  emit(LDC, FP, GlobalAR, 0, "increment FP to accommodate global AR");
  emitComment("END preamble");
  

  cg(n->n_r);
  
  emit(HALT, 0, 0, 0, "end of file");
  fflush(out);
}
/*........................ end of cg.c ......................................*/
