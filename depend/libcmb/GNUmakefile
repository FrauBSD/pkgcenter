# $FrauBSD: depend/libcmb/GNUmakefile 2018-03-22 16:52:34 -0700 freebsdfrau $

LIB=		cmb
SHLIB_MAJOR=	0
INCS=		cmb.h
MAN=		cmb.3

OSX_MIN=	10.6

CFLAGS=		-g -Wall -fPIC

SRCS=		cmb.c
OBJS=		cmb.o

EVAL2= eval2() { echo "$$*"; eval "$$@"; }

all: lib$(LIB).so.$(SHLIB_MAJOR) $(MAN).gz

lib$(LIB).so.$(SHLIB_MAJOR): $(OBJS)
	@$(EVAL2); \
	 LD="$(LD)" LDFLAGS="$(LDFLAGS)"; \
	 case "$${UNAME_s:-$$( uname -s )}" in \
	 Darwin) LDFLAGS="-dylib -macosx_version_min $(OSX_MIN) $$LDFLAGS" ;; \
	 Linux) LD="$(CC)" LDFLAGS="-shared $$LDFLAGS" ;; \
	 esac; \
	 eval2 $$LD -lc $$LDFLAGS $(OBJS) -o $(@)

$(MAN).gz: $(MAN)
	gzip -c $(MAN) > $(@)

$(OBJS): $(SRCS)
	@$(EVAL2); \
	 CFLAGS="$(CFLAGS)"; \
	 case "$${UNAME_s:-$$( uname -s )}" in \
	 Darwin) CFLAGS="-mmacosx-version-min=$(OSX_MIN) $$CFLAGS" ;; \
	 esac; \
	 eval2 $(CC) $$CFLAGS -c $(?) -o $(@)

$(SRCS): $(INCS)
	touch $(@)

install: lib$(LIB).so.$(SHLIB_MAJOR)
	@$(EVAL2); \
	 DESTDIR="$(DESTDIR)"; \
	 : $${DESTDIR:=/usr/local}; \
	 eval2 install -d -o 0 -g 0 -m 755 $$DESTDIR/lib $$DESTDIR/share/man/man3; \
	 eval2 install -C -o 0 -g 0 -m 444 lib$(LIB).so.$(SHLIB_MAJOR) $$DESTDIR/lib/; \
	 eval2 ln -sf lib$(LIB).so.$(SHLIB_MAJOR) $$DESTDIR/lib/lib$(LIB).so; \
	 eval2 install -C -o 0 -g 0 -m 444 $(MAN).gz $$DESTDIR/share/man/man3/

clean:
	rm -f $(OBJS) lib$(LIB).so.$(SHLIB_MAJOR) $(MAN).gz
