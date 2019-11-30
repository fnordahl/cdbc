CFLAGS+=-O2 -g -fpic
LDFLAGS+=-lodbc
LIBDIR?=/usr/lib

all: libcdbc libcdbc.so

libcdbc: cdbc.o
	$(AR) -r libcdbc.a cdbc.o

libcdbc.so: cdbc.o
	$(CC) -shared -o libcdbc.so.0 cdbc.o

clean:
	rm -f *.o *.a *.core

install:
	install -c -m 644 cdbc.h $(DESTDIR)/usr/include/cdbc.h
	install -c -m 644 libcdbc.a $(DESTDIR)$(LIBDIR)/libcdbc.a
	install -c -m 644 libcdbc.so.0 $(DESTDIR)$(LIBDIR)/libcdbc.so.0
