#!/bin/sh
# -*- tab-width: 4 -*- ;; Emacs
# vi: set noexpandtab  :: Vi/ViM
############################################################ IDENT(1)
#
# $Title: Script to unpack a FreeBSD package $
# $Copyright: 1999-2017 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/freebsd/unpack.sh 2017-07-23 16:16:24 -0700 freebsdfrau $
#
############################################################ INFORMATION
#
# Usage: unpack.sh [OPTIONS] package-archive ...
# OPTIONS:
# 	-a   Move the package-archive to an `archive' sub-directory (next
# 	     to unpacked `stage' directory) after successfully unpacking
# 	     the package. Not performed by default.
# 	-d   Debug. Print lots of debugging info to stderr.
# 	-f   Force. Allow unpacking to a directory that already exists.
# 	-h   Print this message to stderr and exit.
# 	-o   Unpack archive to origin sub-directory. Default is to unpack
# 	     to local directory.
#
############################################################ INCLUDES

progdir="${0%/*}"
. "$progdir/Mk/manifest.subr" || exit

############################################################ GLOBALS

pgm="${0##*/}" # Program basename

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
DEBUG=			# -d
FORCE=			# -f
MOVE_PACKAGE=		# -a
USE_ORIGIN_SUBDIR=	# -o

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
	printf "Usage: %s [OPTIONS] package-archive ...\n" "$pgm"
	printf "OPTIONS:\n"
	printf "$optfmt" "-a" \
		"Move the package-archive to an \`archive' sub-directory (next"
	printf "$optfmt" "" \
		"to unpacked \`stage' directory) after successfully unpacking"
	printf "$optfmt" "" \
		"the package. Not performed by default."
	printf "$optfmt" "-d" \
		"Debug. Print lots of debugging info to stderr."
	printf "$optfmt" "-f" \
		"Force. Allow unpacking to a directory that already exists."
	printf "$optfmt" "-h" \
		"Print this message to stderr and exit."
	printf "$optfmt" "-o" \
		"Unpack archive to origin sub-directory. Default is to unpack"
	printf "$optfmt" "" \
		"to local directory."
	die
}

# read_pkg_origin
# Usage:
# 	read_pkg_origin DATA ...
# 		OR
# 	read_pkg_origin < FILE
# 
# DESCRIPTION:
#   Takes the contents of a packing-list (+CONTENTS) either in the form of data
#   being piped-in, or as one or more arguments provided to this function. When
#   provided as an argument, you must encapsulate the data from each packing-
#   list with double-quotes to preserve the multi-line nature of the data. For
#   example:
# 
# 	Simple:
# 		read_pkg_origin "$( cat ./+CONTENTS )"
# 	- or -
# 		read_pkg_origin "`cat ./+CONTENTS`"
# 
# 	Compound:
# 		data="$( cat ./+CONTENTS )"
# 	- or -
# 		data="`cat ./+CONTENTS`"
# 	- or -
# 		data=$( cat ./+CONTENTS )
# 	- or -
# 		data=`cat ./+CONTENTS`
# 	- then -
# 		read_pkg_origin "$data"
# 
#   While you should avoid the following [unsupported] syntaxes:
# 
# 	To be avoided:
# 		read_pkg_origin $( cat ./+CONTENTS )
# 	- or -
# 		read_pkg_origin `cat ./+CONTENTS`
# 	- or -
# 		read_pkg_origin $data
# 
#   Using double-quote encapsulation, you can optionally query multiple
#   packing-lists with the following example syntax:
# 
# 	Multiple arguments:
# 	    read_pkg_origin "$(cat pkgA/+CONTENTS)" "$(cat pkgB/+CONTENTS)"
# 	- or -
# 	    dataA=$( cat pkgA/+CONTENTS )
# 	    dataB=$( cat pkgB/+CONTENTS )
# 	    read_pkg_origin "$dataA" "$dataB"
# 
#   Finally, you can pipe the packing-list data directly into the function. See
#   below:
# 
# 	Handling PIPE data:
# 		cat ./+CONTENTS | read_pkg_origin
# 	- or -
# 		read_pkg_origin < ./+CONTENTS
#
#   Only the first such ORIGIN field from any/all supplied packing-list data is
#   printed, with the exception of when multiple arguments are provided (when
#   the first ORIGIN field is printed for each argument, as-if each argument
#   was a full-and-complete packing-list from an independent package).
#
read_pkg_origin()
{
	local ORIGIN="@comment ORIGIN:*"

	if [ $# -gt 0 ]; then
		# Parse each argument as a full-and-complete packing-list
		while [ $# -gt 0 ]; do
			echo "$1" | while read LINE; do
				case "$LINE" in $ORIGIN)
					echo "${LINE#*:}" && break
				esac
			done
			shift 1
		done
		return $SUCCESS
	fi

	#
	# Expect PIPE data to be provided from STDIN (otherwise, as
	# expected, the user will be prompted to provide the input on
	# the command-line when `read' is invoked
	#
	while read LINE; do
		case "$LINE" in $ORIGIN)
			echo "${LINE#*:}" && break
		esac
	done
}

# read_pkgng_origin
# Usage:
# 	read_pkgng_origin DATA ...
# 		OR
# 	read_pkgng_origin < FILE
# 
# DESCRIPTION:
#   Takes the contents of a packing-list (+MANIFEST or +COMPACT_MANIFEST)
#   either in the form of data being piped-in, or as one or more arguments
#   provided to this function. When provided as an argument, you must
#   encapsulate the data from each packing-list with double-quotes to preserve
#   the multi-line nature of the data. For example:
# 
# 	Simple:
# 		read_pkgng_origin "$( cat ./+COMPACT_MANIFEST )"
# 	- or -
# 		read_pkgng_origin "`cat ./+COMPACT_MANIFEST`"
# 
# 	Compound:
# 		data="$( cat ./+COMPACT_MANIFEST )"
# 	- or -
# 		data="`cat ./+COMPACT_MANIFEST`"
# 	- or -
# 		data=$( cat ./+COMPACT_MANIFEST )
# 	- or -
# 		data=`cat ./+COMPACT_MANIFEST`
# 	- then -
# 		read_pkgng_origin "$data"
# 
#   While you should avoid the following [unsupported] syntaxes:
# 
# 	To be avoided:
# 		read_pkgng_origin $( cat ./+COMPACT_MANIFEST )
# 	- or -
# 		read_pkgng_origin `cat ./+COMPACT_MANIFEST`
# 	- or -
# 		read_pkgng_origin $data
# 
#   Using double-quote encapsulation, you can optionally query multiple
#   packing-lists with the following example syntax:
# 
# 	Multiple arguments:
# 	    read_pkgng_origin \
# 	    	"$( cat pkgA/+COMPACT_MANIFEST )" \
# 	    	"$( cat pkgB/+COMPACT_MANIFEST )"
# 	- or -
# 	    dataA=$( cat pkgA/+COMPACT_MANIFEST )
# 	    dataB=$( cat pkgB/+COMPACT_MANIFEST )
# 	    read_pkgng_origin "$dataA" "$dataB"
# 
#   Finally, you can pipe the packing-list data directly into the function. See
#   below:
# 
# 	Handling PIPE data:
# 		cat ./+COMPACT_MANIFEST | read_pkgng_origin
# 	- or -
# 		read_pkgng_origin < ./+COMPACT_MANIFEST
#
#   Only the first such "origin" field from any/all supplied packing-list data
#   is printed, with the exception of when multiple arguments are provided
#   (when the first "origin" field is printed for each argument, as-if each
#   argument was a full-and-complete packing-list from an independent package).
#
read_pkgng_origin()
{
	local origin

	if [ $# -gt 0 ]; then
		# Parse each argument as a full-and-complete packing-list
		while [ $# -gt 0 ]; do
			origin=
			manifest_read $DEBUG -r origin -i "$1"
			echo "$origin"
			shift 1
		done
		return $SUCCESS
	fi

	#
	# Expect PIPE data to be provided from STDIN (otherwise, as
	# expected, the user will be prompted to provide the input on
	# the command-line when `awk' is invoked).
	#
	origin=
	manifest_read $DEBUG -r origin
	echo "$origin"
}

############################################################ MAIN

#
# Process command-line options
#
while getopts adfho flag; do
	case "$flag" in
	a) MOVE_PACKAGE=1 ;;
	d) DEBUG="$DEBUG${DEBUG:+ }-d" ;;
	f) FORCE=1 ;;
	o) USE_ORIGIN_SUBDIR=1 ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Validate number of arguments
#
[ $# -gt 0 ] || usage

#
# Loop over each remaining package-archive argument(s)
#
while [ $# -gt 0 ]; do

	package="$1"
	shift 1
	
	printf "===> Unpacking \`%s'\n" "$package"

	case "$package" in
	*gz) ar_opt=z ;;
	*bz) ar_opt=j ;;
	*xz) ar_opt=J ;;
	esac

	CAT=
	case "$ar_opt" in
	J) tar --help 2>&1 | awk '/-J/&&found++;END{exit !found}' ||
		CAT=xzcat ar_opt= ;;
	esac

	#
	# Extract packing-list from package
	#
	printf "Extracting +CONTENTS/+COMPACT_MANIFEST from package\n"
	CONTENTS_FORMAT=pkg_tools
	if [ "$CAT" ]; then
		CONTENTS=$( $CAT "$package" |
			tar ${ar_opt}xfO - +CONTENTS 2> /dev/null )
	else
		CONTENTS=$( tar ${ar_opt}xfO "$package" +CONTENTS \
			2> /dev/null )
	fi
	if [ ! "$CONTENTS" ]; then
		CONTENTS_FORMAT=pkgng
		if [ "$CAT" ]; then
			CONTENTS=$( $CAT "$package" | tar ${ar_opt}xfO - \
				+COMPACT_MANIFEST 2> /dev/null )
		else
			CONTENTS=$( tar ${ar_opt}xfO "$package" \
				+COMPACT_MANIFEST 2> /dev/null )
		fi
	fi
	if [ ! "$CONTENTS" ]; then
		err "ERROR: Unable to extract packing-list from package"
		err "ERROR: Skipping package"
		continue
	fi

	#
	# Get the package_name
	#
	# NOTE: By truncating the ORIGIN field of the packing-list we can
	#       reliably determine the package's base name without having
	#       to resort to attempting to infer its name based on the
	#       package's file (which contains the version string). In
	#       addition, it gives us a more definitive way of storing our
	#       package sources (by ORIGIN, which indicates author and/or
	#       section, rather than by base-name only).
	# NOTE: For pkgng, the ORIGIN is the "origin" field in either +MANIFEST
	#       or +COMPACT_MANIFEST.
	#
	case "$CONTENTS_FORMAT" in
	pkg_tools)
		printf "Reading ORIGIN from packing-list: "
		ORIGIN=$( read_pkg_origin "$CONTENTS" ) ;;
	pkgng)
		printf "Reading origin from packing-list: "
		ORIGIN=$( read_pkgng_origin "$CONTENTS" ) ;;
	*)
		err "ERROR: Unknown packing-list format!"
		err "ERROR: Unable to unpack \`%s' (skipping)" "$package"
		continue
	esac
	if [ ! "$ORIGIN" ]; then
		printf "\n"
		err "ERROR: No ORIGIN recorded for package"
		err "ERROR: Unable to unpack \`%s'" "$package"

		read -p "Please provide a path to unpack to: " ORIGIN
		if [ ! "$ORIGIN" ]; then
			err "ERROR: No path provided (skipping package)"
			continue
		fi
		USE_ORIGIN_SUBDIR=1
	else
		printf "%s\n" "$ORIGIN"
	fi

	#
	# Determine where we will unpack the package to.
	#
	if [ "$USE_ORIGIN_SUBDIR" ]; then
		DEST="$ORIGIN"
	else
		DEST="${ORIGIN##*/}"
	fi

	#
	# Make sure that the directory in which we are going to unpack to
	# doesn't already exist. If it does, issue an error and skip this
	# package (unless `-f' is passed).
	# 
	# Otherwise, create the destination and proceed with unpacking.
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
	# Unpack the package
	#
	printf "Unpacking...\n"
	if ! mkdir -p "$DEST/stage"; then
		err "ERROR: Could not create directory \`%s/stage'" "$DEST"
		die "ERROR: Exiting"
	fi
	if [ "$CAT" ]; then
		$CAT "$package" | tar ${ar_opt}xfp - -C "$DEST/stage"
	else
		tar ${ar_opt}xfp "$package" -C "$DEST/stage"
	fi
	if [ $? -ne $SUCCESS ]; then
		err "ERROR: Could not unpack \`%s' to \`%s/stage'" \
			"$package" "$DEST"
		die "ERROR: Exiting"
	fi
	if ! { [ -e "$DEST/.keep-stage" ] || touch "$DEST/.keep-stage"; }; then
		err "ERROR: Could not create file \`%s/.keep-stage'" "$DEST"
		die "ERROR: Exiting"
	fi

	#
	# Copy the packing-list
	#
	case "$CONTENTS_FORMAT" in
	pkg_tools)
		printf "Generating \`PLIST' from \`+CONTENTS'...\n"
		if ! grep -v '^@comment[[:space:]]\{1,\}MD5:' \
			< "$DEST/stage/+CONTENTS" > "$DEST/PLIST"
		then
			err "ERROR: Could not create \`PLIST'"
			die "ERROR: Exiting"
		fi
		;;
	pkgng)
		printf "Generating \`MANIFEST' from \`+MANIFEST'...\n"
		if ! unpack_manifest < "$DEST/stage/+MANIFEST" \
			> "$DEST/MANIFEST"
		then
			err "ERROR: Could not create \`MANIFEST'"
			die "ERROR: Exiting"
		fi
		;;
	esac

	#
	# Remove compiled packing-list
	#
	case "$CONTENTS_FORMAT" in
	pkg_tools)
		printf "Deleting \`+CONTENTS'...\n"
		rm -f "$DEST/stage/+CONTENTS"
		;;
	pkgng)
		printf "Deleting \`+MANIFEST' and \`+COMPACT_MANIFEST'...\n"
		rm -f "$DEST/stage/+MANIFEST" "$DEST/stage/+COMPACT_MANIFEST"
		;;
	esac

	#
	# Archive package
	#
	if [ "$MOVE_PACKAGE" = "1" ]; then
		echo "Archiving package..."
		if ! mkdir "$DEST/archive"; then
			err "ERROR: Unable to create %s \`%s/archive'" \
				"directory" "$DEST"
			die "ERROR: Exiting"
		fi
		if ! mv "$package" "$DEST/archive"; then
			err "ERROR: Unable to %s \`%s' to \`%s/archive'" \
				"move" "$package" "$DEST"
			die "ERROR: Exiting"
		fi
	fi

	#
	# Extract skeleton directory into package repository
	#
	printf "Copying \`skel' structure into package repository...\n"
	tar co --exclude CVS -f - -C "$progdir/skel" . | tar xkvf - -C "$DEST"
	chmod -R u+w "$DEST"

	#
	# Move in the appropriate Makefile
	#
	printf "Adjusting for archive format...\n"
	case "$CONTENTS_FORMAT" in
	pkg_tools)
		rm -vf "$DEST/MANIFEST"
		rm -vf "$DEST/Makefile.ng"
		case "$ar_opt" in
		z) if [ ! -e "$DEST/Makefile" ] ||
		   	cmp "$DEST/Makefile" "$progdir/skel/Makefile"
		   then
		   	mv -vf "$DEST/Makefile.old" "$DEST/Makefile"
		   else
		   	rm -vf "$DEST/Makefile.old"
		   fi
		   ;;
		j) rm -vf "$DEST/Makefile.old" ;;
		esac
		;;
	pkgng)
		rm -vf "$DEST/PLIST"
		rm -vf "$DEST/Makefile.old"
		if [ ! -e "$DEST/Makefile" ] ||
			cmp "$DEST/Makefile" "$progdir/skel/Makefile"
		then
			mv -vf "$DEST/Makefile" "$DEST/Makefile.old"
			mv -vf "$DEST/Makefile.ng" "$DEST/Makefile"
		else
			rm -vf "$DEST/Makefile.ng"
		fi
		cmp "$DEST/stage/+COMMENT" "$progdir/skel/stage/+COMMENT" \
			2> /dev/null && rm -fv "$DEST/stage/+COMMENT"
		cmp "$DEST/stage/+DESC" "$progdir/skel/stage/+DESC" \
			2> /dev/null && rm -fv "$DEST/stage/+DESC"
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
