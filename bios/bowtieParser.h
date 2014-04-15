#ifndef DEF_BOWTIE_PARSER_H
#define DEF_BOWTIE_PARSER_H



/**
 *   \file bowtieParser.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */




/**
 * BowtieQuery.
 */
typedef struct {
  char* sequenceName;
  Array entries;     // of type BowtieEntry
} BowtieQuery;



/**
 * BowtieEntry.
 */
typedef struct {
  char* chromosome;
  char* sequence;
  char* quality;
  int position;
  char strand;
  Array mismatches; // of type BowtieMismatch
} BowtieEntry;



/**
 * BowtieMismatch.
 */
typedef struct {
  int offset;
  char referenceBase;
  char readBase;
} BowtieMismatch;



extern void bowtieParser_initFromFile (char* fileName);
extern void bowtieParser_initFromPipe (char* command);
extern void bowtieParser_deInit (void);
extern void bowtieParser_copyQuery (BowtieQuery **dest, BowtieQuery *orig);
extern void bowtieParser_freeQuery (BowtieQuery *currBowtieQuery);
extern BowtieQuery* bowtieParser_nextQuery (void);
extern Array  bowtieParser_getAllQueries ();



#endif
