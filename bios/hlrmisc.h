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
* CONTACT: clemens.broger@roche.com or detlef.wolf@roche.com                 *
*                                                                            *
*****************************************************************************/

/** 
 *   \file hlrmisc.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */



#ifndef DEF_hlrmisc_H
#define DEF_hlrmisc_H



/* miscellaneous functions not fitting in any other module */

/* declarations for free, malloc, strdup, realloc */
#include <stdlib.h>
#include <string.h>

/* ---- private stuff --- do not call or access from your progs ---- */
/* private */ extern char *hlr_strmcpyI(char *to, char *from, int toLength) ;
/* private */ extern char *hlr_strcpysFunc(char *to, char *from, int toLengthMax) ;
/* private */ extern int hlr_allocCnt ;
/* safe versions of standard memory allocation routines
   performing die() (from log.c) if allocation fails */
/* private */ void *hlr_mallocs(size_t size) ;
/* private */ void *hlr_callocs(size_t nelem, size_t elsize) ;
/* private */ char *hlr_strdups(char *s1) ;
/* ---- end of private stuff -------- */


/* macros for memory managment
   please use those instead of the plain C library
   routines if you want to find memory leaks and verify allocation success
   if you compile with MALLOC_NOCHECK defined, the plain C routines
   are used.
   hlr_mallocExtern is provided to keep track of memory
   allocated by external rouines like gdbm_fetch()
*/
#ifdef MALLOC_NOCHECK
  /* free only when allocated */
#define hlr_free(x)  ((x) ? free(x), x=NULL, 1 : 0)
#define hlr_strdup   strdup
#define hlr_malloc   malloc
#define hlr_calloc   calloc
#define hlr_mallocExtern() 

#else
  /* count allocations and check for allocation success */ 
#define hlr_free(x)  ((x) ? free(x), --hlr_allocCnt, x=0, 1 : 0)
#define hlr_strdup(s) (++hlr_allocCnt, hlr_strdups(s)) 
#define hlr_malloc(n) (++hlr_allocCnt, hlr_mallocs(n)) 
#define hlr_mallocExtern() (++hlr_allocCnt)
#define hlr_calloc(nelem,elsize)  (++hlr_allocCnt, hlr_callocs(nelem,elsize))
#endif

#define hlr_realloc realloc

/**
 * Get the number of pending allocations. 
 */
#define hlr_getAllocCnt()  hlr_allocCnt  


/* copy string into a fixed sized array of char with truncation
   'strmcpy' mnemonic: 'string maximal copy'
   usage: 
     char s[6] ;
     hlr_strmcpy(s, "hello bufffer!") ;
   note: 'to' must not be an expression with side-effects
   see also: hlr_strcpys() below
*/
#define hlr_strmcpy(to, from) hlr_strmcpyI(to, from, sizeof(to))


/* like strcpy, but save against overflow
   like hlr_strmcpy, but die() instead of silent truncation
   copy 'from' to 'to' while checking that 'from' fits into 'to' 
   'to' must be of type char[]
*/
#define hlr_strcpys(to,from) hlr_strcpysFunc((to),(from),sizeof(to)-1)



/* be careful with all of the following macros:
   do not to use expressions with side effects as parameters,
   since they might be evaluated serveral times
*/
#define hlr_strdup0(s) ((s) ? hlr_strdup(s) : NULL)


/**
 * Convert an integer into a string. 
 */                          
#define hlr_itoa(s,i) sprintf(s,"%d",i)
#define HLR_ITOA_SIZE 21

/* usage:
   char str[HLR_ITOA_SIZE] ;
   int i=81062 ;
   hlr_itoa(str,i) ;
The string must be at least 21 bytes long because
the result can be up to 20 bytes
(see ULONG_MAX in limits.h)
*/



#ifndef MIN
/**
 * Minimum of a and b.
 */
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif


#ifndef MAX
/**
 * Maximum of a and b.
 */
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif


/**
 * Number of elements in a.
 */
#define NUMELE(a) ((int)sizeof(a)/sizeof(a[0]))
/*
  char *values[2] ;
  printf("%d", NUMELE(values) ;
  --> prints 2
*/


/* printf("%s", NULL) works on SGI, but not on WinNT or Solaris.
   Therefore to be safe, use
   printf("%s", s0f(s)) instead of printf("%s", s)
   If you are desparate for speed or s is an expression with side effects,
   you may replace s0f(s) with s0(s)
   which is a macro, trading the overhead of a function call
   for evaluating 's' twice. Beware of side-effects when using the macro!
*/
#define s0(s) ((s)?(s):"(null)")
extern char *s0f(char *s) ;

/* a universal pointer to an illegal address;
   referencing it caused e.g. a BUS ERROR;
   use this to initialize local variable to enforce
   assigning a value before use
*/
#define ILLADR (void*)0xFFFFFFFF

/* safe starter for system() calls: */
extern int hlr_system(char *cmd, int nonZeroOK) ;

#endif
