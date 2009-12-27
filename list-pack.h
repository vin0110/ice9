/******************************************************************************
 *
 *  File Name........: list-pack.h
 *
 *  Description......: Provides all the function prototypes to
 *	implement a list ADT.  The operations that can be performed on
 *	a list range from creating, inserting and removing to applying
 *	a function to all items.  A user defined pointer is stored
 *	with every list element.
 *
 *	Each list has a current pointer which can be used to walk thru
 *	the list.  With this (and some other operations) a sorted list
 *	can be created.  For example, the following code will examine each
 *	list element until it locates where to place newItem in the 
 *	sorted list.
 *
 *	List L;		// assumed to be initialized and non-trivial
 *	userItem *item;
 *	userItem *newItem;	// the new item to be inserted
 *
 *	for ( item = listFirst(L); item != NULL; item = listNext(L) )
 *	  if ( stpcmp(newItem->name, item->name) >= 0 ) {
 *	    listPutC(L, newItem);
 *	    break;
 *	  }
 *	}
 *	if ( item == NULL )
 *	  listInsert(L, newItem);	// newItem belongs at end of list
 *
 *	In addition to the current pointer the user can apply a
 *	function to the items in the list.  Suppose there is a
 *	function printItem() that will print the fields in the user
 *	defined structure inserted in the list.  Then:
 *		listApplyAll(L, printItem);
 *	would print all the items in the list.
 *
 *****************************************************************************/

#ifndef LISTPACK_H
#define LISTPACK_H

#include "list.h"

/*........................ Function Prototypes ..............................*/

/*----- Creates and returns a new empty list.  Returns NULL if a new list
	cannot be created. */
List listMake(void);

/*----- Destroys the list L.  If not NULL, applies the function freeFtn 
	to each item in L.*/ 
void listDestroy(List L, void (*freeFtn)(void *));

/*----- Returns the number of items in L. */
int  listSize(List L);
#define listEmpty(L)	(listSize(L) == 0)

/*----- Inserts item at the head of the list L.  The new item is now the 
	current item.  Returns 0 if OK, non-zero otherwise. */
int listPush(List L, void *item);

/*----- Inserts item at the tail of the list L.  The new item is now the 
	current item.  Returns 0 if OK, non-zero otherwise. */
int listInsert(List L, void *item);


/*----- Removes and returns the item at the head of list L.  Returns NULL
	if list is empty.  The current item is now the first item in the
	list. */
void *listPop(List L);
#define listRemove listPop

/*----- Applies the function ftn to the items in list L, starting with the
	first.  If ftn returns a non-zero number then the operation
	stops and the value returned by ftn is returned.  The current item
	is the last item to which ftn was applied.  */
int  listApplyAll(List L, int (*ftn)(void *, void *), void *env);

/*----- Sets current item to the first (last) item in L.  Returns that item.
 	Returns NULL if list is empty. */
void *listFirst(List L);
void *listLast(List L);

/*----- Sets current item to the item following (preceding) the current item.
	Returns new current item.  Returns NULL if there are no items 
	following (preceding) the current item (does not change current). */
void *listNext(List L);
void *listPrev(List L);

/*----- Returns the current item in L.  Returns NULL if L is empty. */
void *listCurrent(List L);

/*----- Inserts item in the list immediately before the current item.  Sets
 	current to the item just inserted.  Returns 0 if OK, non-zero 
	otherwise. */
int listPutC(List L, void *item);

/*----- Removes the current item from the list.  Returns the item.  Sets
	current to the next item in the list, if there is now no next item it
	set it to the previous item. */
void *listDeleteC(List L);

/*----- Applies the function ftn to the current item, returns the value
	returned by ftn.  The first argument to ftn is a pointer to 
	the item.  The second argument is the env pointer. */
int listApplyC(List L, int (*ftn)(void *, void *), void *env);


#endif /* LISTPACK_H */

/*........................ end of list-pack.h ...............................*/
