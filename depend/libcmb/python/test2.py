#!/usr/bin/env python
from cmb import *
import signal
import sys

def interrupt(sig, frame):
    print ""
    sys.exit(0)

signal.signal(signal.SIGINT, interrupt)

total = 0
choice = 2
num = 1000

items = range(0, num)
count = len(items)

config = CMB(size_min = choice, size_max = choice)

print "Silently enumerating choose-%u from %u:" % (choice, num)
def afunc(items):
    global total
    total += 1
    return 0
cmb_callback(config, count, items, afunc)
print "%u callbacks executed" % total
