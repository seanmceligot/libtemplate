#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libtemplate/debug.h"
#include "libtemplate/mybool.h"
#include "libtemplate/xmalloc.h"
#include "libtemplate/stringutil.h"

#ifdef MEMWATCH
#include <memwatch.h>
#endif

static int g_verbose_debug=FALSE;
static int g_verbose_memory=FALSE;
static int g_verbose_error=TRUE;
static int g_verbose_fatal=TRUE;
static int g_verbose_fail=TRUE;
static int g_fail = 0;
static int g_calls = 0;

#define memprintf(address, fmt, args...) if(g_verbose_memory){fprintf(stderr, "%p " fmt "\n" , pointer, ## args);}
//#define memprint(msg) if(g_verbose_memory){fprintf(stderr, "mem: %s\n", msg);}

void * my_xmalloc(const int size, const char* file, const int line) {
  void * pointer;
	assert(size > 0, "malloc w/ zero size called");
  pointer = malloc(size);
 	memprintf(pointer, "xmalloc: %s:%d", file, line);
  if (pointer == NULL) {
    fatalf("out of memory at %s:%d", file, line);
		abort();
  }
	g_calls++;
  return pointer;
}
char* xstrndup(const char* str, int max) 
{
			size_t len;
			char* buf;
			assert(str != NULL,"null value passes");
			len = strlen(str);
			if (len > max) {
							len = max;
			}
			buf =(char*)	xmalloc(len+1);
			safe_strncpy(buf, str, len);
			return buf;		
}
char* xstrdup(const char* str) 
{
			size_t len;
			char* buf;
			assert(str != NULL,"null value passes");
			len = strlen(str);
			buf =(char*)	xmalloc(len+1);
			safe_strncpy(buf, str, len);
			return buf;		
}
void my_xfree(void * pointer, const char* file, const int line) {
  if (pointer == NULL) {
    errorf("xfree on NULL pointer at %s:%d", file, line);
		return;
	}
	g_calls--;
 	memprintf(pointer, "%d xfree: %s:%d", g_calls, file, line);
  free(pointer);
}

void
xmalloc_exit()
{
  debug ("xmalloc_exit");
#ifdef MEMWATCH
  MemWatch_print();
#endif
	debugf("unfreed %d", g_calls);
}

void xmalloc_init() {
				//atexit(xmalloc_exit);
}
