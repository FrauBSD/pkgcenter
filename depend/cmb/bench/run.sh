#!/bin/sh
############################################################ IDENT(1)
#
# $Title: Script to run benchmarks and log results $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cmb/bench/run.sh 2019-07-06 20:41:54 -0700 freebsdfrau $
#
############################################################ CONFIGURATION

#
# Default number of cores to exercise
#
DEFAULT_NCORES=$( getconf _NPROCESSORS_ONLN ||
	getconf NPROCESSORS_ONLN ) 2> /dev/null

#
# Default number of times to run each test
#
DEFAULT_RUNS=15

#
# Tests to perform
#
# NB: %s represents the current hostname
# NB: %u represents the number of cores (-n num)
#
TESTS="
	# Single-Core Test (4.2B ops / 1 core)
	logs/log1.%s.txt = time cmb -oSr 32

	# All-Core Test (4.3B ops x %u cores)
	logs/log2.%s.txt = ./cmb_bench_cpu.sh -an %u 32

	# Spread-Core Test (274B ops / %u cores)
	logs/log3.%s.txt = ./cmb_bench_cpu.sh -n %u 38
" # END-QUOTE

############################################################ ENVIRONMENT

: ${HOSTNAME:=$( hostname )}

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
NCORES=$DEFAULT_NCORES	# -n num
RUNS=$DEFAULT_RUNS	# -r num

############################################################ FUNCTIONS

have(){ type "$@" > /dev/null 2>&1; }

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
	local optfmt="\t%-9s %s\n"
	exec >&2
	printf "Usage: %s [-n num] [-r num]\n" "$pgm"
	printf "Options:\n"
	printf "$optfmt" "-n num" \
		"Exercise num cores. Default $DEFAULT_NCORES."
	printf "$optfmt" "-r num" \
		"Run tests num times for normalization. Default $DEFAULT_RUNS."
	die
}

############################################################ MAIN

#
# Process command-line options
#
while getopts n:hr: flag; do
	case "$flag" in
	n) NCORES="$OPTARG" ;;
	r) RUNS="$OPTARG" ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Check command-line options
#
case "$RUNS" in
""|*[!0-9]*) die "%s: Invalid number argument to \`-n num' option" "$RUNS" ;;
esac
case "$NCORES" in
""|*[!0-9]*) die "%s: Invalid number argument to \`-c num' option" "$NCORES" ;;
esac

#
# Dependency check
#
have cmb || die "You have to install cmb first!"

#
# Perform tests
#
oldIFS="$IFS"
IFS="
" # END-QUOTE
set -- $( echo "$TESTS" | awk -v hostname="$HOSTNAME" -v ncores="$NCORES" '
	BEGIN { gsub(/\..*/, "", hostname) }
	!/^[[:space:]]*(#|$)/ {
		sub(/^[[:space:]]*/, "")
		sub(/[[:space:]]*$/, "")
		if (split($0, a, /[[:space:]]*=[[:space:]]*/) != 2) next
		logfile = a[1] ~ /%s/ ? sprintf(a[1], hostname) : a[1]
		cmdline = a[2] ~ /%u/ ? sprintf(a[2], ncores) : a[2]
		print logfile, cmdline
	}
' )
IFS="$oldIFS"
abort=
for line in "$@"; do
	n=1
	log="${line%%[$IFS]*}"
	cmd="${line#*[$IFS]}"
	case "$log" in
	*/*) mkdir -p "${log%/*}" ;;
	esac
	echo "Command: $cmd" >> "$log"

	#
	# Run test multiple times
	#
	while [ $n -le $RUNS ]; do
		printf "\e[31;1m==>\e[m %s \e[1m# Run %u of %u\e[m\n" \
			"$cmd" $n $RUNS
		if ! $cmd 2>&1 | tee -a $log; then
			abort=1
			echo
			echo Aborted. >&2
			break
		fi
		n=$(( $n + 1 ))
	done

	[ ! "$abort" ] || break
done

exit $SUCCESS

################################################################################
# END
################################################################################
