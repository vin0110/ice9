/******************************************************************************
 *
 *  File Name........: ast.c
 *
 *  Description......:
 *
 *  Created by vin on 01/10/02
 *
 *  Revision History.:
 *
 *  $Log: ast.c,v $
 *  Revision 1.2  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.1  2002/01/16 22:14:03  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: ast.c,v 1.2 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/

#include "list-pack.h"
#include "ice9.h"
#include "symtab.h"
#include "type.h"
#include "ast.h"
#include <stdlib.h>
#include <assert.h>

extern int yynewlines;
extern int SigPrint;
extern Sig Gint, Gbool, Gstr, Gnil, Gerr;
extern Table Vars;

static Node nodeMake(Oper o)
{

  Node n = malloc(sizeof(node_t));
  assert(n);
  n->oper = o;
  n->sig = Gnil;
  n->n_int = 0;
  //  n->n_binop = B_ERR;
  n->n_loc = yynewlines+1;
  n->n_l = n->n_r = NULL;
  return n;
}

Node mkSeq(Node l, Node r)
{
  Node n = nodeMake(O_SEQ);
  n->n_l = l;
  n->n_r = r;
  return n;
}

Node mkExp(Node e)
{
  Node n = nodeMake(O_EXP);
  n->n_r = e;
  return n;
}

Node mkSym(Symbol sym, Node next)
{
  Node n = nodeMake(O_SYM);
  n->n_sym = sym;
  n->n_r = next;
  n->sig = sym->info;
  return n;
}

Node mkArr(Node l, Node exp)
{
  Sig g;
  Node n = nodeMake(O_ARR);
  if (sigCmp(exp->sig, Gint))
    Fatal(LINE, "non-integer index");
  n->n_l = l;
  n->n_r = exp;
  assert(l->sig);
  g = l->sig;
  if (g->type == T_NAME)
    g = g->under;
  assert(g->under);
  n->sig = g->under;
  return n;
}

Node mkId(char *id)
{
  Node n = nodeMake(O_ID);
  n->n_str = symStr(id);
  return n;
}

Node mkIlit(int i)
{
  Node n = nodeMake(O_ILIT);
  n->n_int = i;
  n->sig = Gint;
  return n;
}

Node mkSlit(char *s)
{
  int len;

  Node n = nodeMake(O_SLIT);
  assert(s);
  len = strlen(s);
  n->n_str = malloc(len+1);
  strcpy(n->n_str, s);
  n->sig = Gstr;
  return n;
}

Node mkBlit(int b)
{
  Node n = nodeMake(O_BLIT);
  n->n_int = b;
  n->sig = Gbool;
  return n;
}

Node mkProc(char *name, Node params, Node ret, Node body, Sig g)
{
  Node n = nodeMake(O_PROC);
  n->n_l = mkSeq(params, ret);
  n->n_r = body;
  n->n_str = name;
  n->sig = g;
  return n;
}

Node mkForward(char *name, Node params, Node ret, Sig g)
{
  Node n = nodeMake(O_FORWARD);
  n->n_l = mkSeq(params, ret);
  n->n_str = name;
  n->sig = g;
  return n;
}

Node mkType(char *name, Sig g)
{
  Node n = nodeMake(O_TYPE);
  n->n_str = name;
  n->sig = g;
  return n;
}

Node mkVar(Node idlist, Sig g)
{
  Node n = nodeMake(O_VAR);
  n->n_r = idlist;
  n->sig = g;
  return n;
}

Node mkIf(Node cond, Node then, Node elses)
{
  Node n = nodeMake(O_IF);
  if (sigCmp(cond->sig, Gbool)) {
    Fatal(LINE, "condition not a boolean");
  }
  n->n_l = cond;
  n->n_r = mkSeq(then, elses);
  return n;
}

Node mkDo(Node cond, Node body)
{
  Node n = nodeMake(O_DO);
  n->n_l = cond;
  n->n_r = body;
  return n;
}

Node mkFa(Node id, Node lb, Node ub, Node body)
{
  Node n = nodeMake(O_FA);
  
  if (sigCmp(lb->sig, Gint))
    Fatal(LINE, "lb in fa not an integer");
  if (sigCmp(ub->sig, Gint))
    Fatal(LINE, "ub in fa not an integer");

  n->n_l = id;
  n->n_r = mkSeq(mkSeq(lb, ub), body);
  return n;
}

Node mkExit()
{
  Node n = nodeMake(O_EXIT);
  return n;
}

Node mkWrite(int nl, Node exp)
{
  Node n = nodeMake(O_WRITE);
  n->n_r = exp;
  n->n_int = nl;
  return n;
}

Node mkRead()
{
  Node n = nodeMake(O_READ);
  n->sig = Gint;
  return n;
}

Node mkBreak()
{
  Node n = nodeMake(O_BREAK);
  return n;
}

Node mkReturn(Node exp)
{
  Node n = nodeMake(O_RETURN);
  n->n_r = exp;
  return n;
}

Node mkAssign(Node l, Node r)
{
  Node n = nodeMake(O_ASSIGN);
  n->n_l = l;
  n->n_r = r;
  if (sigCmp(l->sig, r->sig)) {
    Fatal(LINE, "incompatible types in assignment");
  }
  if (!sigCmp(l->sig, Gint) && 
      !sigCmp(l->sig, Gbool) && 
      !sigCmp(l->sig, Gstr)) {
    Fatal(LINE, "assignment to non-scalar variable");
  }
  return n;
}

static Sig ckSig(Binop op, Sig l, Sig r)
{
  if (!l || !r) {
    Fatal(LINE, "incorrect types to binop");
    return Gnil;
  }
  if (sigCmp(l, r)) {
    Fatal(LINE, "incorrect types to binop.");
    return Gnil;
  }

  // l and r are the same
  if (!sigCmp(l, Gstr)) {
    Fatal(LINE, "incorrect types to binop..");
    return Gnil;
  }
  else if (!sigCmp(l, Gbool)) {
    if (op != B_ADD && op != B_MUL) {
      Fatal(LINE, "incorrect types to binop...");
      return Gnil;
    }
  }
  else {
    // l and r ints
    if (op == B_EQ || op == B_NEQ  || op == B_LT  || 
	op == B_GT  || op == B_LE  || op == B_GE) {
      return Gbool;
    }
  }
  return l;
}
 
Node mkBinop(Binop op, Node l, Node r)
{
  Node n = nodeMake(O_BINOP);
  n->n_binop = op;
  n->n_l = l;
  n->n_r = r;
  n->sig = ckSig(op, l->sig, r->sig);
  return n;
}

Node mkCall(char *id, Node elist)
{
  Node n = nodeMake(O_CALL);
  n->n_l = mkId(id);
  n->n_r = elist;
  return n;
}

static prOper(Oper op)
{
  switch (op) {
  case O_SEQ:
    fputs("SEQ:", stdout);
    break;
  case O_EXP:
    fputs("EXP:", stdout);
    break;
  case O_ID:
    fputs("ID:", stdout);
    break;
  case O_SYM:
    fputs("SYM:", stdout);
    break;
  case O_ARR:
    fputs("ARR:", stdout);
    break;
  case O_ILIT:
    fputs("ILIT:", stdout);
    break;
  case O_SLIT:
    fputs("SLIT:", stdout);
    break;
  case O_BLIT:
    fputs("BLIT:", stdout);
    break;
  case O_PROC:
    fputs("PROC:", stdout);
    break;
  case O_FORWARD:
    fputs("FORWARD:", stdout);
    break;
  case O_TYPE:
    fputs("TYPE:", stdout);
    break;
  case O_VAR:
    fputs("VAR:", stdout);
    break;
  case O_IF:
    fputs("IF:", stdout);
      break;
  case O_DO:
    fputs("DO:", stdout);
    break;
  case O_FA:
    fputs("FA:", stdout);
    break;
  case O_EXIT:
    fputs("EXIT:", stdout);
    break;
  case O_WRITE:
    fputs("WRITE:", stdout);
    break;
  case O_READ:
    fputs("READ:", stdout);
    break;
  case O_BREAK:
    fputs("BREAK:", stdout);
    break;
  case O_RETURN:
    fputs("RETURN:", stdout);
    break;
  case O_ASSIGN:
    fputs("ASSIGN:", stdout);
    break;
  case O_BINOP:
    fputs("BINOP(", stdout);
    break;
  case O_CALL:
    fputs("CALL:", stdout);
    break;
  case O_ERR:
    fputs("ERR:", stdout);
    break;
  default:
    Fatal(LINE, "invalid operator");
    break;
  }
}

void printBinop(Binop b)
{
  switch (b) {
  case B_ADD:
    fputs("B_ADD):", stdout);
    break;
  case B_SUB:
    fputs("B_SUB):", stdout);
    break;
  case B_MUL:
    fputs("B_MUL):", stdout);
    break;
  case B_DIV:
    fputs("B_DIV):", stdout);
    break;
  case B_EQ:
    fputs("B_EQ):", stdout);
    break;
  case B_NEQ:
    fputs("B_NEQ):", stdout);
    break;
  case B_LT:
    fputs("B_LT):", stdout);
    break;
  case B_GT:
    fputs("B_GT):", stdout);
    break;
  case B_LE:
    fputs("B_LE):", stdout);
    break;
  case B_GE:
    fputs("B_GE):", stdout);
    break;
  default:
    fputs("B_ERR):", stdout);
    break;
  }
}

#define INDENT(n) do { int i; for (i = 0; i<n*2; i++) putchar(' ');} while (0)

void nodePrint(int indent, Node n)
{
  if (!n)
    return;
  if (n->oper == O_SEQ) {
    nodePrint(indent, n->n_l);
    nodePrint(indent, n->n_r);
    return;
  }
  INDENT(indent);
  prOper(n->oper);
  switch (n->oper) {
  case O_SEQ:
  case O_EXP:
    break;
  case O_ID:
    printf("'%s'", n->n_str);
    break;
  case O_SYM:
    printf("'%s'", n->n_sym->name);
    break;
  case O_ARR:
    break;
  case O_ILIT:
    printf("%d", n->n_int);
    break;
  case O_SLIT:
    printf("'%s'", n->n_str);
    break;
  case O_BLIT:
    printf(" %s", n->n_int ? "TRUE" : "FALSE");
    break;
  case O_PROC:
  case O_FORWARD:
    printf(" %s", n->n_str);
    break;
  case O_TYPE:
    printf(" %s", n->n_str);
    break;
  case O_VAR:
    break;
  case O_IF:
    break;
  case O_DO:
    break;
  case O_FA:
    break;
  case O_EXIT:
    printf(" %d", n->n_int);
    break;
  case O_WRITE:
    printf("", n->n_xtra ? "" : "S" );
    break;
  case O_READ:
    break;
  case O_BREAK:
    break;
  case O_RETURN:
    break;
  case O_ASSIGN:
    break;
  case O_BINOP:
    printBinop(n->n_binop);
    break;
  case O_CALL:
  case O_ERR:
    break;
  default:
    Fatal(LINE, "invalid operator");
    break;
  }
  if (SigPrint) {
    if (n->sig && n->sig->type != T_NIL) {
      printf(" [");
      sigPrint(n->sig);
      putchar(']');
    }
  }
  putchar('\n');
  nodePrint(indent+1, n->n_l);
  nodePrint(indent+1, n->n_r);
}

/*........................ end of ast.c .....................................*/
