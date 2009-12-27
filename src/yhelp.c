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

Node yInsertVar(ylist ids, Node typeid, ylist array)
{
  Sig g, h;
  Symbol m;
  Node n;
  Node idlist;
  ylist t;

#if 0
  m = symLookup(Types, typeid->n_sym->name);
  if (m) {
    g = sigMake(T_NAME);
    g->sym = m;
  }
  else 
#endif
{
    g = typeid->n_sym->info;

    while (array) {
      h = sigMake(T_ARRAY);
      h->under = g;
      h->size = (int)array->head;
      g = h;
      array = array->tail;
    }
  }
  t = ids;
  while (t) {
    if (symLookup(Vars, t->head)) {
      char buf[128];
      sprintf(buf, "name clash: id '%s' previously defined\n", t->head);
      Fatal(LINE, buf);
    }
    if (symInsert(Vars, symMake(symStr(t->head), g))) {
      char buf[128];
      sprintf(buf, "sym tab insert failed %s", t->head);
      Fatal(LINE, buf);
    }
    t = t->tail;
  }
  idlist = idList(ids);
  n = mkVar(idlist, g);
  return n;
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
  Node n, b;
  Sig g = typeid->n_sym->info;

  return declist(ids, g, tail);
}

Node yInsertType(char *id, Node typeid, ylist array)
{
  Sig g, h;
  g = typeid->n_sym->info;

  if (symLookup(Types, id)) {
    char buf[128];
    sprintf(buf, "name clash: type '%s' previously defined\n", id);
    Fatal(LINE, buf);
  }
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
  return mkType(id, g);
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

  if (params->n_r) {
    g->next = mkSiglist(params->n_r);
    g->size = g->next->size + 1;
  }
  else
    g->size = 1;
  return g;
}

static char *curproc;

Node yForward(char *id, Node params, Node rettype)
{
  Node t;
  Symbol m;
  Sig g;

  m = symLookup(Procs, id);
  if (m) {    // not error if forwarded
    char buf[128];
    sprintf(buf, "name clash: proc '%s' previously defined\n", id);
    Fatal(LINE, buf);
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
  
  return mkForward(id, params, rettype, g);
}

Node yProcPre(char *id, Node params, Node rettype)
{
  Node t;
  Symbol m;
  Sig g;

  m = symLookup(Procs, id);
  if (m) {    // not error if forwarded
    g = m->info;
    if (g->size != -1) {
      char buf[128];
      sprintf(buf, "name clash: proc '%s' previously defined\n", id);
      Fatal(LINE, buf);
    }
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
  tabPop(Vars);
  tabPop(Types);

  if (decls)
    return mkSeq(decls, stms);
  else
    return stms;
}

Node yLval(Node id)
{
  Symbol m;

  assert(id);
  if (id->oper == O_SYM)
    return id;
  m = symLookup(Vars, id->n_str);
  if (!m) {
    char buf[128];
    sprintf(buf, "invalid identifier %s", id->n_str);
    Fatal(LINE, buf);
  }
  id->sig = m->info;
  id->n_sym = m;
  id->oper = O_SYM;
  return id;
}

Node yCall(char *str, Node args)
{
  Symbol sym;
  Sig g, parms;
  Node n;
  
  sym = symLookup(Procs, str);
  if (!sym) {
    char buf[128];
    sprintf(buf, "invalid procedure %s\n", str);
    Fatal(LINE, buf);
  }
  //parms = mkSiglist(args);
  // check param sig

  n = mkCall(str, args);
  n->sig = ((Sig)sym->info)->under;
  return n;
}

Node yTypeid(char *str)
{
  Symbol sym;
  Sig g;
  sym = symLookup(Types, str);
  if (!sym) {
    char buf[128];
    sprintf(buf, "invalid typeid %s\n", str);
    Fatal(LINE, buf);
  }
  return mkSym(sym, NULL);
}

/*........................ end of yhelp.c ...................................*/
