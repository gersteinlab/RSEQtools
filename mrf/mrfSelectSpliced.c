#include "log.h"
#include "format.h"
#include "mrf.h"



/** 
 *   \file mrfSelectSpliced.c Module to select a subset of reads that are splice (spanning multiple exons).
 *         Takes MRF from STDIN. \n
 */



static void processEntry (MrfEntry *currEntry) 
{
  int isSpliced;

  isSpliced = 0;
  if (arrayMax (currEntry->read1.blocks) > 1) {
    isSpliced = 1;
  }
  if (currEntry->isPairedEnd) {
    if (arrayMax (currEntry->read2.blocks) > 1) {
      isSpliced = 1;
    }
  }
  if (isSpliced != 0) {
    puts (mrf_writeEntry (currEntry));
  }
}



int main (int argc, char *argv[])
{
  MrfEntry *currEntry;
 
  mrf_init ("-");
  puts (mrf_writeHeader ());
  while (currEntry = mrf_nextEntry ()) {
    processEntry (currEntry);
  }
  mrf_deInit ();
  return 0;
}
