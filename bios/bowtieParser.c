#include "format.h"
#include "log.h"
#include "linestream.h"
#include "common.h"
#include "bowtieParser.h"



/** 
 *   \file bowtieParser.c Module to parse bowtie output files
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



static LineStream ls = NULL;



/**
 * Initialize the bowtieParser module from file.
 * @param[in] fileName File name, use "-" to denote stdin
 */
void bowtieParser_initFromFile (char *fileName)
{
  ls = ls_createFromFile (fileName);
  ls_bufferSet (ls,1);
}



/**
 * Initialize the bowtieParser module from pipe.
 * @param[in] command Command to be executed
 */
void bowtieParser_initFromPipe (char *command)
{
  ls = ls_createFromPipe (command);
  ls_bufferSet (ls,1);
}



/**
 * Deinitialize the bowtieParser module.
 */
void bowtieParser_deInit (void)
{
  ls_destroy (ls);
}



/**
 * Deinitialize the BowtieQuery.
 */
void bowtieParser_freeQuery (BowtieQuery *currBowtieQuery) 
{
  int i;
  BowtieEntry *currBowtieEntry;

  if (currBowtieQuery == NULL) {
    return;
  }
  hlr_free (currBowtieQuery->sequenceName);
  if (currBowtieQuery->entries != NULL) {
    for (i = 0; i < arrayMax (currBowtieQuery->entries); i++) {
      currBowtieEntry = arrp (currBowtieQuery->entries,i,BowtieEntry);
      hlr_free (currBowtieEntry->chromosome);
      hlr_free (currBowtieEntry->sequence);
      hlr_free (currBowtieEntry->quality);
      arrayDestroy (currBowtieEntry->mismatches);
    }
    arrayDestroy (currBowtieQuery->entries);
  }
  freeMem (currBowtieQuery);
}

 

static void bowtieParser_copyEntry (BowtieEntry *dest, BowtieEntry *orig) 
{
  int i;
  BowtieMismatch *origMismatch,*destMismatch;
  
  dest->mismatches = arrayCreate (arrayMax (orig->mismatches) > 0 ? arrayMax (orig->mismatches) : 1,BowtieMismatch);
  for (i = 0; i < arrayMax (orig->mismatches); i++) {
    origMismatch = arrp (orig->mismatches,i,BowtieMismatch);
    destMismatch = arrayp (dest->mismatches,arrayMax (dest->mismatches),BowtieMismatch);
    destMismatch->offset = origMismatch->offset;
    destMismatch->referenceBase = origMismatch->referenceBase;
    destMismatch->readBase = origMismatch->readBase;
  }
  dest->chromosome = hlr_strdup (orig->chromosome);
  dest->sequence = hlr_strdup (orig->sequence);
  dest->quality = hlr_strdup (orig->quality);
  dest->position = orig->position;
  dest->strand = orig->strand;
}



/**
 * Make a copy of a BowtieQuery.
 * @pre *dest
 * @post Use bowtieParser_freeQuery to de-allocate the memory
 */
void bowtieParser_copyQuery (BowtieQuery **dest, BowtieQuery *orig) 
{ 
  int i;

  AllocVar (*dest);
  (*dest)->sequenceName = hlr_strdup (orig->sequenceName);
  (*dest)->entries = arrayCreate (arrayMax (orig->entries),BowtieEntry);
  for (i = 0; i < arrayMax (orig->entries); i++) {
    bowtieParser_copyEntry(arrayp ((*dest)->entries,arrayMax ((*dest)->entries),BowtieEntry), 
                           arrp (orig->entries,i,BowtieEntry));
  }
}



static void bowtieParser_processMismatches (BowtieEntry *currEntry, char* token) 
{
  WordIter w;
  BowtieMismatch *currBowtieMismatch;
  char *item,*pos;

  currEntry->mismatches = arrayCreate (3,BowtieMismatch);
  if (token[0] == '\0') {
    return;
  }
  w = wordIterCreate (token,",",0);
  while (item = wordNext (w)) {
    currBowtieMismatch = arrayp (currEntry->mismatches,arrayMax (currEntry->mismatches),BowtieMismatch);
    pos = strchr (item,':');
    *pos = '\0';
    currBowtieMismatch->offset = atoi (item);
    currBowtieMismatch->referenceBase = *(pos + 1);
    currBowtieMismatch->readBase = *(pos + 3);
  }
  wordIterDestroy (w);
}



static void bowtieParser_processLine (char* line, BowtieQuery* currBowtieQuery) 
{
  WordIter w;
  BowtieEntry *currEntry;

  currEntry = arrayp (currBowtieQuery->entries,arrayMax (currBowtieQuery->entries),BowtieEntry);
  w = wordIterCreate (line,"\t",0);
  currEntry->strand = (wordNext (w))[0];
  currEntry->chromosome = hlr_strdup (wordNext (w));
  currEntry->position = atoi (wordNext (w));
  currEntry->sequence = hlr_strdup (wordNext (w));
  currEntry->quality = hlr_strdup (wordNext (w));
  wordNext (w);
  bowtieParser_processMismatches (currEntry,wordNext (w));
  wordIterDestroy (w);
}



static BowtieQuery* bowtieParser_processNextQuery (int freeMemory)
{
  char *line,*pos;
  static char *queryName = NULL;
  static char *prevBowtieQueryName = NULL;
  static BowtieQuery *currBowtieQuery = NULL;
  int first;

  if (!ls_isEof (ls)) {
    if (freeMemory) {
      bowtieParser_freeQuery (currBowtieQuery);
      currBowtieQuery = NULL;
    }
    AllocVar (currBowtieQuery);
    currBowtieQuery->entries = arrayCreate (5,BowtieEntry);
    first = 1;
    while (line = ls_nextLine (ls)) {
      if (line[0] == '\0') {
	continue;
      }
      pos = strchr (line,'\t');
      *pos = '\0';
      strReplace (&queryName,line);
      if (first == 1 || strEqual (prevBowtieQueryName,queryName)) {
	bowtieParser_processLine (pos + 1,currBowtieQuery); 
      }
      else {
	ls_back (ls,1);
	return currBowtieQuery;
      }
      if (first == 1) {
	currBowtieQuery->sequenceName = hlr_strdup (queryName);
	first = 0;
      }
      strReplace(&prevBowtieQueryName,queryName);
    }
    if (first == 1) {
      return NULL;
    }
    else {
      return currBowtieQuery;
    }
  }
  if (freeMemory) {
    bowtieParser_freeQuery (currBowtieQuery);
    currBowtieQuery = NULL;
  }
  return NULL;  
}



/**
 * Returns a pointer to next BowtieQuery. 
 * @pre The module has been initialized using bowtieParser_init().
 * Parse entries of the following format:
   \verbatim
  
   Output (obtained from running bowtie -h)
   ----------------------------------------

   The 'bowtie' aligner outputs each alignment on a separate line.  Each
   line is a collection of 8 fields separated by tabs; from left to
   right, the fields are:

    1. Name of read that aligned

    2. Orientation of read in the alignment, '-' for reverse complement,
       '+' otherwise

    3. Name of reference sequence where alignment occurs, or ordinal ID
       if no name was provided

    4. 0-based offset into the reference sequence where leftmost
       character of the alignment occurs

    5. Read sequence (reverse-complemented if orientation is '-')

    6. Read qualities (reversed if orientation is '-')

    7. Reserved

    8. Comma-separated list of mismatch descriptors.  If there are no
       mismatches in the alignment, this field is empty.  A single
       descriptor has the format offset:reference-base>read-base.  The
       offset is expressed as a 0-based offset from the high-quality
       (5') end of the read. 


    Example:

    FC20B5TAA_50708:2:1:246:339 	-	chr8	74005895	TAGATGTGTGGTATTATTTCTGAGGGC	IIIIIIIIIIIIIIIIIIIIIIIIIII	785	
    FC20B5TAA_50708:2:1:246:339 	-	chr16	80796190	TAGATGTGTGGTATTATTTCTGAGGGC	IIIIIIIIIIIIIIIIIIIIIIIIIII	785	
    FC20B5TAA_50708:2:1:624:321 	+	chr1	240849136	GGCTTAAAAGCTAGATGACGGGGTGAG	IIIIIIIIIIIIIIIIIIIIIIIIIII	0	9:C>G,26:T>G
    FC20B5TAA_50708:2:1:624:321 	-	chrX	98759270	CTCACCCCGTCATCTAGCTTTTAAGCC	IIIIIIIIIIIIIIIIIIIIIIIIIII	2	22:A>C,26:A>C
    FC20B5TAA_50708:2:1:624:321 	-	chr10	84291740	CTCACCCCGTCATCTAGCTTTTAAGCC	IIIIIIIIIIIIIIIIIIIIIIIIIII	2	22:A>C,26:A>C
    FC20B5TAA_50708:2:1:624:321 	-	chr4	93757480	CTCACCCCGTCATCTAGCTTTTAAGCC	IIIIIIIIIIIIIIIIIIIIIIIIIII	2	22:A>C,26:A>C
    FC20B5TAA_50708:2:1:624:321 	-	chr2	57057350	CTCACCCCGTCATCTAGCTTTTAAGCC	IIIIIIIIIIIIIIIIIIIIIIIIIII	0	22:A>C,24:T>C
    FC20B5TAA_50708:2:1:624:321 	+	chr2	178610898	GGCTTAAAAGCTAGATGACGGGGTGAG	IIIIIIIIIIIIIIIIIIIIIIIIIII	0	22:T>G,26:T>G
    FC20B5TAA_50708:2:1:624:321 	-	chr4	31274212	CTCACCCCGTCATCTAGCTTTTAAGCC	IIIIIIIIIIIIIIIIIIIIIIIIIII	0	9:G>C,26:A>C
    FC20B5TAA_50708:2:1:624:321 	-	chr20	35191648	CTCACCCCGTCATCTAGCTTTTAAGCC	IIIIIIIIIIIIIIIIIIIIIIIIIII	2	9:G>C,22:A>C

   \endverbatim
 */
BowtieQuery* bowtieParser_nextQuery (void)
{
  return bowtieParser_processNextQuery (1);
}



/**
 * Returns an Array of BowtieQueries.
 * @note The memory belongs to this routine.
 */
Array  bowtieParser_getAllQueries ()
{
  Array bowtieQueries;
  BowtieQuery *currBowtieQuery;

  bowtieQueries = arrayCreate (100000,BowtieQuery);
  while (currBowtieQuery = bowtieParser_processNextQuery (0)) {
    array (bowtieQueries,arrayMax (bowtieQueries),BowtieQuery) = *currBowtieQuery;
    freeMem (currBowtieQuery);
  }
  return bowtieQueries;
}
