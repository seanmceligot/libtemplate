#include "libtemplate/stringutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libtemplate/debug.h"
#include "libtemplate/xmalloc.h"

int check_replace() {
	char str[1024];
	char buf[1024];
	const char* input = "<a href=\"file:$1.html\">$1</a>";

	strcpy(str, input);	
	replace(str, "$1", "MainNode", 1024, buf);
	fprintf(stdout, "was %s; is %s\n", input, str);
	return 0;
}

int
main (int argc, char **argv)
{
  int maxlen = 100;
  char text[maxlen];
  char tempbuf[maxlen];
  char *world = "world";
  char *everyone = "everyone";
  int g_verbose_error = 1;
  int g_verbose_debug = 1;
  int g_fail = 0;

	debug("==check_stringutil==");
  safe_strncpy (text, "abc", 3);
  debugf ("save_strncpy abc=%s\n", text);
  assertf (strcmp (text, "abc") == 0, "bad value %s", text);

  strncpy (text, "hello world!", maxlen);
  debugf ("\norig: %s, find: %s, changeto: %s\n", text, world, everyone);
  replace (text, world, everyone, maxlen, tempbuf);
  debugf ("new: %s\n", text);
  assertf (strcmp (text, "hello everyone!") == 0, "bad value %s", text);

  debugf ("\norig: %s, find: %s, changeto: %s\n", text, everyone, world);
  replace (text, everyone, world, maxlen, tempbuf);
  debugf ("new: %s\n", text);
  assertf (strcmp (text, "hello world!") == 0, "bad value %s", text);

  strcat (text, " worldworldworld");
  debugf ("\norig: %s, find: %s, changeto: %s\n", text, world, everyone);
  replace (text, world, everyone, maxlen, tempbuf);
  debugf ("new: %s\n", text);
  assertf (strcmp (text, "hello everyone! everyoneeveryoneeveryone") == 0,
          "bad value %s", text);

  {
    char *list = "one:two:three";
    char **v_list = strsplitc(list, ':', 3);
    assertf (strcmp ("one", v_list[0]) == 0, "bad value %s\n", v_list[0]);
    assertf (strcmp ("two", v_list[1]) == 0, "bad value %s\n", v_list[1]);
    assertf (strcmp ("three", v_list[2]) == 0, "bad value %s\n", v_list[2]);
		strfreev(v_list);
		check_replace();
  }
  return g_fail;
}
