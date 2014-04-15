/* 
 * bits - Handle operations on arrays of bits. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. 
 */


/** 
 *   \file bits.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#ifndef DEF_BITS_H
#define DEF_BITS_H

typedef unsigned char Bits;
extern int bitsInByte[256];

Bits *bitAlloc (int bitCount);
Bits *bitRealloc (Bits *b, int bitCount, int newBitCount);
Bits *bitClone (Bits* orig, int bitCount);
void bitFree (Bits **pB);
void bitSetOne (Bits *b, int bitIx);
void bitClearOne (Bits *b, int bitIx);
void bitSetRange (Bits *b, int startIx, int bitCount);
int bitReadOne (Bits *b, int bitIx);
int bitCountRange (Bits *b, int startIx, int bitCount);
int bitFindSet (Bits *b, int startIx, int bitCount);
int bitFindClear (Bits *b, int startIx, int bitCount);
void bitClear (Bits *b, int bitCount);
void bitClearRange (Bits *b, int startIx, int bitCount);
void bitAnd (Bits *a, Bits *b, int bitCount);
void bitOr (Bits *a, Bits *b, int bitCount);
void bitXor (Bits *a, Bits *b, int bitCount);
void bitNot (Bits *a, int bitCount);
void bitPrint (Bits *a, int startIx, int bitCount);
void bitsInByteInit ();


#endif

