# $FreeBSD$

FILESDIR=	${LIBEXECDIR}/dwatch
FILES=		chmod \
		errno \
		kill \
		nanosleep \
		open \
		vop_create \
		vop_readdir \
		vop_rename \
		vop_symlink

LINKS=	${LIBEXECDIR}/dwatch/chmod ${LIBEXECDIR}/dwatch/fchmodat
LINKS+=	${LIBEXECDIR}/dwatch/chmod ${LIBEXECDIR}/dwatch/lchmod
LINKS+=	${LIBEXECDIR}/dwatch/open ${LIBEXECDIR}/dwatch/openat
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_lookup
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mkdir
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mknod
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_remove
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_rmdir

.include <bsd.prog.mk>
