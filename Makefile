.POSIX:
.PHONY: all clean install uninstall dist

include config.mk

all: pasid

pasid: pasid.o
	$(CC) $(LDFLAGS) -o pasid pasid.o $(LDLIBS)

clean:
	rm -f pasid pasid.o pasid-$(VERSION).tar.gz

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f pasid $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/pasid
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f pasid.1 $(DESTDIR)$(MANPREFIX)/man1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/pasid.1

dist: clean
	mkdir -p pasid-$(VERSION)
	cp -R COPYING config.mk Makefile README pasid.1 \
		pasid.c pasid-$(VERSION)
	tar -cf pasid-$(VERSION).tar pasid-$(VERSION)
	gzip pasid-$(VERSION).tar
	rm -rf pasid-$(VERSION)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/pasid
	rm -f $(DESTDIR)$(MANPREFIX)/man1/pasid.1
