#include "log.h"
#include "format.h"
#include "mrf.h"
#include <stdio.h>



/** 
 *   \file mrfSubsetByTargetName.c Module to subset MRF file by TargetName.
 *         Usage: subsetMrfByTargetName <file.mrf> \n
 */



typedef struct {
  FILE *fp;
  char *targetName;
} Target;



static int sortTargets (Target *a, Target *b) 
{
  return strcmp (a->targetName,b->targetName);
}



static void addTargetNames (Texta targetNames, Array alignmentBlocks)
{
  MrfBlock *currBlock;
  int i;
  
  for (i = 0; i < arrayMax (alignmentBlocks); i++) {
    currBlock = arrp (alignmentBlocks,i,MrfBlock);
    textAdd (targetNames,currBlock->targetName);
  }
}



static void processEntry (Array targets, MrfEntry *currEntry, char *prefix)
{
  int i;
  static Texta targetNames = NULL;
  Target testEntry;
  Target *currTarget;
  static Stringa buffer = NULL;
  int index;

  textCreateClear (targetNames,10);
  addTargetNames (targetNames,currEntry->read1.blocks);
  if (currEntry->isPairedEnd == 1) {
    addTargetNames (targetNames,currEntry->read2.blocks);
  }
  arraySort (targetNames,(ARRAYORDERF)arrayStrcmp);
  arrayUniq (targetNames,NULL,(ARRAYORDERF)arrayStrcmp);
  for (i = 0; i < arrayMax (targetNames); i++) {
    testEntry.targetName = hlr_strdup (textItem (targetNames,i));
    testEntry.fp = NULL;
    if (arrayFindInsert (targets,&testEntry,&index,(ARRAYORDERF)sortTargets) == 1) {
      // new target inserted
      currTarget = arrp (targets,index,Target);
      stringCreateClear (buffer,100);
      stringPrintf (buffer,"%s_%s.mrf",prefix,textItem (targetNames,i));
      if (!(currTarget->fp = fopen (string (buffer),"w"))) {
        die ("Unable to open file: %s",string (buffer));
      }
      fprintf (currTarget->fp,"%s\n",mrf_writeHeader ());
    }
    else {
      // target is already present      
      hlr_free (testEntry.targetName);
    }
    currTarget = arrp (targets,index,Target);
    fprintf (currTarget->fp,"%s\n",mrf_writeEntry (currEntry));
  }
}



int main (int argc, char *argv[])
{
  MrfEntry *currMRF;
  Target *currTarget;
  Array targets;
  int i;

  if (argc != 2) {
    usage ("%s <prefix>",argv[0]);
  }
  targets = arrayCreate (100,Target);
  mrf_init ("-");
  while (currMRF = mrf_nextEntry ()) {
    processEntry (targets,currMRF,argv[1]);
  }
  mrf_deInit ();
  for (i = 0; i < arrayMax (targets); i++) {
    currTarget = arrp (targets,i,Target);
    fclose (currTarget->fp);
    warn ("Closed file for target: %s",currTarget->targetName);
  }
  return 0;
}
