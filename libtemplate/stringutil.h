#ifndef __STRINGUTIL_H
#define __STRINGUTIL_H
#include "mybool.h"

/*
	 * replace matchlen chars at start with changeto
	 * start with grow or shift by strlen(changeto) - deletelen
	 */
int
  replace_insert (char *start, unsigned int deletelen, char *changeto, int
                  maxgrow, char *tempbuf);
/*
	* tempbuf must hold maxlen chars (a little less actually)
*/
int
  replace (char *orig, char *find, char *changeto, int maxlen, char *tempbuf);

void safe_strncpy (char *dest, const char *src, int maxlen);
void safe_strmincpy (char *dest, const char *src, int maxlen, int maxlen2);
void strfreev (char **str_array);
char **strsplit (const char *string, const char *delimiter, int max_items);
char ** strsplitc(const char *str, int delim, int max_items);
void stringutil_set_debug (BOOL on);

#endif
