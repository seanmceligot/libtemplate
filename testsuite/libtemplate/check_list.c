#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libtemplate/strhash.h"
#include "libtemplate/debug.h"
#include "libtemplate/mybool.h"
#include "libtemplate/xmalloc.h"
#include "libtemplate/libtemplate.h"


static int g_verbose_fatal = TRUE;
static int g_verbose_debug = TRUE;
static int g_fail = 0;
static Template *tp;

void
myexit ()
{
  debug ("myexit");
  if (tp != NULL) {
    template_destroy (tp);
  }
  template_shutdown ();
}

int
main (int argc, char **argv)
{
	debug("==check_list==");
	//atexit(myexit);
  xmalloc_init();
  template_set_verbose (TRUE);
  if (!template_init ()) {
    fatal ("Cannot initialize libtemplate");
    return 1;
  }
  tp = template_new ();
  {
    char *strlist = "list:a1=1,b1=2|a2=3,b2=4";
    if (!template_addstrlist (tp, strlist)) {
      fatalf ("cannot add %s\n", strlist);
      return 1;
    }
  }
	{ 
					char* instr = "List:\n"
									"${variables:}\n"
									"\t${variables.type} ${variables.name}; // define ${variables.name}\n"
									"${:variables}\nLIST_END\n";
					FILE* in = NULL;
					const size_t BUFSZ = 1024;
					char* buf[BUFSZ];
					FILE* out = open_memstream(char* buf, BUFSZ);
  				template_parse (tp, in, out);
	}
	myexit();
	xmalloc_exit();
	return g_fail?1:0;
}
