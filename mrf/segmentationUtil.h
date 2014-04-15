#ifndef DEF_SEGMENTATION_UTIL_H
#define DEF_SEGMENTATION_UTIL_H



/**
 *   \file segmentationUtil.h
 */



typedef struct {
  char* targetName;
  int start;
  int end;
} Tar;



typedef struct {
  int position;
  float value;
} Wig;



extern void performSegmentation (Array tars, Array wigs, char* targetName, 
                                 double threshold, int maxGap, int minRun);



#endif
