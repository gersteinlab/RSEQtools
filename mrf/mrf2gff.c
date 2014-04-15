#include "format.h"
#include "log.h"
#include "mrf.h"



/** 
 *   \file mrf2gff.c Module to convert MRF to GFF.
 *         Usage: mrf2gff <prefix> \n
 *         Creates a GFF file of reads that have more than one alignment block (spliced reads). \n
 *         Takes MRF from STDIN. \n
 */



typedef struct {
  int groupNumber;
  char *targetName;
  char *line;
} GffEntry;



static int sortGffEntriesByTargetNameAndGroupNumber (GffEntry *a, GffEntry *b) 
{
  int diff;

  diff = strcmp (a->targetName,b->targetName);
  if (diff != 0) {
    return diff;
  }
  return a->groupNumber - b->groupNumber;
}



static void processRead (Array gffEntries, MrfRead *currRead, int *groupNumber) 
{
  int i;
  MrfBlock *currBlock;
  GffEntry *currGffEntry;
  static Stringa buffer = NULL;

  if (arrayMax (currRead->blocks) == 1) {
    return;
  }
  stringCreateClear (buffer,100);
  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    currGffEntry = arrayp (gffEntries,arrayMax (gffEntries),GffEntry);
    stringPrintf (buffer,"%s\tMRF\tfeature\t%d\t%d\t.\t%c\t.\tTG%d",
                  currBlock->targetName,
                  currBlock->targetStart,
                  currBlock->targetEnd,
                  currBlock->strand,
                  *groupNumber);
    currGffEntry->targetName = hlr_strdup (currBlock->targetName);
    currGffEntry->line = hlr_strdup (string (buffer));
  }
  (*groupNumber)++;
}

 

int main (int argc, char *argv[])
{
  int i,j,groupNumber;
  MrfEntry *currEntry;
  GffEntry *currGffEntry,*nextGffEntry;
  Array gffEntries;
  FILE *fp;
  Stringa buffer;

  if (argc != 2) {
    usage ("%s <prefix>",argv[0]);
  }

  buffer = stringCreate (1000);
  groupNumber = 0;
  mrf_init ("-");
  gffEntries = arrayCreate (100000,GffEntry);
  while (currEntry = mrf_nextEntry ()) {
    processRead (gffEntries,&currEntry->read1,&groupNumber);
    if (currEntry->isPairedEnd) {
      processRead (gffEntries,&currEntry->read2,&groupNumber);
    }
  }
  mrf_deInit ();

  arraySort (gffEntries,(ARRAYORDERF)sortGffEntriesByTargetNameAndGroupNumber);
  i = 0; 
  while (i < arrayMax (gffEntries)) {
    currGffEntry = arrp (gffEntries,i,GffEntry);
    stringPrintf (buffer,"%s_%s.gff",argv[1],currGffEntry->targetName);
    fp = fopen (string (buffer),"w");
    if (fp == NULL) {
      die ("Unable to open file: %s",string (buffer));
    }
    fprintf (fp,"browser hide all\n");
    fprintf (fp,"track name=\"%s_%s\" visibility=2\n",argv[1],currGffEntry->targetName);
    fprintf (fp,"%s\n",currGffEntry->line);
    j = i + 1;
    while (j < arrayMax (gffEntries)) {
      nextGffEntry = arrp (gffEntries,j,GffEntry);
      if (!strEqual (currGffEntry->targetName,nextGffEntry->targetName)) {
        break;
      } 
      fprintf (fp,"%s\n",nextGffEntry->line);
      j++;
    }
    i = j;
    fclose (fp);
  }
  stringDestroy (buffer);
  return 0;
}
