/******************************************************************************
 *
 *  File Name........: list.h
 *
 *  Description......: Contains the typedefs of the linked list.
 *
 *****************************************************************************/

#ifndef LIST_H
#define LIST_H

#include <stdio.h>

/*........................ Type Definitions .................................*/
typedef struct ENTRY {
  struct ENTRY *next, *prev;
  void *item;
} EntryRecord, *Entry;

typedef struct {
  Entry head, tail, current;
  int size;
} ListRecord, *List;

#endif /* LIST_H */


/*........................ end of list.h ....................................*/
