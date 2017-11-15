#!/bin/sh
# -*- tab-width: 4 -*- ;; Emacs
# vi: set noexpandtab  :: Vi/ViM
############################################################ IDENT(1)
#
# $Title: Script to unpack a Linux RPM package $
# $Copyright: 1999-2017 Devin Teske. All rights reserved. $
# $FrauBSD: redhat/unpack.sh 2017-11-15 13:32:49 -0800 freebsdfrau $
#
############################################################ INFORMATION
#
# Usage: unpack.sh [OPTIONS] package-archive ...
# OPTIONS:
# 	-a   Move the package-archive to an `archive' sub-directory (next
# 	     to unpacked `stage' directory) after successfully unpacking
# 	     the package. Not performed by default.
# 	-f   Force. Allow unpacking to a directory that already exists.
# 	-g   Unpack archive to group sub-directory. Default is to unpack
# 	     to local directory.
# 	-h   Print this message to stderr and exit.
#
############################################################ INCLUDES

progdir="${0%/*}"

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
FORCE=			# -f
MOVE_PACKAGE=		# -a
USE_GROUP_SUBDIR=	# -g

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
	printf "$optfmt" "-f" \
		"Force. Allow unpacking to a directory that already exists."
	printf "$optfmt" "-g" \
		"Unpack archive to group sub-directory. Default is to unpack"
	printf "$optfmt" "" \
		"to local directory."
	printf "$optfmt" "-h" \
		"Print this message to stderr and exit."
	die
}

# rpm_get_group RPM_ARCHIVE_FILE
#
# Gets the group from an RPM archive.
#
rpm_get_group()
{
	local file="$1"

	#
	# Validate number of arguments
	#
	[ $# -gt 0 -a "$file" ] || return $SUCCESS

	rpm -q -p "$file" --qf "%{GROUP}\n" | sed -e 's/[[:space:]]/_/g'
}

# rpm_get_name RPM_ARCHIVE_FILE
#
# Gets the name from an RPM archive.
#
rpm_get_name()
{
	local file="$1"

	#
	# Validate number of arguments
	#
	[ $# -gt 0 -a "$file" ] || return $SUCCESS

	rpm -q -p "$file" --qf "%{NAME}\n"
}

############################################################ MAIN

#
# Process command-line options
#
while getopts afgh flag; do
	case "$flag" in
	a) MOVE_PACKAGE=1 ;;
	f) FORCE=1 ;;
	g) USE_GROUP_SUBDIR=1 ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Validate number of arguments
#
[ $# -gt 0 ] || usage

#
# Build a list of querytags found in the template.spec file.
#
querytags=$( awk -v regex="@[[:alnum:]]+(:[[:alnum:]]+)?@" '
{
	while (match($0, regex))
	{
		tag = substr($0, RSTART + 1, RLENGTH - 2)
		print tag
		sub(regex, "")
	}
}' < $progdir/Mk/template.spec )

#
# Loop over each remaining package-archive argument(s)
#
while [ $# -gt 0 ]; do

	package="$1"
	shift 1
	
	printf "===> Unpacking \`%s'\n" "$package"

	#
	# Get the package group and name
	#
	printf "Reading NAME from package: "
	NAME=$( rpm_get_name "$package" ) || die
	if [ ! "$NAME" ]; then
		err "ERROR: No NAME recorded for package (skipping package)"
		continue
	else
		printf "%s\n" "$NAME"
	fi
	printf "Reading GROUP from package: "
	GROUP=$( rpm_get_group "$package" )
	if [ ! "$GROUP" ]; then
		printf "\n"
		err "ERROR: No GROUP recorded for package"
		err "ERROR: Unable to unpack \`%s'" "$package"

		read -p "Please provide a path to unpack to: " GROUP
		if [ ! "$GROUP" ]; then
			err "ERROR: No path provided (skipping package)"
			continue
		fi
		USE_GROUP_SUBDIR=1
	else
		printf "%s\n" "$GROUP"
	fi

	#
	# Determine where we will unpack the package to.
	#
	if [ "$USE_GROUP_SUBDIR" ]; then
		DEST="$GROUP/$NAME"
	else
		DEST="$NAME"
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
	no_absolute_filenames=
	[ "$UNAME_s" = Linux ] && no_absolute_filenames=--no-absolute-filenames
	if ! rpm2cpio "$package" | ( cd "$DEST/stage" &&
		cpio -idmu $no_absolute_filenames --no-preserve-owner )
	then
		err "ERROR: Could not unpack \`%s' to \`%s/stage'" \
			"$package" "$DEST"
		die "ERROR: Exiting"
	fi
	if ! { [ -e "$DEST/.keep-stage" ] || touch "$DEST/.keep-stage"; }; then
		err "ERROR: Could not create file \`%s/.keep-stage'" "$DEST"
		die "ERROR: Exiting"
	fi

	#
	# Obtain querytag values from current package
	#
	for tag in $querytags; do
		case "$tag" in
		*:*) varname="${tag%:*}_${tag#*:}" ;;
		*) varname=$tag
		esac
		eval $varname='$( rpm -q -p "$package" --qf "%{'$tag'}" )'
		export $varname

		#
		# Nullify unused fields with a value of "(none)"
		#
		value=$( eval echo "\"\$$varname\"" )
		[ "$value" = "(none)" ] && eval $varname=
	done

	#
	# Generate the spec-file
	#
	printf "Generating \`%s' from \`%s'...\n" \
		"$DEST/SPECFILE" "$progdir/Mk/template.spec"
	REQUIRES=$( rpm -qRp "$package" |
		awk '!/^(\/|[[:alpha:]]+\()/{print "Requires:", $0}' )
	export REQUIRES
	if ! awk -v dest="$DEST" -v regex="[[:alnum:]]+(:[[:alnum:]]+)?" '
	BEGIN {
		find_cmd = sprintf("find \"%s/src\" ! -type d", dest)
		path_strip = length(dest) + 5
		while (find_cmd | getline path)
		{
			path = substr(path, path_strip)
			paths = paths path "\n"
		}
	}
	{
		if ( $0 ~ /^__REQUIRES__$/ )
		{
			printf "%s\n", ENVIRON["REQUIRES"]
			next
		}
		if ( $0 ~ /^__FILE_LISTING__$/ )
		{
			printf paths
			next
		}

		n = split($0, line_buffer, /@/)
		if (n >= 3)
		{
			s = 1
			printf "%s", line_buffer[s]
			while (s++ < n)
			{
				if (line_buffer[s] ~ regex)
				{
					varname = line_buffer[s]
					sub(/:/, "_", varname)
					printf "%s", ENVIRON[varname]
					s++
					printf "%s", line_buffer[s]
				}
				else printf "%s", line_buffer[s]
			}
			printf "\n"
		}
		else print
	}
	' < "$progdir/Mk/template.spec" > "$DEST/SPECFILE"; then
		err "ERROR: Could not create \`$DEST/SPECFILE'"
		die "ERROR: Exiting"
	fi

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
	# That's it (onto the next, back at the top).
	#
	printf "Done.\n"
done

################################################################################
# END
################################################################################
