#include "log.h"
#include "format.h"
#include "numUtil.h"
#include "mrf.h"



/** 
 *   \file mrfSelectRegion.c Module to select a subset of reads that overlap with a specified region.
 *         Usage:  mrfSelectRegion targetName:targetStart:targetEnd \n
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



static void processEntry (MrfEntry *currEntry, char *targetName, int targetStart, int targetEnd) 
{
  int containment;

  containment = 0;
  containment += isContained (&currEntry->read1,targetName,targetStart,targetEnd);
  if (currEntry->isPairedEnd) {
    containment += isContained (&currEntry->read2,targetName,targetStart,targetEnd);
  }
  if (containment != 0) {
    puts (mrf_writeEntry (currEntry));
  }
}



int main (int argc, char *argv[])
{
  MrfEntry *currEntry;
  char *targetName;
  int targetStart,targetEnd;
  WordIter w;
 
  if (argc != 2) {
    usage ("%s <targetName:targetStart-targetEnd>",argv[0]);
  }
  w = wordIterCreate (argv[1],":- ",0);
  targetName = hlr_strdup (wordNext (w));
  targetStart = atoi (wordNext (w));
  targetEnd = atoi (wordNext (w));
  wordIterDestroy (w);

  mrf_init ("-");
  puts (mrf_writeHeader ());
  while (currEntry = mrf_nextEntry ()) {
    processEntry (currEntry,targetName,targetStart,targetEnd);
  }
  mrf_deInit ();
  hlr_free (targetName);
  return 0;
}
