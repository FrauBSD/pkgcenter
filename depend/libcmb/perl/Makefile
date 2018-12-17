############################################################ IDENT(1)
#
# $Title: Makefile to compile Perl XS bindings to cmb(3) $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/perl/Makefile 2018-12-16 19:33:52 -0800 freebsdfrau $
#
############################################################ OBJECTS

XS=	Cmb.xs
PM=	Cmb/lib/Cmb.pm
SO=	Cmb/blib/arch/auto/Cmb/Cmb.so

############################################################ TARGETS

.PHONY: all

all: $(PM) $(SO)

.PHONY: install

install:
	cd Cmb && $(MAKE) install

$(PM) $(SO): Cmb/$(XS) Cmb/Makefile
	cd Cmb && $(MAKE)

Cmb/$(XS): Cmb/.h2xs_done $(XS)
	cp $(XS) $(@)

Cmb/.h2xs_done:
	h2xs -b 5.8.5 cmb.h -lcmb
	rm -f Cmb/$(XS)
	touch $(@)

Cmb/Makefile:
	cd Cmb && perl Makefile.PL

.PHONY: cleandir

cleandir:
	rm -Rf Cmb

.PHONY: realclean

realclean:
	cd Cmb && [ -e Makefile ] && $(MAKE) realclean
	cd Cmb && rm -f $(XS) const-c.inc const-xs.inc

################################################################################
# END
################################################################################
