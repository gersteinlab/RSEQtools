#include "format.h"
#include "log.h"
#include "linestream.h"
#include "common.h"
#include "bgrParser.h"


/** 
 *   \file bgrParser.c Module to parse BedGraphs files
 *   \author Andrea Sboner (andrea.sboner@yale.edu)
 */



static LineStream ls = NULL;



/**
 * Initialize the bgrParser module from file.
 * @param[in] fileName File name, use "-" to denote stdin
 */
void bgrParser_initFromFile (char *fileName)
{  
  ls = ls_createFromFile (fileName);
  ls_bufferSet (ls,1);
}



/**
 * Initialize the bgrParser module from pipe.
 * @param[in] command Command to be executed
 */
void bgrParser_initFromPipe (char *command)
{
  ls = ls_createFromPipe (command);
  ls_bufferSet (ls,1);
}



/**
 * Deinitialize the bowtieParser module.
 */
void bgrParser_deInit (void)
{
  ls_destroy (ls);
}



/**
 * Retrieve the next entry in the bedGraph file.
 */
BedGraph* bgrParser_nextEntry (void)
{
  char *line;
  static BedGraph *currBedGraph = NULL;
  WordIter w;
  line = ls_nextLine (ls);
  if ( !(ls_isEof( ls ) ) ) {	 
    if ( !strStartsWithC (line,"track") ) {
      AllocVar( currBedGraph );
      w = wordIterCreate (line,"\t",1);
      currBedGraph->chromosome = hlr_strdup (wordNext (w));
      currBedGraph->start = atoi (wordNext (w));
      currBedGraph->end = atoi (wordNext (w));
      currBedGraph->value = atof (wordNext (w));
      wordIterDestroy (w);
    } else {
      bgrParser_nextEntry ( );
    }
  } else {
    return NULL;
  }
  return currBedGraph;
}



/**
 * Retrieve all entries from a bedGraph file.
 */
Array bgrParser_getAllEntries ( void ) 
{
  Array bedGraphs;
  BedGraph *currBedGraph;
  
  bedGraphs = arrayCreate (1000000, BedGraph);
  int i=0;
  while (currBedGraph = bgrParser_nextEntry () ) {
    array(bedGraphs, arrayMax (bedGraphs),BedGraph) = *currBedGraph;
    freeMem ( currBedGraph );
    i++;
  }
  return bedGraphs;
}
  
                          

/**
 * Sort BedGraph entries by chromosome, start and end.
 */
int bgrParser_sort (BedGraph *a, BedGraph *b)
{
  int diff;

  diff = strcmp (a->chromosome,b->chromosome);
  if (diff != 0) {
    return diff;
  }
  diff = a->start - b->start;
  if (diff != 0) {
    return diff;
  }
  return b->end - a->end;
}



/**
 * Free an array of BedGraph elements.
 */
void bgrParser_freeBedGraphs (Array bedGraphs)
{
  BedGraph *currBedGraph;
  int i;
  
  for (i = 0; i < arrayMax (bedGraphs); i++) {
    currBedGraph = arrp (bedGraphs,i,BedGraph);
    hlr_free (currBedGraph->chromosome);
  }
  arrayDestroy (bedGraphs);
}



/**
 * Get BedGraph values for a specified region.
 */
Array bgrParser_getValuesForRegion (Array bedGraphs, char *chromosome, int start, int end)                               
{
  BedGraph testBedGraph;
  int index;
  int i,j;
  BedGraph *currBedGraph;
  static Array bedGraphPtrs = NULL;
  int numOccurances;
  Array entries;

  entries = arrayCreate (1000,double);
  if (bedGraphPtrs == NULL) {
    bedGraphPtrs = arrayCreate (100,BedGraph*);
  }
  else {
    arrayClear (bedGraphPtrs);
  }
  testBedGraph.chromosome = hlr_strdup (chromosome);
  testBedGraph.start = start;
  testBedGraph.end = end;
  arrayFind (bedGraphs,&testBedGraph,&index,(ARRAYORDERF)bgrParser_sort); 
  i = index;
  while (i >= 0) {
    currBedGraph = arrp (bedGraphs,i,BedGraph);
    if (!strEqual (chromosome,currBedGraph->chromosome) || currBedGraph->end < start) {
      break;
    }
    array (bedGraphPtrs,arrayMax (bedGraphPtrs),BedGraph*) = currBedGraph;
    i--;
  }
  i = index + 1;
  while (i < arrayMax (bedGraphs)) {
    currBedGraph = arrp (bedGraphs,i,BedGraph);
    if (!strEqual (chromosome,currBedGraph->chromosome) || currBedGraph->start > end) {
      break;
    }
    array (bedGraphPtrs,arrayMax (bedGraphPtrs),BedGraph*) = currBedGraph;
    i++;
  }
  for (i = start; i < end; i++) {
    numOccurances = 0;
    for (j = 0; j < arrayMax (bedGraphPtrs); j++) {
      currBedGraph = arru (bedGraphPtrs,j,BedGraph*);
      if (currBedGraph->start <= i && i < currBedGraph->end) {
        numOccurances++;
        array (entries,arrayMax (entries), double) = currBedGraph->value;
      } 
    }
    if (numOccurances > 1) {
      die ("Expected only one BedGraph overlap per position");
    }
  }
  hlr_free (testBedGraph.chromosome);
  return entries;
}
