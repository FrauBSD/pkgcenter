#!/bin/sh
############################################################ IDENT(1)
#
# $Title: Script to produce results from benchmark file $
# $Copyright: 2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cmb/bench/stats.sh 2019-07-08 22:09:19 -0700 freebsdfrau $
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
	filename="${file##*/}" # file basename
	case "$file" in
	*/*) sfile="${file%/*}/stats${filename#log}" ;;
	*) sfile="stats${filename#log}"
	esac

	printf "\e[31;1m==>\e[39m %s\e[m\n" "$sfile"
	awk '
	################################################## FUNCTIONS

	function _asort(src, dest,        k, nitems, i, val)
	{
		k = nitems = 0
		for (i in src) dest[++nitems] = src[i]
		for (i = 1; i <= nitems; k = i++) {
			val = dest[i]
			while ((k > 0) && (dest[k] > val)) {
				dest[k+1] = dest[k]; k--
			}
			dest[k+1] = val
		}
		return nitems
	}

	function avg(n, nums,        i, a)
	{
		a = 0
		for (i = 1; i <= n; i++) a += nums[i]
		return a / n
	}

	function stddev(mean, n, nums,        i, cmd, ans)
	{
		cmd = ""
		ans = "(0"
		for (i = 1; i <= n; i++) {
			cmd = cmd sprintf(";n%u=%s", i, nums[i])
			cmd = cmd sprintf(";x%u=(n%u-%s)^2", i, i, mean)
			ans = ans sprintf("+x%u", i)
		}
		cmd = sprintf("echo \"scale=3%s;sqrt(%s)/%u)\" | bc",
			cmd, ans, n) | getline ans
		close(cmd)
		return ans
	}

	function st(seconds,        min, sec)
	{
		min = int(seconds / 60)
		sec = sprintf("%.3f", seconds - min * 60)
		if (sec ~ /\./) sub(/\.?0+$/, "", sec)
		if (min == 0)
			return sprintf("%ss", sec)
		else
			return sprintf("%um%ss", min, sec)
	}

	################################################## MAIN

	sub(/^Command:[[:space:]]*/, "") { cmds[++ncmds] = $0; next }

	sub(/elapsed$/, "", $3) {
		sec = 0
		n = split($3, a, /:/)
		if (n == 3) sec = sprintf("%.4f",
			a[1] * 3600 + a[2] * 60 + a[3])
		else if (n == 2) sec = sprintf("%.4f",
			a[1] * 60 + a[2])
		if (!sec) next
		cmd = cmds[n = ncmds]
		run[n,++nruns[cmd]] = sec
	}

	/^Elapsed Time:/ {
		sec = 0
		n = split($3, a, /[ms]/)
		if (n == 3) sec = a[1] * 60 + a[2]
		if (!sec) next
		cmd = cmds[n = ncmds]
		run[n,++nruns[cmd]] = sec
	}

	################################################## END

	END {
		for (n = 1; n <= ncmds; n++) {
			print cmd = cmds[n]

			#
			# Eliminate outliers
			runs = nruns[cmd]
			delete xrun
			for (i = 1; i <= runs; i++) {
				xrun[i] = run[n,i]
			}
			delete sxrun
			_asort(xrun, sxrun)
			delete outliers
			r = runs
			l = 1
			while (r > 10) {
				if (sxrun[l+1] - sxrun[l] >= \
				    sxrun[r] - sxrun[r-1]) {
					outliers[sxrun[l]]++
					delete sxrun[l++]
				} else {
					outliers[sxrun[r]]++
					delete sxrun[r]
				}
				r--
			}
			delete results
			nresults = 0
			for (i = 1; i <= runs; i++) {
				arun = xrun[i]
				if (arun in outliers) {
					outliers[arun]--
					if (outliers[arun] == 0)
						delete outliers[arun]
					continue
				}
				results[++nresults] = xrun[i]
			}

			#
			# Print results
			#
			min = results[1]
			max = -1
			for (i = 1; i <= nresults; i++) {
				if (results[i] < min) min = results[i]
				if (results[i] > max) max = results[i]
				printf "%2u: %s", i, st(results[i])
				if (i % 4 == 0) printf "\n"
				else printf "\t"
			}
			printf "\n"
			printf "min: %s", st(min)
			printf "\tmax: %s", st(max)
			printf "\tavg: %s", st(mean = avg(nresults, results))
			printf "\tstddev: %s",
				st(stddev(mean, nresults, results))
			printf "\n"
			printf "\n"
		}
	}
	' "$file" | tee "$sfile"
done

exit $SUCCESS

################################################################################
# END
################################################################################
