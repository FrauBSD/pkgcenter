############################################################ IDENT(1)
#
# $Title: Makefile to compile cmb(3) tests $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/tests/Makefile 2019-01-05 21:35:36 -0800 freebsdfrau $
#
############################################################ OBJECTS

TESTS=		test1 test2 test3
SRCS=		test1.c test2.c test3.c

CFLAGS=		-g -Wall -Werror -I/usr/local/include -O2 -pipe
LDFLAGS=	-L/usr/local/lib -lcmb -lcrypto

############################################################ FUNCTIONS

EVAL2=		eval2(){ echo "$$*"; eval "$$@"; }

############################################################ TARGETS

.PHONY: all

all: $(TESTS)

$(TESTS): $(SRCS)
	@$(EVAL2); \
	 [ $(@) -nt $(@).c ] || eval2 $(CC) $(CFLAGS) $(LDFLAGS) $(@).c -o $(@)

.PHONY: clean cleandir distclean

clean cleandir distclean:
	rm -f $(TESTS)

################################################################################
# END
################################################################################
