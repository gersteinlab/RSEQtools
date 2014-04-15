#ifndef DEF_ELAND_MULTI_PARSER_H
#define DEF_ELAND_MULTI_PARSER_H



/**
 *   \file elandMultiParser.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



/**
 * ElandMultiQuery.
 */
typedef struct {
  char* sequenceName;
  char* sequence;
  int exactMatches;
  int oneErrorMatches;
  int twoErrorMatches;
  Array entries;     // of type ElandMultiEntry
} ElandMultiQuery;



/**
 * ElandMultiEntry.
 */
typedef struct {
  char* chromosome;
  int position;
  char strand;
  int numErrors;
} ElandMultiEntry;



extern void elandMultiParser_init (char* fileName);
extern void elandMultiParser_deInit (void);
extern ElandMultiQuery* elandMultiParser_nextQuery (void);



#endif
