#ifndef DEF_INTERVAL_FIND_H
#define DEF_INTERVAL_FIND_H



/**
 *   \file intervalFind.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



/**
 * Interval.
 */
typedef struct {
  int source;
  char *name;
  char *chromosome;
  char strand;
  int start;
  int end;
  int subIntervalCount;
  Array subIntervals; 
} Interval;



/**
 * SubInterval.
 */
typedef struct {
  int start;
  int end;
} SubInterval;



extern void intervalFind_addIntervalsToSearchSpace (char* fileName, int source);
extern Array intervalFind_getOverlappingIntervals (char* chromosome, int start, int end);
extern int intervalFind_getNumberOfIntervals (void);
extern Array intervalFind_getAllIntervals (void);
extern Array intervalFind_getIntervalPointers (void);
extern Array intervalFind_parseFile (char* fileName, int source);
extern void intervalFind_parseLine (Interval *thisInterval, char* line, int source);
extern char* intervalFind_writeInterval (Interval *currInterval);



#endif
