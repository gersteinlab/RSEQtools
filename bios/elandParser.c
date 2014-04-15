#include "format.h"
#include "log.h"
#include "linestream.h"
#include "common.h"
#include "elandParser.h"



/** 
 *   \file elandParser.c Module to parse eland_result.txt files
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



static LineStream ls = NULL;



/**
 * Initialize the elandParser module.
 * @param[in] fileName File name, use "-" to denote stdin
 */
void elandParser_init (char *fileName)
{
  ls = ls_createFromFile (fileName);
}



/**
 * Deinitialize the elandParser module.
 */
void elandParser_deInit (void)
{
  ls_destroy (ls);
}



static void elandParser_freeQuery (ElandQuery *currElandQuery) 
{
  if (currElandQuery == NULL) {
    return;
  }
  hlr_free (currElandQuery->sequenceName);
  hlr_free (currElandQuery->sequence);
  hlr_free (currElandQuery->matchCode);
  if (currElandQuery->chromosome != NULL) {
    hlr_free (currElandQuery->chromosome);
  }
  freeMem (currElandQuery);
}



/**
 * Returns a pointer to next ElandQuery. 
 * @pre The module has been initialized using elandParser_init().
 * Parse entries of the following format:
   \verbatim
   >FC30H5TAA_100308:2:1:1647:1161	GCTTACATTTTTCCTCTCTACATTATC	U0	1	0	0	chr17.fa	8466296	F	..
   >FC30H5TAA_100308:2:1:1588:122	GAGTTAGCCTTGGGACCCCTACTTCTT	U0	1	0	0	chr3.fa	61525628	F	..
   >FC30H5TAA_100308:2:1:1642:123	GGTGAGAGCCGCGACGGGCTTTAGGCG	NM	0	0	0
   >FC30H5TAA_100308:2:1:1630:119	CCGCCATTGCCAGCCCCCAGCTGACGG	R2	0	0	2
   >FC30H5TAA_100308:2:1:1603:120	GCAAGATGAAGTGAAAGGTAAAGAATC	U1	0	1	1	chrM.fa	15277	R	..	26A
   \endverbatim
 */
ElandQuery* elandParser_nextQuery (void)
{
  WordIter w;
  char *line,*token,*pos;
  static ElandQuery *currElandQuery = NULL;
   
  while (line = ls_nextLine (ls)) {
    if (line[0] == '\0') {
      continue;
    }
    elandParser_freeQuery (currElandQuery);
    currElandQuery = NULL;
    AllocVar (currElandQuery);
    w = wordIterCreate (line,"\t",0);
    currElandQuery->sequenceName = hlr_strdup (wordNext (w) + 1); // remove the '>' character at beginning of the line
    currElandQuery->sequence = hlr_strdup (wordNext (w));
    currElandQuery->matchCode = hlr_strdup (wordNext (w));
    if (strEqual (currElandQuery->matchCode,"QC")) {
      wordIterDestroy (w);
      return currElandQuery;
    }
    currElandQuery->exactMatches = atoi (wordNext (w));
    currElandQuery->oneErrorMatches = atoi (wordNext (w));
    currElandQuery->twoErrorMatches = atoi (wordNext (w));
    token = wordNext (w);
    if (token == NULL) {
      wordIterDestroy (w);
      return currElandQuery;
    }
    if (!(pos = strchr (token,'.'))) {
      die ("Expected '.' in chromosome name: %s",token);
    }
    *pos = '\0';
    currElandQuery->chromosome = hlr_strdup (pos + 1);
    currElandQuery->position = atoi (wordNext (w));
    token = wordNext (w);
    if (token[0] == 'F') {
      currElandQuery->strand = '+'; 
    }
    else if (token[0] == 'R') {
      currElandQuery->strand = '-'; 
    } 
    wordIterDestroy (w);
    return currElandQuery;
  }
  elandParser_freeQuery (currElandQuery);
  currElandQuery = NULL;
  return currElandQuery;
}
