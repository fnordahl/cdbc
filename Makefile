CFLAGS+=-O2 -g
LDFLAGS+=-lodbc
LIBDIR?=/usr/lib

all: libcdbc

libcdbc: cdbc.o
	$(AR) -r libcdbc.a cdbc.o

test: libcdbc test.o
	$(CC) -o test test.o libcdbc.a $(LDFLAGS)

clean:
	rm -f *.o *.a *.core

install:
	install -c -m 644 cdbc.h $(DESTDIR)/usr/include/cdbc.h
	install -c -m 644 libcdbc.a $(DESTDIR)$(LIBDIR)/libcdbc.a
