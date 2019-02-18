/* -*- tab-width:  4 -*- ;; Emacs  */
/* vi: set tabstop=4     :: Vi/ViM */

/* Devin Teske (c)2010, July 26, 11:42:38. All Rights Reserved. */

/* system includes */
#include <stdio.h>				/* printf(3) */
#include <stdlib.h>				/* EXIT_SUCCESS exit(3) */
#include <string.h>				/* strncmp(3) */
#include <sys/types.h>			/* u_int/register_t (for machine/cpufunc.h) */
#include <machine/cpufunc.h>	/* read_e/rflags() write_e/rflags()
                            	 * do_cpuid() */
#include <machine/psl.h>		/* PSL_ID */
#include <machine/specialreg.h>	/* AMDID_LM */

/* Preprocessor Macros */

#ifndef AMDID_LM
#define AMDID_LM 0x20000000
#endif


int
main (int argc, char *argv[])
{
	int has_lm = 0;
	char *cpu_vendor;
	int vendor[3];
#ifdef __amd64__
	register_t rflags;
#else
	u_int eflags;
#endif
	u_int regs[4];

	/* Check for presence of "cpuid". */
#ifdef __amd64__
	rflags = read_rflags();
	write_rflags(rflags ^ PSL_ID);
	if (((rflags ^ read_rflags()) & PSL_ID) != 0)
#else
	eflags = read_eflags();
	write_eflags(eflags ^ PSL_ID);
	if (((eflags ^ read_eflags()) & PSL_ID) != 0)
#endif
	{
		/* Fetch the vendor string. */
		do_cpuid(0, regs);
		vendor[0] = regs[1];
		vendor[1] = regs[3];
		vendor[2] = regs[2];
		cpu_vendor = (char *)vendor;

		/* check for vendors that support AMD features. */
		if (strncmp(cpu_vendor, "GenuineIntel", 12) == 0 ||
		    strncmp(cpu_vendor, "AuthenticAMD", 12) == 0)
		{
			/* Has to support AMD features. */
			do_cpuid(0x80000000, regs);
			if (regs[0] >= 0x80000001)
			{
				/* Check for long mode. */
				do_cpuid(0x80000001, regs);
				has_lm = (regs[3] & AMDID_LM);
			}
		}
	}

	printf("x86_64 support: %s\n", has_lm ? "YES" : "NO" );
	exit(EXIT_SUCCESS);
}

/*
 * $Header: /cvsroot/druidbsd/druidbsd/druid/dep/freebsd/util/x86_64.c,v 1.1 2012/01/28 06:59:48 devinteske Exp $
 *
 * $Copyright: 2006-2012 Devin Teske. All rights reserved. $
 *
 * $Log: x86_64.c,v $
 * Revision 1.1  2012/01/28 06:59:48  devinteske
 * Commit initial public beta release (beta 56)
 *
 *
 */
