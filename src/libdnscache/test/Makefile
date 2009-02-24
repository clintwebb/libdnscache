## make file for libdnscache test apps

all: lib-test dns-test


LIBS=
ARGS=
REQH=

lib-test: lib-test.c $(REQH) /usr/include/dnscache.h
	gcc lib-test.c -o $@ -ldnscache

dns-test: dns-test.c $(REQH) /usr/include/dnscache.h
	gcc dns-test.c -o $@ -ldnscache -levactions -levent


clean:
	@-test -e lib-test && rm lib-test
	@-test -e dns-test && rm dns-test



