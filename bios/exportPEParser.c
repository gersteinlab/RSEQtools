#include "format.h"
#include "log.h"
#include "linestream.h"
#include "common.h"
#include "exportPEParser.h"



/** 
 *   \file exportPEParser.c Module to parse *_export.txt files, i.e. output of Illumina GERALD/ELAND platform.
 *   \author Andrea Sboner (andrea.sboner@yale.edu)
 */


static LineStream ls1 = NULL;
static LineStream ls2 = NULL;


/**
 * Initialize the exportPEParser module from a file.
 * @param[in] fileName1 First-end file name
 * @param[in] fileName1 Second-end file name
 */
void exportPEParser_initFromFile (char *fileName1, char* fileName2)
{
  ls1 = ls_createFromFile (fileName1);
  ls2 = ls_createFromFile (fileName2);
}

/**
 * Initialize the exportPEParser module from a command/
 * @param[in] cmd1 command to be executed for the first end
 * @param[in] cmd2 command to be executed for the second end
 */
void exportPEParser_initFromPipe (char *cmd1, char* cmd2)
{
  ls1 = ls_createFromPipe ( cmd1 );
  ls2 = ls_createFromPipe ( cmd2 );
}



/**
 * Deinitialize the exportPEParser module.
 */
void exportPEParser_deInit (void)
{
  ls_destroy (ls1);
  ls_destroy (ls2);
}

static void exportPEParser_freeSingleEnd ( singleEnd *currEntry )
{
  if( currEntry == NULL )
    return;
  hlr_free( currEntry->machine          );
  hlr_free( currEntry->index            );
  hlr_free( currEntry->sequence         );
  hlr_free( currEntry->quality          );
  hlr_free( currEntry->chromosome       );
  hlr_free( currEntry->contig           );
  hlr_free( currEntry->match_descriptor );
  hlr_free( currEntry->partnerChromosome);
  hlr_free( currEntry->partnerContig    );
  freeMem( currEntry );
}

static void exportPEParser_freeEntry ( ExportPE* currExportEntry )
{
  if( currExportEntry == NULL )
    return;
  exportPEParser_freeSingleEnd( currExportEntry->end1 );
  exportPEParser_freeSingleEnd( currExportEntry->end2 );
  freeMem( currExportEntry );
}

static int exportPEParser_processSingleEndEntry( ExportPE* currExportEntry, int readNumber )
{
  char* line;
  WordIter w;
  static singleEnd* currEntry = NULL;
  if( readNumber == 1 ) {
    line = ls_nextLine( ls1 );
  } else 
    line = ls_nextLine( ls2 );
  if( ls_isEof( ls1 ) || ls_isEof( ls2 ) ) 
    return 0;  // no more entries
  AllocVar( currEntry );
  w = wordIterCreate (line,"\t",0);
  currEntry->machine = hlr_strdup( wordNext( w ) );
  currEntry->run_number = atoi( wordNext( w ) );
  currEntry->lane = atoi( wordNext( w ) );
  currEntry->tile = atoi( wordNext( w ) );
  currEntry->xCoor = atoi( wordNext( w ) );
  currEntry->yCoor = atoi( wordNext( w ) );
  currEntry->index = hlr_strdup( wordNext ( w ) );
  currEntry->read_number =  atoi( wordNext ( w ) );
  currEntry->sequence = hlr_strdup( wordNext ( w ) );
  currEntry->quality = hlr_strdup( wordNext ( w ) );
  currEntry->chromosome =  hlr_strdup( wordNext ( w ) );
  currEntry->contig =  hlr_strdup( wordNext ( w ) );
  currEntry->position = atoi ( wordNext ( w ) );
  currEntry->strand =  wordNext ( w )[0] ;
  currEntry->match_descriptor =  hlr_strdup( wordNext ( w ) );
  currEntry->singleScore = atoi( wordNext ( w ) );
  currEntry->pairedScore = atoi( wordNext ( w ) );
  currEntry->partnerChromosome = hlr_strdup( wordNext ( w ) );
  currEntry->partnerContig = hlr_strdup( wordNext ( w ) );
  currEntry->partnerOffset = atoi( wordNext ( w ) );
  currEntry->partnerStrand = wordNext ( w )[0];
  currEntry->filter = wordNext( w )[0];
  if( readNumber == 1)
    currExportEntry->end1 = currEntry;
  else
    currExportEntry->end2 = currEntry;
  wordIterDestroy( w );
  return 1; // still more entries
}

static ExportPE* exportPEParser_processNextEntry ( int freeMemory ) 
{
  static ExportPE* currExportEntry = NULL;
  int moreEntries = 0;
  if( freeMemory ) {
    exportPEParser_freeEntry( currExportEntry );
  }
  AllocVar( currExportEntry );

  moreEntries += exportPEParser_processSingleEndEntry( currExportEntry, 1 );
  moreEntries += exportPEParser_processSingleEndEntry( currExportEntry, 2 );
 
  if( moreEntries > 1) {
    singleEnd* end1 = currExportEntry->end1;
    singleEnd* end2 = currExportEntry->end2;
    Stringa id1 = stringCreate(20);
    Stringa id2 = stringCreate(20);
    stringPrintf( id1, "%s:%d:%d:%d:%d:%d#%s", end1->machine, end1->run_number, end1->lane, end1->tile, end1->xCoor, end1->yCoor, end1->index);
    stringPrintf( id2, "%s:%d:%d:%d:%d:%d#%s", end2->machine, end2->run_number, end2->lane, end2->tile, end2->xCoor, end2->yCoor, end2->index);
    if( !strEqual( string(id1), string(id2) ) )
      die( "The IDs of the two entries do not match\n%s\n%s", exportPEParser_writeEntry( end1 ), exportPEParser_writeEntry( end2 ));
    stringDestroy( id1 );
    stringDestroy( id2 );
    return currExportEntry;
  } else {
    if (moreEntries == 0 )
      return NULL;
    else {
      die("The export files do not have the same length");
      return NULL;
    }
  }
}

/**
 * Read an entry in the export file;
 */
ExportPE* exportPEParser_nextEntry (void)
{
  return exportPEParser_processNextEntry ( 1 );
}

/** 
 * Write an export entry;
 * @param [in] currEntry: a pointer to the single end entry
 * @return string formatted as an export file
 */
char *exportPEParser_writeEntry ( singleEnd* currEntry )
{
  static Stringa buffer = NULL;
  stringCreateClear( buffer, 100 );
  stringPrintf( buffer, "%s\t%d\t%d\t%d\t%d\t%d\t%s\t%d\t%s\t%s\t%s\t%s\t%d\t%c\t%s\t%d\t%d\t%s\t%s\t%d\t%c\t%c",
		currEntry->machine,
		currEntry->run_number,
		currEntry->lane,
		currEntry->tile,
		currEntry->xCoor,
		currEntry->yCoor,
		currEntry->index,
		currEntry->read_number,
		currEntry->sequence,
		currEntry->quality,
		currEntry->chromosome,
		currEntry->contig,
		currEntry->position,
		currEntry->strand,
		currEntry->match_descriptor,
		currEntry->singleScore,
		currEntry->pairedScore,
		currEntry->partnerChromosome,
		currEntry->partnerContig,
		currEntry->partnerOffset,
		currEntry->partnerStrand,
		currEntry->filter);
  return string( buffer );
}
