#ifndef DEF_BGR_PARSER_H
#define DEF_BGR_PARSER_H

/**
 *    \file bgrParser.h
 *    \author Andrea Sboner (andrea.sboner@yale.edu)
 */


/**
 * BedGraph.
 */
typedef struct {
  char*  chromosome;
  int    start;
  int    end;
  double value;
} BedGraph;
  


extern void bgrParser_initFromFile (char *fileName);
extern void bgrParser_initFromPipe (char *command);
extern void bgrParser_deInit (void);

extern BedGraph* bgrParser_nextEntry (void);
extern Array bgrParser_getAllEntries ( void ); 

extern int bgrParser_sort (BedGraph *a, BedGraph *b);
extern Array bgrParser_getValuesForRegion (Array bedGraphs, char *chromosome, int start, int end);
extern void bgrParser_freeBedGraphs (Array bedGraphs);



#endif // DEF_BGR_PARSER_H
