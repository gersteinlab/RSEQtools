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
 *   \file html.h 
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */

#ifndef DEF_HTML_H
#define DEF_HTML_H

#include "format.h"

extern void cgiInit(void) ;

extern char *cgiGet2Post(void) ;
extern void cgiGet2PostReset(void);

extern int cgiGetNextPair (int *first,Array item,Array value) ; /*
  usage:
    int first = 1 ;
    Array item  = arrayCreate(10, char) ;
    Array value = arrayCreate(10, char) ;
    while (getNextPair(&first,item,value)) { ... } 
  */
extern void cgiGetInit(void) ;
extern char *cgiGetNext(Stringa value) ;
extern char *cgiGetByName (char *name) ;
extern char *cgiGetByNameM(char *name) ;
extern void cgiGetF(char *fieldName, Stringa value) ;
extern int cgiGetFo(char *fieldName, Stringa value) ;

void cgiEncodeWord(char *s, Stringa a) ;
void cgiDecodeWord (Stringa a);

extern void cgiURLCreate (char *host,int port,char *program);
extern void cgiURLCreate2(char *cgiServerUrl, char *program);
extern void cgiURLAdd (char *param);
extern void cgiURLAddInt(int param);
extern void cgiURLAddNV(char *name,char *value);
extern char *cgiURLGet (void);

extern int cgiIsCGI (void);
extern void cgiExpiresSet(int seconds_valid) ;
extern void cgiRedirSet(char *url) ;
extern void cgiDomainSet(char *domain) ;
extern void cgiEncodingSet(char *charset) ;
extern void cgiHeader (char *mimeType);
extern int cgiHeaderIsPrinted(void) ;

/* usage of the cookie routine:
  in the process sending the page with the cookie in its header, use
    char *c = cgiConstructCookie("myparam", "somevalue", 0) ;
    cgiHeaderCookie(0, c) ;
  in the process receiving input use
    char *v = cgiReadCookie("myparam")
*/
char *cgiConstructCookie(char *name, char *value, int lifelength) ;
void cgiHeaderCookie(char *mimeType, char *cookieSpec) ;
char *cgiReadCookie(char *name) ;

/* all functions here should start with 'cgi'
   this macro is defined for backward compatibility, 
   don't use getNextPair in new code */
#define getNextPair cgiGetNextPair

/* ------------------- mini-module mp ---------- */
extern void cgiMpInit(void);
extern int cgiMpNext(Stringa item, Array /*of char*/ value, Stringa filename, Stringa contentType);
extern void cgiMpReset(void);
extern void cgiMpDeinit(void);

/* ------------------- for generating Java Applet tag --------- */
extern void html_appletTagOpen(FILE *fp, char *jarFileUrls, char *appletClass, int width, int height) ;
extern void html_appletParam(char *name, char *value) ;
extern void html_appletTagClose(void) ;

/* ------------------- for generating webstart XML jnlp --------- */
extern void html_webstartOpen (FILE *fp,char *codebase,char *title,char *homepage,
                               char *description,char *icon,int allPermissions,
                               char *heap,char *mainClass);
extern void html_webstartAddJar (char *jar);
extern void html_webstartAddArg (char *arg);
extern void html_webstartClose (void);

/* ------------------- more HTML-constructing functions ---------- */
extern void html_URLSet(char *host, int port, char *program) ;
extern void html_URLSet2(char *cgiServerUrl, char *program) ;
extern void html_URLOptSet(char *option) ;
extern char *html_clink3(char *class, char *method, char *p1, char *p2, char *p3) ;
#define html_clink(c,m) html_clink3(c,m,0,0,0) 
#define html_clink1(c,m,p1) html_clink3(c,m,p1,0,0) 
#define html_clink2(c,m,p1,p2) html_clink3(c,m,p1,p2,0) 

extern void html_hlink3(char *class, char *method, char *label, char *p1, char *p2, char *p3) ;
#define html_hlink(c,m,l) html_hlink3(c,m,l,0,0,0) 
#define html_hlink1(c,m,l,p1) html_hlink3(c,m,l,p1,0,0) 
#define html_hlink2(c,m,l,p1,p2) html_hlink3(c,m,l,p1,p2,0) 

extern void html_encode(char *org, Array enc, int withExceptions) ;
extern char *html_encodeS(char *s) ;
extern int html_uniqueIntGet(void) ;

extern char *html_tab2table (char *tab,int firstLineIsHeader,int borderWidth, int withMarkup);
extern char *html_text2tables (char *tab,int firstLineIsHeader,int borderWidth, int withMarkup);

extern void html_printGenericStyleSheet (int bodyFontSize);

#endif
