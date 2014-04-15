#include "log.h"
#include "format.h"
#include "linestream.h"
#include "common.h"
#include "bits.h"
#include "mrf.h"



/** 
 *   \file mrf.c Module to parse mapped read format
 */



#define INIT_MODE_FROM_FILE 1
#define INIT_MODE_FROM_PIPE 2



static LineStream lsMrf = NULL;
static Bits *presentColumnTypes = NULL;
static Array columnTypes = NULL;
static Texta columnHeaders = NULL;
static Texta comments = NULL;
static char *headerLine = NULL;



static void mrf_addColumnType (char *type)
{
  if (strEqual (type,MRF_COLUMN_NAME_BLOCKS)) {
    bitSetOne (presentColumnTypes,MRF_COLUMN_TYPE_BLOCKS);
    array (columnTypes,arrayMax (columnTypes),int) = MRF_COLUMN_TYPE_BLOCKS;
    textAdd (columnHeaders,MRF_COLUMN_NAME_BLOCKS);
  }
  else if (strEqual (type,MRF_COLUMN_NAME_SEQUENCE)) {
    bitSetOne (presentColumnTypes,MRF_COLUMN_TYPE_SEQUENCE);
    array (columnTypes,arrayMax (columnTypes),int) = MRF_COLUMN_TYPE_SEQUENCE;
    textAdd (columnHeaders,MRF_COLUMN_NAME_SEQUENCE);
  }
  else if (strEqual (type,MRF_COLUMN_NAME_QUALITY_SCORES)) {
    bitSetOne (presentColumnTypes,MRF_COLUMN_TYPE_QUALITY_SCORES);
    array (columnTypes,arrayMax (columnTypes),int) = MRF_COLUMN_TYPE_QUALITY_SCORES;
    textAdd (columnHeaders,MRF_COLUMN_NAME_QUALITY_SCORES);
  }
  else if (strEqual (type,MRF_COLUMN_NAME_QUERY_ID)) {
    bitSetOne (presentColumnTypes,MRF_COLUMN_TYPE_QUERY_ID);
    array (columnTypes,arrayMax (columnTypes),int) = MRF_COLUMN_TYPE_QUERY_ID;
    textAdd (columnHeaders,MRF_COLUMN_NAME_QUERY_ID);
  }
  else {
    die ("Unknown presentColumn: %s",type);
  }
}



static void mrf_doInit (char *arg, int initMode) 
{
  Texta tokens;
  char *line;
  int i;

  columnTypes = arrayCreate (20,int);
  columnHeaders = textCreate (20);
  presentColumnTypes = bitAlloc (100);
  comments = textCreate (100);
  if (initMode == INIT_MODE_FROM_FILE) {
    lsMrf = ls_createFromFile (arg);
  }
  else if (initMode == INIT_MODE_FROM_PIPE) {
    lsMrf = ls_createFromPipe (arg);
  }
  else {
    die ("Unknown init mode");
  }
  ls_bufferSet (lsMrf,1);
  while (line = ls_nextLine (lsMrf)) {
    if (line[0] == '#') {
      textAdd (comments,line + 1);
    }
    else {
      ls_back (lsMrf,1);
      break;
    }
  }
  headerLine = hlr_strdup (ls_nextLine (lsMrf));
  tokens = textFieldtokP (headerLine,"\t");
  for (i = 0; i < arrayMax (tokens); i++) {
    mrf_addColumnType (textItem (tokens,i));
  }
}



/**
 * Initialize the module module from a file.
 * @param[in] fileName File name, use "-" to denote stdin
 */
void mrf_init (char *fileName) 
{
  mrf_doInit (fileName,INIT_MODE_FROM_FILE);
}



/**
 * Initialize the module from a command.
 * @param[in] cmd command to be executed
 */
void mrf_initFromPipe (char *cmd) 
{
  mrf_doInit (cmd,INIT_MODE_FROM_PIPE);
}



/**
 * Add a new column type. 
 * @param[in] columnName Name of the new column
 */
void mrf_addNewColumnType (char* columnName)
{
  int i;

  i = 0;
  while (i < arrayMax (columnHeaders)) {
    if (strEqual (textItem (columnHeaders,i),columnName)) {
      break;
    } 
    i++;
  }
  if (i == arrayMax (columnHeaders)) {
    mrf_addColumnType (columnName);
  }
}



/**
 * Deinitialize the mrf module.
 */
void mrf_deInit (void) 
{
  ls_destroy (lsMrf);
  arrayDestroy (columnTypes);
  textDestroy (columnHeaders);
  bitFree (&presentColumnTypes);
  textDestroy (comments);
  hlr_free (headerLine);
}



static void mrf_freeReadAttributes (MrfRead *currRead)
{
  int i;
  MrfBlock *currBlock;
  
  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    hlr_free (currBlock->targetName);
  }
  arrayDestroy (currRead->blocks);
  if (bitReadOne (presentColumnTypes,MRF_COLUMN_TYPE_SEQUENCE)) {
    hlr_free (currRead->sequence);
  }
  if (bitReadOne (presentColumnTypes,MRF_COLUMN_TYPE_QUALITY_SCORES)) {
    hlr_free (currRead->qualityScores);
  }
  if (bitReadOne (presentColumnTypes,MRF_COLUMN_TYPE_QUERY_ID)) {
    hlr_free (currRead->queryId);
  }
}



static void mrf_freeEntry (MrfEntry* currEntry) 
{
  if (currEntry == NULL) {
    return;
  }
  mrf_freeReadAttributes (&currEntry->read1);
  if (currEntry->isPairedEnd == 1) {
    mrf_freeReadAttributes (&currEntry->read2);
  }
  freeMem (currEntry);
}



static void mrf_processBlocks (char *blockString, MrfRead *currRead)
{
  Texta blocks;
  Texta blockFields;
  int i;
  MrfBlock *currBlock;

  blocks = textFieldtok (blockString,",");
  for (i = 0; i < arrayMax (blocks); i++) {
    currBlock = arrayp (currRead->blocks,arrayMax (currRead->blocks),MrfBlock);
    blockFields = textFieldtok (textItem (blocks,i),":");
    currBlock->targetName = hlr_strdup (textItem (blockFields,0));
    currBlock->strand = textItem (blockFields,1)[0];
    currBlock->targetStart = atoi (textItem (blockFields,2));
    currBlock->targetEnd = atoi (textItem (blockFields,3));
    currBlock->queryStart = atoi (textItem (blockFields,4));
    currBlock->queryEnd = atoi (textItem (blockFields,5));
    textDestroy (blockFields);
  }
  textDestroy (blocks);
}



static MrfEntry* mrf_processNextEntry (int freeMemory) 
{
  static MrfEntry *currEntry = NULL;
  char *line,*token,*pos;
  WordIter w;
  int index,columnType;

  while (line = ls_nextLine (lsMrf)) {
    if (line[0] == '\0' || line[0] == '#' || strEqual (line,headerLine)) {
      continue;
    }
    if (freeMemory) {
      mrf_freeEntry (currEntry);
    }
    AllocVar (currEntry);
    if (strchr (line,'|')) {
      currEntry->isPairedEnd = 1;
    }
    else {
      currEntry->isPairedEnd = 0;
    }
    index = 0;
    w = wordIterCreate (line,"\t",0);
    while (token = wordNext (w)) {
      columnType = arru (columnTypes,index,int);
      if (columnType == MRF_COLUMN_TYPE_BLOCKS) {
        if (currEntry->isPairedEnd == 1) {
          pos = strchr (token,'|');
          *pos = '\0';
          currEntry->read1.blocks = arrayCreate (2,MrfBlock);
          currEntry->read2.blocks = arrayCreate (2,MrfBlock);
          mrf_processBlocks (token,&currEntry->read1);
          mrf_processBlocks (pos + 1,&currEntry->read2);
         
        }
        else {
          currEntry->read1.blocks = arrayCreate (2,MrfBlock);
          mrf_processBlocks (token,&currEntry->read1);
        }
      }
      else if (columnType == MRF_COLUMN_TYPE_SEQUENCE) {
        if (currEntry->isPairedEnd == 1) {
          pos = strchr (token,'|');
          *pos = '\0';
          currEntry->read1.sequence = hlr_strdup (token);
          currEntry->read2.sequence = hlr_strdup (pos + 1);
        }
        else {
          currEntry->read1.sequence = hlr_strdup (token);
        }
      }
      else if (columnType == MRF_COLUMN_TYPE_QUALITY_SCORES) {
        if (currEntry->isPairedEnd == 1) {
          pos = strchr (token,'|');
          *pos = '\0';
          currEntry->read1.qualityScores = hlr_strdup (token);
          currEntry->read2.qualityScores = hlr_strdup (pos + 1);
        }
        else {
          currEntry->read1.qualityScores = hlr_strdup (token);
        }
      }
      else if (columnType == MRF_COLUMN_TYPE_QUERY_ID) {
        if (currEntry->isPairedEnd == 1) {
          pos = strchr (token,'|');
          *pos = '\0';
          currEntry->read1.queryId = hlr_strdup (token);
          currEntry->read2.queryId = hlr_strdup (pos + 1);
        }
        else {
          currEntry->read1.queryId = hlr_strdup (token);
        }
      }
      else {
        die ("Unknown columnType: %d",columnType);
      }
      index++;
    }
    wordIterDestroy (w);
    return currEntry;
  }
  if (freeMemory) {
    mrf_freeEntry (currEntry);
  }
  currEntry = NULL;
  return currEntry;
}



/**
 * Returns a pointer to next MrfEntry. 
 * @pre The module has been initialized using mrf_init().
 */
MrfEntry* mrf_nextEntry (void) 
{
  return mrf_processNextEntry (1); 
}



/**
 * Returns an Array of MrfEntries.
 * @pre The module has been initialized using mrf_init().
 * @note The memory belongs to this routine.
 */
Array mrf_parse (void) 
{
  Array mrfEntries;
  MrfEntry *currEntry;

  mrfEntries = arrayCreate (100000,MrfEntry);
  while (currEntry = mrf_processNextEntry (0)) {
    array (mrfEntries,arrayMax (mrfEntries),MrfEntry) = *currEntry;
  }
  return mrfEntries;
}



static void mrf_addTab (Stringa buffer, int *first) 
{
  if (*first == 1) {
    *first = 0;
    return;
  }
  stringCatChar (buffer,'\t');
}

/**
 * Compute and return the length of the read.
 */
int getReadLength (MrfRead *currRead)
{
  MrfBlock *currBlock;
  int i;
  int sum;

  sum = 0;
  for (i = 0; i < arrayMax (currRead->blocks); i++) {
    currBlock = arrp (currRead->blocks,i,MrfBlock);
    sum += (currBlock->targetEnd - currBlock->targetStart + 1);
  }
  return sum;
}


/**
 * Write the mrf header preceeded by comments, if any. 
 * @pre The module has been initialized using mrf_init().
 */
char* mrf_writeHeader (void)
{
  static Stringa buffer = NULL;
  int i;

  stringCreateClear (buffer,100);
  for (i = 0; i < arrayMax (comments); i++) {
    stringAppendf (buffer,"#%s\n",textItem (comments,i));
  }
  for (i = 0; i < arrayMax (columnHeaders); i++) {
    stringAppendf (buffer,"%s%s",textItem (columnHeaders,i), 
		   i < arrayMax (columnHeaders) - 1 ? "\t" : "");
  }
  return string (buffer);
}



static void mrf_writeBlocks (Stringa buffer, Array blocks)
{
  MrfBlock *currBlock;
  int i;

  for (i = 0; i < arrayMax (blocks); i++) {
    currBlock = arrp (blocks,i,MrfBlock);
    stringAppendf (buffer,"%s:%c:%d:%d:%d:%d%s",currBlock->targetName,
                   currBlock->strand,currBlock->targetStart,currBlock->targetEnd,
                   currBlock->queryStart,currBlock->queryEnd,
                   i < arrayMax (blocks) - 1 ? "," : "");
  }
}



/**
 * Write an MrfEntry. 
 * @pre The module has been initialized using mrf_init().
 */
char* mrf_writeEntry (MrfEntry *currEntry)
{
  static Stringa buffer = NULL;
  int first;
  int i;
  int columnType;

  stringCreateClear (buffer,100);
  first = 1;
  for (i = 0; i < arrayMax (columnTypes); i++) {
    columnType = arru (columnTypes,i,int);
    if (bitReadOne (presentColumnTypes,MRF_COLUMN_TYPE_BLOCKS) && columnType == MRF_COLUMN_TYPE_BLOCKS) {
      mrf_addTab (buffer,&first);
      if (currEntry->isPairedEnd == 1) {
        mrf_writeBlocks (buffer,currEntry->read1.blocks);
        stringCatChar (buffer,'|');
        mrf_writeBlocks (buffer,currEntry->read2.blocks);
      }
      else {
        mrf_writeBlocks (buffer,currEntry->read1.blocks);
      }
    }
    if (bitReadOne (presentColumnTypes,MRF_COLUMN_TYPE_SEQUENCE) && columnType == MRF_COLUMN_TYPE_SEQUENCE) {
      mrf_addTab (buffer,&first);
      if (currEntry->isPairedEnd == 1) {
        stringAppendf (buffer,"%s|%s",currEntry->read1.sequence,currEntry->read2.sequence);
      }
      else {
        stringAppendf (buffer,"%s",currEntry->read1.sequence);
      }
    }
    if (bitReadOne (presentColumnTypes,MRF_COLUMN_TYPE_QUALITY_SCORES) && columnType == MRF_COLUMN_TYPE_QUALITY_SCORES) {
      mrf_addTab (buffer,&first);
      if (currEntry->isPairedEnd == 1) {
        stringAppendf (buffer,"%s|%s",currEntry->read1.qualityScores,currEntry->read2.qualityScores);
      }
      else {
        stringAppendf (buffer,"%s",currEntry->read1.qualityScores);
      }
    }
    if (bitReadOne (presentColumnTypes,MRF_COLUMN_TYPE_QUERY_ID) && columnType == MRF_COLUMN_TYPE_QUERY_ID) {
      mrf_addTab (buffer,&first);
      if (currEntry->isPairedEnd == 1) {
        stringAppendf (buffer,"%s|%s",currEntry->read1.queryId,currEntry->read2.queryId);
      }
      else {
        stringAppendf (buffer,"%s",currEntry->read1.queryId);
      }
    }
  }
  return string (buffer);
}

   
