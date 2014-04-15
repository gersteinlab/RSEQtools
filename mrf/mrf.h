#ifndef DEF_MRF_H
#define DEF_MRF_H



/**
 *   \file mrf.h
 */



// required
#define MRF_COLUMN_TYPE_BLOCKS 1
// optional
#define MRF_COLUMN_TYPE_SEQUENCE 2
#define MRF_COLUMN_TYPE_QUALITY_SCORES 3
#define MRF_COLUMN_TYPE_QUERY_ID 4


// required
#define MRF_COLUMN_NAME_BLOCKS "AlignmentBlocks"
// optional
#define MRF_COLUMN_NAME_SEQUENCE "Sequence"
#define MRF_COLUMN_NAME_QUALITY_SCORES "QualityScores"
#define MRF_COLUMN_NAME_QUERY_ID "QueryId"



/**
 * MrfBlock.
 */
typedef struct {
  char *targetName;
  char strand;
  int targetStart;
  int targetEnd;
  int queryStart;
  int queryEnd;
} MrfBlock;



/**
 * MrfRead.
 */
typedef struct {
  Array blocks;  // of type MrfBlock
  char *sequence;
  char *qualityScores;
  char *queryId;
} MrfRead;



/**
 * MrfEntry.
 */
typedef struct {
  int isPairedEnd;
  MrfRead read1;
  MrfRead read2;
} MrfEntry;



extern void mrf_init (char* fileName);
extern void mrf_initFromPipe (char* cmd);
extern void mrf_addNewColumnType (char* columnName);
extern void mrf_deInit (void);
extern MrfEntry* mrf_nextEntry (void);
extern Array mrf_parse (void);
extern char* mrf_writeHeader (void);
extern char* mrf_writeEntry (MrfEntry *currEntry);
extern int getReadLength (MrfRead *currRead);


#endif
