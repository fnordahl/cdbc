CFLAGS+=-O2 -g -fpic -I../lib
LDFLAGS+=-lodbc -lcdbc -L../lib
SNAPCRAFT_PROVIDER?=multipass
SNAPCRAFT_OUTPUT?=cdbc-test.snap

test: test.o
	$(CC) -o test test.o $(LDFLAGS)

test-so: test.o
	$(CC) -o test-so test.o $(LDFLAGS)

clean:
	rm -f *.o *.a *.core

install:
	mkdir -p $(DESTDIR)/usr/bin
	install -c -m 755 test $(DESTDIR)/usr/bin/cdbc-test

test-snap: $(SNAPCRAFT_OUTPUT)
$(SNAPCRAFT_OUTPUT):
	snapcraft snap --provider=$(SNAPCRAFT_PROVIDER) -o $(SNAPCRAFT_OUTPUT)
