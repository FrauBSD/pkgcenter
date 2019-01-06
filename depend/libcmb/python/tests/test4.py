#!/usr/bin/env python
from cmb import *
import signal
import subprocess
import sys

def interrupt(sig, frame):
    print("")
    sys.exit(0)

signal.signal(signal.SIGINT, interrupt)

items = ["%s" % x for x in range(0, 1000)]
choose = 2
nitems = len(items)

config = CMB(size_min = choose, size_max = choose)
total = cmb_count(config, nitems)
dpv = subprocess.Popen(["dpv", "-l", "%u:python" % total],
    stdin = subprocess.PIPE)

def afunc(items):
    global dpv
    out = " ".join([x.decode('utf-8') for x in items]) + "\n"
    dpv.stdin.write(out.encode('utf-8'))
    return 0
cmb_callback(config, nitems, items, afunc)
