#ifndef DEF_BLAST_PARSER_H
#define DEF_BLAST_PARSER_H



/**
 *   \file blastParser.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



/**
 * BlastQuery.
 */
typedef struct {
  char* qName;
  Array entries;  // of type BlastEntry     
} BlastQuery;



/**
 * BlastEntry.
 */
typedef struct {
  char* tName;
  double percentIdentity;
  int alignmentLength;
  int misMatches;
  int gapOpenings;
  int qStart;
  int qEnd;
  int tStart;
  int tEnd;
  double evalue;
  double bitScore;
} BlastEntry;



extern void blastParser_initFromFile (char* fileName);
extern void blastParser_initFromPipe (char* command);
extern void blastParser_deInit (void);
extern BlastQuery* blastParser_nextQuery (void);



#endif
