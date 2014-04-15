#include "log.h"
#include "format.h"
#include "intervalFind.h"
#include "fasta.h"
#include <unistd.h>



/** 
 *   \file createSpliceJunctionLibrary.c Module to create a splice junction library.
 *         Usage: createSpliceJunctionLibrary <file.2bit> <file.annotation> <sizeExonOverlap> \n
 *         Creates all possible pairwise splice junctions within a transcript. \n
 *         The transcripts are specified in file.annotation. \n
 *         Uses file.2bit, which represents the genomic sequence, to extract the junction sequences. \n
 *         sizeExonOverlap defines the number of nucleotides included from each exon. \n
 *         Note: twoBitToFa (part of blat package) must be in the PATH. \n
 */



typedef struct {
  char* chromosome;
  int firstExonEnd;
  int secondExonStart;
} Junction;



static int sortJunctions (Junction *a, Junction *b)
{
  int diff;

  diff = strcmp (a->chromosome,b->chromosome);
  if (diff != 0) {
    return diff;
  }
  diff = a->firstExonEnd - b->firstExonEnd;
  if (diff != 0) {
    return diff;
  }
  return a->secondExonStart - b->secondExonStart;
}



int main (int argc, char *argv[])
{
  Stringa buffer;
  Stringa targetsFile;
  Array intervals;
  int i,j,k;
  Interval *currInterval;
  SubInterval *currSubInterval,*nextSubInterval;
  Array junctions;
  Junction *currJunction;
  FILE *fp;
  int numTargets;
  Array targetSeqs;
  Texta headers;
  Seq *currSeq,*nextSeq;
  int sizeExonOverlap;

  if (argc != 4) {
    usage ("%s <file.2bit> <file.annotation> <sizeExonOverlap>",argv[0]);
  }
  sizeExonOverlap = atoi (argv[3]);
  buffer = stringCreate (100);
  intervalFind_addIntervalsToSearchSpace (argv[2],0);
  intervals = intervalFind_getAllIntervals ();
  junctions = arrayCreate (1000000,Junction);
  for (i = 0; i < arrayMax (intervals); i++) {
    currInterval = arrp (intervals,i,Interval);
    for (j = 0; j < arrayMax (currInterval->subIntervals); j++) {
      currSubInterval = arrp (currInterval->subIntervals,j,SubInterval);
      for (k = j + 1; k < arrayMax (currInterval->subIntervals); k++) {
        nextSubInterval = arrp (currInterval->subIntervals,k,SubInterval);
        currJunction = arrayp (junctions,arrayMax (junctions),Junction);
        currJunction->chromosome = hlr_strdup (currInterval->chromosome);
        currJunction->firstExonEnd = currSubInterval->end;
        currJunction->secondExonStart = nextSubInterval->start;
      }
    }
  }
  arraySort (junctions,(ARRAYORDERF)sortJunctions);
  arrayUniq (junctions,NULL,(ARRAYORDERF)sortJunctions); 
  targetsFile = stringCreate (100);
  stringPrintf (targetsFile,"targets_%d.txt",getpid ());
  if (!( fp = fopen (string (targetsFile),"w")) ){
    die ("Unable to open target file: %s",string (targetsFile));
  }
  numTargets = 0;
  headers = textCreate (1000000);
  for (i = 0; i < arrayMax (junctions); i++) {
    currJunction = arrp (junctions,i,Junction);
    fprintf (fp,"%s:%d-%d\n",currJunction->chromosome,currJunction->firstExonEnd - sizeExonOverlap,currJunction->firstExonEnd);
    fprintf (fp,"%s:%d-%d\n",currJunction->chromosome,currJunction->secondExonStart,currJunction->secondExonStart + sizeExonOverlap);
    stringPrintf (buffer,"%s|%d|%d|%d",currJunction->chromosome,currJunction->firstExonEnd - sizeExonOverlap,currJunction->secondExonStart,sizeExonOverlap);
    textAdd (headers,string (buffer));
    numTargets = numTargets + 2;
  }
  fclose (fp);
  stringPrintf (buffer,"twoBitToFa %s stdout -noMask -seqList=%s",argv[1],string (targetsFile));
  fasta_initFromPipe (string (buffer));
  targetSeqs = fasta_readAllSequences (0);
  fasta_deInit ();

  if (numTargets != arrayMax (targetSeqs)) {
    die ("Invalid number of tragets");
  }
  j = 0;
  for (i = 0; i < arrayMax (targetSeqs); i = i + 2) {
    currSeq = arrp (targetSeqs,i,Seq);
    nextSeq = arrp (targetSeqs,i + 1,Seq);
    printf (">%s\n%s%s\n",textItem (headers,j),currSeq->sequence,nextSeq->sequence);
    j++;
  }
  stringPrintf (buffer,"rm -rf %s",string (targetsFile));
  hlr_system (string (buffer),0);
  stringDestroy (targetsFile);
  stringDestroy (buffer);
  return 0;
}
