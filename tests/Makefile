CFLAGS+=-O2 -g -fpic -I../lib
LDFLAGS+=-lodbc

test: test.o
	$(CC) -o test test.o ../lib/libcdbc.a $(LDFLAGS)

test-so: test.o
	$(CC) -o test-so test.o -L../lib -lcdbc $(LDFLAGS)

clean:
	rm -f *.o *.a *.core
