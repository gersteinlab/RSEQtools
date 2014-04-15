/* 
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. 
 */


/** 
 *   \file bits.c Handle operations on arrays of bits. 
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#include "common.h"
#include "bits.h"


static Bits oneBit[8] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
static Bits leftMask[8] = {0xFF, 0x7F, 0x3F, 0x1F,  0xF,  0x7,  0x3,  0x1,};
static Bits rightMask[8] = {0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF,};
static int inittedBitsInByte = 0;
int bitsInByte[256];


/** 
 * Initialize bitsInByte array. 
 */
void bitsInByteInit(void)
{
  int i;
  
  if (!inittedBitsInByte)
    {
      inittedBitsInByte = 1;
      for (i=0; i<256; ++i)
        {
	  int count = 0;
	  if (i&1)
	    count = 1;
	  if (i&2)
	    ++count;
	  if (i&4)
	    ++count;
	  if (i&8)
	    ++count;
	  if (i&0x10)
	    ++count;
	  if (i&0x20)
	    ++count;
	  if (i&0x40)
	    ++count;
	  if (i&0x80)
	    ++count;
	  bitsInByte[i] = count;
	}
    }
}



/** 
 * Allocate bits. 
 */
Bits *bitAlloc(int bitCount)
{
  int byteCount = ((bitCount+7)>>3);
  return needLargeZeroedMem(byteCount);
}



/** 
 * Resize a bit array. If b is null, allocate a new array 
 */
Bits *bitRealloc(Bits *b, int bitCount, int newBitCount)
{
  int byteCount = ((bitCount+7)>>3);
  int newByteCount = ((newBitCount+7)>>3);
  return needLargeZeroedMemResize(b, byteCount, newByteCount);
}



/**
 * Clone bits. 
 */
Bits *bitClone(Bits* orig, int bitCount)
{
  int byteCount = ((bitCount+7)>>3);
  Bits* bits = needLargeZeroedMem(byteCount);
  memcpy(bits, orig, byteCount);
  return bits;
}



/** 
 * Free bits. 
 */
void bitFree(Bits **pB)
{
  freez(pB);
}



/** 
 * Set a single bit. 
 */
void bitSetOne(Bits *b, int bitIx)
{
  b[bitIx>>3] |= oneBit[bitIx&7];
}



/** 
 * Clear a single bit. 
 */
void bitClearOne(Bits *b, int bitIx)
{
  b[bitIx>>3] &= ~oneBit[bitIx&7];
}



/** 
 * Set a range of bits. 
 */
void bitSetRange(Bits *b, int startIx, int bitCount)
{
  int endIx = (startIx + bitCount - 1);
  int startByte = (startIx>>3);
  int endByte = (endIx>>3);
  int startBits = (startIx&7);
  int endBits = (endIx&7);
  int i;
  
  if (startByte == endByte)
    {
      b[startByte] |= (leftMask[startBits] & rightMask[endBits]);
      return;
    }
  b[startByte] |= leftMask[startBits];
  for (i = startByte+1; i<endByte; ++i)
    b[i] = 0xff;
  b[endByte] |= rightMask[endBits];
}



/**
 * Read a single bit. 
 */
int bitReadOne(Bits *b, int bitIx)
{
  return (b[bitIx>>3] & oneBit[bitIx&7]) != 0;
}



/**
 * Count number of bits set in range. 
 */
int bitCountRange(Bits *b, int startIx, int bitCount)
{
  int endIx = (startIx + bitCount - 1);
  int startByte = (startIx>>3);
  int endByte = (endIx>>3);
  int startBits = (startIx&7);
  int endBits = (endIx&7);
  int i;
  int count = 0;
  
  if (!inittedBitsInByte)
    bitsInByteInit();
  if (startByte == endByte)
    return bitsInByte[b[startByte] & leftMask[startBits] & rightMask[endBits]];
  count = bitsInByte[b[startByte] & leftMask[startBits]];
  for (i = startByte+1; i<endByte; ++i)
    count += bitsInByte[b[i]];
  count += bitsInByte[b[endByte] & rightMask[endBits]];
  return count;
}




/* Find the index of the the next set bit. */
static int bitFind(Bits *b, int startIx, int val, int bitCount)
{
  unsigned char notByteVal = val ? 0 : 0xff;
  int iBit = startIx;
  int endByte = ((bitCount-1)>>3);
  int iByte;
  
  /* scan initial byte */
  while (((iBit & 7) != 0) && (iBit < bitCount))
    {
      if (bitReadOne(b, iBit) == val)
        return iBit;
      iBit++;
    }
  
  /* scan byte at a time, if not already in last byte */
  iByte = (iBit >> 3);
  if (iByte < endByte)
    {
      while ((iByte < endByte) && (b[iByte] == notByteVal))
        iByte++;
      iBit = iByte << 3;
    }
  
  /* scan last byte */
  while (iBit < bitCount)
    {
      if (bitReadOne(b, iBit) == val)
        return iBit;
      iBit++;
    }
  return bitCount;  /* not found */
}



/**
 * Find the index of the the next set bit. 
 */
int bitFindSet(Bits *b, int startIx, int bitCount)
{
  return bitFind(b, startIx, 1, bitCount);
}



/** 
 *Find the index of the the next clear bit. 
 */
int bitFindClear(Bits *b, int startIx, int bitCount)
{
  return bitFind(b, startIx, 0, bitCount);
}



/** 
 *Clear many bits (possibly up to 7 beyond bitCount). 
 */
void bitClear(Bits *b, int bitCount)
{
  int byteCount = ((bitCount+7)>>3);
  zeroBytes(b, byteCount);
}



/**
 * Clear a range of bits. 
 */
void bitClearRange(Bits *b, int startIx, int bitCount)
{
  int endIx = (startIx + bitCount - 1);
  int startByte = (startIx>>3);
  int endByte = (endIx>>3);
  int startBits = (startIx&7);
  int endBits = (endIx&7);
  int i;
  
  if (startByte == endByte)
    {
      b[startByte] &= ~(leftMask[startBits] & rightMask[endBits]);
      return;
    }
  b[startByte] &= ~leftMask[startBits];
  for (i = startByte+1; i<endByte; ++i)
    b[i] = 0x00;
  b[endByte] &= ~rightMask[endBits];
}



/**
 * And two bitmaps. Put result in a. 
 */
void bitAnd(Bits *a, Bits *b, int bitCount)
{
  int byteCount = ((bitCount+7)>>3);
  while (--byteCount >= 0)
    {
      *a = (*a & *b++);
      a++;
    }
}



/**
 * Or two bitmaps. Put result in a. 
 */
void bitOr(Bits *a, Bits *b, int bitCount)
{
  int byteCount = ((bitCount+7)>>3);
  while (--byteCount >= 0)
    {
      *a = (*a | *b++);
      a++;
    }
}



/**
 * Xor two bitmaps. Put result in a. 
 */
void bitXor(Bits *a, Bits *b, int bitCount)
{
  int byteCount = ((bitCount+7)>>3);
  while (--byteCount >= 0)
    {
      *a = (*a ^ *b++);
      a++;
    }
}



/** 
 * Flip all bits in a. 
 */
void bitNot(Bits *a, int bitCount)
{
  int byteCount = ((bitCount+7)>>3);
  while (--byteCount >= 0)
    {
      *a = ~*a;
      a++;
    }
}



/**
 * Print part or all of bit map as a string of 0s and 1s.  
 * Mostly useful for debugging 
 */
void bitPrint(Bits *a, int startIx, int bitCount)
{
  int i;
  for (i = startIx; i < bitCount; i++)
    {
      if (bitReadOne(a, i))
        putchar('1');
      else
        putchar('0');
    }
  putchar('\n');
}

