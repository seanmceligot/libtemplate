#include "libtemplate/llist.h"
#include "libtemplate/debug.h"
#include <stdio.h>
#include <string.h>
#include "libtemplate/xmalloc.h"

static int g_verbose_error = 1;
static int g_verbose_debug = 1;
static int g_fail = 0;
static int g_index;
static char *OK[3] = { "one", "two", "three" };

void
assert_llist_item (void *data, void *userdata)
{
  debugf ("%s: %s", (char *) userdata, (char *) data);
  assert_str_equals ((char *) data, OK[g_index++]);
}

LList *
getlist ()
{
  LList *list = llist_new ();
  list = llist_append (list, "one");
  list = llist_append (list, "two");
  list = llist_append (list, "three");
  return list;
}
void check_free_each() {
  LList *list;
  debug ("llist_free_each");
  list = getlist ();
  g_index = 0;
  llist_free_each (list, assert_llist_item, "llist_free_each");
}
void check_next() {
  LList *list, *head;
  debug ("llist_next");
  head=list = getlist ();
  g_index = 0;
  while (!llist_empty (list)) {
    char *str = (char *) llist_data (list);
    assert_llist_item (str, "llist_next");
    list = llist_next (list);
  }
  llist_free (head);
}
void check_foreach() {
  LList *list;
  debug ("llist_foreach");
  list = getlist ();
  g_index = 0;
  llist_foreach (list, assert_llist_item, "llist_foreach");
  llist_free (list);
}
void check_remove() {
  LList *list;
  debug ("llist_remove");
  list = getlist ();
  g_index = 0;
  while (!llist_empty (list)) {
    char *str = (char *) llist_data (list);
    assert_llist_item (str, "llist_remove");
    list = llist_remove (list);
  }
  llist_free (list);
}
int
main (int argc, char **argv)
{
  debug ("==check_llist==");
  xmalloc_init ();

	//check_foreach();
	//check_free_each();
	check_next();
	//check_remove();

	debugf ("failures: %d", g_fail);
  return g_fail ? 1 : 0;
}
