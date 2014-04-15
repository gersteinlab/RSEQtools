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
 *   \file hlrmisc.c Miscelleanous routines
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#include <stdlib.h>

#include "hlrmisc.h"
#include "log.h"

int hlr_allocCnt = 0 ;  /* number of memory blocks allocated */


char *hlr_strmcpyI(char *to, char *from, int toLength) {
  to[toLength-1] = '\0' ;
  strncpy(to, from, toLength-1) ;
  return to ;
}



char *hlr_strcpysFunc(char *to, char *from, int toLengthMax) 
{ /*
  documentation see hlrmisc.h, macro hlr_strcpys()
  */
  if (strlen(from) > toLengthMax)
    die("hlr_strcpys: overflow: toLengthMax=%d, strlen(from)=%d, from='%.50s...'", 
        toLengthMax, strlen(from), from) ;
  strcpy(to, from) ;
  return to ;
}



void *hlr_mallocs(size_t size) 
{
  void *p = malloc(size) ;
  if (!p)
    die("hlr_mallocs(%d)", size) ;
  return p ;
} 



void *hlr_callocs(size_t nelem, size_t elsize)
{
  void *p = calloc(nelem, elsize) ;
  if (!p)
    die("hlr_callocs(%d,%d)", nelem, elsize) ;
  return p ;
}



char *hlr_strdups(char *s1)
{
  char *s2 = strdup(s1) ;
  if (!s2)
    die("hlr_strdups([%d bytes]) failed.", strlen(s1)+1) ;
  return s2 ;
}



char *s0f(char *s)
{
  return s0(s) ;
}



/**
 * Execute shell command.
 * @param[in] cmd
 * @param[in] nonZeroOK If 1, then a non-zero exit status from system is tolerated. If 0, then a non-zero exit status from system leads to a die()
 * @return Exit status (0=OK). A non-zero exit status is only be returned if nonZeroOK=1
 * @note Exit status -1 indicates a serious condition of the operating system therefore causes a die() even if nonZeroOK is 1.
 */
int hlr_system(char *cmd, int nonZeroOK)
{ 
  int rc = system(cmd) ;
  if (rc == -1 || (rc != 0 && !nonZeroOK))
    die("system(%s) exit status %d.", cmd, rc) ;
  return rc ;
}

