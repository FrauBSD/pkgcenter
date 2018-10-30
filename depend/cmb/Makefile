# $FreeBSD$

.include <src.opts.mk>

PROG=		cmb

CFLAGS+=	-I${.CURDIR}

LIBADD=		cmb

.if ${MK_OPENSSL} != "no"
CFLAGS+=	-DHAVE_LIBCRYPTO
LIBADD+=	crypto
.endif

.include <bsd.prog.mk>
