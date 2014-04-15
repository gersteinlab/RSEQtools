#ifndef DEF_ELAND_PARSER_H
#define DEF_ELAND_PARSER_H



/**
 *   \file elandParser.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



/**
 * ElandQuery.
 */
typedef struct {
  char* sequenceName;
  char* sequence;
  char* matchCode;
  int exactMatches;
  int oneErrorMatches;
  int twoErrorMatches;
  char* chromosome;
  int position;
  char strand;
} ElandQuery;



extern void elandParser_init (char* fileName);
extern void elandParser_deInit (void);
extern ElandQuery* elandParser_nextQuery (void);



#endif
