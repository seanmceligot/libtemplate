prefix=$(DESTDIR)/usr
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
OBJS=main.o stringutil.o

all:  libtemplate.a template$(EXE)

libtemplate.a: libtemplate.o 
	ar rc $@ $<  $(GLIB_A)

libtemplate.dll.a: libtemplate.o 
	gcc -g -shared -Wl,--export-all -Wl,--out-implib=libtemplate.dll.a -o libtemplate.dll libtemplate.o ${LIBS}

clean:
	rm -vf *.o libtemplate$(EXE) *.stackdump *~ *.a *.so *.dll *.la

template$(EXE): ${OBJS}  libtemplate.a
	gcc -g -o $@ ${OBJS} -L. ${LIBS} -ltemplate

ARGS=--verbose -k name=Test -l 'variables:name=Foo,type=int|name=bar,type=long' -r '/replace([0-9]*)Numbers/substring is ($$1)/' -r /replaceAll/AllWASREPLACED/ test.template
test:
	./template ${ARGS}

gdb:
	echo "set args ${ARGS}" > gdb.args
	gdb -x gdb.args template 

ef:
	LD_PRELOAD=libefence.so.0.0 ./template ${ARGS}

.PHONY: ef gdb test splint

install: .PHONY
	if [ ! -d "$(prefix)/bin" ]; then install -d $(prefix)/bin;fi
	install -svm755 template$(EXE) $(prefix)/bin/
	if [ ! -d "$(prefix)/lib" ]; then install -d $(prefix)/lib;fi
	if [ -f "libtemplate.a" ]; then install -vm644 libtemplate.a $(prefix)/lib;fi
	if [ -f "libtemplate.dll" ]; then install -vm644 libtemplate.dll $(prefix)/lib;fi
	if [ -f "libtemplate.dll.a" ]; then install -vm644 libtemplate.dll.a $(prefix)/lib;fi
	if [ ! -d "$(prefix)/include" ]; then install -d $(prefix)/include;fi
	install -vm444 libtemplate.h $(prefix)/include/

%.o: %.c
	$(CC)  $(CFLAGS) -fPIC $(INCLUDES) $(GLIB_CFLAGS) -c $<

splint:
	splint +ptrnegate -predboolint -I/usr/include/glib-1.2 -I/usr/lib/glib/include +posixstrictlib +charint ${file}
