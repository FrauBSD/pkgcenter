# $FrauBSD: pkgcenter/depend/libdpv/GNUmakefile 2019-01-19 17:44:39 -0800 freebsdfrau $

LIB=		dpv
SHLIB_MAJOR=	0
INCS=		dpv.h
MAN=		dpv.3

CFLAGS=		-fPIC -Wall -I. -I/usr/include/dialog
LIBS=		-lc -ldialog -lfigpar -lncurses
LDFLAGS=	-shared $(LIBS)

PREFIX=		/usr
LIBDIR=		$(PREFIX)/lib
INCDIR=		$(PREFIX)/include
DATADIR=	$(PREFIX)/share
MANDIR=		$(DATADIR)/man

SRCS=		dpv.c dprompt.c dialogrc.c dialog_util.c status.c util.c
OBJS=		dpv.o dprompt.o dialogrc.o dialog_util.o status.o util.o

all: GNUmakefile lib$(LIB).so.$(SHLIB_MAJOR) $(MAN).gz

lib$(LIB).so.$(SHLIB_MAJOR): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(@)

$(MAN).gz: $(MAN)
	gzip -c $(MAN) > $(@)

$(OBJS): $(SRCS)
	@eval2(){ echo "$$*"; eval "$$@"; }; \
	 set -e; \
	 for src in $(?); do \
	 	eval2 $(CC) $(CFLAGS) -c $$src -o $${src%.*}.o; \
	 done

$(SRCS): $(INCS)
	touch $(@)

install: lib$(LIB).so.$(SHLIB_MAJOR)
	install -d -o 0 -g 0 -m 755 $(LIBDIR) $(MANDIR)/man3
	install -C -o 0 -g 0 -m 444 lib$(LIB).so.$(SHLIB_MAJOR) $(LIBDIR)/
	ln -sf lib$(LIB).so.$(SHLIB_MAJOR) $(LIBDIR)/lib$(LIB).so
	install -C -o 0 -g 0 -m 444 $(INCS) $(INCDIR)/
	install -C -o 0 -g 0 -m 444 $(MAN).gz $(MANDIR)/man3/

uninstall:
	rm -f $(LIBDIR)/lib$(LIB).so.$(SHLIB_MAJOR) $(LIBDIR)/lib$(LIB).so
	rm -f $(INCDIR)/$(INCS) $(MANDIR)/man3/$(MAN).gz

clean:
	rm -f $(OBJS) $(MAN).gz $(MAN).cat.gz
	rm -f $(LIB).po $(LIB).So lib$(LIB).so lib$(LIB).so.$(SHLIB_MAJOR)
	rm -f lib$(LIB).so.$(SHLIB_MAJOR).full lib$(LIB).so.$(SHLIB_MAJOR).debug
	rm -f lib$(LIB).a lib$(LIB)_p.a

cleandepend:
	rm -f .depend .depend.* GPATH GRTAGS GSYMS GTAGS

cleandir: clean cleandepend
