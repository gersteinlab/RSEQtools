/*
 * Sequence module. 
 *
 * Assumes that DNA is stored as a character.
 * The DNA it generates will include the bases 
 * as lowercase tcag.  It will generally accept
 * uppercase as well, and also 'n' or 'N' or '-'
 * for unknown bases. 
 *
 * Amino acids are stored as single character upper case. 
 *
 * This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. 
 */


/** 
 *   \file seq.h
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#ifndef DEF_SEQ_H
#define DEF_SEQ_H

#include "common.h"
#include "bits.h"

/**
 * Seq.
 */
typedef struct {
  char *name;           /* Name of sequence. */
  char *sequence;       /* Sequence base by base. */
  int size;             /* Size of sequence. */
  Bits* mask;           /* Repeat mask (optional) */
} Seq; 


typedef Seq dnaSeq;	/* Preferred use if DNA */
typedef Seq aaSeq;	/* Preferred use if protein. */


/* Numerical values for bases. */
#define T_BASE_VAL 0
#define U_BASE_VAL 0
#define C_BASE_VAL 1
#define A_BASE_VAL 2
#define G_BASE_VAL 3
#define N_BASE_VAL 4   /* Used in 1/2 byte representation. */

typedef char DNA;
typedef char AA;
typedef char Codon; 

/* A little array to help us decide if a character is a 
 * nucleotide, and if so convert it to lower case. 
 * Contains zeroes for characters that aren't used
 * in DNA sequence. */
extern DNA ntChars[256];
extern AA aaChars[256];

/* An array that converts alphabetical DNA representation
 * to numerical one: X_BASE_VAL as above.  For charaters
 * other than [atgcATGC], has -1. */
extern int ntVal[256];
extern int aaVal[256];
extern int ntValLower[256];	/* NT values only for lower case. */
extern int ntValUpper[256];	/* NT values only for upper case. */

/* Like ntVal, but with T_BASE_VAL in place of -1 for nonexistent nucleotides. */
extern int ntValNoN[256];     

/* Like ntVal but with N_BASE_VAL in place of -1 for 'n', 'x', '-', etc. */
extern int ntVal5[256];

/* Inverse array - takes X_BASE_VAL int to a DNA char value. */
extern DNA valToNt[];

/* Similar array that doesn't convert to lower case. */
extern DNA ntMixedCaseChars[256];

/* Another array to help us do complement of DNA  */
extern DNA ntCompTable[256];

/* Arrays to convert between lower case indicating repeat masking, and
 * a 1/2 byte representation where the 4th bit indicates if the characeter
 * is masked. Uses N_BASE_VAL for `n', `x', etc.
 */
extern int ntValMasked[256];
extern DNA valToNtMasked[256];
 

void seq_init (); 
void seq_complement (DNA *dna, long length);
void seq_reverseComplement (DNA *dna, long length);
aaSeq* seq_translateSeqN (dnaSeq *inSeq, unsigned offset, unsigned size, int stop);
aaSeq* seq_translateSeq (dnaSeq *inSeq, unsigned offset, int stop);
int seq_seqIsLower (Seq *seq);
Bits* seq_maskFromUpperCaseSeq (Seq *seq);
void seq_toRna (DNA *dna);
AA seq_lookupCodon (DNA *dna); 
AA seq_lookupMitochondrialCodon (DNA *dna);
Codon seq_codonVal (DNA *start);
DNA* seq_valToCodon (int val);
AA* seq_dnaTranslate (DNA *dna, int terminateAtStopCodon);
void seq_dnaMixedCaseFilter (char *in, DNA *out);
void seq_dnaFilter (char *in, DNA *out);
void seq_aaFilter (char *in, DNA *out);
void seq_dnaBaseHistogram (DNA *dna, int dnaSize, int histogram[4]);
int seq_intronOrientation (DNA *iStart, DNA *iEnd);
int seq_dnaOrAaScoreMatch (char *a, char *b, int size, int matchScore, int mismatchScore, char ignore);


#endif
