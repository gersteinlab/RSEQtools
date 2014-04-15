#include "format.h"
#include "log.h"
#include "linestream.h"
#include "common.h"
#include "elandMultiParser.h"



/** 
 *   \file elandMultiParser.c Module to parse eland_multi.txt files
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



static LineStream ls = NULL;



/**
 * Initialize the elandMultiParser module.
 * @param[in] fileName File name, use "-" to denote stdin
 */
void elandMultiParser_init (char *fileName)
{
  ls = ls_createFromFile (fileName);
}



/**
 * Deinitialize the elandMultiParser module.
 */
void elandMultiParser_deInit (void)
{
  ls_destroy (ls);
}



static void elandMultiParser_freeQuery (ElandMultiQuery *currElandMultiQuery) 
{
  int i;
  ElandMultiEntry *currElandMultiEntry;

  if (currElandMultiQuery == NULL) {
    return;
  }
  hlr_free (currElandMultiQuery->sequenceName);
  hlr_free (currElandMultiQuery->sequence);
  if (currElandMultiQuery->entries != NULL) {
    for (i = 0; i < arrayMax (currElandMultiQuery->entries); i++) {
      currElandMultiEntry = arrp (currElandMultiQuery->entries,i,ElandMultiEntry);
      hlr_free (currElandMultiEntry->chromosome);
    }
    arrayDestroy (currElandMultiQuery->entries);
  }
  freeMem (currElandMultiQuery);
}



/**
 * Returns a pointer to next ElandMultiQuery. 
 * @pre The module has been initialized using elandMultiParser_init().
 * Parse entries of the following format:
   \verbatim
   >FC30M30_111308:1:1:1713:829/1	AATAAACTCTCCTACTGATGATAGATGTTTTTTCT	NM
   >FC30M30_111308:1:1:1713:1605/1	ATATGAACAACGCCATGTTCTTGCAGAAAACGCTT	1:2:0	chr1.fa:16762278R0,21622953R1,143540857F1
   >FC30M30_111308:1:1:1666:671/1	ATCTACACCACCTCAATCACACTACTCCCCCTACC	0:0:1	chrM.fa:5359F2
   \endverbatim
 */
ElandMultiQuery* elandMultiParser_nextQuery (void)
{
  WordIter w1,w2;
  char *line,*token,*firstColon,*lastColon,*pos1,*pos2;
  static char* chromosome = NULL;
  int lengthToken;
  static ElandMultiQuery *currElandMultiQuery = NULL;
  ElandMultiEntry *currElandMultiEntry;
  
  while (line = ls_nextLine (ls)) {
    if (line[0] == '\0') {
      continue;
    }
    elandMultiParser_freeQuery (currElandMultiQuery);
    currElandMultiQuery = NULL;
    AllocVar (currElandMultiQuery);
    w1 = wordIterCreate (line,"\t",0);
    currElandMultiQuery->sequenceName = hlr_strdup (wordNext (w1) + 1); // remove the '>' character at beginning of the line
    currElandMultiQuery->sequence = hlr_strdup (wordNext (w1));
    token = wordNext (w1);
    if (strEqual (token,"NM") || strEqual (token,"QC") || strEqual (token,"RM")) {
      wordIterDestroy (w1);
      return currElandMultiQuery;
    }
    firstColon = strchr (token,':');
    lastColon = strrchr (token,':');
    if (firstColon == NULL || lastColon == NULL) {
      die ("Expected the following format: x:y:z");
    }
    *firstColon = '\0';
    *lastColon = '\0';
    currElandMultiQuery->exactMatches = atoi (token);
    currElandMultiQuery->oneErrorMatches = atoi (firstColon + 1);
    currElandMultiQuery->twoErrorMatches = atoi (lastColon + 1);
    token = wordNext (w1);
    if (token == NULL) {
      wordIterDestroy (w1);
      return currElandMultiQuery;
    }
    w2 =  wordIterCreate (token,",",0);
    currElandMultiQuery->entries = arrayCreate (5,ElandMultiEntry);
    while (token = wordNext (w2)) {
      currElandMultiEntry = arrayp (currElandMultiQuery->entries,arrayMax (currElandMultiQuery->entries),ElandMultiEntry);
      lengthToken = strlen (token);
      if (token[lengthToken - 2] == 'F') {
        currElandMultiEntry->strand = '+';
      }
      else if (token[lengthToken - 2] == 'R') {
        currElandMultiEntry->strand = '-';
      }
      else {
        die ("Unexpected strand: %s",token);
      }
      currElandMultiEntry->numErrors = atoi (token + lengthToken - 1);
      token[lengthToken - 2] = '\0';
      if (pos1 = strchr (token,':')) {
        pos2 = strchr (token,'.');
        *pos2 = '\0';
        strReplace (&chromosome,token);
        token = pos1 + 1;
      }
      currElandMultiEntry->position = atoi (token);
      currElandMultiEntry->chromosome = hlr_strdup (chromosome);
    }
    wordIterDestroy (w2);
    wordIterDestroy (w1);
    return currElandMultiQuery;
  }
  elandMultiParser_freeQuery (currElandMultiQuery);
  currElandMultiQuery = NULL;
  return NULL;
}
