/*-
 * Copyright (c) 2018-2019 Devin Teske <dteske@FreeBSD.org>
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
__FBSDID("$FrauBSD: //github.com/FrauBSD/pkgcenter/depend/cmb/cmb.c 2019-03-09 16:00:54 -0800 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <sys/param.h>
#include <sys/time.h>

#include <cmb.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
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

#ifndef UINT_MAX
#define UINT_MAX 0xFFFFFFFF
#endif

static char version[] = "$Version: 3.2.2 $";

/* Environment */
static char *pgm; /* set to argv[0] by main() */

/* Globals */
static uint8_t opt_quiet = FALSE;
static uint8_t opt_silent = FALSE;
static int cmb_transform_precision = 0;
static const char digit[11] = "0123456789";

/* Function prototypes */
static void	_Noreturn cmb_usage(void);
static uint64_t	cmb_rand_range(uint64_t range);
static		CMB_ACTION(cmb_add);
static		CMB_ACTION(cmb_div);
static		CMB_ACTION(cmb_mul);
static		CMB_ACTION(cmb_nop);
static		CMB_ACTION(cmb_sub);
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
static		CMB_ACTION_BN(cmb_add_bn);
static		CMB_ACTION_BN(cmb_div_bn);
static		CMB_ACTION_BN(cmb_mul_bn);
static		CMB_ACTION_BN(cmb_nop_bn);
static		CMB_ACTION_BN(cmb_sub_bn);
#endif
static size_t	numlen(const char *s);
static size_t	rangelen(const char *s, size_t nlen, size_t slen);
static size_t	unumlen(const char *s);
static size_t	urangelen(const char *s, size_t nlen, size_t slen);
static uint8_t	parse_range(const char *s, uint32_t *min, uint32_t *max);
static uint8_t	parse_urange(const char *s, uint32_t *min, uint32_t *max);

/* Inline functions */
static inline uint8_t	p2(uint64_t x) { return (x == (x & -x)); }
static inline uint64_t	urand64(void) { return (((uint64_t)lrand48() << 42)
    + ((uint64_t)lrand48() << 21) + (uint64_t)lrand48());
}

int
main(int argc, char *argv[])
{
	uint8_t opt_empty = FALSE;
#ifdef HAVE_LIBCRYPTO
	uint8_t opt_nossl = FALSE;
#endif
	uint8_t opt_randi = FALSE;
	uint8_t opt_range = FALSE;
	uint8_t opt_total = FALSE;
	uint8_t opt_version = FALSE;
	char *cp;
	char *cmdver = version;
	char **items = argv;
	char **items_tmp = NULL;
	const char *libver = cmb_version(CMB_VERSION);
	char *opt_file = NULL;
	char *opt_transform = NULL;
	int ch;
	int len;
	int retval = EXIT_SUCCESS;
	uint32_t n;
	uint32_t nitems = 0;
	uint32_t r;
	uint32_t range_max = 0;
	uint32_t range_min = 0;
	size_t config_size = sizeof(struct cmb_config);
	size_t optlen;
	struct cmb_config *config = NULL;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
	BIGNUM *count_bn;
#endif
	uint64_t count;
	uint64_t nstart = 0; /* negative start */
	uint64_t ull;
	unsigned long ul;
	long double ld;
	struct timeval tv;
	char range_tmp[11];

	pgm = argv[0]; /* store a copy of invocation name */

	/* Allocate config structure */
	if ((config = malloc(config_size)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	bzero(config, sizeof(struct cmb_config));

	/*
	 * Process command-line options
	 */
#define OPTSTRING "0c:Dd:ef:i:k:Nn:op:qr:Ss:tvX:"
	while ((ch = getopt(argc, argv, OPTSTRING)) != -1) {
		switch(ch) {
		case '0': /* NUL terminate */
			config->nul_terminate = TRUE;
			break;
		case 'c': /* count */
			if (*optarg < 48 || *optarg > 57) {
				errx(EXIT_FAILURE, "-c: %s `%s'",
				    strerror(EINVAL), optarg);
				/* NOTREACHED */
			}
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			if (!opt_nossl &&
			    BN_dec2bn(&(config->count_bn), optarg) == 0)
				errx(EXIT_FAILURE, "OpenSSL Error?!");
			if (opt_nossl) {
#endif
				errno = 0;
				config->count = strtoull(optarg,
				    (char **)NULL, 10);
				if (errno != 0) {
					errx(EXIT_FAILURE, "-c: %s `%s'",
					    strerror(errno), optarg);
					/* NOTREACHED */
				}
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			}
#endif
			break;
		case 'D': /* debug */
			config->debug = TRUE;
			break;
		case 'd': /* delimiter */
			config->delimiter = optarg;
			break;
		case 'e': /* empty */
			opt_empty = TRUE;
			config->show_empty = opt_empty;
			break;
		case 'f': /* file */
			opt_file = optarg;
			break;
		case 'i': /* start */
			if ((optlen = strlen(optarg)) > 0 &&
			    strncmp("random", optarg, optlen) == 0) {
				opt_randi = TRUE;
			} else if (optlen == 0 || numlen(optarg) != optlen) {
				errx(EXIT_FAILURE, "-i: %s `%s'",
				    strerror(EINVAL), optarg);
				/* NOTREACHED */
			}
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			if (!opt_randi && !opt_nossl &&
			    BN_dec2bn(&(config->start_bn), optarg) == 0)
				errx(EXIT_FAILURE, "OpenSSL Error?!");
			if (!opt_randi && opt_nossl) {
#else
			if (!opt_randi) {
#endif
				errno = 0;
				if (*optarg == '-') {
					nstart = strtoull(&optarg[1],
					    (char **)NULL, 10);
				} else {
					config->start = strtoull(optarg,
					    (char **)NULL, 10);
				}
				if (errno != 0) {
					errx(EXIT_FAILURE, "-i: %s `%s'",
					    strerror(errno), optarg);
					/* NOTREACHED */
				}
			}
			break;
		case 'k': /* size */
			if (!parse_range(optarg, &(config->size_min),
			    &(config->size_max))) {
				errx(EXIT_FAILURE, "-k: %s `%s'",
				    strerror(errno), optarg);
				/* NOTREACHED */
			}
			break;
		case 'N': /* numbers */
			config->show_numbers = TRUE;
			break;
		case 'n': /* args */
			if (*optarg < 48 || *optarg > 57) {
				errx(EXIT_FAILURE, "-n: %s `%s'",
				    strerror(EINVAL), optarg);
				/* NOTREACHED */
			}
			errno = 0;
			ull = strtoull(optarg, (char **)NULL, 10);
			if (errno != 0) {
				errx(EXIT_FAILURE, "-n: %s `%s'",
				    strerror(errno), optarg);
				/* NOTREACHED */
			} else if (ull > UINT_MAX) {
				errx(EXIT_FAILURE, "-n: %s `%s'",
				    strerror(ERANGE), optarg);
				/* NOTREACHED */
			}
			nitems = (uint32_t)ull;
			break;
		case 'o': /* disable openssl */
#ifdef HAVE_LIBCRYPTO
			opt_nossl = TRUE;
#endif
			break;
		case 'p': /* prefix */
			config->prefix = optarg;
			break;
		case 'q': /* quiet */
			opt_quiet = 1;
			break;
		case 'r': /* range */
			if (!parse_urange(optarg, &range_min, &range_max)) {
				errx(EXIT_FAILURE, "-r: %s `%s'",
				    strerror(errno), optarg);
				/* NOTREACHED */
			}
			if (unumlen(optarg) == strlen(optarg)) {
				range_max = range_min;
				range_min = 1;
			}
			opt_range = TRUE;
			break;
		case 'S': /* silent */
			opt_silent = TRUE;
			break;
		case 's': /* suffix */
			config->suffix = optarg;
			break;
		case 't': /* total */
			opt_total = TRUE;
			break;
		case 'X': /* transform */
			opt_transform = optarg;
			break;
		case 'v': /* version */
			opt_version = TRUE;
			break;
		default: /* unhandled argument (based on switch) */
			cmb_usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;
	items = argv;

	if (opt_version) {
		cmdver += 10; /* Seek past "$Version: " */
		cmdver[strlen(cmdver)-2] = '\0'; /* Place NUL before "$" */
#ifdef HAVE_OPENSSL_CRYPTO_H
		printf("%s: %s (%s; %s)\n", pgm, cmdver, libver,
		    SSLeay_version(SSLEAY_VERSION));
#else
		printf("%s: %s (%s)\n", pgm, cmdver, libver);
#endif
		exit(EXIT_FAILURE);
	}

	/* Print total for num items if given `-t -r range' */
	if (opt_total && opt_range) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		if (range_min < range_max)
			nitems = range_max - range_min + 1;
		else
			nitems = range_min - range_max + 1;
		if (!opt_nossl) {
			char *count_str;

			if ((count_bn =
			    cmb_count_bn(config, nitems)) != NULL) {
				count_str = BN_bn2dec(count_bn);
				printf("%s\n", count_str);
#ifdef HAVE_OPENSSL_CRYPTO_H
				OPENSSL_free(count_str);
#endif
				BN_free(count_bn);
			} else
				printf("0\n");
		} else {
#endif
			count = cmb_count(config, nitems);
			if (errno) {
				err(EXIT_FAILURE, NULL);
				/* NOTREACHED */
			}
			printf("%"PRIu64"\n", count);
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		}
#endif
		exit(EXIT_SUCCESS);
	}

	/* At least one non-option argument is required */
	if (argc == 0 && !opt_empty && opt_file == NULL && !opt_range) {
		cmb_usage();
		/* NOTREACHED */
	}

	/* Read arguments ... */
	if (opt_file != NULL) {
		/* ... from file if give `-f file' */
		if (argc > 0)
			warnx("arguments ignored when `-f file' given");
		if ((items = malloc(sizeof(char *) * 0xffffffff)) == NULL)
			errx(EXIT_FAILURE, "Out of memory?!");
		items = cmb_parse_file(config, opt_file, &nitems, nitems);
		if (items == NULL && errno != 0) {
			err(EXIT_FAILURE, NULL);
			/* NOTREACHED */
		}
	} else if (opt_range) {
		/* ... generated from `-r range' */
		nitems = range_max - range_min + 1;
		items = calloc(nitems, sizeof(char *));
		r = range_min;
		for (n = 0; n < nitems; n++) {
			len = snprintf(range_tmp, sizeof(range_tmp), "%u", r);
			items[n] = (char *)malloc((unsigned long)len + 1);
			memcpy(items[n], range_tmp, len + 1);
			if (range_min > range_max)
				r--;
			else
				r++;
		}
	} else if (nitems == 0 || nitems > (uint32_t)argc)
		/* ... from command-line */
		nitems = (uint32_t)argc;

	/*
	 * Time-based benchmarking (-S for silent) and transforms (-X func).
	 *
	 * NB: For benchmarking, the call-stack is still incremented into the
	 *     action, while using a nop function allows us to benchmark
	 *     various action overhead.
	 */
	if (opt_silent && opt_transform == NULL) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		if (opt_nossl)
#endif
			config->action = cmb_nop;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		else
			config->action_bn = cmb_nop_bn;
#endif
		config->show_numbers = FALSE;
	} else if (opt_transform != NULL) {
		if ((optlen = strlen(opt_transform)) == 0) {
			errx(EXIT_FAILURE, "-X: %s `'", strerror(EINVAL));
			/* NOTREACHED */
		}
		if (strncmp("multiply", opt_transform, optlen) == 0) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			if (opt_nossl)
#endif
				config->action = cmb_mul;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			else
				config->action_bn = cmb_mul_bn;
#endif
		} else if (strncmp("divide", opt_transform, optlen) == 0) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			if (opt_nossl)
#endif
				config->action = cmb_div;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			else
				config->action_bn = cmb_div_bn;
#endif
		} else if (strncmp("add", opt_transform, optlen) == 0) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			if (opt_nossl)
#endif
				config->action = cmb_add;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			else
				config->action_bn = cmb_add_bn;
#endif
		} else if (strncmp("subtract", opt_transform, optlen) == 0) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			if (opt_nossl)
#endif
				config->action = cmb_sub;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
			else
				config->action_bn = cmb_sub_bn;
#endif
		} else {
			errx(EXIT_FAILURE, "-X: %s `%s'", strerror(EINVAL),
			    opt_transform);
			/* NOTREACHED */
		}

		/*
		 * Convert items into array of pointers to long double
		 * NB: Transformation function does not perform conversions
		 */
		items_tmp = calloc(nitems, sizeof(long double *));
		if (items_tmp == NULL)
			errx(EXIT_FAILURE, "Out of memory?!");
			/* NOTREACHED */
		for (n = 0; n < nitems; n++) {
			ul = sizeof(long double);
			if ((items_tmp[n] = (char *)malloc(ul)) == NULL)
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			ld = strtold(items[n], NULL);
			memcpy(items_tmp[n], &ld, ul);
			if ((cp = strstr(items[n], ".")) != NULL) {
				len = (int)strlen(items[n]);
				len -= cp - items[n] + 1;
				if (len > cmb_transform_precision)
					cmb_transform_precision = len;
			}
		}
		items = items_tmp;
	}

	/*
	 * Calculate combinations
	 */
	if (opt_total) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		if (!opt_nossl) {
			char *count_str;

			if ((count_bn =
			    cmb_count_bn(config, nitems)) != NULL) {
				count_str = BN_bn2dec(count_bn);
				printf("%s\n", count_str);
#ifdef HAVE_OPENSSL_CRYPTO_H
				OPENSSL_free(count_str);
#endif
				BN_free(count_bn);
			} else
				printf("0\n");
		} else {
#endif
			count = cmb_count(config, nitems);
			if (errno) {
				err(EXIT_FAILURE, NULL);
				/* NOTREACHED */
			}
			printf("%"PRIu64"\n", count);
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		}
	} else if (!opt_nossl) {
		if (opt_randi) {
			if ((count_bn =
			    cmb_count_bn(config, nitems)) != NULL) {
				if (config->start_bn == NULL)
					config->start_bn = BN_new();
				if (BN_rand_range(config->start_bn, count_bn))
					BN_add_word(config->start_bn, 1);
			}
		} else if (config->start_bn != NULL &&
		    BN_is_negative(config->start_bn)) {
			if ((count_bn =
			    cmb_count_bn(config, nitems)) != NULL) {
				BN_add(config->start_bn,
				    count_bn, config->start_bn);
				BN_add_word(config->start_bn, 1);
				if (BN_is_negative(config->start_bn))
					BN_zero(config->start_bn);
				BN_free(count_bn);
			}
		}
		retval = cmb_bn(config, nitems, items);
#endif
	} else {
		if (opt_randi) {
			count = cmb_count(config, nitems);
			if (errno) {
				err(EXIT_FAILURE, NULL);
				/* NOTREACHED */
			}
			if (gettimeofday(&tv,NULL) == 0) {
				srand48((long)tv.tv_usec);
				config->start = cmb_rand_range(count) + 1;
			}
		} else if (nstart != 0) {
			count = cmb_count(config, nitems);
			if (errno) {
				err(EXIT_FAILURE, NULL);
				/* NOTREACHED */
			}
			if (count > nstart)
				config->start = count - nstart + 1;
			else
				config->start = 0;
		}
		retval = cmb(config, nitems, items);
		if (errno) {
			err(EXIT_FAILURE, NULL);
			/* NOTREACHED */
		}
	}

	/*
	 * Clean up
	 */
	if (opt_transform) {
		for (n = 0; n < nitems; n++)
			free(items_tmp[n]);
		free(items_tmp);
	}

	return (retval);
}

/*
 * Print short usage statement to stderr and exit with error status.
 */
static void
cmb_usage(void)
{
	fprintf(stderr, "usage: %s [options] item1 [item2 ...]\n", pgm);
#define OPTFMT		"\t%-10s %s\n"
#define OPTFMT_1U	"\t%-10s %s%u%s\n"
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, OPTFMT, "-0",
	    "NUL terminate combinations (use with `xargs -0').");
	fprintf(stderr, OPTFMT, "-c num",
	    "Produce num combinations (default `0' for all).");
	fprintf(stderr, OPTFMT, "-D",
	    "Enable debugging information on stderr."
	);
	fprintf(stderr, OPTFMT, "-d text", "Item delimiter (default is ` ').");
	fprintf(stderr, OPTFMT, "-e", "Show empty set with no items.");
	fprintf(stderr, OPTFMT, "-f file",
	    "Read items from file; `-' for stdin.");
	fprintf(stderr, OPTFMT, "-i num",
	    "Skip the first num-1 combinations.");
	fprintf(stderr, OPTFMT, "-k size",
	    "Number or range (`min..max' or `min-max') of items.");
	fprintf(stderr, OPTFMT, "-N", "Show combination sequence numbers.");
	fprintf(stderr, OPTFMT, "-n num",
	    "Limit arguments taken from the command-line.");
	fprintf(stderr, OPTFMT, "-o",
	    "Disable OpenSSL (limits calculations to 64-bits).");
	fprintf(stderr, OPTFMT, "-p text", "Prefix text for each line.");
	fprintf(stderr, OPTFMT, "-q",
	    "Quiet. Do not print items from set when given `-X op'.");
	fprintf(stderr, OPTFMT_1U, "-r range",
	    "Specify range of up-to ", UINT_MAX, " items.");
	fprintf(stderr, OPTFMT, "-S", "Silent (for performance benchmarks).");
	fprintf(stderr, OPTFMT, "-s text", "Suffix text for each line.");
	fprintf(stderr, OPTFMT, "-t",
	    "Print number of combinations and exit.");
	fprintf(stderr, OPTFMT, "-v",
	    "Print version info to stdout and exit.");
	fprintf(stderr, OPTFMT, "-X op",
	    "Perform math on items where `op' is add, sub, div, or mul.");
	exit(EXIT_FAILURE);
}

/*
 * Return pseudo-random 64-bit unsigned integer in range 0 <= return <= range.
 */
static uint64_t
cmb_rand_range(uint64_t range)
{
	static const uint64_t M = (uint64_t)~0;
	uint64_t exclusion = p2(range) ? 0 : M % range + 1;
	uint64_t res = 0;

	while ((res = urand64()) < exclusion) {}
	return (res % range);
}

static size_t
numlen(const char *s)
{
	if (s == NULL || *s == '\0')
		return (0);
	else if (*s == '-')
		return (strspn(&s[1], digit) + 1);
	else
		return (strspn(s, digit));
}

static size_t
rangelen(const char *s, size_t nlen, size_t slen)
{
	size_t rlen;
	const char *cp = s;

	if (nlen == 0)
		return (0);
	else if (nlen == slen)
		return (nlen);

	cp += nlen;
	if (*cp == '-') {
		cp += 1;
		if (*cp == '\0')
			return (0);
		return (nlen + strspn(cp, digit) + 1);
	} else if (strncmp(cp, "..", 2) == 0) {
		cp += 2;
		if ((rlen = numlen(cp)) == 0)
			return (0);
		return (nlen + rlen + 2);
	} else
		return (0);
}

static uint8_t
parse_range(const char *s, uint32_t *min, uint32_t *max)
{
	const char *cp;
	size_t nlen;
	size_t optlen;
	uint64_t ull;

	errno = 0;

	if (s == NULL || ((nlen = numlen(s)) != (optlen = strlen(s)) &&
	    rangelen(s, nlen, optlen) != optlen)) {
		errno = EINVAL;
		return (FALSE);
	}

	if (*s == '-') {
		ull = strtoull(&s[1], (char **)NULL, 10);
		*min = UINT_MAX - (uint32_t)ull;
	} else {
		ull = strtoull(s, (char **)NULL, 10);
		*min = (uint32_t)ull;
	}
	if (errno != 0)
		return (FALSE);
	else if (ull > UINT_MAX) {
		errno = ERANGE;
		return (FALSE);
	}

	cp = &s[nlen];
	if (*cp == '\0')
		*max = *s == '-' ? (uint32_t)ull : *min;
	else if ((strncmp(cp, "..", 2)) == 0) {
		cp += 2;
		if (*cp == '-') {
			ull = strtoull(&cp[1], (char **)NULL, 10);
			*max = UINT_MAX - (uint32_t)ull;
		} else {
			ull = strtoull(cp, (char **)NULL, 10);
			*max = (uint32_t)ull;
		}
		if (errno != 0)
			return (FALSE);
		else if (ull > UINT_MAX) {
			errno = ERANGE;
			return (FALSE);
		}
	} else if (*cp == '-') {
		ull = strtoull(&cp[1], (char **)NULL, 10);
		if (errno != 0)
			return (FALSE);
		else if (ull > UINT_MAX) {
			errno = ERANGE;
			return (FALSE);
		}
		*max = (uint32_t)ull;
	} else {
		errno = EINVAL;
		return (FALSE);
	}

	return (TRUE);
}

static size_t
unumlen(const char *s)
{
	if (s == NULL || *s == '\0')
		return (0);
	else
		return (strspn(s, digit));
}

static size_t
urangelen(const char *s, size_t nlen, size_t slen)
{
	size_t rlen;
	const char *cp = s;

	if (nlen == 0)
		return (0);
	else if (nlen == slen)
		return (nlen);

	cp += nlen;
	if (*cp == '-') {
		cp += 1;
		if (*cp == '\0')
			return (0);
		return (nlen + strspn(cp, digit) + 1);
	} else if (strncmp(cp, "..", 2) == 0) {
		cp += 2;
		if ((rlen = unumlen(cp)) == 0)
			return (0);
		return (nlen + rlen + 2);
	} else
		return (0);
}

static uint8_t
parse_urange(const char *s, uint32_t *min, uint32_t *max)
{
	const char *cp;
	size_t nlen;
	size_t optlen;
	uint64_t ull;

	errno = 0;

	if (s == NULL || ((nlen = unumlen(s)) != (optlen = strlen(s)) &&
	    urangelen(s, nlen, optlen) != optlen) || *s == '-') {
		errno = EINVAL;
		return (FALSE);
	}

	ull = strtoull(s, (char **)NULL, 10);
	*min = *max = (uint32_t)ull;
	if (errno != 0)
		return (FALSE);
	else if (ull > UINT_MAX) {
		errno = ERANGE;
		return (FALSE);
	}

	cp = &s[nlen];
	if (*cp == '\0')
		*max = *min;
	else if ((strncmp(cp, "..", 2)) == 0) {
		cp += 2;
		ull = strtoull(cp, (char **)NULL, 10);
		*max = (uint32_t)ull;
		if (errno != 0)
			return (FALSE);
		else if (ull > UINT_MAX) {
			errno = ERANGE;
			return (FALSE);
		}
	} else if (*cp == '-') {
		ull = strtoull(&cp[1], (char **)NULL, 10);
		if (errno != 0)
			return (FALSE);
		else if (ull > UINT_MAX) {
			errno = ERANGE;
			return (FALSE);
		}
		*max = (uint32_t)ull;
	} else {
		errno = EINVAL;
		return (FALSE);
	}

	return (TRUE);
}

/*
 * For performance benchmarking
 */
static
CMB_ACTION(cmb_nop)
{
	(void)config;
	(void)seq;
	(void)nitems;
	(void)items;
	return (0);
}
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
static
CMB_ACTION_BN(cmb_nop_bn)
{
	(void)config;
	(void)seq;
	(void)nitems;
	(void)items;
	return (0);
}
#endif

/*
 * Set transformations
 */
static CMB_TRANSFORM_OP(*, cmb_mul);
static CMB_TRANSFORM_OP(/, cmb_div);
static CMB_TRANSFORM_OP(+, cmb_add);
static CMB_TRANSFORM_OP(-, cmb_sub);
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
static CMB_TRANSFORM_OP_BN(*, cmb_mul_bn);
static CMB_TRANSFORM_OP_BN(/, cmb_div_bn);
static CMB_TRANSFORM_OP_BN(+, cmb_add_bn);
static CMB_TRANSFORM_OP_BN(-, cmb_sub_bn);
#endif
