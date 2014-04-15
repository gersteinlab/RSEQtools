#include "log.h"
#include "format.h"
#include "linestream.h"
#include "numUtil.h"
#include "intervalFind.h"



/**
 *   \file intervalFind.c Module to efficiently find intervals that overlap with a query interval.
 *   The algorithm is based on containment sublists. 
     See Alekseyenko, A.V., Lee, C.J. (2007) Nested Containment List (NCList): A new algorithm for 
     accelerating interval query of genome alignment and interval databases. Bioinformatics 23: 1386-1393.
     (http://bioinformatics.oxfordjournals.org/cgi/content/abstract/23/11/1386)
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 *   Note: The Interval format is zero-based and half-open.
 */



typedef struct {
  char* chromosome;
  int start;
  int end;
  Array sublist;  // of type Interval*
} SuperInterval;



static Array intervals = NULL;
static Array superIntervals = NULL;
static int superIntervalAssigned = 0;



/**
 * Get the total number of intervals that have been added to the search space.
 * @return The total number of intervals in the search space.
 * @pre Intervals were added to the search space using intervalFind_addIntervalsToSearchSpace()
 */
int intervalFind_getNumberOfIntervals (void)
{
  return arrayMax (intervals);
}



/**
 * Retrieve all the intervals that have been added to the search space.
 * @return An Array of type Interval.
 * @note This function generates a copy of the intervals array. 
 * @pre Intervals were added to the search space using intervalFind_addIntervalsToSearchSpace().
 */
Array intervalFind_getAllIntervals (void)
{
  return arrayCopy (intervals);
}



/**
 * Get an array of Interval pointers from the intervals that have been added to the search space.
 * @return An Array of type Interval pointers.
 * @note The user is not allowed to modify the content of the array. 
 * @pre Intervals were added to the search space using intervalFind_addIntervalsToSearchSpace().
 */
Array intervalFind_getIntervalPointers (void)
{
  Array intervalPointers;
  int i;
  Interval *currInterval;

  intervalPointers = arrayCreate (arrayMax (intervals),Interval*);
  for (i = 0; i < arrayMax (intervals); i++) {
    currInterval = arrp (intervals,i,Interval);
    array (intervalPointers,arrayMax (intervalPointers),Interval*) = currInterval;
  }
  return intervalPointers;
}



static void processCommaSeparatedList (Array results, char *str) 
{
  WordIter w;
  char *tok;

  w = wordIterCreate (str,",",0);
  while (tok = wordNext (w)) {
    if (tok[0] == '\0') {
      continue;
    }
    array (results,arrayMax (results),int) = atoi (tok);
  }
  wordIterDestroy (w);
}



/**
 * Parse a line in the Interval format. 
 * @param[in] thisInterval Pointer to an Interval. Must be allocated and deallocated externally.\n
 * @param[in] line Line in Interval format\n
 * @param[in] source An integer that specifies the source. This is useful when multiple files are used.
 * See intervalFind_addIntervalsToSearchSpace() for details.
 * @pre None.
*/
void intervalFind_parseLine (Interval *thisInterval, char* line, int source)
{
  WordIter w;
  Array subIntervalStarts;
  Array subIntervalEnds;
  SubInterval *currSubInterval;
  int i;

  w = wordIterCreate (line,"\t",0);
  thisInterval->source = source;
  thisInterval->name = hlr_strdup (wordNext (w));
  thisInterval->chromosome = hlr_strdup (wordNext (w));
  thisInterval->strand = wordNext (w)[0];
  thisInterval->start = atoi (wordNext (w));
  thisInterval->end = atoi (wordNext (w));
  thisInterval->subIntervalCount = atoi (wordNext (w));
  subIntervalStarts = arrayCreate (thisInterval->subIntervalCount,int);
  subIntervalEnds = arrayCreate (thisInterval->subIntervalCount,int);
  processCommaSeparatedList (subIntervalStarts,wordNext (w));
  processCommaSeparatedList (subIntervalEnds,wordNext (w));
  if (arrayMax (subIntervalStarts) != arrayMax (subIntervalEnds)) {
    die ("Unequal number of subIntervalStarts and subIntervalEnds");
  }
  thisInterval->subIntervals = arrayCreate (thisInterval->subIntervalCount,SubInterval);
  for (i = 0; i < thisInterval->subIntervalCount; i++) {
    currSubInterval = arrayp (thisInterval->subIntervals,arrayMax (thisInterval->subIntervals),SubInterval);
    currSubInterval->start = arru (subIntervalStarts,i,int);
    currSubInterval->end = arru (subIntervalEnds,i,int);
  }
  arrayDestroy (subIntervalStarts);
  arrayDestroy (subIntervalEnds);
  wordIterDestroy (w);
}



static void parseFileContent (Array theseIntervals, char* fileName, int source)
{
  LineStream ls;
  char *line;
  Interval *currInterval;

  ls = ls_createFromFile (fileName);
  while (line = ls_nextLine (ls)) {
    if (line[0] == '\0') {
      continue;
    }
    currInterval = arrayp (theseIntervals,arrayMax (theseIntervals),Interval);
    intervalFind_parseLine (currInterval,line,source);
  }
  ls_destroy (ls);
}



/**
 * Add intervals to the search space. 
 * @param[in] fileName File name of the file that contains the interval and subintervals to search against. 
   This tab-delimited file must have the following format:
   
   \verbatim
   Column:   Description:
   1         Name of the interval
   2         Chromosome 
   3         Strand
   4         Interval start
   5         Interval end
   6         Number of subintervals
   7         Subinterval starts (comma-delimited)
   8         Subinterval end (comma-delimited)
   \endverbatim

   This is an example:
   \verbatim
   uc001aaw.1      chr1    +       357521  358460  1       357521  358460
   uc001aax.1      chr1    +       410068  411702  3       410068,410854,411258    410159,411121,411702
   uc001aay.1      chr1    -       552622  554252  3       552622,553203,554161    553066,553466,554252
   uc001aaz.1      chr1    +       556324  557910  1       556324  557910
   uc001aba.1      chr1    +       558011  558705  1       558011  558705
   \endverbatim
   Note in this example the intervals represent a transcripts, while the subintervals denote exons.

 * @param[in] source An integer that specifies the source. This is useful when multiple files are used.
*/
void intervalFind_addIntervalsToSearchSpace (char* fileName, int source)
{
  intervals = arrayCreate (100000,Interval);
  parseFileContent (intervals,fileName,source);
}



/**
 * Parse a file in the Interval format. 
 * @param[in] fileName File name of the file that contains the interval and subintervals.\n
 * @param[in] source An integer that specifies the source. This is useful when multiple files are used.
 * See intervalFind_addIntervalsToSearchSpace() for details.
 * @return Array of intervals. The user is responsible to free up the memory. The user can modify the returned Array. 
 * @pre None.
*/
Array intervalFind_parseFile (char* fileName, int source)
{
  Array theseIntervals;

  theseIntervals = arrayCreate (100000,Interval);
  parseFileContent (theseIntervals,fileName,source);
  return theseIntervals;
}



static int sortIntervalsByChromosomeAndStartAndEnd (Interval *a, Interval *b)
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



static int sortSuperIntervalsByChromosomeAndStartAndEnd (SuperInterval *a, SuperInterval *b)
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



static void assignSuperIntervals (void)
{
  int i,j;
  Interval *currInterval,*nextInterval;
  SuperInterval *currSuperInterval;

  superIntervals = arrayCreate (100000,SuperInterval);
  arraySort (intervals,(ARRAYORDERF)sortIntervalsByChromosomeAndStartAndEnd);
  i = 0;
  while (i < arrayMax (intervals)) {
    currInterval = arrp (intervals,i,Interval);
    currSuperInterval = arrayp (superIntervals,arrayMax (superIntervals),SuperInterval);
    currSuperInterval->chromosome = hlr_strdup (currInterval->chromosome);
    currSuperInterval->start = currInterval->start; 
    currSuperInterval->end = currInterval->end;
    currSuperInterval->sublist = arrayCreate (10,Interval*);
    array (currSuperInterval->sublist,arrayMax (currSuperInterval->sublist),Interval*) = currInterval;
    j = i + 1;
    while (j < arrayMax (intervals)) {
      nextInterval = arrp (intervals,j,Interval);
      if (strEqual (currInterval->chromosome,nextInterval->chromosome) &&
	  currInterval->start <= nextInterval->start && 
	  currInterval->end >= nextInterval->end) { 
	array (currSuperInterval->sublist,arrayMax (currSuperInterval->sublist),Interval*) = nextInterval;
      }
      else {
	break;
      }
      j++;
    }
    i = j;
  }
  arraySort (superIntervals,(ARRAYORDERF)sortSuperIntervalsByChromosomeAndStartAndEnd);
}



static void addIntervals (Array matchingIntervals, Array sublist, int start, int end) 
{
  int i;
  Interval *currInterval;

  for (i = 0; i < arrayMax (sublist); i++) {
    currInterval = arru (sublist,i,Interval*);
    if (rangeIntersection (currInterval->start,currInterval->end,start,end) >= 0) {
      array (matchingIntervals,arrayMax (matchingIntervals),Interval*) = currInterval;
    }
  }
}



/**
 * Get the intervals that overlap with the query interval.
 * @param[in] chromosome Chromosome of the query interval
 * @param[in] start Start of the query interval
 * @param[in] end End of the query interval
 * @return An Array of Interval pointers. If no overlapping intervals are found, 
   then an empty Array is returned
 * @note The user is not allowed to modify the content of the array. 
 * @pre Intervals were added to the search space, see intervalFind_addIntervalsToSearchSpace()
 */
Array intervalFind_getOverlappingIntervals (char* chromosome, int start, int end)
{
  SuperInterval testSuperInterval;
  SuperInterval *currSuperInterval;
  int i,index;
  static Array matchingIntervals = NULL;

  if (superIntervalAssigned == 0) {
     assignSuperIntervals ();
     superIntervalAssigned = 1;
  }
  if (matchingIntervals == NULL) {
    matchingIntervals = arrayCreate (20,Interval*);
  }
  else {
    arrayClear (matchingIntervals);
  }
  testSuperInterval.chromosome = chromosome;
  testSuperInterval.start = start;
  testSuperInterval.end = end;
  arrayFind (superIntervals,&testSuperInterval,&index,(ARRAYORDERF)sortSuperIntervalsByChromosomeAndStartAndEnd);
  // Index points to the location where testSuperInterval would be inserted
  i = index;
  while (i >= 0) {
    currSuperInterval = arrp (superIntervals,i,SuperInterval);
    if (!strEqual (currSuperInterval->chromosome,chromosome) || currSuperInterval->end < start) {
      break;
    }
    addIntervals (matchingIntervals,currSuperInterval->sublist,start,end); 
    i--;
  }
  i = index + 1;
  while (i < arrayMax (superIntervals)) {
    currSuperInterval = arrp (superIntervals,i,SuperInterval);
    if (!strEqual (currSuperInterval->chromosome,chromosome) ||  currSuperInterval->start > end) {
      break;
    }
    addIntervals (matchingIntervals,currSuperInterval->sublist,start,end); 
    i++;
  }
  return matchingIntervals;
}



/**
 * Write an Interval to a string.
 * @param[in] currInterval Pointer to an Interval
 * @return A char* representing the Interval in tab-delimited format
 */
char* intervalFind_writeInterval (Interval *currInterval) 
{
  SubInterval *currSubInterval;
  int i;
  static Stringa buffer = NULL;

  stringCreateClear (buffer,100);
  stringPrintf (buffer,"%s\t%s\t%c\t%d\t%d\t%d\t",currInterval->name,currInterval->chromosome,currInterval->strand,currInterval->start,currInterval->end,currInterval->subIntervalCount);
  for (i = 0; i < arrayMax (currInterval->subIntervals); i++) {
    currSubInterval = arrp (currInterval->subIntervals,i,SubInterval);
    stringAppendf (buffer,"%d%s",currSubInterval->start,i < arrayMax (currInterval->subIntervals) - 1 ? "," : "\t");
  }
  for (i = 0; i < arrayMax (currInterval->subIntervals); i++) {
    currSubInterval = arrp (currInterval->subIntervals,i,SubInterval);
    stringAppendf (buffer,"%d%s",currSubInterval->end,i < arrayMax (currInterval->subIntervals) - 1 ? "," : "");
  }
  return string (buffer);
}
