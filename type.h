/******************************************************************************
 *
 *  File Name........: type.h
 *
 *  Description......:
 *
 *  Created by vin on 01/10/02
 *
 *  Revision History.:
 *
 *  $Log: type.h,v $
 *  Revision 1.2  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.1  2002/01/16 22:14:04  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: type.h,v 1.2 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/

#ifndef TYPE_H
#define TYPE_H
#include "symtab.h"

typedef enum { 
  T_INT, T_BOOL, T_STR, T_PROC, T_ARRAY, T_LIST, T_NAME, T_NIL, T_ERR
} Type;

typedef struct _sig {
  Type type;
  struct _sig *under, *next;
  int size;
  char *name;
  Symbol sym;
} *Sig, sig_t;

Sig sigMake(Type);
void sigDestroy(Sig);
Sig sigCopy(Sig);
int sigCmp(Sig, Sig);
void sigPrint(Sig);

#endif /* TYPE_H */
/*........................ end of type.h ....................................*/
