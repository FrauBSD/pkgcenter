#!/usr/bin/env python
from __future__ import print_function
from cmb import *
from sys import stderr

items = ["a", "b", "c", "d"]
choose = 2
nitems = len(items)

print("Enumerating choose-%s from %u:" % (choose, nitems), file = stderr)
config = CMB(size_min = choose, size_max = choose)
def afunc(items):
    print("\t%s" % (" ".join([x.decode('utf-8') for x in items])))
    return 0
cmb_callback(config, nitems, items, afunc)
