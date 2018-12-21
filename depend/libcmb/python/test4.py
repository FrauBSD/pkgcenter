#!/usr/bin/env python
from cmb import *
import signal
import subprocess
import sys

def interrupt(sig, frame):
    sys.exit(0)

signal.signal(signal.SIGINT, interrupt)

items = range(0, 1000)
choose = 2
nitems = len(items)

config = CMB(size_min = choose, size_max = choose)
total = cmb_count(config, nitems)
dpv = subprocess.Popen(["dpv", "-l", "%u:perl" % total],
    stdin = subprocess.PIPE)

def afunc(items):
    global dpv
    dpv.stdin.write(" ".join(items) + "\n")
    return 0
cmb_callback(config, nitems, items, afunc)
