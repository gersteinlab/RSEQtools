/** 
 *   \file fasta.c Module to handle FASTA sequences
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */


#include "log.h"
#include "format.h"
#include "linestream.h"
#include "stringUtil.h"
#include "common.h"
#include "fasta.h"



#define NUM_CHARACTRS_PER_LINE 60


static LineStream lsFasta = NULL;



/**
 * Initialize the FASTA module using a file name.
 * @note Use "-" to denote stdin.
 * @post fasta_nextSequence(), fasta_readAllSequences() can be called.
 */
void fasta_initFromFile (char* fileName) 
{
  lsFasta = ls_createFromFile (fileName);
  ls_bufferSet (lsFasta,1);
}



/**
 * Deinitialize the FASTA module. Frees module internal memory.
 */
void fasta_deInit (void) 
{
  ls_destroy (lsFasta);
}



/**
 * Initialize the FASTA module using a pipe.
 * @post fasta_nextSequence(), fasta_readAllSequences() can be called.
 */
void fasta_initFromPipe (char* command)
{
  lsFasta = ls_createFromPipe (command);
  ls_bufferSet (lsFasta,1);
}



static void fasta_freeSeq (Seq* currSeq)
{
  if (currSeq == NULL) {
    return;
  }
  hlr_free (currSeq->name);
  hlr_free (currSeq->sequence);
  freeMem (currSeq);
  currSeq = NULL;
}



static Seq* fasta_processNextSequence (int freeMemory, int truncateName)
{
  char *line;
  static Stringa buffer = NULL;
  static Seq* currSeq = NULL;
  int count;

  if (ls_isEof (lsFasta)) {
    if (freeMemory) {
      fasta_freeSeq (currSeq);
    }
    return NULL;
  }
  count = 0;
  stringCreateClear (buffer,1000);
  while (line = ls_nextLine (lsFasta)) {
    if (line[0] == '\0') {
      continue;
    }
    if (line[0] == '>') {
      count++;
      if (count == 1) {
	if (freeMemory) {
	  fasta_freeSeq (currSeq);
	}
	AllocVar (currSeq);
	currSeq->name = hlr_strdup (line + 1);
	if (truncateName) {
	  currSeq->name = firstWordInLine (skipLeadingSpaces (currSeq->name));
	}
	continue;
      }
      else if (count == 2) {
	currSeq->sequence = hlr_strdup (string (buffer));
	currSeq->size = stringLen (buffer);
	ls_back (lsFasta,1);
	return currSeq;
      }
    }
    stringCat (buffer,line);
  }
  currSeq->sequence = hlr_strdup (string (buffer));
  currSeq->size = stringLen (buffer);
  return currSeq;
} 



/**
 * Returns a pointer to the next FASTA sequence.
 * @param[in] truncateName If truncateName > 0, leading spaces of the name are skipped. Furthermore, the name is truncated after the first white space. If truncateName == 0, the name is stored as is.
 * @note The memory belongs to this routine.
 */
Seq* fasta_nextSequence (int truncateName) 
{
  return fasta_processNextSequence (1,truncateName);
}



/**
 * Returns an Array of FASTA sequences.
 * @param[in] truncateName If truncateName > 0, leading spaces of the name are skipped. Furthermore, the name is truncated after the first white space. If truncateName == 0, the name is stored as is.
 * @note The memory belongs to this routine.
 */
Array fasta_readAllSequences (int truncateName)
{
  Array seqs;
  Seq *currSeq;

  seqs = arrayCreate (100000,Seq);
  while (currSeq = fasta_processNextSequence (0,truncateName)) {
    array (seqs,arrayMax (seqs),Seq) = *currSeq;
    freeMem (currSeq);
  }
  return seqs;
}



/**
 * Prints currSeq to stdout.
 */
void fasta_printOneSequence (Seq* currSeq) 
{
  char *seq;
  
  seq = insertWordEveryNthPosition (currSeq->sequence,"\n",NUM_CHARACTRS_PER_LINE);
  printf(">%s\n%s\n",currSeq->name,seq);
}



/**
 * Prints seqs to stdout.
 */
void fasta_printSequences (Array seqs)
{
  int i;
  Seq *currSeq;
  
  for (i = 0; i < arrayMax (seqs); i++) {
    currSeq = arrp (seqs,i,Seq);
    fasta_printOneSequence (currSeq); 
  }
}
