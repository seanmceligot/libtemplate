INCLUDES=-I/usr/include
EXE=
GLIB=glib
GLIB_LIBS=${shell pkg-config --libs $(GLIB)}

#GLIB_A=/usr/lib/libglib-2.0.dll.a
GLIB_A=/usr/lib/libglib.a
LIBS=-L/usr/lib -lm -lpcre ${GLIB_LIBS}
GLIB_CFLAGS=${shell pkg-config --cflags $(GLIB)}
WARN=-Wall
CFLAGS=-g $(WARN) 
CC=gcc
OBJS=main.o

all:  libtemplate.a libtemplate$(EXE)

libtemplate.a: libtemplate.o 
	ar rc $@ $<  $(GLIB_A)

libtemplate.dll.a: libtemplate.o 
	gcc -g -shared -Wl,--export-all -Wl,--out-implib=libtemplate.dll.a -o libtemplate.dll libtemplate.o ${LIBS}

clean:
	rm -vf *.o libtemplate$(EXE) *.stackdump *~ *.a *.so *.dll *.la

libtemplate$(EXE): ${OBJS}  libtemplate.a
	gcc -g -o $@ ${OBJS} ${LIBS} -L. -ltemplate

test: .PHONY
	./libtemplate name=Test 'variables:name=Foo,type=int|name=bar,type=long' test.template

.PHONY:
prefix=$(DESTDIR)/usr
install: .PHONY
	if [ ! -d "$(prefix)/bin" ]; then install -d $(prefix)/bin;fi
	install -svm755 libtemplate $(prefix)/bin/
	if [ ! -d "$(prefix)/lib" ]; then install -d $(prefix)/lib;fi
	install -vm644 libtemplate.a $(prefix)/lib/
	if [ -f "libtemplate.dll" ]; then install -Dvm644 libtemplate.dll $(prefix)/lib;fi
	if [ -f "libtemplate.dll.a" ]; then install -Dvm644 libtemplate.dll.a $(prefix)/lib;fi
	if [ ! -d "$(prefix)/include" ]; then install -d $(prefix)/include;fi
	install -vm444 libtemplate.h $(prefix)/include/

%.o: %.c
	$(CC)  $(CFLAGS) -fPIC $(INCLUDES) $(GLIB_CFLAGS) -c $<

