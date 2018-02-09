# $FreeBSD$

FILESDIR=	${LIBEXECDIR}/dwatch
FILES=		chmod \
		errno \
		io \
		ip \
		kill \
		nanosleep \
		open \
		proc \
		proc-signal \
		tcp \
		udp \
		vop_create \
		vop_readdir \
		vop_rename \
		vop_symlink

LINKS=	${LIBEXECDIR}/dwatch/chmod ${LIBEXECDIR}/dwatch/fchmodat
LINKS+=	${LIBEXECDIR}/dwatch/chmod ${LIBEXECDIR}/dwatch/lchmod
LINKS+=	${LIBEXECDIR}/dwatch/io ${LIBEXECDIR}/dwatch/io-done
LINKS+=	${LIBEXECDIR}/dwatch/io ${LIBEXECDIR}/dwatch/io-start
LINKS+=	${LIBEXECDIR}/dwatch/ip ${LIBEXECDIR}/dwatch/ip-receive
LINKS+=	${LIBEXECDIR}/dwatch/ip ${LIBEXECDIR}/dwatch/ip-send
LINKS+=	${LIBEXECDIR}/dwatch/open ${LIBEXECDIR}/dwatch/openat
LINKS+=	${LIBEXECDIR}/dwatch/proc ${LIBEXECDIR}/dwatch/proc-create
LINKS+=	${LIBEXECDIR}/dwatch/proc ${LIBEXECDIR}/dwatch/proc-exec
LINKS+=	${LIBEXECDIR}/dwatch/proc ${LIBEXECDIR}/dwatch/proc-exec-failure
LINKS+=	${LIBEXECDIR}/dwatch/proc ${LIBEXECDIR}/dwatch/proc-exec-success
LINKS+=	${LIBEXECDIR}/dwatch/proc ${LIBEXECDIR}/dwatch/proc-exit
LINKS+=	${LIBEXECDIR}/dwatch/proc-signal ${LIBEXECDIR}/dwatch/proc-signal-clear
LINKS+=	${LIBEXECDIR}/dwatch/proc-signal ${LIBEXECDIR}/dwatch/proc-signal-discard
LINKS+=	${LIBEXECDIR}/dwatch/proc-signal ${LIBEXECDIR}/dwatch/proc-signal-send
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/accept-established
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/accept-refused
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/connect-established
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/connect-refused
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/connect-request
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-accept
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-connect
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-established
LINKS+=	${LIBEXECDIR}/dwatch/tcp ${LIBEXECDIR}/dwatch/tcp-refused
LINKS+=	${LIBEXECDIR}/dwatch/udp ${LIBEXECDIR}/dwatch/udp-receive
LINKS+=	${LIBEXECDIR}/dwatch/udp ${LIBEXECDIR}/dwatch/udp-send
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_lookup
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mkdir
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_mknod
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_remove
LINKS+=	${LIBEXECDIR}/dwatch/vop_create ${LIBEXECDIR}/dwatch/vop_rmdir

.include <bsd.prog.mk>
