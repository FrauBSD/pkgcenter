############################################################ IDENT(1)
#
# $Title: Makefile to compile Perl XS bindings to cmb(3) $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/perl/Makefile 2019-04-10 14:13:36 -0700 freebsdfrau $
#
############################################################ OBJECTS

XS=	Cmb.xs
PM=	Cmb/lib/Cmb.pm

SLIB=	libcmb.so.0
XLIB=	/usr/local/lib/libcmb.so.0
BLIB=	Cmb/blib/arch/auto/Cmb

############################################################ TARGETS

.PHONY: all

all: $(PM) $(BLIB)

.PHONY: install test

install test: all
	cd Cmb && $(MAKE) $(@)

$(PM) $(BLIB): Cmb/$(XS) Cmb/Makefile
	cd Cmb && $(MAKE)
	[ "$$( uname -s )" != Darwin ] || \
		install_name_tool -change $(SLIB) $(XLIB) $(BLIB)/Cmb.bundle

Cmb/$(XS): Cmb/.h2xs_done $(XS)
	cp $(XS) $(@)

Cmb/.h2xs_done:
	h2xs -b 5.8.5 ../cmb.h -lcmb
	rm -f Cmb/$(XS)
	touch $(@)

Cmb/Makefile:
	cd Cmb && perl Makefile.PL

.PHONY: realclean cleandir distclean

realclean cleandir distclean:
	[ ! -e Cmb/Makefile ] || ( cd Cmb && $(MAKE) realclean )
	cd Cmb && rm -f $(XS) const-c.inc const-xs.inc

################################################################################
# END
################################################################################
