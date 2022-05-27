VERSION = 1.0.1-rev+${shell git rev-parse --short=16 HEAD}
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
LDLIBS = -lpulse
LDFLAGS = -s ${LDLIBS}
INCS = -I/usr/include
CFLAGS = -std=c99 -pedantic -Wall -Wextra -Os ${INCS} -DVERSION="\"${VERSION}\""
CC = cc

SRC = src/debug.c \
	  src/pasid.c

OBJ = ${SRC:.c=.o}

all: pasid

${OBJ}:	src/debug.h

pasid: ${OBJ}
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f pasid ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/pasid
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@cp -f man/pasid.1 ${DESTDIR}${MANPREFIX}/man1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/pasid.1

dist: clean
	@mkdir -p pasid-${VERSION}
	@cp -R LICENSE Makefile README man src pasid-${VERSION}
	@tar -cf pasid-${VERSION}.tar pasid-${VERSION}
	@gzip pasid-${VERSION}.tar
	@rm -rf pasid-${VERSION}

uninstall:
	@rm -f ${DESTDIR}${PREFIX}/bin/pasid
	@rm -f ${DESTDIR}${MANPREFIX}/man1/pasid.1

clean:
	@rm -f pasid pasid-${VERSION}.tar.gz ${OBJ}

.PHONY: all clean install uninstall dist
