#include "log.h"
#include "format.h"
#include "mrf.h"
#include "bowtieParser.h"
#include "stringUtil.h"
#include "common.h"
#include "seq.h"



/** 
 *   \file bowtie2mrf.c Module to convert Bowtie format into MRF.
 *         Run bowtie2mrf to see command line details. Takes bowtie output from STDIN. \n
 *         Note that in order to be run in paired mode, bowtie must have been ran with -k 1 and -m 1. \n 
 */



#define MODE_GENOMIC 1
#define MODE_JUNCTIONS 2
#define MODE_PAIRED 3



static void printAlignmentBlock (char *chromosome, char strand, int targetStart, int targetEnd, int queryStart, int queryEnd) 
{
  printf ("%s:%c:%d:%d:%d:%d",
          chromosome,
          strand,
          targetStart,
          targetEnd,
          queryStart,
          queryEnd);
}



static void  printReadSequence ( char* sequence, char strand, Stringa buffer ) 
{
  if( strand == '-' ) {
    seq_reverseComplement( sequence, strlen( sequence ));
  }
  stringPrintf( buffer, "%s", sequence);
}



static void printPairedData (BowtieEntry* prevEntry, BowtieEntry* currEntry, Stringa bufferSequence, Stringa bufferQuality, int sequenceLength) 
{
  static char* copy = NULL;
  WordIter w;
  char* chromosome1 = NULL; 
  char* chromosome2 = NULL;
  int startFirstExon, startSecondExon, sizeExonOverlap, numNucleotidesFirstExon, numNucleotidesSecondExon, start1,start2;
  Stringa block1 = stringCreate(50);
  Stringa block2 = stringCreate(50);
  int order;

  if( prevEntry->strand == '-' ) seq_reverseComplement( prevEntry->sequence, strlen( prevEntry->sequence ) );
  if( currEntry->strand == '-' ) seq_reverseComplement( currEntry->sequence, strlen( currEntry->sequence ) );
  if( strchr( prevEntry->chromosome, '|')  ) { // junction the first element
      strReplace (&copy,prevEntry->chromosome);
      w = wordIterCreate (copy,"|",0);
      strReplace (&chromosome1,wordNext (w));
      startFirstExon = atoi (wordNext (w));
      startSecondExon = atoi (wordNext (w));
      sizeExonOverlap = atoi (wordNext (w));
      wordIterDestroy (w);
      numNucleotidesFirstExon = sizeExonOverlap - prevEntry->position + 1;
      numNucleotidesSecondExon = sequenceLength - numNucleotidesFirstExon;
      stringPrintf (block1, "%s:%c:%d:%d:%d:%d,%s:%c:%d:%d:%d:%d",
		    chromosome1,
		    prevEntry->strand,
		    startFirstExon + 1 + prevEntry->position - 1,
		    startFirstExon + 1 + sizeExonOverlap - 1,     
		    1,                                           
		    numNucleotidesFirstExon,
		    chromosome1,
		    prevEntry->strand,
		    startSecondExon + 1,
		    startSecondExon + 1 + numNucleotidesSecondExon - 1,
		    numNucleotidesFirstExon + 1,
		    sequenceLength);
      start1 = startFirstExon;
  } else {
    strReplace( &chromosome1, prevEntry->chromosome );
    start1 = prevEntry->position;
    stringPrintf( block1, "%s:%c:%d:%d:%d:%d",
		  prevEntry->chromosome,
		  prevEntry->strand,
		  prevEntry->position,
		  prevEntry->position + ((int)strlen(prevEntry->sequence)) - 1,
		  1,
		  (int) strlen( prevEntry->sequence ));
  }
  if( strchr( currEntry->chromosome, '|')  ) { // junction the second element
      strReplace (&copy,currEntry->chromosome);
      w = wordIterCreate (copy,"|",0);
      strReplace (&chromosome2,wordNext (w));
      startFirstExon = atoi (wordNext (w));
      startSecondExon = atoi (wordNext (w));
      sizeExonOverlap = atoi (wordNext (w));
      wordIterDestroy (w);
      numNucleotidesFirstExon = sizeExonOverlap - currEntry->position + 1;
      numNucleotidesSecondExon = sequenceLength - numNucleotidesFirstExon;
      stringPrintf( block2,"%s:%c:%d:%d:%d:%d,%s:%c:%d:%d:%d:%d",
		    chromosome2,
		    currEntry->strand,
		    startFirstExon + 1 + currEntry->position - 1,
		    startFirstExon + 1 + sizeExonOverlap - 1,     
		    1,                                           
		    numNucleotidesFirstExon,
		    chromosome2,
		    currEntry->strand,
		    startSecondExon + 1,
		    startSecondExon + 1 + numNucleotidesSecondExon - 1,
		    numNucleotidesFirstExon + 1,
		    sequenceLength);
      start2 = startFirstExon;
  } else {
    strReplace( &chromosome2, currEntry->chromosome );
    start2 = currEntry->position;
    stringPrintf( block2, "%s:%c:%d:%d:%d:%d",
		  currEntry->chromosome,
		  currEntry->strand,
		  currEntry->position,
		  currEntry->position + sequenceLength - 1,
		  1,
		  sequenceLength);
  }

  order = strcmp( chromosome1, chromosome2 );
  if( order == 0 ) order = start1 - start2;
  if( order < 0 ) {
    printf( "%s|%s", string(block1), string(block2) );
    stringPrintf( bufferSequence, "%s|%s", prevEntry->sequence, currEntry->sequence);
    stringPrintf( bufferQuality,  "%s|%s", prevEntry->quality,  currEntry->quality);
  } else {
    printf( "%s|%s", string(block2), string(block1) );
    stringPrintf( bufferSequence, "%s|%s", currEntry->sequence, prevEntry->sequence);
    stringPrintf( bufferQuality,  "%s|%s", currEntry->quality,  prevEntry->quality);
  }
  stringDestroy( block1 );
  stringDestroy( block2 );
}



int main (int argc, char *argv[])
{
  BowtieQuery *currQuery;
  BowtieEntry *currEntry;
  BowtieQuery *prevQuery=NULL;
  int i;
  int mode;
  int includeSequences,includeQualityScores, includeIDs;
  int sequenceLength;
  WordIter w;
  static char *copy = NULL;
  static char *chromosome = NULL;
  int startFirstExon;
  int startSecondExon;
  int sizeExonOverlap;
  int numNucleotidesFirstExon;
  int numNucleotidesSecondExon;
  Stringa bufferSequence = stringCreate(50);      
  Stringa bufferQuality  = stringCreate(50);      
  Stringa bufferID       = stringCreate(50);
  Stringa tmp            = stringCreate(50);

  if (argc < 2) {
    usage ("%s <genomic|junctions|paired> [-sequence] [-qualityScores] [-IDs]",argv[0]);
  }
  seq_init();

  if (strEqual ("genomic",argv[1])) {
    mode = MODE_GENOMIC;
  } 
  else if (strEqual ("junctions",argv[1])) {
    mode = MODE_JUNCTIONS;
  }
  else if (strEqual ("paired", argv[1])) {
    mode = MODE_PAIRED;
  }
  else {
    usage ("%s genomic|junctions|paired> [-sequence] [-qualityScores] [-IDs]",argv[0]);
  }
 
  includeSequences = 0;
  includeQualityScores = 0;
  includeIDs = 0;
  i = 2;
  while (i < argc) {
    if (strEqual (argv[i],"-sequence")) {
      includeSequences = 1;      
    }
    if (strEqual (argv[i],"-qualityScores")) {
      includeQualityScores = 1; 
    }
    if (strEqual (argv[i],"-IDs")) {
      includeIDs = 1; 
    }
    i++;
  }
  printf ("%s",MRF_COLUMN_NAME_BLOCKS);
  if (includeSequences == 1) {
    printf ("\t%s",MRF_COLUMN_NAME_SEQUENCE);
  }
  if (includeQualityScores == 1) {
    printf ("\t%s",MRF_COLUMN_NAME_QUALITY_SCORES);
  }
  if (includeIDs == 1) {
    printf ("\t%s",MRF_COLUMN_NAME_QUERY_ID);
  }
  puts ("");
  
  bowtieParser_initFromFile ("-");
  while (currQuery = bowtieParser_nextQuery ()) {
    if (arrayMax (currQuery->entries) != 1) {
      continue;
    }
    stringPrintf( bufferID,"%s", currQuery->sequenceName ); 
    currEntry = arrp (currQuery->entries,0,BowtieEntry);
    sequenceLength = (int)strlen (currEntry->sequence);
    switch (mode) {
    case MODE_GENOMIC: 
      printAlignmentBlock (currEntry->chromosome,
                           currEntry->strand,
                           currEntry->position,
                           currEntry->position + sequenceLength - 1,
                           1,
                           sequenceLength);
      printReadSequence( currEntry->sequence, currEntry->strand, bufferSequence );
      stringPrintf( bufferQuality,  "%s", currEntry->quality ); 
      break;
    case  MODE_JUNCTIONS: 
      strReplace (&copy,currEntry->chromosome);
      if (countChars (currEntry->chromosome,'|') != 3) {
        die ("Expected 3 '|' characters in the targetName for spliced reads: %s",currEntry->chromosome);
      }
      w = wordIterCreate (copy,"|",0);
      strReplace (&chromosome,wordNext (w));
      startFirstExon = atoi (wordNext (w));
      startSecondExon = atoi (wordNext (w));
      sizeExonOverlap = atoi (wordNext (w));
      wordIterDestroy (w);
      numNucleotidesFirstExon = sizeExonOverlap - currEntry->position + 1;
      numNucleotidesSecondExon = sequenceLength - numNucleotidesFirstExon;
      printAlignmentBlock (chromosome,
                           currEntry->strand,
                           startFirstExon + 1 + currEntry->position - 1,
                           startFirstExon + 1 + sizeExonOverlap - 1,     
                           1,                                           
                           numNucleotidesFirstExon);                     
      printf (",");
      printAlignmentBlock (chromosome,
                           currEntry->strand,
                           startSecondExon + 1,
                           startSecondExon + 1 + numNucleotidesSecondExon - 1,
                           numNucleotidesFirstExon + 1,
                           sequenceLength);
      printReadSequence( currEntry->sequence, currEntry->strand, bufferSequence );
      stringPrintf( bufferQuality,  "%s", currEntry->quality );   
      break;
    case MODE_PAIRED:  
      bowtieParser_copyQuery( &prevQuery, currQuery );
      currQuery = bowtieParser_nextQuery();
      if (arrayMax (currQuery->entries) != 1) {
	continue;
      }
      char *prevID = hlr_strdup( prevQuery->sequenceName );
      char *p = rindex( prevID, '/' );
      if( p == NULL ) die( "No '/' in the ID name as expected from bowtie output for paired end mapping.");
      *p='\0';
      char *currID = hlr_strdup( currQuery->sequenceName );
      p = rindex (currID, '/' );
      if( p == NULL ) die( "No '/' in the ID name as expected from bowtie output for paired end mapping.");
      *p = '\0';
      if( strEqual( prevID, currID ) ) {
	BowtieEntry *prevEntry = arrp( prevQuery->entries, 0, BowtieEntry );
	currEntry = arrp( currQuery->entries, 0, BowtieEntry);
	printPairedData( prevEntry, currEntry, bufferSequence, bufferQuality, sequenceLength );
      }	else {
	die("Expected to be equal: %s\t%s\nNote: run bowtie allowing only one hit", prevQuery->sequenceName, currQuery->sequenceName);
      }
      hlr_free(prevID);
      hlr_free(currID);
      bowtieParser_freeQuery( prevQuery );
      break;
    default:
      die ("Unknown mode");
    }
    if (includeSequences == 1) {
      printf ("\t%s",string( bufferSequence ));
    }
    if (includeQualityScores == 1) {
      printf ("\t%s",string( bufferQuality) );
    }
    if (includeIDs == 1) {
      if( mode==MODE_PAIRED ) {
        if( strEndsWith( string(bufferID), "/1") || strEndsWith( string(bufferID), "/2")) 
          stringChop( bufferID, 2);
        stringPrintf( tmp, "%s|%s", string( bufferID ), string( bufferID ) );
        stringPrintf( bufferID, "%s", string(tmp) );
	}
      printf ("\t%s",string( bufferID) );
    }
    puts ("");
  } 
  stringDestroy( bufferSequence );
  stringDestroy( bufferQuality );
  stringDestroy( bufferID );
  stringDestroy( tmp );
  bowtieParser_deInit ();
  return 0;
}

