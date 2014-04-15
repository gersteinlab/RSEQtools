#include "format.h"
#include "mrf.h"
#include "sam.h"



/** 
 *   \file sam.c SAM utilities.
 */



int sortSamEntriesByQname (SamEntry *a, SamEntry *b)
{
  return strcmp (a->qname, b->qname);
}



Stringa genCigar (MrfRead *read)
{
  int ltargetEnd = 0;
  int i;
  Stringa cigar = stringCreate(10);

  for (i = 0; i < arrayMax (read->blocks); i++) {
    MrfBlock *block = arrp (read->blocks, i, MrfBlock);
    if (ltargetEnd > 0)
	  stringAppendf (cigar, "%iN", block->targetStart - ltargetEnd - 1);
    ltargetEnd = block->targetEnd;
    stringAppendf (cigar, "%iM", block->queryEnd - block->queryStart + 1);
  }
  return cigar;
}



void destroySamEArray (Array a)
{
  int i;
  for (i = 0; i < arrayMax(a); i++) {
    SamEntry *currSamE = arrp (a, i, SamEntry);
    free (currSamE->qname);
    free (currSamE->rname);
    free (currSamE->cigar);
    free (currSamE->mrnm);
    if (currSamE->seq)
      free (currSamE->seq);
    if (currSamE->qual)
      free (currSamE->qual);
    if (currSamE->tags)
      free (currSamE->tags);
  }
  arrayDestroy (a);
}
