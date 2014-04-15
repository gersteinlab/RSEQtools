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
 *   \file linestream.c Read lines from a file, pipe or buffer
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


/* 
   Module lineStream
   Transparent way to read lines from a file, pipe or buffer
   The lines always come without \n at the end
*/


#include "plabla.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include PLABLA_INCLUDE_IO_UNISTD

#include "log.h"
#include "format.h"
#include "hlrmisc.h"
#include "linestream.h"


static char *nextLineFile (LineStream this1);
static char *nextLinePipe (LineStream this1);
static char *nextLineBuffer (LineStream this1);
static void register_nextLine (LineStream this1,char *(*f)(LineStream this1));


/**
 * Creates a line stream from a file.
 * @param[in] fn File name ("-" means stdin)
 * @return  A line stream object, NULL if file could not been opened; to learn details call warnReport() from module log.c
 */
LineStream ls_createFromFile (char *fn)
{ 
  LineStream this1;

  if (!fn)
    die ("ls_createFromFile: no file name given");
  this1 = (LineStream) hlr_malloc (sizeof (struct _lineStreamStruct_));
  this1->line = NULL;
  this1->count = 0;
  this1->status = 0;
  if (strcmp (fn,"-") == 0)
    this1->fp = stdin;
  else
    this1->fp = fopen (fn,"r");
  if (!this1->fp) {
    warnAdd("ls_createFromFile", 
            stringPrintBuf("'%s': %s", fn, strerror(errno))) ;
    hlr_free (this1);
    return NULL;
  }
  register_nextLine (this1,nextLineFile);
  this1->buffer = NULL ;
  return this1;
}



/** 
 * Returns the next line of a file and closes the file if no further line was found. The line can be of any length.
 * A trailing \n or \r\n is removed.
 * @param[in] this1 line stream object
 * @return The line, NULL if no further line was found
 * @note Memory managed by this routine
 */
static char *nextLineFile (LineStream this1)
{ 
  int ll;

  if (!this1)
    die ("nextLineFile: NULL LineStream");
  if (!(ll = getLine (this1->fp,&this1->line,&this1->lineLen))) {
    fclose (this1->fp);
    this1->fp = NULL;
    hlr_free (this1->line);
    return NULL;
  }
  if (ll > 1 && this1->line[ll-2] == '\r')
    this1->line[ll-2] = '\0';
  else if (ll > 0 && this1->line[ll-1] == '\n')
    this1->line[ll-1] = '\0';
  this1->count++;
  return this1->line;
}



/** 
 * Creates a line stream from a pipe.
 * Example: ls_createFromPipe ("zcat test.dat.Z");
 * @param[in] command As it would be written on the command line
 * @return A line stream object, NULL if the pipe could not been opened
 * @post warnCount(NULL,NULL) !=0 if problem occured
 */
LineStream ls_createFromPipe (char *command)
{ 
  LineStream this1;

  if (!command)
    die ("ls_createFromPipe: no command given");
  this1 = (LineStream) hlr_malloc (sizeof (struct _lineStreamStruct_));
  this1->line = NULL;
  this1->count = 0;
  this1->status = -2;  /* undetermined */
  this1->fp = PLABLA_POPEN (command,"r");
  if (!this1->fp) {
    warnAdd("ls_createFromPipe", 
            stringPrintBuf("'%s': %s", command, strerror(errno))) ;
    return NULL;
  }
  register_nextLine (this1,nextLinePipe);
  this1->buffer = NULL ;
  return this1;
}



static char *nextLinePipe (LineStream this1)
{ /* returns the next line from the pipe and closes the pipe if
     no further line was found. The line can be of any length and
     is returned without \n at the end
     input: line stream object
     output: the line
             NULL if no further line was found
  */
  int ll;

  if (!this1)
    die ("nextLinePipe: NULL LineStream");
  if (!(ll = getLine (this1->fp,&this1->line,&this1->lineLen))) {
    this1->status = PLABLA_PCLOSE (this1->fp);
    this1->fp = NULL;
    hlr_free (this1->line);
    return NULL;
  }
  if (ll > 1 && this1->line[ll-2] == '\r')
    this1->line[ll-2] = '\0';
  else if (ll > 0 && this1->line[ll-1] == '\n')
    this1->line[ll-1] = '\0';
  this1->count++;
  return this1->line;
}



/**
 * Creates a line stream from a buffer.
 * @param[in] buffer  A buffer pointer, must not be NULL
 * @return A line stream object
 * @note The buffer will be destroyed after using ls_nextLine (newline characters are replaced by null-termination characters). 
   Work on a copy of the buffer if you want to use it again.
 */
LineStream ls_createFromBuffer (char *buffer)
{ 
  LineStream this1;
  int len ;
  int manySepsAreOne = 0 ;

  if (!buffer) 
    die ("ls_createFromBuffer: NULL buffer");
  len = strlen(buffer) ;
  if (len) {
    if (buffer[len-1] == '\n') 
      buffer[--len] = '\0' ;
  }
  else
    manySepsAreOne = 1 ; /* immediately return NULL */
      
  this1 = (LineStream) hlr_malloc (sizeof (struct _lineStreamStruct_));
  this1->count = 0;
  this1->status = 0;
  this1->wi = wordIterCreate(buffer,"\n", manySepsAreOne);
  register_nextLine (this1,nextLineBuffer);
  this1->buffer = NULL ;
  return this1;
}



static char *nextLineBuffer (LineStream this1)
{ /* 
  returns the next line of a buffer. The line can be of any length and
  a trailing \n or \r\n is removed.
  input: line stream object
  output: the line
          NULL if no further line was found
  */
  char *s ;
  int len ;

  if (!this1)
    die ("nextLineFile: NULL LineStream");
  s = wordNextG(this1->wi, &len) ;
  if (!s) {
    wordIterDestroy (this1->wi);
    return NULL;
  }
  this1->count++;
  if (len && s[len-1] == '\r')
    s[len-1] = '\0' ;
  return s;
}



/**
 * Destroys a line stream object after closing the file or pipe if they are still open (stream not read to the end) 
   or after destroying the word iterator if the stream was over a buffer.
 * @param[in] this1 A line stream 
 * @note Do not call this function, but use the macro ls_destroy
*/
void ls_destroy_func (LineStream this1)
{ 
  char line[1000];

  if (!this1) 
    return ;

  if (this1->nextLine_hook == nextLinePipe && this1->fp) {
    while (fgets (line,sizeof (line),this1->fp)) {}
    this1->status = PLABLA_PCLOSE (this1->fp);
    hlr_free (this1->line);
  }
  else if (this1->nextLine_hook == nextLineFile && this1->fp) {
    /* if (this1->fp == stdin) */
    if (!PLABLA_ISATTY(fileno(this1->fp)))
      while (fgets (line,sizeof (line),this1->fp)) {}
    fclose (this1->fp);
    hlr_free (this1->line);
  }
  else if (this1->nextLine_hook == nextLineBuffer && this1->wi) {
    wordIterDestroy (this1->wi);
  }
  stringDestroy(this1->buffer) ;
  hlr_free (this1);
}




static void register_nextLine (LineStream this1,char *(*f)(LineStream this1))
{ /* internally used to register the actual function which gets the
     next line 
  */
  this1->nextLine_hook = f;
}



/**
 * Get the next line from a line stream object.
 * This function is called from the application programs independently whether the stream is from a file, pipe or buffer
 * @param[in] this1 A line stream 
 * @return A line without trailing newlines if there is still a line, else NULL
 * @note The memory returned belongs to this routine; it may be read and written to, 
   but not free'd or realloc'd by the user of this routine; it stays stable until the next call ls_nextLine(this1).
  */
char *ls_nextLine (LineStream this1)
{ 
  char *line ;
  if (!this1) 
    die("%s", warnCount(NULL,NULL) ? warnReport() : "ls_nextLine: invalid LineStream") ;
  if (this1->buffer) {
    if (this1->bufferBack) {
      this1->bufferBack = 0 ;
      line = this1->bufferLine ;
    }
    else {
      /* only get the next line if there we did not yet see the
         end of file */
      line = this1->bufferLine ? this1->nextLine_hook (this1) : NULL ;
      if (line) {
        stringCpy(this1->buffer, line) ;
        this1->bufferLine = string(this1->buffer) ;
      }
      else
        this1->bufferLine = NULL ;
    }
  }
  else 
    line = this1->nextLine_hook (this1) ;
  return line ;
}



/**
 * Push back 'lineCnt' lines.
 * @param[in] this1 A line stream 
 * @param[in] lineCnt  How many lines should ls_nextLine() repeat (currently, only lineCnt==1 is supported)
 * @pre ls_bufferSet() was called.
 * @post Next call to ls_nextLine() will return the same line again
*/
void ls_back(LineStream this1, int lineCnt) 
{ 
  if (! this1->buffer)
    die("ls_back() without preceeding ls_bufferSet()") ;
  if (this1->bufferBack)
    die("ls_back() twice in a row") ;
  if (lineCnt != 1)
    die("ls_back: sorry, not yet implemented") ;
  this1->bufferBack = 1 ;
}



/**
 * Set how many lines the linestream should buffer.
 * @param[in] this1 A line stream 
 * @param[in] lineCnt How many lines should ls_nextLine() repeat (currently, only lineCnt==1 is supported)
 * @pre ls_create*
 * @post ls_back() will work
 */
void ls_bufferSet(LineStream this1, int lineCnt)
{ 
  if (this1->buffer || this1->count)
    die("ls_bufferSet() more than once or too late") ;
  if (lineCnt != 1)
    die("ls_bufferSet() sorry, not yet implemented") ;
  this1->buffer = stringCreate(80) ;
  this1->bufferBack = 0 ;
  this1->bufferLine = "" ;  /* dummy init, to kick off reading */
}



/** 
 * Returns the number of the current line.
 * @param[in] this1 A line stream 
*/
int ls_lineCountGet (LineStream this1)
{ 
  return this1->count;
}



/**
 * Skips remainder of line stream and returns exit status which  only meaningful when created from a pipe - exit status 
   for file and buffer will always be 0.
 * @param[in] this1 A line stream 
 * @return If 'this1' was created by ls_createFromPipe(command),the exit status of 'command' is returned; else 0 
 * @post line stream is read to its end - ls_nextLine() must not be called anymore.
 * @note For reasons of efficiency, skip does not actually read the linestream therefore ls_lineCountGet() 
   will not return the correct line number after this function has been called.
 */
int ls_skipStatusGet(LineStream this1)
{ 
  if (this1->nextLine_hook == nextLineBuffer) {
    if (this1->wi)
      wordIterDestroy(this1->wi);
  }
  else if (this1->nextLine_hook == nextLineFile) {
    if (this1->fp) {
      fclose (this1->fp);
      this1->fp = NULL;
      hlr_free (this1->line);
    }
  }
  else if (this1->nextLine_hook == nextLinePipe) {
    if (this1->fp)
      while(nextLinePipe(this1))
	;
  }

  return this1->status ;
}



/**
 * Redirect a linestream.
 * @param[in] this1 A line stream created by one of ls_create*()
 * @param[in] filename  Name of file to write lines to; special cases: '-'  means stdout, NULL means /dev/null (discard) 
 * @post this1 contains no more lines; file 'filename' contains the contents of 'this1'
 */
void ls_cat(LineStream this1, char *filename) 
{ 
  if (filename) {
    char *line ;
    FILE *f ;
    if (strEqual(filename, "-"))
      f = stdout ;
    else 
      f = fopen(filename, "w") ;
    if (!f) {
      die("%s: in ls_cat(%s)", strerror(errno), filename) ;
    }
    while (line = ls_nextLine(this1)) {
      fputs(line, f) ;
      putc('\n', f) ;
    }
    if (f != stdout)
      fclose(f) ;
  }
  else 
    while (ls_nextLine(this1)) ;
}



/**
 * Determine the state of a line stream.
 * @param[in] this1 A line stream
 * @return 0 if there are more lines retrievable by ls_nextLine(). 1 if line stream is at end i.e. ls_nextLine() 
   has returned NULL and must not be called anymore.
 */
int ls_isEof(LineStream this1) 
{ 
  if (this1->nextLine_hook == nextLineBuffer)
    return this1->wi == NULL ? 1 : 0 ;
  else
    return this1->fp == NULL ? 1 : 0 ;
}
