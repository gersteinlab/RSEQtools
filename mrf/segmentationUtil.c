#include "log.h"
#include "format.h"
#include "segmentationUtil.h"



/**
 *   \file segmentationUtil.c Segmentation utilities.
 */



void performSegmentation (Array tars, Array wigs, char* targetName, double threshold, int maxGap, int minRun)
{
  Tar *currTar;
  Wig *currWig,*nextWig;
  int i,j,endPosition;
  int countBelowThreshold;

  i = 0; 
  while (i < arrayMax (wigs)) {
    currWig = arrp (wigs,i,Wig);
    if (currWig->value < threshold) {
      i++;
      continue;
    }
    j = i + 1;
    endPosition = j;
    countBelowThreshold = 0;
    while (j < arrayMax (wigs)) {
      nextWig = arrp (wigs,j,Wig);
      if (nextWig->value < threshold) {
        countBelowThreshold++;
        if (countBelowThreshold >= maxGap) {
          break;
        }
      }
      else {
        countBelowThreshold = 0;
        endPosition = j;
      }
      j++;
    }
    if ((endPosition - 1 - currWig->position + 1) >= minRun) {
      currTar = arrayp (tars,arrayMax (tars),Tar);
      currTar->start = currWig->position;
      currTar->end = endPosition + 1;
      currTar->targetName = hlr_strdup (targetName);
     }
    i = j;
  }
}

