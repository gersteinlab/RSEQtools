#ifndef DEF_EXPORTPE_PARSER_H
#define DEF_EXPORTPE_PARSER_H



/**
 *   \file exportPEParser.h
 *   \author Andrea Sboner (andrea.sboner@yale.edu)
 */


/**
 * ExportPEsingle
 */

typedef struct {
  char* machine;           // 1 machine
  int run_number;          // 2 run number
  int lane;                // 3 lane
  int tile;                // 4 tile 
  int xCoor;               // 5 x coordinate of cluster
  int yCoor;               // 6 y coordinate of cluster
  char* index;             // 7 index string
  int read_number;         // 8 read number
  char* sequence;          // 9 read
  char* quality;           // 10 quality
  char* chromosome;        // 11 match chromosome
  char* contig;            // 12 match contig
  int position;            // 13 match position
  char strand;             // 14 match strand
  char* match_descriptor;  // 15 match descriptor
  int singleScore;         // 16 single read alignment score
  int pairedScore;         // 17 paired end alignment score
  char* partnerChromosome; // 18 partner chromosome
  char* partnerContig;     // 19 partner contig
  int partnerOffset;       // 20 partner offset
  char partnerStrand;      // 21 partner strand
  char filter;             // 22 filter
} singleEnd;

typedef struct {
  singleEnd* end1;
  singleEnd* end2;
} ExportPE;

extern void exportPEParser_initFromFile (char* fileName1, char* fileName2 );
extern void exportPEParser_initFromPipe (char* cmd1, char* cmd2);
extern void exportPEParser_deInit ( void );
extern ExportPE* exportPEParser_nextEntry( void);
extern char* exportPEParser_writeEntry ( singleEnd* currEntry );
#endif
