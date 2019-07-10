#!/bin/sh
############################################################ IDENT(1)
#
# $Title: Script to compare results from benchmark stats file $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cmb/bench/cmp.sh 2019-07-09 21:06:57 -0700 freebsdfrau $
#
############################################################ CONFIGURATION

DEFAULT_LOGDIR=logs

############################################################ GLOBALS

pgm="${0##*/}" # Program basename

#
# Global exit status
#
SUCCESS=0
FAILURE=1

#
# Command-line options
#
LOGDIR="$DEFAULT_LOGDIR"	# -d dir

############################################################ FUNCTIONS

die()
{
	local fmt="$1"
	if [ "$fmt" ]; then
		shift 1 # fmt
		printf "%s: $fmt\n" "$pgm" "$@" >&2
	fi
	exit $FAILURE
}

usage()
{
	exec >&2
	echo "Usage: $pgm [-d logdir] hostA hostB"
	die
}

############################################################ MAIN

#
# Process command-line options
#
while getopts d:h flag; do
	case "$flag" in
	d) LOGDIR="$OPTARG" ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Check command-line options
#
[ -e "$LOGDIR" ] || die "%s: No such file or directory" "$LOGDIR"
[ -d "$LOGDIR" ] || die "%s: Not a directory" "$LOGDIR"

#
# Read benchmark files
#
for file1 in "$LOGDIR"/stats*.$1.txt; do
	if [ ! -e "$file1" ]; then
		printf "\e[33;1mWARNING!\e[m %s: No such file or directory\n" \
			"$file1"
		continue
	fi

	# Check for file with similar name
	file2="${file1%.$1.txt}.$2.txt"
	if [ ! -e "$file2" ]; then
		printf "\e[33;1mWARNING!\e[m %s: No such file or directory\n" \
			"$file2"
		continue
	fi
	awk -v file1="$file1" -v file2="$file2" '
	################################################## FUNCTIONS

	function ts(str,        n, a, sec)
	{
		n = split(str, a, /[ms]/)
		if (n == 2) sec = a[1]
		else if (n == 3) sec = a[1] * 60 + a[2]
		else sec = 0
		if (sec ~ /\./) sub(/\.?0+$/, "", sec)
		return sec
	}

	################################################## MAIN

	BEGIN {
		stderr = "/dev/stderr"

		# Get information about primary
		getline < file1
		if (/^Base/) {
			while (getline < file1 && $1 ~ /:$/) { }
			cmd1 = $0
		} else  cmd1 = $0
		while (getline < file1) {
			if (!sub(/.*avg: /, "")) continue
			avg1 = $1
			if (!sub(/.*stddev: /, "")) continue
			stddev1 = $1
		}
		close(file1)

		# Get information about baseline to compare against
		getline < file2
		if (/^Base/) {
			while (getline < file2 && $1 ~ /:$/) { }
			cmd2 = $0
		} else  cmd2 = $0
		while (getline < file2) {
			if (!sub(/.*avg: /, "")) continue
			avg2 = $1
			if (!sub(/.*stddev: /, "")) continue
			stddev2 = $1
		}
		close(file2)

		# Make sure we are comparing the same tests
		if (cmd1 != cmd2) {
			printf "\033[33;1mWARNING!\033[m %s\n",
				"Command mismatch" > stderr
			printf "\033[2mfile1=[%s] cmd1=[%s]\033[m\n",
				file1, cmd1 > stderr
			printf "\033[2mfile2=[%s] cmd2=[%s]\033[m\n",
				file2, cmd2 > stderr
			printf "\n" > stderr
			fflush(stderr)
		}

		# Calculate comparison statistics
		upavg = sprintf("%.3f", 100 - ts(avg1) * 100 / ts(avg2))
		if (upavg > 0) {
			speed = "faster"
		} else {
			speed = "slower"
			upavg = sprintf("%.3f", upavg * -1)
		}
		if (upavg ~ /\./) sub(/\.?0+$/, "", upavg)
		updev = sprintf("%.3f", 100 - ts(stddev1) * 100 / ts(stddev2))
		if (updev > 0) {
			runtime = "tighter"
		} else {
			runtime = "looser"
			updev = sprintf("%.3f", updev * -1)
		}
		if (updev ~ /\./) sub(/\.?0+$/, "", updev)

		# Print comparison statistics
		compared2 = file2
		sub(/\.txt$/, "", compared2)
		sub(/.*\./, "", compared2)
		print cmd1
		fmt = "\033[35;1mvs %s:\033[m "
		if (speed == "faster")
			fmt = fmt "\033[32;1;4m%s%% %s\033[m" # grn bld und
		else
			fmt = fmt "\033[31;1;4m%s%% %s\033[m" # red bld und
		fmt = fmt " with "
		if (runtime == "tighter")
			fmt = fmt "\033[32m%s%% %s runtime\033[m" # grn
		else
			fmt = fmt "\033[31m%s%% %s runtime\033[m" # red
		fmt = fmt "\n\n"
		printf fmt, compared2, upavg, speed, updev, runtime
	}
	' /dev/null # Pedantic
done

exit $SUCCESS

################################################################################
# END
################################################################################
