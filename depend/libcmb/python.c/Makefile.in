############################################################ IDENT(1)
#
# $Title: Makefile to run user-supplied targets in sub-directories $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/python.c/Makefile 2018-12-31 23:23:46 -0800 freebsdfrau $
#
############################################################ OBJECTS

SUBDIRS=	cmb
TARGETS=	clean cleandir install uninstall

############################################################ TARGETS

.PHONY: all

.PHONY: $(TARGETS)

all $(TARGETS):
	@for dir in $(SUBDIRS); do ( cd $$dir && $(MAKE) $(@) ); done

################################################################################
# END
################################################################################
