#include "format.h"
#include "log.h"
#include "bgrParser.h"
#include "linestream.h"
#include "intervalFind.h"
#include "common.h"

/**
 * \file bgrQuantifier <annotation.interval>.
 * \pre: it requires a BedGraph file from STDIN normalized by the number of mapped nucleotides
 */
int main( int argc, char* argv[] ) {
  Array bgrs;
  Array intervals;
  Array entries;
  int i, j, length;
  double value;
  if( argc < 2 ) {
    usage("%s <annotation.interval>\n%s requires a BedGraph from STDIN", argv[0], argv[0]);
  }
  bgrs = arrayCreate( 1000, BedGraph );
  bgrParser_initFromFile ( "-" );
  bgrs = bgrParser_getAllEntries ();
  bgrParser_deInit();
  arraySort( bgrs, (ARRAYORDERF) bgrParser_sort );
  
  intervalFind_addIntervalsToSearchSpace ( argv[1], 0 );
  intervals = intervalFind_getAllIntervals ();
  
  for( i=0; i<arrayMax(intervals); i++ ) {
    Interval *currInterval = arrp( intervals, i, Interval );
    length = currInterval->end - currInterval->start;
    entries = bgrParser_getValuesForRegion( bgrs, currInterval->chromosome, currInterval->start, currInterval->end);
    value = 0.0;
    for( j=0; j<arrayMax( entries ); j++) 
      value += arru( entries, j, double );
    
    printf("%s\t%s:%d-%d\t%f\n", currInterval->name, 
	   currInterval->chromosome, 
	   currInterval->start+1, 
	   currInterval->end, 
	   value /= length / 1000.0 );       
    arrayDestroy( entries );
  }
  arrayDestroy( intervals );
  return 0;
}
