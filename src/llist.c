#include <stdlib.h>
#include <stdio.h>
#include "libtemplate/llist.h"
#include "libtemplate/xmalloc.h"

LList *
llist_new ()
{
  return NULL;
}
LList* new_node(void* data) {
		LList* list = xmalloc (sizeof (LList));
    list->next = NULL;
    list->data = data;
		return list;
}
/*
	 returns the new start of the list
	 */
LList *
llist_append (LList * list, void *item)
{
  LList *cur = list;
  if (list == NULL) {
    return new_node(item);
  }
  while (cur->next != NULL) {
    cur = cur->next;
  }
  {
    cur->next = new_node(item);
  }
  return list;
}

/*
	 returns the new start of the list
	 */
LList *
llist_prepend (LList * list, void *item)
{
  LList *cur = list;
  if (list == NULL) {
    return new_node(item);
  }
  cur->next = list;
  cur->data = item;
  return cur;
}

LList *
llist_next (LList * list)
{
  return list->next;
}

LList *
llist_remove (LList * list)
{
  LList *next = list->next;
  xfree (list);
  return next;
}

void
llist_free (LList * list)
{
  if (list != NULL) {
    LList *cur = list;
    while (cur->next != NULL) {
      LList *freeme = cur;
      cur = cur->next;
      xfree (freeme);
    }
    xfree (cur);
	}
}

void
llist_free_each (LList * list, LListForEachFunc userfunc, void *user_data)
{
  LList *cur = list;
  while (cur->next != NULL) {
    LList *freeme = cur;
    userfunc (freeme->data, user_data);
    cur = cur->next;
    xfree (freeme);
  }
  if (cur->data != NULL) {
    userfunc (cur->data, user_data);
  }
  xfree (cur);
}

BOOL
llist_empty (LList * list)
{
  return list == NULL;
}

void
llist_foreach (LList * list, LListForEachFunc userfunc, void *user_data)
{
  LList *cur = list;
  while (cur->next != NULL) {
    userfunc (cur->data, user_data);
    cur = cur->next;
  }
  userfunc (cur->data, user_data);
}

void
print_llist_item (void *data, void *userdata)
{
  printf (userdata, data);
}

void *
llist_data (LList * list)
{
  return list->data;
}

