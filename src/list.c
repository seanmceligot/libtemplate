#include <stdlib.h>
#include <stdio.h>
/*
 	* a really simple list
	* maybe I will improve it someday.
*/

typedef struct _List List;
struct _List {
  void **list;
  int count;
  int allocated;
  int grow;
};

void
list_new (List * list, int count, int grow)
{
  list->allocated = count;
  list->list = (void **) xmalloc (list->allocated * sizeof (char *));
  list->count = 0;
  list->grow = grow;
}

void
list_add (List * list, void *data)
{
  list->count++;
  if (list->count > list->allocated) {
    list->allocated += list->grow;
    realloc (list->list, list->allocated * sizeof (char *));
  }
  list->list[list->count - 1] = data;
}

void *
list_get (List * list, int index)
{
  return list->list[index];
}

int
list_size (List * list)
{
  return list->count;
}

void
list_debugf (List * list)
{
  int i;
  for (i = 0; i < list->count; i++) {
    fprintf (stdout, "%d: (%s)\n", i, (char *) list->list[i]);
  }
}
void
list_free (List * list)
{
  xfree (list->list);
}

#ifdef LIST_MAIN
int
main (int argc, char **argv)
{
  List list;
  list_new (&list, 1, 1);
  list_add (&list, "one");
  list_add (&list, "two");
  list_debugf (&list);
  fprintf (stdout, "get 1 = (%s)\n", (void *) list_get (&list, 1));
  list_free (&list);
}
#endif
