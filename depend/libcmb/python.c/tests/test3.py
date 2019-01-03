#!/usr/bin/env python
from cmb import *
from sys import stderr

items = ["a", "b", "c", "d"]
choose = 2
nitems = len(items)

sys.stderr.write("Enumerating choose-%s from %u:\n" % (choose, nitems))
config = CMB(size_min = choose, size_max = choose)
def afunc(items):
    print("\t%s" % " ".join(items))
    return 0
cmb_callback(config, nitems, items, afunc)
