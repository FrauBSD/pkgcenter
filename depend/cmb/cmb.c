/*-
 * Copyright (c) 2018 Devin Teske <dteske@FreeBSD.org>
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
 *
 * $FrauBSD: depend/cmb/cmb.c 2018-03-27 18:57:52 -0700 freebsdfrau $
 * $FreeBSD$
 */

#include <sys/cdefs.h>
#ifdef __FBSDID
__FBSDID("$FrauBSD: depend/cmb/cmb.c 2018-03-27 18:57:52 -0700 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <sys/param.h>

#include <cmb.h>
#include <err.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __FreeBSD_version
#define HAVE_OPENSSL_BN_H 1
#define HAVE_OPENSSL_CRYPTO_H 1
#elif defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef HAVE_OPENSSL_BN_H
#include <openssl/bn.h>
#endif
#ifdef HAVE_OPENSSL_CRYPTO_H
#include <openssl/crypto.h>
#endif

/* Environment */
static char *pgm; /* set to argv[0] by main() */

/* Function prototypes */
static void	usage(void);

int
main(int argc, char *argv[])
{
	uint8_t opt_total = FALSE;
	char *cp;
	int ch;
	int retval = EXIT_SUCCESS;
	uint64_t nitems = 0;
	size_t config_size = sizeof(struct cmb_config);
	struct cmb_config *config = NULL;

	pgm = argv[0]; /* store a copy of invocation name */

	/* Allocate config structure */
	if ((config = malloc(config_size)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	bzero(config, sizeof(struct cmb_config));

	/*
	 * Process command-line options
	 */
	while ((ch = getopt(argc, argv, "0c:d:i:k:n:p:s:t")) != -1) {
		switch(ch) {
		case '0': /* NUL terminate */
			config->nul_terminate = TRUE;
			break;
		case 'c': /* count */
			config->count = strtoull(optarg, (char **)NULL, 10);
			break;
		case 'd': /* delimiter */
			config->delimiter = optarg;
			break;
		case 'i': /* start */
			config->start = strtoull(optarg, (char **)NULL, 10);
			break;
		case 'k': /* range */
			config->range_min =
			    strtoull(optarg, (char **)NULL, 10);
			if ((cp = strstr(optarg, "..")) != NULL) {
				config->range_max =
				    strtoull(cp + 2, (char **)NULL, 10);
			} else if ((cp = strstr(optarg, "-")) != NULL) {
				config->range_max =
				    strtoull(cp + 1, (char **)NULL, 10);
			} else {
				config->range_max = config->range_min;
			}
			break;
		case 'n': /* args */
			nitems = strtoull(optarg, (char **)NULL, 10);
			break;
		case 'p': /* prefix */
			config->prefix = optarg;
			break;
		case 's': /* suffix */
			config->suffix = optarg;
			break;
		case 't': /* total */
			opt_total = TRUE;
			break;
		default: /* unhandled argument (based on switch) */
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	/*
	 * Calculate combinations
	 */
	if (nitems == 0) nitems = (uint64_t)argc;
	if (opt_total) {
#if defined(HAVE_OPENSSL_BN_H) && defined(HAVE_OPENSSL_CRYPTO_H)
		BIGNUM *count;
		char *count_str;

		if ((count = cmb_count_bn(config, nitems)) != NULL) {
			count_str = BN_bn2dec(count);
			printf("%s\n", count_str);
			OPENSSL_free(count_str);
			BN_free(count);
		}
#else
		printf("%"PRIu64"\n", cmb_count(config, nitems));
#endif
	} else {
#ifdef HAVE_OPENSSL_BN_H
		retval = cmb_bn(config, nitems, argv);
#else
		retval = cmb(config, nitems, argv);
#endif
	}

	return (retval);
}

/*
 * Print short usage statement to stderr and exit with error status.
 */
static void
usage(void)
{
	fprintf(stderr, "Usage: %s [options] item1 item2 ...\n", pgm);
	fprintf(stderr, "OPTIONS:\n");
#define OPTFMT "\t%-11s %s\n"
	fprintf(stderr, OPTFMT, "-0",
	    "Terminate combinations with ASCII NUL instead of newline.");
	fprintf(stderr, OPTFMT, "-c num", "Produce num combinations.");
	fprintf(stderr, OPTFMT, "-d text",
	    "Delimiter for separating items. Default is space.");
	fprintf(stderr, OPTFMT, "-i num",
	    "Set starting position in combination set. Default is 1.");
	fprintf(stderr, OPTFMT, "-k range",
	    "Number or range of numbers for combination set(s).");
	fprintf(stderr, OPTFMT, "-p text",
	    "Prefix each combination with text.");
	fprintf(stderr, OPTFMT, "-s text",
	    "Suffix each combination with text.");
	fprintf(stderr, OPTFMT, "-t",
	    "Print total number of combinations and exit.");
	exit(EXIT_FAILURE);
}
