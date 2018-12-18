#!/usr/bin/env python
from cmb import *

config = CMB(
    debug = 0,
    nul_terminate = 0,
    show_empty = 0,
    show_numbers = 1,
    delimiter = ",",
    prefix = "\t[",
    suffix = "]",
    size_min = 2,
    size_max = 3,
    count = 6,
    start = 4,
)
print "config = {"
for key in config.keys():
    print "\t\"%s\": \"%s\"," % (key, config[key])
print "}"

vers = 1;
items = ["a", "b", "c", "d"]
count = len(items)
ilist = ", ".join(items)
seq = 1

print "cmb_version(): %s" % cmb_version()
print "cmb_version(%i): %s" % (vers, cmb_version(vers))
print "size_min=%u size_max=%u" % (config["size_min"], config["size_max"])
print "cmb_count(config, %u) = %u" % (count, cmb_count(config, count))
print "cmb_print(config, %u, %u, [%s]):" % (seq, count, ilist)
res = cmb_print(config, seq, count, items)
print "\tRESULT: %i" % res
print "cmb(config, %u, [%s]):" % (count, ilist)
print "NOTE: { start = %u, count = %u }" % (config["start"], config["count"])
res = cmb(config, count, items)
print "\tRESULT: %i" % res

#
# Callbacks:
#
config["show_numbers"] = 0
print "cmb_callback(%u, [%s], afunc):" % (count, ilist)
num_calls = 0
def afunc():
    global num_calls
    num_calls += 1
    return 0
res = cmb_callback(config, count, items, afunc)
print "\tnum_calls: %i" % num_calls
print "\tRESULT: %i" % res

################################################################################
# END
################################################################################
