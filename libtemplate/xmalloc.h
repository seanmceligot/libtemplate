#ifndef __XMALLOC_H
#define __XMALLOC_H

#include "config.h"
#include "mybool.h"

#ifdef WITH_DMALLOC
#include <dmalloc.h>
#else   /* WITH_DMALLOC */
#define WITH_DEBUG
#define xmalloc(size) xmalloc_fl(size, __FILE__, __LINE__)
#define xfree(pointer) xfree_fl(pointer, __FILE__, __LINE__)
#endif  /* WITH_DMALLOC */
void xmalloc_init ();
void xmalloc_exit ();

#ifndef WITH_DMALLOC
void *xmalloc_fl (const int size, const char *file, const int line);
void xfree_fl (void *pointer, const char *file, const int line);
char *xstrdup (const char *str);

#endif  /* WITH_DMALLOC  */
char *xstrndup (const char *str, size_t max);
void xmalloc_set_debug (BOOL on);

#endif  /* __XMALLOC_H */
