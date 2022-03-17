PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
LDLIBS = -lpulse
LDFLAGS = -s ${LDLIBS}
INCS = -I/usr/include
CFLAGS = -pedantic -Wall -Wextra -Os ${INCS}
CC = cc

all: pasid

pasid: pasid.c
	${CC} -o $@ $< ${LDFLAGS} ${CFLAGS}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f pasid ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/pasid
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@cp -f pasid.1 ${DESTDIR}${MANPREFIX}/man1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/pasid.1

uninstall:
	@rm -f ${DESTDIR}${PREFIX}/bin/pasid
	@rm -f ${DESTDIR}${MANPREFIX}/man1/pasid.1

clean:
	@rm -f pasid

.PHONY: all clean install uninstall
