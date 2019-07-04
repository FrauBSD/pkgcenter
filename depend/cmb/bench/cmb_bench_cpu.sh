#!/bin/sh
############################################################ IDENT(1)
#
# $Title: Combinatorics based CPU benchmark $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cmb/bench/cmb_bench_cpu.sh 2019-07-04 10:26:52 -0700 freebsdfrau $
#
############################################################ ENVIRONMENT

# How many threads to use (default = all the cores)
: ${THREADS:=$( getconf _NPROCESSORS_ONLN )}

# Perform n-choose-k with n value of $SET and k range of 1..$SET
: ${SET:=38}

# Solve on all cores (AC=1) or solve once spread over $THREADS
: ${AC:=}

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

usage()
{
	local optfmt="\t%-9s %s\n"
	exec >&2
	printf "Usage: %s [-a] [-n num] [set-num]\n" "$pgm"
	printf "Options:\n"
	printf "$optfmt" "-a" "Solve on all threads instead of spreading."
	printf "$optfmt" "-n num" "Use num threads instead of nproc threads."
	printf "Notes:\n"
	printf "\tThe default set-num is 38.\n"
	exit $FAILURE
}

interrupt() # Ctrl-C handler
{
	printf "\nAborted.\n"
	exec 2> /dev/null
	kill $pids
	ABORTED=1
}

############################################################ MAIN

#
# Process comnand-line options
#
while getopts an: flag; do
	case "$flag" in
	a) AC=1 ;;
	n) THREADS="$OPTARG" ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Process command-line argument
#
[ $# -gt 0 ] && SET="$1"

#
# Gather information about requested benchmark
#
S=$( date +%s ) # Start time in seconds since epoch (Jan 1, 1970 00:00:00 UTC)
T=$( cmb -otr $SET ) # Total number of combinations in n-choose-k given $SET
if [ ! "$AC" ]; then
	C=$(( $T / $THREADS )) # Combinations for each thread
fi

#
# Inform the user of what we are about to do
#
printf "Started: %s\n" "$( date )"
printf "Spawning $THREADS threads to to solve C(%d,S)... " "$SET"

#
# Spawn threads
#
for n in $( seq 1 $THREADS ); do
	cmd="cmb -oSr"
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
printf "Total combinations in C(%d,S) is %'d\n" "$SET" "$T"
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
	running=
	for pid in $pids; do
		kill -0 $pid 2> /dev/null && running=1 && break
	done
	[ "$running" ] || break
	if [ $(( $n % 20 )) -eq 0 ]; then
		printf "%s -- load: %.2f\n" "$( date )" \
			"$( uptime | awk '($0=$(NF-2)) sub(/,$/,"")' )"
		n=1
	else
		n=$(( $n + 1 ))
	fi
	sleep 0.5
done

#
# All done
#
E=$(( $( date +%s ) - $S )) # End time in seconds since epoch
[ "$ABORTED" ] || printf "Solved C(%d,S): %s\n" "$SET" "$( date )"
min=$(( $E / 60 ))
sec=$(( $E - 60 * $min ))
printf "Elapsed Time: %dm%ds\n" "$min" "$sec"

exit $SUCCESS

################################################################################
# END
################################################################################
