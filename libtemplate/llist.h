#ifndef __LLIST_H
#define __LLIST_H

#include "libtemplate/mybool.h"
typedef void (*LListForEachFunc) (void *data, void *user_data);

typedef struct _LList LList;
struct _LList {
  LList *next;
  void *data;
};

LList *llist_new ();
/*
	 returns the new start of the list
	 */
LList *llist_prepend (LList * list, void *item);
LList *llist_append (LList * list, void *item);
LList *llist_next (LList * list);
LList *llist_remove (LList * list);
void llist_free (LList * list);
void llist_free_each (LList * list, LListForEachFunc foreach,
                      void *user_data);
void llist_foreach (LList * list, LListForEachFunc foreach, void *user_data);
void print_llist_item (void *data, void *userdata);
void *llist_data (LList * list);
BOOL llist_empty (LList * list);
#endif
