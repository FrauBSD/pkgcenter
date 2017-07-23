# -*- tab-width: 4 -*- ;; Emacs
# vi: set noexpandtab  :: Vi/ViM
# vi: set syntax=make  ::
################################################################################
############################### GLOBAL VARIABLES ###############################
################################################################################

PKGCENTER = ../..

#
# Repository Specifics
#
DIRS = $$( $(FIND) . -mindepth 4 -maxdepth 4 -name Makefile \
               | $(SED) -e 's:/Makefile$$::' )

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
	@$(CAT) $(PKGCENTER)/redhat/Mk/HELP_REDHAT

usage:
	@$(CAT) $(PKGCENTER)/redhat/Mk/USAGE_REDHAT

#
# Recursive Targets
#

.PHONY: all_redhat depend_redhat forcedepend_redhat
.PHONY: clean_redhat distclean_redhat
.PHONY: pull_redhat commit_redhat
.PHONY: tag_redhat forcetag_redhat taglist_redhat

all_redhat depend_redhat forcedepend_redhat clean_redhat distclean_redhat \
pull_redhat commit_redhat tag_redhat forcetag_redhat taglist_redhat:
	@export CONTINUE=; \
	 ( set -- $(MFLAGS); \
	   while getopts k flag; do [ "$$flag" = "k" ] && exit; done; \
	   false; \
	 ) && export CONTINUE=1; \
	 for dir in $(DIRS); do \
	     target=$(@); target=$${target%_redhat}; \
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
# $FrauBSD: redhat/rhel6-x86_64/Makefile 2017-07-23 16:16:24 -0700 freebsdfrau $
#
################################################################################
