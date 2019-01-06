############################################################ IDENT(1)
#
# $Title: Makefile to compile cmb(3) tests $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/tests/Makefile 2019-01-05 21:10:56 -0800 freebsdfrau $
#
############################################################ OBJECTS

TESTS=		test1
SRCS=		test1.c

CFLAGS=		-g -Wall -Werror -I/usr/local/include $$CFLAGS
LDFLAGS=	-L/usr/local/lib -lcmb -lcrypto

############################################################ FUNCTIONS

EVAL2=		eval2(){ echo "$$*"; eval "$$@"; }

############################################################ TARGETS

.PHONY: all

all: $(TESTS)

$(TESTS): $(SRCS)
	@$(EVAL2);                                                           \
	 set -e;                                                             \
	 [ $(@).o -nt $(@).c ] || eval2 $(CC) $(CFLAGS) -c $(@).c -o $(@).o; \
	 [ $(@) -nt $(@).o ] || eval2 $(CC) $(LDFLAGS) $(@).o -o $(@)

.PHONY: clean

clean:
	rm -f *.o

.PHONY: cleandir distclean

cleandir distclean: clean
	rm -f $(TESTS)

################################################################################
# END
################################################################################
