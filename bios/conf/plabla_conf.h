/*****************************************************************************
*                                                                            *
*  Copyright (C) 2001,  F. Hoffmann-La Roche & Co., AG, Basel, Switzerland.  *
*                                                                            *
* This file is part of "Roche Bioinformatics Software Objects and Services"  *
*    written by the bioinformatics group at Hoffmann-La Roche, Basel.        *
*      It shall not be reproduced or copied or disclosed to others           *
*                      without written permission.                           *
*                                                                            *
*                          - All rights reserved -                           *
*                                                                            *
*****************************************************************************/

/* 
file: plabla_conf.h
select platform to compile for

(PLatform ABstraction LAyer)
started: 2000-09-06  wolfd
*/

#define BIOS_PLATFORM_IRIX      1
#define BIOS_PLATFORM_SOLARIS   2
#define BIOS_PLATFORM_WINNT     3
#define BIOS_PLATFORM_LINUX     4

#define BIOS_PLATFORM     BIOS_PLATFORM_LINUX

/* number of bits in an integer variable; currently 32 and 64 are supported */
#define BIOS_BITS_PER_INT 64


