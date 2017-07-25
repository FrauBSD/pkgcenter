#!/bin/sh
# -*- tab-width: 4 -*- ;; Emacs
# vi: set noexpandtab  :: Vi/ViM
############################################################ IDENT(1)
#
# $Title: Script to create a new package $
# $Copyright: 1999-2017 Devin Teske. All rights reserved. $
# $FrauBSD: freebsd/create.sh 2017-07-25 13:28:33 -0700 freebsdfrau $
#
############################################################ INFORMATION
#
# Usage: create.sh [OPTIONS] package-dir ...
# OPTIONS:
# 	-h   Print this message to stderr and exit.
# 	-f   Force. Allow creation over a directory that already exists.
#
############################################################ GLOBALS

pgm="${0##*/}" # Program basename
progdir="${0%/*}" # Program directory

#
# Global exit status
#
SUCCESS=0
FAILURE=1

#
# OS Glue
#
: ${UNAME_s:=$( uname -s )}

#
# Command-line options
#
FORCE=		# -f

############################################################ FUNCTIONS

# err FORMAT [ARGUMENT ...]
#
# Print a message to stderr.
#
err()
{
	local fmt="$1"
	shift 1 # fmt
	[ "$fmt" ] || return $SUCCESS
	printf "$fmt\n" "$@" >&2
}

# die [FORMAT [ARGUMENT ...]]
#
# Optionally print a message to stderr before exiting with failure status.
#
die()
{
	err "$@"
	exit $FAILURE
}

# usage
#
# Prints a short syntax statement and exits.
#
usage()
{
	local optfmt="\t%-4s %s\n"
	exec >&2
	printf "Usage: %s [OPTIONS] package-dir ...\n" "$pgm"
	printf "OPTIONS:\n"
	printf "$optfmt" "-h" \
		"Print this message to stderr and exit."
	printf "$optfmt" "-f" \
		"Force. Allow creation over a directory that already exists."
	die
}

if ! type realpath > /dev/null 2>&1; then
case "$UNAME_s" in
Darwin)
realpath()
{
	perl -le 'use Cwd; print Cwd::abs_path(@ARGV)' -- "$@"
}
;;
*)
realpath()
{
	readlink -f "$@"
}
esac
fi

############################################################ MAIN

#
# Process command-line options
#
while getopts fh flag; do
	case "$flag" in
	f) FORCE=1 ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Validate number of arguments
#
[ $# -gt 0 ] || usage

#
# Loop over each remaining package-dir argument(s)
#
while [ $# -gt 0 ]; do

	DEST="$1"
	shift 1

	#
	# Get the package name and proper Makefile from pathname
	#
	NAME="${DEST##*/}"
	if [ "$DEST" -a ! "$NAME" ]; then
		DEST="${DEST%/*}"
		NAME="${DEST##*/}"
	fi
	if [ "$NAME" ]; then
		printf "===> Creating \`%s'\n" "$DEST"
	else
		usage # NOTREACHED
	fi

	#
	# Detect older package creation (used later in Makefile fixup)
	#
	ar_opt=J
	case "$( realpath "$DEST" )" in
	*/RELENG_[1-4][_/]*) ar_opt=z ;;
	*/RELENG_[5-9][_/]*) ar_opt=j ;;
	esac

	#
	# Make sure that the directory we are going to create doesn't already
	# exist. If it does, issue an error and skip this package (unless `-f'
	# is passed).
	# 
	# Otherwise, create the destination.
	#
	printf "Creating package repository directory: "
	if [ -e "$DEST" ]; then
		printf "\n"
		err "ERROR: Directory \`%s' already exists" "$DEST"
		if [ ! "$FORCE" ]; then
			err "ERROR: Skipping package (use \`-f' to override)"
			continue
		else
			err "ERROR: Proceeding anyway (\`-f' was passed)"
		fi
	fi
	if ! mkdir -p "$DEST"; then
		printf "\n"
		err "ERROR: Could not create directory \`%s'" "$DEST"
		die "ERROR: Exiting"
	fi
	printf "%s\n" "$DEST"

	#
	# Create the `stage' directory within the package repository
	#
	printf "Creating package \`stage' directory...\n"
	if ! mkdir -p "$DEST/stage"; then
		err "ERROR: Could not create directory \`%s/stage'" "$DEST"
		die "ERROR: Exiting"
	fi

	#
	# Extract skeleton directory into package repository
	#
	printf "Copying \`skel' structure into package repository...\n"
	tar co --exclude CVS -f - -C "$progdir/skel" . | tar xkvf - -C "$DEST"

	#
	# Makefile fixup: move in the appropriate Makefile
	#
	printf "Adjusting for archive format...\n"
	case "$ar_opt" in
	z) if [ ! -e "$DEST/Makefile" ] || \
	      cmp "$DEST/Makefile" "$progdir/skel/Makefile"; then
	   	mv -vf "$DEST/Makefile.old" "$DEST/Makefile"
	   else
	   	rm -vf "$DEST/Makefile.old"
	   fi
	   rm -vf "$DEST/Makefile.ng" "$DEST/MANIFEST"
	   ;;
	j) rm -vf "$DEST/Makefile.old" "$DEST/Makefile.ng" "$DEST/MANIFEST" ;;
	J) rm -vf "$DEST/Makefile.old"
	   if [ ! -e "$DEST/Makefile" ] ||
	   	cmp "$DEST/Makefile" "$progdir/skel/Makefile"
	   then
	   	mv -vf "$DEST/Makefile.ng" "$DEST/Makefile"
	   else
	   	rm -vf "$DEST/Makefile.ng"
	   fi
	   rm -vf "$DEST/PLIST" "$DEST/stage/+COMMENT" "$DEST/stage/+DESC"
	   ;;
	esac

	#
	# That's it (onto the next, back at the top).
	#
	printf "Done.\n"
done

################################################################################
# END
################################################################################
