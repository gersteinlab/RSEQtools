#include "log.h"
#include "format.h"
#include "mrf.h"
#include "fasta.h"
#include "seq.h"

/** 
 *   \file mrfValidate.c Module to validate MRF files.
 */

int processBlock ( MrfBlock* currBlock, MrfBlock* prevBlock, int gaps ) {
  int errorCode = 0;
  if( currBlock->queryEnd < currBlock->queryStart )
    errorCode += 1;
  if( currBlock->targetEnd < currBlock->targetStart )
    errorCode += 2;
  int querySize  = currBlock->queryEnd  - currBlock->queryStart  + 1;
  int targetSize = currBlock->targetEnd - currBlock->targetStart + 1;
  if( targetSize != querySize )
    errorCode += 4;

  if( prevBlock != NULL ) {
    if( !strEqual( prevBlock->targetName, currBlock->targetName) )
      errorCode += 8;
    if( prevBlock->strand != currBlock->strand )
      errorCode += 16;
    if( currBlock->queryStart <= prevBlock->queryStart || currBlock->queryEnd <= prevBlock->queryEnd )
      errorCode += 32;
    if( currBlock->targetStart <= prevBlock->targetStart || currBlock->targetEnd <= prevBlock->targetEnd )
      errorCode += 64;
    if( gaps == 0 ) {
      if( (currBlock->queryStart - prevBlock->queryEnd ) != 1 )
	errorCode += 128;    
    }
  }
  return errorCode;
}

int main (int argc, char *argv[]) 
{
  if( argc != 3 ) {
    usage( "%s <errors|noerrors> <gaps|nogaps>", argv[0]);
    return -1;
  }
  MrfEntry *currEntry;
  MrfBlock *currBlock;
  int i, error, errorCode, numErrors;
  int gaps = 0;
  if( strEqual( "gaps", argv[2] ) )
    gaps = 1;
  seq_init();
  mrf_init ("-");
  printf( "%s\n", mrf_writeHeader());
  numErrors = 0;
  while (currEntry = mrf_nextEntry ()) {
    error = 0;
    for( i=0; i<arrayMax(currEntry->read1.blocks); i++) {   
      currBlock=arrp( currEntry->read1.blocks, i, MrfBlock);
      if( i==0 ) {
	errorCode = processBlock ( currBlock, NULL, gaps );
      } else {
	errorCode = processBlock ( currBlock, arrp( currEntry->read1.blocks, i-1, MrfBlock ), gaps );
      }
      if(  errorCode != 0 ) {
	warn( "Error code (read1): %d\t%s:%c:%d:%d:%d:%d", errorCode, currBlock->targetName, currBlock->strand, currBlock->targetStart, 
	      currBlock->targetEnd, currBlock->queryStart, currBlock->queryEnd);	
	error++;
      }
    }
    if (currEntry->isPairedEnd & error == 0) {
      for( i=0; i<arrayMax(currEntry->read2.blocks); i++) {
	currBlock=arrp( currEntry->read2.blocks, i, MrfBlock);
	if( i==0 ) {
	  errorCode = processBlock ( currBlock, NULL , gaps ); 
	} else {
	  errorCode = processBlock (currBlock, arrp( currEntry->read2.blocks, i-1, MrfBlock) , gaps);
	}
	if( errorCode !=0 ) {
	  warn( "Error code (read2): %d\t%s:%c:%d:%d:%d:%d", errorCode, currBlock->targetName, currBlock->strand, currBlock->targetStart, 
		currBlock->targetEnd, currBlock->queryStart, currBlock->queryEnd);
	  error++;
	}
      }
    }
    if( error > 0 && strEqual(argv[1], "errors")) {
      printf( "%s\n", mrf_writeEntry( currEntry ));
      error=0;
      numErrors++;
    } 
    if( error == 0 && strEqual(argv[1], "noerrors")) 
      printf( "%s\n", mrf_writeEntry( currEntry ));
  }
  mrf_deInit ();
  warn("%s: done", argv[0]);
  return 0;

}
