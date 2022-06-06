/*-
 * Copyright (c) 2022 Devin Teske <dteske@FreeBSD.org>
 * Copyright (c) 2022 Faraz Vahedi <kfv@kfv.io>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define parse(item) do { \
	if ((item = malloc(strlen(optarg)+1)) == NULL)           \
		err(EXIT_FAILURE, #item);                        \
	if (snprintf(item, strlen(optarg)+1, "%s", optarg) < 0)  \
		return (-1);                                     \
} while (0)

/* Debugging */
static bool debug = false;

/* Extra information */
static char *pgm; /* set to argv[0] by main() */

/* Function prototypes */
static void usage(void);

int
main(int argc, char *argv[])
{
	int ch;
	char *directive = NULL;
	char *value = NULL;
	char rpath[PATH_MAX] = {0};

	pgm = argv[0]; /* store a copy of invocation name */

	/*
	 * Process command-line options
	 */
	while ((ch = getopt(argc, argv,
	    "Dc:d:v:h")) != -1) {
		switch(ch) {
		case 'D': /* debugging */
			debug = true;
			break;
		case 'c': /* resolve configuration file path */
			if (realpath(optarg, rpath) == 0)
				err(EXIT_FAILURE, "%s", rpath);
			break;
		case 'd': /* set directive */
			parse(directive);
			break;
		case 'h': /* help/usage */
			usage();
			break; /* NOTREACHED */
		case 'v': /* set value */
			parse(value);
			break;
		default:
			usage();
			break; /* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	/* Sanity checks */
	if (!strlen(rpath))
		errx(EXIT_FAILURE, "Configuration file is not provided");

	free(directive);
	free(value);

	exit(EXIT_SUCCESS);
}

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >  201710L && __has_c_attribute(noreturn)
[[noreturn]]
#endif
#if __STDC_VERSION__ >= 201112L
_Noreturn
#endif
#endif
static void
usage(void)
{

	fprintf(stderr, "Usage: ...\n"); /* XXX: Incomplete */
	exit(EXIT_FAILURE);
}
