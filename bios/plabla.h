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



#ifndef _PLABLA_H_
#define _PLABLA_H_


#include "plabla_conf.h"

#ifndef BIOS_PLATFORM
#define BIOS_PLATFORM -1 /* "undefined, because #include plabla_conf.h failed" */
#endif

#if (BIOS_PLATFORM == BIOS_PLATFORM_IRIX) || (BIOS_PLATFORM == BIOS_PLATFORM_SOLARIS) || (BIOS_PLATFORM == BIOS_PLATFORM_LINUX)
#define PLABLA_INCLUDE_IO_UNISTD <unistd.h>
#define PLABLA_POPEN popen
#define PLABLA_PCLOSE pclose
#define PLABLA_OPEN open
#define PLABLA_CLOSE close
#define PLABLA_ISATTY isatty
#endif

#if BIOS_PLATFORM == BIOS_PLATFORM_IRIX
#define PLABLA_FLOCK_OPENFFLAG O_RDONLY
#define PLABLA_FLOCK(fildes) flock(fildes,LOCK_EX)
#define PLABLA_FLOCKNB(fildes) flock(fildes,LOCK_EX|LOCK_NB)
#define PLABLA_FUNLOCK(fildes) flock(fildes,LOCK_UN)
#define PLABLA_ISLOCKED EWOULDBLOCK
#endif

#if BIOS_PLATFORM == BIOS_PLATFORM_LINUX
#define PLABLA_FLOCK_OPENFFLAG O_RDONLY
#define PLABLA_FLOCK(fildes) flock(fildes,LOCK_EX)
#define PLABLA_FLOCKNB(fildes) flock(fildes,LOCK_EX|LOCK_NB)
#define PLABLA_FUNLOCK(fildes) flock(fildes,LOCK_UN)
#define PLABLA_ISLOCKED EWOULDBLOCK
#endif

#if BIOS_PLATFORM == BIOS_PLATFORM_SOLARIS
#define PLABLA_FLOCK_OPENFFLAG O_RDWR
#define PLABLA_FLOCK(fildes) lockf(fildes,F_LOCK,0)
#define PLABLA_FLOCKNB(fildes) lockf(fildes,F_TLOCK,0)
#define PLABLA_FUNLOCK(fildes) lockf(fildes,F_ULOCK,0)
#define PLABLA_ISLOCKED EAGAIN
#endif

#if BIOS_PLATFORM == BIOS_PLATFORM_WINNT
#define PLABLA_INCLUDE_IO_UNISTD <io.h>
#define PLABLA_OPEN _open
#define PLABLA_CLOSE _close
#define PLABLA_ISATTY _isatty
#define PLABLA_POPEN _popen
#define PLABLA_PCLOSE _pclose
#endif

#if BIOS_BITS_PER_INT == 32
#define intgr int
#define intgr2 short
#define intgr4 int
#define intgr8 long long
#define INTGR_MIN (-2147483647-1)
#define INTGR_MAX (2147483647)
#define UINTGR_MAX (4294967295u)
#define INTGR_FM ""
#endif

#if BIOS_BITS_PER_INT == 64
#define intgr long
#define intgr2 short
#define intgr4 int
#define intgr8 long 
#define INTGR_MIN (-9223372036854775807LL-1)
#define INTGR_MAX (9223372036854775807LL)
#define UINTGR_MAX (18446744073709551615uLL)
#define INTGR_FM "l"
#endif

#if BIOS_BITS_PER_INT == 32 && BIOS_PLATFORM == BIOS_PLATFORM_SOLARIS
#define PLABLA_STAT stat64
#define PLABLA_LSTAT lstat64
#else
#define PLABLA_STAT stat
#define PLABLA_LSTAT lstat
#endif

#endif
