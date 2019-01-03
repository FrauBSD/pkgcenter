import sys
if sys.version_info[0] < 3:
    from cmb import *
else:
    from cmb.cmb import *
