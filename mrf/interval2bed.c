#include "format.h"
#include "log.h"
#include "intervalFind.h"



/** 
 *   \file interval2bed.c Module to convert Interval format to BED format.
 *         Usage: interval2bed <trackName> \n
 *         Takes Interval format from stdin.
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
       printf ("%s\t%d\t%d\t%s\t900\t%c\n",
               currInterval->chromosome,currSubInterval->start,currSubInterval->end,currInterval->name,currInterval->strand);
    }
  }
  return 0;
}
