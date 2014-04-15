#ifndef DEF_STRINGUTIL_H
#define DEF_STRINGUTIL_H

/** 
 *   \file stringUtil.h
 *   \author Lukas Habegger (lukas.habegger@yale.edu)
 */


extern char* subString (char *str, int start, int end);
/**
 * Returns position of needle in haystack or NULL if it's not there. 
 */
#define stringIn(needle, haystack) strstr(haystack, needle)
extern char *rStringIn(char *needle, char *haystack);
extern char *stringBetween(char *start, char *end, char *haystack);
extern void toggleCase(char *s, int size);
extern void stripChar(char *s, char c);
extern int countChars(char *s, char c);
extern int countSame(char *a, char *b);
extern char *skipLeadingSpaces(char *s);
extern char *skipToSpaces(char *s);
extern void eraseTrailingSpaces(char *s);
extern void eraseWhiteSpace(char *s);
extern char *trimSpaces(char *s);
extern int hasWhiteSpace(char *s);
extern char *firstWordInLine(char *line);
extern char *lastWordInLine(char *line);
extern char *addSuffix(char *head, char *suffix);
extern void chopSuffix(char *s);
extern void chopSuffixAt(char *s, char c);
extern char *chopPrefix(char *s);
extern char *chopPrefixAt(char *s, char c);
extern char *naForNull(char *s);
extern char *naForEmpty(char *s);
extern char *emptyForNull(char *s);
extern char *nullIfAllSpace(char *s);
extern char *trueFalseString(int b);
extern char *skipNumeric(char *s);
extern char *skipToNumeric(char *s);
extern char *insertWordEveryNthPosition (char *string, char *word, int n);

#endif
