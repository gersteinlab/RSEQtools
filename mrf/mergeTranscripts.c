#include "log.h"
#include "format.h"
#include "linestream.h"
#include "intervalFind.h"
#include "bits.h"



/** 
 * \file mergeTranscripts.c Module to merge a set of transcripts from the same gene.
 * Usage: mergeTranscripts <knownIsoforms.txt> <file.annotation> <longestIsoform|compositeModel|intersection> \n
 * \n  
 * Obtain unique exons from various transcript isoforms based on \n
 *   1) longest isoform \n
 *   2) composite model (union of the exons from the different transcript isoforms) \n  
 *   3) intersection (intersection of the exons of the different transcript isoforms) \n  
 * \n  
 * The file knownIsoforms.txt should have two columns and no header: \n  
 * Column 1: id (int). Transcripts with the same id belong to the same gene. \n  
 * Column 2: name of the transcript. \n   
 * \n  
 * Example: \n  
 * 1       uc009vip.1 \n  
 * 1       uc001aaa.2 \n  
 * 2       uc009vis.1 \n  
 * 2       uc001aae.2 \n  
 * 2       uc009viu.1 \n  
 * 2       uc009vit.1 \n  
 * \n 
 * The transcripts are specified in file.annotation. \n
 */
 


typedef struct {
  int id;
  char* transcriptName;
} KnownIsoform;



static int sortIntervalsByName (Interval *a, Interval *b) 
{
  return strcmp (a->name,b->name);
}



static void generateOutput (char* transcriptName, char* chromosome, char strand,
                            Array exonStarts, Array exonEnds) 
{
  int i;

  if (arrayMax (exonStarts) != arrayMax (exonEnds)) {
    die ("exonStarts and exonEnds must have the same number of elements");
  }
  printf ("%s\t%s\t%c\t%d\t%d\t%d\t",transcriptName,chromosome,strand,
          arru (exonStarts,0,int),arru (exonEnds,arrayMax (exonEnds) - 1,int),arrayMax (exonStarts));
  for (i = 0; i < arrayMax (exonStarts); i++) {
    printf ("%d%s",arru (exonStarts,i,int),i < arrayMax (exonStarts) - 1 ? "," : "\t");
  }
  for (i = 0; i < arrayMax (exonEnds); i++) {
    printf ("%d%s",arru (exonEnds,i,int),i < arrayMax (exonEnds) - 1 ? "," : "\n");
  }
}
                            


static int sortKnownIsoformsById (KnownIsoform *a, KnownIsoform *b) 
{
  return a->id - b->id;
}



static Array readKnownIsoforms (char* fileName) 
{
  LineStream ls;
  char* line;
  char* pos;
  Array knownIsoforms;
  KnownIsoform *currKnownIsoform;

  knownIsoforms = arrayCreate (30000,KnownIsoform);
  ls = ls_createFromFile (fileName);
  while (line = ls_nextLine (ls)) {
    if (line[0] == '\0') {
      continue;
    }
    if (pos = strchr (line,'\t')) {
      *pos = '\0';
      currKnownIsoform = arrayp (knownIsoforms,arrayMax (knownIsoforms),KnownIsoform);
      currKnownIsoform->id = atoi (line);
      currKnownIsoform->transcriptName = hlr_strdup (pos + 1);
    }
  }
  ls_destroy (ls);
  return knownIsoforms;
}



int main (int argc, char *argv[]) 
{
  Stringa buffer;
  Array transcripts;
  Array knownIsoforms;
  Array isoTranscripts;
  Interval *currTranscript,testTranscript;
  SubInterval *currExon;
  KnownIsoform *currKnownIsoform,*nextKnownIsoform;
  int i,j,k,l,m;
  int index,exonBaseCount,maxExonBaseCount;
  int min,max,bitCount;
  int numberOfTranscripts;
  Array exonStarts,exonEnds;
  Texta isoNames;
  Bits *bits;
  int numOverlaps;
  int bitSet;

  if (argc != 4 ||
      !(strEqual (argv[3],"longestIsoform") || 
        strEqual (argv[3],"compositeModel") ||
        strEqual (argv[3],"intersection"))) {
    usage ("%s <knownIsoforms.txt> <file.annotation> <longestIsoform|compositeModel|intersection>",argv[0]);
  }
  buffer = stringCreate (100);
  isoNames = textCreate (100);
  exonStarts = arrayCreate (100,int);
  exonEnds = arrayCreate (100,int);
  isoTranscripts = arrayCreate (100,Interval*);
  knownIsoforms = readKnownIsoforms (argv[1]); 
  arraySort (knownIsoforms,(ARRAYORDERF)sortKnownIsoformsById);
  intervalFind_addIntervalsToSearchSpace (argv[2],0);
  transcripts = intervalFind_getAllIntervals ();
  arraySort (transcripts,(ARRAYORDERF)sortIntervalsByName);
  numberOfTranscripts = 0;
  i = 0; 
  while (i < arrayMax (knownIsoforms)) {
    currKnownIsoform = arrp (knownIsoforms,i,KnownIsoform);
    textClear (isoNames);
    textAdd (isoNames,currKnownIsoform->transcriptName);
    j = i + 1;
    while (j < arrayMax (knownIsoforms)) {
      nextKnownIsoform = arrp (knownIsoforms,j,KnownIsoform);
      if (currKnownIsoform->id == nextKnownIsoform->id) {
        textAdd (isoNames,nextKnownIsoform->transcriptName);
      }
      else {
        break;
      }
      j++;
    }
    i = j;
    arrayClear (isoTranscripts);
    for (k = 0; k < arrayMax (isoNames); k++) {
      testTranscript.name = hlr_strdup (textItem (isoNames,k));
      if (arrayFind (transcripts,&testTranscript,&index,(ARRAYORDERF)sortIntervalsByName)) {
        currTranscript = arrp (transcripts,index,Interval);
        array (isoTranscripts,arrayMax (isoTranscripts),Interval*) = currTranscript;
      }
      else {
         warn ("Unable to find transcript for isoform %s!",testTranscript.name);
      }
      hlr_free (testTranscript.name);
    }
    if (arrayMax (isoTranscripts) > 0) {
      numberOfTranscripts += arrayMax (isoTranscripts);
      stringClear (buffer);
      arraySort (isoNames,(ARRAYORDERF)arrayStrcmp);
      for (k = 0; k < arrayMax (isoNames); k++) {
        stringAppendf (buffer,"%s%s",textItem (isoNames,k),k < arrayMax (isoNames) - 1 ? "|" : "");
      }
      arrayClear (exonStarts);
      arrayClear (exonEnds);
      if (strEqual (argv[3],"longestIsoform")) {
        index = 0;
        maxExonBaseCount = 0;
        for (k = 0; k < arrayMax (isoTranscripts); k++) {
          currTranscript = arru (isoTranscripts,k,Interval*);
          exonBaseCount = 0;
          for (l = 0; l < arrayMax (currTranscript->subIntervals); l++) {
            currExon = arrp (currTranscript->subIntervals,l,SubInterval);
            exonBaseCount = exonBaseCount + currExon->end - currExon->start + 1;
          }
          if (exonBaseCount > maxExonBaseCount) {
            maxExonBaseCount = exonBaseCount;
            index = k;
          }
        }
        currTranscript = arru (isoTranscripts,index,Interval*);
        for (k = 0; k < arrayMax (currTranscript->subIntervals); k++) {
          currExon = arrp (currTranscript->subIntervals,k,SubInterval);
          array (exonStarts,arrayMax (exonStarts),int) = currExon->start;
          array (exonEnds,arrayMax (exonEnds),int) = currExon->end;
        }
      }
      else if (strEqual (argv[3],"compositeModel") || strEqual (argv[3],"intersection")) {
        currTranscript = arru (isoTranscripts,0,Interval*);
        min = currTranscript->start;
        max = currTranscript->end;
        for (k = 1; k < arrayMax (isoTranscripts); k++) {
          currTranscript = arru (isoTranscripts,k,Interval*);
          if (currTranscript->start < min) {
            min = currTranscript->start;
          }
          if (currTranscript->end > max) {
            max = currTranscript->end;
          }
        }
        bitCount = max - min + 1;
        bits = bitAlloc (bitCount);
        if (strEqual (argv[3],"compositeModel")) {
          for (k = 0; k < arrayMax (isoTranscripts); k++) {
            currTranscript = arru (isoTranscripts,k,Interval*);
            for (l = 0; l < arrayMax (currTranscript->subIntervals); l++) {
              currExon = arrp (currTranscript->subIntervals,l,SubInterval);
              for (m = currExon->start; m <= currExon->end; m++) {
                bitSetOne (bits,m - min);
              }
            }
          }
        }
        if (strEqual (argv[3],"intersection")) {
          bitSet = 0;
          for (k = min; k < max; k++) {
            numOverlaps = 0;
            for (l = 0; l < arrayMax (isoTranscripts); l++) {
              currTranscript = arru (isoTranscripts,l,Interval*);
              m = 0; 
              while (m < arrayMax (currTranscript->subIntervals)) {
                currExon = arrp (currTranscript->subIntervals,m,SubInterval);
                if (currExon->start <= k && k <= currExon->end) {
                  numOverlaps++;
                  break;
                }
                m++;
              }
            }
            if (numOverlaps == arrayMax (isoTranscripts)) {
              bitSetOne (bits,k - min);
              bitSet = 1;
            }
          }
          if (bitSet == 0) {
            continue;
          }
        }
        if (bitReadOne (bits,0) == 1) {
          array (exonStarts,arrayMax (exonStarts),int) = min;
        }
        for (k = min; k < max; k++) {
          if (bitReadOne (bits,k - min) == 1 &&
              bitReadOne (bits,k - min + 1) == 0) {
            array (exonEnds,arrayMax (exonEnds),int) = k;
          }
          if (bitReadOne (bits,k - min) == 0 &&
              bitReadOne (bits,k - min + 1) == 1) {
            array (exonStarts,arrayMax (exonStarts),int) = k + 1;
          }
        }
        if (bitReadOne (bits,bitCount - 1) == 1) {
          array (exonEnds,arrayMax (exonEnds),int) = max;
        }
        bitFree (&bits);
      }
      else {
        die ("Unknown mode");
      }
      generateOutput (string (buffer),currTranscript->chromosome,currTranscript->strand,exonStarts,exonEnds);
    }
  }
  if (numberOfTranscripts != arrayMax (transcripts)) {
    warn ("Number of transcripts in annotation file: %d",arrayMax (transcripts));
    warn ("Number of transcripts in output file: %d",numberOfTranscripts);
  }
  return 0;
}
