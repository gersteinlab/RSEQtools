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
 *   \file log.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


/*  
purpose: basic logging service
optional inputs (at compile time): HLR_ASSERT_OFF
                                   HLR_DEBUG_ON
*/

#ifndef DEF_LOG_H
#define DEF_LOG_H

#include <stdio.h>
#include <stdarg.h>

typedef void (*RoMsgFunc)(char *format, va_list args) ;
typedef void (*LogExitFunc)(void) ;

extern void log_registerExit(LogExitFunc f) ;
extern void log_registerDie(RoMsgFunc f) ;
extern void log_deregisterDie(RoMsgFunc f) ;
extern void log_registerWarn(RoMsgFunc f) ;
extern void log_deregisterWarn(RoMsgFunc f) ;
extern void log_registerUsage(RoMsgFunc f) ;
extern void log_deregisterUsage(RoMsgFunc f) ;
extern void log_registerRomsg(RoMsgFunc f) ;
extern void log_deregisterRomsg(RoMsgFunc f) ;

extern void die(char *format,...) ;
extern void warn(char *format,...) ;
extern void usage(char *format,...) ;
extern void romsg(char *format,...) ;


/* private */ extern int log_debugLevel ;
#define debugLevelSet(level) (log_debugLevel=(level))
#define debugLevelGet() log_debugLevel
#ifdef HLR_DEBUG_ON
#define debugMsg(level,msg) {if (level<=debugLevelGet()) printf msg;}
#else
#define debugMsg(level,msg) 
#endif
/* example:
   debugMsg(1,("error %s",var))
   debugLevelSet(9)  -- see all messages
   debugLevelSet(0)  -- see only very serious messages
   -- note the paranthesis surrounding the second argument of the macro --
   -- if omitted the compiler will produce messages which are hard to understand --
*/


extern void warnAdd(char *source, char *msg) ;
extern char *warnReport(void) ;
extern void warnReset(void) ;
extern void warnIterInit(void) ;
extern int warnIterNext(char **source, char **msg) ;
extern int warnCount(char *sourceStart, char *msgStart) ;


extern void LogOpen(char *fname) ;
extern void Log(char *format,...) ;
extern void LogT(char *format,...) ;
extern void LogClose (void) ;
extern int LogOpened(void) ;
extern void LogPrintTime(FILE *f) ;

/* asserts that 'truecondition' is true;
   if 'truecondition' evaluates to 0, 
     'message' is displayed and the program is aborted.;
   else (i.e. non-zero result) nothing happens;
   hlr_assert is an expression that returns 1; therefore things like
     while (hlr_assert(memory_check(),"msg"), i<10) { ... }
   are syntactically valid.
   If at compile time the symbol 'HLR_ASSERT_OFF' is defined,
   no code is generated for hlr_assert. The idea is to allow for
   programs to run in production mode as fast a possible.
*/
#ifdef HLR_ASSERT_OFF
#define hlr_assert(truecondition,message) 1
#else
#define hlr_assert(truecondition,message) ((truecondition)?1:(die(message),0))
#endif

#endif

