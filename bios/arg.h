#ifndef DEF_ARG_H
#define DEF_ARG_H



/**
 *   \file arg.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */




/**
 * Arg.
 */
typedef struct {
  char *name;
  char *value;
  Texta allowedValues;
  int isRequired;
} Arg;



extern void arg_init (int argc, char *argv[], char *options);



#endif
