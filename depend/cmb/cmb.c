/*-
 * Copyright (c) 2018 Devin Teske <dteske@FreeBSD.org>
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
#ifdef __FBSDID
__FBSDID("$FrauBSD: pkgcenter/depend/cmb/cmb.c 2018-11-05 17:21:58 -0800 freebsdfrau $");
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
#else
#include <sys/time.h>
#endif
#ifdef HAVE_OPENSSL_CRYPTO_H
#include <openssl/crypto.h>
#endif

/* Environment */
static char *pgm; /* set to argv[0] by main() */

/* Function prototypes */
static void	usage(void);
#ifndef HAVE_OPENSSL_BN_H
static uint64_t	rand_range(uint64_t range);
#endif

/* Inline functions */
#ifndef HAVE_OPENSSL_BN_H
static inline uint8_t	p2(uint64_t x) { return (x == (x & -x)); }
static inline uint64_t	urand64(void) { return
    (((uint64_t)lrand48() << 42) + ((uint64_t)lrand48() << 21) + lrand48());
}
#endif

int
main(int argc, char *argv[])
{
	uint8_t opt_empty = FALSE;
	uint8_t opt_randi = FALSE;
	uint8_t opt_total = FALSE;
	char *cp;
	int ch;
	int retval = EXIT_SUCCESS;
	uint64_t nitems = 0;
	size_t config_size = sizeof(struct cmb_config);
	size_t optlen;
	struct cmb_config *config = NULL;
#if defined(HAVE_OPENSSL_BN_H) && defined(HAVE_OPENSSL_CRYPTO_H)
	BIGNUM *count;
#else
	uint64_t count;
	uint64_t nstart = 0; /* negative start */
	struct timeval tv;
#endif

	pgm = argv[0]; /* store a copy of invocation name */

	/* Allocate config structure */
	if ((config = malloc(config_size)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	bzero(config, sizeof(struct cmb_config));

	/*
	 * Process command-line options
	 */
	while ((ch = getopt(argc, argv, "0c:d:ei:k:n:p:s:t")) != -1) {
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
		case 'e': /* empty */
			opt_empty = TRUE;
			config->show_empty = opt_empty;
			break;
		case 'i': /* start */
			if ((optlen = strlen(optarg)) > 0 &&
			    strncmp("random", optarg, optlen) == 0) {
				opt_randi = TRUE;
			}
#ifdef HAVE_OPENSSL_BN_H
			if (!opt_randi &&
			    BN_dec2bn(&(config->start_bn), optarg) == 0)
				errx(EXIT_FAILURE, "OpenSSL Error?!");
#else
			if (!opt_randi) {
				config->start =
				    strtoull(optarg, (char **)NULL, 10);
				if (*optarg == '-' && config->start > 0) {
					nstart =
					    strtoll(optarg, (char **)NULL, 10);
				}
			}
#endif
			break;
		case 'k': /* size */
			config->size_min = strtoull(optarg, (char **)NULL, 10);
			if ((cp = strstr(optarg, "..")) != NULL) {
				config->size_max =
				    strtoull(cp + 2, (char **)NULL, 10);
			} else if ((cp = strstr(optarg, "-")) != NULL) {
				config->size_max =
				    strtoull(cp + 1, (char **)NULL, 10);
			} else {
				config->size_max = config->size_min;
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
	if (argc == 0 && !opt_empty)
		usage(); /* NOTREACHED */

	/*
	 * Calculate combinations
	 */
	if (nitems == 0 || nitems > (uint64_t)argc)
		nitems = (uint64_t)argc;
	if (opt_total) {
#if defined(HAVE_OPENSSL_BN_H) && defined(HAVE_OPENSSL_CRYPTO_H)
		char *count_str;

		if ((count = cmb_count_bn(config, nitems)) != NULL) {
			count_str = BN_bn2dec(count);
			printf("%s\n", count_str);
			OPENSSL_free(count_str);
			BN_free(count);
		} else
			printf("0\n");
#else
		count = cmb_count(config, nitems);
		if (errno)
			err(errno, NULL);
		printf("%"PRIu64"\n", count);
#endif
	} else {
#ifdef HAVE_OPENSSL_BN_H
		if (opt_randi) {
			if ((count = cmb_count_bn(config, nitems)) != NULL) {
				if (config->start_bn == NULL)
					config->start_bn = BN_new();
				if (BN_rand_range(config->start_bn, count))
					BN_add_word(config->start_bn, 1);
			}
		} else if (config->start_bn != NULL &&
		    BN_is_negative(config->start_bn)) {
			if ((count = cmb_count_bn(config, nitems)) != NULL) {
				BN_add(config->start_bn,
				    count, config->start_bn);
				BN_add_word(config->start_bn, 1);
				if (BN_is_negative(config->start_bn))
					BN_zero(config->start_bn);
				BN_free(count);
			}
		}
		retval = cmb_bn(config, nitems, argv);
#else
		if (opt_randi) {
			count = cmb_count(config, nitems);
			if (errno)
				err(errno, NULL);
			if (gettimeofday(&tv,NULL) == 0) {
				srand48((long)tv.tv_usec);
				config->start = rand_range(count) + 1;
			}
		} else if (nstart != 0) {
			count = cmb_count(config, nitems);
			if (errno)
				err(errno, NULL);
			if (count >= (nstart * -1))
				config->start = count + nstart + 1;
			else
				config->start = 0;
		}
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
	fprintf(stderr, "usage: %s [options] item1 item2 ...\n", pgm);
#define OPTFMT "\t%-10s %s\n"
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, OPTFMT, "-0",
	    "NUL terminate combinations (use with `xargs -0').");
	fprintf(stderr, OPTFMT, "-c num",
	    "Produce num combinations (default `0' for all).");
	fprintf(stderr, OPTFMT, "-d str", "Item delimiter (default is ` ').");
	fprintf(stderr, OPTFMT, "-e", "Show empty set with no items.");
	fprintf(stderr, OPTFMT, "-i num",
	    "Skip the first num-1 combinations.");
	fprintf(stderr, OPTFMT, "-k size",
	    "Number or range (`min..max' or `min-max') of items.");
	fprintf(stderr, OPTFMT, "-n num",
	    "Limit arguments taken from the command-line.");
	fprintf(stderr, OPTFMT, "-p str", "Prefix text for each line.");
	fprintf(stderr, OPTFMT, "-s str", "Suffix text for each line.");
	fprintf(stderr, OPTFMT, "-t",
	    "Print number of combinations and exit.");
	exit(EXIT_FAILURE);
}

#ifndef HAVE_OPENSSL_BN_H
/*
 * Return pseudo-random 64-bit unsigned integer in range 0 <= return <= range.
 */
static uint64_t
rand_range(uint64_t range)
{
	static const uint64_t M = (uint64_t)~0;
	uint64_t exclusion = p2(range) ? 0 : M % range + 1;
	uint64_t res = 0;

	while ((res = urand64()) < exclusion) {}
	return (res % range);
}
#endif
