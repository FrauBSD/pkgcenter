############################################### IDENT(1)
#
# $Title: Makefile for cputools $
# $Copyright: 2006-2019 Devin Teske. All rights reserved. $
# $FrauBSD: pkgcenter/depend/cputools/Makefile 2019-02-19 11:43:31 -0800 freebsdfrau $
#
############################################### CONFIGURATION

EPT          = ept
HTT          = htt
VTX          = vtx
X86_64       = x86_64

############################################### COMPILER

CC           = cc

LDFLAGS      = -static
INCLUDE      = -I.
CFLAGS       = -Wall -O -pipe $(INCLUDE)

RM           = /bin/rm
STRIP        = /usr/bin/strip
DU           = /usr/bin/du

############################################### TARGETS

noargs: all

all: $(EPT) $(HTT) $(VTX) $(X86_64)

$(EPT): $(EPT).c
	$(CC) -c $(CFLAGS) $(@).c -o $(@).o
	$(CC) $(LDFLAGS) $(@).o -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(HTT): $(HTT).c
	$(CC) -c $(CFLAGS) $(@).c -o $(@).o
	$(CC) $(LDFLAGS) $(@).o -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(VTX): $(VTX).c
	$(CC) -c $(CFLAGS) $(@).c -o $(@).o
	$(CC) $(LDFLAGS) $(@).o -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(X86_64): $(X86_64).c
	$(CC) -c $(CFLAGS) $(@).c -o $(@).o
	$(CC) $(LDFLAGS) $(@).o -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

clean:
	$(RM) -f *.o
	$(RM) -f *.core

distclean cleandir: clean
	$(RM) -f $(EPT)
	$(RM) -f $(HTT)
	$(RM) -f $(VTX)
	$(RM) -f $(X86_64)

################################################################################
# END
################################################################################
