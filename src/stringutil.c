#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "libtemplate/llist.h"
#include "libtemplate/xmalloc.h"
#include "libtemplate/debug.h"

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static int g_verbose_debug=FALSE;

void
safe_strncpy (char *dest, const char *src, int maxlen)
{
  strncpy (dest, src, maxlen);
  dest[maxlen] = 0;
}

/*
	* safe_str(min)cpy - take the min of maxlen1 and maxlen2 for the max len
	 */
void
safe_strmincpy (char *dest, const char *src, int maxlen1, int maxlen2)
{
  int maxlen = min (maxlen1, maxlen2);
  strncpy (dest, src, maxlen);
  dest[maxlen] = 0;
}
int strcount(const char* str, int c) 
{
		int count = 0;
		while(*str) {
						if (*str == c) {
										count++;
						}
						str++;
		}
		return count;
}
char **
strsplitc(const char *str, int delim, int max_items) 
{
				int i;
				size_t len;
				char** sa;
				char* next;
				int ntokens = strcount(str, delim);
				int elements = ntokens+1;
				debugf("split %s %c", str, delim);
				if ((max_items)&&(elements > max_items )) {
								elements= max_items;
				}

				sa = xmalloc(sizeof(char*)*(elements+1));
				for (i = 0; i < elements-1; i++) {
								next = strchr(str, delim);
								len = next-str;
								sa[i] = xmalloc(len+1);
								safe_strncpy(sa[i], str, len);
								str+= len+1;
				}
				len = strlen(str);
				sa[elements-1] = xmalloc(len+1);
				safe_strncpy(sa[elements-1], str, len);
			  sa[elements] = NULL;
				return sa;
}

void
strfreev (char **str_array)
{

  if (str_array) {
    int i;
    for (i = 0; str_array[i] != NULL; i++) {
      xfree (str_array[i]);
		}
    xfree (str_array);
  }
}
/*
	 * replace matchlen chars at start with changeto
	 * start with grow or shift by strlen(changeto) - deletelen
	 */
int
replace_insert (char *start, unsigned int deletelen, char *changeto, int
                maxgrow, char *tempbuf)
{
  unsigned int stringlen = strlen (start);
  unsigned int changelen = strlen (changeto);
  int diff = changelen - deletelen;
  int growth = 0;
  if (diff == 0) {
    strncpy (start, changeto, changelen);
    if (changelen > stringlen) {
      start[changelen] = 0;
    }
  } else {
    growth += diff;
    if (growth > maxgrow) {
      fprintf (stderr, "maxlen exceeded\n");
      return growth;
    } else {
      char *endptr = start + deletelen;
      strcpy (tempbuf, endptr);
      strncpy (start, changeto, changelen);
      strcpy (endptr + diff, tempbuf);
      if (changelen > stringlen) {

        start[changelen + strlen(tempbuf)] = 0;
      }
    }
  }
  return growth;

}

/*
	* tempbuf must hold maxlen chars (a little less actually)
*/
int
replace (char *orig, char *find, char *changeto, int maxlen, char *tempbuf)
{
  int count = 0;
  char *ptr = orig;
  int matchlen = strlen (find);
  int changelen = strlen (changeto);
  int textlen = strlen (orig);
  int maxgrow = maxlen - textlen;
  char *match = strstr (ptr, find);
  while (match) {
    ptr = match;
    maxgrow -= replace_insert (ptr, matchlen, changeto, maxgrow, tempbuf);
    ptr += changelen;
    match = strstr (ptr, find);
    count++;
  }
  return count;
}

void
stringutil_set_debug (BOOL on)
{
  g_verbose_debug = on;
}
