#include "log.h"
#include "format.h"
#include "linestream.h"
#include "segmentationUtil.h"



/**
 * \file wigSegmenter.c Module to segment a WIG signal track using the maxGap-minRun algorithm.
 *       Usage: wigSegmenter <wigPrefix> <threshold> <maxGap> <minRun> \n
 */



int main (int argc, char *argv[])
{
  Stringa buffer;
  LineStream ls1,ls2;
  char *fileName;
  char *targetName;
  char *line,*pos;
  int i,j,position;
  Array wigs;
  Wig *currWig;
  double threshold;
  int maxGap;
  int minRun;
  Array tars;
  Tar *currTar;
  
  if (argc != 5) {
    usage ("%s <wigPrefix> <threshold> <maxGap> <minRun>",argv[0]);
  }
  threshold = atof (argv[2]);
  maxGap = atoi (argv[3]);
  minRun = atoi (argv[4]);
  buffer = stringCreate (100);
  tars = arrayCreate (1000000,Tar);
  wigs = arrayCreate (250000000,Wig);
  stringPrintf (buffer,"ls -1 %s*.wig",argv[1]);
  ls1 = ls_createFromPipe (string (buffer));
  while (fileName = ls_nextLine (ls1)) {
    ls2 = ls_createFromFile (fileName);
    ls_nextLine (ls2); // discard track name line
    line = ls_nextLine (ls2);
    if(!strStartsWithC (line,"variableStep chrom=")) {
      die ("Expected second line to start with 'variableStep chrom=': %s",line);
    } 
    pos = strchr (line,'=');
    targetName = hlr_strdup (pos + 1);
    pos = strchr (targetName,' ');
    *pos = '\0';
    j = -1;
    while (line = ls_nextLine (ls2)) {
      pos = strchr (line,'\t');
      *pos = '\0';
      position = atoi (line);
      while (1) {
        j++;
        currWig = arrayp (wigs,arrayMax (wigs),Wig);
        currWig->position = j;
        if (j == position) {
          currWig->value = atof (pos + 1);
          break;
        }
        else {
          currWig->value = 0;
        }
      }
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
