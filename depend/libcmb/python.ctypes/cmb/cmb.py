# Python module
# -*- tab-width:  4 -*- ;; Emacs
# -*- coding: utf-8 -*- ;; PEP 0263 
# vi: set tabstop=4     :: Vi/ViM
# vi: set fileencoding=utf-8 :: PEP 0263
############################################################ IDENT(1)
#
# $Title: Python bindings for libcmb $
# $Copyright: 2018 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/libcmb/python/cmb/cmb.py 2019-01-05 19:30:37 -0800 freebsdfrau $
#
############################################################ LICENSE
#
# Copyright (c) 2018 Devin Teske. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
############################################################ INFORMATION

"""
ctypes interface to libcmb.
"""

############################################################ INCLUDES

import ctypes
import sys

############################################################ GLOBALS

g_action = None;

#
# Library instance
#
libcmb = ctypes.CDLL('libcmb.so.0')

############################################################ CLASSES

class CMB(ctypes.Structure):

    def keys(self):
        keys = []
        for key in (
            "count",
            "debug",
            "delimiter",
            "nul_terminate",
            "prefix",
            "show_empty",
            "show_numbers",
            "size_max",
            "size_min",
            "start",
            "suffix",
        ):
            if getattr(self, key) != None:
                keys.append(key)
        return keys

    def __getitem__(self, key):
        return getattr(self, key)

    def __setitem__(self, key, value):
        if (type(value) is str):
            return setattr(self, key, value.encode('utf-8'))
        else:
            return setattr(self, key, value)


#
# Function pointers
#
CMB_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(CMB),
    ctypes.c_uint64, ctypes.c_uint32, ctypes.POINTER(ctypes.c_char_p))

CMB._fields_ = [
        ("debug", ctypes.c_uint8),
        ("nul_terminate", ctypes.c_uint8),
        ("show_empty", ctypes.c_uint8),
        ("show_numbers", ctypes.c_uint8),
        ("delimiter", ctypes.c_char_p),
        ("prefix", ctypes.c_char_p),
        ("suffix", ctypes.c_char_p),
        ("size_min", ctypes.c_uint32),
        ("size_max", ctypes.c_uint32),
        ("count", ctypes.c_uint64),
        ("start", ctypes.c_uint64),
        ("action", CMB_CALLBACK),
    ]

############################################################ TYPES

#
# Strings
#
libcmb.cmb_version.restype = ctypes.c_char_p

#
# Integers
#
libcmb.cmb_count.restype = ctypes.c_uint64
libcmb.cmb_print.restype = ctypes.c_int
libcmb.cmb.restype = ctypes.c_int

############################################################ FUNCTIONS

#
# Get library version
#
def cmb_version(*args):
    return libcmb.cmb_version(0 if len(args) == 0 else args[0])

#
# Number of combinations for given configuration and number of items
#
def cmb_count(config, nitems):
    return libcmb.cmb_count(ctypes.pointer(config), nitems)

#
# Callback for printing items to stdout
#
def cmb_print(config, seq, nitems, items):
    citems = (ctypes.c_char_p * len(items))()
    if sys.version_info[0] < 3:
        citems[:] = items
    else:
        citems[:] = [bytes(x, 'utf-8') for x in items]
    return libcmb.cmb_print(ctypes.pointer(config), seq, nitems, citems)

#
# Combine options to stdout
#
def cmb(config, nitems, items):
    citems = (ctypes.c_char_p * len(items))()
    if sys.version_info[0] < 3:
        citems[:] = items
    else:
        citems[:] = [bytes(x, 'utf-8') for x in items]
    return libcmb.cmb(ctypes.pointer(config), nitems, citems)

#
# Callback for cmb_callback
#
def g_callback(config, seq, nitems, items):
    pitems = []
    for i in range(0, nitems):
        pitems.append(items[i])
    if (config.contents.show_numbers):
        return g_action(seq, pitems)
    else:
        return g_action(pitems)

#
# Combine options to callbacks
#
def cmb_callback(config, nitems, items, action):
    global g_action
    citems = (ctypes.c_char_p * len(items))()
    if sys.version_info[0] < 3:
        citems[:] = ["%s" % x for x in items]
    else:
        citems[:] = [bytes(y, 'utf-8') for y in ["%s" % x for x in items]]
    _action = config["action"]
    g_action = action
    config["action"] = CMB_CALLBACK(g_callback)
    res = libcmb.cmb(ctypes.pointer(config), nitems, citems)
    config["action"] = _action
    return res

################################################################################
# END
################################################################################
