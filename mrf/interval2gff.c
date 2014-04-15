#include "format.h"
#include "log.h"
#include "intervalFind.h"



/** 
 *   \file interval2gff.c Module to convert Interval format to GFF format.
 *         Usage: interval2gff <trackName> \n
 *         Takes Interval format from STDIN. \n
 *         Note: the GFF format is one-based and inclusive.
 */



int main (int argc, char *argv[])
{
  int i,j;
  Array intervals;
  Interval *currInterval;
  SubInterval *currSubInterval;

  if (argc != 2) {
    usage ("%s <trackName>",argv[0]);
  }
  intervalFind_addIntervalsToSearchSpace ("-",0);
  intervals = intervalFind_getAllIntervals ();
  puts ("browser hide all");
  printf ("track name=\"%s\" visibility=2\n",argv[1]);
  for (i = 0; i < arrayMax (intervals); i++) {
    currInterval = arrp (intervals,i,Interval);
    for (j = 0; j < arrayMax (currInterval->subIntervals); j++) {
      currSubInterval = arrp (currInterval->subIntervals,j,SubInterval);
       printf ("%s\tannotation\texon\t%d\t%d\t.\t%c\t.\tgroup%d\n",
               currInterval->chromosome,currSubInterval->start + 1,currSubInterval->end,currInterval->strand,i);
    }
  }
  return 0;
}
