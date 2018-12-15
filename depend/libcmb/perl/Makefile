############################################################ IDENT(1)
#
# $Title: Makefile to compile Perl XS bindings to cmb(3) $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/perl/Makefile 2018-12-15 06:15:46 -0800 freebsdfrau $
#
############################################################ OBJECTS

XS=	Cmb/Cmb.xs
PM=	Cmb/lib/Cmb.pm
SO=	Cmb/blib/arch/auto/Cmb/Cmb.so

############################################################ TARGETS

all: $(PM) $(SO)

$(PM) $(SO): $(XS) Cmb/Makefile
	cd Cmb && make

$(XS):
	h2xs -b 5.8.5 cmb.h -lcmb
	cp Cmb.xs Cmb/

install:
	cd Cmb && make install

Cmb/Makefile:
	cd Cmb && perl Makefile.PL

cleandir:
	rm -Rf Cmb

################################################################################
# END
################################################################################
