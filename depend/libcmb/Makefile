# $FreeBSD$

.include <src.opts.mk>

PACKAGE=lib${LIB}
LIB=		cmb
SHLIB_MAJOR=	0
INCS=		cmb.h
MAN=		cmb.3

SRCS=		cmb.c

CFLAGS+=	-I${.CURDIR}

LIBADD+=	m

.if ${MK_OPENSSL} != "no"
CFLAGS+=	-DHAVE_LIBCRYPTO
LIBADD+=	crypto
.endif

.include <bsd.lib.mk>
