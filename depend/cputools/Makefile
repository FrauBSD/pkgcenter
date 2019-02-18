############################################### IDENT(1)
#
# $Title: Makefile for cputools $
# $Copyright: 2006-2019 Devin Teske. All rights reserved. $
# $FrauBSD: //github.com/FrauBSD/pkgcenter/depend/cputools/Makefile 2019-02-17 20:02:00 -0800 freebsdfrau $
#
############################################### CONFIGURATION

HTT          = htt
HTT_SRCS     = htt.c
             
X86_64       = x86_64
X86_64_SRCS  = x86_64.c

############################################### COMPILER

CC           = cc

LDFLAGS      = -static
INCLUDE      = -I.
CFLAGS       = -Wall -O -pipe $(INCLUDE)

SHELL        = /bin/sh
RM           = /bin/rm
STRIP        = /usr/bin/strip
DU           = /usr/bin/du

############################################### TARGETS

noargs: all

all: $(HTT) $(X86_64)

$(HTT): $(HTT_SRCS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(>) -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(X86_64): $(X86_64_SRCS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(>) -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

clean:
	$(RM) -f *.o
	$(RM) -f *.core

distclean: clean
	$(RM) -f $(HTT)
	$(RM) -f $(X86_64)

################################################################################
# END
################################################################################
