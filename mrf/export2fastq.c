#include "log.h"
#include "format.h"
#include "linestream.h"



/** 
 *   \file export2fastq.c Module to convert ELAND export file format into FASTQ.
 *         Reads a file in ELAND export format from STDIN. \n
 */



int main (int argc, char *argv[])
{
  LineStream ls;
  char *line;
  Stringa id;
  WordIter w;
  char *read = NULL;
  char *quality = NULL;

  id = stringCreate(50);
  ls = ls_createFromFile ( "-" ); 
  while ((line = ls_nextLine (ls)) ) {
    w = wordIterCreate (line,"\t",0);
    stringAppendf (id,"%s:",wordNext (w)); // 1 machine
    wordNext (w); // 2 run number
    stringAppendf (id,"%s:",wordNext (w)); // 3 lane
    stringAppendf (id,"%s:",wordNext (w)); // 4 tile
    stringAppendf (id,"%s:",wordNext (w)); // 5 x coordinate of cluster
    stringAppendf (id,"%s#",wordNext (w)); // 6 y coordinate of cluster
    stringAppendf (id,"%s", wordNext (w)); // 7 index string
    stringAppendf (id,"/%s",wordNext (w)); // 8 read number
    strReplace (&read,wordNext (w)); // 9 read
    strReplace (&quality,wordNext (w)); // 10 quality   
    printf("@%s\n%s\n+\n%s\n", string(id), read, quality);
    wordIterDestroy (w);  
    stringClear(id);
  }
  ls_destroy (ls);
  stringDestroy(id);
  return 0;
}
