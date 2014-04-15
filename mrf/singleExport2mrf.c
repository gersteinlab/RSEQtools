#include "log.h"
#include "format.h"
#include "linestream.h"
#include "mrf.h"
#include <stdio.h>



/** 
 *   \file singleExport2mrf.c Module to convert single-end reads from ELAND export format to MRF.
 */



static void freeMemory (char *read, char *quality, char *chromosome, char *partnerChromosome) 
{
  hlr_free (read);
  hlr_free (quality);
  hlr_free (chromosome);
  hlr_free (partnerChromosome);
}



static char convertStrand (char strand) 
{
  if (strand == 'F') {
    return '+';
  }
  if (strand == 'R') {
    return '-';
  }
  die ("Unexpected strand: %c",strand);
  return strand;
}



int main (int argc, char *argv[])
{
  LineStream ls;
  char *line;
  WordIter w;
  char *read;
  char *quality;
  int readLength;
  int position;
  int offset;
  char *chromosome = NULL;
  char *partnerChromosome = NULL;
  char strand;
  char *contig;
  char *partnerContig;
  char partnerStrand;
  char filter;
  char *pos;
  Stringa id;
  int startFirstExon;
  int startSecondExon;
  int sizeExonOverlap;
  int numNucleotidesFirstExon;
  int numNucleotidesSecondExon;
 
  id = stringCreate (100);
  printf ("%s\n",MRF_COLUMN_NAME_BLOCKS);
  ls = ls_createFromFile ("-");
  while (line = ls_nextLine (ls)) {
    stringClear (id);
    w = wordIterCreate (line,"\t",0);
    stringAppendf (id,"%s_",wordNext (w)); // 1 machine
    wordNext (w); // 2 run number
    stringAppendf (id,"%s_",wordNext (w)); // 3 lane
    stringAppendf (id,"%s_",wordNext (w)); // 4 tile
    stringAppendf (id,"%s_",wordNext (w)); // 5 x coordinate of cluster
    stringAppendf (id,"%s_",wordNext (w)); // 6 y coordinate of cluster
    stringAppendf (id,"%s",wordNext (w)); // 7 index string
    wordNext (w); // 8 read number
    read = hlr_strdup (wordNext (w)); // 9 read
    readLength = strlen (read); 
    quality = hlr_strdup (wordNext (w)); // 10 quality
    chromosome = hlr_strdup (wordNext (w)); // 11 match chromosome
    contig = wordNext (w); // 12 match contig, true if splice junction
    position = atoi (wordNext (w)); // 13 match position
    strand = wordNext (w)[0]; // 14 match strand
    wordNext (w); // 15 match descriptor
    wordNext (w); // 16 single read alignment score
    wordNext (w); // 17 paired end alignment score
    partnerChromosome = hlr_strdup (wordNext (w)); // 18 partner chromosome
    partnerContig = wordNext (w); // 19 partner contig
    offset = atoi (wordNext (w)); // 20 partner offset
    partnerStrand = wordNext (w)[0]; // 21 partner strand
    filter = wordNext (w)[0]; // 22 filter
    wordIterDestroy (w);
    // only accept reads that pass quality filtering
    if(filter != 'Y') {
      freeMemory (read,quality,chromosome,partnerChromosome); 
      continue;
    }
    // skip reads with NM, QC, RM, multi-hits 
    if (strEqual (chromosome,"NM") ||
        strEqual (chromosome,"QC") || 
        strEqual (chromosome,"RM") || 
        strchr (chromosome,':')) {
      freeMemory (read,quality,chromosome,partnerChromosome); 
      continue;
    }    
    if (position < 0) {
      freeMemory (read,quality,chromosome,partnerChromosome); 
      continue;
    }
    if (strStartsWith (chromosome,"chr")) {
      pos = strchr (chromosome,'.');	  
      *pos = '\0';
      printf ("%s:%c:%d:%d:1:%d\n",chromosome,convertStrand (strand),position,position + readLength - 1,readLength);
    } 
    else if (strstr (chromosome,"splice") != NULL) {
      w = wordIterCreate (contig,"|",0);
      strReplace (&chromosome,wordNext (w));
      startFirstExon = atoi (wordNext (w));
      startSecondExon = atoi (wordNext (w));
      sizeExonOverlap = atoi (wordNext (w));
      wordIterDestroy (w);
      numNucleotidesFirstExon = sizeExonOverlap - position + 1;
      numNucleotidesSecondExon = readLength - numNucleotidesFirstExon;
      printf ("%s:%c:%d:%d:%d:%d",
              chromosome,
              convertStrand (strand),
              startFirstExon + 1 + position - 1,
              startFirstExon + 1 + sizeExonOverlap - 1,
              1,
              numNucleotidesFirstExon);
      printf (",");
      printf ("%s:%c:%d:%d:%d:%d\n",
              chromosome,
              convertStrand (strand),
              startSecondExon + 1,
              startSecondExon + 1 + numNucleotidesSecondExon - 1,
              numNucleotidesFirstExon + 1,
              readLength);
    }
    else if (strstr (chromosome,"spike") != NULL) {
      printf ("%s:%c:%d:%d:1:%d\n",contig,convertStrand (strand),position,position + readLength - 1,readLength);
    }
    else {
      die ("Unexpected case: %s, %s",chromosome,contig);
    }
    freeMemory (read,quality,chromosome,partnerChromosome); 
  }
  ls_destroy (ls);
  stringDestroy (id);
  return 0;
}



