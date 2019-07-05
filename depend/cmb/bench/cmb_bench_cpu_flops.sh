#!/bin/sh
############################################################ IDENT(1)
#
# $Title: Combinatorics based CPU benchmark for floating-point operations $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cmb/bench/cmb_bench_cpu_flops.sh 2019-07-04 23:58:53 -0700 freebsdfrau $
#
############################################################ ENVIRONMENT

# How many threads to use (default = all the cores)
: ${THREADS:=$( getconf _NPROCESSORS_ONLN 2> /dev/null ||
	getconf NPROCESSORS_ONLN 2> /dev/null )}

# Perform n-choose-k with n value of $SET and k value of 2
: ${SET:=1000000}

# Solve on all cores (AC=1) or solve once spread over $THREADS
: ${AC:=}

# Use OpenSSL?
: ${USE_OPENSSL:=}

# Number translations
: ${LANG:=en_US.UTF-8}

############################################################ GLOBALS

pgm="${0##*/}" # Program basename

#
# Global exit status
#
SUCCESS=0
FAILURE=1

#
# Miscellaneous
#
ABORTED=

############################################################ FUNCTIONS

die()
{
	local fmt="$1"
	if [ "$fmt" ]; then
		shift 1 # fmt
		printf "%s: $fmt\n" "$pgm" "$@"
	fi
	exit $FAILURE
}

usage()
{
	local optfmt="\t%-9s %s\n"
	exec >&2
	printf "Usage: %s [-aO] [-n num] [set-num]\n" "$pgm"
	printf "Options:\n"
	printf "$optfmt" "-a" "Solve on all threads instead of spreading."
	printf "$optfmt" "-n num" "Use num threads instead of nproc threads."
	printf "$optfmt" "-O" "Enable/use OpenSSL."
	printf "Notes:\n"
	printf "\tThe default set-num is 1,000,000.\n"
	die
}

interrupt() # Ctrl-C handler
{
	printf "\nAborted.\n"
	exec 2> /dev/null
	kill $pids
	ABORTED=1
}

get_elapsed()
{
	local t="$(( $( date +%s ) - $S ))" # time in seconds since epoch
	min=$(( $t / 60 ))
	sec=$(( $t - 60 * $min ))
	[ ${#sec} -eq 1 ] && sec="0$sec"
	elapsed="${min}m${sec}s"
}

############################################################ MAIN

#
# Process comnand-line options
#
while getopts ahn:O flag; do
	case "$flag" in
	a) AC=1 ;;
	n) THREADS="$OPTARG" ;;
	O) USE_OPENSSL=1 ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Process command-line argument
#
[ $# -gt 0 ] && SET="$1"

#
# Check settings
#
case "$THREADS" in
""|*[!0-9]*) die "Please set THREADS to a number or use \`-n num'" ;;
esac
case "$SET" in
""|*[!0-9]*) die "Please set SET to a number or use \`num' argument" ;;
esac

#
# Gather information about requested benchmark
#
S=$( date +%s ) # Start time in seconds since epoch (Jan 1, 1970 00:00:00 UTC)
T=$( cmb -otrk2 $SET ) # Total number of combinations in n-choose-k given $SET
if [ ! "$AC" ]; then
	C=$(( $T / $THREADS )) # Combinations for each thread
fi

#
# Inform the user of what we are about to do
#
printf "Started: %s\n" "$( date )"
[ ! "$USE_OPENSSL" ] || printf "OpenSSL enabled -- %s\n" "$( cmb -v )"
printf "Spawning %d threads to to solve C(%d,2) => division... " \
	"$THREADS" "$SET"

#
# Spawn threads
#
for n in $( seq 1 $THREADS ); do
	if [ "$USE_OPENSSL" ]; then
		cmd="cmb -Sr -k 2 -X div"
	else
		cmd="cmb -oSr -k 2 -X div"
	fi
	if [ "$AC" ]; then # All-core stress test
		$cmd $SET &
	else
		[ $n -eq $THREADS ] || cmd="$cmd -c $C"
		$cmd -i $(( $C * ($n - 1) + 1 )) $SET &
	fi
	pids="$pids $!"
done
printf "done\n"

#
# Make sure we kill the threads when Ctrl-C is caught
#
trap interrupt SIGINT

#
# Inform the user of what is currently taking place
#
export LANG
printf "Total combinations in C(%d,2) is %'d\n" "$SET" "$T"
if [ "$AC" ]; then # All-core stress test
	printf "Each thread working on %'d combinations\n" "$T"
else
	printf "Each thread working on %'d combinations\n" "$C"
fi

#
# Wait for thread completion, sharing load average during every 10s until done
#
printf "Waiting for threads to complete. Press Ctrl-C to cancel test.\n"
n=20
while :; do
	_pids=
	for pid in $pids; do
		kill -0 $pid 2> /dev/null && _pids="$_pids $pid"
	done
	[ "$_pids" ] || break
	pids="${_pids# }"
	if [ $(( $n % 20 )) -eq 0 ]; then
		get_elapsed
		printf "%s -- load: %'8.2f -- elapsed: %8s\n" "$( date )" \
			"$( uptime | awk '
				BEGIN {
					x = sprintf("%'\''.1f", 1.5)
					gsub(/[[:digit:]]/, "", x)
				}
				($0=$(NF-2)) sub(/,$/,"") {
					gsub(/\./, x)
					print
				}
			' )" \
			"$elapsed"
		n=1
	else
		n=$(( $n + 1 ))
	fi
	sleep 0.5
done

#
# All done
#
[ "$ABORTED" ] || printf "Solved C(%d,2) => division: %s\n" "$SET" "$( date )"
get_elapsed
printf "Elapsed Time: %s\n" "$elapsed"

exit $SUCCESS

################################################################################
# END
################################################################################
