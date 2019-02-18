# -*- tab-width:  4 -*- ;; Emacs
# vi: set tabstop=4     :: Vi/ViM

############################################### Configuration

PROG_NAME    = util
SRCS         = main.c
             
SLEEP        = sleep
SLEEP_SRCS   = sleep.c
             
SWAPSZ       = swapsz
SWAPSZ_SRCS  = swapsz.c
             
HTT          = htt
HTT_SRCS     = htt.c
             
PHYSMEM      = physmem
PHYSMEM_SRCS = physmem.c

X86_64       = x86_64
X86_64_SRCS  = x86_64.c

############################################### Compiler

CC           = cc

LDFLAGS      = -static
INCLUDE      = -I.
CFLAGS       = -Wall -O -pipe $(INCLUDE)

SHELL        = /bin/sh
RM           = /bin/rm
STRIP        = /usr/bin/strip
DU           = /usr/bin/du

############################################### Rules

noargs: all

all: $(PROG_NAME) $(SLEEP) $(SWAPSZ) $(HTT) $(PHYSMEM) $(X86_64)

$(PROG_NAME): $(SRCS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(>) -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(SLEEP): $(SLEEP_SRCS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(>) -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(SWAPSZ): $(SWAPSZ_SRCS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(>) -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(HTT): $(HTT_SRCS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(>) -o $(@)
	$(STRIP) -s $(@)
	$(DU) -h $(@)

$(PHYSMEM): $(PHYSMEM_SRCS)
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
	$(RM) -f $(PROG_NAME)
	$(RM) -f $(SLEEP)
	$(RM) -f $(SWAPSZ)
	$(RM) -f $(HTT)
	$(RM) -f $(PHYSMEM)
	$(RM) -f $(X86_64)

################################################################################
# END
################################################################################
#
# $Header: /cvsroot/druidbsd/druidbsd/druid/dep/freebsd/util/Makefile,v 1.1 2012/01/28 06:59:47 devinteske Exp $
#
# $Copyright: 2006-2012 Devin Teske. All rights reserved. $
#
# $Log: Makefile,v $
# Revision 1.1  2012/01/28 06:59:47  devinteske
# Commit initial public beta release (beta 56)
#
#
################################################################################
