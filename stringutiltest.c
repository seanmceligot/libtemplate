#include "stringutil.h"
#include <stdio.h>

void
assert (int assertion, int line)
{
  if (!assertion) {
    fprintf (stderr, "asertion at line %d failed\n", line);
    abort ();
  }
}
int
main (int argc, char **argv)
{
  int maxlen = 100;
  char text[maxlen];
  char tempbuf[maxlen];
  char *world = "world";
  char *everyone = "everyone";

  safe_strncpy (text, "abc", 3);
  printf ("save_strncpy abc=%s\n", text);
  assert (strcmp (text, "abc") == 0, __LINE__);

  strncpy (text, "hello world!", maxlen);
  printf ("\norig: %s, find: %s, changeto: %s\n", text, world, everyone);
  replace (text, world, everyone, maxlen, tempbuf);
  printf ("new: %s\n", text);
  assert (strcmp (text, "hello everyone!") == 0, __LINE__);

  printf ("\norig: %s, find: %s, changeto: %s\n", text, everyone, world);
  replace (text, everyone, world, maxlen, tempbuf);
  printf ("new: %s\n", text);
  assert (strcmp (text, "hello world!") == 0, __LINE__);

  strcat (text, " worldworldworld");
  printf ("\norig: %s, find: %s, changeto: %s\n", text, world, everyone);
  replace (text, world, everyone, maxlen, tempbuf);
  printf ("new: %s\n", text);
  assert (strcmp (text, "hello everyone! everyoneeveryoneeveryone") == 0,
          __LINE__);

}
