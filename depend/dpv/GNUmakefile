# $FrauBSD: pkgcenter/depend/dpv/GNUmakefile 2019-01-19 17:44:56 -0800 freebsdfrau $

PROG=		dpv
MAN=		dpv.1

CFLAGS=		-Wall -I. -I/usr/include/dialog
LIBS=		-lc -ldialog -ldpv -lfigpar -lncurses
LDFLAGS=	$(LIBS)

PREFIX=		/usr
BINDIR=		$(PREFIX)/bin
DATADIR=	$(PREFIX)/share
MANDIR=		$(DATADIR)/man

SRCS=		dpv.c
OBJS=		dpv.o

all: $(PROG) $(MAN).gz

$(PROG): $(OBJS)
	@eval2(){ echo "$$*"; eval "$$*"; }; \
	 LDFLAGS="$(LDFLAGS)"; \
	 [ "$${UNAME_s:-$$( uname -s )}" = Linux ] && \
	 	LDFLAGS="$$LDFLAGS -lbsd"; \
	 eval2 $(CC) $(OBJS) $$LDFLAGS -o $(@)

$(MAN).gz: $(MAN)
	gzip -c $(MAN) > $(@)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -c $(?) -o $(@)

install: $(PROG)
	install -d -o 0 -g 0 -m 755 $(BINDIR) $(MANDIR)/man1
	install -C -o 0 -g 0 -m 555 $(PROG) $(BINDIR)/
	install -C -o 0 -g 0 -m 444 $(MAN).gz $(MANDIR)/man1/

uninstall:
	rm -f $(BINDIR)/$(PROG) $(MANDIR)/man1/$(MAN).gz

clean:
	rm -f $(OBJS) $(MAN).gz $(MAN).cat.gz
	rm -f $(PROG) $(PROG).full $(PROG).debug

cleandepend:
	rm -f .depend .depend.* GPATH GRTAGS GSYMS GTAGS

cleandir: clean cleandepend
