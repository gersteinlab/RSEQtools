#include "format.h"
#include "log.h"
#include "arg.h"



/** 
 *   \file arg.c Module to parse command line arguments
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */



static Array args = NULL;



/**
 * Initialize the command line parser.
 * @param[in] argc Number of command line elements (from main)
 * @param[in] argv Command line elements (from main)
 * @param[in] options Specifies the command line elements 


type:0
type:1:
type:1:a|b|c
type:0:a|b|c





 */
void arg_init (int argc, char *argv[], char *options)
{
  int numRequiredOptions;
  Texta argElements,allowedValues;
  char *pos1,*pos2;
  int i;

  args = arrayCreate (100,Arg);
  argElements = textStrtok (options,",");
  for (i = 0; i < arrayMax (argElements); i++) {
    pos1 = strchr (textItem (argElements,i),':');
    pos2 = strrchr (textItem (argElements,i),':');
    
  }
  textDestroy (argElements);
}


arg_init (argc,argv,"type:1:all|subset,min:0:MULTI,version:0:NONE")
