LDADDS=-lpcre
INCLUDES=-I${top_srcdir}

TESTS = check_stringutil check_llist check_strhash
check_PROGRAMS = ${TESTS}

check_stringutil_SOURCES = check_stringutil.c 
check_stringutil_LDADD= $(LDADDS) ../../src/stringutil.o ../../src/llist.o ../../src/xmalloc.o

check_llist_SOURCES = check_llist.c 
check_llist_LDADD= $(LDADDS) ../../src/llist.o ../../src/xmalloc.o ../../src/stringutil.o

check_strhash_SOURCES = check_strhash.c 
check_strhash_LDADD= $(LDADDS) ../../src/strhash.o ../../src/xmalloc.o ../../src/stringutil.o ../../src/llist.o

