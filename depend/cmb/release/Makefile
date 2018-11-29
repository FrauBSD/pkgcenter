############################################################ IDENT(1)
#
# $Title: Makefile to produce release tarballs $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cmb/release/Makefile 2018-11-29 02:20:57 -0800 freebsdfrau $
#
############################################################ OBJECTS

# Set to prevent removing tarball directories
NO_CLEAN=

# Where to get $Version$ from
SRC=		../cmb.c

# Default configure args
DEFAULT_ARGS=

############################################################ FUNCTIONS

QUIETLY=	quietly(){ eval "$$*" > /dev/null 2>&1; }

EVAL2=		exec 3<&1; eval2(){ echo "$$*" >&3;eval "$$*"; }

STATUSES=	ok(){ printf "[ \033[32mOK\033[m ] %s\n" "$$*"; }; \
		fail(){ printf "[\033[31mFAIL\033[m] %s\n" "$$*"; exit 1; }

EXIT_TRAPS=	etrap(){ ET="$$* $${ET:+&& $$ET}"; trap "$$ET" EXIT; }; \
		parte(){ ET=; trap - EXIT; }; parte

############################################################ TARGETS

.PHONY: all

all: cmb

.PHONY: cmb

cmb:
	@$(QUIETLY); \
	 $(EVAL2); \
	 $(STATUSES); \
	 $(EXIT_TRAPS); \
	 quietly type cmb || fail cmb not installed; \
	 ok cmb installed; \
	 make=$(MAKE); \
	 args="$(DEFAULT_ARGS)"; \
	 case "$${UNAME_s:=$$( uname -s )}" in \
	 FreeBSD) \
	 	quietly type gmake || fail gmake not installed; \
	 	ok gmake installed; \
	 	make=gmake; \
	 	;; \
	 esac; \
	 version=$$( awk 'match($$0, /\$$Version: [^$$]* \$$/) { \
	 	print substr($$0, RSTART + 10, RLENGTH - 12); \
	 	exit found=1; \
	 } END { exit !found }' "$(SRC)" 2> /dev/null ) || \
	 	fail Version not found in $(SRC); \
	 ok Version $$version found in $(SRC); \
	 cd ..; \
	 [ ! -e cmb ] || fail build directory not clean; \
	 ok build directory clean; \
	 cmb -e D:--enable-debug o:--without-openssl | while read line; do \
	 	variant=; \
	 	args="$(DEFAULT_ARGS)"; \
	 	set -- $$line; \
	 	for arg; do \
	 		variant="$$variant$${arg%%:*}"; \
	 		args="$$args $${arg#*:}"; \
	 	done; \
	 	args="$${args# }"; \
	 	printf "\033[36m>>> Configure args: %s <<<\033[m\n" "$$args"; \
	 	eval2 ./configure $$args; \
	 	etrap eval2 $$make cleandir; \
	 	eval2 $$make; \
	 	cmbdir=release/cmb-$$version$${variant:+-$$variant}; \
	 	eval2 mkdir -p $$cmbdir; \
	 	[ "$(NO_CLEAN)" ] || etrap eval2 rm -Rf $$cmbdir; \
	 	eval2 cp cmb cmb.1.gz $$cmbdir/; \
	 	eval2 tar czfo $$cmbdir.tgz $$cmbdir; \
	 	parte; \
	 	[ "$(NO_CLEAN)" ] || eval2 rm -Rf $$cmbdir; \
	 	eval2 $$make cleandir; \
	 done

.PHONY: clean distclean

clean:
	rm -f cmb-*/cmb cmb-*/cmb.1.gz
	@$(QUIETLY); \
	 $(EVAL2); \
	 $(STATUSES); \
	 quietly type cmb || fail cmb not installed; \
	 version=$$( awk 'match($$0, /\$$Version: [^$$]* \$$/) { \
	 	print substr($$0, RSTART + 10, RLENGTH - 12); \
	 	exit found=1; \
	 } END { exit !found }' "$(SRC)" 2> /dev/null ) || \
	 	fail Version not found in $(SRC); \
	 cmb -ed "" D o | while read variant; do \
	 	cmbdir=cmb-$$version$${variant:+-$$variant}; \
	 	eval2 "[ ! -e $$cmbdir ] || rmdir $$cmbdir"; \
	 done

cleandir: clean
	rm -f cmb-*.tgz

################################################################################
# END
################################################################################
