#!/bin/sh
exec 9<<EOF
import numpy as np

def nump2(n, k):
    a = np.ones((k, n-k+1), dtype=int)
    a[0] = np.arange(n-k+1)
    for j in range(1, k):
        reps = (n-k+j) - a[j-1]
        a = np.repeat(a, reps, axis=1)
        ind = np.add.accumulate(reps)
        a[j, ind[:-1]] = 1-reps[1:]
        a[j, 0] = j
        a[j] = np.add.accumulate(a[j])
    return a

k = 2
n = 10000
N = np.arange(n)
M = nump2(n,k)
for i in range(0, len(M[0])):
    print(M[0][i], M[1][i])
EOF
${PYTHON:-python} -c "$(cat <&9)" | {
	read LINE
	( echo "$LINE"; cat ) | dpv -l 49995000:-
}
