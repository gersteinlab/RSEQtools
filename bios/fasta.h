/** 
 *   \file fasta.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */


#ifndef DEF_FASTA_H
#define DEF_FASTA_H


#include "seq.h"


extern void fasta_initFromFile (char *fileName);
extern void fasta_initFromPipe (char *command);
extern void fasta_deInit (void);
extern Seq* fasta_nextSequence (int truncateName);
extern Array fasta_readAllSequences (int truncateName);
extern void fasta_printOneSequence (Seq *currSeq);
extern void fasta_printSequences (Array seqs);


#endif
