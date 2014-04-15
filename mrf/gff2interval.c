#include "format.h"
#include "log.h"
#include "linestream.h"



/** 
 *   \file gff2interval.c Module to convert GFF to the Interval format.
 *         Usage: gff2interval \n
 *         Takes GFF from STDIN. \n
 *         Note: The GFF format is one-based and inclusive. The Interval format is zero-based and half-open. \n
 */



typedef struct {
  char *targetName;
  int start;
  int end;
  char strand;
  char *group;
} GffEntry;



static int sortGffEntries (GffEntry *a, GffEntry *b)
{
  int diff;

  diff = strcmp (a->group,b->group);
  if (diff != 0) {
    return diff;
  }
  return a->start - b->start;
}



static void processGroups (Array groups)
{
  GffEntry *currEntry;
  int i;
  static int interval = 1;

  currEntry = arru (groups,0,GffEntry*);
  printf ("Interval_%d_%s\t%s\t%c\t%d\t%d\t%d\t",
          interval,currEntry->group,
          currEntry->targetName,
          currEntry->strand,
          arru (groups,0,GffEntry*)->start,
          arru (groups,arrayMax (groups) - 1,GffEntry*)->end,
          arrayMax (groups));
  for (i = 0; i < arrayMax (groups); i++) {
    currEntry = arru (groups,i,GffEntry*);
    printf ("%d%s",currEntry->start,i < arrayMax (groups) - 1 ? "," : "\t");
  }
  for (i = 0; i < arrayMax (groups); i++) {
    currEntry = arru (groups,i,GffEntry*);
    printf ("%d%s",currEntry->end,i < arrayMax (groups) - 1 ? "," : "\n");
  }
  interval++;
}



int main (int argc, char *argv[]) 
{
  LineStream ls;
  char *line;
  Texta tokens;
  Array entries;
  GffEntry *currEntry,*nextEntry;
  int i,j;
  Array groups;

  entries = arrayCreate (100000,GffEntry);
  ls = ls_createFromFile ("-");
  // get rid of two header lines:
  // browser hide all
  // track name="Name" visibility=2
  ls_nextLine (ls);
  ls_nextLine (ls);
  while (line = ls_nextLine (ls)) {
    tokens = textFieldtok (line,"\t");
    currEntry = arrayp (entries,arrayMax (entries),GffEntry);
    currEntry->targetName = hlr_strdup (textItem (tokens,0));
    currEntry->start = atoi (textItem (tokens,3)) - 1; // make zero-based
    currEntry->end = atoi (textItem (tokens,4));
    currEntry->strand = textItem (tokens,6)[0];
    currEntry->group = hlr_strdup (textItem (tokens,8));
    textDestroy (tokens);
  }
  arraySort (entries,(ARRAYORDERF)sortGffEntries);

  groups = arrayCreate (100,GffEntry*);
  i = 0;
  while (i < arrayMax (entries)) {
    arrayClear (groups);
    currEntry = arrp (entries,i,GffEntry);
    array (groups,arrayMax (groups),GffEntry*) = currEntry;
    j = i + 1;
    while (j < arrayMax (entries)) {
      nextEntry = arrp (entries,j,GffEntry);
      if (strEqual (currEntry->group,nextEntry->group)) {
        array (groups,arrayMax (groups),GffEntry*) = nextEntry;
      }
      else {
        break;
      }
      j++;
    }
    i = j;
    processGroups (groups);
  }
  return 0;
}
