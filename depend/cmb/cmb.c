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
__FBSDID("$FrauBSD: pkgcenter/depend/cmb/cmb.c 2019-08-23 20:37:14 -0700 freebsdfrau $");
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

#ifndef UINT_MAX
#define UINT_MAX 0xFFFFFFFF
#endif

static char version[] = "$Version: 3.9.5 $";

/* Environment */
static char *pgm; /* set to argv[0] by main() */

/* Globals */
static uint8_t opt_quiet = FALSE;
static uint8_t opt_silent = FALSE;
static const char digit[11] = "0123456789";

#ifndef _Noreturn
#define _Noreturn __attribute__((noreturn))
#endif

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
static		CMB_ACTION(cmb_add_find);
static		CMB_ACTION(cmb_div_find);
static		CMB_ACTION(cmb_mul_find);
static		CMB_ACTION(cmb_sub_find);
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
static		CMB_ACTION_BN(cmb_add_find_bn);
static		CMB_ACTION_BN(cmb_div_find_bn);
static		CMB_ACTION_BN(cmb_mul_find_bn);
static		CMB_ACTION_BN(cmb_sub_find_bn);
#endif
static size_t	numlen(const char *s);
static size_t	rangelen(const char *s, size_t nlen, size_t slen);
static size_t	unumlen(const char *s);
static size_t	urangelen(const char *s, size_t nlen, size_t slen);
static uint8_t	parse_range(const char *s, uint32_t *min, uint32_t *max);
static uint8_t	parse_unum(const char *s, uint32_t *n);
static uint8_t	parse_urange(const char *s, uint32_t *min, uint32_t *max);
static uint32_t	range_char(uint32_t start, uint32_t stop, uint32_t idx,
    char *dst[]);
static uint32_t	range_float(uint32_t start, uint32_t stop, uint32_t idx,
    char *dst[]);

/* Inline functions */
static inline uint8_t	p2(uint64_t x) { return (x == (x & -x)); }
static inline uint64_t	urand64(void) { return (((uint64_t)lrand48() << 42)
    + ((uint64_t)lrand48() << 21) + (uint64_t)lrand48());
}

/*
 * Transformations (-X op)
 */
struct cmb_xfdef
{
	const char *opname;
	CMB_ACTION((*action));
	CMB_ACTION((*action_find));
};
static struct cmb_xfdef cmb_xforms[] = {
	/* opname    action */
	{"multiply", cmb_mul, cmb_mul_find},
	{"divide",   cmb_div, cmb_div_find},
	{"add",      cmb_add, cmb_add_find},
	{"subtract", cmb_sub, cmb_sub_find},
	{NULL,       NULL,    NULL},
};
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
struct cmb_xfdef_bn
{
	const char *opname;
	CMB_ACTION_BN((*action_bn));
	CMB_ACTION_BN((*action_find_bn));
};
static struct cmb_xfdef_bn cmb_xforms_bn[] = {
	/* opname    action_bn */
	{"multiply", cmb_mul_bn, cmb_mul_find_bn},
	{"divide",   cmb_div_bn, cmb_div_find_bn},
	{"add",      cmb_add_bn, cmb_add_find_bn},
	{"subtract", cmb_sub_bn, cmb_sub_find_bn},
	{NULL,       NULL,       NULL},
};
#endif

int
main(int argc, char *argv[])
{
	uint8_t free_find = FALSE;
	uint8_t opt_empty = FALSE;
	uint8_t opt_file = FALSE;
	uint8_t opt_find = FALSE;
#ifdef HAVE_LIBCRYPTO
	uint8_t opt_nossl = FALSE;
#endif
	uint8_t opt_nulparse = FALSE;
	uint8_t opt_nulprint = FALSE;
	uint8_t opt_precision = FALSE;
	uint8_t opt_randi = FALSE;
	uint8_t opt_range = FALSE;
	uint8_t opt_total = FALSE;
	uint8_t opt_version = FALSE;
	const char *cp;
	char *cmdver = version;
	char *endptr = NULL;
	char **items = NULL;
	char **items_tmp = NULL;
	const char *libver = cmb_version(CMB_VERSION);
	char *opt_transform = NULL;
	int ch;
	int len;
	int retval = EXIT_SUCCESS;
	uint32_t i;
	uint32_t n;
	uint32_t nitems = 0;
	uint32_t rstart = 0;
	uint32_t rstop = 0;
	size_t config_size = sizeof(struct cmb_config);
	size_t cp_size = sizeof(char *);
	size_t optlen;
	struct cmb_config *config = NULL;
	struct cmb_xitem *xitem = NULL;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
	BIGNUM *count_bn;
#endif
	uint64_t count;
	uint64_t fitems = 0;
	uint64_t nstart = 0; /* negative start */
	uint64_t ritems = 0;
	uint64_t ull;
	unsigned long ul;
	struct timeval tv;

	pgm = argv[0]; /* store a copy of invocation name */

	/* Allocate config structure */
	if ((config = malloc(config_size)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	bzero(config, sizeof(struct cmb_config));

	/*
	 * Process command-line options
	 */
#define OPTSTRING "0c:Dd:eF:fi:k:Nn:oP:p:qrSs:tvX:z"
	while ((ch = getopt(argc, argv, OPTSTRING)) != -1) {
		switch(ch) {
		case '0': /* NUL terminate */
			config->options ^= CMB_OPT_NULPARSE;
			opt_nulparse = TRUE;
			break;
		case 'c': /* count */
			if ((optlen = strlen(optarg)) == 0 ||
			    unumlen(optarg) != optlen) {
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
			config->options ^= CMB_OPT_DEBUG;
			break;
		case 'd': /* delimiter */
			config->delimiter = optarg;
			break;
		case 'e': /* empty */
			opt_empty = TRUE;
			config->options ^= CMB_OPT_EMPTY;
			break;
		case 'F': /* find */
			opt_find = TRUE;
			if (cmb_transform_find != NULL)
				free(cmb_transform_find);
			if ((cmb_transform_find =
			    malloc(sizeof(struct cmb_xitem))) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			cmb_transform_find->cp = optarg;
			endptr = NULL;
			errno = 0;
			cmb_transform_find->as.ld = strtold(optarg, &endptr);
			if (endptr == NULL || *endptr != '\0') {
				if (errno == 0)
					errno = EINVAL;
				errx(EXIT_FAILURE, "-F: %s `%s'",
				    strerror(errno), optarg);
				/* NOTREACHED */
			}
			break;
		case 'f': /* file */
			opt_file = TRUE;
			opt_range = FALSE;
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
			config->options ^= CMB_OPT_NUMBERS;
			break;
		case 'n': /* n-args */
			if (!parse_unum(optarg, &nitems)) {
				errx(EXIT_FAILURE, "-n: %s `%s'",
				    strerror(errno), optarg);
				/* NOTREACHED */
			}
			break;
		case 'o': /* disable openssl */
#ifdef HAVE_LIBCRYPTO
			opt_nossl = TRUE;
#endif
			break;
		case 'P': /* precision */
			if (!parse_unum(optarg,
			    (uint32_t *)&cmb_transform_precision)) {
				errx(EXIT_FAILURE, "-n: %s `%s'",
				    strerror(errno), optarg);
				/* NOTREACHED */
			}
			opt_precision = TRUE;
			break;
		case 'p': /* prefix */
			config->prefix = optarg;
			break;
		case 'q': /* quiet */
			opt_quiet = 1;
			break;
		case 'r': /* range */
			opt_range = TRUE;
			opt_file = FALSE;
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
		case 'v': /* version */
			opt_version = TRUE;
			break;
		case 'X': /* transform */
			opt_transform = optarg;
			break;
		case 'z': /* zero */
			opt_nulprint = TRUE;
			config->options ^= CMB_OPT_NULPRINT;
			break;
		default: /* unhandled argument (based on switch) */
			cmb_usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	/*
	 * Process `-v' command-line option
	 */
	if (opt_version) {
		cmdver += 10; /* Seek past "$Version: " */
		cmdver[strlen(cmdver)-2] = '\0'; /* Place NUL before "$" */
#ifdef HAVE_OPENSSL_CRYPTO_H
		printf("%s: %s (%s; %s)", pgm, cmdver, libver,
		    SSLeay_version(SSLEAY_VERSION));
#else
		printf("%s: %s (%s)", pgm, cmdver, libver);
#endif
		if (cmb_build_info.debug)
			printf(" [debug]");
		printf("\n");
		exit(EXIT_SUCCESS);
	}

	/*
	 * At least one non-option argument is required unless `-e' is given
	 */
	if (argc == 0 && !opt_empty) {
		warnx("argument required unless `-e'");
		cmb_usage();
		/* NOTREACHED */
	}

	/*
	 * `-X op' required if given `-F num'
	 */
	if (opt_find && opt_transform == NULL) {
		errx(EXIT_FAILURE, "`-X op' required when using `-F num'");
		/* NOTREACHED */
	}

	/*
	 * `-X op' required if given `-P num'
	 */
	if (opt_precision && opt_transform == NULL) {
		errx(EXIT_FAILURE, "`-X op' required when using `-P num'");
		/* NOTREACHED */
	}

	/*
	 * `-f' required if given `-0'
	 */
	if (opt_nulparse && !opt_file) {
		errx(EXIT_FAILURE, "`-f' required when using `-0'");
		/* NOTREACHED */
	}

	/*
	 * Calculate number of items
	 */
	if (nitems == 0 || nitems > (uint32_t)argc)
		nitems = (uint32_t)argc;
	if (opt_range) {
		for (n = 0; n < nitems; n++) {
			if (!parse_urange(argv[n], &rstart, &rstop)) {
				errx(EXIT_FAILURE, "-r: %s `%s'",
				    strerror(errno), argv[n]);
				/* NOTREACHED */
			}
			if (unumlen(argv[n]) == strlen(argv[n])) {
				rstop = rstart;
				rstart = 1;
			}
			if (rstart < rstop)
				ull = rstop - rstart + 1;
			else
				ull = rstart - rstop + 1;
			if (ritems + ull > UINT_MAX) {
				errx(EXIT_FAILURE, "-r: Too many items");
				/* NOTREACHED */
			}
			ritems += ull;
		}
	}

	/*
	 * Print total for num items and exit if given `-t -r'
	 */
	if (opt_total && opt_range) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		if (!opt_nossl) {
			char *count_str;

			count_bn = cmb_count_bn(config, (uint32_t)ritems);
			if (opt_silent)
				exit(EXIT_SUCCESS);
			if (count_bn != NULL) {
				count_str = BN_bn2dec(count_bn);
				printf("%s%s", count_str,
				    opt_nulprint ? "" : "\n");
				OPENSSL_free(count_str);
				BN_free(count_bn);
			} else
				printf("0%s", opt_nulprint ? "" : "\n");
		} else {
#endif
			count = cmb_count(config, (uint32_t)ritems);
			if (opt_silent)
				exit(EXIT_SUCCESS);
			if (errno) {
				err(EXIT_FAILURE, NULL);
				/* NOTREACHED */
			}
			printf("%"PRIu64"%s", count, opt_nulprint ? "" : "\n");
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		}
#endif
		exit(EXIT_SUCCESS);
	}

	/*
	 * Read arguments ...
	 */
	if (opt_file) {
		/* ... as a series of files if given `-f' */
		for (n = 0; n < nitems; n++) {
			items_tmp = cmb_parse_file(config, argv[n], &i, 0);
			if (items_tmp == NULL && errno != 0) {
				err(EXIT_FAILURE, NULL);
				/* NOTREACHED */
			}
			if (fitems + i > UINT_MAX) {
				errx(EXIT_FAILURE, "-f: Too many items");
				/* NOTREACHED */
			}
			fitems += (uint64_t)i;
			items = realloc(items, (size_t)(fitems * cp_size));
			if (items == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			(void)memcpy(&items[fitems-i], items_tmp, i * cp_size);
		}
		nitems = (uint32_t)fitems;
	} else if (opt_range) {
		/* ... as a series of ranges if given `-r' */
		if ((items = calloc(ritems, sizeof(char *))) == NULL) {
			errx(EXIT_FAILURE, "Out of memory?!");
			/* NOTREACHED */
		}
		i = 0;
		for (n = 0; n < nitems; n++) {
			parse_urange(argv[n], &rstart, &rstop);
			if (unumlen(argv[n]) == strlen(argv[n])) {
				rstop = rstart;
				rstart = 1;
			}
			if (opt_transform != NULL)
				i = range_float(rstart, rstop, i, items);
			else
				i = range_char(rstart, rstop, i, items);
		}
		nitems = (uint32_t)ritems;
	} else {
		/* ... as a series of strings or numbers if given `-X op' */
		items = argv;
	}

	/*
	 * Time-based benchmarking (-S for silent) and transforms (-X op).
	 *
	 * NB: For benchmarking, the call-stack is still incremented into the
	 *     action, while using a nop function allows us to benchmark
	 *     various action overhead.
	 */
	if (opt_silent && opt_transform == NULL) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		if (opt_nossl)
			config->action = cmb_nop;
		else
			config->action_bn = cmb_nop_bn;
#else
		config->action = cmb_nop;
#endif
		config->options &= ~CMB_OPT_NUMBERS;
	} else if (opt_transform != NULL) {
		if ((optlen = strlen(opt_transform)) == 0) {
			errx(EXIT_FAILURE, "-X: %s `'", strerror(EINVAL));
			/* NOTREACHED */
		}
		ch = -1;
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		if (opt_nossl) {
			while ((cp = cmb_xforms[++ch].opname) != NULL) {
				if (strncmp(cp, opt_transform, optlen) != 0)
					continue;
				if (opt_find)
					config->action =
					    cmb_xforms[ch].action_find;
				else
					config->action = cmb_xforms[ch].action;
				break;
			}
		} else {
			while ((cp = cmb_xforms_bn[++ch].opname) != NULL) {
				if (strncmp(cp, opt_transform, optlen) != 0)
					continue;
				if (opt_find)
					config->action_bn =
					    cmb_xforms_bn[ch].action_find_bn;
				else
					config->action_bn =
					    cmb_xforms_bn[ch].action_bn;
				break;
			}
		}
		if (config->action == NULL && config->action_bn == NULL) {
			errx(EXIT_FAILURE, "-X: %s `%s'", strerror(EINVAL),
			    opt_transform);
			/* NOTREACHED */
		}
#else
		while ((cp = cmb_xforms[++ch].opname) != NULL) {
			if (strncmp(cp, opt_transform, optlen) != 0)
				continue;
			if (opt_find)
				config->action = cmb_xforms[ch].action_find;
			else
				config->action = cmb_xforms[ch].action;
			break;
		}
		if (config->action == NULL) {
			errx(EXIT_FAILURE, "-X: %s `%s'", strerror(EINVAL),
			    opt_transform);
			/* NOTREACHED */
		}
#endif

		/*
		 * Convert items into array of struct pointers
		 * NB: Transformation function does not perform conversions
		 */
		if (!opt_range) {
			ul = sizeof(struct cmb_xitem *);
			if ((items_tmp = calloc(nitems, ul)) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			for (n = 0; n < nitems; n++) {
				if ((xitem = malloc(sizeof(struct cmb_xitem)))
				    == NULL) {
					errx(EXIT_FAILURE, "Out of memory?!");
					/* NOTREACHED */
				}
				xitem->cp = items[n];
				endptr = NULL;
				errno = 0;
				xitem->as.ld = strtold(items[n], &endptr);
				if (endptr == NULL || *endptr != '\0') {
					if (errno == 0)
						errno = EINVAL;
					errx(EXIT_FAILURE, "-X: %s `%s'",
						strerror(errno), items[n]);
					/* NOTREACHED */
				}
				items_tmp[n] = (char *)xitem;
				if (opt_precision)
					continue;
				if ((cp = strchr(items[n], '.')) != NULL) {
					len = (int)strlen(items[n]);
					len -= cp - items[n] + 1;
					if (len > cmb_transform_precision)
						cmb_transform_precision = len;
				}
			}
			items = items_tmp;
		}
	}

	/*
	 * Adjust precision based on `-F num'
	 */
	if (opt_find) {
		if ((cp = strchr(cmb_transform_find->cp, '.')) != NULL) {
			len = (int)strlen(cmb_transform_find->cp);
			len -= cp - cmb_transform_find->cp + 1;
			if (len > cmb_transform_precision) {
				cmb_transform_precision = len;
			} else {
				len = snprintf(NULL, 0, "%.*Lf",
				    cmb_transform_precision,
				    cmb_transform_find->as.ld) + 1;
				cmb_transform_find->cp =
				    malloc((unsigned long)len);
				if (cmb_transform_find->cp == NULL) {
					errx(EXIT_FAILURE, "Out of memory?!");
					/* NOTREACHED */
				}
				free_find = TRUE;
				(void)sprintf(cmb_transform_find->cp, "%.*Lf",
				    cmb_transform_precision,
				    cmb_transform_find->as.ld);
			}
		} else if (cmb_transform_precision > 0) {
			len = snprintf(NULL, 0, "%.*Lf",
			    cmb_transform_precision,
			    cmb_transform_find->as.ld);
			cmb_transform_find->cp = malloc((unsigned long)len);
			if (cmb_transform_find->cp == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			free_find = TRUE;
			(void)sprintf(cmb_transform_find->cp,
				"%.*Lf", cmb_transform_precision,
				cmb_transform_find->as.ld);
		}
	}

	/*
	 * Calculate combinations
	 */
	if (opt_total) {
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
		if (!opt_nossl) {
			char *count_str;

			count_bn = cmb_count_bn(config, nitems);
			if (opt_silent)
				exit(EXIT_SUCCESS);
			if (count_bn != NULL) {
				count_str = BN_bn2dec(count_bn);
				printf("%s%s", count_str,
				    opt_nulprint ? "" : "\n");
				OPENSSL_free(count_str);
				BN_free(count_bn);
			} else
				printf("0%s", opt_nulprint ? "" : "\n");
		} else {
#endif
			count = cmb_count(config, nitems);
			if (opt_silent)
				exit(EXIT_SUCCESS);
			if (errno) {
				err(EXIT_FAILURE, NULL);
				/* NOTREACHED */
			}
			printf("%"PRIu64"%s", count, opt_nulprint ? "" : "\n");
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
	if (opt_file || opt_range) {
		for (n = 0; n < nitems; n++)
			free(items[n]);
		free(items);
	} else if (opt_transform) {
		for (n = 0; n < nitems; n++) {
			memcpy(&xitem, &items[n], sizeof(char *));
			if (opt_range)
				free(xitem->cp);
			free(xitem);
		}
		free(items);
	}
	if (opt_find) {
		if (free_find)
			free(cmb_transform_find->cp);
		free(cmb_transform_find);
		if (cmb_transform_find_buf != NULL)
			free(cmb_transform_find_buf);
	}
	free(config);

	return (retval);
}

/*
 * Print short usage statement to stderr and exit with error status.
 */
static void
cmb_usage(void)
{
	fprintf(stderr, "usage: %s [options] [item ...]\n", pgm);
#define OPTFMT		"\t%-10s %s\n"
#define OPTFMT_1U	"\t%-10s %s%u%s\n"
	fprintf(stderr, "OPTIONS:\n");
	fprintf(stderr, OPTFMT, "-0",
	    "Read items terminated by NUL when given `-f'.");
	fprintf(stderr, OPTFMT, "-c num",
	    "Produce num combinations (default `0' for all).");
	fprintf(stderr, OPTFMT, "-D",
	    "Enable debugging information on stderr."
	);
	fprintf(stderr, OPTFMT, "-d text", "Item delimiter (default is ` ').");
	fprintf(stderr, OPTFMT, "-e", "Show empty set with no items.");
	fprintf(stderr, OPTFMT, "-F num",
	    "Find `-X op' results matching num.");
	fprintf(stderr, OPTFMT, "-f",
	    "Treat arguments as files to read items from; `-' for stdin.");
	fprintf(stderr, OPTFMT, "-i num",
	    "Skip the first num-1 combinations.");
	fprintf(stderr, OPTFMT, "-k size",
	    "Number or range (`min..max' or `min-max') of items.");
	fprintf(stderr, OPTFMT, "-N", "Show combination sequence numbers.");
	fprintf(stderr, OPTFMT, "-n num",
	    "Limit arguments taken from the command-line.");
	fprintf(stderr, OPTFMT, "-o",
	    "Disable OpenSSL (limits calculations to 64-bits).");
	fprintf(stderr, OPTFMT, "-P num",
	    "Set floating point precision when given `-X op'.");
	fprintf(stderr, OPTFMT, "-p text", "Prefix text for each line.");
	fprintf(stderr, OPTFMT, "-q",
	    "Quiet. Do not print items from set when given `-X op'.");
	fprintf(stderr, OPTFMT_1U, "-r",
	    "Treat arguments as ranges of up-to ", UINT_MAX, " items.");
	fprintf(stderr, OPTFMT, "-S", "Silent (for performance benchmarks).");
	fprintf(stderr, OPTFMT, "-s text", "Suffix text for each line.");
	fprintf(stderr, OPTFMT, "-t",
	    "Print number of combinations and exit.");
	fprintf(stderr, OPTFMT, "-v",
	    "Print version info to stdout and exit.");
	fprintf(stderr, OPTFMT, "-X op",
	    "Perform math on items where `op' is add, sub, div, or mul.");
	fprintf(stderr, OPTFMT, "-z",
	    "Print combinations NUL terminated (use with `xargs -0').");
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
parse_unum(const char *s, uint32_t *n)
{
	uint64_t ull;

	errno = 0;

	if (s == NULL || unumlen(s) != strlen(s)) {
		errno = EINVAL;
		return (FALSE);
	}

	ull = strtoull(optarg, (char **)NULL, 10);
	if (errno != 0)
		return (FALSE);
	else if (ull > UINT_MAX) {
		errno = ERANGE;
		return (FALSE);
	}

	*n = (uint32_t)ull;

	return (TRUE);
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

static uint32_t
range_char(uint32_t start, uint32_t stop, uint32_t idx, char *dst[])
{
	int len;
	uint32_t num;

	if (start <= stop) {
		for (num = start; num <= stop; num++) {
			len = snprintf(NULL, 0, "%u", num) + 1;
			if ((dst[idx] = malloc((unsigned long)len)) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			(void)sprintf(dst[idx], "%u", num);
			idx++;
		}
	} else {
		for (num = start; num >= stop; num--) {
			len = snprintf(NULL, 0, "%u", num) + 1;
			if ((dst[idx] = (char *)malloc((unsigned long)len)) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			(void)sprintf(dst[idx], "%u", num);
			idx++;
		}
	}

	return (idx);
}

static uint32_t
range_float(uint32_t start, uint32_t stop, uint32_t idx, char *dst[])
{
	int len;
	uint32_t num;
	size_t size;
	struct cmb_xitem *xitem;

	size = sizeof(struct cmb_xitem);
	if (start <= stop) {
		for (num = start; num <= stop; num++) {
			if ((xitem = malloc(size)) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			len = snprintf(NULL, 0, "%u", num) + 1;
			if ((xitem->cp = malloc((unsigned long)len)) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			(void)sprintf(xitem->cp, "%u", num);
			xitem->as.ld = (long double)num;
			dst[idx] = (char *)xitem;
			idx++;
		}
	} else {
		for (num = start; num >= stop; num--) {
			if ((xitem = malloc(size)) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			len = snprintf(NULL, 0, "%u", num) + 1;
			if ((xitem->cp = malloc((unsigned long)len)) == NULL) {
				errx(EXIT_FAILURE, "Out of memory?!");
				/* NOTREACHED */
			}
			(void)sprintf(xitem->cp, "%u", num);
			xitem->as.ld = (long double)num;
			dst[idx] = (char *)xitem;
			idx++;
		}
	}

	return (idx);
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
 * Transformation functions
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

/*
 * Find transformation functions
 */
static CMB_TRANSFORM_OP_FIND(*, cmb_mul_find);
static CMB_TRANSFORM_OP_FIND(/, cmb_div_find);
static CMB_TRANSFORM_OP_FIND(+, cmb_add_find);
static CMB_TRANSFORM_OP_FIND(-, cmb_sub_find);
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
static CMB_TRANSFORM_OP_FIND_BN(*, cmb_mul_find_bn);
static CMB_TRANSFORM_OP_FIND_BN(/, cmb_div_find_bn);
static CMB_TRANSFORM_OP_FIND_BN(+, cmb_add_find_bn);
static CMB_TRANSFORM_OP_FIND_BN(-, cmb_sub_find_bn);
#endif
