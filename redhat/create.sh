#!/bin/sh
# -*- tab-width: 4 -*- ;; Emacs
# vi: set noexpandtab  :: Vi/ViM
############################################################ IDENT(1)
#
# $Title: Script to create a new RPM $
# $Copyright: 1999-2017 Devin Teske. All rights reserved. $
# $FrauBSD: redhat/create.sh 2017-11-15 13:21:18 -0800 freebsdfrau $
#
############################################################ INFORMATION
#
# Usage: create.sh [OPTIONS] package-dir ...
# OPTIONS:
# 	-h   Print this message to stderr and exit.
# 	-f   Force. Allow creation over a directory that already exists.
#
############################################################ CONFIGURATION

#
# Default `License' value for new SPECFILE
#
LICENSE="BSD"

#
# Default `Version' value for new SPECFILE
#
VERSION="1.0"

#
# Default `Release' value for new SPECFILE
#
RELEASE="1"

#
# Default `Buildarch' value for new SPECFILE
#
ARCH="noarch"

#
# Default `URL' value for new SPECFILE
#
URL="https://www.fraubsd.org/"

#
# Default `Packager' value for new SPECFILE
#
PACKAGER="First Last <flast@fraubsd.org>"

#
# Default `Vendor' value for new SPECFILE
#
VENDOR="The FrauBSD Project https://www.fraubsd.org/"

#
# Default `Changelog' information for new SPECFILE
#
CHANGELOGTIME_day=$( date "+%a %b %m %Y" )
CHANGELOGNAME="First Last <flast@fraubsd.org> 1.0-1"
CHANGELOGTEXT="- Package created."

############################################################ GLOBALS

pgm="${0##*/}" # Program basename
progdir="${0%/*}" # Program directory

#
# Global exit status
#
SUCCESS=0
FAILURE=1

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
}' < "$progdir/Mk/template.spec" )

#
# Initialize querytag values for generating new/clean template.spec file(s).
#
for tag in $querytags; do
	case "$tag" in
	*:*) varname="${tag%:*}_${tag#*:}";;
	*) varname="$tag";;
	esac
	case "$varname" in
	LICENSE|VERSION|RELEASE|ARCH|URL|PACKAGER|VENDOR) 
		: see configuration section for default values
		export $varname;;
	CHANGELOGTIME_day|CHANGELOGNAME|CHANGELOGTEXT)
		: see configuration section for default values
		export $varname;;
	*)
		export $varname=
	esac
done

#
# Loop over each remaining package-dir argument(s)
#
while [ $# -gt 0 ]; do

	DEST="$1"
	shift 1

	#
	# Get the package name and group from pathname
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
	case "$DEST" in
	*/*)
		GROUP=$( awk -v path="${DEST%/*}" '
			BEGIN { plen = length(path) }
			{
				line = $0
				gsub(/[[:space:]]+/, "_", line)
				llen = length(line)
				if (plen < llen) next

				if (plen == llen)
					text2match = "/" path
				else
					text2match = substr(path,
						(plen + 1) - (llen + 1))

				if ("/" line == text2match)
				{
					print $0
					exit
				}
			}
		' "$progdir/Mk/GROUPS" )
		: ${GROUP:=${DEST%/*}}
		;;
	*)
		GROUP=
	esac
	printf "Package NAME: %s\n" "$NAME"
	printf "Package GROUP: %s\n" "$GROUP"

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
	# Declare initial dependencies and file-list to be null
	#
	export REQUIRES= FILE_LISTING=

	#
	# Declare initial summary/description to be the same as the name
	#
	export NAME GROUP
	SUMMARY="$NAME"
	DESCRIPTION="$NAME"
	export SUMMARY DESCRIPTION

	#
	# Generate the spec-file
	#
	printf "Generating \`%s' from \`%s'...\n" \
	       "$DEST/SPECFILE" "$progdir/Mk/template.spec"
	if ! awk -v regex="[[:alnum:]]+(:[[:alnum:]]+)?" '
	{
		if ($0 ~ /^__REQUIRES__$/)
		{
			printf "%s\n", ENVIRON["REQUIRES"]
			next
		}
		if ($0 ~ /^__FILE_LISTING__$/)
		{
			printf "%s\n", ENVIRON["FILE_LISTING"]
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
	# That's it (onto the next, back at the top).
	#
	printf "Done.\n"
done

################################################################################
# END
################################################################################
