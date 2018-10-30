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
 * $FrauBSD: pkgcenter/depend/cmb/cmb.c 2018-10-30 13:56:39 -0700 freebsdfrau $
 * $FreeBSD$
 */

#include <sys/cdefs.h>
#ifdef __FBSDID
__FBSDID("$FrauBSD: pkgcenter/depend/cmb/cmb.c 2018-10-30 13:56:39 -0700 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <sys/param.h>

#include <cmb.h>
#include <err.h>
#include <errno.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(__FreeBSD_version)
#ifdef HAVE_LIBCRYPTO
#define HAVE_OPENSSL_BN_H 1
#define HAVE_OPENSSL_CRYPTO_H 1
#else
#undef HAVE_OPENSSL_BN_H
#undef HAVE_OPENSSL_CRYPTO_H
#endif
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
#ifdef HAVE_OPENSSL_BN_H
			if (BN_dec2bn(&(config->count_bn), optarg) == 0)
				errx(EXIT_FAILURE, "OpenSSL Error?!");
#else
			config->count = strtoull(optarg, (char **)NULL, 10);
#endif
			break;
		case 'd': /* delimiter */
			config->delimiter = optarg;
			break;
		case 'i': /* start */
#ifdef HAVE_OPENSSL_BN_H
			if (BN_dec2bn(&(config->start_bn), optarg) == 0)
				errx(EXIT_FAILURE, "OpenSSL Error?!");
#else
			config->start = strtoull(optarg, (char **)NULL, 10);
#endif
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

	/* At least one non-option argument is required */
	if (argc == 0) usage(); /* NOTREACHED */

	/*
	 * Calculate combinations
	 */
	if (nitems == 0 || nitems > (uint64_t)argc) nitems = (uint64_t)argc;
	if (opt_total) {
#if defined(HAVE_OPENSSL_BN_H) && defined(HAVE_OPENSSL_CRYPTO_H)
		BIGNUM *count;
		char *count_str;

		if ((count = cmb_count_bn(config, nitems)) != NULL) {
			count_str = BN_bn2dec(count);
			printf("%s\n", count_str);
			OPENSSL_free(count_str);
			BN_free(count);
		} else
			printf("0\n");
#else
		uint64_t count;

		count = cmb_count(config, nitems);
		if (errno)
			err(errno, NULL);
		printf("%"PRIu64"\n", count);
#endif
	} else {
#ifdef HAVE_OPENSSL_BN_H
		retval = cmb_bn(config, nitems, argv);
#else
		retval = cmb(config, nitems, argv);
		if (errno)
			err(errno, NULL);
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
	fprintf(stderr, "usage: %s %s item1 ...\n", pgm,
		"[-0t] [-c #] [-d str] [-i #] [-k range] [-p str] [-s str]");
	exit(EXIT_FAILURE);
}
