#include "log.h"
#include "format.h"
#include "numUtil.h"


/** 
 *   \file numUtil.c Numeric utilities
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



typedef struct {
  double value1;
  double value2;
  double rank1;
  double rank2;
} ValuePair;




/** 
 * Returns rounded a*p/q 
 */
int roundingScale(int a, int p, int q)
{
  if (a > 100000 || p > 100000)
    {
      double x = a;
      x *= p;
      x /= q;
      return round(x);
    }
  else
    return (a*p + q/2)/q;
}



/** 
 * Return amount of bases two ranges intersect over, 0 or negative if no intersection. 
 */
int  rangeIntersection(int start1, int end1, int start2, int end2)
{
  int s = MAX(start1,start2);
  int e = MIN(end1,end2);
  return e-s;
}



/** 
 * Return number of bases in intersection of two ranges, or zero if they don't intersect. 
 */
int positiveRangeIntersection(int start1, int end1, int start2, int end2)

{
  int ret = rangeIntersection(start1,end1,start2,end2);
  if (ret < 0)
    ret = 0;
  return ret;
}



/** 
 * Return byte-swapped version of a. 
 */
unsigned byteSwap32(unsigned a)
{
  union {unsigned whole; unsigned char bytes[4];} u,v;
  u.whole = a;
  v.bytes[0] = u.bytes[3];
  v.bytes[1] = u.bytes[2];
  v.bytes[2] = u.bytes[1];
  v.bytes[3] = u.bytes[0];
  return v.whole;
}



/**
 * Return base two number of digits. 
 */
int digitsBaseTwo(unsigned long x)
{
  int digits = 0;
  while (x)
    {
      digits += 1;
      x >>= 1;
    }
  return digits;
}



/**
 * Return number of digits base 10. 
 */
int digitsBaseTen(int x)
{
  int digCount = 1;
  if (x < 0)
    {
      digCount = 2;
      x = -x;
    }
  while (x >= 10)
    {
      digCount += 1;
      x /= 10;
    }
  return digCount;
}



static int sortValuePairsByValue1 (ValuePair *a, ValuePair *b) 
{
  if (a->value1 < b->value1) {
    return -1;
  }
  if (a->value1 > b->value1) {
    return 1;
  }
  return 0;
}



static int sortValuePairsByValue2 (ValuePair *a, ValuePair *b) 
{
  if (a->value2 < b->value2) {
    return -1;
  }
  if (a->value2 > b->value2) {
    return 1;
  }
  return 0;
}



static double calculateSumTerm (Array ties) 
{
  int i;
  double sumTerm = 0.0;

  for (i = 0; i < arrayMax (ties); i++) {
    sumTerm += (pow (arru (ties,i,int),3) - arru (ties,i,int));
  }
  return sumTerm;
}



static double calculateC1 (Array ties1, Array ties2, int N)
{
  double sumTerm1,sumTerm2;

  if (arrayMax (ties1) == 0 && arrayMax (ties2) == 0) {
    return 0.0;
  }
  sumTerm1 = calculateSumTerm (ties1);
  sumTerm2 = calculateSumTerm (ties2);
  return (sumTerm1 + sumTerm2) / (2 * N * ((N * N) - 1));
}



static double calculateC2 (Array ties1, Array ties2, int N)
{
  double sumTerm1,sumTerm2;
  double term1,term2;

  if (arrayMax (ties1) == 0 && arrayMax (ties2) == 0) {
    return 1.0;
  }
  sumTerm1 = calculateSumTerm (ties1);
  sumTerm2 = calculateSumTerm (ties2);
  term1 = 1.0 - (sumTerm1 / (N * ((N * N) - 1)));
  term2 = 1.0 - (sumTerm2 / (N * ((N * N) - 1)));
  return sqrt (term1 * term2);
}



double spearmanCorrelation (Array a, Array b)
{
  Array valuePairs;
  ValuePair *currVP,*nextVP;
  int i,j,k;
  double sumSquaredRankDifferentials;
  double rankDifferential; 
  int N;
  Array ties1,ties2;
  int tieCount;
  double C1,C2;
  double sumRanks;

  if (arrayMax (a) != arrayMax (b)) {
    die ("Expected the same number of elements for the two arrays");
  }
  N = arrayMax (a);
  valuePairs = arrayCreate (N,ValuePair);
  for (i = 0; i < N; i++) {
    currVP = arrayp (valuePairs,arrayMax (valuePairs),ValuePair);
    currVP->value1 = arru (a,i,double);
    currVP->value2 = arru (b,i,double);
  }
  arraySort (valuePairs,(ARRAYORDERF)sortValuePairsByValue1);
  ties1 = arrayCreate (N,int);
  i = 0; 
  while (i < arrayMax (valuePairs)) {
    currVP = arrp (valuePairs,i,ValuePair);
    sumRanks = i + 1;
    tieCount = 1;
    j = i + 1;
    while (j < arrayMax (valuePairs)) {
      nextVP = arrp (valuePairs,j,ValuePair);
      if (currVP->value1 == nextVP->value1) {
        tieCount++;
      }
      else {
        break;
      }
      sumRanks += j + 1;
      j++;
    }
    if (tieCount > 1) {
      array (ties1,arrayMax (ties1),int) = tieCount;
    }
    for (k = i; k < i + tieCount; k++) {
      currVP = arrp (valuePairs,k,ValuePair);
      currVP->rank1 = sumRanks / tieCount;
    }
    i = j;
  }
  arraySort (valuePairs,(ARRAYORDERF)sortValuePairsByValue2);
  ties2 = arrayCreate (N,int);
  i = 0; 
  while (i < arrayMax (valuePairs)) {
    currVP = arrp (valuePairs,i,ValuePair);
    sumRanks = i + 1;
    tieCount = 1;
    j = i + 1;
    while (j < arrayMax (valuePairs)) {
      nextVP = arrp (valuePairs,j,ValuePair);
      if (currVP->value2 == nextVP->value2) {
        tieCount++;
      }
      else {
        break;
      }
      sumRanks += j + 1;
      j++;
    }
    if (tieCount > 1) {
      array (ties2,arrayMax (ties2),int) = tieCount;
    }
    for (k = i; k < i + tieCount; k++) {
      currVP = arrp (valuePairs,k,ValuePair);
      currVP->rank2 = sumRanks / tieCount;
    }
    i = j;
  }
  C1 = calculateC1 (ties1,ties2,N);
  C2 = calculateC2 (ties1,ties2,N);
  sumSquaredRankDifferentials = 0;
  for (i = 0; i < arrayMax (valuePairs); i++) {
    currVP = arrp (valuePairs,i,ValuePair);
    rankDifferential = currVP->rank1 - currVP->rank2;
    sumSquaredRankDifferentials += (rankDifferential * rankDifferential);
  }
  arrayDestroy (valuePairs);
  arrayDestroy (ties1);
  arrayDestroy (ties2);  
  return ((1.0 - ((6.0 * sumSquaredRankDifferentials) / (N * ((N * N) - 1)))) - C1) / C2;
}




GraphCoordTrans gr_ct_create (double minU, double maxU, int minP, int maxP)
{ /* creates a coordinate transformer
     input: minU,maxU : user coordinates
            minP,maxP : pixel coordinates
     output: -
  */
  GraphCoordTrans this1;

  this1 = (GraphCoordTrans) hlr_malloc (sizeof (struct _coordTransStruct_));
  this1->minU = minU;
  this1->maxU = maxU;
  this1->minP = minP;
  this1->maxP = maxP;
  return this1;
}



void gr_ct_destroy_func (GraphCoordTrans this1)
{
  if (!this1)
    die ("gr_ct_destroy_func: no GraphCoordTrans");
  hlr_free (this1);
}



int gr_ct_toPix (GraphCoordTrans ct,double x)
{ /* returns pixel coordinate for the user coordinate x
  */
  if (!ct)
    die ("gr_ct_toPix: no GraphCoordTrans");
  return ct->minP + (x - ct->minU) * (ct->maxP - ct->minP) / (ct->maxU - ct->minU);
}



double gr_ct_toUser (GraphCoordTrans ct,int x)
{ /* returns user coordinate for the pixel coordinate
  */
  if (!ct)
    die ("gr_ct_toUser: no GraphCoordTrans");
  return ct->minU + (x - ct->minP) * (ct->maxU - ct->minU) / (ct->maxP - ct->minP);
}
