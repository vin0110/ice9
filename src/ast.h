/******************************************************************************
 *
 *  File Name........: ast.h
 *
 *  Description......:
 *
 *  Created by vin on 01/10/02
 *
 *  Revision History.:
 *
 *  $Log: ast.h,v $
 *  Revision 1.2  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.1  2002/01/16 22:14:03  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: ast.h,v 1.2 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/

#ifndef AST_H
#define AST_H

#include "type.h"
#include "symtab.h"

typedef enum {
  B_ADD,
  B_SUB,
  B_MUL,
  B_DIV,
  B_MOD,
  B_EQ,
  B_NEQ,
  B_LT,
  B_GT,
  B_LE,
  B_GE,
  B_ERR,
  B_QUEST,
} Binop;

/* all operators use oper, sig, n_loc fields */
typedef enum {			/* addtional fields that are used */
  O_SEQ,			/* n_l, n_r */
  O_EXP,			/* n_r */
  O_ID,				/* n_str */
  O_SYM,			/* n_sym, n_r */
  O_ARR,			/* n_l, n_r */
  O_ILIT,			/* n_int */
  O_SLIT,			/* n_str */
  O_BLIT,			/* n_int */
  O_PROC,			/* n_str, n_l, n_r */
  O_FORWARD,			/* n_str, n_l */
  O_TYPE,			/* n_str */
  O_VAR,			/* n_r */
  O_IF,				/* n_l, n_r */
  O_DO,				/* n_l, n_r */
  O_FA,				/* n_l, n_r */
  O_EXIT,			/*  */
  O_WRITE,			/* n_int, n_r */
  O_READ,			/*  */
  O_BREAK,			/*  */
  O_RETURN,			/* n_r */
  O_ASSIGN,			/* n_l, n_r */
  O_BINOP,			/* n_binop, n_l, n_r */
  O_UNIOP,			/* n_binop, n_r */
  O_CALL,			/* n_l, n_r */
  O_ERR
} Oper;

typedef struct _node {
  Oper oper;
  Sig sig;
  union {
    Symbol u_sym;
    int u_int;
    char *u_str;
    Binop u_binop;
    int u_xtra;
  } n_u;
  int n_loc;
  int cg_value, cg_loc;
  struct _node *n_l, *n_r;
} *Node, node_t;

#define n_sym	n_u.u_sym
#define n_int	n_u.u_int
#define n_str	n_u.u_str
#define n_binop n_u.u_binop
#define n_xtra	n_u.u_xtra

Node mkSeq(Node, Node);
Node mkExpStm(Node);
Node mkExp(Node);
Node mkId(char *);
Node mkSym(Symbol, Node);
Node mkArr(Node, Node);
Node mkIlit(int);
Node mkSlit(char *);
Node mkBlit(int);
Node mkProc(char *, Node, Node, Node, Sig);
Node mkForward(char *, Node, Node, Sig);
Node mkType(char *, Sig);
Node mkVar(Node, Sig);
Node mkIf(Node, Node, Node);
Node mkDo(Node, Node);
Node mkFa(Node, Node, Node, Node);
Node mkExit();
Node mkWrite(int, Node); /* int is nl? */
Node mkRead();
Node mkBreak();
Node mkReturn();
Node mkAssign(Node, Node);
Node mkBinop(Binop, Node, Node);
Node mkUniop(Binop, Node);
Node mkCall(char *, Node);
void nodePrint(int, Node);

#endif /* AST_H */
/*........................ end of ast.h .....................................*/
