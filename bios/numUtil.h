#ifndef DEF_NUMUTIL_H
#define DEF_NUMUTIL_H


/** 
 *   \file numUtil.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */


#include <math.h>



/** 
 * Return log base two of number x. 
 */
#define logBase2(x)(log(x)/log(2))



/** 
 *  Round floating point value to nearest integer.
 */
#define round(a) ((int)((a)+0.5))



/** 
 * Round floating point value to nearest long long. 
 */
#define roundll(a) ((long long)((a)+0.5))



extern int roundingScale(int a, int p, int q);
extern int rangeIntersection(int start1, int end1, int start2, int end2);
extern int positiveRangeIntersection(int start1, int end1, int start2, int end2);
extern unsigned byteSwap32(unsigned a);
extern int digitsBaseTwo(unsigned long x);
extern int digitsBaseTen(int x);
extern double spearmanCorrelation (Array a, Array b);


typedef struct _coordTransStruct_ {
  double minU;
  double maxU;
  int minP;
  int maxP;
} *GraphCoordTrans;

extern GraphCoordTrans gr_ct_create (double minU, double maxU, int minP, int maxP);
extern void gr_ct_destroy_func (GraphCoordTrans this); /* do not use this function */
#define gr_ct_destroy(this) (gr_ct_destroy_func(this), this=NULL) /* use this one */
extern int gr_ct_toPix (GraphCoordTrans ct, double x);
extern double gr_ct_toUser (GraphCoordTrans ct, int x);


#endif
