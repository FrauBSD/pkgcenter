############################################################ IDENT(1)
#
# $Title: Makefile to install python package $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/python/Makefile 2018-12-20 18:10:18 -0800 freebsdfrau $
#
############################################################ OBJECTS

PACKAGE_NAME=	cmb
PACKAGE_FILES=	__init__.py \
		cmb.py

############################################################ FUNCTIONS

EVAL2=		exec 3<&1; eval2(){ echo "$$*" >&3;eval "$$*"; }

############################################################ TARGETS

.PHONY: all

all:

.PHONY: install uninstall

install:
	@$(EVAL2); \
	 set -e; \
	 : $${PREFIX:=/usr/local}; \
	 eval2 cd $(PACKAGE_NAME); \
	 for pkgdir in $$PREFIX/lib/python*/site-packages; do \
	 	[ -d $$pkgdir ] || continue; \
	 	eval2 mkdir -p $$pkgdir/$(PACKAGE_NAME); \
	 	eval2 cp $(PACKAGE_FILES) $$pkgdir/$(PACKAGE_NAME)/; \
	 done

uninstall:
	@$(EVAL2); \
	 set -e; \
	 : $${PREFIX:=/usr/local}; \
	 for pkgdir in $$PREFIX/lib/python*/site-packages/$(PACKAGE_NAME); do \
	 	[ -d $$pkgdir ] || continue; \
	 	eval2 cd $$pkgdir; \
	 	eval2 rm -f $(PACKAGE_FILES); \
	 	eval2 cd -; \
	 	eval2 rmdir $$pkgdir ||:; \
	 done

.PHONY: distclean clean cleandir

distclean clean cleandir:
	rm -f $(PACKAGE_NAME)/*.pyc *.pyc

################################################################################
# END
################################################################################
