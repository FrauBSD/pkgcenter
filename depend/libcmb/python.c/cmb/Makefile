############################################################ IDENT(1)
#
# $Title: Makefile to compile/install python extension $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/python.c/cmb/Makefile 2018-12-31 23:23:46 -0800 freebsdfrau $
#
############################################################ OBJECTS

PACKAGE_NAME=	cmb
PACKAGE_FILES=	__init__.py \
		$(LIB)

PREFIX=		/usr/local
LIB=		cmb.so
SRC=		cmb.c
OBJ=		cmb.o
INC=		-I$(PREFIX)/include -I$(PREFIX)/include/python2.7

LDFLAGS=	-lpthread -L$(PREFIX)/lib -fstack-protector -L$(PREFIX)/lib -lcmb
CFLAGS=		-g -Wall -fno-strict-aliasing -O2 -pipe -fstack-protector -fno-strict-aliasing -DNDEBUG

############################################################ FUNCTIONS

EVAL2=		exec 3<&1; eval2(){ echo "$$*" >&3;eval "$$*"; }

############################################################ TARGETS

.PHONY: all

all: $(LIB)

$(LIB): Makefile $(OBJ)
	$(CC) $(LDFLAGS) -shared $(OBJ) -o $(@)

$(OBJ): Makefile $(SRC)
	$(CC) $(CFLAGS) -fPIC $(INC) -c $(SRC) -o $(OBJ)

.PHONY: install uninstall

install:
	@$(EVAL2);                                                   \
	 set -e;                                                     \
	 : $${PREFIX:=/usr/local};                                   \
	 for pkgdir in $$PREFIX/lib/python*/site-packages; do        \
	 	[ -d $$pkgdir ] || continue;                         \
	 	eval2 mkdir -p $$pkgdir/$(PACKAGE_NAME);             \
	 	eval2 cp $(PACKAGE_FILES) $$pkgdir/$(PACKAGE_NAME)/; \
	 done

uninstall:
	@$(EVAL2);                                                            \
	 set -e;                                                              \
	 : $${PREFIX:=/usr/local};                                            \
	 for pkgdir in $$PREFIX/lib/python*/site-packages/$(PACKAGE_NAME); do \
	 	[ -d $$pkgdir ] || continue;                                  \
	 	eval2 cd $$pkgdir;                                            \
	 	eval2 rm -f $(PACKAGE_FILES);                                 \
	 	eval2 cd -;                                                   \
	 	eval2 rmdir $$pkgdir ||:;                                     \
	 done

.PHONY: distclean clean cleandir

clean:
	rm -f *.o *.pyc

distclean cleandir: clean
	rm -f $(LIB)

################################################################################
# END
################################################################################
