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
* CONTACT: clemens.broger@roche.com                                          *
*                                                                            *
*****************************************************************************/

#ifndef DEF_FORMAT_H
#define DEF_FORMAT_H


/** 
 *   \file format.h  
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#include <stdio.h>
#include <string.h>
#include "plabla.h"

#include "array.h"
#include "hlrmisc.h"

/* -----------------------------------------------------------
   do not refer to anything inside this struct from your programs */


/**
 * WordIter.
 */
typedef struct wordIterStruct {
  char *cp ;   /* current position */
  char *seps ;
  int manySepsAreOne ;
  int atEnd ;
} *WordIter ;
extern WordIter wordIterCreate(char *s, char *seps, int manySepsAreOne) ;

/* do not call in your programs: */
/* private */ extern void wordIterDestroy_func(WordIter thisOne) ;
/* private */ extern void textDestroyFunc(Array a) ;
/* ------------------------------------- end of private section */


/* ------------ string handling functions / Array of char -------------- */
/* do not use 'String' since this is already used in e.g.
   /usr/include/X11/Intrinsic.h:
   typedef char *String;
*/

/**
 * Array of char that is null-terminated (Stringa).
 */
#define Stringa Array

/**
 * stringCreateClear.
 * @see stringClear()
 * @see stringCreate()
 */
#define stringCreateClear(s,n) {if(s) stringClear(s); else s=stringCreate(n);}

/**
 * stringCreateOnce.
 * @see stringCreate()
 */
#define stringCreateOnce(s,n) {if(!s) s=stringCreate(n);}

/**
 * Destroy a Stringa.
 * @see arrayDestroy()
 */
#define stringDestroy arrayDestroy

/**
 * Convert Stringa to char*.
 */
#define string(stringa) arrp(stringa,0,char)
#define stringCp(stringa,index) arrp(stringa,index,char)
#define stringC(stringa,index) arru(stringa,index,char)

/**
 * Get the length of a Stringa (not counting the null-termination character).
 */
#define stringLen(stringa) (arrayMax(stringa)-1)

extern Stringa stringCreate(int initialSize) ;
extern void stringTerminate(Array s /* of char */) ;
extern void stringTerminateP(Array s /* of char */, char *cp) ;
extern void stringTerminateI(Array s /* of char */, int i) ;

extern void stringCat(Stringa s1, char *s2) ;
extern void stringCatInt(Stringa s, int i) ;
extern void stringCatChar (Stringa s,char c);
extern void stringNCat(Stringa s1, char *s2, int n) ;
extern void stringCpy(Stringa s1, char *s2) ;
extern void stringNCpy(Stringa s1, char *s2, int n) ;
extern void stringClear(Array s1) ;
extern void stringAdjust(Array s1) ;
extern void stringChop(Stringa s, int n) ;
extern void stringInsert(Stringa s, int p, char *i) ;
extern char *stringCut(Stringa s, int pos, int len) ;
extern int stringTrim(Stringa s /* of char */, char *left, char *right) ;
extern int stringTranslate(Stringa s, char *fromChars, char *toChars) ;
extern int stringPrintf(Stringa str, char *format, ...) ;
extern int stringAppendf(Stringa str, char *format, ...) ;
extern char *stringPrintBuf(char *format, ...) ;
extern int isEmptyString(Stringa s) ;


/* ------------ string handling functions / zero-terminated string --------- */
extern void strReplace(char **s1, char *s2) ;
extern void toupperStr(char *s) ; /* converts string to uppercase */
extern void tolowerStr(char *s) ; /* converts string to lowercase */
extern char *strCaseStr (char *s, char *t) ;  /* case-insensitive strstr() */
extern char *strCopySubstr(char *pszString, char cBegin, char cEnd, Array acSubstr /* of char */) ;
extern int strTranslate(char *s, char *fromChars, char *toChars) ;
extern int strTrim(char *s, char *left, char *right) ;
extern void strScramble(char *s) ;
extern void strUnscramble(char *s) ;
extern int isBlankStr(char *s) ;

/**
 * Check if s1 and s2 are different. 
 */
#define strDiffer(s1,s2) (strcmp((s1),(s2)) != 0)

/**
 * Check if s1 and s2 are equal. 
 */
#define strEqual(s1,s2) (strcmp((s1),(s2)) == 0)

/**
 * Check if the first n characters of s1 and s2 are different. 
 */
#define strNDiffer(s1,s2,n) (strncmp((s1),(s2),n) != 0)

/**
 * Check if the first n characters of s1 and s2 are equal. 
 */
#define strNEqual(s1,s2,n) (strncmp((s1),(s2),n) == 0)


#if BIOS_PLATFORM == BIOS_PLATFORM_IRIX || BIOS_PLATFORM == BIOS_PLATFORM_SOLARIS || BIOS_PLATFORM == BIOS_PLATFORM_LINUX
#define strCaseDiffer(s1,s2) (strcasecmp((s1),(s2)) != 0)
#define strCaseEqual(s1,s2) (strcasecmp((s1),(s2)) == 0)
#define strNCaseDiffer(s1,s2,n) (strncasecmp((s1),(s2),n) != 0)
#define strNCaseEqual(s1,s2,n) (strncasecmp((s1),(s2),n) == 0)
#endif

#if BIOS_PLATFORM == BIOS_PLATFORM_WINNT
#define strCaseDiffer(s1,s2) (stricmp((s1),(s2)) != 0)
#define strCaseEqual(s1,s2) (stricmp((s1),(s2)) == 0)
#define strNCaseDiffer(s1,s2,n) (strnicmp((s1),(s2),n) != 0)
#define strNCaseEqual(s1,s2,n) (strnicmp((s1),(s2),n) == 0)
#endif


extern int strEndsWith(char *s, char *suffix) ;

/**
 * Check if s1 starts with s2.
 * strStartsWith() always works, but is slower than strStartsWithC()
 * @note s2 must not be an expression with side effects since it is evaluated twice
 */
#define strStartsWith(s1,s2) (strncmp((s1),(s2),strlen(s2)) == 0)

/**
 * Check if s1 starts with s2.
 * Same as strStartsWith, but can only be used if s2 is a string constant, e.g. if (strStartsWithC(s, "CC   ")) ...strStartsWith() always works, but is slower than strStartsWithC()
 */
#define strStartsWithC(s1,s2) (strncmp((s1),(s2),sizeof(s2)-1) == 0)




/* ----- text -- dealing with Arrays of strings -------------- */


/**
 * A Texta is an Array of char*. The memory referenced by the char pointers belongs to the Texta
 */
#define Texta Array
extern Texta textClone(Texta a) ;
extern void textClear(Texta a) ;

/**
 * Create a Texta.
 * @see arrayCreate()
 */
#define textCreate(initialSize) arrayCreate(initialSize,char*)


/**
 * textCreateClear.
 * @see textClear()
 * @see textCreate()
 */
#define textCreateClear(t,n) {if(t) textClear(t); else t=textCreate(n);}

/**
 * Destroy a Texta.
 * @see textDestroyFunc()
 */
#define textDestroy(x)    ((x) ? textDestroyFunc(x), x=NULL, 1 : 0)
extern Texta textStrtok(char *s,char *sep);
extern Texta textStrtokP(char *s, char *sep) ;
extern Texta textFieldtok(char *s, char *sep);
extern Texta textFieldtokP(char *s, char *sep);
extern void textUniqKeepOrder(Texta t) ;  /* duplicate removal */

/**
 * Add a string to the end of a Texta.
 */
#define textAdd(t,s) (array((t),arrayMax(t),char*)=hlr_strdup(s))

/**
 * Get a pointer to the ith line in a Texta.
 */
#define textItem(text,index)  arru((text),(index),char*)
extern void textJoin(Stringa s, char *sep, Array a /* of char* */) ;



/* --- combination of strtok() and strField() with multiple instances --- */

/**
 * wordTokIterCreate.
 * @see wordIterCreate()
 */
#define wordTokIterCreate(s,seps) wordIterCreate(s,seps,1)

/**
 * wordFldIterCreate.
 * @see wordIterCreate()
 */
#define wordFldIterCreate(s,seps) wordIterCreate(s,seps,0)

/**
 * wordNext.
 * @see wordNextG()
 */
#define wordNext(this1) (wordNextG((this1),NULL))
char *wordNextG(WordIter this1, int *lenP) ;

/**
 * wordDestroy.
 * @see wordIterDestroy_func()
 */
#define wordIterDestroy(this) (wordIterDestroy_func(this), this=0)

/* usage:
  WordIter wi = wordTokIterCreate("hello world", " \n\t") ;
  char *w ;
  while (w = wordNext(wi)) 
    printf("w='%s'\n", w) ;
  wordIterDestroy(wi) ;
*/

/* --------- getLine(): like gets(), but with arbitrary length lines ------
  usage:
    static char *line = 0 ;
    static int buflen ;
    int len ;
    while (getLine(stdin, &line, &buflen)) { 
      len = stripNlGetLength(line) ;
      printf("%d %s\n", len, line) ;
    }
    hlr_free(line) ;
  notes: 
   - 'buflen' is not the length of the returned line
   - getLine() does not strip the trailing newline
   - 'line' and 'buflen' belong tightly together
*/

extern int getLine(FILE *stream, char **buffer, int *buflen) ;
extern int stripNlGetLength(char *line) ;


#endif
