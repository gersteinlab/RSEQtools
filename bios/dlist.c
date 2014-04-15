/* 
 * dlist.c - Doubly-linked list routines. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. 
 */

/** 
 *   \file dlist.c Generic doubly-linked list routines
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#include "common.h"
#include "dlist.h"


/** 
 * Initialize list to be empty. 
 */
void dlListInit(dlList *dl)
{
  dl->head = (dlNode *)(&dl->nullMiddle);
  dl->nullMiddle = NULL;
  dl->tail = (dlNode *)(&dl->head);
}



/** 
 * Return a new doubly linked list. 
 */
dlList *dlListNew(void)
{
  dlList *dl;
  AllocVar(dl);
  dl->head = (dlNode *)(&dl->nullMiddle);
  dl->tail = (dlNode *)(&dl->head);
  return dl;
}



/** 
 * Reset a list to the empty state (does not free values).  
 */
void dlListReset(dlList *dl)
{
  dlNode *node, *next;
  for (node = dl->head; node->next != NULL; node = next)
    {
      next = node->next;
      freeMem(node);
    }
  dl->head = (dlNode *)(&dl->nullMiddle);
  dl->nullMiddle = NULL;
  dl->tail = (dlNode *)(&dl->head);
}



/** 
 * Free up a doubly linked list and it's nodes, but not the node values. 
 */
void dlListFree(dlList **pList)
{
  dlList *list = *pList;
  if (list != NULL)
    {
      dlListReset(list);
      freez(pList);
    }
}



/**
 * Free all values in doubly linked list and the list itself.  
 * Just calls freeMem on all values. 
 */
void dlListFreeAndVals(dlList **pList)
{
  dlList *list = *pList;
  if (list != NULL)
    {
      dlNode *node;
      for (node = list->head; node->next != NULL; node = node->next)
        freeMem(node->val);
      dlListFree(pList);
    }
}



void dlInsertBetween(dlNode *before, dlNode *after, dlNode *newNode)
{
  before->next = newNode; 
  newNode->prev = before; 
  newNode->next = after;  
  after->prev = newNode; 
}



/**
 * Add a node to list before anchor member. 
 */
void dlAddBefore(dlNode *anchor, dlNode *newNode)
{
  dlInsertBetween(anchor->prev, anchor, newNode);
}



/**
 * Add a node to list after anchor member. 
 */
void dlAddAfter(dlNode *anchor, dlNode *newNode)
{
  dlInsertBetween(anchor, anchor->next, newNode);
}



/** 
 * Add a node to head of list. 
 */
void dlAddHead(dlList *list, dlNode *newNode)
{
  dlNode *head = list->head;
  dlInsertBetween(head->prev, head, newNode);
}



/**
 * Add a node to tail of list. 
 */
void dlAddTail(dlList *list, dlNode *newNode)
{
  dlNode *tail = list->tail;
  dlInsertBetween(tail, tail->next, newNode);
}



/**
 * Create a node containing val and add to list before anchor member. 
 */
dlNode *dlAddValBefore(dlNode *anchor, void *val)
{
  dlNode *node = AllocA(dlNode);
  node->val = val;
  dlAddBefore(anchor, node);
  return node;
}



/**
 * Create a node containing val and add to list after anchor member. 
 */
dlNode *dlAddValAfter(dlNode *anchor, void *val)
{
  dlNode *node = AllocA(dlNode);
  node->val = val;
  dlAddAfter(anchor, node);
  return node;
}



/**
 * Create a node containing val and add to head of list. 
 */
dlNode *dlAddValHead(dlList *list, void *val)
{
  dlNode *node = AllocA(dlNode);
  node->val = val;
  dlAddHead(list, node);
  return node;
}



/** 
 * Create a node containing val and add to tail of list. 
 */
dlNode *dlAddValTail(dlList *list, void *val)
{
  dlNode *node = AllocA(dlNode);
  node->val = val;
  dlAddTail(list, node);
  return node;
}



/**
 * Removes a node from list. Node is not freed. 
 */
void dlRemove(dlNode *node)
{
  dlNode *before = node->prev;
  dlNode *after = node->next;
  before->next = after;
  after->prev = before;
  node->prev = NULL;
  node->next = NULL;
}



/**
 * Removes head from list. Node is not freed. 
 */
void dlRemoveHead(dlList *list)
{
  dlRemove(list->head);
}



/** 
 * Remove tail from list. Node is not freed. 
 */
void dlRemoveTail(dlList *list)
{
  dlRemove(list->tail);
}



/**
 * Remove first node from list and return it. 
 */
dlNode *dlPopHead(dlList *list)
{
  dlNode *node = list->head;
  if (node->next == NULL)
    return NULL;
  dlRemove(node);
  return node;
}



/**
 * Remove last node from list and return it. 
 */
dlNode *dlPopTail(dlList *list)
{
  dlNode *node = list->tail;
  if (node->prev == NULL)
    return NULL;
  dlRemove(node);
  return node;
}



/**
 * Removes a node from list and frees it. 
 */
void dlDelete(dlNode **nodePtr)
{
  dlNode *node = *nodePtr;
  if (node != NULL)
    {
      dlRemove(node);
      freeMem(node);
    }
}



/**
 * Return length of list. 
 */
int dlCount(dlList *list)
{
  dlNode *node;
  int count = 0;

  for (node = list->head; !dlEnd(node); node = node->next) {
    count++;
  }
  return count;
}



struct dlSorter 
/* Helper structure for sorting dlNodes preserving order */
{
  dlNode *node;
};



static int (*compareFunc)(const void *elem1, const void *elem2);
/* Node comparison pointer, just used by dlSortNodes and helpers. */



static int dlNodeCmp(const void *elem1, const void *elem2)
/* Compare two dlSorters indirectly, by calling compareFunc. */
{
  struct dlSorter *a = (struct dlSorter *)elem1;
  struct dlSorter *b = (struct dlSorter *)elem2;
  return compareFunc(&a->node->val, &b->node->val);
}



/** 
 * Sort a singly linked list with Qsort and a temporary array. 
 * The arguments to the compare function in real, non-void, life are pointers to pointers 
   of the type that is in the val field of the nodes of the list. 
 */
void dlSort(dlList *list, int (*compare )(const void *elem1, const void *elem2))
{
  int len = dlCount(list);
  
  if (len > 1)
    {
      /* Move val's onto an array, sort, and then put back into list. */
      struct dlSorter *sorter = needLargeMem(len * sizeof(sorter[0])), *s;
      dlNode *node;
      int i;
      
      for (i=0, node = list->head; i<len; ++i, node = node->next)
	{
	  s = &sorter[i];
	  s->node = node;
	}
      compareFunc = compare;
      qsort(sorter, len, sizeof(sorter[0]), dlNodeCmp);
      dlListInit(list);
      for (i=0; i<len; ++i)
	dlAddTail(list, sorter[i].node);
      freeMem(sorter);
    }
}



/** 
 * Return 1 if list is empty. 
 */
int dlEmpty(dlList *list)
{
  return dlIsEmpty(list);
}



/** 
 * Get the node before the head of the list. 
 */
dlNode *dlGetBeforeHead(dlList *list)
{
  if (dlEmpty(list))
    return list->head;
  else
    return list->head->prev;
}



/**
 * Get the node after the tail of the list. 
 */
dlNode *dlGetAfterTail(dlList *list)
{
  if (dlEmpty(list))
    return list->tail;
  else
    return list->tail->next;
}



/**
 * Move items from b to end of a. 
 */
void dlCat(dlList *a, dlList *b)
{
  dlNode *node;
  while ((node = dlPopHead(b)) != NULL)
    dlAddTail(a, node);
}



/**
 * Return node on list if any that has associated val. 
 */
dlNode *dlValInList(dlList *list, void *val)
{
  dlNode *node;
  for (node = list->head; !dlEnd(node); node = node->next)
    if (node->val == val)
      return node;
  return NULL;
}
