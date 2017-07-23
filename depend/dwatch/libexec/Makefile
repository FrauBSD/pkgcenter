# $FreeBSD$

FILESDIR=	${LIBEXECDIR}/dwatch
FILES=		chmod \
		fchmod \
		fchmodat \
		kill \
		nanosleep \
		vop_create \
		vop_readdir \
		vop_rename \
		vop_symlink

LINKS= ${LIBEXECDIR}/dwatch/chmod ${LIBEXECDIR}/dwatch/lchmod
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mkdir
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mknod
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_remove
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_rmdir
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_lookup

.include <bsd.prog.mk>
