/******************************************************************************
 *
 *  File Name........: ast.c
 *
 *  Description......:
 *
 *  Created by vin on 01/10/02
 *
 *
 *****************************************************************************/

#include "list-pack.h"
#include "ice9.h"
#include "type.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

extern int yynewlines;
extern int SigPrint;
extern Sig Gint, Gbool, Gstr, Gnil, Gerr;
extern Table Vars;

static char *binop2str(Binop);

static Node nodeMake(Oper o)
{

  Node n = malloc(sizeof(node_t));
  assert(n);
  n->oper = o;
  n->sig = Gnil;
  n->n_int = 0;
  n->n_loc = yynewlines;
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

Node mkExpStm(Node e)
{
  Node n = mkExp(e);
  n->n_xtra = 1;
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
  if (DoSemantic) {
    if (sigCmp(exp->sig, Gint))
      FatalS(LINE, "non-integer index\n");
  }
  n->n_l = l;
  n->n_r = exp;
  if (DoSemantic) {
    assert(l->sig);
    g = l->sig;
    if (g->type == T_NAME)
      g = g->under;
    assert(g->under);
    n->sig = g->under;
  }
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
    FatalS(LINE, "condition not a boolean\n");
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
    FatalS(LINE, "lb in fa not an integer\n");
  if (sigCmp(ub->sig, Gint))
    FatalS(LINE, "ub in fa not an integer\n");

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

Node mkReturn()
{
  Node n = nodeMake(O_RETURN);
  return n;
}

Node mkAssign(Node l, Node r)
{
  Node n = nodeMake(O_ASSIGN);
  assert(l);
  assert(r);
  n->n_l = l;
  n->n_r = r;
  if (DoSemantic) {
    if (sigCmp(l->sig, r->sig)) {
      FatalS(LINE, "incompatible types in assignment\n");
    }
    if (!sigCmp(l->sig, Gint) && 
	!sigCmp(l->sig, Gbool) && 
	!sigCmp(l->sig, Gstr)) {
      FatalS(LINE, "assignment to non-scalar variable\n");
    }
  }
  return n;
}

static int ckSigUni(Binop op, Sig r) 
{
  switch (op) {
  case B_SUB:	return sigCmp(r, Gint);
  case B_QUEST: return sigCmp(r, Gbool);
  default:
    FatalS(0, "invalid unary operator %s\n", binop2str(op));
  }
  return -1;
}

static Sig ckSig(Binop op, Sig l, Sig r)
{
  if (!l || !r) {
    Fatal(LINE, "binop (%s) missing arugment(s)\n", binop2str(op));
    return Gnil;
  }
  if (sigCmp(l, r)) {
    FatalS(LINE, "types in binop (%s) do not match\n", binop2str(op));
    return Gnil;
  }

  // l and r are the same
  if (!sigCmp(l, Gstr)) {
    FatalS(LINE, "invalid type for binop (%s)\n", binop2str(op));
    return Gnil;
  }
  else if (!sigCmp(l, Gbool)) {
    if (op != B_ADD && op != B_MUL && op != B_NEQ && B_EQ) {
      FatalS(LINE, "invalid type for binop (%s)\n", binop2str(op));
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
  if (DoSemantic)
    n->sig = ckSig(op, l->sig, r->sig);
  return n;
}

Node mkUniop(Binop op, Node r)
{
  Node n = nodeMake(O_UNIOP);
  n->n_binop = op;
  n->n_l = NULL;
  n->n_r = r;
  if (DoSemantic) {
    if (ckSigUni(op, r->sig))
      FatalS(LINE, "type mistmatch for unary operator %s\n", binop2str(op));
    else
      n->sig = Gint;
  }
  return n;
}

Node mkCall(char *id, Node elist)
{
  Node n = nodeMake(O_CALL);
  n->n_l = mkId(id);
  n->n_r = elist;
  return n;
}

static void prOper(Oper op)
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
  case O_UNIOP:
    fputs("UNIOP(", stdout);
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
    Fatal(LINE, "invalid operator: %s\n", binop2str(op));
    break;
  }
}

static char *binop2str(Binop b)
{
  switch (b) {
  case B_ADD:	return "B_ADD";
  case B_SUB:	return "B_SUB";
  case B_MUL:	return "B_MUL";
  case B_DIV:	return "B_DIV";
  case B_MOD:	return "B_MOD";
  case B_EQ:	return "B_EQ";
  case B_NEQ:	return "B_NEQ";
  case B_LT:	return "B_LT";
  case B_GT:	return "B_GT";
  case B_LE:	return "B_LE";
  case B_GE:	return "B_GE";
  case B_QUEST:	return "B_QUEST";
  default:	return "B_ERR";
  }
}

void printBinop(Binop b){
  fprintf(stdout, "%s):", binop2str(b));
}

static void prFa(int, Node);
static void prDo(int, Node);
static void prIf(int, Node);
static void prCall(int, Node);
static void prProc(int, Node);

#define INDENT(n) do { int i; for (i = 0; i<n*2; i++) putchar(' ');} while (0)

void nodePrint(int indent, Node n)
{
  if (!n) {
    INDENT(indent);
    puts("nil");
    return;
  }
  if (n->oper == O_SEQ) {
    if (n->n_l)
      nodePrint(indent, n->n_l);
    nodePrint(indent, n->n_r);
    return;
  }
  else if (n->oper == O_SYM && !DoSemantic)
    return;
  INDENT(indent);
  prOper(n->oper);
  switch (n->oper) {
  case O_SEQ:
    CompilerError(n->n_loc, "Found SEQ where none should be\n");
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
    printf(" %s\n", n->n_str);
    prProc(indent, n);
    return;
  case O_FORWARD:
    printf(" %s", n->n_str);
    break;
  case O_TYPE:
    printf(" %s", n->n_str);
    break;
  case O_VAR:
    break;
  case O_IF:
    prIf(indent,n);
    return;
  case O_DO:
    prDo(indent,n);
    return;
  case O_FA:
    prFa(indent,n);
    return;
  case O_EXIT:
    printf(" %d", n->n_int);
    break;
  case O_WRITE:
    printf("%s", n->n_xtra ? "" : "S" );
    break;
  case O_READ:
  case O_BREAK:
  case O_RETURN:
  case O_ASSIGN:
    break;
  case O_UNIOP:
  case O_BINOP:
    printBinop(n->n_binop);
    break;
  case O_CALL:
    prCall(indent,n);
    return;
  case O_ERR:
  default:
    Fatal(LINE, "invalid operator: %d\n", n->oper);
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
  if (n->n_l)
    nodePrint(indent+1, n->n_l);
  if (n->n_r)
    nodePrint(indent+1, n->n_r);
}

static void prFa(int indent, Node n)
{
  putchar('\n');
  nodePrint(indent+1, n->n_l);
  INDENT(indent);
  printf("(from)\n");
  nodePrint(indent+1, n->n_r->n_l->n_l);
  INDENT(indent);
  printf("(to)\n");
  nodePrint(indent+1, n->n_r->n_l->n_r);
  INDENT(indent);
  printf("(body)\n");
  nodePrint(indent+1, n->n_l->n_r);
}

static void prDo(int indent, Node n)
{
  putchar('\n');
  nodePrint(indent+1, n->n_l);
  INDENT(indent);
  printf("(body)\n");
  nodePrint(indent+1, n->n_r);
}

static void prIf(int indent, Node n)
{
  putchar('\n');
  nodePrint(indent+1, n->n_l);
  INDENT(indent);
  printf("(then)\n");
  nodePrint(indent+1, n->n_r->n_l);
  INDENT(indent);
  printf("(else)\n");
  nodePrint(indent+1, n->n_r->n_r);
}

static void prCall(int indent, Node n)
{
  putchar('\n');
  nodePrint(indent+1, n->n_l);
  INDENT(indent);
  printf("(arglist)\n");
  nodePrint(indent+1, n->n_r);
}

static void prProc(int indent, Node n)
{

  INDENT(indent);
  printf("(return)\n");
  nodePrint(indent+1, n->n_l->n_r);
  if (DoSemantic) {
    INDENT(indent);
    printf("(formals)\n");
    nodePrint(indent+1, n->n_l->n_l);
    INDENT(indent);
    printf("(locals)\n");
    nodePrint(indent+1, n->n_r->n_l);
  }
  INDENT(indent);
  printf("(body)\n");
  nodePrint(indent+1, n->n_r->n_r);
}

/*........................ end of ast.c .....................................*/
