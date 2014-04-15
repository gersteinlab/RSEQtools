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
 *   \file linestream.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#ifndef _nextline_h_
#define _nextline_h_

#include "format.h"

/**
 * LineStream.
 */
typedef struct _lineStreamStruct_ {
  /* the members of this struct are PRIVATE for the
     LineStream module -- DO NOT access from outside
     the LineStream module */
  FILE *fp;
  char *line;
  int lineLen;
  WordIter wi;
  int count;
  int status ;  /* exit status of popen() */
  char *(*nextLine_hook)(struct _lineStreamStruct_ *);
  Stringa buffer ;    /* NULL if not in buffered mode, else used
                         used for remembering last line seen */
  char *bufferLine ;  /* pointer to 'buffer' or NULL if EOF */
  int bufferBack ;    /* 0=normal, 1=take next line from buffer */
} *LineStream;

extern LineStream ls_createFromFile (char *fn);
extern LineStream ls_createFromPipe (char *command);
extern LineStream ls_createFromBuffer (char *buffer);
extern char *ls_nextLine (LineStream this1);
extern void ls_destroy_func (LineStream this1); /* do not use this function */

/**
 * Destroy a line stream.
 * @see ls_destroy_func()
 */
#define ls_destroy(this1) (ls_destroy_func(this1),this1=NULL) /* use this one */
extern int ls_lineCountGet (LineStream this1);
extern void ls_cat(LineStream this1, char *filename) ;
extern int ls_skipStatusGet(LineStream this1) ;
extern int ls_isEof(LineStream this1) ;
extern void ls_bufferSet(LineStream this1, int lineCnt) ;
extern void ls_back(LineStream this1, int lineCnt) ;
#endif
