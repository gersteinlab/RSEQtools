#include "format.h"
#include "log.h"
#include "linestream.h"
#include "common.h"
#include "blastParser.h"



/** 
 *   \file blastParser.c Module to parse tab-delimited BLAST output
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



static LineStream ls = NULL;



/**
 * Initialize the blastParser module from file.
 * @param[in] fileName File name, use "-" to denote stdin
 */
void blastParser_initFromFile (char* fileName)
{
  ls = ls_createFromFile (fileName);
  ls_bufferSet (ls,1);
}





/**
 * Initialize the blastParser module from pipe.
 * @param[in] command Command to be executed
 */
void blastParser_initFromPipe (char* command)
{
  ls = ls_createFromPipe (command);
  ls_bufferSet (ls,1);
}




/**
 * Deinitialize the blatParser module.
 */
void blastParser_deInit (void)
{
  ls_destroy (ls);
}



static void blastParser_freeQuery (BlastQuery *currBlastQuery) 
{
  int i;
  BlastEntry *currBlastEntry;

  if (currBlastQuery == NULL) {
    return;
  }
  hlr_free (currBlastQuery->qName);
  for (i = 0; i < arrayMax (currBlastQuery->entries); i++) {
    currBlastEntry = arrp (currBlastQuery->entries,i,BlastEntry);
    hlr_free (currBlastEntry->tName);
  }
  arrayDestroy (currBlastQuery->entries);
  freeMem (currBlastQuery);
}



static void blastParser_processLine (char* line, BlastQuery* currBlastQuery) 
{
  WordIter w;
  BlastEntry *currEntry;

  currEntry = arrayp (currBlastQuery->entries,arrayMax (currBlastQuery->entries),BlastEntry);
  w = wordIterCreate (line,"\t",0);
  currEntry->tName = hlr_strdup (wordNext (w));
  currEntry->percentIdentity = atof (wordNext (w));
  currEntry->alignmentLength = atoi (wordNext (w));
  currEntry->misMatches = atoi (wordNext (w));
  currEntry->gapOpenings = atoi (wordNext (w));
  currEntry->qStart = atoi (wordNext (w));
  currEntry->qEnd = atoi (wordNext (w));
  currEntry->tStart = atoi (wordNext (w));
  currEntry->tEnd = atoi (wordNext (w));
  currEntry->evalue = atof (wordNext (w));
  currEntry->bitScore = atof (wordNext (w));
  wordIterDestroy (w);
}



/**
 * Get the next BlastQuery.
 * @pre The module has been initialized using blastParser_init().
 */
BlastQuery* blastParser_nextQuery (void) 
{
  char *line,*pos;
  static char *queryName = NULL;
  static char *prevBlastQueryName = NULL;
  static BlastQuery *currBlastQuery = NULL;
  int first;

  if (!ls_isEof (ls)) {
    blastParser_freeQuery (currBlastQuery);
    currBlastQuery = NULL;
    AllocVar (currBlastQuery);
    currBlastQuery->entries = arrayCreate (5,BlastEntry);
    first = 1;
    while (line = ls_nextLine (ls)) {
      if (line[0] == '\0') {
	continue;
      }
      pos = strchr (line,'\t');
      *pos = '\0';
      strReplace (&queryName,line);
      if (first == 1 || strEqual (prevBlastQueryName,queryName)) {
	blastParser_processLine (pos + 1,currBlastQuery); 
      }
      else {
	ls_back (ls,1);
	return currBlastQuery;
      }
      if (first == 1) {
	currBlastQuery->qName = hlr_strdup (queryName);
	first = 0;
      }
      strReplace(&prevBlastQueryName,queryName);
    }
    if (first == 1) {
      return NULL;
    }
    else {
      return currBlastQuery;
    }
  }
  blastParser_freeQuery (currBlastQuery);
  currBlastQuery = NULL;
  return NULL;
}



