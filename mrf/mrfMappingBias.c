#include "format.h"
#include "log.h"
#include "mrf.h"
#include "intervalFind.h"



/** 
 *   \file mrfMappingBias.c Module to calculate mapping bias for a given annotation set.
 *         Aggregates mapped reads that overlap with transcripts (specified in file.annotation) and \n 
 *         outputs the counts over a standardized transcript where 0 represents the 5' end of the transcript and \n
 *         1 denotes the 3' end of the transcripts. This analysis is done in a strand specific way. \n
 *         \n
 *         Usage: mrfMappingBias <file.annotation> \n
 *         Takes MRF from STDIN. \n
 */



#define NUM_BINS 100



static void collectAlignmentBlocks (Array blocks, MrfRead *currRead) 
{
  int i;

  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    array (blocks,arrayMax (blocks),MrfBlock*) = arrp (currRead->blocks,i,MrfBlock);
  }
}



int main (int argc, char *argv[])
{
  MrfEntry *currEntry;
  MrfBlock *currBlock;
  Array blocks;
  Array intervals;
  Interval *currInterval;
  SubInterval *currSubInterval;
  int h,i,j,k;
  int count;
  int exonBaseCount;
  int relativeStart,relativeEnd;
  int foundRelativeStart;
  int normalizedCounts[NUM_BINS];
  double normalizedValue,normalizedRelativeStart,normalizedRelativeEnd;

  if (argc != 2) {
    usage ("%s <file.annotation>",argv[0]);
  }
  
  intervalFind_addIntervalsToSearchSpace (argv[1],0);
  for (k = 0; k < NUMELE (normalizedCounts); k++) {
    normalizedCounts[k] = 0;
  } 
  blocks = arrayCreate (10,MrfBlock*);
  mrf_init ("-");
  while (currEntry = mrf_nextEntry ()) {
    arrayClear (blocks);
    collectAlignmentBlocks (blocks,&currEntry->read1);
    if (currEntry->isPairedEnd) {
      collectAlignmentBlocks (blocks,&currEntry->read2);
    }
    for (h = 0; h < arrayMax (blocks); h++) {
      currBlock = arru (blocks,h,MrfBlock*);
      intervals = intervalFind_getOverlappingIntervals (currBlock->targetName,currBlock->targetStart,currBlock->targetEnd);
      count = 0;
      for (i = 0; i < arrayMax (intervals); i++) {
        currInterval = arru (intervals,i,Interval*);
        if (currInterval->strand == currBlock->strand) {
          count++;
        }
      }
      if (count != 1) {
        continue;
      }
      exonBaseCount = 0;
      foundRelativeStart = 0;
      currInterval = arru (intervals,0,Interval*);
      for (j = 0; j < arrayMax (currInterval->subIntervals); j++) {
        currSubInterval = arrp (currInterval->subIntervals,j,SubInterval);
        for (k = currSubInterval->start; k <= currSubInterval->end; k++) {
          exonBaseCount++;
          if (currBlock->targetStart <= k && foundRelativeStart == 0) {
            relativeStart = exonBaseCount;
            foundRelativeStart = 1;
          }
          if (k <= currBlock->targetEnd) {
            relativeEnd = exonBaseCount;
          }
        }
      }
      if (relativeEnd <= relativeStart) {
        // query transcript overlaps with an intronic region of the annotated transcript
        continue;
      }
      if (currBlock->strand == '+') {
        normalizedRelativeStart = (double)relativeStart / exonBaseCount;
        normalizedRelativeEnd = (double)relativeEnd / exonBaseCount;
      }
      else if (currBlock->strand == '-') {
        normalizedRelativeStart = 1 - (double)relativeEnd / exonBaseCount;
        normalizedRelativeEnd = 1 - (double)relativeStart / exonBaseCount;
      }
      else {
        continue; // if currBlock->strand == '.', ambiguous
      }
      for (k = 0; k < NUMELE (normalizedCounts); k++) {
        normalizedValue = (double)k / NUM_BINS;
        if (normalizedValue >= normalizedRelativeStart && normalizedValue <= normalizedRelativeEnd) {
          normalizedCounts[k]++;
        }
      }
    }
  }
  for (k = 0; k < NUMELE(normalizedCounts); k++) {
    printf ("%f\t%d\n",(double)k / NUM_BINS,normalizedCounts[k]);
  }
  mrf_deInit ();
  return 0;
}
