#include "log.h"
#include "format.h"
#include "numUtil.h"
#include "intervalFind.h"
#include "mrf.h"



/** 
 *   \file mrfSelectAnnotated.c Module to select a subset of reads that overlap with a specified annotation set.
 *         Usage:  mrfSelectAnnotated <file.annotation> <include|exclude> \n
 *         Takes MRF from STDIN. \n
 */



#define MODE_INCLUDE 1
#define MODE_EXCLUDE 2



static int isContained (MrfRead *currRead)
{
  MrfBlock* currBlock;
  Array annotatedTranscripts;
  Interval *currTranscript;
  SubInterval *currExon;
  int overlap;
  int i,j,k;

  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    annotatedTranscripts = intervalFind_getOverlappingIntervals (currBlock->targetName,currBlock->targetStart,currBlock->targetEnd);
    for (j = 0; j < arrayMax (annotatedTranscripts); j++) {
      currTranscript = arru (annotatedTranscripts,j,Interval*);
      for (k = 0; k < arrayMax (currTranscript->subIntervals); k++) {
        currExon = arrp (currTranscript->subIntervals,k,SubInterval);
        overlap = rangeIntersection (currBlock->targetStart,currBlock->targetEnd,currExon->start,currExon->end);
        if (overlap > 0) {
          return 1;
        }
      } 
    }
  }
  return 0;
}



static void processEntry (MrfEntry *currEntry, int mode) 
{
  int containment;

  containment = 0;
  containment += isContained (&currEntry->read1);
  if (currEntry->isPairedEnd) {
    containment += isContained (&currEntry->read2);
  }
  if ((containment != 0 && mode == MODE_INCLUDE) ||
      (containment == 0 && mode == MODE_EXCLUDE)) {
    puts (mrf_writeEntry (currEntry));
  }
}



int main (int argc, char *argv[])
{
  MrfEntry *currEntry;
  int mode;
 
  if (argc != 3) {
    usage ("%s <file.annotation> <include|exclude>",argv[0]);
  }
  intervalFind_addIntervalsToSearchSpace (argv[1],0);
  if (strEqual (argv[2],"include")) {
    mode = MODE_INCLUDE;
  }
  else if (strEqual (argv[2],"exclude")) {
    mode = MODE_EXCLUDE;
  }
  else {
     usage ("%s <file.annotation> <include|exclude>",argv[0]);
  }

  mrf_init ("-");
  puts (mrf_writeHeader ());
  while (currEntry = mrf_nextEntry ()) {
    processEntry (currEntry,mode);
  }
  mrf_deInit ();
  return 0;
}
