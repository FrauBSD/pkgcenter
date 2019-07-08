#!/bin/sh
############################################################ IDENT(1)
#
# $Title: Script to produce results from benchmark file $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cmb/bench/stats.sh 2019-07-08 10:57:24 -0700 freebsdfrau $
#
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
HOSTINFO=	# -H

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
	echo "Usage: $pgm [-H] file ..."
	die
}

############################################################ MAIN

#
# Process command-line options
#
while getopts Hh flag; do
	case "$flag" in
	H) HOSTINFO=1 ;;
	*) usage # NOTREACHED
	esac
done
shift $(( $OPTIND - 1 ))

#
# Get host information
#
if [ "$HOSTINFO" ]; then
	fmt="%-13s %s\n"
	printf "$fmt" "Hostname:" "$HOSTNAME"
	dmidecode | awk -v fmt="$fmt" '
		# Motherboard
		/^Base/, /^$/ {
			sub(/^[[:space:]]*/, "")
			sub(/[[:space:]]*$/, "")
			if (sub(/^Manufacturer:[[:space:]]*/, ""))
				mbase = $0
			else if (sub(/^Product Name:[[:space:]]*/, ""))
				mprod = $0
		}

		# CPU
		/^Processor/, /^$/ {
			sub(/^[[:space:]]*/, "")
			sub(/[[:space:]]*$/, "")
			if (!sub(/^Version:[[:space:]]*/, "")) next
			cpu[$0]++
		}

		# Memory
		/^Memory/, /^$/ {
			sub(/^[[:space:]]*/, "")
			sub(/[[:space:]]*$/, "")
			if (sub(/^Manufacturer:[[:space:]]*/, ""))
				mem_maker = $0
			else if (/^Size:[[:space:]]/)
				mem_size = $2 substr($3,1,1)
			else if (/^Type:[[:space:]]/)
				mem_type = $2
			else if (/^Speed:[[:space:]]/) {
				mem_speed = $2
				if ($3 ~ /MHz/) mem_speed = \
					sprintf("%.3f", mem_speed / 1000)
				mem_speed = mem_speed "GHz"
			} else if (/^$/ && mem_maker != "") {
				mem_desc = sprintf("%s %s %s @ %s",
					mem_maker, mem_size, mem_type,
					mem_speed)
				mem[mem_desc]++
				sizemem[mem_desc] = mem_size
				unitmem[mem_desc] = mem_size
				sub(/[^0-9]+/, "", sizemem[mem_desc])
				sub(/^[0-9]+/, "", unitmem[mem_desc])
				mem_maker = ""
			}
		}

		END {
			if (mbase != "")
				printf fmt, "Base board:", mbase " " mprod
			for (c in cpu) {
				n = cpu[c]
				gsub(/\(R\)/, "", c)
				gsub(/ +CPU +/, " ", c)
				printf fmt, "Processor(s):", c " x " n
			}
			nmemtypes = 0
			for (m in mem) nmemtypes++
			if (nmemtypes == 1) {
				while ((cmd = "free -h") | getline) {
					if (!/Mem:/) continue
					avail = $2
					break
				}
				close(cmd)
			}
			for (m in mem) {
				n = mem[m]
				mem_desc = sprintf("%s x %u (%u%s total",
					m, n, sizemem[m] * n, unitmem[m])
				if (nmemtypes == 1) {
					mem_desc = sprintf("%s; %s avail)",
						mem_desc, avail)
				}
				printf fmt, "Memory:", mem_desc
			}
		}
	' # END-QUOTE
	printf "$fmt" "Kernel:" "$( uname -r )"
	printf "\n"
fi

#
# Read benchmark files
#
for file in "$@"; do
	awk '
		/^Command:/
		$1 ~ /user$/
		/^Elapsed Time:/
	' "$file"
done

exit $SUCCESS

################################################################################
# END
################################################################################
