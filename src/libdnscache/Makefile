## make file for libdnscache

MAINFILE=libdnscache.so
LIBVER=1.0.1
LIBFILE=$(MAINFILE).$(LIBVER)
SONAME=$(MAINFILE).1

all: $(LIBFILE)

MANPATH=/usr/local/man

LIBS=
ARGS=
REQH=

# Need to be able to make 'man-pages' as well.  Not sure where to get the source for those... 

libdnscache.o: dnscache.c dnscache.h $(REQH)
	gcc -c -fPIC dnscache.c -o $@ $(ARGS)

$(LIBFILE): libdnscache.o
	gcc -shared -Wl,-soname,$(SONAME) -o $@ $^


install: $(LIBFILE) dnscache.h
	cp dnscache.h /usr/include/
	cp $(LIBFILE) /usr/lib/
	@-test -e /usr/lib/$(MAINFILE) && rm /usr/lib/$(MAINFILE)
	ln -s /usr/lib/$(LIBFILE) /usr/lib/$(MAINFILE)
	nice ldconfig
	@echo "Install complete."

manpages: 
	@mkdir tmp.install
	@cp manpages/* tmp.install/
	@gzip tmp.install/*.3
	@cp -v tmp.install/*.3.gz $(MANPATH)/man3/
	@rm -r tmp.install


clean:
	@-[ -e libdnscache.o ] && rm libdnscache.o
	@-[ -e libdnscache.so* ] && rm libdnscache.so*
	@-test -e lib-test && rm lib-test

