############################################################ IDENT(1)
#
# $Title: Makefile to compile Perl XS bindings to cmb(3) $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/perl/Makefile 2018-12-15 22:03:50 -0800 freebsdfrau $
#
############################################################ OBJECTS

XS=	Cmb/Cmb.xs
PM=	Cmb/lib/Cmb.pm
SO=	Cmb/blib/arch/auto/Cmb/Cmb.so

############################################################ TARGETS

.PHONY: all

all: $(PM) $(SO)

.PHONY: install

install:
	cd Cmb && make install

$(PM) $(SO): $(XS) Cmb/Makefile
	cd Cmb && make

$(XS):
	h2xs -b 5.8.5 cmb.h -lcmb
	cp Cmb.xs Cmb/

Cmb/Makefile:
	cd Cmb && perl Makefile.PL

.PHONY: cleandir

cleandir:
	rm -Rf Cmb

################################################################################
# END
################################################################################
