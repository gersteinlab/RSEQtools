#ifndef DEF_MRF_UTIL_H
#define DEF_MRF_UTIL_H



/**
 *   \file mrfUtil.h
 */



/**
 * Tar.
 */
typedef struct {
  char* targetName;
  int start;
  int end;
} Tar;



extern Array readTarsFromBedFile (char *fileName);



#endif
