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

/** 
 *   \file format.c  Collection of string handling utilities 
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


/* technical comments:

the functions collected here belong into x groups:

1. string handling, where string is Array of char
2. str handling, where str is a '\0'-terminated char[]
3. text handling, where text is an Array of char*
4. line and word handling
   (reading a line of arbitrary length,
    reading quotes strings,
    nestable strtok(),
    dissecting strings of the form dbname:seqname)

a note on terminolgy:
"Array" has a special meaning here: it denotes the dynamic array datatype,
see #include "array.h"
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "hlrmisc.h"
#include "log.h"
#include "format.h"

/* ---------- part 1: String = Array of char with 
                      arru(string,arrayMax(string)-1,char) == '\0' ----- */ 


/**
 * Create an array of char and make it null-terminated.
 *
 * \verbatim
 * This pattern occurs very often:
 * Stringa a = NULL ;
 * if (a)
 *   stringClear(a) ;
 * else
 *   a = stringCreate(n) ;
 * 
 * Therefore there is this shortcut:
 * #define stringCreateClear(s,n) {if(s) stringClear(s); else s=stringCreate(n);}
 * \endverbatim
 */
Array stringCreate(int initialSize) 
{
  Array a = arrayCreate(initialSize, char) ;
  array(a, 0, char) = '\0' ;
  return a ;
}



/**
 * Terminate string.
 * @param[in] s Array of char, not yet null-terminated
 * @param[out] s Now of type Stringa, i.e. an Array of char* (null-terminated), 
   suitable for use in all string* functions.
 */
void stringTerminate(Array s /* of char */)
{
  if (!s || !arrayMax(s))
    die("stringTerminate: not a Stringa") ;
  if (!arru(s, arrayMax(s)-1, char))
    die("stringTerminate: already terminated") ;
  array(s, arrayMax(s), char) = '\0' ;
}



/**
 * Terminate string at position 'cp'.
 * @param[in] cp A pointer to a position within the string
 * @param[in] s A valid Stringa
 * @param[out] s *cp in s will be null-terminated and stringLen(s) is adjusted
 * @note stringTerminateP (s, string (s)) is equivalent to stringClear (s)
 */
void stringTerminateP(Array s /* of char */, char *cp) 
{ 
  stringTerminateI(s, cp-string(s)) ;
}



/**
 * Terminate string at the i+1 char.
 * @param[in] i An index in s, 0 <= i <= stringLen (s), if i > stringLen (s) or i < 0 is an error
 * @param[in] s A valid Stringa of length i
 * @param[out] s *cp in s will be null-terminated and stringLen(s) is adjusted
 * @note stringTerminateI (s,0) is equivalent to stringClear (s)
 */
void stringTerminateI(Array s /* of char */, int i) 
{ 
  if (i<0 || i>stringLen(s))
    die("stringTerminateI: s='%s', i=%d", string(s), i) ;
  arru(s, i, char) = '\0' ;
  arraySetMax(s, i+1) ;
}



void stringAdjust(Array s1) { 
  /*  
      after having messed around in a string array by interting
      a '\0', the invariant arru(s1, arrayMax(s1)-1, char) == '\0'
      is violated.
      this function cures it.
      input : s1  -- an Array of char
      output: s1  -- up to first '\0'
  */
  int l ;
  if (!s1) die("stringAdjust: null") ;
  if (!arrayMax(s1)) {
    stringClear(s1) ;
    return ;
  }
  l = strlen(string(s1)) ;
  if (l >= arrayMax(s1))
    die("stringAdjust: memory allocation error? actual string length is %d, but array knows only about %d chars", l, arrayMax(s1)-1) ;
  arraySetMax(s1, l+1) ;
}



/**
 * Remove the trailing n chars from s.
 * If n is equal or larger than the length of s, the result is the empty string.
 * @param[in] s A valid Stringa
 * @param[in] n How many chars to chop off, n >= 0
 * @param[out] s Modified
 */
void stringChop(Stringa s, int n) 
{
  int l = arrayMax(s) ;
  if (n >= l) {
    stringClear(s) ;
    return ;
  }
  arru(s, l-n-1, char) = '\0' ;
  arraySetMax(s, l-n) ;
  return ;
}



/**
 * Removes a string segment.
 * Removes the segment [pos..pos+len-1] from s and returns a pointer to a copy of this segment. 
   If pos is beyond the end the string, nothing is done and NULL returned; 
   If pos is within the string but pos+len-1 is beyond the end of the string or len is -1, 
   the tail of string starting at pos is removed.
 * @param[in] s A valid Stringa
 * @param[in] pos Start position of the segment to extract (based on 0), pos >= 0 
 * @param[in] len Number of chars to cut out, -1 means as many as possible; 
   it is ok if len asks for more than is actually present; 
   if len is 0 s is not changed and an empty string returned.
 * @param[out] s Without specified segment
 * @return Pointer to segment cut out this memory stays stable until the next call to this routine. 
   This memory belongs to this routine; it may be read and modified but not realloc'd or free'd by the user of this routine. 
 */
char *stringCut(Array s, int pos, int len) 
{
  static Array r = NULL ;
  int l = arrayMax(s) - 1 ;  /* don't count the terminating '\0' */
  char *from ;
  char *to ;
  char c ;

  if (l < 0 || arru(s, l, char))
    die("stringCut: not a proper string") ;

  if (pos >= l)
    return NULL ;
  if (len < 0 || len > l - pos)
    len = l - pos ;

  stringCreateClear(r, len+1) ;

  if (len > 0) {
    stringNCpy(r, arrp(s,pos,char), len) ;
    to = arrp(s,pos,char) ;
    from = to + len - 1 ;
    while (c = *++from)
      *to++ = c ;
    *to = '\0' ;
    arraySetMax(s, to - string(s) + 1) ;
  }

  return string(r) ;
}



/**
 * Insert string 'i' at position p into array string 's'.
 * @param[in] s Destination
 * @param[in] p 0 .. arrayMax(s)  (=strlen(string(s))) 0 means prepending 'i' to 's', 
   arrayMax(s) means appending (same as stringCat())
 * @param[in] i Source
 * @param[out] s Modified
 */
void stringInsert(Stringa s, int p, char *i) 
{
  int il = strlen(i) ;
  int k = arrayMax(s) - p ; /* number of chars to move */
  int oldlen = arrayMax(s)-1 ;
  int maxnew = oldlen+il ;
  char *to ;
  char *from ;
  if (!il) 
    return ;
  array(s, maxnew, char) = '\0' ;  /* allocate */
  to = arrp(s, maxnew, char) ;            
  from = arrp(s, oldlen, char) ;
  if (k < 0) 
    die("stringInsert: k=%d", k) ;
  while (k--)
    *to-- = *from-- ;               /* shuffle up to make room */
  memcpy(arrp(s,p,char), i, il) ;   /* fill the room */
  /* if (strlen(string(s)) != arrayMax(s)-1) die("oops") ; */
}



/**
 * Appends null-terminated string s2 to s1. 
 * @param[in] s1 Valid Stringa
 * @param[in] s2 Null-terminated string
 * @param[out] s1 With contents of s2 appended
 */
void stringCat(Array s1, char *s2) 
{ 
  int i = arrayMax(s1) - 1 ; /* index of the trailing \0 */
  int l = strlen(s2) ;
  if (arru(s1, i, char)) 
    die("stringCat: s1 is not null-terminated (i=%d)", i) ;
  array(s1, i + l, char) = '\0' ;   /* allocate */
  memcpy(arrp(s1, i, char), s2, l) ;
}  



/**
 * Convert i into a string and append it to string Array s.
 */
void stringCatInt(Array s, int i) {
  char c[30] ;
  hlr_itoa(c, i) ;
  stringCat(s, c) ;
}



/**
 * Append single character 'c' to a Stringa 's'. 
 */
void stringCatChar (Stringa s,char c)
{ 
  arru(s, arrayMax(s)-1, char) = c ;
  array(s, arrayMax(s), char) = '\0';
}



/**
 * Analogous to strncpy() from the C library.
 * Analogous to stringNCat() from this module, except that s1 is cleared before appending.
 */
void stringNCpy(Stringa s1, char *s2, int n) 
{ 
  stringClear(s1) ;
  stringNCat(s1, s2, n) ;
}



/**
 * Appends first n chars from string s2 to s1. Same functionality as strncat().
 * @param[in] s1 Array of char, null-termintated 
 * @param[in] s2 String need not be null-terminated, but may not contain internal nulls
 * @param[in] n Number of chars from s2 to be copied, if n <= 0, 
   s1 is not changed. n larger than the length of s2 is ok, too.
 * @param[out] s1 With first min(n,strlen(s2)) chars from s2 appended
 */
void stringNCat(Stringa s1, char *s2, int n) 
{
  int i ;
  int l = 0 ;         /* length of s2 --> number of chars to copy */
  char *cp = s2 - 1 ;
  if (n <= 0) 
    return ;
  while (*++cp)
    if (++l >= n) break ;
  /* now l holds number of chars to copy */
  i = arrayMax(s1) - 1 ; /* index of the trailing \0 */
  if (arru(s1, i, char)) 
    die("stringNCat: s1 is not null-terminated (i=%d)", i) ;
  array(s1, i + l, char) = '\0' ;     /* allocate and terminate string */
  memcpy(arrp(s1, i, char), s2, l) ;  /* as fast as possible */
  /* this is the same functionality, but slower: 
     strncpy(arrp(s1, i, char), s2, l) ; */
}  



/**
 * Copies null-terminated string s2 to s1. Same functionality as strcpy()
 * @param[in] s1 Array of char, null-termintated 
 * @param[in] s2 String need not be null-terminated, but may not contain internal nulls
 * @param[out] s1 With contents of s2
 */
void stringCpy(Array s1, char *s2) 
{ 
  arraySetMax (s1,0) ;
  array(s1, strlen(s2), char) = '\0' ;   /* allocate */
  strcpy(arrp(s1, 0, char), s2) ;
}  



/**
 * Erases the contents of Array s1, leaving an empty string.
 */
void stringClear(Array s1) 
{ 
  arraySetMax (s1,0) ;
  array(s1, 0, char) = '\0' ;
}



/**
 * Exactly like strTrim(), but with a Stringa.
 * @param[in] s Must exist
 */
int stringTrim(Array s /* of char */, char *left, char *right)
{ 
  int len = strTrim(string(s), left, right) ;
  arraySetMax(s, len+1) ;  
  return len ;
}



static int strTranslate_resultLen = 0 ;  /* set by strTranslate */



/** 
 * Exactly like strTranslate(), but with a Stringa.
 * @param[in] s Must exist
 */
int stringTranslate(Array s, char *fromChars, char *toChars)
{ 
  int cnt = strTranslate(string(s), fromChars, toChars) ;
  arraySetMax(s, strTranslate_resultLen+1) ;  
  return cnt ;
}



static int computeResultLength(char *format, va_list args) 
{ /*
  -- for use in stringPrintf() and stringAppendf() --
  function estimates maximum length of output by looking at
  format string and arguments and extends string before printing
  into it
  */
  int deflength = 18; /* default size for int/float/pointer types */
  int maxlength = 0; /* estimated maximum string length */
  char *cp = format;
  int isPercent = 0; /* flag for inside/outside conversion specification */

  /* parse format string to estimate maximum length */
  while (*cp) {
    /* toggle percent flag to handle escaped '%'s correctly */
    if (*cp == '%')
      isPercent = (isPercent) ? 0 : 1;
    else if (isPercent) {
      /* handle one conversion specification e.g. %20.10s, %5.2f, etc */
      int width = 0;
      int prec = 0;
      int dot = 0; /* flag for dot found */
      int asteriskCnt = 0; /* flag/counter for value substitution by '*' */
      long int intConv = 0;
      char *intConvEnd = NULL;
      char *arg = NULL;
      int arglength = 0;

      for (;;) {
	/* try to read an integer */
	intConv = strtol(cp, &intConvEnd, 10);

	/* if an integer was found */
	if (intConvEnd != cp) {
	  /* step over integer found */
	  cp = intConvEnd;

	  /* assign to width or prec depending on dot-found flag */
	  if (dot)
	    prec = intConv;
	  else
	    width = intConv;
	}
	/* if substitution count asterisks to avoid crash */
	else if (*cp == '*')
	  asteriskCnt ++;

	/* skip potential extensions of conversion specifier */
	if (strchr("lLh", *cp))
	  cp ++;
	  
	if (*cp == 'l')
	  cp ++;
	  
	/* if conversion type specifier found */
	if (strchr("pdinouxXeEfgGcCsS", *cp)) {
	  /* reset flags  and get pointer to current argument */
	  isPercent = 0;
          if (strchr("eEfgG", *cp)) {
            /* on IRIX 6.5 using the O32 ABI there is a bug in
               va_arg(x,t): it always sizeof(t) from the arg list
               instead of the actual argument size; obviously this
               leads to garbarge with 8byte types like float when
               using va_arg(x,char*).
               The problem has been fixed by SGI in the N32 ABI.
               The workaround for O32 is this code:
            */
            va_arg(args, double);
            arg = NULL ;
          }
          else
            arg = va_arg(args, char *);

	  if (asteriskCnt) {
	    if (asteriskCnt == 2) {
	      width = (int) arg;
	      arg = va_arg(args, char *);
	      prec = (int) arg;
	      arg = va_arg(args, char *);
	    }
	    else if (asteriskCnt == 1) {
	      if (dot)
		prec = (int) arg;
	      else
		width = (int) arg;
	      
	      arg = va_arg(args, char *);
	    }
	    else
	      die("stringPrintf(): cannot handle format string %s", format);
	  }

	  if (*cp == 's') {
	    /* estimate max length for strings */
	    if (prec)
	      arglength = prec;
	    else 
	      arglength = arg ? strlen(arg) : 6 /* (null) */ ;

	    maxlength += MAX(arglength, width);
	  }
	  else if (*cp == 'S') {
	    /* do we need that? it's not tested. #include <widec.h>
	       if (prec)
	         arglength = prec * sizeof(wchar_t);
	       else
	         arglength = wssize((wchar_t *) arg) * sizeof(wchar_t);

	       maxlength += MAX(arglength, width * sizeof(wchar_t));
	       */

	    die("stringPrintf()/Appendf(): wide character strings not handled");
	  }
	  else if (*cp == 'c') {
	    maxlength ++;
	  }
	  else if (*cp == 'C') {
	    /* see above
	       maxlength += sizeof(wchar_t);
	       */

	    die("stringPrintf(): wide characters not handled");
	  }
	  else {
	    /* estimate max length for all other types */
	    maxlength += (deflength + MAX(width, prec));
	  }

	  /* end of conversion type specification, exit loop */
	  break;
	}
	else {
	  /* if stopped on a dot set flag */
	  if (*cp == '.')
	    dot = 1;

	  /* if stopped on a dollar complain */
	  if (*cp == '$')
	    warn("stringPrintf(): argument position manipulation not supported");

	  cp ++;
	}
      }
    }

    maxlength ++;
    cp ++;
  }
  return maxlength ;
}



/**
 * Formatted printing into a Stringa (similar to sprintf()).
 * @param[in] str Target string which receives formatted printing as in sprintf(); 
   must not be NULL; must be created using stringCreate()
 * @param[in] format Format string as in sprintf(), only difference: 
   argument position manipulation e.g. "%2$15.12s" or "%*3$s" is not supported
 * @param[in] ... Variable argument list, must match format string, else behaviour is undefined
 * @param[out] str Contains formatted output
 * @return Number of characters printed into str not counting the trailing null-termination character
 * @pre None
 * @post None
 */
int stringPrintf(Stringa str, char *format, ...) 
{ 
  va_list args ;
  int maxlength ;
  int resultLen ;

  va_start(args, format) ;
  maxlength = computeResultLength(format, args) ;
  va_end(args);

  array(str, maxlength + 1, char) ;  /* allocate space */

  va_start(args, format);
  resultLen = vsprintf(string(str), format, args);
  va_end(args);

  if (resultLen > maxlength || resultLen < 0)
    die("stringPrintf(): oops") ;

  arraySetMax(str, resultLen+1) ; 

  return resultLen ;
}



/**
 * Same as stringPrintf(), but the result is appended to 'str'.
 * @return Number of chars appended
 */
int stringAppendf(Stringa str, char *format, ...) 
{ 
  va_list args ;
  int maxlength ;
  int len = stringLen(str) ;
  int resultLen ;

  va_start(args, format) ;
  maxlength = computeResultLength(format, args) ;
  va_end(args);

  array(str, len + maxlength + 1, char) ; /* allocate space */

  va_start(args, format);
  resultLen = vsprintf(string(str)+len, format, args);
  va_end(args);
  if (resultLen > maxlength || resultLen < 0)
    die("stringAppend(): oops") ;

  arraySetMax(str, len+resultLen+1) ;  /* turn into Stringa */
  return resultLen ;
}



/**
 * Like sprintf() from the standard C library, but with unlimited length and own memory management.
 * @param[in] format  Template to be filled
 * @param[in] ... Variable number of arguments, must match 'format'
 * @return Formatted string; memory managed by this routine; may be written to by user, 
   but not free'd or realloced; stable until next call to this routine.
 */
char *stringPrintBuf(char *format, ...) 
{
  static Stringa str = NULL ;
  va_list args ;
  int maxlength ;
  int resultLen ;
  stringCreateOnce(str, 100) ;

  va_start(args, format) ;
  maxlength = computeResultLength(format, args) ;
  va_end(args);

  array(str, maxlength + 1, char) ;  /* allocate space */

  va_start(args, format);
  resultLen = vsprintf(string(str), format, args);
  va_end(args);

  if (resultLen > maxlength || resultLen < 0)
    die("stringPrintBuf(): oops") ;

  arraySetMax(str, resultLen+1) ;  /* turn into Stringa */
  return string(str) ;
}
  


/* ------------- part 2: char str[], '\0'-terminated --------- */ 


/** 
 * Converts string to uppercase. 
 */
void toupperStr(char *s) 
{ 
  register char *cp = s - 1 ;
  while (*++cp) 
    *cp = toupper(*cp) ;
}



/** 
 * Converts string to lowercase. 
 */
void tolowerStr(char *s) 
{
  register char *cp = s - 1 ;
  while (*++cp) 
    *cp = tolower(*cp) ;
}



/**
 * Case-insensitive version of strstr(3C) from the C-libarary.
 * @param[in] s String to be searched in (subject)
 * @param[in] t String to look for in s (query)
 * @return If t is the empty string return s, else if t does not occur in s return NULL, 
   else pointer to first position of t in s
 */
char *strCaseStr (char *s, char *t) 
{ 
  char *p , *r ;
  if (*t == '\0') 
    return s ;
  for ( ; *s != '\0'; s++) {
    for (p = s, r = t; *r != '\0' && tolower(*p) == tolower(*r); p++, r++) ;
    if (r > t && *r == '\0')
      return s ;
  }
  return NULL ;
}


/** 
 * Replace previous contents of s1 with copy of s2. s2 can be NULL. 
   This function is the same as strdup() from the C library, 
   except that it free()s the target string before duplicating.
 * @param[in] s1 Place where a pointer to a string is stored
 * @param[in] s2 Contents of s2 will replace contents of s1
 * @param[out] s2 Pervious contents free()d, new memory allocated
 */
void strReplace(char **s1, char *s2) 
{ 
  if (!s1) 
    die("strReplace: NULL") ;
  hlr_free(*s1) ;
  if (s2)
    *s1 = hlr_strdup(s2) ;
}



/**
 * From a supplied string copy a substring delimited by two supplied characters, excluding these characters.
 * @param[in] string String to copy from
 * @param[in] begin Start copying after the leftmost occurrence of this character in string
 * @param[in] end Stop copying before the leftmost occurrence of this character from occurrence of begin on; 
   may be null-terminated to copy to the end of string
 * @param[in] substr Stringa, must exist
 * @param[out] substr Filled with string extracted; empty string if nothing extracted;
 * @return Position after location of end, NULL if no substring extracted
 */
char *strCopySubstr(char *string, char begin, char end, 
                    Array substr /* of char */) 
{ 
  char *pbegin = NULL ;
  char *pend = NULL ;
  char *nextPos = NULL ;
  stringClear(substr) ;

  if(pbegin = strchr(string, begin)) {
    pbegin ++;

    if(pend = strchr(pbegin, end)) {
      nextPos = pend + 1;
      pend --;
      stringNCpy(substr, pbegin, pend - pbegin + 1) ;
    }
  }
  return nextPos ;
}



/** 
 * Translates each character from 's' which matches one of the characters in 'fromChars' with the corresponding character from 'toChars' or, if this position in 'toChars' is not filled, deletes this character from s, thus shortening 's'.
 * This function resembles the Unix command and the Perl function 'tr'.
  \verbatim
   example: strTranslate("abc", "ac", "b") modifies
            "abc" into "bb" and returns 2
            strTranslate("a|b|c", "|", "|")
            just counts the number of '|' chars
  \endverbatim   
 * @param[in] s
 * @param[in] fromChars
 * @param[in] toChars
 * @param[out] s
 * @return Number of chars translated or modified
 * @post strTranslate_resultLen contains the length of the string after processing; used by stringTranslate() to avoid using strlen()
*/
int strTranslate(char *s, char *fromChars, char *toChars)
{ 
  char *from = s - 1 ;
  char *to = s ;
  char c ;
  int toLen = strlen(toChars) ;
  char *hit ;
  int cnt = 0 ;

  while (c = *++from) {
    if (hit = strchr(fromChars, c)) {
      ++cnt ;
      if (hit - fromChars < toLen) 
        *to++ = toChars[hit - fromChars] ;
    }
    else
      *to++ = c ;
  }
  *to = '\0' ;
  strTranslate_resultLen = to - s ;
  return cnt ;
}



/**
 * Remove leading and trailing characters from s. 
  \verbatim
  example: strTrim("<<=text=>>", "=<", "=")
           returns 7 and leaves output "text=>>" 
 \endverbatim  	   
 * @param[in] s Zero-terminated string of char or NULL (nothing will happen)
 * @param[in] left Set of chars to be removed from left end of s, NULL or empty string to leave beginning of s as is
 * @param[in] right Set of chars to be removed from right end of s, NULL or empty string to leave tail of s as is
 * @param[out] s Changed
 * @return Length of s after trim
 */
int strTrim(char *s, char *left, char *right)
{ 
  /* tried to keep this efficent:
     - don't use strlen() unless unavoidable
     - first remove right side, because then there is less
     to be shifted left
  */ 
  int len ;
  char *cp ;
  if (!s)
    return 0 ;
  len = strlen(s) ;

  if (len && right) {
    /* move to last char of sequence, then run as long as
       there are only chars from 'right' or we bump into the
       beginning of the string */
    cp = s + len - 1 ;  
    while( cp >= s && strchr(right, *cp))
      *cp-- = '\0' ;
    len = cp - s + 1 ;
  }

  if (len && left) {
    /* move cp to the first char not in 'left'; then start
       shuffling chars from this position onward down to the beginning
       of the string */
    cp = s ;
    while (*cp && strchr(left, *cp))
      ++cp ;
    len = len - (cp - s) ;
    while (*cp)
      *s++ = *cp++ ;
    *s = '\0' ;
  }
  return len ;
}



/**
 * Encrypt the input string such that it is unreadable to humans and can easily be strUnscrambled() again.
 * @param[in] s Must not contain 0xFF
 * @param[out] s Scrambled
 */
void strScramble(char *s) 
{ 
  char *cp = s - 1 ;
  while (*++cp) {
    if ((unsigned char)*cp == 255)
      die("clsv_scramble: cannot scramble 0xFF") ;
    *cp = *cp ^ 255 ;
  }
}



/**
 * Antidot for strScramble().
 */
void strUnscramble(char *s) 
{ 
  strScramble(s) ;  /* scramble + scramble = unscramble */
}



/**
 * Check if s is blank.
 * @return True, if string consists of whitespace only or is of length 0, else returns false 
 */
int isBlankStr(char *s) {
  
  char *cp = s-1 ;
  while (*++cp && isspace(*cp)) ;
  return *cp == '\0' ;
}



int isEmptyString(Array s) {
  if (!s || arrayMax(s)<1) 
    die("isEmptyString: NULL or corrupt string") ;
  return !arru(s,0,char) ;
}




/* -------- part 3: text handling, where text is an Array of char[] ----- */ 

/* a Texta is an Array of char*, where the memory referenced by
   the char pointers belongs to the Texta;
#define Texta Array */
/* #define textCreate(initialSize) arrayCreate(initialSize,char*) */



/**
 * Free the strings in 'a' and make the array empty.
 * The strings must have been allocated using hlr_malloc() or hlr_strdup() if you expect hlr_getAllocCnt() to work.
 */
void textClear(Texta a /*of char* */)
{
  int i = arrayMax(a) ;
  while (i--) 
    hlr_free(arru(a, i ,char*)) ;
  arrayClear(a) ;
}




/**
 * Free storage allocated to text a.
 * @param[in] a Array of char*; the char* should have been allocated with hlr_malloc()
 * @note This function is private to this module. Don't call it from your program. Use textDestroy() instead
 */
void textDestroyFunc(Texta a) 
{ 
  if (!a) 
    return ;
  textClear(a) ;
  arrayDestroy(a) ;
}



/**
 * Produce a clone of the Array of strings 'a'.
 * @param[in] a Array of strings, NULL ok
 * @return Array of char* with same contents as 'a', but new memory allocated
 * @note The user of this routine is responsible for freeing the memory allocated
 */
Texta textClone(Array a /* of char* */)
{ 
  int i ;
  Texta b ;
  char *s ;
  if (!a) 
    return NULL ;
  i = arrayMax(a) ;
  b = textCreate(i) ;    /* original size */
  if (i == 0)
    return b ;
  array(b, i-1, char*) = NULL ;  /* access to make arru() work */
  while (i--) {
    s = arru(a,i,char*) ;
    arru(b,i,char*) = (s ? hlr_strdup(s) : NULL) ;
  }
  return b ;
}  



/**
 * Splits string 's' in words separated by one or more characters from string 'sep'.
 * Same result as repeated calls to strtok() from the standard C libary.
 * @param[in] s Input string
 * @param[in] sep Separation character(s)
 * @param[out] s The contents changed!
 * @return An Array of pointers to C-strings. The user of this routine is responsible for freeing the 
   Array returned after use; (e.g. using textDestroy() )
 */
Texta textStrtok(char *s, char *sep)
{ 
  WordIter wi=wordIterCreate(s,sep,1) ;
  char *pos ;
  Texta a=textCreate(10) ;
  while (pos=wordNext(wi))
    array(a,arrayMax(a),char*)=hlr_strdup(pos) ;
  wordIterDestroy(wi) ;
  return a ;
}



/**
 * Same as textStrtok(), but does not alter its input 's'.
 * Suffix 'P' stands for 'Preserving its input'
 */
Texta textStrtokP(char *s, char *sep)
{ 
  char *cp = hlr_strdup(s) ;
  Texta t = textStrtok(cp, sep) ;
  hlr_free(cp) ;
  return t ;
}



/**
 * Splits string 's' in words separated by any of the characters from string 'sep'.
 * @param[in] s Input string
 * @param[in] sep Separation character(s)
 * @param[out] s The contents changed!
 * @return An Array of pointers to C-strings. The user of this routine is responsible for freeing the 
   Array returned after use; (e.g. using textDestroy() )
 */
Texta textFieldtok(char *s, char *sep)
{ 
  WordIter wi=wordIterCreate(s,sep,0) ;
  char *pos ;
  Texta a=textCreate(10) ;
  while (pos=wordNext(wi))
    array(a,arrayMax(a),char*)=hlr_strdup(pos) ;
  wordIterDestroy(wi) ;
  return a ;
}



/**
 * Same as textFieldtok() but does not alter its input 's'.
 * Suffix 'P' stands for 'Preserving its input'
 */
Texta textFieldtokP(char *s, char *sep)
{
  char *cp = hlr_strdup(s) ;
  Texta t = textFieldtok(cp, sep) ;
  hlr_free(cp) ;
  return t ;
}



/**
 * textJoin.
   \verbatim
   example:
     Texta t = textStrtok("a b c", " ") ;
     Stringa s = stringCreate(10) ;
     textJoin(s, "-", t) ;
     puts(string(s)) ;
   prints "a-b-c"
   \endverbatim
 * @param[in] s Contents will be overridden
 * @param[in] sep Separator between elements, e.g. ","
 * @param[in] a Array of char* (can be empty, but not NULL)
 * @param[out] s Filled; empty string if empty Array
 */
void textJoin(Stringa s, char *sep, Array a /*of char* */)
{ 
  int i ;
  stringClear(s) ;
  if (! arrayMax(a))
    return ;
  stringCpy(s, arru(a,0,char*)) ;
  for (i=1; i<arrayMax(a); ++i) {
    stringCat(s, sep) ;
    stringCat(s, arru(a,i,char*)) ;
  }
}



/**
 * Remove duplicate strings from t without changing the order.
   \verbatim
   Note on runtime complexity: 
     Execution time: O(n*n*log n)  (where n is arrayMax(t))

     The implementation below has a lot of room for improvments, e.g.
     - using a hashed lookup table for seen values
       would reduce runtime complexity to O(n)
     - if no hashed lookup table availabe:
       s = arrayCopy(t)        -- O(n)
       arraySort(s)            -- O(n * log n)
       remove duplicates in s  -- O(n)
       f = arrayCopy(s)        -- O(n)
       remove duplicates in t with lookup in s and flagging in f
                               -- O(n * log n) 
       would trade space for time
   \endverbatim
 * @param[in] t
 * @param[in] t Duplicates removed, first occurences kept
 * @note It must be ok to free elements from t
 */
void textUniqKeepOrder(Texta t)
{ 
  Texta b = textCreate(arrayMax(t)) ;  /* index of elements seen */
  int from = -1 ;
  int to = -1 ;
  char *cp ;
  while (++from < arrayMax(t)) {
    cp = arru(t, from, char*) ;
    if (arrayFindInsert(b, &cp, NULL, (ARRAYORDERF) arrayStrcmp)) {
      ++to ;         /* new */
      arru(t, to, char*) = cp ;
    }
    else
      hlr_free(cp) ; /* already present */
  }
  arraySetMax(t, to + 1) ;
  arrayDestroy(b) ;
}


/*
Add a string to the end of a Texta
#define textAdd(t,s) (array((t),arrayMax(t),char*)=hlr_strdup(s))
*/

/*
get a pointer to the i the line in a Texta (i=0..arrayMax-1)
#define textItem(text, i)  arru((text),(i),char*)
*/


/* --------------- part 4: line and word handling -------------- */


#define GETLINE_MIN 80
#define GETLINE_INC 1024


/**
 * Read an arbitrary long line from stream, functionality analog to gets(3S).
   \verbatim
   usage example:
     char *line = NULL ;
     int buflen ;
     while (getLine(stdin, &line, &buflen)) { 
       printf("%s", line) ;
     }
     hlr_free(line) ;
   \endverbatim
 * @param[in] stream
 * @param[in] buffer Pointer to a string
 * @param[in] buflen Current length of buffer, for interal use of getLine()
 * @param[out] buffer Might be re-alloated
 * @param[out] buflen Adjusted
 * @return Number of chars put into buffer, including trailing \n (except for the last line), 0 if EOF encountered
 * @post Next call to getLine gives next line
 */
int getLine(FILE *stream, char **buffer, int *buflen) 
{ 
  int buffree ;
  char *startp ;
  char *bufp ;
  int c ;

  if (!buffer) die("getLine() without buffer");
  if (! *buffer) {
    *buflen = GETLINE_MIN ;
    *buffer = hlr_malloc(*buflen) ;
  }
  buffree = (*buflen) -1 ;
  bufp = *buffer ;
  startp = bufp ;

  while ((c = getc(stream)) != EOF) {

    if (! --buffree) {
      *buflen = *buflen + GETLINE_INC ;
      buffree += GETLINE_INC ;
      *buffer = realloc(*buffer, *buflen) ;
      if (! *buffer) die("getLine: realloc") ;
      bufp = *buffer + (bufp - startp) ;  /* adjust to new location */
      startp = *buffer ;
    }

    if (!c) warn("getLine: read a NULL character") ; /* warn on binary data */
    *(bufp++) = c ;          /* append char to buffer */
    if (c == '\n') break ;
  }

  *bufp = '\0' ;
  return bufp - startp ;
}




/**
 * Strip the trailing \n character off line, if present and return the length of the resulting line.
 * This function is typically used in conjunction with getLine().
 * @param[in] line
 * @param[out] line
 * @return Length of output line
 */
int stripNlGetLength(char *line) 
{ 
  /*
  implementation note: since strlen() is a slow function if
    lines are long, I try to use it only when unavoidable.
    Since stripNlGetLength() has to find the end of the string
    I pass its finding back, so programs using this function
    would not need to call strlen() again.
  */
  int len = strlen(line) ;
  if (!len)
    return 0 ;
  --len ;
  if (line[len] == '\n') 
    line[len] = '\0' ;
  else
    ++len ;
  return len ;
}




/* ------------------ module word parser 2 ---------- begin 
analogous to strtok() from the standard C library,
but a bit more flexible and allows multiple independent
interleaved scans in different strings.

usage:
  WordIter wi = wordTokIterCreate("hallo welt", " \n\t") ;
  char *w ;
  while (w = wordNext(wi)) 
    printf("w='%s'\n", w) ;
  wordIterDestroy(wi) ;

#define wordTokIterCreate(s,seps) wordIterCreate(s,seps,1)
#define wordFldIterCreate(s,seps) wordIterCreate(s,seps,0)
#define wordIterDestroy(this) (wordIterDestroy_func(this), this=0)
*/


/**
 * wordIterDestroy_func.
 * @param[in] this1 Created by wordIterCreate*
 * @post 'this1' is no more accessible
 * @note Do not call in your programs, use wordIterDestroy() instead.
 */
void wordIterDestroy_func(WordIter this1) 
{ 
  if (!this1) 
    die("wordIterDestroy: null") ;
  hlr_free(this1) ;
}


/**
 * Create an iterator returning the words of 's', broken at any char from 'seps'. 
   If 'manySepsAreOne' is 1, consequtive stretches of separators are treated as one separator (so there are no empty words).
 * @param[in] s String to break
 * @param[in] seps Set of word separator chars
 * @param[in] manySepsAreOne 1 or 0
 * @param[out] s Is destroyed
 * @returns WordIter object for use in wordNext() and wordIterDestroy()
 * @note sep must be kept stable during the whole scan
 * @note if 's' is the empty string, then in mode manySepsAreOne==1, wordNext() will immediately return NULL, 
   in mode manySepsAreOne==0, wordNext() will return one empty string, then NULL
 */
WordIter wordIterCreate(char *s, char *seps, int manySepsAreOne) 
{ 
  WordIter this1 ;
  if (!s || !seps || !*seps)
    die("wordIterCreate: some null/empty input") ;
  this1 = (WordIter) hlr_malloc(sizeof(struct wordIterStruct)) ;
  this1->seps = seps ;
  this1->cp = s ;
  this1->manySepsAreOne = manySepsAreOne ;
  this1->atEnd = 0 ;
  return this1 ;
}


/*
from format.h
#define wordNext(this1) (wordNextG(this1,NULL))
*/



/**
 * wordNextG.
 * @param[in] this1 Created by wordTokIterCreate() or wordFldIterCreate()
 * @param[in] lenP Valid place to put an int; NULL if no interest
 * @param[out] lenP If lenP not NULL: strlen() of result, computed efficiently
 * @return  NULL if no more word, else pointer to beginning of null-terminated word
 */
char *wordNextG(WordIter this1, int *lenP) 
{ 
  char *cp ;
  char *word ;
  if (!this1) 
    die("wordNext: null") ;
  if (lenP)
    *lenP = 0 ;
  if (this1->atEnd) 
    return NULL ;
  cp = this1->cp ;
  if (this1->manySepsAreOne) { /* skip to first non-sep */
    --cp ;
    while (*++cp && strchr(this1->seps, *cp)) ;
  }
  else {
    if (! *cp) {
      this1->atEnd = 1 ;
      return cp ;
    }
    if (strchr(this1->seps, *cp)) {
      ++this1->cp ;
      *cp = '\0' ;
      return cp ;
    }
  }
  if (! *cp) {
    this1->atEnd = 1 ;
    return NULL ;
  }

  /* here holds: we are on the beginning of a word, on a non-separator char */
  /* now run until end of this word */
  word = cp ;
  --cp ;
  while (*++cp && !strchr(this1->seps, *cp)) ;
  if (lenP)
    *lenP = cp - word ;

  if (! *cp ) 
    this1->atEnd = 1 ;
  else {
    *cp = '\0' ;        /* mark end of word */
    this1->cp = cp + 1 ; /* next time we start after the end of this separator */
  }
  return word ;
}

/* does s1 start with s2?
   strStartsWith() always works, but is slower than strStartsWithC()
                   s2 must not be an expression with side effects
                   since it is evaluated twice
   strStartsWithC() same as strStartsWith, but can only be used 
                    if s2 is a string constant, e.g.
                    if (strStartsWithC(s, "CC   ")) ...
#define strStartsWith(s1,s2) (strncmp((s1),(s2),strlen(s2)) == 0)
#define strStartsWithC(s1,s2) (strncmp((s1),(s2),sizeof(s2)-1) == 0)
*/


/**
 * Checks if string 's' end with 'suffix'.
 * @param[in] s Not NULL
 * @param[in] suffix  Not NULL; if empty, 1 will be returned
 * @return 1 if yes, 0 if no
 */
int strEndsWith(char *s, char *suffix)
{ 
  int len = strlen(s) ;
  int slen = strlen(suffix) ;
  if (slen > len)
    return 0 ;
  return strEqual(s + len - slen, suffix) ;
}

