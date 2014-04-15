#include <stdlib.h>
#include <time.h>
#include "log.h"
#include "format.h"
#include "intervalFind.h"
#include "numUtil.h"
#include "mrf.h"



/** 
 *   \file mrfAnnotationCoverage.c Module to calculate annotation coverage.
 *         Usage: mrfAnnotationCoverage <file.annotation> <numTotalReads> <numReadsToSample> <coverageFactor> \n
 *         Sample a set of reads from MRF and determine the fraction of transcripts (specified in file.annotation) that have at least <coverageFactor>-times uniform coverage. \n
 *         Takes MRF from STDIN. \n
 */



typedef struct {
  Interval *intervalPtr;
  int overlap;
} Match;



static int sortMatches (Match *a, Match *b)
{
  return a->intervalPtr - b->intervalPtr;
}



static double calculateCoverage (Array matches, double coverageFactor)
{
  double ratio;
  int i,j,k;
  int totalIntervalLength;
  int totalOverlap;
  Match *currMatch,*nextMatch;
  Interval *currInterval;
  SubInterval *currSubInterval;
  int numIntervalsAboveCoverageFactor;
  
  arraySort (matches,(ARRAYORDERF)sortMatches);
  numIntervalsAboveCoverageFactor = 0;
  i = 0;
  while (i < arrayMax (matches)) {
    currMatch = arrp (matches,i,Match);
    currInterval = currMatch->intervalPtr;
    totalIntervalLength = 0;
    for (k = 0; k < arrayMax (currInterval->subIntervals); k++) {
      currSubInterval = arrp (currInterval->subIntervals,k,SubInterval);
      totalIntervalLength += (currSubInterval->end - currSubInterval->start + 1);
    } 
    totalOverlap = currMatch->overlap;
    j = i + 1;
    while (j < arrayMax (matches)) {
      nextMatch = arrp (matches,j,Match);
      if (currMatch->intervalPtr == nextMatch->intervalPtr) {
        totalOverlap += nextMatch->overlap;
      }
      else {
        break;
      }
      j++;
    }
    ratio = 1.0 * totalOverlap / totalIntervalLength;
    if (ratio >= coverageFactor) {
      numIntervalsAboveCoverageFactor++;
    }
    i = j;
  }
  return numIntervalsAboveCoverageFactor * 1.0 / intervalFind_getNumberOfIntervals ();
}



static void intersectWithAnnotation (Array matches, char *chromosome, int start, int end) 
{
  Array annotatedTranscripts;
  Interval *currTranscript;
  SubInterval *currExon;
  int overlap;
  Match *currMatch;
  int i,j;

  annotatedTranscripts = intervalFind_getOverlappingIntervals (chromosome,start,end);
  for (i = 0; i < arrayMax (annotatedTranscripts); i++) {
    currTranscript = arru (annotatedTranscripts,i,Interval*);
    for (j = 0; j < arrayMax (currTranscript->subIntervals); j++) {
      currExon = arrp (currTranscript->subIntervals,j,SubInterval);
      overlap = rangeIntersection (start,end,currExon->start,currExon->end);
      if (overlap > 0) {
        currMatch = arrayp (matches,arrayMax (matches),Match);
        currMatch->intervalPtr = currTranscript;
        currMatch->overlap = overlap;
      }
    } 
  }
}



static void processRead (Array matches, MrfRead *currRead) 
{
  MrfBlock *currBlock;
  int i;
  
  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    intersectWithAnnotation (matches,currBlock->targetName,
                             currBlock->targetStart,currBlock->targetEnd);
  }
}



int main (int argc, char *argv[])
{
  int numTotalReads,numReadsToSample;
  float fractionToSample;
  MrfEntry *currEntry;
  Array matches;
  double fractionDetected;
  int readsSampled;
  double coverageFactor;

  if (argc != 5) {
    usage ("%s <file.annotation> <numTotalReads> <numReadsToSample> <coverageFactor>",argv[0]);
  }
  intervalFind_addIntervalsToSearchSpace (argv[1],0);
  srand (time (0));
  numTotalReads = atoi (argv[2]);
  numReadsToSample = atoi (argv[3]);
  coverageFactor = atof (argv[4]);
  fractionToSample = 1.0 * numReadsToSample / numTotalReads;
  readsSampled = 0;
  matches = arrayCreate (300000,Match);
  mrf_init ("-"); 
  while (currEntry = mrf_nextEntry ()) {
    if ((1.0 * rand () / RAND_MAX) > fractionToSample) {
      continue;
    }  
    readsSampled++;
    processRead (matches,&currEntry->read1);
    if (currEntry->isPairedEnd) {
      processRead (matches,&currEntry->read2);
    }
  }
  mrf_deInit ();
  fractionDetected = calculateCoverage (matches,coverageFactor);
  puts ("fractionDetected\tnumTotalReads\tnumReadsToSample\tnumReadsSampled\trequiredCoverage");
  printf ("%.3f\t%s\t%s\t%d\t%.1fx\n",fractionDetected,argv[2],argv[3],readsSampled,coverageFactor);
  return 0;
}
