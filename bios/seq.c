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
 *   \file seq.c Module to handle DNA and protein sequences
 *   \author Adapted by Lukas Habegger (lukas.habegger@yale.edu)
 */


#include "log.h"
#include "format.h"
#include "common.h"
#include "seq.h"
#include "numUtil.h"


#define MASKED_BASE_BIT 8


struct codonTable
{
  DNA *codon;	/* Lower case. */
  AA protCode;	/* Upper case. The "Standard" code */
  AA mitoCode;	/* Upper case. Vertebrate mitochondrial translations */
};


struct codonTable codonTable[] = 
  {
    {"ttt", 'F', 'F',},
    {"ttc", 'F', 'F',},
    {"tta", 'L', 'L',},
    {"ttg", 'L', 'L',},
    
    {"tct", 'S', 'S',},
    {"tcc", 'S', 'S',},
    {"tca", 'S', 'S',},
    {"tcg", 'S', 'S',},

    {"tat", 'Y', 'Y',},
    {"tac", 'Y', 'Y',},
    {"taa", 0, 0,},
    {"tag", 0, 0,},

    {"tgt", 'C', 'C',},
    {"tgc", 'C', 'C',},
    {"tga", 0, 'W',},
    {"tgg", 'W', 'W',},

    {"ctt", 'L', 'L',},
    {"ctc", 'L', 'L',},
    {"cta", 'L', 'L',},
    {"ctg", 'L', 'L',},

    {"cct", 'P', 'P',},
    {"ccc", 'P', 'P',},
    {"cca", 'P', 'P',},
    {"ccg", 'P', 'P',},

    {"cat", 'H', 'H',},
    {"cac", 'H', 'H',},
    {"caa", 'Q', 'Q',},
    {"cag", 'Q', 'Q',},

    {"cgt", 'R', 'R',},
    {"cgc", 'R', 'R',},
    {"cga", 'R', 'R',},
    {"cgg", 'R', 'R',},

    {"att", 'I', 'I',},
    {"atc", 'I', 'I',},
    {"ata", 'I', 'M',},
    {"atg", 'M', 'M',},

    {"act", 'T', 'T',},
    {"acc", 'T', 'T',},
    {"aca", 'T', 'T',},
    {"acg", 'T', 'T',},

    {"aat", 'N', 'N',},
    {"aac", 'N', 'N',},
    {"aaa", 'K', 'K',},
    {"aag", 'K', 'K',},

    {"agt", 'S', 'S',},
    {"agc", 'S', 'S',},
    {"aga", 'R', 0,},
    {"agg", 'R', 0,},

    {"gtt", 'V', 'V',},
    {"gtc", 'V', 'V',},
    {"gta", 'V', 'V',},
    {"gtg", 'V', 'V',},

    {"gct", 'A', 'A',},
    {"gcc", 'A', 'A',},
    {"gca", 'A', 'A',},
    {"gcg", 'A', 'A',},

    {"gat", 'D', 'D',},
    {"gac", 'D', 'D',},
    {"gaa", 'E', 'E',},
    {"gag", 'E', 'E',},

    {"ggt", 'G', 'G',},
    {"ggc", 'G', 'G',},
    {"gga", 'G', 'G',},
    {"ggg", 'G', 'G',},
  };

/* A table that gives values 0 for t
			     1 for c
			     2 for a
			     3 for g
 * (which is order aa's are in biochemistry codon tables)
 * and gives -1 for all others. */
int ntVal[256];
int ntValLower[256];	/* NT values only for lower case. */
int ntValUpper[256];	/* NT values only for upper case. */
int ntVal5[256];
int ntValNoN[256]; /* Like ntVal, but with T_BASE_VAL in place of -1 for nonexistent ones. */
DNA valToNt[(N_BASE_VAL|MASKED_BASE_BIT)+1];

/* convert tables for bit-4 indicating masked */
int ntValMasked[256];
DNA valToNtMasked[256];


static int inittedNtVal = 0;

static void initNtVal()
{
  if (!inittedNtVal)
    {
      int i;
      for (i=0; i<NUMELE(ntVal); i++)
        {
	  ntValUpper[i] = ntValLower[i] = ntVal[i] = -1;
	  ntValNoN[i] = T_BASE_VAL;
	  if (isspace(i) || isdigit(i))
	    ntVal5[i] = ntValMasked[i] = -1;
	  else
            {
	      ntVal5[i] = N_BASE_VAL;
	      ntValMasked[i] = (islower(i) ? (N_BASE_VAL|MASKED_BASE_BIT) : N_BASE_VAL);
            }
        }
      ntVal5['t'] = ntVal5['T'] = ntValNoN['t'] = ntValNoN['T'] = ntVal['t'] = ntVal['T'] = 
    	ntValLower['t'] = ntValUpper['T'] = T_BASE_VAL;
      ntVal5['u'] = ntVal5['U'] = ntValNoN['u'] = ntValNoN['U'] = ntVal['u'] = ntVal['U'] = 
    	ntValLower['u'] = ntValUpper['U'] = U_BASE_VAL;
      ntVal5['c'] = ntVal5['C'] = ntValNoN['c'] = ntValNoN['C'] = ntVal['c'] = ntVal['C'] = 
    	ntValLower['c'] = ntValUpper['C'] = C_BASE_VAL;
      ntVal5['a'] = ntVal5['A'] = ntValNoN['a'] = ntValNoN['A'] = ntVal['a'] = ntVal['A'] = 
    	ntValLower['a'] = ntValUpper['A'] = A_BASE_VAL;
      ntVal5['g'] = ntVal5['G'] = ntValNoN['g'] = ntValNoN['G'] = ntVal['g'] = ntVal['G'] = 
    	ntValLower['g'] = ntValUpper['G'] = G_BASE_VAL;
      
      valToNt[T_BASE_VAL] = valToNt[T_BASE_VAL|MASKED_BASE_BIT] = 't';
      valToNt[C_BASE_VAL] = valToNt[C_BASE_VAL|MASKED_BASE_BIT] = 'c';
      valToNt[A_BASE_VAL] = valToNt[A_BASE_VAL|MASKED_BASE_BIT] = 'a';
      valToNt[G_BASE_VAL] = valToNt[G_BASE_VAL|MASKED_BASE_BIT] = 'g';
      valToNt[N_BASE_VAL] = valToNt[N_BASE_VAL|MASKED_BASE_BIT] = 'n';
      
      /* masked values */
      ntValMasked['T'] = T_BASE_VAL;
      ntValMasked['U'] = U_BASE_VAL;
      ntValMasked['C'] = C_BASE_VAL;
      ntValMasked['A'] = A_BASE_VAL;
      ntValMasked['G'] = G_BASE_VAL;
      
      ntValMasked['t'] = T_BASE_VAL|MASKED_BASE_BIT;
      ntValMasked['u'] = U_BASE_VAL|MASKED_BASE_BIT;
      ntValMasked['c'] = C_BASE_VAL|MASKED_BASE_BIT;
      ntValMasked['a'] = A_BASE_VAL|MASKED_BASE_BIT;
      ntValMasked['g'] = G_BASE_VAL|MASKED_BASE_BIT;
      
      valToNtMasked[T_BASE_VAL] = 'T';
      valToNtMasked[C_BASE_VAL] = 'C';
      valToNtMasked[A_BASE_VAL] = 'A';
      valToNtMasked[G_BASE_VAL] = 'G';
      valToNtMasked[N_BASE_VAL] = 'N';
      
      valToNtMasked[T_BASE_VAL|MASKED_BASE_BIT] = 't';
      valToNtMasked[C_BASE_VAL|MASKED_BASE_BIT] = 'c';
      valToNtMasked[A_BASE_VAL|MASKED_BASE_BIT] = 'a';
      valToNtMasked[G_BASE_VAL|MASKED_BASE_BIT] = 'g';
      valToNtMasked[N_BASE_VAL|MASKED_BASE_BIT] = 'n';
      
      inittedNtVal = 1;
    }
}



/**
 * Returns one letter code for protein, 0 for stop codon, or X for bad input.
 */
AA seq_lookupCodon(DNA *dna)
{
  int ix;
  int i;
  char c;
  
  if (!inittedNtVal)
    initNtVal();
  ix = 0;
  for (i=0; i<3; ++i)
    {
      int bv = ntVal[(int)dna[i]];
      if (bv<0)
	return 'X';
      ix = (ix<<2) + bv;
    }
  c = codonTable[ix].protCode;
  c = toupper(c);
  return c;
}



/**
 * Returns one letter code for protein, 0 for stop codon, or X for bad input.
 */
AA seq_lookupMitochondrialCodon(DNA *dna)
{
  int ix;
  int i;
  char c;

  if (!inittedNtVal)
    initNtVal();
  ix = 0;
  for (i=0; i<3; ++i)
    {
      int bv = ntVal[(int)dna[i]];
      if (bv<0)
	return 'X';
      ix = (ix<<2) + bv;
    }
  c = codonTable[ix].mitoCode;
  c = toupper(c);
  return c;
}



/** 
 * Return value from 0-63 of codon starting at start. Returns -1 if not a codon. 
 */
Codon seq_codonVal(DNA *start)
{
  int v1,v2,v3;
  
  if ((v1 = ntVal[(int)start[0]]) < 0)
    return -1;
  if ((v2 = ntVal[(int)start[1]]) < 0)
    return -1;
  if ((v3 = ntVal[(int)start[2]]) < 0)
    return -1;
  return ((v1<<4) + (v2<<2) + v3);
}



/** 
 * Return codon corresponding to val (0-63) 
 */
DNA* seq_valToCodon(int val)
{
  assert(val >= 0 && val < 64);
  return codonTable[val].codon;
}



char* seq_dnaTranslate (DNA *dna, int terminateAtStopCodon)
{
  static Stringa buffer = NULL;
  int i;
  int dnaSize;
  char aa;

  stringCreateClear (buffer,500);
  dnaSize = strlen(dna);
  for (i=0; i<dnaSize-2; i+=3) {
    aa = seq_lookupCodon(dna+i);
    if (aa == 0) {
      if (terminateAtStopCodon) {
	break;
      }
      aa = '*';
    }
    stringCatChar (buffer,aa);
  }
  return string (buffer);
}


/* A little array to help us decide if a character is a 
 * nucleotide, and if so convert it to lower case. */
char ntChars[256];

static void initNtChars()
{
  static int initted = 0;

  if (!initted)
    {
      zeroBytes(ntChars, sizeof(ntChars));
      ntChars['a'] = ntChars['A'] = 'a';
      ntChars['c'] = ntChars['C'] = 'c';
      ntChars['g'] = ntChars['G'] = 'g';
      ntChars['t'] = ntChars['T'] = 't';
      ntChars['n'] = ntChars['N'] = 'n';
      ntChars['u'] = ntChars['U'] = 'u';
      ntChars['-'] = 'n';
      initted = 1;
    }
}

char ntMixedCaseChars[256];

static void initNtMixedCaseChars()
{
  static int initted = 0;
  
  if (!initted)
    {
      zeroBytes(ntMixedCaseChars, sizeof(ntMixedCaseChars));
      ntMixedCaseChars['a'] = 'a';
      ntMixedCaseChars['A'] = 'A';
      ntMixedCaseChars['c'] = 'c';
      ntMixedCaseChars['C'] = 'C';
      ntMixedCaseChars['g'] = 'g';
      ntMixedCaseChars['G'] = 'G';
      ntMixedCaseChars['t'] = 't';
      ntMixedCaseChars['T'] = 'T';
      ntMixedCaseChars['n'] = 'n';
      ntMixedCaseChars['N'] = 'N';
      ntMixedCaseChars['u'] = 'u';
      ntMixedCaseChars['U'] = 'U';
      ntMixedCaseChars['-'] = 'n';
      initted = 1;
    }
}



DNA ntCompTable[256];
static int inittedCompTable = 0;

static void initNtCompTable()
{
  zeroBytes(ntCompTable, sizeof(ntCompTable));
  ntCompTable[' '] = ' ';
  ntCompTable['-'] = '-';
  ntCompTable['='] = '=';
  ntCompTable['a'] = 't';
  ntCompTable['c'] = 'g';
  ntCompTable['g'] = 'c';
  ntCompTable['t'] = 'a';
  ntCompTable['u'] = 'a';
  ntCompTable['n'] = 'n';
  ntCompTable['-'] = '-';
  ntCompTable['.'] = '.';
  ntCompTable['A'] = 'T';
  ntCompTable['C'] = 'G';
  ntCompTable['G'] = 'C';
  ntCompTable['T'] = 'A';
  ntCompTable['U'] = 'A';
  ntCompTable['N'] = 'N';
  ntCompTable['R'] = 'Y';
  ntCompTable['Y'] = 'R';
  ntCompTable['M'] = 'K';
  ntCompTable['K'] = 'M';
  ntCompTable['S'] = 'S';
  ntCompTable['W'] = 'W';
  ntCompTable['V'] = 'B';
  ntCompTable['H'] = 'D';
  ntCompTable['D'] = 'H';
  ntCompTable['B'] = 'V';
  ntCompTable['X'] = 'N';
  ntCompTable['r'] = 'y';
  ntCompTable['y'] = 'r';
  ntCompTable['s'] = 's';
  ntCompTable['w'] = 'w';
  ntCompTable['m'] = 'k';
  ntCompTable['k'] = 'm';
  ntCompTable['v'] = 'b';
  ntCompTable['h'] = 'd';
  ntCompTable['d'] = 'h';
  ntCompTable['b'] = 'v';
  ntCompTable['x'] = 'n';
  ntCompTable['('] = ')';
  ntCompTable[')'] = '(';
  inittedCompTable = 1;
}



/** 
 * Complement DNA (not reverse). 
 */
void seq_complement(DNA *dna, long length)
{
  int i;
  
  if (!inittedCompTable) 
    initNtCompTable();
  for (i=0; i<length; ++i)
    {
      *dna = ntCompTable[(int)*dna];
      ++dna;
    }
}



/**
 * Reverse complement DNA. 
 */
void seq_reverseComplement(DNA *dna, long length)
{
  reverseBytes(dna, length);
  seq_complement(dna, length);
}



/** 
 * Return 1 if sequence is all lower case, 0 otherwise. 
 */
int seq_seqIsLower(Seq *seq)
{
  int size = seq->size, i;
  char *poly = seq->sequence;
  for (i=0; i<size; ++i)
    if (!islower(poly[i]))
      return 0;
  return 1;
}



/** 
 * Return a translated sequence.  Offset is position of first base to translate. 
 * If size is 0 then use length of inSeq. 
 */
aaSeq* seq_translateSeqN(dnaSeq *inSeq, unsigned offset, unsigned inSize, int stop)
{
  aaSeq *seq;
  DNA *dna = inSeq->sequence;
  AA *pep, aa;
  int i, lastCodon;
  int actualSize = 0;
  
  assert(offset <= inSeq->size);
  if ((inSize == 0) || (inSize > (inSeq->size - offset)))
    inSize = inSeq->size - offset;
  lastCodon = offset + inSize - 3;
  
  AllocVar(seq);
  seq->sequence = pep = needLargeMem(inSize/3+1);
  for (i=offset; i <= lastCodon; i += 3)
    {
      aa = seq_lookupCodon(dna+i);
      if (aa == 0)
	{
	  if (stop)
	    break;
	  else
	    aa = 'Z';
	}
      *pep++ = aa;
      ++actualSize;
    }
  *pep = 0;
  assert(actualSize <= inSize/3+1);
  seq->size = actualSize;
  seq->name = hlr_strdup(inSeq->name);
  return seq;
}



/**
 * Return a translated sequence.  Offset is position of first base to
 * translate. If stop is 1 then stop at first stop codon.  
 * (Otherwise represent stop codons as 'Z').
 */
aaSeq* seq_translateSeq(dnaSeq *inSeq, unsigned offset, int stop)
{
  return seq_translateSeqN(inSeq, offset, 0, stop);
}



/** 
 * Allocate a mask for sequence and fill it in based on sequence case. 
 */
Bits* seq_maskFromUpperCaseSeq(Seq *seq)
{
  int size = seq->size, i;
  char *poly = seq->sequence;
  Bits *b = bitAlloc(size);
  for (i=0; i<size; ++i)
    {
      if (isupper(poly[i]))
        bitSetOne(b, i);
    }
  return b;
}



/** 
 * Convert T's to U's.
 */
void seq_toRna(DNA *dna)
{
  DNA c;
  for (;;)
    {
      c = *dna;
      if (c == 't')
	*dna = 'u';
      else if (c == 'T')
	*dna = 'U';
      else if (c == 0)
	break;
      ++dna;
    }
}



/* Run chars through filter. */
static void dnaOrAaFilter(char *in, char *out, char filter[256])
{
  char c;
  seq_init();
  while ((c = *in++) != 0)
    {
      if ((c = filter[(int)c]) != 0) *out++ = c;
    }
  *out++ = 0;
}



/** 
 * Filter out non-DNA characters and change to lower case. 
 */
void seq_dnaFilter(char *in, DNA *out)
{
  dnaOrAaFilter(in, out, ntChars);
}



/** 
 * Filter out non-DNA characters but leave case intact. 
 */
void seq_dnaMixedCaseFilter(char *in, DNA *out)
{
  dnaOrAaFilter(in, out, ntMixedCaseChars);
}



/** 
 * Filter out non-aa characters and change to upper case. 
 */
void seq_aaFilter(char *in, DNA *out)
{
  dnaOrAaFilter(in, out, aaChars);
}



/** 
 * Count up frequency of occurance of each base and store results in histogram. 
 */
void seq_dnaBaseHistogram(DNA *dna, int dnaSize, int histogram[4])
{
  int val;
  zeroBytes(histogram, 4*sizeof(int));
  while (--dnaSize >= 0)
    {
      if ((val = ntVal[(int)*dna++]) >= 0)
        ++histogram[val];
    }
}



/**
 * Given a gap in genome from iStart to iEnd, return 1 for GT/AG intron between left and right, 
 * -1 for CT/AC, 0 for no intron. Assumes DNA is lower cased. 
 */
int seq_intronOrientation(DNA *iStart, DNA *iEnd)
{
  if (iEnd - iStart < 32)
    return 0;
  if (iStart[0] == 'g' && iStart[1] == 't' && iEnd[-2] == 'a' && iEnd[-1] == 'g')
    {
      return 1;
    }
  else if (iStart[0] == 'c' && iStart[1] == 't' && iEnd[-2] == 'a' && iEnd[-1] == 'c')
    {
      return -1;
    }
  else
    return 0;
}



/**
 * Compare two sequences (without inserts or deletions) and score. 
 */
int seq_dnaOrAaScoreMatch(char *a, char *b, int size, int matchScore, int mismatchScore, char ignore)
{
  int i;
  int score = 0;
  for (i=0; i<size; ++i)
    {
      char aa = a[i];
      char bb = b[i];
      if (aa == ignore || bb == ignore)
        continue;
      if (aa == bb)
        score += matchScore;
      else
        score += mismatchScore;
    }
  return score;
}



/* Tables to convert from 0-20 to ASCII single letter representation of proteins. */
int aaVal[256];
AA valToAa[20];
AA aaChars[256]; /* 0 except for value aa characters. Converts to upper case rest. */



struct aminoAcidTable
{
  int ix;
  char letter;
  char abbreviation[3];
  char *name;
};

struct aminoAcidTable aminoAcidTable[] = 
{
  {0, 'A', "ala", "alanine"},
  {1, 'C', "cys", "cysteine"},
  {2, 'D', "asp",  "aspartic acid"},
  {3, 'E', "glu",  "glutamic acid"},
  {4, 'F', "phe",  "phenylalanine"},
  {5, 'G', "gly",  "glycine"},
  {6, 'H', "his",  "histidine"},
  {7, 'I', "ile",  "isoleucine"},
  {8, 'K', "lys",  "lysine"},
  {9, 'L', "leu",  "leucine"},
  {10, 'M', "met", "methionine"},
  {11, 'N', "asn", "asparagine"},
  {12, 'P', "pro", "proline"},
  {13, 'Q', "gln", "glutamine"},
  {14, 'R', "arg", "arginine"},
  {15, 'S', "ser", "serine"},
  {16, 'T', "thr", "threonine"},
  {17, 'V', "val", "valine"},
  {18, 'W', "try", "tryptophan"},
  {19, 'Y', "tyr", "tyrosine"},
};


static void initAaVal()
/* Initialize aaVal and valToAa tables. */
{
  int i;
  char c, lowc;
  
  for (i=0; i<NUMELE(aaVal); ++i)
    aaVal[i] = -1;
  for (i=0; i<NUMELE(aminoAcidTable); ++i)
    {
      c = aminoAcidTable[i].letter;
      lowc = tolower(c);
      aaVal[(int)c] = aaVal[(int)lowc] = i;
      aaChars[(int)c] = aaChars[(int)lowc] = c;
      valToAa[i] = c;
    }
  aaChars['x'] = aaChars['X'] = 'X';
}



/**
 * Initialize the seq module. 
 */
void seq_init()
{
  static int opened = 0;
  if (!opened)
    {
      initNtVal();
      initAaVal();
      initNtChars();
      initNtMixedCaseChars();
      initNtCompTable();
      opened = 1;
    }
}

