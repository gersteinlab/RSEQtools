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
 *   \file array.c Dynamic arrays
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "array.h"

static char* mallocErrorMsg = "array: malloc/realloc/calloc failed." ;

static int nArrays = 0 ;


Array uArrayCreate(int n, int size)
{ 
  Array new = (Array) malloc(sizeof(struct ArrayStruct)) ;

  if (!new)
    die(mallocErrorMsg) ;
  if (size <= 0)
    die("negative size %d in uArrayCreate", size) ;
  if (n < 1)
    n = 1 ;
  new->base = calloc(n, size) ;
  if (!new->base)
    die(mallocErrorMsg) ;
  new->dim = n ;
  new->max = 0 ;
  new->size = size ;
  nArrays++ ;
  return new ;
}



static void arrayExtend (Array a, int n)
{
  char *new ;
  int oldsize, newsize ;

  if (!a || n < a->dim)
    return ;

  if (a->dim < 1 << 20)  /* 1 megabyte */
    a->dim *= 2 ;
  else
    a->dim += 1 << 20 ;
  if (n >= a->dim)
    a->dim = n + 1 ;

  newsize = a->dim * a->size ;
  oldsize = a->size*a->max ;
  if (newsize <= oldsize) 
    die("arrayExtend: oldsize %d, newsize %d", oldsize, newsize) ;
  new = malloc(newsize) ;
  if (!new)
    die(mallocErrorMsg) ;
  memcpy(new, a->base, oldsize) ;
  memset(new+oldsize, 0, newsize-oldsize) ;
  free(a->base) ;
  a->base = new ;
}



/**
 * Clear contents of Array a. 
 */
void arrayClear(Array a)
{ 
  memset(a->base, 0, (size_t)(a->dim * a->size)) ;
  a->max = 0 ;
}



void uArrayDestroy (Array a)
{
  if(a) {
    free (a->base) ;
    free(a) ;
    nArrays-- ;
  }
}



/**
 * Returns number of Arrays currently allocated.
 */
int arrayNumber(void)
{ 
  return nArrays ;
}



char *uArray(Array a, int i) 
{
  if (i >= a->max) { 
    if (i >= a->dim)
      arrayExtend (a,i) ;
    a->max = i+1 ;
  }
  return a->base + i*a->size ;
}



char *uArrayCheck(Array a, int i)
{
  if (i < 0)
    die ("array: referencing array element %d < 0", i) ;

  return uArray(a, i) ;
}



char *uArrCheck(Array a, int i)
{
  if (i >= a->max || i < 0)
    die ("array index %d out of bounds [0,%d]",
	       i, a->max - 1) ;
  return a->base + i*a->size ;
}



char *uArrPop(Array a)
{
  int i = --a->max ;
  if (i < 0)
    die ("stackPop: empty stack") ;
  return a->base + i*a->size ;
}


/**
 * Copies an Array a.
 */
Array arrayCopy(Array a)
{
  Array b ;
  if (a && a->size) {
    b = uArrayCreate (a->max, a->size) ;
    memcpy(b->base, a->base, a->max * a->size);
    b->max = a->max ;
    return b;
  }
  else
    return NULL ;
}



/**
 * Move elements from an Array to another Array.
 * Move elements from[start..end] to to[m..m+end-start+1].
 * @param[in] from Array to cut elements out of 
 * @param[in] start Start index in 'from'
 * @param[in] end End index in 'from' 
 * @param[in] to Array to append cut elements to. NULL if this is not needed (only a good idea if elements don't own memory via pointers). 
 * @param[out] from Elements removed; formerly used space. after end of data filled with binary zeros.
 * @param[out] to If not NULL: moved elements from 'from' appended. 
 */
void arrayMove(Array from, int start, int end, Array to) 
{ 
  int i ;
  int mf ; /* number of elements in 'from' */
  int n ;  /* number of elements to move */
  int nb ; /* number of bytes to move from 'from' to 'to' */
  int mb ; /* number of bytes to move down within 'from' */
  char *fromp ; /* pointer to start position in 'from' */
  char *fromp2 ; /* beginning of area to shift in 'from' */
  char *fromp3 ; /* beginning of area to clear in 'from' */
  char *top ;   /* pointer to target position in 'to' */

  if (!from || start < 0 || end >= arrayMax(from) || start > end)
    die("arrayMove: max=%d start=%d end=%d",
        from ? arrayMax(from) : -1, start, end) ;
  if (to && to->size != from->size)
    die("arrayMove: size mismatch %d/%d",
        from->size, to->size) ;
  if (to == from)
    die("arrayMove: from and to are the same.") ;

  mf = arrayMax(from) ;
  n = end - start + 1 ; /* number of elements to move */
  nb = n * from->size ; /* number of bytes to move and set to zero */
  fromp = from->base + start*from->size ;
  if (to) {
    int mt = arrayMax(to) ;  /* number of elements in 'to' */
    uArray(to, mt + n - 1) ; /* allocate target space */
    top = to->base + mt*to->size ;
    memcpy(top, fromp, nb) ;
  }
  mb = (mf - end -1) * from->size ;
  if (mb) {
    fromp2 = from->base + (end+1)*from->size ;
    i = mb ;
    while (i--) 
      *fromp++ = *fromp2++ ; /* shuffle down */
  }
  fromp3 = from->base + (mf - n)*from->size ;
  memset(fromp3, 0, nb) ;
  from->max = mf - n ;
}



/**
 * Remove byte-wise identical entries. 
 * @param[in] a Array sorted such that byte-wise identical entries are adjacent
 * @param[out] a Byte-wise identical duplicates removed
 * @note Byte-wise identical can be different from contents-wise
 * @note Use arrayUniq() for duplicate removal based on equality defined by order() function
 */
void arrayByteUniq(Array a)
{ 
  int i, j, k , as ;
  char *x, *y, *ab  ;
  
  if (!a || !a->size || arrayMax(a) < 2 )
    return ;

  ab = a->base ; 
  as = a->size ;
  for (i = 1, j = 0 ; i < arrayMax(a) ; i++)
    { x = ab + i * as ; y = ab + j * as ;
      for (k = a->size ; k-- ;)		
	if (*x++ != *y++) 
	  goto different ;
      continue ;
      
    different:
      if (i != ++j)
	{ x = ab + i * as ; y = ab + j * as ;
	  for (k = a->size ; k-- ;)	 
	    *y++ = *x++ ;
	}
    }
  arrayMax(a) = j + 1 ;
}



/**
 * @param[in] a Array sorted by the same order function that is specified as the second argument
 * @param[in] b Array of same type as a, NULL ok (if one is not interested in getting the duplicates)
 * @param[out] a All duplicates which are identical according to the specified order function are moved to b
 * @param[out] b If not NULL: duplicates appended
 * @note If byte-wise equality is sufficient and array elements do not own memory outside the Array, 
   one can use  arrayByteUniq() instead of this function to save a few CPU cycles.
 */
void arrayUniq(Array a, Array b, int (*order)(void*,void*))
{ 
  int i, j, k;
  char *to ; 
  char *from ;
  char *r ;
  
  if (!a || !a->size || (b && a->size != b->size))
    die("arrayUniq: bad input") ;

  if (arrayMax(a) < 2)
    return ;

  j = 0;
  to = a->base;
  for (i=1; i<arrayMax(a); i++) {
    from = a->base + i * a->size;
    if (order (to, from)) { /* differ: stay in a */
      to += a->size ;
      if (to != from) {
        r = to;
        k = a->size ;
        while (k--)
          *r++ = *from++;
      }
      j++;
    }
    else {  /* equal: move to b */
      if (b) {
        r = uArray(b, b->max) ;
        k = a->size ;
        while (k--)
          *r++ = *from++;
      }
    }
  }
  arrayMax(a) = j+1;
  return;
}



/**
 * Sort an Array a according to order of order().
 */
void arraySort (Array a, int (*order)(void*,void*))
{
  unsigned int n = a->max,
               s = a->size ;
  void *v = a->base ;

  if (n > 1) 
#ifndef THINK_C
    qsort(v, n, s, (int (*)(const void *, const void *)) order) ;
#else 
    { extern void qcksrt (char *base, int n, int size, 
			  int (*comp)(void*, void*)) ;
      qcksrt (v, n, s, order) ;
    }
#endif

}



/**
 * Check if s is an entry in a.
 * @return TRUE if arru(a,i,) matches *s, else FALSE
 */
int arrayIsEntry (Array a, int i, void *s)
{
  char *cp = uArray(a,i), *cq = s ;
  int j = a->size;

  while (j--)
    if (*cp++ != *cq++) 
      return 0 ;
  return 1;
}



/**
 * Finds Entry s from Array a  sorted in ascending order of order().
 * @param[in] ip If NULL, then no output
 * @param[out] ip If ip is not NULL, then if found: sets *ip to index found; 
   else: sets *ip one step left of expected index
 * @return 1 if found, else 0  
 */
int arrayFind(Array a, void *s, int *ip, int (* order)(void*, void*))
{

  int ord;
  int i = 0 , j = arrayMax(a), k;

  if(!j || (ord = order(s,uArray(a,0)))<0)
    /* array empty or s smaller than first element */
    { if (ip)
	*ip = -1; 
      return 0;    /* not found */
    }   

  if (ord == 0)  /* exact match on first element */
    { if (ip)
	*ip = 0;
      return 1;    /* found */
    }

  if ((ord = order(s,uArray(a,--j)))>0 )
    /* s larger than last element of array */
    { if (ip)
	*ip = j; 
      return 0;
    }
  
  if (ord == 0)  /* exact match on last element */
    { if (ip)
	*ip = j;
      return 1;
    }

  for (;;)
    { k = i + ((j-i) >> 1) ; /* midpoint */
      if ((ord = order(s, uArray(a,k))) == 0)
	{ if (ip)
	    *ip = k; 
	  return 1;
	}
      if (ord > 0) 
	(i = k);
      else
	(j = k) ;
      if (i == (j-1) )
        break;
    }
  if (ip)
    *ip = i ;
  return 0;
}



/**
 * Removes Entry s from Array a sorted in ascending order of order().
 * @return 1 if found, else 0  
 */
int arrayRemove (Array a, void * s, int (* order)(void*,void*))
{
  int i;

  if (arrayFind(a, s, &i,order))
    {
      /* memcpy would be faster but regions overlap
       * and memcpy is said to fail with some compilers
       */
      char *cp = uArray(a,i),  *cq = cp + a->size ;
      int j = (arrayMax(a) - i)*(a->size) ;
      while(j--)
	*cp++ = *cq++;

      arrayMax(a) --;
      return 1;
    }
  else
    return 0;
}



/**
 * Removes Entry with index i from Array a.
 */
int arrayRemoveD (Array a, int i)
{
  if (i<arrayMax(a)){
    /* memcpy would be faster but regions overlap
     * and memcpy is said to fail with some compilers
     */
    char *cp = uArray(a,i),  *cq = cp + a->size ;
    int j = (arrayMax(a) - i)*(a->size) ;
    while(j--)
      *cp++ = *cq++;

    arrayMax(a) --;
    return 1;
  }
  else
    return 0;
}



/**
 * Try to find element s in Array a; insert it if not there
 * @param[in] ip Where to put the index value; if ip is NULL, the index value is not returned
 * @param[out] ip Index of s (where found or inserted)
 * @return 1 if inserted, 0 if found
*/
int arrayFindInsert(Array a, void * s, int *ip, int (*order)(void*,void*)) 
{
  int i, j, k ;
  int *ip2 = ip ? ip : &i ;
  char *cp, *cq ;

  if (arrayFind(a, s, ip2, order)) 
    return 0;  /* no doubles */
  
  j = arrayMax(a) + 1;
  cp = uArray(a, arrayMax(a)) ; /* to create space */

	/* avoid memcpy for same reasons as above */
  {
    cp = cp + a->size - 1 ;  /* cp is now end of last element */
    cq = cp - a->size ;
    k = (j - *ip2 - 2)*(a->size); /* k = # of bytes to move */
    while(k--)
      *cp-- = *cq--;
    
    cp = arrp(a,*ip2+1,char); 
    cq = (char *) s; 
    k = a->size;
    while(k--)
      *cp++ = *cq++;
  }
  ++*ip2 ;
  return 1;
}



/**
 * Comparison function for strings (char*).
 */
int arrayStrcmp(char **s1, char **s2) 
{ 
  return strcmp(*s1, *s2) ;
}



/**
 * Comparison function for integers (int).
 */
int arrayIntcmp(int *ip1, int *ip2) 
{
  if (*ip1 == *ip2) return 0 ;
  return (*ip1 < *ip2) ? -1 : 1 ;
}



/**
 * Comparison function for doubles (double).
 */
int arrayDoublecmp(double *dp1, double *dp2) 
{
  if (*dp1 == *dp2) return 0 ;
  return (*dp1 < *dp2) ? -1 : 1 ;
}
