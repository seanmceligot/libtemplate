#ifndef __XMALLOC_H
#define __XMALLOC_H

#define xmalloc(size) my_xmalloc(size, __FILE__, __LINE__)
#define xfree(pointer) my_xfree(pointer, __FILE__, __LINE__)

void * my_xmalloc(const int size, const char* file, const int line);
void my_xfree(void * pointer, const char* file, const int line);
void xmalloc_init();
void xmalloc_exit();
char* xstrdup(const char* str);
char* xstrndup(const char* str, int max);

#endif
