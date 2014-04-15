#ifndef DEF_BLAT_PARSER_H
#define DEF_BLAT_PARSER_H



/**
 *   \file blatParser.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



/**
 * BlatQuery.
 */
typedef struct {
  char* qName;
  Array entries;     // of type PslEntry
} BlatQuery;



/**
 * PslEntry.
 */
typedef struct {
  int matches;       // Number of bases that match that aren't repeats
  int misMatches;    // Number of bases that don't match
  int repMatches;    // Number of bases that match but are part of repeats
  int nCount;        // Number of 'N' bases
  int qNumInsert;    // Number of inserts in query
  int qBaseInsert;   // Number of bases inserted in query
  int tNumInsert;    // Number of inserts in target
  int tBaseInsert;   // Number of bases inserted in target
  char strand;       // + or - for query strand, optionally followed by + or  for target strand
  int qSize;         // Query sequence size
  int qStart;        // Alignment start position in query
  int qEnd;          // Alignment end position in query
  char* tName;       // Target sequence name
  int tSize;         // Target sequence size
  int tStart;        // Alignment start position in target
  int tEnd;          // Alignment end position in target
  int blockCount;    // Number of blocks in alignment. A block contains no gaps.
  Array blockSizes;  // Size of each block in a comma separated list
  Array qStarts;     // Start of each block in query in a comma separated list
  Array tStarts;     // Start of each block in target in a comma separated list
} PslEntry;



extern void blatParser_initFromFile (char* fileName);
extern void blatParser_initFromPipe (char* command);
extern void blatParser_deInit (void);
extern BlatQuery* blatParser_nextQuery (void);



#endif
