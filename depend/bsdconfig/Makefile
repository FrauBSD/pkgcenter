# $FreeBSD: head/usr.sbin/bsdconfig/Makefile 335306 2018-06-17 21:09:21Z dteske $

.include <src.opts.mk>

SUBDIR=	console \
	diskmgmt \
	docsinstall \
	dot \
	include \
	includes \
	mouse \
	networking \
	packages \
	password \
	security \
	share \
	startup \
	timezone \
	ttys \
	usermgmt

.if ${MK_EXAMPLES} != "no"
SUBDIR+= examples
.endif

FILESDIR=	${LIBEXECDIR}/bsdconfig
FILES=		USAGE

#? DO NOT MERGE THESE (3) LINES
PREFIX?=	/usr
SCRIPTSDIR?=	${PREFIX}/sbin

SCRIPTS= bsdconfig

MAN= bsdconfig.8

.include <bsd.prog.mk>
