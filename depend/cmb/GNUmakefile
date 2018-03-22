# $FrauBSD: depend/cmb/GNUmakefile 2018-03-22 16:54:09 -0700 freebsdfrau $

PROG=		cmb
MAN=		cmb.1

OSX_MIN=	10.8

CFLAGS=		-g -Wall -I../libcmb

SRCS=		cmb.c
OBJS=		cmb.o

EVAL2= eval2() { echo "$$*"; eval "$$@"; }

all: $(PROG) $(MAN).gz

$(PROG): $(OBJS)
	@$(EVAL2); \
	 LD="$(LD)" LDFLAGS="$(LDFLAGS)"; \
	 case "$${UNAME_s:-$$( uname -s )}" in \
	 Darwin) LDFLAGS="-macosx_version_min $(OSX_MIN) $$LDFLAGS" ;; \
	 Linux) LD="$(CC)" ;; \
	 esac; \
	 eval2 $$LD $$LDFLAGS $(?) -lc -lcmb -o $(@)

$(MAN).gz: $(MAN)
	gzip -c $(MAN) > $(@)

$(OBJS): $(SRCS)
	@$(EVAL2); \
	 CFLAGS="$(CFLAGS)"; \
	 case "$${UNAME_s:-$$( uname -s )}" in \
	 Darwin) CFLAGS="-mmacosx-version-min=$(OSX_MIN) $$CFLAGS" ;; \
	 esac; \
	 eval2 $(CC) $$CFLAGS -c $(?) -o $(@)

install: $(PROG)
	@$(EVAL2); \
	 DESTDIR="$(DESTDIR)"; \
	 : $${DESTDIR:=/usr/local}; \
	 eval2 install -d -o 0 -g 0 -m 755 $$DESTDIR/bin $$DESTDIR/share/man/man1; \
	 eval2 install -C -o 0 -g 0 -m 555 $(PROG) $$DESTDIR/bin/; \
	 eval2 install -C -o 0 -g 0 -m 444 $(MAN).gz $$DESTDIR/share/man/man1/

clean:
	rm -f $(PROG) $(OBJS) $(MAN).gz
	rm -f libcmb.so.0
