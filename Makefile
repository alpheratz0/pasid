VERSION = 0.1.0

CC      = cc
CFLAGS  = -std=c99 -pedantic -Wall -Wextra -Os -DVERSION="\"${VERSION}\""
LDLIBS  = -lpulse
LDFLAGS = -s ${LDLIBS}

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

all: pasid

.c.o:
	${CC} -c $< ${CFLAGS}

pasid: pasid.o
	${CC} -o $@ $< ${LDFLAGS}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f pasid ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/pasid
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f man/pasid.1 ${DESTDIR}${MANPREFIX}/man1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/pasid.1

dist: clean
	mkdir -p pasid-${VERSION}
	cp -R LICENSE Makefile README man src pasid-${VERSION}
	tar -cf pasid-${VERSION}.tar pasid-${VERSION}
	gzip pasid-${VERSION}.tar
	rm -rf pasid-${VERSION}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/pasid
	rm -f ${DESTDIR}${MANPREFIX}/man1/pasid.1

clean:
	rm -f pasid pasid.o pasid-${VERSION}.tar.gz

.PHONY: all clean install uninstall dist
