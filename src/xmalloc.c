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

#ifdef WITH_DEBUG
static int g_verbose_debug = FALSE;
static int g_verbose_memory = FALSE;
static int g_verbose_error = TRUE;
static int g_verbose_fatal = TRUE;
static int g_verbose_fail = TRUE;
static int g_fail = 0;
static int g_calls = 0;
#endif

#define memprintf(address, fmt, args...) if(g_verbose_memory){fprintf(stderr, "%p " fmt "\n" , pointer, ## args);}
//#define memprint(msg) if(g_verbose_memory){fprintf(stderr, "mem: %s\n", msg);}

#ifndef WITH_DMALLOC
void *
xmalloc_fl (const int size, const char *file, const int line)
{
  void *pointer;

#ifdef WITH_DEBUG
  assert (size > 0, "malloc w/ zero size called");

#endif /* WITH_DEBUG  */
  pointer = malloc (size);

#ifdef WITH_DEBUG
  memprintf (pointer, "xmalloc: %s:%d", file, line);
  if (pointer == NULL)
    {
      fatalf ("out of memory at %s:%d", file, line);
      abort ();
    }
  g_calls++;

#endif /* WITH_DEBUG */
  return pointer;
}

#endif /* WITH_DMALLOC */
char *
xstrndup (const char *str, size_t max)
{
  size_t len;
  char *buf;
#ifdef WITH_DEBUG
  assert (str != NULL, "null value passes");
#endif /* WITH_DEBUG */
  len = strlen (str);
  if (len > max)
    {
      len = max;
    }
  buf = (char *) xmalloc (len + 1);
  safe_strncpy (buf, str, len);
  return buf;
}

#ifndef WITH_DMALLOC
char *
xstrdup (const char *str)
{

#ifdef WITH_DEBUG
  size_t len;
  char *buf;
  assert (str != NULL, "null value passes");
  len = strlen (str);
  buf = (char *) xmalloc (len + 1);
  safe_strncpy (buf, str, len);
  return buf;

#else /* WITH_DEBUG  */
  return strdup (str);

#endif /* WITH_DEBUG */
}

#endif /* WITH_DMALLOC */

#ifndef WITH_DMALLOC
void
xfree_fl (void *pointer, const char *file, const int line)
{

#ifdef WITH_DEBUG
  if (pointer == NULL)
    {
      errorf ("xfree on NULL pointer at %s:%d", file, line);
      return;
    }
  g_calls--;
  memprintf (pointer, "%d xfree: %s:%d", g_calls, file, line);

#endif /* WITH_DEBUG */
  free (pointer);
}

#endif
void
xmalloc_exit ()
{

#ifdef MEMWATCH
  MemWatch_print ();

#endif
#ifdef WITH_DEBUG
  debug ("xmalloc_exit");
  debugf ("unfreed %d", g_calls);

#endif
}

void
xmalloc_init ()
{

  //atexit(xmalloc_exit);
}
void
xmalloc_set_debug (BOOL on)
{
#ifdef WITH_DEBUG
  g_verbose_debug = on;
#endif
}
