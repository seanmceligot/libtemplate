AM_CFLAGS= -Wall #-DMEMWATCH
LDADDS=-lpcre

DISTCLEANFILES = .deps/*.P
INCLUDES=-I${top_srcdir}

lib_LIBRARIES= libtemplate.a
libtemplate_a_SOURCES = xmalloc.c libtemplate.c llist.c stringutil.c strhash.c 

bin_PROGRAMS = template
template_SOURCES = template.c xmalloc.c libtemplate.c llist.c stringutil.c strhash.c 
template_LDADD=${LDADDS}

