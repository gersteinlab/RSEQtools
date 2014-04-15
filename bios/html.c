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
 *   \file html.c Parse HTML CGI POST data and other CGI routines
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "log.h"
#include "hlrmisc.h"
#include "format.h"
#include "linestream.h"

#include "html.h"

static char *gCgiBuffer = NULL;
static int gCgiBufferSize;


static int hexdigit (register int c) 
{ /*
  Convert a hex digit (character) to a numeric value.
  The character must be a valid hex digit.  The result is undefined
  if it is not.  Digits > 9 can be either upper or lower case.
  */
  c &= 0x7f;
  if (c >= 'a' && c <= 'f')
    return (c - 'a' + 10);
  if (c >= 'A' && c <= 'F')
    return (c - 'A' + 10);
  return (c - '0');
}



static char x2c (char *const x) 
{ /*
  Convert a 2 digit hex string to a char.
  For example, x2c ("3f") ==> '?'.  No error checking is done.
  */
  return (hexdigit (x[0]) * 16) + hexdigit (x[1]) ;
}



/**
 * URL-decodes a string.
 * @param[in] a Array of char, null-terminated
 * @param[out] a Modified. Encoded hex characters are turned into their ascii equivalents, and +'s are turned back into spaces.
 */
void cgiDecodeWord (Stringa a) 
{ 
  char *cp = string(a) - 1 ;	 /* one less for start of while loop */
  char c;
  char *c2 = string(a);
  
  while (c = *++cp) {
    switch (c) {

    case '%':
      *c2++ = x2c(++cp);
      ++cp;	  /* skip second hex digit */
      if (!*cp)
        die ("cgiDecodeWord: % in post not followed by two characters.");
      break;
      
    case '+':
      *c2++ = ' ';
      break;
      
    default:
      *c2++ = c;
    }
  }
  
  *c2 = '\0';
  stringAdjust(a) ;
}



static char *g_domain = NULL ;


/**
 * Set the domain to be used in the next cookie to be constructed.
 * @param[in] domain Domain name, or NULL to quit setting a domain e.g. ".roche.com"
 * @post cgiConstructCookie() will add the domain spec
 */
void cgiDomainSet(char *domain) 
{ 
  strReplace(&g_domain, domain) ;
}



/**
 * Construct a string for a transient or permanent cookie.
 * The returned string can be used in cgiHeaderCookie().
 * @param[in] name Name of the cookie for use in cgiReadCookie
 * @param[in] value Value to be stored 
 * @param[in] lifelength  0 : transient cookie (bound to browser process), >0 : go to disk and stay there for 'lifelength' seconds, 
   <0 : delete this cookie 
 * @return Pointer to constructed text; this memory may be modified but not free'd nor realloc'ed by the caller of this routine. 
   Stable until next call to this routine
 */
char *cgiConstructCookie(char *name, char *value, int lifelength) 
{  
  /*
  18-May-1998> dmd: drop quotes and use cgiEncodeWord
  */
  static Stringa s = NULL ;
  static Stringa e = NULL ;
  stringCreateOnce(s, 40) ;
  stringCreateOnce(e, 40) ;

  cgiEncodeWord (value,e) ;

  stringCpy(s, name) ;
  stringCat(s, "=") ;
  stringCat(s, string(e)) ;
  if (lifelength != 0) {
    /* date format according to http://www.faqs.org/rfcs/rfc822.html, section 5.1 */
    char niceDate[40] ;
    time_t t;
    struct tm *tmp ;
    t = time(NULL) + lifelength ;
    tmp = localtime(&t) ;
    strftime(niceDate, sizeof(niceDate), "%a, %d-%b-%y 00:00:00 GMT", tmp) ;
    stringCat(s, "; expires=") ;
    stringCat(s, niceDate) ;
  }
  stringCat(s, "; path=/;") ;
  if (g_domain && *g_domain) {
    stringCat(s, " domain=") ;
    stringCat(s, g_domain) ;
    stringCat(s, ";") ;
  }    
  return string(s) ;
}



/**
 * If the current process is a CGI process and received a cookie from the client, 
   the cookie is examined for a parameter called 'name', whose value is extracted and returned.
 * @param[in] name Name of the cookie whose value is the be returned
 * @return Pointer to value of cookie named 'name' if available; else 0 
 * @note This memory may be modified but not free'd nor realloc'ed by the caller of this routine it is 
   kept stable until the next call to cgiReadCookie()
 */
char *cgiReadCookie(char *name) {  
  /*
  1998-05-13 dorand: extend to handle unquoted values
  1998-05-19 dorand: return last name=value 
  */
  /*

----------------------------------------------------------------------------
Cookies:

NAME=VALUE
     This string is a sequence of characters excluding semi-colon, comma 
     and white space. If there is a need to place such data in the name 
     or value, some encoding method such as URL style %XX encoding is 
     recommended, though no encoding is defined or required. 

     However, experience has shown that quoted strings with 
     embedded blanks work (see rfc 2109)!

----------------------------------------------------------------------------

 implementation notes

  Support syntax above and if VALUE is quote delimited, unquote it.

  sample value of HTTP_COOKIE:
    Detlefs_cookie_name="apple tree"; zwei=zwei; zwei="drei"
  then
  name                -> value
  -------------------    ----------
  Detlefs_cookie_name -> apple tree
  zwei                -> drei
  drei                -> null

  algorithm should be:

  look for 'name=', 
  if value starts with '"' 
  then return until terminating '"'
  else return until eos
  */

  char *cp ;
  char *cp1 ;
  static Array cookie = 0 ;
  static Array value  = 0 ;

  if (! getenv("HTTP_COOKIE"))              
    return 0 ;                                 /* no cookies to examine */

  if (!cookie)
    cookie = stringCreate(100) ;

  stringCpy(cookie, getenv("HTTP_COOKIE")) ;   /* don't alter original: */
                                               /* allows multiple entry */
  if (!value)
    value = stringCreate(40) ;

  stringCpy(value, name) ;
  stringCat(value, "=") ;

  cp   = NULL;
  cp1  = string(cookie) ;          
           
  while (cp1 = strstr(cp1, string(value))) {  /* find start of name= */
    cp1 = cp1+strlen(name)+1 ;                /* skip name= */
    cp  = cp1 ;			              /* save start */
  }

  if (!cp) 
     return 0 ;

  if (*cp == '"') {                          
    stringCpy(value, cp+1) ;                   /* this value is quoted */

    if ((cp = strchr(string(value), '"')))     /* terminating '"' found */
      *cp = '\0' ;                             /* remove it */
    else 
      die("Cannot parse cookie ]]%s[[. Trailing \" missing.", string(cookie)) ;
  }
  else {     
    stringCpy(value, cp) ;                     /* this value is not quoted */

    if ((cp = strchr(string(value), ';')))     /* terminating ';' found */
      *cp = '\0' ;                             /* remove it */
  }   

  stringAdjust(value) ;
  cgiDecodeWord(value) ;
  return string(value) ;
}



static FILE *gHeaderDestination = NULL ;    



static void cgiLogImpl(char *format, va_list args) { /*
  the routine ensures that the HTTP protocol header is completed
  before writing any error message using functions die(), warn(),
  usage(), or romsg() from module log.
  usage: like printf()
  */
  gHeaderDestination = stderr ;
  cgiHeader(NULL) ;
}
  


/**
 * Initialize the CGI module.
 * Within a CGI program, this function should be called as early as possible.
 * cgiInit ensures that error messages generated by die() and warn() will output a cgi header, if this has not yet happened. 
   This makes sure that die/warn in lower level routines (e.g. from this library) will not result in a "server error" 
   message displayed in the user's browser.
 */
void cgiInit(void) 
{ 
  /*
  aklenk 2001:
  a tentative solution to the problem, that the apache web server
  writes output to stderr to the error log file immediately instead
  of the user's browser - this results in server errors and/or empty
  documents and invisible error messages
  */

  char *server = getenv("SERVER_SOFTWARE");

  if (server && strNCaseEqual(server, "apache", 6)) {
    int fd;

    if (fflush(stderr)) 
      perror("PROBLEM: cgiInit: fflush") ;
    if (PLABLA_CLOSE(2) == -1) 
      perror("PROBLEM: cgiInit: close(2)") ;
    fd = dup(1); 
    if (fd == -1) {
      printf("PROBLEM: could not duplicate stdout\n") ;
      perror("PROBLEM: cgiInit: dup") ;
    }
    if (fd != 2) 
      printf("PROBLEM: cgiInit(): could not redirect stderr (fd %d) - possibly cgiInit() several calls to cgiInit().\n", fd) ;
    setbuf(stderr, NULL) ;
  }
  gHeaderDestination = stdout;
  log_registerDie(&cgiLogImpl) ;
  log_registerWarn(&cgiLogImpl) ;
  log_registerUsage(&cgiLogImpl) ;
  log_registerRomsg(&cgiLogImpl) ;
}



/**
 * Enables access to data generated by method GET via cgiGetNextPair(). 
   The data generated by method POST is not accessible after this call.
 * @return Pointer to value of 'QUERY_STRING' or NULL
 * @pre None
 * @post If 'QUERY_STRING' is defined, cgiGetNextPair() can be called on name/value pairs of 'QUERY_STRING' (method GET), 
   but not on name/value pairs of stdin (method POST) anymore until you call cgiGet2PostReset() afterwards.
 */
char *cgiGet2Post(void)
{ 
  /*
  implementation: content of 'QUERY_STRING' is copied to gCgiBuffer,
    the allocated memory is not freed until the program exits.
  */
  char *val = getenv("QUERY_STRING");
  if (val&&strlen(val)) {
    gCgiBufferSize=strlen(val);
    hlr_free(gCgiBuffer);
    gCgiBuffer = hlr_strdup(val);
  }
  return val;
}



/**
 * Allows to use the function cgiGetNextPair() after cgiGet2Post() was called, by resetting the CGI buffer. 
 * Call this function directly after using cgiGet2Post(). 
 */
void cgiGet2PostReset(void)    
{ 
  hlr_free(gCgiBuffer);
}



/**
 * Provides the next item/value pair.
  \verbatim
  usage:
    int first = 1 ;
    Array item  = arrayCreate(10, char) ;
    Array value = arrayCreate(10, char) ;
    while (cgiGetNextPair(&first,item,value)) { ... }
  \endverbatim 
 * This function is the basis for all other calls here to read from a form (cgiGetNext/cgiGetInit, cgiGetF, cgiGetFo, cgiGetByName)
 * This function is necessary, all the others are convenience 
 */
int cgiGetNextPair (int *first,Array item,Array value)
{ 
  /*
  1998-05-26> dmd: make re-entry possible
  */

  static char *bufferPtr;
  int mode;
  int c;

  if (gCgiBuffer == NULL) {
    char *contentLength = getenv ("CONTENT_LENGTH");
    gCgiBufferSize = contentLength ? atoi (contentLength) : 0;
    gCgiBuffer = hlr_malloc (gCgiBufferSize);
    fread (gCgiBuffer,gCgiBufferSize,1,stdin);
    bufferPtr = gCgiBuffer;
  }

  if (*first) {
    *first = 0;
    bufferPtr = gCgiBuffer;
  }

  if (item == 0 || value == 0) 
    die ("cgiGetNextPair: Array for item or value not initialized");
  arrayClear (item);
  arrayClear (value);
  mode = 'i';
  while (bufferPtr - gCgiBuffer < gCgiBufferSize) { /* 2000-03-07  klenka: final (?) bugfix */
    c = *(bufferPtr++);
    if (c == '+') c = ' ';
    if (c == '=') {
      array (item,arrayMax (item),char) = '\0';
      cgiDecodeWord (item) ;  /* bug fix: 1998-07-13  wolfd */
      mode = 'v';

      /* 1999-09-08  klenka: bug fix: check gCgiBuffer size even in case c == '=' */
      if (bufferPtr - gCgiBuffer == gCgiBufferSize) {
        array (value,arrayMax (value),char) = '\0';
        return 1;
      }
    }
    else if (c == '&' || bufferPtr - gCgiBuffer == gCgiBufferSize) {
      if (mode == 'i') {
        /* free (gCgiBuffer); */   /* dmd */
        return 0;
      }
      else {
        if (bufferPtr - gCgiBuffer == gCgiBufferSize)
          array (value,arrayMax (value),char) = c;
        array (value,arrayMax (value),char) = '\0';
	cgiDecodeWord (value);
        return 1;
      }
    }
    else {
      if (mode == 'i')
        array (item,arrayMax (item),char) = c;
      else if (mode == 'v' &&  c != '\r')
        array (value,arrayMax (value),char) = c;
    }
  }

  return 0;
}

static int gFirst = -1 ;
static Stringa gItem = NULL ;



/**
 * @pre Running in a CGI
 * @post cgiGetNext() can be called and will return the first field of the POSTed data, if any
*/
void cgiGetInit(void)
{ 
  gFirst = 1 ;
  stringCreateOnce(gItem, 20) ;
}



/**
 * Get the name and value of the next form field
 * @param[in] value Existing Stringa
 * @return Name of field, NULL if no more field;
 * @note The memory returned is managed by this routine; it may be written to, but not free'd or realloc'd by the user; 
   it stays stable until the next call to this routine.
 * @param[out] value Filled with contents of field; contents undefined if no more field
 * @pre successful cgiGetInit() or cgiGetNext()
 * @post Next call to cgiGetNext() returns next form field
 * @note cgiGetInit/cgiGetNext do not add functionality to the basic routine cgiGetNextPair() but are usually more convenient to use.
 */
char *cgiGetNext(Stringa value)
{ 
  if (gFirst == -1)
    die("cgiGetNext() without cgiGetInit()") ;
  if (cgiGetNextPair(&gFirst, gItem, value))
    return string(gItem) ;
  gFirst = -1 ;
  return NULL ;
}



/** 
 * Get and return value of name from POST 
 * @param[in] name Name of item posted
 * @return NULL if no item named 'name' found, else pointer to value associated with 'name'
 * @note The memory returend is managed by this routine. The user may change it, but not free or realloc it. 
   The value returned is stable until the next call to this routine  
 */
char *cgiGetByName(char *name)
{ 
  int first = 1 ;
  static Array item  = NULL ;
  static Array value = NULL ;

  if (!value) { 
    value = arrayCreate(10, char) ;
    item  = arrayCreate(10, char) ;
  }

  while (cgiGetNextPair (&first,item,value)) 
    if (strEqual(name, string(item)))
      return string(value) ;
  return NULL ;
}



/**
 * Like cgiGetByName(), but die()s if field not found (M = mandatory).
 */
char *cgiGetByNameM(char *name)
{ 
  char *cp = cgiGetByName(name) ;
  if (!cp)
    die("cgiGetByNameM: name %s not found", name) ;
  return cp ;
}



/**
 * Optionally get next field from form. Abort if field is not named according to 'fieldName'.
 * @param[in] value Array of char, null-terminated, must exist
 * @param[in] fieldName Name of expected field
 * @param[out] value Contains field contents, if 1 returned, else contains an empty string
 * @return 1 if field read, 0 if there was no field to read
 * @note Usage: implicitly calls cgiGetNextPair() -- beware when using cgiGetFo() and cgiGetNextPair() in the same process.
 * @note Can be used only for one form within one process; there is no way to reset and read the next form.
 */
int cgiGetFo(char *fieldName, Stringa value) 
{
  static int first = 1 ;
  static Array item = NULL ;
  stringCreateOnce(item, 10) ;
  if (cgiGetNextPair(&first, item, value)) {
    if (strDiffer(string(item), fieldName)) 
      die("cgiGetFo: expected to read field %s from form, but found %s.",
	       fieldName, string(item)) ;
    return 1 ;
  }
  stringClear(value) ;
  return 0 ;
}



/**
 * Like cgiGetFo, but field's existence is mandatory.
 */
void cgiGetF(char *fieldName, Stringa value) 
{
  if (!	cgiGetFo(fieldName, value))
    die("cgiGetF: expected field %s not found", fieldName) ;
}



/**
 * Encode string for use in a URL.
 * @param[in] s Null-terminated string
 * @param[in] a Array of char (must exist)
 * @param[out] a With URL-conform translation of s
 */
void cgiEncodeWord (char *s, Stringa a) 
{ 
  char *cp = s - 1 ;
  unsigned char c ;
  char hex[3] ;

  if (!a) 
    die ("cgiEncode") ;
  arrayClear(a) ;
  while (c = *++cp) {
    if ( !isalnum(c) && c != '_' && c != '-' && c != '.' && c != ':') {
      sprintf(hex, "%02X", c) ;
      array(a, arrayMax(a), char) = '%' ;
      array(a, arrayMax(a), char) = hex[0] ;
      array(a, arrayMax(a), char) = hex[1] ;
    }
    else
      array(a, arrayMax(a), char) = c ;
  }
  array(a, arrayMax(a), char) = '\0' ;
}




/* ---------- cgiURL: mini-module for constructing parameterized URLs ---- */

static Stringa cgiurl = NULL ;
static int cgiHasParams ;
static Stringa cgiword = NULL ;



/**
 * @param[in] host e.g. bioinfo.bas.roche.com
 * @param[in] port e.g. 8080, 0 means no port
 * @param[in] program e.g. /htbin/fetch_noform.cgi
 * @pre none
 * @post cgiURLAdd(), cgiURLAddNV() cgiURLAddInt and cgiURLGet() can be called
 * @note Typycial sequence of calls: cgiURLCreate() -- cgiURLAdd() -- cgiURLAddInt() -- cgiURLGet() -- cgiURLCreate() ...
 */
void cgiURLCreate (char *host,int port,char *program) 
{ 
  char portStr[20];
  stringCreateOnce(cgiurl, 50) ;
  stringCreateOnce(cgiword, 50) ;
  if (!host)
    die("cgiURLCreate: NULL host") ;
  stringCpy (cgiurl,"http://");
  stringCat (cgiurl,host);
  if (port) {
    stringCat (cgiurl,":");
    sprintf (portStr,"%d", port);
    stringCat (cgiurl,portStr);
  }
  if (program[0] != '/')
    stringCat (cgiurl,"/");
  stringCat (cgiurl, program);
  cgiHasParams = 0 ;
}



 /**
  * @param[in] cgiServerUrl e.g. "http://bioinfo:8080/htbin"
  * @param[in] program e.g. fetch_noform or NULL (if NULL, then only cgiServerUrl is used)
  * @pre none
  * @post cgiURLAdd() and cgiURLGet() can be called
  * @note Typycial sequence of calls: cgiURLCreate2() -- cgiURLAdd() -- cgiURLAdd() -- cgiURLGet() -- cgiURLCreate2() ...
  * @note This is an alternative to cgiURLCreate, in case you already have an URL stub.
  */
void cgiURLCreate2(char *cgiServerUrl, char *program) 
{
 
  stringCreateOnce(cgiurl, 50) ;
  stringCreateOnce(cgiword, 50) ;
  stringCpy(cgiurl, cgiServerUrl) ;
  if (program) {
    stringCat(cgiurl, "/") ;
    stringCat(cgiurl, program) ;
  }
  cgiHasParams = 0 ;
}



/**
 * Add parameter to URL currently under construction.
 * @param[in] param  Null-terminated string
 * @pre Last call was cgiURLCreate(), cgiURLCreate(), cgiURLAdd(), or cgiURLAddInt()
 * @post cgiURLGet() will return URL with param included
*/
void cgiURLAdd(char *param) 
{
  if (!cgiurl) 
    die ("cgiURLAdd without cgiURLCreate") ;
  stringCat (cgiurl,cgiHasParams ? "+" : "?") ;
  cgiHasParams = 1 ;
  cgiEncodeWord (param, cgiword) ;
  stringCat (cgiurl,string (cgiword)) ;
}



/**
 * Add name-value pair to url currently under construction.
 * @param[in] name Null-terminated string
 * @param[in] value Null-terminated string
 * @pre Last call was cgiURLCreate(), cgiURLCreate2(), or cgiURLAddNV()                  
 * @post cgiURLGet() will return URL with param included
 */
void cgiURLAddNV(char *name,char *value) {
  
  if (!cgiurl) 
    die ("cgiURLAddNV without cgiURLCreate") ;
  stringCat (cgiurl,cgiHasParams ? "&" : "?") ;
  cgiHasParams = 1 ;
  cgiEncodeWord (name, cgiword) ;
  stringCat (cgiurl,string (cgiword)) ;
  if (value){
    stringCat (cgiurl,"=");
    cgiEncodeWord (value, cgiword) ;
    stringCat (cgiurl,string (cgiword)) ;
  }
}



/**
 * Add integer parameter to url currently under construction.
 * @param[in] param  Number to be added
 * @pre Last call was cgiURLCreate(), cgiURLCreate2(), cgiURLAdd(), or cgiURLAddInt()
 * @post cgiURLGet() will return URL with param included
 */
void cgiURLAddInt(int param) 
{
  char s[21] ;
  hlr_itoa(s, param) ;
  cgiURLAdd(s) ;
}



/**
 * @return URL constructed. The memory is managed by this routine. 
   It may by written to, but not free'd or realloc'd by the caller of this this routine. 
   It contents stay stable until the next call to cgiURLCreate() or cgiURLCreate2()
 * @pre Last call was cgiURLCreate(), cgiURLCreate2(), cgiURLAdd(), cgiURLAddNV(), or cgiURLAddInt(). 
   For calling sequence see cgiURLCreate())
 * @post Next allowed functions are cgiURLCreate(), cgiURLCreate2() or repeated calls of cgiURLGet()
 */
char *cgiURLGet(void) 
{ 
  return string(cgiurl);
}



/* ---------- end of mini-module cgiURL ----------------------- */



/**
 * @return 1, if process is a CGI program called by the HTTP deamon, 0 otherwise
 */
int cgiIsCGI(void) 
{
  return ((getenv("HTTP_HOST") != NULL) || (getenv("HTTP_USER_AGENT") != NULL)) ;
}


static int gHeaderPrinted = 0 ;
static int gHeaderExpires = -1 ;  /* # of seceond the page is valid; -1 = eternity */
static Stringa gHeaderRedirUrl = NULL ;   /* "Location:" for HTTP redirection */
static Stringa gHeaderCharSet = NULL ;   /* character set specification  */




/**
 * Sets the lifetime of the next html page to be constructed.
 * @param[in] seconds_valid  If a WWW browser directed back to a page visited before it has the choice to 
   load the page from its cache or retrieve it from the HTTP server. 'seconds_valid' decides that choice: 
   if the more than 'seconds_valid' seconds have passed, the browser is forced to reload the page; 
   else it can get if from its cache. 0 for 'seconds_valid' is OK.
 * @pre cgiHeader() has not yet been called
 * @post cgiHeader()  will set an expires-flag
 * @note Uses a standard HTTP 1.0 protocol feature that should work everywhere
 */
void cgiExpiresSet(int seconds_valid) 
{ 
  if (seconds_valid < -1)
    die("cgiExpiresSet: %d", seconds_valid) ;
  if (gHeaderPrinted)
    die("cgiExpiresSet: called after cgiHeader()") ;
  gHeaderExpires = seconds_valid ;
}



/**
 * Instruct the next cgiHeader call to immediately redirect the browser to address 'url'. 
   This is usefull for setting a cookie and redirecting at the same time.
 */
void cgiRedirSet(char *url)
{ 
  stringCreateOnce(gHeaderRedirUrl, 50) ;
  stringCpy(gHeaderRedirUrl, url) ;
}



/**
 * Sets the character set specification which is written into the cgi header.
 * @param[in] charset Character set, e.g. "iso-8859-1", "utf-8", ... If NULL, 
   no character set specification is written into the cgi header.
 * @pre cgiHeader() has not yet been called
 * @post cgiHeader() will set the character set specification
 */
void cgiEncodingSet(char *charset)
{ 
  stringCreateOnce(gHeaderCharSet, 15) ;
  if (charset && *charset)
    stringCpy(gHeaderCharSet,charset) ;
  else
    stringDestroy(gHeaderCharSet) ;
}



/**
 * Print html header of type, but only if running under WWW. Print only once during a process. 
   The behaviour of this function can be customized by cgiRedirSet(), cgiEncodingSet() and cgiExpiresSet()
 * @param[in] mimeType If NULL, text/html is assumed, if empty string, do not print header and prevent die, 
   warn etc. to print header (useful if header has been printed outside of this process)
 * @pre gHeaderDestination is set (where to write the header)
*/
void cgiHeader(char *mimeType) {
  char exp[70] ;
  static Stringa charset = NULL;
  stringCreateOnce(charset,25);

  if (!cgiIsCGI()) 
    return ;
  if (mimeType && mimeType[0] == '\0')
    gHeaderPrinted = 1 ;
  if (gHeaderPrinted) 
    return ;
  gHeaderPrinted = 1 ;
  if (gHeaderExpires > -1) {
    char date[40] ;
    time_t t = time(0) + ((gHeaderExpires == 0) ? -36000 : gHeaderExpires) ;
    /* 0 means immediate refresh: since the clocks of the server running
       this cgi and the clock on the computer where the brower is sitting
       are usually off by a few minutes, i go back a while in time to force
       a reload */
    struct tm *gmt = gmtime(&t);
    strftime(date, 39, "%a, %d %b %Y %T GMT", gmt) ;
    sprintf(exp, "\r\nExpires: %.40s", date) ;
  }
  else
    exp[0] = '\0' ;

  if (gHeaderCharSet)
    stringPrintf(charset,"; charset=%s",string(gHeaderCharSet));
  else
    stringClear(charset);

  if (!gHeaderDestination)
    gHeaderDestination = stderr;
  fprintf(gHeaderDestination, "Content-Type: %s%s%s\r\n", 
    mimeType ? mimeType : "text/html", string(charset), exp) ; 
  if (gHeaderRedirUrl)
    fprintf(gHeaderDestination, "Location: %s\r\n", string(gHeaderRedirUrl)) ;
  fputs("\r\n", gHeaderDestination) ;
  
  fflush(gHeaderDestination) ;  /* to be sure this comes before any error msg */
}



/**
 * Like cgiHeader, but include a cookie for transmission to the client.
 * @pre cgiHeader() has not yet been called. If it was called before, cgiHeaderCookie() has no effect
 */
void cgiHeaderCookie(char *mimeType, char *cookieSpec) 
{
  Array s = stringCreate(40) ;
  stringCpy(s, mimeType ? mimeType : "text/html") ;
  stringCat(s, "\r\nSet-Cookie: ") ;
  stringCat(s, cookieSpec) ;
  cgiHeader(string(s)) ;  /* treat like an extended mime type */
  arrayDestroy(s) ;
}



/**
 * @return 1 if cgiHeader is printed, 0 otherwise
 */
int cgiHeaderIsPrinted(void) 
{
  return gHeaderPrinted ;
}




/* ---------------------------- html_ functions ------------ */
/* the following functions are targeted to construct HTML text
   e.g. for hyperlinks
   in future I imagine here routines for building HTML tables.
   i think this is another class of functions than the cgi* stuff,
   so i give it a new prefix: html_
1998-06-17  wolfd
*/

static char *html_host = NULL ;
static char *html_program = NULL ;
static char *html_option = NULL ;
static int html_port = 0 ;  /* 0  = default HTTP port (80),
                               -1 = port and cgi path contained in html_host
                               >0 = use this port in constructing the URL
                            */


/**
 * Fix URL parts for subsequent calls to html_hlink() and html_clink().
 * Inputs:  like cgiURLCreate()
   \verbatim
   Typical usage:
   html_URLSet("bioinfo.bas.roche.com", 8080, "/htbin/proteomiscgidev") ;
   printf("<FORM ACTION=%s METHOD=POST>\n", html_clink("login", "do")) ;
   html_hlink("menu", "display", "Goto Menu") ;
   \endverbatim
 */
void html_URLSet(char *host, int port, char *program)
{
  if (port < 0)
    die("html_URLSet") ;
  strReplace(&html_host, host) ;
  html_port = port ;
  strReplace(&html_program, program) ;
  hlr_free(html_option) ;
}



/**
 * Same function as html_URLSet(), but different interface.
 * Inputs: like cgiURLCreate2()
 */
void html_URLSet2(char *cgiServerUrl, char *program)
{ 
  html_port = -1 ; /* indicate mode */
  strReplace(&html_host, cgiServerUrl) ;
  strReplace(&html_program, program) ;
  hlr_free(html_option) ;
}



/**
 * Sets optional argument to be added to the URL.
 * @param[in] option Can also be NULL, can be changed after this function has been called without affecting the postcondition
 * @pre html_URLSet
 * @post html_clink3() will insert 'option' as first parameter
 */
void html_URLOptSet(char *option)
{ 
  if (!html_host)
    die("html_URLOptSet: call html_URLSet() first") ;
  strReplace(&html_option, option) ;
}



/**
 * Construct parameterized URL to the program set in html_URLSet() (c for CGI).
 * @pre html_URLSet() was called
 */
char *html_clink3(char *class, char *method, char *p1, char *p2, char *p3) 
{ 
  if (html_port == -1)
    cgiURLCreate2(html_host, html_program) ;
  else
    cgiURLCreate(html_host, html_port, html_program) ;
  if (html_option)
    cgiURLAdd(html_option) ;
  cgiURLAdd(class) ;
  cgiURLAdd(method) ;
  if (p1)
    cgiURLAdd(p1) ;
  if (p2)
    cgiURLAdd(p2) ;
  if (p3)
    cgiURLAdd(p3) ;
  return cgiURLGet() ;
}



/**
 * Construct a parameterized hyperlink to the program set in html_URLSet() (h for hyperlink)
 * @pre html_URLSet() was called
 */
void html_hlink3(char *class, char *method, char *label, char *p1, char *p2, char *p3) 
{
  printf("<a href=\"%s\">%s</a>", html_clink3(class, method, p1, p2, p3), label) ;
}



/**
 * HTML-encode HTML-special chars.
 \verbatim 
 There are three operating modes: 
  - standard: 'inText' is assumed to be non-HTML; every HTML-special char
    is encoded. This mode is turned on by setting 'withExceptions' to 0;
  - a "half-HTML-mode": assume the text is non-HTML,
    except for hyperlinks (<a href>) and the two basic
    formatting markups <b> and <em>
    this mode is turned on by setting 'withExceptions' to 1
    and 'inText' IS NOT starting with "<html>"
  - a full HTML-mode
    'inText' is treated as being already in HTML format --
    i.e. this routine just copies the 'inText' to 'outText' without change;
    this mode is turned on by setting 'withExceptions' to 1
    and 'inText' starts with "<html>"
  \endverbatim
  * @param[in] inText Original text to be encoded
  * @param[in] outText Array of char, null-terminated
  * @param[in] withExceptions  If 1, then exceptions are left alone, else every special char is encoded
  * @param[out] outText Filled with encoded version of 'inText'
  */
void html_encode(char *inText, Array outText, int withExceptions) 
{ 
  char *cp = inText ;
  int i = 0;
  int j = 0;
  int inAnchorExcep = 0;
  int doReplace = 0;
  int step = 1;

  static struct {
    char orgChar;
    char *encoded;
  } et[] = {           /* table of chars needing encoding */
    { '<',  "&lt;"},
    { '>',  "&gt;"},
    { '&',  "&amp;"},
    { '"',  "&quot;"},
    { '\0', NULL } 
  } ;

  static struct {
    char *text ;
    int len ;
  } exceptions[] = {
    { "<A HREF", 7 },
    { "</A>", 4 },
    { "<EM>", 4 },
    { "</EM>", 5 },
    { "<B>", 3 },
    { "</B>", 4 },
    { NULL, 0 }
  } ;

  if (!inText || !outText)
    die("html_encode: null input") ;

  if (withExceptions && strNCaseEqual(inText, "<html>", 6)) {
    stringCpy(outText, inText) ;
    return ;
  }

  stringClear(outText);

  while (*cp) {
    doReplace = 0;
    step = 1;

    for (i = 0; et[i].orgChar; i ++ ) {

      if (*cp == et[i].orgChar) {

        if (withExceptions) {
          for( j = 0; exceptions[j].text; j ++ ) {
            if (strNCaseEqual (cp, exceptions[j].text, exceptions[j].len)) {
              if (j == 0)  /* remember anchor start was found */
                inAnchorExcep = 1;
              step = exceptions[j].len ;
              break;
            }
          }
          
          if (!exceptions[j].text)
            if (!inAnchorExcep) 
              doReplace = 1 ;
          
          if (inAnchorExcep && *cp == '>')
            inAnchorExcep = 0 ;
        }
        else
          doReplace = 1 ;

        break;
      }
    }
     
    if (doReplace) 
      stringCat(outText, et[i].encoded) ;
    else /* copy as is */
      stringNCat(outText, cp, step) ;
    
    cp += step ;
  }
}



/**
 * Like html_encode(), but manages the buffer.
 * @note Things like  printf("%s %s", html_encodeS(s1), html_encodeS(s2)) don't work because there is only one buffer.
 */
char *html_encodeS(char *s)
{ 
  static Stringa b = NULL ;
  stringCreateOnce(b, 100) ;
  html_encode(s, b, /*withExceptions*/0) ;
  return string(b) ;
}



static int gUniqueInt = 0 ;



/**
 * Generate an integer that is unique for this process.
   \verbatim
   Typcial usage: 
    printf("<a href=\"http://adr\" target=%d>", html_uniqueIntGet()) ;
    This will make the browser to open a new window when the user clicks the hyperlink
    \endverbatim
 * @bug Processes running in the same second will have the same "unique" id.
 */
int html_uniqueIntGet(void)
{
  if (!gUniqueInt)
    gUniqueInt = time(NULL) ;
  return gUniqueInt ;
}



/** 
 * Converts tab delimited ASCII data into HTML tables.
 * @param[in] tab Buffer containing data; new-line charater delimites rows, tab character separates items within a row; 
   HTML options can be included in the TD or TH tags: these options must be enclosed by backslash characters and 
   occur directly at the beginning of an item
 * @param[in] firstLineIsHeader If 1, for the first non-empty line TH tags will be used
 * @param[in] borderWidth 0 means no border
 * @param[in] withMarkup  1 to parse for \...\ at the beginning of each field, 0 to output each field unchanged
 * @param[out] tab Contents destroyed
 * @return A buffer containing an HTML formatted table; read/write access OK; memory managed by this routine.
  \verbatim
  Example:

  "Name \t Age \n
  Adam \t 45 \n
  Eva \t\bgcolor=red\ 42 \n"
  
  would be transformed into (assuming firstLineIsHeader==1, withMarkup==1)
  
  <table border=0>
  <tr><th>Name</th><th>Age</th></tr>\n
  <tr><td>Adam</td><td>45</td></tr>\n
  <tr><td>Eva</td><td bgcolor=red>42</td></tr>\n
  </table>
  \endverbatim
 */
char *html_tab2table(char *tab, int firstLineIsHeader, int borderWidth, int withMarkup)
{ 
  static Stringa html = NULL;
  LineStream ls;
  char *line;
  char hd = firstLineIsHeader ? 'H' : 'D' ; /* hd = <tH> or <tD> */
  WordIter wi;
  char *word;
  char *endOfMarkup;
  char *value ;
  
  stringCreateClear (html,1000);
  ls = ls_createFromBuffer (tab);
  stringAppendf (html,"<TABLE BORDER=%d>\n",borderWidth);
  while (line = ls_nextLine (ls)) {
    if (line[0] == '\0')
      continue;
    stringCat(html,"<TR>");

    wi = wordFldIterCreate (line,"\t");
    while (word = wordNext (wi))
      if (withMarkup && *word=='\\' && (endOfMarkup=strchr(word+1,'\\')) ) {
        *endOfMarkup='\0';
        value = endOfMarkup[1] ? endOfMarkup+1 : "&nbsp;" ; 
        stringAppendf (html,"<T%c %s>%s</T%c>",
                       hd,word+1,value,hd);
      }
      else {
        value = word[0] ? word : "&nbsp;" ; 
        stringAppendf (html,"<T%c>%s</T%c>",
                       hd,value,hd);
      } 

    wordIterDestroy (wi);
    stringCat(html,"</TR>\n");
    hd = 'D' ;
  }
  ls_destroy (ls);
  stringCat (html,"</TABLE>\n");
  return string (html);
}



/** 
 * Like html_tab2table(), but allows for several tables separted by lines not containing a tab character.
 * Inputs: same as for html_text2tables()
 * Output: contents of 'tab' destroyed
 * @return A buffer containing an HTML formatted table; read/write access OK; memory managed by this routine.
 * @note Input format: one or more table sections can be embedded in normal text; the beginning and end of 
   these sections is determined automatically (lines in tables contain at least one tab);
 \verbatim
  Example:

  "This is just ordinary text\n
  The table starts in the next line\n
  Name \t Age \n
  Adam \t 45 \n
  Eva \t\bgcolor=red\ 42 \n
  This is a footer"
  
  would be transformed into (assuming firstLineIsHeader==1, withMarkup==1)
  
  "This is just ordinary text<br>\n
  The table starts in the next line<br>\n
  <table border=0>
  <tr><th>Name</th><th>Age</th></tr>\n
  <tr><td>Adam</td><td>45</td></tr>\n
  <tr><td>Eva</td><td bgcolor=red>42</td></tr>\n
  </table>
  This is a footer<br>"
 \endverbatim
  */
char *html_text2tables (char *tab,int firstLineIsHeader,int borderWidth, int withMarkup)
{ 
  static Stringa html = NULL;
  static Stringa tabbuf  = NULL;
  LineStream ls;
  char *line;

  stringCreateClear (html,1000);
  stringCreateClear (tabbuf, 1000);
  ls = ls_createFromBuffer (tab);
  while (line = ls_nextLine (ls)) {
    if (line[0] == '\0')
      continue;
    if (!strchr(line,'\t')) {
      if (stringLen(tabbuf)) {
        stringCat(html, html_tab2table(string(tabbuf), firstLineIsHeader, borderWidth, withMarkup)) ;
        stringClear(tabbuf) ;
      }
      stringCat(html, line) ;
      stringCat(html, "<br>\n") ;
      continue;
    }
    /* line with 2 or more columns */
    stringCat(tabbuf, line) ;
    stringCat(tabbuf, "\n") ;
  }
  ls_destroy (ls);
  if (stringLen(tabbuf))
    stringCat(html, html_tab2table(string(tabbuf), firstLineIsHeader, borderWidth, withMarkup)) ;
  return string (html);
}

static FILE *gAppletTagOutFileP = NULL ;
static Texta gParamNames = NULL;
static Texta gParamValues = NULL;
static Stringa gEmbedTag = NULL;



/** 
 * Construct html that will start a java applet under Internet Explorer and in case this is not possible, 
   show a suitable diagnostic message to the user.
 * @param fp Output destination, e.g. stdout
 * @param jarFileUrls Example: "http://bioinfo.bas.roche.com:8080/apps/humangenome/genomeviewer.jar" (separate several JARs with ',' (comma))
 * @param appletClass Example: GenomeApplet.class
 * @param width In pixels
 * @param height In pixels
 * @post HTML for loading applet has been printed; html_appletParam() and html_appletTagClose() can be called
 * @bug Forces restart of Java Runtime Environment if mixed use of several Java versions.
 * @bug Under some circumstances starts a second Java console window
 */
void html_appletTagOpen(FILE *fp, char *jarFileUrls, char *appletClass, int width, int height)
{ 
  gAppletTagOutFileP = fp ;
  fprintf(fp, "<OBJECT classid=\"clsid:8AD9C840-044E-11D1-B3E9-00805F499D93\"\n"
         "WIDTH=%d HEIGHT=%d\n"
         "CODEBASE=\"http://java.sun.com/products/plugin/1.3/jinstall-13-win32.cab#Version=1,3,0,0\">\n"
         "<PARAM NAME=\"type\" VALUE=\"application/x-java-applet;version=1.3\">\n"
         "<PARAM NAME=\"code\" VALUE=\"%s\">\n"
         "<PARAM NAME=\"archive\" VALUE=\"%s\">\n",
         width, height, appletClass, jarFileUrls) ;
  if (!gParamNames)
    gParamNames = textCreate (10);
  else
    textClear (gParamNames);
  if (!gParamValues)
    gParamValues = textCreate (10);
  else
    textClear (gParamValues);
  stringCreateClear (gEmbedTag,100);
  stringCat (gEmbedTag,"<COMMENT>\n");
  stringAppendf (gEmbedTag,"<EMBED type=\"application/x-java-applet;version=1.3\" width=%d height=%d\n",
                 width,height);
  stringCat (gEmbedTag,"pluginspage=\"http://java.sun.com/products/plugin/1.3/plugin-install.html\"\n");
  stringCat (gEmbedTag,"code=\"");
  stringCat (gEmbedTag,appletClass);
  stringCat (gEmbedTag,"\"\narchive=\"");
  stringCat (gEmbedTag,jarFileUrls);
  stringCat (gEmbedTag,"\"\n");
}



/**
 * Include a parameter to an open applet tag.
 * @param[in] name Cannot be NULL
 * @param[in] value Can be NULL
 * @pre html_appletTagOpen() 
 * @bug neither name nor value must contain a double quote 
 */
void html_appletParam(char *name, char *value)
{ 
  if (strchr(name, '"') || (value && strchr(value, '"'))) 
    die("html_appletParam: \" found.") ;
  textAdd (gParamNames,name);
  if (value)
    textAdd (gParamValues,value);
  else
    textAdd (gParamValues,"");
}



/**
 * @pre html_appletTagOpen()
 * @post HTML closing applet tag has been printed html_appletTagOpen() can be called again
 */
void html_appletTagClose(void)
{
  int i;

  for (i=0;i<arrayMax (gParamNames);i++)
    fprintf(gAppletTagOutFileP,"<PARAM NAME=\"%s\" VALUE=\"%s\">\n",
            arru (gParamNames,i,char *),arru (gParamValues,i,char *));
  fprintf (gAppletTagOutFileP,"%s",string (gEmbedTag));
  for (i=0;i<arrayMax (gParamNames);i++)
    fprintf(gAppletTagOutFileP,"%s=\"%s\"\n",
            arru (gParamNames,i,char *),arru (gParamValues,i,char *));
  fprintf (gAppletTagOutFileP,"<NOEMBED>\n</COMMENT>\n");
  fprintf(gAppletTagOutFileP, "<B><FONT COLOR=red>Sorry, cannot start Java applet. Possible reasons: "
         "You are neither using MS-InternetExplorer nor Netscape (Mozilla) or your browser does not support "
         "Java or Java has been disabled (check your browser settings)</FONT></B>\n");
  fprintf (gAppletTagOutFileP,"</NOEMBED></EMBED></OBJECT>\n");
}





/* --------------------------- web start jnlp ------------------------------------------------- */

static FILE *gFp;
static Texta gJars = NULL;
static Texta gArgs = NULL;
static char *gMainClass = NULL;



/**
 * @param[in] fp Where to write the XML text
 * @param[in] codebase Code base of JAVA application
 * @param[in] title Title of the application
 * @param[in] homepage Home page of the application (can be NULL)
 * @param[in] description Description of the application (can be NULL)
 * @param[in] icon Icon of the application (can be NULL)
 * @param[in] allPermissions Whether the application has all permissions (essentially whether it is signed)
 * @param[in] heap Will be used in the j2se tag: max-heap-size, e.g. 400m
 * @param[in] mainClass Main class of the application
 */
void html_webstartOpen (FILE *fp,char *codebase,char *title,char *homepage,
                        char *description,char *icon,int allPermissions,
                        char *heap,char *mainClass)
{ 
  gFp = fp;
  fprintf (fp,"<jnlp spec='1.0+' codebase='%s'>\n",codebase);
  fprintf (fp,"  <information>\n");
  if (!title)
    die ("html_webstartOpen: a title is required");
  fprintf (fp,"    <title>%s</title>\n",title);
  fprintf (fp,"    <vendor>Bioinformatics Basel</vendor>\n");
  if (homepage)
    fprintf (fp,"    <homepage href='%s'/>\n",homepage);
  if (icon)
    fprintf (fp,"    <icon href='%s'/>\n",icon);
  if (description)
    fprintf (fp,"    <description>%s</description>\n",description);
  fprintf (fp,"  </information>\n");
  if (allPermissions) {
    fprintf (fp,"  <security>\n");
    fprintf (fp,"    <all-permissions/>\n");
    fprintf (fp,"  </security>\n");
  }
  fprintf (fp,"  <resources>\n");
  if (heap)
    fprintf (fp,"    <j2se version='1.4.+' max-heap-size='%s'/>\n",
             heap);
  else
    fprintf (fp,"    <j2se version='1.4.+'/>\n");
  gMainClass = hlr_strdup (mainClass);
  if (!gJars)
    gJars = textCreate (5);
  else
    textClear (gJars);
  if (!gArgs)
    gArgs = textCreate (5);
  else
    textClear (gArgs);
}



/**
 * @param[in] jar Name of a jar file
 * @pre Call after html_webstartOpen
 * @note Call this function for every jar used by the application
 */
void html_webstartAddJar (char *jar)
{ 
  textAdd (gJars,jar);
}



/**
 * @param[in] arg An argument of the application
 * @pre Call after html_webstartOpen
 * @note Call this function for every argument used to invoke the application
 */
void html_webstartAddArg (char *arg)
{
  static Stringa enc = NULL;

  stringCreateOnce (enc,20);
  html_encode (arg,enc,0);
  textAdd (gArgs,string (enc));
}



/**
 *  Must be called to finish writing the jnlp XML.
 */
void html_webstartClose (void)
{ 
  int i;

  for (i=0;i<arrayMax (gJars);i++)
    fprintf (gFp,"    <jar href='%s'/>\n",arru (gJars,i,char *));
  fprintf (gFp,"  </resources>\n");
  fprintf (gFp,"  <application-desc main-class='%s'>\n",gMainClass);
  hlr_free (gMainClass);
  for (i=0;i<arrayMax (gArgs);i++)
    fprintf (gFp,"    <argument>%s</argument>\n",arru (gArgs,i,char *));
  fprintf (gFp,"  </application-desc>\n");
  fprintf (gFp,"</jnlp>\n");
  if (gFp != stdout)
    fclose (gFp);
}

/* --------------------------------------------------------------------
  2000-06-08  klenka

  mini-module mp (multipart) for reading data from multipart forms
  these forms MUST have ENCTYPE="multipart/form-data" and can contain
  separate parts where each part may contain text or binary data

  functions from this mini-module use prefix cgiMp

  example program using these functions:
  on page:
  http://bioinfo.bas.roche.com:8080/bios/common/bioCompLibSeminar.html
  see 'Code example for file upload via WWW browser'
*/

static int gMpUsed = 0;
static int gMpBufferSize = 0;
static char *gMpBuffer = NULL;
static char gMpDelimiter[] = { 0x0d, 0x0a };
static int gMpBoundarySize = 0;
static char *gMpBoundary = NULL;
static char *gMpReadPos = NULL;
  
  
static int cgiIsMpInitialized(void) {
  /*
    returns 1 if mini module mp is initialized and 0 else
   */

  return gMpBufferSize && gMpBuffer && gMpBoundarySize && gMpBoundary;
}


static char *memmem(char *s1, int sz1, char *s2, int sz2) 
{ /*
  equivalent of strstr() on memory, returns pointer to first byte
  of first occurrence of s2 in s1
  */

  char *result = NULL;
  char *cp = s1;
  int sz = sz1 - sz2;

  while (cp - s1 <= sz) {
    if (!memcmp(cp, s2, sz2)) {
      result = cp;
      break;
    }

    cp ++;
  }

  return result;
}


/* useful for debugging when using mem stuff instead of zero-terminated strings
static void memputs(char *s, int sz) {
  int i = 0;

  while (i < sz)
    putchar((int) s[i ++]);
}
*/



/**
 * Initialize module for reading multi-part forms.
 * @note These forms MUST have ENCTYPE="multipart/form-data" 
 * @pre mp not initialized
 * @post mp initialized, other cgiMp... functions can be called
  \verbatim
  Example call sequence:
  cgiMpInit();
  while (cgiMpNext(a, b, c, d))
  ...
  cgiMpReset();
  while (cgiMpNext(a, b, c, d))
  ...
  cgiMpDeinit();
        
  deinitialization may be crucial e.g. if VERY LARGE files are read
  
  since these functions are reading from stdin initialization will 
  be allowed only once; but contents can be read multiple times by 
  using cgiMpReset()
  \endverbatim
 */
void cgiMpInit(void) 
{ 
  char *contentType = getenv("CONTENT_TYPE");
  char *delimiterPos = ILLADR;

  if (cgiIsMpInitialized())
    die("cgiMpInit(): illegal use - already initialized");

  if (gMpUsed)
    die("cgiMpInit(): illegal use - has been used before");

  if (!contentType || strNDiffer(contentType, "multipart/form-data", 19))
    die("cgiMpInit(): invalid content-type: %s", s0(contentType));

  gMpBufferSize = atoi(getenv("CONTENT_LENGTH"));
  gMpBuffer = hlr_malloc(gMpBufferSize);

  if (fread(gMpBuffer, gMpBufferSize, 1, stdin) != 1)
    die("cgiMpInit(): input length does not match content length %d", gMpBufferSize);

  delimiterPos = memmem(gMpBuffer, gMpBufferSize, gMpDelimiter, sizeof(gMpDelimiter));

  if (!delimiterPos || delimiterPos == gMpBuffer)
    die("cgiMpInit(): invalid content format: %s", gMpBuffer);

  gMpBoundarySize = delimiterPos - gMpBuffer;
  gMpBoundary = hlr_malloc(gMpBoundarySize);
  memcpy(gMpBoundary, gMpBuffer, gMpBoundarySize);

  gMpUsed = 1;

  cgiMpReset();
}



/**
 * Returns next item/value pair plus optional information for file upload.
 * @param[in] item Stringa to store item name in, must not be NULL
 * @param[in] value Array of char* to store item value in, must not be NULL
 * @param[in] filename Stringa to store filename in if any; may be NULL if not interested
 * @param[in] contentType Stringa to store content MIME type in; may be NULL if not interested
 * @param[out] item Name of the item read
 * @param[out] value Value of the item; a Stringa if form data or a text file; binary buffer (no trailing null-termination character)
   when data from a binary file
 * @param[out] filename The filename if value is from a file, an empty string "" if value is not from a file but a form field.
 * @param[out] contentType MIME type of content; empty ("") if form data or a text file (MIME type="text/plain"); a binary file else.
 * @return 0 if all items used up, 1 else
 * @pre Module initialized: cgiMpInit()
 */
int cgiMpNext(Stringa item, Array value, Stringa filename, 
              Stringa contentType) 
{ 
  int result = 0;
  char *readPos = ILLADR;
  char *boundaryPos = ILLADR;
  char *delimiterPos = ILLADR;
  char *namePos = ILLADR;
  char *filenamePos = ILLADR;
  Stringa myContentType = stringCreate(32);

  if (!cgiIsMpInitialized())
    die("cgiMpNext(): illegal use - not initialized");

  stringClear(item);
  stringClear(value);
  
  if (filename)
    stringClear(filename);

  if (contentType)
    stringClear(contentType);

  boundaryPos = memmem(gMpReadPos, gMpBufferSize - (gMpReadPos - gMpBuffer), 
		       gMpBoundary, gMpBoundarySize);

  if (boundaryPos) {
    result = 1;
    readPos = gMpReadPos;
    gMpReadPos = boundaryPos + gMpBoundarySize + sizeof(gMpDelimiter);

    /* parse content-disposition line */
    delimiterPos = memmem(readPos, boundaryPos - readPos, 
			  gMpDelimiter, sizeof(gMpDelimiter));

    if (delimiterPos == NULL)
      die("cgiMpNext(): no line delimiter found until next boundary: %s", readPos);

    if (strNDiffer(readPos, "Content-Disposition: form-data", 30))
      die("cgiMpNext(): invalid part format", readPos);

    if (namePos = memmem(readPos, delimiterPos - readPos, "name=", 5))
      strCopySubstr(namePos, '"', '"', item);
    else
      die("cgiMpNext(): invalid part format: %s", readPos);

    if (filename)
      if (filenamePos = memmem(namePos, delimiterPos - readPos, "filename=", 9))
	strCopySubstr(filenamePos, '"', '"', filename);

    /* parse optional content-type line */
    readPos = delimiterPos + sizeof(gMpDelimiter);
    delimiterPos = memmem(readPos, boundaryPos - readPos, 
			  gMpDelimiter, sizeof(gMpDelimiter));

    if (delimiterPos == NULL)
      die("cgiMpNext(): no line delimiter found until next boundary: %s", readPos);

    if (strNEqual(readPos, "Content-Type: ", 14)) {
      strCopySubstr(readPos + 13, ' ', 0x0d, myContentType);
      if(strEqual(string(myContentType), "text/plain"))
        stringClear(myContentType);
      readPos = delimiterPos + sizeof(gMpDelimiter);
      delimiterPos = memmem(readPos, boundaryPos - readPos, 
			    gMpDelimiter, sizeof(gMpDelimiter));
    }

    /* require blank line separation of part header and part data */
    if (delimiterPos != readPos)
      die("cgiMpNext(): invalid part format: %s", readPos);

    readPos += sizeof(gMpDelimiter);

    /* read part data (without final delimiter) */
    if (isEmptyString(myContentType)) {
      stringNCpy(value, readPos, boundaryPos - readPos - sizeof(gMpDelimiter));
      stringTranslate(value, "\r", "");
    }
    else {
      arraySetMax(value, 0);
      
      while (readPos < boundaryPos - sizeof(gMpDelimiter)) {
	array(value, arrayMax(value), char) = *readPos;
	readPos ++;
      }
    }
  }

  if (contentType)
    stringCpy(contentType, string(myContentType));

  stringDestroy(myContentType);

  return result;
}



/**
 * Reset multi-part reading to first part.
 * @pre Module initialized, 0 - all parts have been read
 * @post Ready for reading from first part again
 */
void cgiMpReset(void) 
{ 
  if (!cgiIsMpInitialized())
    die("cgiMpReset(): illegal use - not initialized");

  gMpReadPos = gMpBuffer + gMpBoundarySize + sizeof(gMpDelimiter);
}



/**
 * Deinitialize multi-part module and release all temporarily allocated resources.
 * @pre Module initialized
 * @post Module not initialized
 * @note Important to use if some parts are files which can be VERY LARGE
*/
void cgiMpDeinit(void) 
{ 
  if (!cgiIsMpInitialized())
    die("cgiMpDeinit(): illegal use - not initialized");

  gMpReadPos = NULL;
  gMpBoundarySize = 0;
  hlr_free(gMpBoundary);
  gMpBufferSize = 0;
  hlr_free(gMpBuffer);
}

/* ------------------------ end of mini-module mp --------------------- */




/**
 * Print a generic style sheet. This should be enclosed within the head tags.
 */
void html_printGenericStyleSheet (int bodyFontSize)
{
  puts ("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">");
  puts ("<style type=\"text/css\" media=\"screen\">");
  printf ("body {font:%dpx Verdana,Arial,Helvetica, sans-serif; color:#2F4F4F; margin:50px 50px 50px 50px;}\n",bodyFontSize);
  puts ("h1 {font-size:20px; font-weight:1200; color:#0E4C92;}");
  puts ("h2 {font-size:14px; font-weight:1000; color:#0E4C92;}");
  puts (".note {font-style:italic}");
  puts ("a       {color:#0000CD; text-decoration:none;}");
  puts ("a:hover {color:#4B0082; text-decoration:none;}");  
  printf ("table {border-collapse:collapse; font-size:%dpx;}\n",bodyFontSize);
  puts ("</style>");
}
