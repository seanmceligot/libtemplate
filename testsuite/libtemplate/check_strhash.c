#include <stdio.h>
#include <stdlib.h>
#include "libtemplate/strhash.h"
#include "libtemplate/debug.h"
#include "libtemplate/xmalloc.h"


static int g_verbose_debug=1;
static int g_fail = 0;


int
main (int argc, char **argv)
{
  StrHash* sh;
	char* str;
	debug("==check_strhash==");
	xmalloc_init();

  sh = strhash_new (1, 1);
  strhash_put (sh, "a", "one");
  strhash_put (sh, "b", "two");
  strhash_put (sh, "b", "three");
  strhash_debugf(sh);
	str = strhash_find (sh, "b");	
  debugf( "find b = (%s)\n", str );
	strhash_free(sh);
	return g_fail?1:0;
}
