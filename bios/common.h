/* 
 * Common.h - functions that are commonly used. 
 *
 * This file is copyright 2002-2005 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. 
 */


/** 
 *   \file common.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */




#ifndef DEF_COMMON_H	
#define DEF_COMMON_H


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include "format.h"


/**
 * TableRow.
 */
typedef struct {
  Texta tableColumns;
} TableRow;



/****************************************************************************************
*  Memory functions
****************************************************************************************/


void *needMem (size_t size);
void *needLargeMem (size_t size);
void *needLargeZeroedMem (size_t size);
void *needLargeMemResize (void* vp, size_t size);
void *needLargeZeroedMemResize (void* vp, size_t oldSize, size_t newSize);
void *cloneMem (void *pt, size_t size);


/** 
 * Allocate copy of a structure.
 */
#define CloneVar(pt) cloneMem(pt, sizeof((pt)[0]))


void freeMem (void *pt);
void freez (void *ppt);


/**
 * Shortcut to allocating a single variable on the heap and assigning pointer to it. 
 */
#define AllocVar(pt) (pt = needMem(sizeof(*pt)))


/**
 * Shortcut to allocating a variable on heap of a specific type.
 */
#define AllocA(type) needMem(sizeof(type))




/****************************************************************************************
* Other Functions
****************************************************************************************/


void zeroBytes (void *vpt, int count);     
void reverseBytes (char *bytes, long length);

Texta readList (char* fileName);
Array readTable (char* fileName, char* delimiter);


#endif 
