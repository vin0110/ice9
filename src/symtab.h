/******************************************************************************
 *
 *  File Name........: symtab.h
 *
 *  Description......:
 *
 *  Created by vin on 01/10/02
 *
 *  Revision History.:
 *
 *  $Log: symtab.h,v $
 *  Revision 1.2  2002/01/16 22:14:03  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: symtab.h,v 1.2 2002/01/16 22:14:03 vin Exp $
 *
 *****************************************************************************/

#ifndef SYMTAB_H
#define SYMTAB_H

#include "list-pack.h"

typedef List Table;
typedef struct {
  char *name;
  int level;
  int location;
  void  *info;	/* is a Sig, but use void * to avoid circular defintions. */
} *Symbol, symbol_t;

Table tabMake();
int tabDestroy(Table);
void tabPrint(Table);
void symPrint(Symbol);
Symbol symLookup(Table, char *key);
int symInsert(Table, Symbol);
int tabPop(Table);
int tabPush(Table);
int tabLevel(Table);
Symbol symMake(char *, void *);
int symDestroy(Symbol);
char *symStr(char *);
int varSetLocation(Table);

#endif /* SYMTAB_H */
/*........................ end of symtab.h ..................................*/
