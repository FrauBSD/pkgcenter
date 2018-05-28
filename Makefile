# -*- tab-width: 4 -*- ;; Emacs
# vi: set noexpandtab  :: Vi/ViM
# vi: set syntax=make  ::
################################################################################
############################### GLOBAL VARIABLES ###############################
################################################################################

PKGCENTER = .

#
# Repository Specifics
#
DIRS = $$( ( $(FIND) freebsd -mindepth 4 -maxdepth 4 -name Makefile; \
             $(FIND) redhat  -mindepth 5 -maxdepth 5 -name Makefile; \
           ) | $(SED) -e 's:/Makefile$$::' )

#
# Standard utility pathnames
#
CAT  = cat
FIND = find
GIT  = git
SED  = sed

################################################################################
################################ BUILD  TARGETS ################################
################################################################################

#
# Generic Targets
#

.PHONY: all help targets usage

all help targets:
	@$(CAT) $(PKGCENTER)/Mk/HELP

usage:
	@$(CAT) $(PKGCENTER)/Mk/USAGE

#
# Recursive Targets
#

.PHONY: all_all depend_all forcedepend_all clean_all distclean_all
.PHONY: pull_all commit_all tag_all forcetag_all

all_all depend_all forcedepend_all clean_all distclean_all \
pull_all commit_all tag_all forcetag_all:
	@export CONTINUE=; \
	 ( set -- $(MFLAGS); \
	   while getopts k flag; do [ "$$flag" = "k" ] && exit; done; \
	   false; \
	 ) && export CONTINUE=1; \
	 for dir in $(DIRS); do \
	     target=$(@); target=$${target%_all}; \
	     echo "--> Making $$target in $$dir"; \
	     ( cd "$$dir" && \
	       $(MAKE) $${MAKELEVEL:+--no-print-directory} $(MFLAGS) $$target \
	     ) || [ "$$CONTINUE" ] || exit 1; \
	 done

#
# Git Addition Targets
#

.PHONY: import commit pull

import:
	@$(PKGCENTER)/Mk/git_import

commit:
	@$(GIT) commit

pull:
	@$(GIT) pull

################################################################################
# END
################################################################################
#
# $Copyright: 1999-2017 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/Makefile 2017-07-23 16:16:24 -0700 freebsdfrau $
#
################################################################################
