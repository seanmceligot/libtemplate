//  libtemplate - a c template engine
//  Copyright (C) 2002 Sean Patrick McEligot
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include "libtemplate/libtemplate.h"
#include "libtemplate/stringutil.h"
#include "libtemplate/debug.h"
#include "libtemplate/mybool.h"
#include "libtemplate/llist.h"
#include "libtemplate/xmalloc.h"

static int g_verbose_debug = FALSE;

/*
 * FindReplace is used for regex s/r
 */
typedef struct _FindReplace FindReplace;
struct _FindReplace {
  pcre *find;
  char *replace;
};
typedef enum
  { NONE = 0, KEY, START_INDEX, END_INDEX, INSERT, REGEX } MatchType;

#define MAX_KEY_SIZE 255

typedef struct _ParseData ParseData;
struct _ParseData {
  LList *queued_lines;
  StrHash *current_list_hash;
  char *lineptr;
  MatchType match_type;
  char match[MAX_KEY_SIZE];
  char submatch[MAX_KEY_SIZE];
  FindReplace *findreplace;
  char *inbuf;
  size_t inbuf_line_len;
  size_t inbuf_sz;
  FILE *in;
  FILE *out;
};

static pcre *template_key_re;
static pcre *template_start_index_re;
static pcre *template_end_index_re;
static pcre *template_insert_re;

// this is the $ in regex substitutions like /in(sub)in/out$1out/
char template_regsub_prefix = '$';

// key's to be substituted look match this pattern
char *template_key_pattern = "\\${([a-zA-Z_\\.]+)}";
// lists start with this pattern
static char *template_start_pattern = "\\${([a-zA-Z_]+):}";
static char *template_end_pattern = "\\${:([a-zA-Z_]+)}";
static char *template_insert_pattern = "\\${i:([^}]*)}";

// private methods
static pcre *template_compile (const char *pattern);
void output (ParseData * pd, char *lineptr);
void outputn (ParseData * pd, char *lineptr, size_t len);
void template_examine_line (Template * tp, ParseData * pd);
static void free_find_replace (void *data, void *user_data);

int
template_init ()
{
  template_key_re = template_compile (template_key_pattern);
  template_start_index_re = template_compile (template_start_pattern);
  template_end_index_re = template_compile (template_end_pattern);
  template_insert_re = template_compile (template_insert_pattern);
  return (template_key_re != NULL) && (template_start_index_re != NULL)
    && (template_end_index_re != NULL) && (template_insert_re != NULL);
}

void
template_shutdown ()
{
  debug ("template_shutdown");
}

Template *
template_new ()
{
  Template *tp = xmalloc (sizeof (Template));
  tp->pairs = strhash_new (1, 2);
  tp->lists = strhash_new (1, 2);
  tp->regexes = NULL;
  return tp;
}
void free_pairs(void* key, void* data, void* user_data) {
				xfree(key);
				xfree(data);
}
void free_listhash(void *vlisthashp, void* user_data) {
				TemplateListHashPtr listhashp = (TemplateListHashPtr)vlisthashp;
				strhash_free_each(listhashp, free_pairs, NULL);
}
void free_list(void* key, void* data, void* user_data) {
				char* listname = (char*)key;
				TemplateListPtr list = (TemplateListPtr) data;
				debugf("freeing listname %s", listname);
				xfree(listname);
				llist_free_each(list, free_listhash, NULL);
}
void
template_destroy (Template * tp)
{
  debug ("template_destroy");
  strhash_free_each (tp->pairs, free_pairs, NULL);
  tp->pairs = NULL;
  strhash_free_each (tp->lists, free_list, NULL);
  tp->lists = NULL;
  if (tp->regexes) {
    llist_free_each (tp->regexes, free_find_replace, NULL);
  }
  tp->regexes = NULL;
  xfree (tp);
}

static void
getresult (int index, int *result, int *out_offset, int *out_len)
{
  int end;
  index *= 2;
  *out_offset = result[index + 0];
  end = result[index + 1];
  *out_len = end - *out_offset;

}

void
handle_regex (Template * tp, ParseData * pd, FindReplace * pair)
{
  int linelen = strlen (pd->match);
  int resultcount;
  const int RCOUNT = 99;
  int result[RCOUNT];
  static char buf[1024];
  char *line = pd->match;

  resultcount =
    pcre_exec (pair->find, NULL, line, linelen, 0, 0, result, RCOUNT);
  if (resultcount == 1) {
    int start, len;
    getresult (0, result, &start, &len);
    //debugf ("result[%d] = %d", 0, result[0]);
    //debugf ("result[%d] = %d", 1, result[1]);
    //pcre_get_substring_list (lineptr, result, resultcount, matches);
    //debugf ("find_replace (%s) result = %s", lineptr, matches[0][0]);
    debugf ("start %d len %d", start, len);
    output (pd, pair->replace);
  } else if (resultcount > 1) {
    int i;
    int start, len;
    int mstart, mlen;
    char insert[1024];
    char tag[3];
    tag[0] = template_regsub_prefix;
    tag[2] = 0;
    strcpy (insert, pair->replace);
    debugf ("insert: %s", insert);
    getresult (0, result, &start, &len);
    for (i = 1; i < resultcount; i++) {
      tag[1] = '0' + i;
      getresult (i, result, &mstart, &mlen);
      {
        char sub[mlen];
        safe_strncpy (sub, line + mstart, mlen);
        debugf ("sub: %s", sub);
        replace (insert, tag, sub, 1024, buf);
        debugf ("insert now: %s", insert);
      }
    }
    output (pd, insert);
  } else                        //(resultcount < 0)
  {
    if (resultcount != PCRE_ERROR_NOMATCH) {
      debugf ("Matching error %d (%s)", resultcount, line);
    }
  }
}
static void
free_find_replace (void *data, void *user_data)
{
  FindReplace *pair = (FindReplace *) data;
  if (pair != NULL) {
    xfree (pair->replace);
    xfree (pair);
  }
}

void
template_addkeyvalue (Template * tp, const char *key, const char *value)
{
  debugf ("%s=%s", key, value);
  strhash_put (tp->pairs, (void *) key, (void *) value);
}

TemplateListPtr
template_list_new ()
{
  LList *list = llist_new ();
  return list;
}

TemplateListHashPtr
template_listhash_new ()
{
  StrHash *hash = strhash_new (10, 10);
  return hash;
}

void
template_listhashput (TemplateListHashPtr hash, const char *key,
                      const char *value)
{
  debugf ("%s = %s", key, value);
  strhash_put (hash, (void *) key, (void *) value);
}

TemplateListPtr
template_listadd (TemplateListPtr list, TemplateListHashPtr hash)
{
  return llist_append (list, hash);
}

void
template_addlist (Template * tp, const char *listname, TemplateListPtr list)
{
  debugf ("adding list %s", listname);
  strhash_put (tp->lists, (void *) listname, (void *) list);
}

// str is like:
// listname:row1_key1=val1,row1_key2=val2|row2_key1=val1,row2_key2=val2
int
template_addstrlist (Template * tp, const char *str)
{
  char *listname;
  char *strwholelist;
  char **v_commalist;
  int listno = 0;
  TemplateListPtr tplist;
  // variables:name=Foo,type=int|name=bar,type,long
  char **v_namelist = strsplitc (str, ':', 2);
  if (!v_namelist[1]) {
    strfreev (v_namelist);
    return FALSE;
  }
  listname = xstrdup (v_namelist[0]);
  strwholelist = v_namelist[1];
  v_commalist = strsplitc (strwholelist, '|', 0);
  tplist = template_list_new ();
  while (v_commalist[listno] != NULL) {
    int pairno = 0;
    char **v_pairs = strsplitc (v_commalist[listno], ',', 2);
    TemplateListHashPtr tplistpairs = template_listhash_new ();
    while (v_pairs[pairno] != NULL) {
      char **keyvalue = strsplitc (v_pairs[pairno], '=', 0);
      char *key = keyvalue[0];
      char *value = keyvalue[1];
      char *keyname = xmalloc (strlen (listname) + strlen (key) + 2);
      strcpy (keyname, listname);
      strcat (keyname, ".");
      strcat (keyname, key);
      template_listhashput (tplistpairs, keyname, xstrdup (value));
      pairno++;
      strfreev (keyvalue);
      //xfree (keyname);
    }
    strfreev (v_pairs);
    tplist = template_listadd (tplist, tplistpairs);
    listno++;
  }
  template_addlist (tp, listname, tplist);
  strfreev (v_namelist);
  strfreev (v_commalist);
  return TRUE;
}

static pcre *
template_compile (const char *pattern)
{
  const char *error;
  int erroffset;
  pcre *re;
  debugf ("compile:%s", pattern);
  re = pcre_compile (pattern, 0, &error, &erroffset, NULL);
  if (!re) {
    debugf ("PCRE compilation failed at offset %d: %s", erroffset, error);
    return NULL;
  }
  return re;
}

MatchType
template_getmatch (Template * tp, const char *lineptr, int *out_result,
                   FindReplace ** findreplace)
{
  const int RCOUNT = 6;
  int result[RCOUNT];

  int linelen = (int) strlen (lineptr);
  int resultcount;
  LList *rlist;
  int start = -1;
  MatchType match_type = NONE;
  resultcount =
    pcre_exec (template_start_index_re, NULL, lineptr, linelen, 0, 0, result,
               RCOUNT);
  if (resultcount > 0) {
    start = result[0];
    match_type = START_INDEX;
    memcpy (out_result, result, sizeof (result));
  }
  resultcount =
    pcre_exec (template_end_index_re, NULL, lineptr, linelen, 0, 0, result,
               RCOUNT);
  if (resultcount > 0) {
    if ((start < 0) || (result[0] < start)) {
      start = result[0];
      match_type = END_INDEX;
      memcpy (out_result, result, sizeof (result));
    }
  }
  resultcount =
    pcre_exec (template_key_re, NULL, lineptr, linelen, 0, 0, result, RCOUNT);
  if (resultcount > 0) {
    if ((start < 0) || (result[0] < start)) {
      start = result[0];
      match_type = KEY;
      memcpy (out_result, result, sizeof (result));
    }
  }
  resultcount =
    pcre_exec (template_insert_re, NULL, lineptr, linelen, 0, 0, result,
               RCOUNT);
  if (resultcount > 0) {
    if ((start < 0) || (result[0] < start)) {
      start = result[0];
      match_type = INSERT;
      memcpy (out_result, result, sizeof (result));
    }
  }
  rlist = tp->regexes;

  while (rlist) {
    FindReplace *pair = (FindReplace *) llist_data (rlist);
    resultcount =
      pcre_exec (pair->find, NULL, lineptr, linelen, 0, 0, result, RCOUNT);
    if (resultcount > 0) {
      if ((start < 0) || (result[0] < start)) {
        start = result[0];
        match_type = REGEX;
        *findreplace = pair;
        memcpy (out_result, result, sizeof (result));
      }
    }
    rlist = llist_next (rlist);
  }
  return match_type;
}

int
template_addregex (Template * tp, const char *re_find, const char *replace)
{

  pcre *fre = template_compile (re_find);
  FindReplace *fr;
  if (!fre) {
    return FALSE;
  }
  fr = (FindReplace *) xmalloc (sizeof (FindReplace));
  fr->find = fre;
  fr->replace = strdup (replace);
  if (!tp->regexes) {
    tp->regexes = llist_new ();
  }
  tp->regexes = llist_append (tp->regexes, fr);
  return TRUE;
}

void
debug_string_hash (void *key, void *value, void *user)
{
  debugf ("<stringhash:%s=%s>", (char *) key, (char *) value);
}

ParseData *
pd_new ()
{
  ParseData *pd = xmalloc (sizeof (ParseData));
  pd->inbuf_sz = 1024;
  pd->inbuf = xmalloc (pd->inbuf_sz);
  return pd;
}

void
pd_free (ParseData * pd)
{
  xfree (pd->inbuf);
  xfree (pd);
}

/*
	 1. get line from pd->in into pd->inbuf 
	 2. point pd->lineptr to start of inbuf
	 return FALSE if end-of-file or i/o error
	 */
BOOL
pd_getline (ParseData * pd)
{
  pd->lineptr = NULL;
  *pd->inbuf = 0;
#ifdef HAVE_GETLINE
  if (feof (pd->in))
    return FALSE;
  pd->inbuf_line_len = getline (&pd->inbuf, &pd->inbuf_sz, pd->in);
  if (line_len < 0) {
    return FALSE;
  }
#else
  if (!fgets (pd->inbuf, pd->inbuf_sz, pd->in)) {
    return FALSE;
  }
#endif
  pd->lineptr = pd->inbuf;
  return TRUE;
}

void
handle_key (Template * tp, ParseData * pd)
{
  char *value = NULL;
  debugf ("tp key <<%s>>", pd->match);
  // examine key
  if (pd->current_list_hash) {
    value = (char *) strhash_find (pd->current_list_hash, pd->submatch);
    if (!value) {
      debugf ("<list lookup %s failed>", pd->submatch);
    }
  }
  if (!value) {
    value = (char *) strhash_find (tp->pairs, pd->submatch);
    if (!value) {
      debugf ("<lookup %s failed>", pd->submatch);
    }
  }
  if (value) {
    debugf ("pd->lineptr: %s", pd->lineptr);
    debugf ("value: %s", value);
    output (pd, value);
  } else {
    output (pd, pd->match);
  }
}
void
handle_insert (Template * tp, ParseData * pd)
{
  int ch;
  FILE *ins = fopen (pd->submatch, "r");
  if (ins == NULL) {
    debugf ("could not open (%s)", pd->submatch);
    return;
  }
  while ((ch = fgetc (ins)) != EOF) {
    fputc (ch, pd->out);
  }
  fclose (ins);
}

BOOL
get_end_index (char *lineptr, int *start, int *len)
{
  const int RCOUNT = 6;
  int result[RCOUNT];
  int linelen;
  int resultcount;

  if (!lineptr) {
    return TRUE;
  }
  debugf ("get_end_inded %s", lineptr);
  linelen = (int) strlen (lineptr);
  resultcount =
    pcre_exec (template_end_index_re, NULL, lineptr, linelen, 0, 0, result,
               RCOUNT);
  if (resultcount > 0) {
    getresult (0, result, start, len);
    return TRUE;

  }
  return FALSE;
}

void
handle_start_index (Template * tp, ParseData * pd)
{
  char *afterloop;
  LList *current_items;

  debugf ("%s", pd->submatch);
  current_items = (LList *) strhash_find (tp->lists, pd->submatch);
  if (!current_items) {
    debugf ("<list %s not found>", pd->submatch);
    output (pd, pd->match);
    return;
  }
  pd->queued_lines = llist_new ();
  {
    int start;
    int len;
    char *beforeend;
    /* queue up line until end of loop */
    if (*pd->lineptr == '\n') {
      /* skip empty start loop line */
      pd_getline (pd);
    }
    while (!get_end_index (pd->lineptr, &start, &len)) {
      debugf ("queueing: %s", pd->lineptr);
      pd->queued_lines =
        llist_append (pd->queued_lines, xstrdup (pd->lineptr));
      pd_getline (pd);
    }
    beforeend = xstrndup (pd->lineptr, start);
		if (*beforeend) {
    	debugf ("queueing: %s", beforeend);
    	pd->queued_lines = llist_append (pd->queued_lines, beforeend);
		} else {
			xfree(beforeend);
		}
		if ( (start == 0) && (*(pd->lineptr+len) ==  '\n') ) {
						/* skip empty end loop line */
						pd->lineptr += start+len+1;
		} else {
    	pd->lineptr += start + len;
		}
  }
  {
    /* we still have to examine the data after the end tag later */
    afterloop = pd->lineptr;

    while (!llist_empty (current_items)) {
      LList *lines = pd->queued_lines;
      pd->current_list_hash = (StrHash *) llist_data (current_items);

      while (!llist_empty (lines)) {
        pd->lineptr = llist_data (lines);
        debugf ("loop line: %s", pd->lineptr);
        while ((pd->lineptr != NULL) && *pd->lineptr) {
          template_examine_line (tp, pd);
        }
        lines = llist_next (lines);
      }
      current_items = llist_next (current_items);
    }

    while (!llist_empty (pd->queued_lines)) {
      xfree (llist_data (pd->queued_lines));
      pd->queued_lines = llist_remove (pd->queued_lines);
    }
    pd->lineptr = afterloop;
  }
}

/*
	 * make replacements up to end of line
	 * pd->lineptr is incremented to the end of each replacement
	 */
void
template_examine_line (Template * tp, ParseData * pd)
{
  int start;
  int len;
  int mlen;
  int mstart;
  const int RCOUNT = 6;
  int result[RCOUNT];

  pd->match_type =
    template_getmatch (tp, pd->lineptr, result, &pd->findreplace);

  if (pd->match_type == NONE) {
    output (pd, pd->lineptr);
    pd->lineptr = NULL;
    return;
  }
  getresult (0, result, &start, &len);
  // write out data before match
  outputn (pd, pd->lineptr, start);
  // move to start of match
  pd->lineptr += start;
  safe_strmincpy (pd->match, pd->lineptr, len, MAX_KEY_SIZE);
  debugf ("match (%s)", pd->match);
  if (pd->match_type != REGEX) {
    getresult (1, result, &mstart, &mlen);
    safe_strmincpy (pd->submatch, pd->lineptr + mstart - start, mlen,
                    MAX_KEY_SIZE);
    debugf ("submatch (%s)", pd->submatch);
  }
  pd->lineptr += len;
  switch (pd->match_type) {
  case KEY:
    handle_key (tp, pd);
    break;
  case INSERT:
    handle_insert (tp, pd);
    break;
  case START_INDEX:
    handle_start_index (tp, pd);
    break;
  case REGEX:
    handle_regex (tp, pd, pd->findreplace);
    break;
  case NONE:
  case END_INDEX:
    debugf ("%s", "what are we doing here?");
    break;
  }
  // move to end of match;

}

void
output (ParseData * pd, char *outstr)
{
  fputs (outstr, pd->out);
  if (g_verbose_debug) {
    fflush (pd->out);
  }
}

void
outputn (ParseData * pd, char *lineptr, size_t len)
{
  int i;
  for (i = 0; i < len; i++) {
    fputc (*lineptr, pd->out);
    lineptr++;
  }
  if (g_verbose_debug) {
    fflush (pd->out);
  }
}
void
template_parse (Template * tp, FILE * in, FILE * out)
{
  ParseData *pd = pd_new ();
  pd->in = in;
  pd->out = out;
  //debug("file len is: %d", end);
  while (pd_getline (pd)) {
    //debug("read line: %s", inbuf);
    while ((pd->lineptr != NULL) && *pd->lineptr) {
      template_examine_line (tp, pd);
    }                           // end while (*lineptr)
  }
	pd_free(pd);
  fflush (pd->out);
}

void
template_set_verbose (BOOL tf)
{
  g_verbose_debug = tf;
}
