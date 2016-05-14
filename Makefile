CFLAGS=-O2 -g
LDFLAGS=-liodbc
LIBDIR?=/usr/lib

all: libcdbc

libcdbc: cdbc.o
	$(AR) -r libcdbc.a cdbc.o

clean:
	rm -f *.o *.a *.core

install:
	install -c -m 644 cdbc.h $(DESTDIR)/usr/include/cdbc.h
	install -c -m 644 libcdbc.a $(DESTDIR)$(LIBDIR)/libcdbc.a
