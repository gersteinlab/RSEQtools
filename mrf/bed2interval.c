#include "format.h"
#include "log.h"
#include "mrfUtil.h"



/** 
 *   \file bed2interval.c Module to convert BED format into Interval format.
 *         Takes BED from STDIN \n
 *         Note: The Interval and BED formats are zero-based and half-open. \n
 */



int main (int argc, char *argv[]) 
{
  Array tars;
  Tar *currTar;
  int i;

  tars = readTarsFromBedFile ("-");
  for (i = 0; i < arrayMax (tars); i++) {
    currTar = arrp (tars,i,Tar);
    printf ("BED_%d\t%s\t.\t%d\t%d\t1\t%d\t%d\n",
            i + 1,currTar->targetName,currTar->start,currTar->end,currTar->start,currTar->end);
  }
  return 0;
}
