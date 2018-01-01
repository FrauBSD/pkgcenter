# $FreeBSD$

FILESDIR=	${LIBEXECDIR}/dwatch
FILES=		chmod \
		errno \
		kill \
		nanosleep \
		open \
		tcp \
		vop_create \
		vop_readdir \
		vop_rename \
		vop_symlink

LINKS=	${LIBEXECDIR}/dwatch/chmod ${LIBEXECDIR}/dwatch/fchmodat
LINKS+=	${LIBEXECDIR}/dwatch/chmod ${LIBEXECDIR}/dwatch/lchmod
LINKS+=	${LIBEXECDIR}/dwatch/open ${LIBEXECDIR}/dwatch/openat
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/accept-established
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/accept-refused
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/connect-established
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/connect-refused
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/connect-request
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-accept
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-connect
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-established
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-refused
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_lookup
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mkdir
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mknod
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_remove
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_rmdir

.include <bsd.prog.mk>
