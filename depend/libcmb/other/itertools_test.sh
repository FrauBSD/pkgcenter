#!/bin/sh
exec 9<<EOF
import itertools
r = ["%s" % x for x in range(10000)]
z = itertools.combinations(r, 2)
for _ in z:
    print(" ".join(_))
EOF
${PYTHON:-python} -c "$( cat <&9 )" | dpv -l 49995000:itertools
