/******************************************************************************
 *
 *  File Name........: yhelp.c
 *
 *  Description......:
 *
 *  Created by vin on 01/11/02
 *
 *  Revision History.:
 *
 *  $Log: yhelp.c,v $
 *  Revision 1.2  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.1  2002/01/16 22:14:04  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: yhelp.c,v 1.2 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/

#include "yhelp.h"
#include <stdlib.h>
#include <assert.h>
#include "ice9.h"
#include "symtab.h"
#include "list-pack.h"

int DoSemantic = 1;
extern Table Procs, Vars, Types;
extern Sig Gnil, Gint;

ylist yList(void *item, ylist tl)
{
  ylist y = malloc(sizeof(ylist_t));
  assert(y);
  y->head = item;
  y->tail = tl;
  return y;
}

void yListFree(ylist l, int flag)
{
  if (!l)
    return;
  if (l->tail)
    yListFree(l->tail, flag);
  if (flag) {
    if (l->head)
      free(l->head);
  }
  free(l);
}

static Node idList(ylist hd)
{
  Node n;
  if (!hd)
    return NULL;
  n = mkId(hd->head);
  n->n_r = idList(hd->tail);
  return n;
}

/* 
 * insert a list of variables in the the Vars symbol table
 * ids 		- list of vars (char *)
 * typeid	- base type
 * array	- optional array declarations; a list of integers
 */
Node yInsertVar(ylist ids, Node typeid, ylist array)
{
  Sig g, h;
  Symbol m;
  Node idlist;
  Node t;

  if (!DoSemantic) return NULL;

  m = symLookup(Types, typeid->n_sym->name);
  if (m) {
    g = sigMake(T_NAME);
    g->sym = m;
    g->under = m->info;
  }
  else {
    FatalS(0, "type '%s' not found\n", typeid->n_sym->name);
  }
  while (array) {
    h = sigMake(T_ARRAY);
    h->under = g;
    h->size = (int)array->head;
    g = h;
    array = array->tail;
  }
  idlist = idList(ids);
  t = idlist;
  while (t) {
    if (symLookup(Vars, t->n_str))
      FatalS(LINE, "name clash: id '%s' previously defined\n", (char*)t->n_str);
    if (symInsert(Vars, symMake(symStr(t->n_str), g)))
      FatalS(LINE, "sym tab insert failed %s\n", (char*)t->n_str);
    t->sig = g;
    t = t->n_r;
  }

  return idlist;
}

static Node declist(ylist ids, Sig g, Node tail)
{
  Node n;
  Symbol m;

  if (ids == NULL)
    return tail;
  m = symMake(symStr(ids->head), g);
  n = mkSym(m, declist(ids->tail, g, tail));
  return n;
}

Node yDeclist(ylist ids, Node typeid, Node tail)
{
  Sig g;
  if (DoSemantic)
    g = typeid->n_sym->info;
  else
    g = NULL;

  return declist(ids, g, tail);
}

Node yInsertType(char *id, Node typeid, ylist array)
{
  Sig g, h;

  if (!DoSemantic) return NULL;

  if (symLookup(Types, id))
    FatalS(LINE, "name clash: type '%s' previously defined\n", id);

  g = typeid->n_sym->info;

  while (array) {
    h = sigMake(T_ARRAY);
    h->under = g;
    h->size = (int)array->head;
    g = h;
    array = array->tail;
  }
  h = sigMake(T_NAME);
  h->under = g;
  h->sym = symMake(symStr(id), h);
  symInsert(Types, h->sym);

  return NULL;
}

void yFa(char *id)
{
  tabPush(Vars);
  symInsert(Vars, symMake(symStr(id), Gint));
}

static Sig mkSiglist(Node params)
{
  Sig g;

  if (!params) return NULL;

  g = sigMake(T_LIST);
  g->under = params->sig;

  if (params->oper == O_SEQ && params->n_r) {
    g->next = mkSiglist(params->n_r);
    g->size = g->next->size + 1;
  }
  else
    g->size = 1;
  return g;
}

Node yForward(char *id, Node params, Node rettype)
{
  Symbol m;
  Sig g;

  m = symLookup(Procs, id);
  if (m) {    // not error if forwarded
    FatalS(LINE, "name clash: proc '%s' previously defined\n", id);
  }
  else {
    g = sigMake(T_PROC);
    if (rettype)
      g->under = rettype->n_sym->info;
    else
      g->under = Gnil;
    if (params)
      g->next = mkSiglist(params);
    g->size = -1;
    m = symMake(symStr(id), g);
    symInsert(Procs, m);
  }
  
  return NULL;
}

Node yProcPre(char *id, Node params, Node rettype)
{
  Node t;
  Symbol m;
  Sig g;

  m = symLookup(Procs, id);
  if (m) {    // not error if forwarded
    g = m->info;
    if (g->size != -1)
      FatalS(LINE, "name clash: proc '%s' previously defined\n", id);
    g->size = 1;
  }
  else {
    g = sigMake(T_PROC);
    if (rettype)
      g->under = rettype->n_sym->info;
    else
      g->under = Gnil;
    if (params)
      g->next = mkSiglist(params);
    m = symMake(symStr(id), g);
    symInsert(Procs, m);
  }
  tabPush(Vars);
  // @@@ add parameters to var symtable
  t = params;
  while (t) {
    symInsert(Vars, symMake(symStr(t->n_sym->name), t->n_sym->info));
    t = t->n_r;
  }
  if (rettype) {
    symInsert(Vars, symMake(symStr(id), rettype->n_sym->info));
  }
  tabPush(Types);
  
  return mkProc(id, params, rettype, NULL, g);
}

Node yProcPost(Node decls, Node stms)
{
#ifdef TM
  // set locations of variables
  varSetLocation(Vars);
#endif
  tabPop(Vars);
  tabPop(Types);

  return mkSeq(decls, stms);
}

Node yLval(Node id)
{
  Symbol m;

  assert(id);
  if (id->oper == O_SYM || !DoSemantic)
    return id;
  m = symLookup(Vars, id->n_str);
  if (!m)
    FatalS(LINE, "invalid identifier %s\n", id->n_str);
  id->sig = m->info;
  id->n_sym = m;
  id->oper = O_SYM;
  return id;
}

Node yCall(char *str, Node args)
{
  Symbol sym;
  Node n;
  Sig proc, parms;

  sym = symLookup(Procs, str);
  if (!sym) 
    FatalS(LINE, "invalid procedure %s\n", str);
  else {
    proc = (Sig)sym->info;
#if 0
    puts("foo");
    nodePrint(0,args);
    sigPrint(proc); putchar('\n');
#endif
    parms = mkSiglist(args);
    // check param sig
    if (sigCmp(proc->next, parms)) {
#if 0
      printf("proc: ");
      sigPrint(proc->next);
      printf("\nparms: ");
      sigPrint(parms);
      putchar('\n');
#endif
      FatalS(LINE, "actual and formal parameters do not match\n");
    }
  }
  n = mkExp(mkCall(str, args));
  if (proc && proc->under)
    n->sig = proc->under;
  else
    n->sig = NULL;
  return n;
}

Node yTypeid(char *str)
{
  Symbol sym;
  if (!DoSemantic) return NULL;

  sym = symLookup(Types, str);
  if (!sym) {
    FatalS(LINE, "invalid typeid %s\n", str);
    return NULL;
  }
  return mkSym(sym, NULL);
}

/*........................ end of yhelp.c ...................................*/
