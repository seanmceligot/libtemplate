#ifndef __LIBTEMPLATE_H__
#define __LIBTEMPLATE_H__

#include <stdio.h>
#include "libtemplate/llist.h"
#include "libtemplate/strhash.h"
#include "libtemplate/mybool.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*
	 The data and logic for lists can be confusing. Here is a summary 
	 starting from Template.lists:
	 ( see the source of template_addstrlist for an example )

	 StrHash* lists:
	 		key: listname 
			data TemplateListPtr
			new: template_list_new
			add: template_addlist

	 TemplateListPtr (List*):
	 		elements:  TemplateListHashPtr
			new: template_list_new
			add: template_listadd

		TemplateListHashPtr:
			key: (char*) keyname 
			data: (char*) value
			new: template_listhash_new
			add: template_listhash_put
*/
typedef struct _Template Template;
struct _Template {
  FILE *out;
  FILE *in;
  /*private*/
	/* current list in loops */
  LList *current_items; 
  StrHash *pairs;
	
  StrHash *lists;
  LList *regexes;
};
typedef StrHash *TemplateListHashPtr;
typedef LList *TemplateListPtr;

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
  // return value is the new list; make sure you use it!
TemplateListPtr template_listadd (TemplateListPtr list,
                                  TemplateListHashPtr hash);
// add list to template
void template_addlist (Template * tpe, const char *listname,
                       TemplateListPtr list);
int template_addregex (Template * tpe, const char *re_find,
                       const char *re_replace);
void template_set_verbose (BOOL on);
void template_set_debug (BOOL on);

#ifdef  __cplusplus
}
#endif

#endif // defined __LIBTEMPLATE_H
