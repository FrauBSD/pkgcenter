############################################################ IDENT(1)
#
# $Title: Makefile to compile/install python extension $
# $Copyright: 2018-2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/python.c/cmb/Makefile 2019-01-02 23:48:51 -0800 freebsdfrau $
#
############################################################ OBJECTS

PYTHON=		python3.6
PYTHONINC=	$(PYTHON)m
PYTHONLIB=	$(PYTHON)m

#PYTHON=		python2.7
#PYTHONINC=	$(PYTHON)
#PYTHONLIB=	$(PYTHON)

PACKAGE_NAME=	cmb
PACKAGE_FILES=	__init__.py \
		$(LIB)

PREFIX=		/usr/local
LIB=		cmb.so
SRC=		cmb.c
OBJ=		cmb.o
INC=		-I$(PREFIX)/include -I$(PREFIX)/include/$(PYTHONINC)

LDFLAGS=	-lpthread -L$(PREFIX)/lib -fstack-protector -lcmb -l$(PYTHONLIB)
CFLAGS=		-g -Wall -Wno-missing-braces -fno-strict-aliasing -O2 -pipe -fstack-protector -fno-strict-aliasing -DNDEBUG

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
	@$(EVAL2);                                      \
	 set -e;                                        \
	 : $${PREFIX:=/usr/local};                      \
	 pkgdir="$$PREFIX/lib/$(PYTHON)/site-packages"; \
	 [ -d $$pkgdir ] || exit 0;                     \
	 eval2 mkdir -p $$pkgdir/$(PACKAGE_NAME);       \
	 eval2 cp $(PACKAGE_FILES) $$pkgdir/$(PACKAGE_NAME)/

uninstall:
	@$(EVAL2);                                                      \
	 set -e;                                                        \
	 : $${PREFIX:=/usr/local};                                      \
	 pkgdir="$$PREFIX/lib/$(PYTHON)/site-packages/$(PACKAGE_NAME)"; \
	 [ -d $$pkgdir ] || exit 0;                                     \
	 eval2 cd $$pkgdir;                                             \
	 eval2 rm -f $(PACKAGE_FILES);                                  \
	 eval2 cd -;                                                    \
	 eval2 rmdir $$pkgdir ||:

.PHONY: distclean clean cleandir

clean:
	rm -f *.o *.pyc
	rm -Rf __pycache__

distclean cleandir: clean
	rm -f $(LIB)

################################################################################
# END
################################################################################
