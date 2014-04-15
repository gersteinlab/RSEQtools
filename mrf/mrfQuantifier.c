#include "log.h"
#include "format.h"
#include "numUtil.h"
#include "common.h"
#include "intervalFind.h"
#include "mrf.h"



/** 
 *   \file mrfQuantifier.c Module to calculate gene expression values.
 *         Calculates RPKM values for a set of transcripts (specified in file.annotation). \n
 *         \n
 *         Usage: mrfQuantifier <file.annotation> <singleOverlap|multipleOverlap> \n
 *         file.annotation: annotation set in Interval format. \n
 *         singleOverlap: reads that overlap with multiple annotated features are ignored. \n 
 *         multipleOverlap: reads that overlap with multiple annotated features are counted multiple times. \n 
 *         Takes MRF from stdin. \n
 */



#define MODE_SINGLE_OVERLAP 1
#define MODE_MULTIPLE_OVERLAP 2



typedef struct {
  Interval *transcript;
  int overlap;
} TranscriptEntry;



static int sortTranscriptEntriesByTranscriptPointer (TranscriptEntry *a, TranscriptEntry *b)
{
  return a->transcript - b->transcript;
}



static int sortTranscriptEntriesByTranscriptName (TranscriptEntry *a, TranscriptEntry *b)
{
  return strcmp (a->transcript->name,b->transcript->name);
}



static int sortTranscriptsByName (Interval *a, Interval *b)
{
  return strcmp (a->name,b->name);
}



static int sortTranscriptsPointers (Interval **a, Interval **b)
{
  return *a - *b;
}



static void addOverlap (Array transcriptEntries, Interval *currTranscript, int overlap) 
{
  TranscriptEntry *currTranscriptEntry,testTranscriptEntry;
  int index;

  testTranscriptEntry.transcript = currTranscript;
  if (arrayFind (transcriptEntries,&testTranscriptEntry,&index,(ARRAYORDERF)sortTranscriptEntriesByTranscriptPointer)) {
    currTranscriptEntry = arrp (transcriptEntries,index,TranscriptEntry);
    currTranscriptEntry->overlap += overlap;
  }
  else {
    die ("Expected to find transcript pointer");
  }
}



static void intersectWithAnnotationSingleOverlapMode (Array transcriptEntries, char *chromosome, int start, int end) 
{
  Array annotatedTranscripts;
  Interval *currTranscript,*thisTranscript;
  SubInterval *currExon;
  int overlap;
  int i,j;
  int numOverlappingTranscripts;
  int overlapFound;

  annotatedTranscripts = intervalFind_getOverlappingIntervals (chromosome,start,end);
  if (arrayMax (annotatedTranscripts) > 1) {
    numOverlappingTranscripts = 0;
    for (i = 0; i < arrayMax (annotatedTranscripts); i++) {
      currTranscript = arru (annotatedTranscripts,i,Interval*);
      j = 0; 
      overlapFound = 0;
      while (j < arrayMax (currTranscript->subIntervals)) {
        currExon = arrp (currTranscript->subIntervals,j,SubInterval);
        if (rangeIntersection (start,end,currExon->start,currExon->end) > 0) {
          overlapFound = 1;
          break;
        }
        j++;
      }
      if (overlapFound == 1) {
        numOverlappingTranscripts++;
        thisTranscript = currTranscript;
      }
    }
    if (numOverlappingTranscripts != 1) {
      return;
    }
  }
  else if (arrayMax (annotatedTranscripts) == 1) {
    thisTranscript = arru (annotatedTranscripts,0,Interval*);
  }
  else {
    return;
  }
  for (i = 0; i < arrayMax (thisTranscript->subIntervals); i++) {
    currExon = arrp (thisTranscript->subIntervals,i,SubInterval);
    overlap = rangeIntersection (start,end,currExon->start,currExon->end);
    if (overlap > 0) {
      addOverlap (transcriptEntries,thisTranscript,overlap); 
    }
  } 
}



static void intersectWithAnnotationMultipleOverlapMode (Array transcriptEntries, char *chromosome, int start, int end) 
{
  Array annotatedTranscripts;
  Interval *currTranscript;
  SubInterval *currExon;
  int overlap;
  int i,j;

  annotatedTranscripts = intervalFind_getOverlappingIntervals (chromosome,start,end);
  for (i = 0; i < arrayMax (annotatedTranscripts); i++) {
    currTranscript = arru (annotatedTranscripts,i,Interval*);
    for (j = 0; j < arrayMax (currTranscript->subIntervals); j++) {
      currExon = arrp (currTranscript->subIntervals,j,SubInterval);
      overlap = rangeIntersection (start,end,currExon->start,currExon->end);
      if (overlap > 0) {
        addOverlap (transcriptEntries,currTranscript,overlap); 
      }
    } 
  }
}



static void processRead (Array transcriptEntries, MrfRead *currRead, int mode) 
{
  MrfBlock *currBlock;
  int i;

  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    if (mode == MODE_SINGLE_OVERLAP) {
      intersectWithAnnotationSingleOverlapMode (transcriptEntries,currBlock->targetName,
                                                currBlock->targetStart-1,currBlock->targetEnd); // Interval: zero-based; MRF: 1-based
    }
    else if (mode == MODE_MULTIPLE_OVERLAP) {
      intersectWithAnnotationMultipleOverlapMode (transcriptEntries,currBlock->targetName,
                                                  currBlock->targetStart-1,currBlock->targetEnd);// Interval: zero-based; MRF: 1-based
    }
  }
}


  
int main (int argc, char *argv[])
{
  MrfEntry *currMRF;
  Array intervals;
  Array intervalPointers;
  Interval *currTranscript;
  SubInterval *currExon;
  int i,j;
  Array transcriptEntries;
  TranscriptEntry *currTranscriptEntry,testTranscriptEntry;
  int transcriptLength;
  int numMrfEntries;
  double factor;
  int index;
  int mode;
  long int totalNumNucleotides; 

  if (argc != 3) {
    usage ("%s <file.annotation> <singleOverlap|multipleOverlap>",argv[0]);
  }
  if (strEqual (argv[2],"singleOverlap")) {
    mode = MODE_SINGLE_OVERLAP;
  }
  else if (strEqual (argv[2],"multipleOverlap")) {
    mode = MODE_MULTIPLE_OVERLAP;
  }
  else {
    usage ("%s <file.annotation> <singleOverlap|multipleOverlap>",argv[0]);
  }
  intervalFind_addIntervalsToSearchSpace (argv[1],0);
  intervalPointers = intervalFind_getIntervalPointers ();
  arraySort (intervalPointers,(ARRAYORDERF)sortTranscriptsPointers);
  intervals = intervalFind_getAllIntervals ();  
  arraySort (intervals,(ARRAYORDERF)sortTranscriptsByName);
  transcriptEntries = arrayCreate (1000000,TranscriptEntry);
  for (i = 0; i < arrayMax (intervalPointers); i++) {
    currTranscriptEntry = arrayp (transcriptEntries,arrayMax (transcriptEntries),TranscriptEntry);
    currTranscriptEntry->transcript = arru (intervalPointers,i,Interval*);
    currTranscriptEntry->overlap = 0;
  }
  arraySort (transcriptEntries,(ARRAYORDERF)sortTranscriptEntriesByTranscriptPointer);
  numMrfEntries = 0;
  totalNumNucleotides = 0;
  mrf_init ("-");
  while (currMRF = mrf_nextEntry ()) {
    numMrfEntries++;
    processRead (transcriptEntries,&currMRF->read1,mode);
    totalNumNucleotides += getReadLength (&currMRF->read1);   
    if (currMRF->isPairedEnd) {
      processRead (transcriptEntries,&currMRF->read2,mode);
      totalNumNucleotides += getReadLength (&currMRF->read2);
    }
    if ((numMrfEntries % 1000000) == 0) {
      warn ("Processed %d MrfEntries...",numMrfEntries);
    }
  }
  warn ("Processed %d MrfEntries...",numMrfEntries);
  warn ("Number of mapped nucleotides: %ld", totalNumNucleotides );
  mrf_deInit ();
  factor = (double)totalNumNucleotides / 1000000; 
  arraySort (transcriptEntries,(ARRAYORDERF)sortTranscriptEntriesByTranscriptName);
  for (i = 0; i < arrayMax (intervals); i++) {
    currTranscript = arrp (intervals,i,Interval);
    transcriptLength = 0;
    for (j = 0; j < arrayMax (currTranscript->subIntervals); j++) {
      currExon = arrp (currTranscript->subIntervals,j,SubInterval);
      transcriptLength += currExon->end - currExon->start; // Interval: zero-based, half open
    }
    AllocVar (testTranscriptEntry.transcript);
    testTranscriptEntry.transcript->name = hlr_strdup (currTranscript->name);
    if (arrayFind (transcriptEntries,&testTranscriptEntry,&index,(ARRAYORDERF)sortTranscriptEntriesByTranscriptName)) {
      currTranscriptEntry = arrp (transcriptEntries,index,TranscriptEntry);
      printf ("%s\t%f\n",currTranscriptEntry->transcript->name,currTranscriptEntry->overlap / (transcriptLength * factor) * 1000.0);
    }
    else {
      die ("Expected to find transcript name");
    }
    hlr_free (testTranscriptEntry.transcript->name);
    freeMem (testTranscriptEntry.transcript);
  }
  return 0;
}
