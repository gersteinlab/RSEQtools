#include "format.h"
#include "log.h"
#include "linestream.h"
#include "common.h"
#include "blatParser.h"



/** 
 *   \file blatParser.c Module to parse psl BLAT output
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



#define NUM_PSL_HEADER_LINES 5



static LineStream ls = NULL;



/**
 * Initialize the blatParser module from file.
 * @param[in] fileName File name, use "-" to denote stdin
 */
void blatParser_initFromFile (char *fileName)
{
  int i;

  ls = ls_createFromFile (fileName);
  ls_bufferSet (ls,1);
  for (i = 0; i < NUM_PSL_HEADER_LINES; i++) {
    ls_nextLine (ls);
  }
}



/**
 * Initialize the blatParser module from pipe.
 * @param[in] command Command to be executed
 */
void blatParser_initFromPipe (char *command)
{
  int i;

  ls = ls_createFromPipe (command);
  ls_bufferSet (ls,1);
  for (i = 0; i < NUM_PSL_HEADER_LINES; i++) {
    ls_nextLine (ls);
  }
}



/**
 * Deinitialize the blatParser module.
 */
void blatParser_deInit (void)
{
  ls_destroy (ls);
}



static void blatParser_freeQuery (BlatQuery *currBlatQuery) 
{
  int i;
  PslEntry *currPslEntry;

  if (currBlatQuery == NULL) {
    return;
  }
  hlr_free (currBlatQuery->qName);
  for (i = 0; i < arrayMax (currBlatQuery->entries); i++) {
    currPslEntry = arrp (currBlatQuery->entries,i,PslEntry);
    hlr_free (currPslEntry->tName);
    arrayDestroy (currPslEntry->blockSizes);
    arrayDestroy (currPslEntry->tStarts);
    arrayDestroy (currPslEntry->qStarts);
  }
  arrayDestroy (currBlatQuery->entries);
  freeMem (currBlatQuery);
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
 * Returns a pointer to next BlatQuery. 
 * @pre The module has been initialized using blatParser_init().
 */
BlatQuery* blatParser_nextQuery (void)
{
  WordIter w;
  char *line;
  static char *queryName = NULL;
  static char *prevBlatQueryName = NULL;
  static BlatQuery *currBlatQuery = NULL;
  PslEntry *currPslEntry;
  int matches,misMatches,repMatches,nCount,qNumInsert,qBaseInsert,tNumInsert,tBaseInsert;
  char strand;
  int first;
  
  if (!ls_isEof (ls)) {
    blatParser_freeQuery (currBlatQuery);
    currBlatQuery = NULL;
    AllocVar (currBlatQuery);
    currBlatQuery->entries = arrayCreate (5,PslEntry);
    first = 1;
    while (line = ls_nextLine (ls)) {
      if (line[0] == '\0') {
	continue;
      }
      w = wordIterCreate (line,"\t",0);
      matches = atoi (wordNext (w));
      misMatches = atoi (wordNext (w));
      repMatches = atoi (wordNext (w));
      nCount = atoi (wordNext (w));
      qNumInsert = atoi (wordNext (w));
      qBaseInsert = atoi (wordNext (w));
      tNumInsert = atoi (wordNext (w));
      tBaseInsert = atoi (wordNext (w));
      strand = (wordNext (w))[0];
      strReplace (&queryName,wordNext (w));
      if (first == 1 || strEqual (prevBlatQueryName,queryName)) {
	currPslEntry = arrayp (currBlatQuery->entries,arrayMax (currBlatQuery->entries),PslEntry);
	currPslEntry->matches = matches;
	currPslEntry->misMatches = misMatches;
	currPslEntry->repMatches = repMatches;
	currPslEntry->nCount = nCount;
	currPslEntry->qNumInsert = qNumInsert;
	currPslEntry->qBaseInsert = qBaseInsert;
	currPslEntry->tNumInsert = tNumInsert;
	currPslEntry->tBaseInsert = tBaseInsert;
	currPslEntry->strand = strand;
	currPslEntry->qSize = atoi (wordNext (w));
	currPslEntry->qStart = atoi (wordNext (w));
	currPslEntry->qEnd = atoi (wordNext (w));
	currPslEntry->tName = hlr_strdup (wordNext (w));
	currPslEntry->tSize = atoi (wordNext (w));
	currPslEntry->tStart = atoi (wordNext (w));
	currPslEntry->tEnd = atoi (wordNext (w));
	currPslEntry->blockCount = atoi (wordNext (w));
	currPslEntry->blockSizes = arrayCreate (5,int);  
	processCommaSeparatedList (currPslEntry->blockSizes,wordNext (w));
	currPslEntry->qStarts = arrayCreate (5,int);   
	processCommaSeparatedList (currPslEntry->qStarts,wordNext (w));
	currPslEntry->tStarts = arrayCreate (5,int); 
	processCommaSeparatedList (currPslEntry->tStarts,wordNext (w));
      }
      else {
	ls_back (ls,1);
	return currBlatQuery;
      }
      if (first == 1) {
	currBlatQuery->qName = hlr_strdup (queryName);
	first = 0;
      }
      strReplace(&prevBlatQueryName,queryName);
      wordIterDestroy (w);
    }
    if (first == 1) {
      return NULL;
    }
    else {
      return currBlatQuery;
    }
  }
  blatParser_freeQuery (currBlatQuery);
  currBlatQuery = NULL;
  return NULL;
}
