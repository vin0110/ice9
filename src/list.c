/******************************************************************************
 *
 *  File Name........: list.c
 *
 *****************************************************************************/

/*........................ Include Files ....................................*/
#include "list-pack.h"

/*........................ Local Defines ....................................*/

/*........................ Glocal Variables .................................*/

/*........................ External Functions ...............................*/
extern void *malloc(int), free(void *);

/*-----------------------------------------------------------------------------
 *
 * Name...........: listMake
 *
 */

List listMake()

{
  List l;

  l = malloc(sizeof(ListRecord));
  if ( l == NULL )
    return NULL;

  l->size = 0;
  l->head = l->tail = l->current = NULL;
  return l;
} /*---------- End of listMake ----------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: 
 *
 */

void listDestroy(List L, void (*freeFtn)(void *))

{
  Entry e, t;

  if ( L == NULL )
    return;

  e = L->head;
  while ( e != NULL ) {
    (*freeFtn)(e->item);
    t = e->next;
    free(e);
    e = t;
  }

  free(L);
  return;
} /*---------- End of listDestroy -------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listSize
 *
 */

int listSize(List L)

{
  if ( L == NULL )
    return 0;

  return L->size;
} /*---------- End of listSize ----------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listPush
 *
 */

int listPush(List L, void *item)

{
  if ( L == NULL )
    return 1;

  listFirst(L);
  return listPutC(L, item);
} /*---------- End of listPush ----------------------------------------------*/


/*-----------------------------------------------------------------------------
 *
 * Name...........: listPutC
 *
 */

int listPutC(List L, void *item)

{
  Entry e;

  if ( L == NULL )
    return 1;

  e = malloc(sizeof(EntryRecord));
  if ( e == NULL )
    return 1;

  e->item = item;

  if ( L->current == NULL ) {
    /* empty list */
    e->next = e->prev = NULL;
    L->head = L->current = L->tail = e;
  }
  else {
    e->prev = L->current->prev;
    e->next = L->current;
    if ( L->head == L->current )
      L->head = e;
    if ( L->current->prev != NULL )
      L->current->prev->next = e;
    L->current->prev = e;
    L->current = e;
  }
  ++L->size;

  return 0;
} /*---------- End of listPutC ----------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listInsert
 *
 */

int listInsert(List L, void *item)

{
  Entry e;

  if ( L == NULL )
    return 1;

  e = malloc(sizeof(EntryRecord));
  if ( e == NULL )
    return 1;

  e->item = item;
  e->next = NULL;
  e->prev = L->tail;

  if ( L->tail )
    L->tail->next = e;		/* at least on element in L */
  else
    L->head = e;		/* no elements in L */

  L->tail = L->current = e;
  ++L->size;

  return 0;
} /*---------- End of listInsert --------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listPop
 *
 */

void *listPop(List L)

{
  void *item;
  Entry e;

  if ( L == NULL || L->head == NULL )
    return NULL;

  e = L->head;
  if ( e->next == NULL )
    /* one element is the list */
    L->head = L->tail = L->current = NULL;
  else {
    L->head = L-> current = e->next;
    L->head->prev = NULL;
  }

  --L->size;
  item = e->item;
  free(e);
  return item;
} /*---------- End of listPop -----------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: 
 *
 */

int listApplyAll(List L, int (*ftn)(void *, void *), void *env)

{
  int rc;

  if ( L == NULL )
    return 0;

  for ( L->current = L->head; L->current != NULL; 
        L->current = L->current->next ) {
    rc = (*ftn)(L->current->item, env);
    if ( rc != 0 )
      return rc;
  }
  L->current = L->tail;
  return 0;
} /*---------- End of listApplyAll ------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listFirst
 *
 */

void *listFirst(List L)

{
  if ( L == NULL || L->current == NULL )
    return NULL;

  L->current = L->head;
  return L->current->item;

} /*---------- End of listFirst ---------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listLast
 *
 */

void *listLast(List L)

{
  if ( L == NULL || L->current == NULL )
    return NULL;

  L->current = L->tail;
  return L->current->item;

} /*---------- End of listLast ----------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listNext
 *
 */

void *listNext(List L)

{
  if ( L == NULL || L->current == NULL )
    return NULL;

  if ( L->current->next == NULL )
    return NULL;

  L->current = L->current->next;
  return L->current->item;

} /*---------- End of listNext ----------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listPrev
 *
 */

void *listPrev(List L)

{
  if ( L == NULL || L->current == NULL )
    return NULL;

  if ( L->current->prev == NULL )
    return NULL;

  L->current = L->current->prev;
  return L->current->item;

} /*---------- End of listPrev ----------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listCurrent
 *
 */

void *listCurrent(List L)

{
  if ( L == NULL || L->current == NULL )
    return NULL;

  return L->current->item;

} /*---------- End of listCurrent -------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listDeleteC
 *
 */

void *listDeleteC(List L)

{
  Entry e;
  void *item;

  if ( L == NULL || L->current == NULL )
    return NULL;

  e = L->current;
  if ( L->current->next == NULL ) {
    /* current is last in list */
    L->current = L->current->prev;
    L->tail = L->current;
    if ( L->current != NULL ) 
      L->current->next = NULL;
    else
      L->head = NULL;
  }
  else if ( L->current->prev == NULL ) {
    /* current is first in list */
    L->current = L->current->next;
    L->head = L->current;
    L->current->prev = NULL;
  }
  else {
    /* current is not at the head or tail of list */
    L->current = L->current->next;
    e->next->prev = e->prev;
    e->prev->next = e->next;
  }

  --L->size;
  item = e->item;
  free(e);
  return item;
} /*---------- End of listDeleteC -------------------------------------------*/

/*-----------------------------------------------------------------------------
 *
 * Name...........: listApplyC
 *
 */

int listApplyC(List L, int (*ftn)(void *, void *), void *env)

{
  if ( L == NULL || L->current == NULL )
    return 0;

  return (*ftn)(L->current->item, env);

} /*---------- End of listApplyC --------------------------------------------*/

/*........................ end of list.c ....................................*/
