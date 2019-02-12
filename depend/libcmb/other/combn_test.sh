#!/bin/sh
exec 9<<EOF
x <- combn(seq(1:10000),2)
for (i in 1:ncol(x)) cat(x[,i],"\n")
EOF
${R:-Rscript} - <&9 | {
	read LINE
	( echo "$LINE"; cat ) | dpv -l 49995000:combn
}
