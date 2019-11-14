#!/bin/sh
# $FreeBSD: head/usr.sbin/bsdconfig/examples/add_some_packages.sh 268999 2014-07-22 23:10:12Z dteske $
#
# This sample installs a short list of packages from the main HTTP site.
#
[ "$_SCRIPT_SUBR" ] || . /usr/share/bsdconfig/script.subr || exit 1
nonInteractive=1
_httpPath=http://pkg.freebsd.org
mediaSetHTTP
mediaOpen
for package in wget bash rsync; do
	packageAdd
done
