#!/usr/bin/env python
from cmb import *

total = 0
items = ["a", "b", "c"]
nitems = len(items)
config = CMB()

print("Testing non-zero callback return:")
def afunc(items):
    global total
    total += 1
    return total - 1
cmb_callback(config, nitems, items, afunc)
print("%u of %u callbacks executed" % (total, cmb_count(config, nitems)))
