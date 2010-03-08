/******************************************************************************
 *
 *  File Name........: yhelp.h
 *
 *  Description......:
 *
 *  Created by vin on 01/11/02
 *
 *  Revision History.:
 *
 *  $Log: yhelp.h,v $
 *  Revision 1.2  2002/01/18 20:47:09  vin
 *  Three working phases: all the way to CG'g C code.
 *  Robustness not verified.
 *
 *  Revision 1.1  2002/01/16 22:14:04  vin
 *  The basics of semantics are in and working.
 *
 *
 *  $Id: yhelp.h,v 1.2 2002/01/18 20:47:09 vin Exp $
 *
 *****************************************************************************/

#ifndef YHELP_H
#define YHELP_H

#include "ast.h"

typedef struct _ylist {
  void *head;
  struct _ylist *tail;
} *ylist, ylist_t;

ylist yList(void *, ylist);
void yListFree(ylist, int);
void yFa(char *);

Node yInsertVar(ylist, Node, ylist);
Node yDeclist(ylist, Node, Node);
Node yInsertType(char *, Node, ylist);
Node yForward(char *, Node, Node);
Node yProcPre(char *, Node, Node);
Node yProcPost(char *, Node, Node);
Node yLval(Node);
Node yQuest(Node);
Node yCall(char *, Node);
Node yTypeid(char *);

#endif /* YHELP_H */
/*........................ end of yhelp.h ...................................*/
