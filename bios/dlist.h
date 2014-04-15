/* 
 * dlist.h - Headers for generic doubly-linked list routines. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. 
 */


/** 
 *   \file dlist.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#ifndef DEF_DLIST_H
#define DEF_DLIST_H



/**
 * A node in a doubly linked list. 
 */
typedef struct _dlNode_ {
  struct _dlNode_ *next;
  struct _dlNode_ *prev;
  void *val;
} dlNode;



/** 
 * A doubly linked list. 
 */
typedef struct {
  dlNode *head;
  dlNode *nullMiddle;
  dlNode *tail;
} dlList;


/**
 * True if node past end. 
 */
#define dlEnd(node) (node->next == NULL)

/** 
 * True if node before start. 
 */
#define dlStart(node) (node->prev == NULL)


/* Iterate on a doubly linked list as so:
    for (el = list->head; !dlEnd(el); el = el->next)
        val = el->val;
   or
    for (el = list->tail; !dlStart(el); el = el->prev)
        val = el->val;
 */


dlList *dlListNew (void);
void dlListInit(dlList *dl);
void dlListReset(dlList *dl);
void dlListFree(dlList **pList);
void dlListFreeAndVals(dlList **pList);
void dlAddBefore(dlNode *anchor, dlNode *newNode);
void dlAddAfter(dlNode *anchor, dlNode *newNode);
void dlAddHead(dlList *list, dlNode *newNode);
void dlAddTail(dlList *list, dlNode *newNode);
dlNode *dlAddValBefore(dlNode *anchor, void *val);
dlNode *dlAddValAfter(dlNode *anchor, void *val);
dlNode *dlAddValHead(dlList *list, void *val);
dlNode *dlAddValTail(dlList *list, void *val);
void dlRemove(dlNode *node);
void dlRemoveHead(dlList *list);
void dlRemoveTail(dlList *list);
dlNode *dlPopHead(dlList *list);
dlNode *dlPopTail(dlList *list);
void dlDelete(dlNode **nodePtr);
int dlCount(dlList *list);
int dlEmpty(dlList *list);

/** 
 * Return 1 if list is empty. 
 */
#define dlIsEmpty(list) ((list)->head->next == NULL)


dlNode *dlGetBeforeHead(dlList *list);
dlNode *dlGetAfterTail(dlList *list);
void dlSort(dlList *list, int (*compare )(const void *elem1,  const void *elem2));
void dlCat(dlList *a, dlList *b);
dlNode *dlValInList(dlList *list, void *val);

#endif 
