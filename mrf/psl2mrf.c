#include "log.h"
#include "format.h"
#include "mrf.h"
#include "blatParser.h"



/** 
 *   \file psl2mrf.c Module to convert BLAT PSL format into MRF.
 *         Reads a file in PSL format from STDIN. \n
 */



int main (int argc, char *argv[])
{
  BlatQuery *currQuery;
  PslEntry *currEntry;
  int i,j,k;
  
  printf ("%s\n",MRF_COLUMN_NAME_BLOCKS);
  blatParser_initFromFile ("-");
  while (currQuery = blatParser_nextQuery ()) {
    for (i = 0; i < arrayMax (currQuery->entries); i++) {
      currEntry = arrp (currQuery->entries,i,PslEntry);
      if (arrayMax (currEntry->qStarts) != arrayMax (currEntry->blockSizes) ||
          arrayMax (currEntry->tStarts) != arrayMax (currEntry->blockSizes)) {
        die ("qStarts, tStarts, and blockSizes have different sizes!");
      }
      if (currEntry->strand == '+') {
        for (j = 0; j < arrayMax (currEntry->qStarts); j++) {
          printf ("%s:%c:%d:%d:%d:%d%s",
                  currEntry->tName,
                  currEntry->strand,
                  arru (currEntry->tStarts,j,int) + 1,
                  arru (currEntry->tStarts,j,int) + arru (currEntry->blockSizes,j,int),
                  arru (currEntry->qStarts,j,int) + 1,
                  arru (currEntry->qStarts,j,int) + arru (currEntry->blockSizes,j,int),
                  j < arrayMax (currEntry->qStarts) - 1 ? "," : "");
        }
      }
      else if (currEntry->strand == '-') {
        for (j = arrayMax (currEntry->qStarts) - 1,k = 0; j >= 0; j--,k++) {
           printf ("%s:%c:%d:%d:%d:%d%s",
                   currEntry->tName,
                   currEntry->strand,
                   arru (currEntry->tStarts,k,int) + 1,
                   arru (currEntry->tStarts,k,int) + arru (currEntry->blockSizes,k,int),
                   currEntry->qSize - arru (currEntry->qStarts,j,int) - arru (currEntry->blockSizes,j,int) + 1,
                   currEntry->qSize - arru (currEntry->qStarts,j,int),
                   j > 0 ? "," : "");
        }
      }
      else {
        die ("Unexpected strand: %c",currEntry->strand);
      }
      printf ("%s",i < arrayMax (currQuery->entries) - 1 ? "," : "");
    }
    puts ("");
  } 
  blatParser_deInit ();
  return 0;
}

