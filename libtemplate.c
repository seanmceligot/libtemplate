//  tpe - a c template engine
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
#include "libtemplate.h"
#include "stringutil.h"

int template_verbose;

typedef struct _FindReplace FindReplace;
struct _FindReplace {
  pcre *find;
  char *replace;
};
static pcre *template_key_re;
static pcre *template_start_index_re;
static pcre *template_end_index_re;
static pcre *template_insert_re;

static char *template_key_pattern = "\\${([a-zA-Z_\\.]+)}";
static char *template_start_pattern = "\\${([a-zA-Z_]+):}";
static char *template_template_end_pattern = "\\${:([a-zA-Z_]+)}";
static char *template_insert_pattern = "\\${i:([^}]*)}";

typedef enum { NONE = 0, KEY, START_INDEX, END_INDEX, INSERT } MatchType;

// private methods
static void template_examine_line (Template * tpe, char *inbuf);
static MatchType template_getmatch (const char *line, int *out_offset,
                                    char *match, const unsigned int maxlen,
                                    int *out_len);
static BOOL template_getsubmatch (pcre * re, const char *lineptr,
                                  const int linelen, int *out_offset,
                                  int *out_len, char *out_match,
                                  const int match_maxlen);
static pcre *template_compile (const char *pattern);

int
template_init ()
{
  template_key_re = template_compile (template_key_pattern);
  template_start_index_re = template_compile (template_start_pattern);
  template_end_index_re = template_compile (template_template_end_pattern);
  template_insert_re = template_compile (template_insert_pattern);
  return (template_key_re != NULL) && (template_start_index_re != NULL)
    && (template_end_index_re != NULL) && (template_insert_re != NULL);
  template_verbose = FALSE;
}

void
template_shutdown ()
{
}

Template *
template_new ()
{
  Template *tpe = malloc (sizeof (Template));
  tpe->out = stdout;
  tpe->pairs = g_hash_table_new (g_str_hash, g_str_equal);
  tpe->lists = g_hash_table_new (g_str_hash, g_str_equal);
  tpe->current_list_hash = NULL;
  tpe->regexes = NULL;
  return tpe;
}

static void
getresult (int index, int *result, int *out_start, int *out_end, int *out_len)
{
	index *= 2;
  *out_start = result[index + 0];
  if (index == 0) {
    *out_len = result[index + 1];
    *out_end = *out_start + *out_len;
  } else {
    *out_end = result[index + 1];
    *out_len = *out_end - *out_start;
  }
}
static void
find_replace (gpointer data, gpointer user_data)
{
  FindReplace *pair = (FindReplace *) data;

  char *lineptr = (char *) user_data;
  int linelen = strlen (lineptr);
  const int RCOUNT = 99;
  int result[RCOUNT];
  int resultcount;
  static char buf[1024];

  if (data == NULL) {
    return;
  }
  resultcount =
    pcre_exec (pair->find, NULL, lineptr, linelen, 0, 0, result, RCOUNT);
  if (resultcount < 0) {
    if (resultcount != PCRE_ERROR_NOMATCH) {
      debug ("tpe:Matching error %d (%s)\n", resultcount, lineptr);
    }
  } else if (resultcount == 1) {
    int start, end, len;
    getresult (0, result, &start, &end, &len);
    debug ("line: %s\n", lineptr);
    //debug("result[%d] = %d\n", 0, result[0]);
    //debug("result[%d] = %d\n", 1, result[1]);
    //pcre_get_substring_list (lineptr, result, resultcount, matches);
    //debug ("find_replace (%s) result = %s\n", lineptr, matches[0][0]);
    debug ("%d %d %d\n", start, end, len);
    {
      int growth =
        replace_insert (lineptr + start, len, pair->replace, 1024 - start,
                        buf);
      debug ("growth: %d\n", growth);
      debug ("line: %s\n", lineptr);
    }
  } else if (resultcount > 1) {
    int i;
    int start, end, len;
    int mstart, mend, mlen;
		char insert[1024];
		strcpy(insert, pair->replace);
		debug("insert: %s\n", insert);
    debug ("line: %s\n", lineptr);
    getresult (0, result, &start, &end, &len);
    for (i = 1; i < resultcount; i++) {
			char tag[3];
			strcpy(tag, "$N");
			tag[1] = '0'+i;
			getresult(i, result, &mstart, &mend, &mlen);
			{
							char sub[mlen];
							strncpy(sub, lineptr+mstart, mlen);
							sub[mlen] = 0;
							debug("sub: %s\n", sub);
							replace(insert, tag, sub, 1024, buf);
							debug("insert now: %s\n", insert);
			}
    }
		replace_insert(lineptr+start, len, insert, 1024-start, buf);
  }

}

static void
free_find_replace (gpointer data, gpointer user_data)
{
  FindReplace *pair = (FindReplace *) data;
  if (pair != NULL) {
    free (pair->replace);
    free (pair);
  }
}

void
template_destroy (Template * tpe)
{
  g_hash_table_destroy (tpe->pairs);
  // TODO: free list items and list names
  g_hash_table_destroy (tpe->lists);
  if (tpe->regexes) {
    g_slist_foreach (tpe->regexes, (GFunc) free_find_replace, NULL);
    g_slist_free (tpe->regexes);
  }
  tpe->regexes = NULL;
  free (tpe);
}

void
template_parse (Template * tpe, FILE * _in)
{
  static char inbuf[1024];
  tpe->in = _in;
  tpe->filelinestart = ftell (tpe->in);
  while (fgets (inbuf, sizeof (inbuf), tpe->in) != NULL) {
    template_examine_line (tpe, inbuf);
    tpe->filelinestart = ftell (tpe->in);
  }
}
void
template_addkeyvalue (Template * tpe, const char *key, const char *value)
{
  debug ("tpe:%s=%s\n", key, value);
  g_hash_table_insert (tpe->pairs, (gpointer) key, (gpointer) value);
}

TemplateListPtr
template_list_new ()
{
  GSList *list = g_slist_alloc ();
  return list;
}

TemplateListHashPtr
template_listhash_new ()
{
  GHashTable *hash = g_hash_table_new (g_str_hash, g_str_equal);
  return hash;
}

void
template_listhashput (TemplateListHashPtr hash, const char *key,
                      const char *value)
{
  debug ("%s = %s\n", key, value);
  g_hash_table_insert (hash, (gpointer) key, (gpointer) value);
}

void
template_listadd (TemplateListPtr list, TemplateListHashPtr hash)
{
  g_slist_append (list, (gpointer) hash);
}

void
template_addlist (Template * tpe, const char *listname, TemplateListPtr list)
{
  debug ("adding list %s\n", listname);
  g_hash_table_insert (tpe->lists, (gpointer) listname, list);
}

// str is like:
// listname:row1_key1=val1,row1_key2=val2|row2_key1=val1,row2_key2=val2
int
template_addstrlist (Template * tpe, const char *str)
{
  gchar *listname;
  gchar *strwholelist;
  gchar **v_commalist;
  int listno = 0;
  TemplateListPtr tpelist;
  // variables:name=Foo,type=int|name=bar,type,long
  gchar **v_namelist = g_strsplit (str, ":", 2);
  if (!v_namelist[1]) {
    return FALSE;
  }
  listname = strdup (v_namelist[0]);
  strwholelist = v_namelist[1];
  v_commalist = g_strsplit (strwholelist, "|", 0);
  tpelist = template_list_new ();
  while (v_commalist[listno] != NULL) {
    int pairno = 0;
    gchar **v_pairs = g_strsplit (v_commalist[listno], ",", 2);
    TemplateListHashPtr tpelistpairs = template_listhash_new ();
    while (v_pairs[pairno] != NULL) {
      gchar **keyvalue = g_strsplit (v_pairs[pairno], "=", 0);
      char *key = keyvalue[0];
      char *value = keyvalue[1];
      char *keyname = malloc (strlen (listname) + strlen (key) + 2);
      strcpy (keyname, listname);
      strcat (keyname, ".");
      strcat (keyname, key);
      template_listhashput (tpelistpairs, keyname, value);
      pairno++;
      //g_strfreev (keyvalue);
      //free (keyname);
    }
    g_strfreev (v_pairs);
    template_listadd (tpelist, tpelistpairs);
    listno++;
  }
  template_addlist (tpe, listname, tpelist);
  g_strfreev (v_namelist);
  g_strfreev (v_commalist);
  return TRUE;
}

BOOL
template_getsubmatch (pcre * re, const char *lineptr, const int linelen,
                      int *out_offset, int *out_len, char *out_match,
                      const int match_maxlen)
{
  const int RCOUNT = 6;
  int result[RCOUNT];
  int resultcount =
    pcre_exec (re, NULL, lineptr, linelen, 0, 0, result, RCOUNT);
  if (resultcount < 0) {
    if (resultcount != PCRE_ERROR_NOMATCH) {
      debug ("tpe:Matching error %d (%s)\n", resultcount, lineptr);
    }
    return FALSE;
  } else if (resultcount == 0) {
    return FALSE;
  }
  *out_offset = result[0];
  *out_len = result[1];
  if (resultcount > 1) {
    const int mstart = result[2];
    const int mend = result[3];
    const int mlen = min (mend - mstart, match_maxlen);
    strncpy (out_match, lineptr + mstart, mlen);
    out_match[mlen] = 0;
    //debug("<<<%s>>>", out_match);
  }
  return TRUE;
}

static pcre *
template_compile (const char *pattern)
{
  const char *error;
  int erroffset;
  pcre *re;
  //debug("tpe:%s\n", pattern);
  re = pcre_compile (pattern, 0, &error, &erroffset, NULL);
  if (!re) {
    debug ("PCRE compilation failed at offset %d: %s\n", erroffset, error);
    return NULL;
  }
  return re;
}

MatchType
template_getmatch (const char *line, int *out_offset, char *match,
                   const unsigned int maxlen, int *out_len)
{
  int linelen = (int) strlen (line);
  if (template_getsubmatch
      (template_start_index_re, line, linelen, out_offset, out_len, match,
       maxlen)) {
    return START_INDEX;
  }
  if (template_getsubmatch
      (template_end_index_re, line, linelen, out_offset, out_len, match,
       maxlen)) {
    return END_INDEX;
  }
  if (template_getsubmatch
      (template_key_re, line, linelen, out_offset, out_len, match, maxlen)) {
    return KEY;
  }
  if (template_getsubmatch
      (template_insert_re, line, linelen, out_offset, out_len, match, maxlen))
  {
    return INSERT;
  }

  return NONE;
}

int
template_addregex (Template * tpe, const char *re_find, const char *replace)
{

  pcre *fre = template_compile (re_find);
  FindReplace *fr;
  if (!fre) {
    return FALSE;
  }
  fr = (FindReplace *) malloc (sizeof (FindReplace));
  fr->find = fre;
  fr->replace = strdup (replace);
  if (!tpe->regexes) {
    tpe->regexes = g_slist_alloc ();
  }
  g_slist_append (tpe->regexes, (gpointer) fr);
  return TRUE;
}

void
debug_string_hash (gpointer key, gpointer value, gpointer user)
{
  debug ("<stringhash:%s=%s>", (char *) key, (char *) value);
}

void
template_examine_line (Template * tpe, char *inbuf)
{
  char tmpbuf[1024];
  char match[1024];
  int offset = 0;
  int out_len = 0;
  int type;
  long loop_start;
  char *lineptr = inbuf;
  char *value;

  inbuf[strlen (inbuf) - 1] = '\0';
  if (tpe->regexes) {
    g_slist_foreach (tpe->regexes, (GFunc) find_replace, inbuf);
  }

  type = template_getmatch (lineptr, &offset, match, 1024, &out_len);
  while (type == KEY || type == INSERT) {
    if (type == KEY) {
      // print up to offset
      //printf("*lineptr %c, offset %d\n", *lineptr, offset);
      {
        int i;
        for (i = 0; i < offset; i++) {
          fputc (lineptr[i], tpe->out);
        }
      }
      debug ("tpe key <<%s>>", match);
      value = NULL;
      // examine key
      if (tpe->current_list_hash) {
        value = (char *) g_hash_table_lookup (tpe->current_list_hash, match);
        if (!value) {
          debug ("<list lookup %s failed>", match);
        }
      }
      if (!value) {
        value = (char *) g_hash_table_lookup (tpe->pairs, match);
        if (!value) {
          debug ("<lookup %s failed>", match);
        }
      }
      if (value) {
        fprintf (tpe->out, "%s", value);
      } else {
        fprintf (tpe->out, "%s", match);
      }
      lineptr += out_len;
    } else {                    //INSERT
      FILE *ins = fopen (match, "r");
      if (ins == NULL) {
        debug ("could not open (%s)\n", match);
      } else {
        while (fgets (tmpbuf, sizeof (tmpbuf), ins) != NULL) {
          fputs (tmpbuf, tpe->out);
        }
        fclose (ins);
      }
      lineptr += out_len;
    }
    type = template_getmatch (lineptr, &offset, match, 1024, &out_len);
  }
  switch (type) {
  case START_INDEX:
    loop_start = tpe->filelinestart + strlen (lineptr) + 1;
    //debug("tpe list: %s\n", match);
    tpe->current_items = (GSList *) g_hash_table_lookup (tpe->lists, match);
    if (tpe->current_items) {
      tpe->current_items = (GSList *) g_slist_next (tpe->current_items);
      if (tpe->current_items) {
        tpe->current_list_hash =
          (GHashTable *) g_slist_nth_data (tpe->current_items, 0);
        //g_hash_table_foreach(tpe->current_list_hash, debug_string_hash,NULL);
      }
    } else {
      debug ("<list %s not found>", match);
    }
    break;
  case END_INDEX:
    if (!tpe->current_items) {
      break;
    }
    tpe->current_items = (GSList *) g_slist_next (tpe->current_items);
    if (tpe->current_items != NULL) {
      ////fprintf(stderr/gc, "seeking to %ld\n", loop_start );
      fseek (tpe->in, loop_start, SEEK_SET);
      tpe->current_list_hash =
        (GHashTable *) g_slist_nth_data (tpe->current_items, 0);
    } else {
      tpe->current_items = NULL;
      tpe->current_list_hash = NULL;
    }
    break;
  case NONE:
    fprintf (tpe->out, "%s\n", lineptr);
    break;
  }                             // switch
}
