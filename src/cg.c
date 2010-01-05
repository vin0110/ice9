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

static void cgType(FILE *out, Sig g)
{
  while (g->type == T_ARRAY) {
    g = g->under;
    assert(g);
  }
  switch (g->type) {
  case T_INT:
    fputs("T_int ", out);
    break;
  case T_BOOL:
    fputs("int ", out);
    break;
  case T_STR:
    fputs("char *", out);
    break;
  case T_NAME:
    fprintf(out, "T_%s ", g->sym->name);
    break;
  default:
    CompilerError(LINE, "invalid type: %d\n", g->type);
  }
}

static void cgArrayDecl(FILE *out, Sig g)
{
  assert(g);
  while (g->type == T_ARRAY) {
    fprintf(out, "[%d]", g->size);
    g = g->under;
  }
}

static void cgVarDecls(FILE *out, Node n)
{
  if (!n) return;
  Sig g = n->sig;
  for ( ; n != NULL; n = n->n_r) {
    cgType(out, g);
    fprintf(out, "%s", n->n_str);
    cgArrayDecl(out, g);
    fputs(";\n", out);
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
    fprintf(out, "[%d]", g->size);
    cgArray(out, g->under);
  }
}

static char *cgBinop(Node n)
{
  Binop b = n->n_binop;
  switch (b) {
  case B_ADD:
    if (sigCmp(n->sig, Gint))
      return "||";
    else
      return "+";
  case B_SUB:	return "-";
  case B_MUL:
    if (sigCmp(n->sig, Gint))
      return "&&";
    else
      return "*";
  case B_DIV:	return "/";
  case B_MOD:	return "%";
  case B_EQ:	return "==";
  case B_NEQ:	return "!=";
  case B_LT:	return "<";
  case B_GT:	return ">";
  case B_LE:	return "<=";
  case B_GE:	return ">=";
  default:    break;
  }
  CompilerError(LINE, "invalid binop: %d\n", b);
  return "ERR";
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
    cg(out, n->n_r);
    if (n->n_xtra)
      fputs(";\n", out);
    break;
  case O_ID:
    fprintf(out, "%s", n->n_str);
    break;
  case O_SYM:
    assert(!n->n_r);
    fprintf(out, "%s", n->n_sym->name);
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
	fputs("[__bounds(", out);
	cg(out, n->n_r);
	fprintf(out, ",%d,%d)]", n->n_l->sig->size, n->n_loc);
	n = n->n_l;
      }
    }
    break;
  case O_ILIT:
    assert(!n->n_r);
    fprintf(out, "%dL", n->n_int);
    break;
  case O_SLIT:
    assert(!n->n_r);
    fprintf(out, "\"%s\"", n->n_str);
    break;
  case O_BLIT:
    assert(!n->n_r);
    fprintf(out, "%d", n->n_int);
    break;
  case O_PROC:
    {
      Node p;
      fputc('\n', out);
      // output type of function or void and set return arg
      if (n->n_l->n_r) {
	cgType(out, n->n_l->n_r->sig);
	ReturnArg = n->n_str;
      }
      else {
	fputs("void ", out);
	ReturnArg = "";
      }
      // output name and arglist
      fprintf(out, "P_%s(", n->n_str);
      p = n->n_l->n_l;
      while (p) {
	Symbol m = p->n_sym;
	cgType(out, m->info);
	fprintf(out, "%s", m->name);
	p = p->n_r;
	if (p)
	  fputs(", ", out);
      }
      fputs(")\n{\n", out);
      if (n->n_l->n_r) {
	// output return variable declaration
	cgType(out, n->n_l->n_r->sig);
	fprintf(out, "%s;\n", n->n_str);
      }
      // output local variables
      cgVarDecls(out, n->n_r->n_l);
      // output body
      cg(out, n->n_r->n_r);
      fprintf(out,"return %s;}\n", ReturnArg);
    }
    break;
  case O_FORWARD:
    {
      Node p;
      fputc('\n', out);
      if (n->n_l->n_r)
	cgType(out, n->n_l->n_r->sig);
      else
	fputs("void ", out);
      fprintf(out, "%s(", n->n_str);
      p = n->n_l->n_l;
      while (p) {
	Symbol m = p->n_sym;
	cgType(out, m->info);
	p = p->n_r;
	if (p)
	  fputs(", ", out);
      }
      fputs(");\n", out);
    }
    break;
    break;
  case O_TYPE:
    fprintf(out, "typedef ");
    cgType(out, n->sig);
    fprintf(out, "T_%s", n->n_str);
    cgArray(out, n->sig);
    fputs(";\n", out);
    break;
  case O_VAR:
    { 
      Node l;
      cgType(out, n->sig);
      l = n->n_r;
      while (l) {
	fprintf(out, "%s", l->n_str);
	cgArray(out, n->sig);
	l = l->n_r;
	if (l)
	  fputs(", ", out);
      }
      fputs(";\n", out);
    }
    break;
  case O_IF:
    fputs("if (", out);
    cg(out, n->n_l);
    fputs(") {\n", out);
    cg(out, n->n_r->n_l);
    fputs(";}\n", out);
    if (n->n_r->n_r) {
      fputs("else {\n", out);
      cg(out, n->n_r->n_r);
      fputs(";}\n", out);
    }
    break;
  case O_DO:
    fputs("while (", out);
    cg(out, n->n_l);
    fputs(") {\n", out);
    cg(out, n->n_r);
    fputs("}\n", out);
    break;
  case O_FA:
    fputs("{ int ", out);
    cg(out, n->n_l);
    fputs(";\nfor (", out);
    cg(out, n->n_l);
    fputs(" = ", out);
    cg(out, n->n_r->n_l->n_l); // lb
    fputs("; ", out);
    cg(out, n->n_l);
    fputs(" <= ", out);
    cg(out, n->n_r->n_l->n_r); // lb
    fputs("; ++", out);
    cg(out, n->n_l);
    fputs(") {\n", out);
    cg(out, n->n_r->n_r);
    fputs("}\n}\n", out);
    break;
  case O_EXIT:
    fputs("exit(0);\n", out);
    break;
  case O_WRITE:
    fputs("_WRITE", out);
    if (!n->n_int) fputc('S', out);
    if (!sigCmp(n->n_r->sig, Gint))
      fputc('I', out);
    else
      fputc('S', out);
    fputc('(', out);
    cg(out, n->n_r);
    fputs(");\n", out);
    break;
  case O_READ:
    fputs("__read()", out);
    break;
  case O_BREAK:
    fputs("break;\n", out);
    break;
  case O_RETURN:
    fprintf(out, "return ");
    fprintf(out, "%s;\n", ReturnArg);
    break;
  case O_ASSIGN:
    cg(out, n->n_l);
    fputs(" = ", out);
    cg(out, n->n_r);
    fputs(";\n", out);
    break;
  case O_UNIOP:
    if (n->n_binop == B_SUB) {
      fprintf(out, "( 0 -");
      cg(out, n->n_r);
      fputc(')', out);
    }
    else if (n->n_binop == B_QUEST) {
      fputc('(', out);
      cg(out, n->n_r);
      fprintf(out, " == 1)");
    }
    else
      CompilerError(n->n_loc, "Invalid uniary op: %d\n", n->n_binop);
    break;
  case O_BINOP:
    fputc('(', out);
    cg(out, n->n_l);
    fprintf(out, " %s ", cgBinop(n));
    cg(out, n->n_r);
    fputc(')', out);
    break;
  case O_CALL:
    {
      Node p;
      fputs("P_", out);
      cg(out, n->n_l);
      fputc('(', out);
      p = n->n_r;
      if (p) {
	while (p->oper == O_SEQ) {
	  cg(out, p->n_l);
	  p = p->n_r;
	  if (p)
	    fputs(", ", out);
	}
	cg(out, p);
      }
      fputs(")", out);
    }
    break;
  case O_ERR:
    CompilerError(n->n_loc, "invalid node in cg, oper=%d\n", n->oper);
    break;
  }
}

void cgGen(FILE *out, Node n)
{
  if (!n)
    return;

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
  
  fflush(out);
}
/*........................ end of cg.c ......................................*/
