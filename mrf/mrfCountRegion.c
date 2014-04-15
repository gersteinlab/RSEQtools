#include "log.h"
#include "format.h"
#include "numUtil.h"
#include "mrf.h"



/** 
 *   \file mrfCountRegion.c Module to count the total number of reads that overlap with a specified region.
 *         Usage:  mrfCountRegion targetName:targetStart:targetEnd \n
 *         Takes MRF from STDIN. \n
 */



static int isContained (MrfRead *currRead, char *targetName, int targetStart, int targetEnd)
{
  MrfBlock* currBlock;
  int i;

  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    if (strEqual (currBlock->targetName,targetName)) {
      if (rangeIntersection (currBlock->targetStart,currBlock->targetEnd,targetStart,targetEnd) > 0 ) {
        return 1;
      }     
    }
  }
  return 0;
}



static int processEntry (MrfEntry *currEntry, char *targetName, int targetStart, int targetEnd) 
{
  int containment;

  containment = 0;
  containment += isContained (&currEntry->read1,targetName,targetStart,targetEnd);
  if (currEntry->isPairedEnd) {
    containment += isContained (&currEntry->read2,targetName,targetStart,targetEnd);
  }
  return containment;
}



int main (int argc, char *argv[])
{
  MrfEntry *currEntry;
  char *targetName;
  int targetStart,targetEnd;
  WordIter w;
  int count=0;
  int currCount=0; 

  if (argc != 2) {
    usage ("%s <targetName:targetStart-targetEnd>",argv[0]);
  }

  w = wordIterCreate (argv[1],":- ",0);
  targetName = hlr_strdup (wordNext (w));
  targetStart = atoi (wordNext (w));
  targetEnd = atoi (wordNext (w));
  wordIterDestroy (w);

  mrf_init ("-");
  while (currEntry = mrf_nextEntry ()) {
    currCount = processEntry (currEntry,targetName,targetStart,targetEnd);
    count = currCount+count;
  }
  printf("Count for %s:%d-%d = %d\n", targetName, targetStart, targetEnd, count);
  mrf_deInit ();
  hlr_free (targetName);
  return 0;
}
