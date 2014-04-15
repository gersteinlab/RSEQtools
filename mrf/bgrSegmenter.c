#include "log.h"
#include "format.h"
#include "linestream.h"
#include "segmentationUtil.h"



/**
 * \file bgrSegmenter.c Module to segment a BedGraph signal track using the maxGap-minRun algorithm.
 *       Usage: bgrSegmenter <bgrPrefix> <threshold> <maxGap> <minRun> \n
 */



int main (int argc, char *argv[])
{
  Stringa buffer;
  LineStream ls1,ls2;
  char *fileName;
  char *targetName;
  char *line;
  int i;
  Array wigs;
  Wig *currWig;
  double threshold;
  int maxGap;
  int minRun;
  Array tars;
  Tar *currTar;
  
  if (argc != 5) {
    usage ("%s <bgrPrefix> <threshold> <maxGap> <minRun>",argv[0]);
  }
  threshold = atof (argv[2]);
  maxGap = atoi (argv[3]);
  minRun = atoi (argv[4]);
  buffer = stringCreate (100);
  tars = arrayCreate (1000000,Tar);
  wigs = arrayCreate (250000000,Wig);
  stringPrintf (buffer,"ls -1 %s*.bgr",argv[1]);
  ls1 = ls_createFromPipe (string (buffer));
  while (fileName = ls_nextLine (ls1)) {
    ls2 = ls_createFromFile (fileName);
    ls_nextLine (ls2); // discard track name line
    int prevEnd = 0;
    while (line = ls_nextLine (ls2)) {
      if ( strStartsWithC( line, "track" ) || 
	   strStartsWithC( line, "chrom" ) ) 
	continue;
      WordIter w = wordIterCreate( line, "\t", 0 );
      targetName = hlr_strdup ( wordNext(w));
      int start = atoi( wordNext ( w ) ) + 1;
      int end = atoi( wordNext ( w ) );
      float value = atof( wordNext( w ) );
      //if( prevEnd > -1) {
	for( i=prevEnd; i < start; i++) {
	  currWig = arrayp (wigs,arrayMax (wigs),Wig);
	  currWig->position = i;
	  currWig->value = 0;
	}
	//}
      for( i=start; i < (end+1); i++) {
	currWig = arrayp (wigs,arrayMax (wigs),Wig);
	currWig->position = i;
	currWig->value = value;
      }
      prevEnd = end+1;
      wordIterDestroy( w );
    }
    ls_destroy (ls2);

    performSegmentation (tars,wigs,targetName,threshold,maxGap,minRun);
    arrayClear (wigs);
    warn ("Done with %s",targetName); 
    hlr_free (targetName);
  }
  ls_destroy (ls1);
  for (i = 0; i < arrayMax (tars); i++) {
    currTar = arrp (tars,i,Tar);
    printf ("%s\t%d\t%d\n",currTar->targetName,currTar->start,currTar->end);
  } 
  stringDestroy (buffer);
  return 0;
}
