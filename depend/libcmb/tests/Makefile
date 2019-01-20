############################################################ IDENT(1)
#
# $Title: Makefile to compile cmb(3) tests $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/tests/Makefile 2019-01-19 16:48:37 -0800 freebsdfrau $
#
############################################################ OBJECTS

TESTS=		test1 test2 test3 test4 test5
SRCS=		test1.c test2.c test3.c test4.c test5.c

CFLAGS=		-g -Wall -Werror -I/usr/local/include -O2 -pipe
LDFLAGS=	-L/usr/local/lib -lcmb

############################################################ FUNCTIONS

EVAL2=		eval2(){ echo "$$*"; eval "$$@"; }

############################################################ TARGETS

.PHONY: all

all: $(TESTS)

$(TESTS): $(SRCS)
	@$(EVAL2); \
	 [ $(@) -nt $(@).c ] || eval2 $(CC) $(CFLAGS) $(LDFLAGS) $(@).c -o $(@)

.PHONY: clean

clean:
	rm -Rf *.dSYM

.PHONY: cleandir distclean

cleandir distclean: clean
	rm -f $(TESTS)

################################################################################
# END
################################################################################
