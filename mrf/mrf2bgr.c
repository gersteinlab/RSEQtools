#include "log.h"
#include "format.h"
#include "mrf.h"
#include "format.h"



/** 
 *   \file mrf2bgr.c Module to convert MRF to BedGraph.
 *         Generates a BedGraph, where the counts are normalized by the total number of mapped nucleotides per million, unless doNotNormalize is specified. In this case, the raw coverage is reported. \n
 *         Usage: mrf2bgr <prefix> [doNotNormalize] \n
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



//Create region from single MrfRead
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



void write_bedGraphHeader( FILE *f, char* prefix, char* targetName, char* trackName) 
{
  fprintf(f, "track type=bedGraph name=%s_%s description=%s visibility=full\n", prefix, targetName, trackName ? trackName : prefix  );
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
  long int totalNumNucleotides;
  int doNotNormalize;


  if (argc < 2) {
    usage ("%s <prefix> [doNotNormalize]",argv[0]);
  }
  doNotNormalize = 0;
  if (argc == 3) {
    doNotNormalize = 1;
  }
  buffer = stringCreate (100);
  mrf_init ("-");
  regions = arrayCreate (10000000,Region);  
  totalNumNucleotides = 0;
  while (currEntry = mrf_nextEntry ()) {
    processRead (regions, &currEntry->read1);
    totalNumNucleotides += getReadLength (&currEntry->read1); 
    if (currEntry->isPairedEnd) {
      processRead (regions,&currEntry->read2);
      totalNumNucleotides += getReadLength (&currEntry->read2); 
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
    stringPrintf (buffer,"%s_%s.bgr",argv[1],currRegion->targetName);
    fp = fopen (string (buffer),"w");
    if (fp == NULL) {
      die ("Unable to open file: %s",string (buffer));
    }
    write_bedGraphHeader(fp, argv[1], currRegion->targetName,NULL);
    for (k = 0; k < arrayMax (positions); k++) {
      if (arru (positions,k,int) > 0) {
	int offset=1;
	while( arru(positions, k, int) == arru(positions, k+offset, int)) {
	  offset++;
	  if( (k+offset) == arrayMax( positions ) ) { break; } // last element
	}	       
	if (doNotNormalize == 0) {
          fprintf (fp,"%s\t%d\t%d\t%f\n",currRegion->targetName, k-1, (k-1)+offset, (double)arru (positions,k,int) / ((double) totalNumNucleotides / 1000000.0));
        }
        else {
          fprintf (fp,"%s\t%d\t%d\t%d\n",currRegion->targetName, k-1, (k-1)+offset, arru (positions,k,int));
        }
	k=(k-1)+offset;
      }
    }
    fclose (fp);
  }
  stringDestroy (buffer);
  return 0;
}
