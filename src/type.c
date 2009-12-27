/******************************************************************************
 *
 *  File Name........: type.c
 *
 *  Description......:
 *
 *  Created by vin on 01/10/02
 *
 *  Revision History.:
 *
 *  $Log: type.c,v $
 *  Revision 1.2  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.1  2002/01/16 22:14:03  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: type.c,v 1.2 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/
//#include "ice9.h"
#include <stdlib.h>
#include "type.h"
#include <assert.h>

Sig sigMake(Type t)
{
  Sig g;
  g = malloc(sizeof(sig_t));
  assert(g);
  g->type = t;
  g->under = g->next = NULL;
  g->size = 1;
  g->sym = NULL;
  return g;
}

void sigDestroy(Sig t)
{
  assert(t);
  if (t->under)
    sigDestroy(t->under);
#if 0
  if (t->sym)
    symDestroy(t->sym);
#endif
  sigDestroy(t);
}

Sig sigCopy(Sig g)
{
  Sig o;

  if (!g)
    return NULL;
  o = sigMake(g->type);
  o->under = sigCopy(g->under);
  return o;
}

/* return 0 if Sig are equal */
int sigCmp(Sig l, Sig r)
{
#if 0
  printf("sigCmp: ");
  sigPrint(l);
  putchar(',');
  sigPrint(r);
  putchar('\n');
#endif
  if (!l)
    return r != NULL;
  if (!r)
    return -1;
  if (l == r)
    return 0;
  while (l->type == T_NAME)
    l = l->under;
  while (r->type == T_NAME)
    r = r->under;
  if (l->type != r->type)
    return -1;
  if (sigCmp(l->under, r->under))
    return -1;
  if (sigCmp(l->next, r->next))
    return -1;
  return 0;
}

static void prType(Type t)
{
  switch (t) {
  case T_INT:	printf("T_INT");	break;
  case T_BOOL:	printf("T_BOOL");	break;
  case T_STR:	printf("T_STR");	break;
  case T_PROC:	printf("T_PROC");	break;
  case T_ARRAY:	printf("T_ARRAY");	break;
  case T_NAME:	printf("T_NAME");	break;
  case T_LIST:	printf("T_LIST");	break;
  case T_NIL:	printf("T_NIL");	break;
  case T_ERR:	printf("T_ERR");	break;
  default: Fatal("invalid type\n");	break;
  }
}

void sigPrint(Sig g)
{
  if (!g) return;
  prType(g->type);
  printf("(%d)", g->size);
  if (g->under) {
    putchar('[');
    sigPrint(g->under);
    putchar(']');
  }
  if (g->next) {
    putchar(',');
    sigPrint(g->next);
  }
}

/*........................ end of type.c ....................................*/

