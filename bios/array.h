/*****************************************************************************
* Copyright (C) 2002,  F. Hoffmann-La Roche & Co., AG, Basel, Switzerland.   *
*                                                                            *
* This file is part of "Roche Bioinformatics Software Objects and Services"  *
*                                                                            *
* This file is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Lesser General Public                 *
* License as published by the Free Software Foundation; either               *
* version 2.1 of the License, or (at your option) any later version.         *
*                                                                            *
* This file is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of             *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          * 
* Lesser General Public License for more details.                            *
*                                                                            *
* To obtain a copy of the GNU Lesser General Public License                  *
* please write to the Free Software                                          *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  *
* or visit the WWW site http://www.gnu.org/copyleft/lesser.txt               *
*                                                                            *
* SCOPE: this licence applies to this file. Other files of the               *
*        "Roche Bioinformatics Software Objects and Services" may be         *
*        subject to other licences.                                          *
*                                                                            *
* This file is derived from the ACEDB genome database package (as of 1995)   *
* written by Richard Durbin (MRC LMB, UK) rd@mrc-lmba.cam.ac.uk, and         *
* Jean Thierry-Mieg (CRBM du CNRS, France) mieg@kaa.cnrs-mop.fr, which is    *
* Copyright (C) J Thierry-Mieg and R Durbin, 1991                            *
*                                                                            *
* modifed by Detlef.Wolf@Roche.com and Clemens.Broger@roche.com              *
*                                                                            *
* CONTACT: clemens.broger@roche.com or detlef.wolf@roche.com                 *
*                                                                            *
*****************************************************************************/

/** 
 *   \file array.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#ifndef DEF_ARRAY_H
#define DEF_ARRAY_H
 
/* #define ARRAY_CHECK either here or in a single file to
   check the bounds on arru() and arrp() calls
   if defined here can be removed from specific C files by defining
   ARRAY_NO_CHECK 
*/

/* #define ARRAY_CHECK */

/**
 * Array.
 */
typedef struct ArrayStruct
{ char* base ;    // char* since need to do pointer arithmetic in bytes 
  int   dim ;     // length of alloc'ed space 
  int   size ;
  int   max ;     // largest element accessed via array() -1 
} *Array ;
 
/* NB we need the full definition for arru() for macros to work
   do not use it in user programs - it is private.
*/

extern Array   uArrayCreate (int n, int size) ;
extern void    uArrayDestroy (Array a) ;
extern char    *uArray (Array a, int index) ;
extern char    *uArrCheck (Array a, int index) ;
extern char    *uArrayCheck (Array a, int index) ;
extern char    *uArrPop(Array a) ;

/**
 * Create an Array of n elements having type type.
 */
#define arrayCreate(n,type)	uArrayCreate(n,sizeof(type))

/**
 * Destroy Array a.
 */
#define arrayDestroy(a)		((a) ? uArrayDestroy(a), a=NULL, 1 : 0)

/**
 * Return the number of elements in the array.
 */
#define arrayMax(ar)   ((ar)->max)


#if (defined(ARRAY_CHECK) && !defined(ARRAY_NO_CHECK))
#define arrp(ar,i,type)	((type*)uArrCheck(ar,i))
#define arru(ar,i,type)	(*(type*)uArrCheck(ar,i))
#define arrayp(ar,i,type)	((type*)uArrayCheck(ar,i))
#define array(ar,i,type)	(*(type*)uArrayCheck(ar,i))
#else
#define arru(ar,i,type)	((*(type*)((ar)->base + (i)*(ar)->size)))
#define arrp(ar,i,type)	(((type*)((ar)->base + (i)*(ar)->size)))
#define arrayp(ar,i,type)	((type*)uArray(ar,i))
#define array(ar,i,type)	(*(type*)uArray(ar,i))
#endif
/* only use arru()/arrp() when accessing an existing element */


extern Array  arrayCopy (Array a) ;
extern void   arrayMove(Array from, int start, int end, Array to) ;
extern void   arrayClear(Array a) ;

/* use with care -- arguments are evaluated twice (macro) */
#define arraySetMax(ar,j) (uArray(ar,j), (ar)->max = (j))
 
extern int arrayNumber(void) ; 


/* JTM's package to hold sorted arrays of ANY TYPE, extended by Roche */
#define ARRAYORDERF int(*)(void *,void *)
#define arrayInsert(a,elem,order) arrayFindInsert(a,elem,NULL,order) 
extern int     arrayFindInsert(Array a, void *s, int *ip, int (*order)(void*,void*));
extern int     arrayRemove(Array a, void * s, int (*order)(void*,void*));
extern int     arrayRemoveD(Array a,int i);
extern void    arraySort(Array a, int (*order)(void*,void*)) ;
extern int     arrayFind(Array a, void *s, int *ip, int (*order)(void*,void*));
extern int     arrayIsEntry(Array a, int i, void *s);
extern int     arrayStrcmp(char **s1, char **s2) ; 
extern int     arrayIntcmp(int *ip1, int *ip2) ;  
extern void    arrayByteUniq(Array a) ;
extern void    arrayUniq(Array a, Array b, int (*order)(void*,void*)) ;
extern int     arrayDoublecmp(double *dp1, double *dp2) ;  

/* using an Array as a push/pop stack */
/* boundary checking: arrayTop() and arrayTopp() check for
   empty Array only if ARRAY_CHECK is in effect.
   arrayPop() always checks for stack underflow
*/

#define Stacka Array
#define stackCreate  arrayCreate
#define stackDestroy arrayDestroy
#define stackDepth    arrayMax
#define stackPush(a,elem,type)  (array(a,arrayMax(a),type)=(elem))
#define stackTopp(a,type)	(arrp(a,arrayMax(a)-1,type))
#define stackTop(a,type)	(arru(a,arrayMax(a)-1,type))
#define stackPop(a,type)        ((*(type*)uArrPop(a)))


#endif

