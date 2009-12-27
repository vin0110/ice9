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

extern Sig Gint;

static void cgType(FILE *out, Sig g)
{
  while (g->type == T_ARRAY) {
    g = g->under;
    assert(g);
  }
  switch (g->type) {
  case T_INT:
  case T_BOOL:
    fputs("int ", out);
    break;
  case T_STR:
    fputs("char *", out);
    break;
  case T_NAME:
    fprintf(out, "T_%s ", g->sym->name);
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
  case B_EQ:	return "==";
  case B_NEQ:	return "!=";
  case B_LT:	return "<";
  case B_GT:	return ">";
  case B_LE:	return "<=";
  case B_GE:	return ">=";
  }
}

static void cg(FILE *out, Node n)
{
  if (!n)
    return;

  switch (n->oper) {
  case O_SEQ:
    cg(out, n->n_l);
    cg(out, n->n_r);
    break;
  case O_EXP:
    cg(out, n->n_r);
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
	fputc('[', out);
	cg(out, n->n_r);
	fputc(']', out);
	n = n->n_l;
      }
    }
    break;
  case O_ILIT:
    assert(!n->n_r);
    fprintf(out, "%d", n->n_int);
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
      if (n->n_l->n_r)
	cgType(out, n->n_l->n_r->sig);
      else
	fputs("void ", out);
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
      cg(out, n->n_r);
      fputs("}\n", out);
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
    fputs("}\n", out);
    if (n->n_r->n_r) {
      fputs("else {\n", out);
      cg(out, n->n_r->n_r);
      fputs("}\n", out);
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
    cg(out, n->n_r);
    fputs(";\n", out);
    break;
  case O_ASSIGN:
    cg(out, n->n_l);
    fputs(" = ", out);
    cg(out, n->n_r);
    fputs(";\n", out);
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
    Fatal(n->n_loc, "invalid node in cg");
    break;
  }
}

void cgGen(FILE *out, Node n)
{
  if (!n)
    return;

  fprintf(out, "/* generated by ice9 */\n");
  fprintf(out, "/* runtime */\n"
	  "#include <errno.h>\n#include <stdlib.h>\n#include <stdio.h>\n"
	  "typedef int T_int;\ntypedef char * T_string;\n");
  fprintf(out, "int __read(){\n");
  fprintf(out, "char _readbuf[512]; int i;\n");
  fprintf(out, "if (!fgets(_readbuf, 512, stdin)) { puts(\"READ ERROR\"); "
	  "exit(-1);}\n");
  fprintf(out, "errno = 0;\n i = strtol(_readbuf, NULL, 10);"
	  "if (errno) perror(\"CONVERSION ERROR\");\nreturn i;\n}\n");
  fprintf(out, "#define _WRITEI(i)\tprintf(\"%%d\\n\", i)\n");
  fprintf(out, "#define _WRITESI(i)\tprintf(\"%%d\", i)\n");
  fprintf(out, "#define _WRITES(s)\tprintf(\"%%s\\n\", s)\n");
  fprintf(out, "#define _WRITESS(s)\tprintf(\"%%s\", s)\n");
  fprintf(out, "\n");
  if (n->n_l) {
    fprintf(out, "/* global declarations */\n");
    cg(out, n->n_l);
  }

  fprintf(out, "\n/* main program */\n");
  fprintf(out, "main()\n{\n");
  cg(out, n->n_r);
  fprintf(out, "}\n");
  
  fflush(out);
}
/*........................ end of cg.c ......................................*/
