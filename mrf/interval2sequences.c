#include "log.h"
#include "format.h"
#include "intervalFind.h"
#include "fasta.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>



/** 
 *   \file interval2sequences.c Module to retrieve genomic/exonic sequences for an annotation set.
 *         Usage: interval2sequences <file.annotation> <exonic|genomic> \n
 *         The genes/transcripts are specified in file.annotation. \n
 *         Uses file.2bit to extract the genomic sequences. \n
 *         Note: twoBitToFa (part of blat package) must be in the PATH. \n
 */



int main (int argc, char *argv[])
{
  Stringa buffer;
  Array intervals;
  int i,j;
  Interval *currInterval;
  SubInterval *currSubInterval;
  FILE *fp;
  Array targetSeqs;
  Seq *currSeq;
  Stringa targetsFile;
  int index;
  Stringa sequence;

  if (argc != 4) {
    usage ("%s <file.2bit> <file.annotation> <exonic|genomic>",argv[0]);
  }
  buffer = stringCreate (100);
  intervalFind_addIntervalsToSearchSpace (argv[2],0);
  intervals = intervalFind_getAllIntervals ();
  targetsFile = stringCreate (100);
  stringPrintf (targetsFile,"targets_%d.txt",getpid ());
  if (!( fp = fopen (string (targetsFile),"w")) ){
    die ("Unable to open target file: %s",string (targetsFile));
  }
  if (strEqual (argv[3],"genomic")) {
    for (i = 0; i < arrayMax (intervals); i++) {
      currInterval = arrp (intervals,i,Interval);
      fprintf (fp,"%s:%d-%d\n",currInterval->chromosome,currInterval->start,currInterval->end);
    }
  }
  else if (strEqual (argv[3],"exonic")) {
    for (i = 0; i < arrayMax (intervals); i++) {
      currInterval = arrp (intervals,i,Interval);
      for (j = 0; j < arrayMax (currInterval->subIntervals); j++) {
        currSubInterval = arrp (currInterval->subIntervals,j,SubInterval);
        fprintf (fp,"%s:%d-%d\n",currInterval->chromosome,currSubInterval->start,currSubInterval->end);
      }
    }
  }
  else {
    usage ("%s <file.2bit> <file.annotation> <exonic|genomic>",argv[0]);
  }
  fclose (fp);
  stringPrintf (buffer,"twoBitToFa %s stdout -noMask -seqList=%s",argv[1],string (targetsFile));
  fasta_initFromPipe (string (buffer));
  targetSeqs = fasta_readAllSequences (0);
  fasta_deInit ();
  if (strEqual (argv[3],"genomic")) {
    for (i = 0; i < arrayMax (targetSeqs); i++) {
      currSeq = arrp (targetSeqs,i,Seq);
      currInterval = arrp (intervals,i,Interval);
      printf (">%s|%s|%c|%d|%d\n%s\n",currInterval->name,currInterval->chromosome,currInterval->strand,currInterval->start,currInterval->end,currSeq->sequence);
    }
  }
  if (strEqual (argv[3],"exonic")) {
    sequence = stringCreate (1000);
    index = 0;
    for (i = 0; i < arrayMax (intervals); i++) {
      currInterval = arrp (intervals,i,Interval);
      stringPrintf (buffer,"%s|%s|%c|",currInterval->name,currInterval->chromosome,currInterval->strand);
      stringClear (sequence);
      for (j = 0; j < arrayMax (currInterval->subIntervals); j++) {
        currSubInterval = arrp (currInterval->subIntervals,j,SubInterval);
        stringAppendf (buffer,"%d|%d%s",currSubInterval->start,currSubInterval->end,j < arrayMax (currInterval->subIntervals) - 1 ? "|" : "");
        currSeq = arrp (targetSeqs,index,Seq);
        stringCat (sequence,currSeq->sequence);
        index++; 
      }
      printf (">%s\n%s\n",string (buffer),string (sequence));
    }
    stringDestroy (sequence);
  }
  stringPrintf (buffer,"rm -rf %s",string (targetsFile));
  hlr_system (string (buffer),0);
  stringDestroy (buffer);
  stringDestroy (targetsFile);
  return 0;
}
