#include "format.h"
#include "log.h"
#include "linestream.h"
#include "mrf.h"
#include "sam.h"
#include <stdlib.h>
#include "seq.h"


/** 
 *   \file mrf2sam.c Module to convert MRF to SAM.
 */



#define S_PLUS	0
#define S_MINUS	1


char* rev(char* str)
{
  int end= strlen(str)-1;
  int start = 0;

  while( start<end )
  {
    str[start] ^= str[end];
    str[end] ^=   str[start];
    str[start]^= str[end];

    ++start;
    --end;
  }

  return str;
}


static int getFivep (MrfRead *read, int strand)
{
  MrfBlock *block;
  if (strand == S_PLUS) {
    block = arrp (read->blocks, 0, MrfBlock);
    return block->targetStart;
  } else {
    block = arrp (read->blocks, arrayMax(read->blocks) - 1, MrfBlock);
    return block->targetEnd;
  }
}



static void printSamE (MrfEntry *entry, int i)
{
  // If the read is a pair-end read
  if (entry->isPairedEnd) {
    Stringa qname;
    int flags1, flags2;
    Stringa cigar1, cigar2;
    int pos1, pos2;		// Leftmost positino
    int mapq1, mapq2;
    char *rname1, *rname2;
    char *seq1, *seq2;
    char *qual1, *qual2;
    int isize1, isize2;
    int strand1, strand2;

    MrfBlock *block1 = arrp (entry->read1.blocks, 0, MrfBlock);
    MrfBlock *block2 = arrp (entry->read2.blocks, 0, MrfBlock);

    flags1 = flags2 = S_READ_PAIRED | S_PAIR_MAPPED;
    flags1 |= S_FIRST;
    flags2 |= S_SECOND;

    strand1 = strand2 = S_PLUS;
    if (block1->strand == '-') {
      flags1 |= S_QUERY_STRAND;
      strand1 = S_MINUS;
    }
    if (block2->strand == '-') {
      flags2 |= S_MATE_STRAND;
      strand2 = S_MINUS;
    }

    isize1 = getFivep(&(entry->read2), strand2) -
             getFivep(&(entry->read1), strand1);
    isize2 = isize1 * (-1);

    pos1 = block1->targetStart;
    pos2 = block2->targetStart;
    cigar1 = genCigar(&(entry->read1));
    cigar2 = genCigar(&(entry->read2));
    rname1 = strdup(block1->targetName);
    rname2 = strdup(block2->targetName);

    // Generate query names
    qname = stringCreate (20);
    stringPrintf (qname, "%s_%i_%i_%i", rname1, pos1, pos2, i);

    // Map qualities
    // Temporary 255 for both for now
    mapq1 = mapq2 = 255;

    // Quality scores, if given
    if (entry->read1.qualityScores) {
      qual1 = strdup (entry->read1.qualityScores);
      if( strand1 == S_MINUS )
	qual1=rev( qual1 );
    }
    else
    	qual1 = strdup ("*");
    if (entry->read2.qualityScores) {
      qual2 = strdup (entry->read2.qualityScores);
      if( strand2 == S_MINUS )
	qual2=rev( qual2 );
    } else
    	qual2 = strdup ("*");

    // Sequences, if given
    if (entry->read1.sequence) {
      seq1 = strdup (entry->read1.sequence);
      if( strand1 == S_MINUS )
	seq_reverseComplement( seq1, strlen( seq1 ) );    	
    } else 
      seq1 = strdup ("*");
    
    if (entry->read2.sequence) {
      seq2 = strdup (entry->read2.sequence);
      if( strand2 == S_MINUS )
	seq_reverseComplement( seq2, strlen( seq2 ) );    
    }
    else
      seq2 = strdup ("*");

    printf ("%s\t%i\t%s\t%i\t%i\t%s\t%s\t%i\t%i\t%s\t%s\n",
            string(qname), flags1, rname1, pos1, mapq1, string(cigar1),
            rname2, pos2,
            isize1, seq1, qual1);
    printf ("%s\t%i\t%s\t%i\t%i\t%s\t%s\t%i\t%i\t%s\t%s\n",
            string(qname), flags2, rname2, pos2, mapq2, string(cigar2),
            rname1, pos1,
            isize2, seq2, qual2);

    free (qual1);
    free (qual2);
    free (seq1);
    free (seq2);
    free (rname1);
    free (rname2);
    stringDestroy (cigar1);
    stringDestroy (cigar2);
    stringDestroy (qname);

  // If the entry is only a single read
  } else {
    char *rname;
    Stringa qname;
    int flags = 0;
    Stringa cigar;
    int pos;
    int mapq;
    char *seq;
    char *qual;

    MrfBlock *block = arrp (entry->read1.blocks, 0, MrfBlock);

    cigar = genCigar(&(entry->read1));
    pos = block->targetStart;
    if (block->strand == '+') flags |= S_QUERY_STRAND;

    // Mapping quality
    // Temporary as 255
    mapq = 255;

    rname = strdup (block->targetName);

    qname = stringCreate (20);
    stringPrintf (qname, "%s_%i_0_%i", rname, pos, i);

    if (entry->read1.qualityScores)
    	qual = strdup (entry->read1.qualityScores);
    else
    	qual = strdup ("*");

    if (entry->read1.sequence) {   
      seq = strdup (entry->read1.sequence);
      if( block->strand == '-' )
	seq_reverseComplement( seq, strlen( seq ) );
    }
    else
      seq = strdup ("*");
    
    printf ("%s\t%i\t%s\t%i\t%i\t%s\t*\t0\t0\t%s\t%s\n",
            string(qname), flags, rname, pos, mapq, string(cigar),
            seq, qual);

    free (qual);
    free (seq);
    free (rname);
    stringDestroy (cigar);
    stringDestroy (qname);
  }
}



int main (int argc, char **argv)
{
  int i = 0;
  MrfEntry *currMRFE;
  seq_init();
  mrf_init ("-"); 
  while (currMRFE = mrf_nextEntry()) {
    printSamE(currMRFE, i);
    i++;
  }
  mrf_deInit ();
  return EXIT_SUCCESS;
}
