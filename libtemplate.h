#ifndef __TPE_H
#define __TPE_H

#include <glib.h>
#include <sys/types.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef  __cplusplus
extern "C"
{
#endif

  typedef struct _Template Template;
  struct _Template
  {
    FILE *out;
    FILE *in;
    //private
    GSList *current_items;
    GHashTable *current_list_hash;
    GHashTable *pairs;
    GHashTable *lists;
    long filelinestart;
    GSList *regexes;
  };
  typedef GHashTable *TemplateListHashPtr;
  typedef GSList *TemplateListPtr;

// call template_init before anything else once per session
  int template_init ();
// call at exit
  void template_shutdown ();

// call to get a new initialized tpe instance
  Template *template_new ();

// when to destroy a tpe instance
  void template_destroy (Template * tpe);

// add string in format
// key=value
//   or
// listkey1:key1=value1,key2=value2|key1=value3,key2=value4
  int template_addstrlist (Template * tpe, const char *str);

// add key/value
  void template_addkeyvalue (Template * tpe, const char *key,
			     const char *value);

// template_parse to template and write to tpe->out
  void template_parse (Template * tpe, FILE * in, FILE * out);

// get a new list
  TemplateListPtr template_list_new ();
// get a new list hashtable
  TemplateListHashPtr template_listhash_new ();
// add key/value to hashtable
  void template_listhashput (TemplateListHashPtr hash, const char *key,
			     const char *value);
// addhashtable to list 
  void template_listadd (TemplateListPtr list, TemplateListHashPtr hash);
// add list to template
  void template_addlist (Template * tpe, const char *listname,
			 TemplateListPtr list);
  int template_addregex (Template * tpe, const char *re_find,
			 const char *re_replace);
#ifdef  __cplusplus
}				// extern c
#endif
#endif				// defined __LIBTEMPLATE_H
