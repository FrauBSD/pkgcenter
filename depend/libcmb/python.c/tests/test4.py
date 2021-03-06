#!/usr/bin/env python
from cmb import *
import signal
import subprocess
import sys

def interrupt(sig, frame):
    print("")
    sys.exit(0)

signal.signal(signal.SIGINT, interrupt)

items = ["%s" % x for x in range(0, 10000)]
choose = 2
nitems = len(items)

config = CMB(size_min = choose, size_max = choose)
total = cmb_count(config, nitems)
dpv = subprocess.Popen(["dpv", "-l", "%u:cpython" % total],
    stdin = subprocess.PIPE,
    universal_newlines = True)

def afunc(items):
    global dpv
    dpv.stdin.write(" ".join(items) + "\n")
    return 0
cmb_callback(config, nitems, items, afunc)
dpv.stdin.close()
dpv.wait()
