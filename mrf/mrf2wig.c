#include "log.h"
#include "format.h"
#include "mrf.h"



/** 
 *   \file mrf2wig.c Module to convert MRF to WIG.
 *         Usage: mrf2wig <prefix> [doNotNormalize] \n
 *         By default, the values are normalized by the total number of of mapped nucleotides per million, unless doNotNormalize is specified. In this case, the raw coverage is reported. \n
 *         If the option "counts" is used, then the raw counts are used instead. \n
 *         Takes MRF from STDIN. \n
 */



typedef struct {
  char *targetName;
  int start;
  int end;
} Region;



static int sortRegionsByTargetName (Region *a, Region *b)
{
  return strcmp (a->targetName,b->targetName);
}



static void updateCounts (Array positions, int start, int end) 
{
  int i;

  for (i = start; i <= end; i++) {
    array (positions,i,int)++;
  }
}



static void processRead (Array regions, MrfRead *currRead)
{
  int i;
  MrfBlock *currBlock;
  Region *currRegion;
 
  for(i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    currRegion = arrayp (regions,arrayMax (regions),Region);
    currRegion->targetName = hlr_strdup (currBlock->targetName);
    currRegion->start = currBlock->targetStart;
    currRegion->end = currBlock->targetEnd;
  }
}



int main (int argc, char *argv[])
{
  Stringa buffer;
  int i,j,k;
  MrfEntry *currEntry;
  Array positions;
  FILE *fp;
  Array regions;
  Region *currRegion,*nextRegion;
  int numberOfReads;
  int useCounts;
  
  if (argc < 2) {
    usage ("%s <prefix> [doNotNormalize]",argv[0]);
  }
  useCounts = 0;
  if (argc == 3 && strEqual (argv[2],"doNotNormalize")) {
    useCounts = 1;
  }
  buffer = stringCreate (100);
  numberOfReads = 0;
  mrf_init ("-");
  regions = arrayCreate (10000000,Region);
  while (currEntry = mrf_nextEntry ()) {
    processRead (regions,&currEntry->read1);
    numberOfReads++;
    if (currEntry->isPairedEnd) {
      processRead (regions,&currEntry->read2);
      numberOfReads++;
    }
  }
  mrf_deInit ();
  
  arraySort (regions,(ARRAYORDERF)sortRegionsByTargetName);
  positions = arrayCreate (10000000,int);
  i = 0; 
  while (i < arrayMax (regions)) {
    currRegion = arrp (regions,i,Region);
    arrayClear (positions);
    updateCounts (positions,currRegion->start,currRegion->end);
    j = i + 1;
    while (j < arrayMax (regions)) {
      nextRegion = arrp (regions,j,Region);
      if (!strEqual (currRegion->targetName,nextRegion->targetName)) {
        break;
      } 
      updateCounts (positions,nextRegion->start,nextRegion->end);
      j++;
    }
    i = j;
    stringPrintf (buffer,"%s_%s.wig",argv[1],currRegion->targetName);
    fp = fopen (string (buffer),"w");
    if (fp == NULL) {
      die ("Unable to open file: %s",string (buffer));
    }
    fprintf (fp,"track type=wiggle_0 name=\"%s_%s\"\n",argv[1],currRegion->targetName);
    fprintf (fp,"variableStep chrom=%s span=1\n",currRegion->targetName);
    for (k = 0; k < arrayMax (positions); k++) {
      if (arru (positions,k,int) > 0) {
        if (useCounts == 1) {
          fprintf (fp,"%d\t%d\n",k,arru (positions,k,int));
        }
        else {
          fprintf (fp,"%d\t%0.2f\n",k,arru (positions,k,int) / ((double)numberOfReads / 1000000));
        }
      }
    }
    fclose (fp);
  }
  stringDestroy (buffer);
  return 0;
}
