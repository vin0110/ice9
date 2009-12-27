/******************************************************************************
 *
 *  File Name........: symtab.c
 *
 *  Description......:
 *
 *  Created by vin on 01/11/02
 *
 *  Revision History.:
 *
 *  $Log: symtab.c,v $
 *  Revision 1.3  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.2  2002/01/16 22:14:03  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: symtab.c,v 1.3 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/

#include <stdlib.h>
#include "symtab.h"
#include <assert.h>

Table tabMake()
{
  List l, r;
  l = listMake();
  r = listMake();
  listInsert(l, r);
  return l;
}

static void XsymDestroy(void *);

static void tabListDestroy(void *l)
{
  listDestroy((List)l, XsymDestroy);
}

int tabDestroy(Table t)
{
  List l;

  if (t == NULL)
    return -1;

  listDestroy(t, tabListDestroy);

  return 0;
}

static int tabPrint2(void *a, void *b)
{
  printf("%s\n", ((Symbol)a)->name);
  return 0;
}

static int tabPrint1(void *a, void *b)
{
  List l = (List)a;
  int *level = (void *)b;

  printf("Level %d\n", *level);
  *level = *level - 1;
  listApplyAll(l, tabPrint2, NULL);
  return 0;
}

void tabPrint(Table t)
{
  int level = tabLevel(t);
  listApplyAll(t, tabPrint1, &level);
}

Symbol symLookup(Table t, char *key)
{
  List l;
  Symbol m;
  int len;

  if (t == NULL || key == NULL)
    return NULL;

  len = strlen(key);
  if (len <= 0)
    return NULL;

  for (l = listFirst(t); l != NULL; l = listNext(t)) {
    for (m = listFirst(l); m != NULL; m = listNext(l)) {
      if (!strcmp(key, m->name)) {
	return m;
      }
    }
  }
  return NULL;
}

int symInsert(Table t, Symbol sym)
{
  List l;
  Symbol m;
  char *key;
  int len;

  if (t == NULL || sym == NULL)
    return -1;
  key = sym->name;
  len = strlen(key);
  if (len <= 0)
    return -2;

  l = listFirst(t);
  if (l == NULL)
    return -1;
  for (m = listFirst(l); m != NULL; m = listNext(l)) {
    if (!strcmp(m->name, key))
      return -3;
  }
  return listInsert(l, sym);
}

int tabPush(Table t)
{
  List l;

  if (t == NULL)
    return -1;

  l = listMake();
  return listPush(t, l);
}

int tabPop(Table t)
{
  if (t == NULL)
    return -1;
  return listPop(t) == NULL;
}

int tabLevel(Table t)
{
  return listSize(t);
}

Symbol symMake(char *key, void *info)
{
  Symbol sym;
  int len;

  if (key == NULL || (len = strlen(key)) <= 0)
    return NULL;

  sym = malloc(sizeof(symbol_t));
  if (sym == NULL)
    return NULL;

  sym->name = symStr(key);
  sym->info = info;
  return sym;
}

int symDestroy(Symbol sym)
{
  if (sym == NULL)
    return -1;
  free(sym->name);
  free(sym);
  return 0;
}

void XsymDestroy(void *a)
{
  symDestroy((Symbol)a);
}

char *symStr(char *key)
{
  char *name;
  int len = strlen(key);
  name = malloc(len+1);
  assert(name);
  strcpy(name, key);

  return name;
}
/*........................ end of symtab.c ..................................*/
