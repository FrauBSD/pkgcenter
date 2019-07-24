/*-
 * Copyright (c) 2002-2019 Devin Teske <dteske@FreeBSD.org>
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
 * $FrauBSD: pkgcenter/depend/libcmb/cmb.h 2019-07-23 21:29:49 -0700 freebsdfrau $
 * $FreeBSD$
 */

#ifndef _CMB_H_
#define _CMB_H_

#include <sys/param.h>
#include <sys/types.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

#ifdef HAVE_OPENSSL_CRYPTO_H
#include <openssl/crypto.h>
#endif
#ifdef HAVE_OPENSSL_BN_H
#include <openssl/bn.h>
#ifndef OPENSSL_free
#define OPENSSL_free(x) (void)(x)
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef CMB_DEBUG
#define CMB_DEBUG FALSE
#endif

/*
 * Version constants for cmb_version(3)
 */
#define CMB_VERSION		0
#define CMB_VERSION_LONG	1

/*
 * Header version info
 */
#define CMB_H_VERSION_MAJOR	3
#define CMB_H_VERSION_MINOR	5
#define CMB_H_VERSION_PATCH	0

/*
 * Macros for cmb_config options bitmask
 */
#define CMB_OPT_DEBUG		0x01	/* Enable debugging */
#define CMB_OPT_NULPARSE	0x02	/* NUL delimit cmb_parse*() */
#define CMB_OPT_NULPRINT	0x04	/* NUL delimit cmb_print*() */
#define CMB_OPT_EMPTY		0x08	/* Show empty set with no items */
#define CMB_OPT_NUMBERS		0x10	/* Show combination sequence numbers */
#define CMB_OPT_RESERVED	0x20	/* Reserved for future use by cmb(3) */
#define CMB_OPT_OPTION1		0x40	/* Available (unused by cmb(3)) */
#define CMB_OPT_OPTION2		0x80	/* Available (unused by cmb(3)) */

/*
 * Macros for defining call-back functions/pointers
 */
#define CMB_ACTION(x) \
    int x(struct cmb_config *config, uint64_t seq, uint32_t nitems, \
        char *items[])
#ifdef HAVE_OPENSSL_BN_H
#define CMB_ACTION_BN(x) \
    int x(struct cmb_config *config, BIGNUM *seq, uint32_t nitems, \
        char *items[])
#endif

/*
 * Anatomy of config option to pass as cmb*() config argument
 */
struct cmb_config {
	uint8_t options;	/* CMB_OPT_* bitmask. Default 0 */
	char	*delimiter;	/* Item separator (default is " ") */
	char	*prefix;	/* Prefix for each combination */
	char	*suffix;	/* Suffix for each combination */
	uint32_t size_min;	/* Minimum number of elements in combination */
	uint32_t size_max;	/* Maximum number of elements in combination */

	uint64_t count;		/* Number of combinations */
	uint64_t start;		/* Starting combination */

	void *data;		/* Reserved for action callback */

	/*
	 * cmb(3) function callback; called for each combination (default is
	 * cmb_print()). If the return from action() is non-zero, cmb() will
	 * stop calculation. The cmb() return value is the first non-zero
	 * result from action(), zero otherwise.
	 */
	CMB_ACTION((*action));

#ifdef HAVE_OPENSSL_BN_H
	BIGNUM	*count_bn;	/* bn(3) number of combinations */
	BIGNUM	*start_bn;	/* bn(3) starting combination */

	/*
	 * cmb_bn(3) function callback; called for each combination (default is
	 * cmb_print_bn()). If the return from action_bn() is non-zero,
	 * cmb_bn() will stop calculation. The cmb_bn() return value is the
	 * first non-zero result from action_bn(), zero otherwise.
	 */
	CMB_ACTION_BN((*action_bn));
#endif
};

__BEGIN_DECLS
int		cmb(struct cmb_config *_config, uint32_t _nitems,
		    char *_items[]);
uint64_t	cmb_count(struct cmb_config *_config, uint32_t _nitems);
char **		cmb_parse(struct cmb_config *_config, int _fd,
		    uint32_t *_nitems, uint32_t _max);
char **		cmb_parse_file(struct cmb_config *_config, char *_path,
		    uint32_t *_nitems, uint32_t _max);
int		cmb_print(struct cmb_config *_config, uint64_t _seq,
		    uint32_t _nitems, char *_items[]);
const char *	cmb_version(int _type);
#ifdef HAVE_OPENSSL_BN_H
int		cmb_bn(struct cmb_config *_config, uint32_t _nitems,
		    char *_items[]);
BIGNUM *	cmb_count_bn(struct cmb_config *_config, uint32_t _nitems);
int		cmb_print_bn(struct cmb_config *_config, BIGNUM *_seq,
		    uint32_t _nitems, char *_items[]);
#endif

/* Inline functions */
static inline void cmb_print_seq(uint64_t seq) { printf("%"PRIu64" ", seq); }
#ifdef HAVE_OPENSSL_BN_H
static inline void cmb_print_seq_bn(BIGNUM *seq) { char *seq_str;
    printf("%s ", seq_str = BN_bn2dec(seq));
    OPENSSL_free(seq_str);
}
#endif /* HAVE_OPENSSL_BN_H */
__END_DECLS

/*
 * Transformations
 */

extern int cmb_transform_precision;
struct cmb_xitem {
	char *cp;			/* original item */
	union cmb_xitem_type {
		long double ld;		/* item as long double */
	} as;
};

#define CMB_TRANSFORM_EQ(eq, op, x, seqt, seqp) \
    int                                                                      \
    x(struct cmb_config *config, seqt, uint32_t nitems, char *items[])       \
    {                                                                        \
    	uint8_t show_numbers = FALSE;                                        \
    	uint32_t n;                                                          \
    	long double ld;                                                      \
    	long double total = 0;                                               \
    	const char *delimiter = " ";                                         \
    	const char *prefix = NULL;                                           \
    	const char *suffix = NULL;                                           \
    	struct cmb_xitem *xitem = NULL;                                      \
    	                                                                     \
    	if (config != NULL) {                                                \
    		if (config->delimiter != NULL)                               \
    			delimiter = config->delimiter;                       \
    		if ((config->options & CMB_OPT_NUMBERS) != 0)                \
    			show_numbers = TRUE;                                 \
    		prefix = config->prefix;                                     \
    		suffix = config->suffix;                                     \
    	}                                                                    \
    	if (!opt_silent) {                                                   \
    		if (show_numbers)                                            \
    			seqp(seq);                                           \
    		if (prefix != NULL && !opt_quiet)                            \
    			printf("%s", prefix);                                \
    	}                                                                    \
    	if (nitems > 0) {                                                    \
    		memcpy(&xitem, &items[0], sizeof(struct cmb_xitem *));       \
    		total = xitem->as.ld;                                        \
    		if (!opt_silent && !opt_quiet) {                             \
    			printf("%s", xitem->cp);                             \
    			if (nitems > 1)                                      \
    				printf("%s" #op "%s", delimiter, delimiter); \
    		}                                                            \
    	}                                                                    \
    	for (n = 1; n < nitems; n++) {                                       \
    		memcpy(&xitem, &items[n], sizeof(struct cmb_xitem *));       \
    		ld = xitem->as.ld;                                           \
    		total = eq;                                                  \
    		if (!opt_silent && !opt_quiet) {                             \
    			printf("%s", xitem->cp);                             \
    			if (n < nitems - 1)                                  \
    				printf("%s" #op "%s", delimiter, delimiter); \
    		}                                                            \
    	}                                                                    \
    	if (!opt_silent) {                                                   \
    		if (suffix != NULL && !opt_quiet)                            \
    			printf("%s", suffix);                                \
    		printf("%s%.*Lf\n", opt_quiet ? "" : " = ",                  \
    			cmb_transform_precision, total);                     \
    	}                                                                    \
    	return (0);                                                          \
    }

#define CMB_TRANSFORM_OP(op, x) \
	CMB_TRANSFORM_EQ(total op ld, op, x, uint64_t seq, cmb_print_seq)
#define CMB_TRANSFORM_FN(op, fn, x) \
	CMB_TRANSFORM_EQ(fn(total, ld), op, x, uint64_t seq, cmb_print_seq)

#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
#define CMB_TRANSFORM_OP_BN(op, x) \
	CMB_TRANSFORM_EQ(total op ld, op, x, BIGNUM *seq, cmb_print_seq_bn)
#define CMB_TRANSFORM_FN_BN(op, fn, x) \
	CMB_TRANSFORM_EQ(fn(total, ld), op, x, BIGNUM *seq, cmb_print_seq_bn)
#endif

extern struct cmb_xitem *cmb_transform_find;

#ifndef LDBL_EPSILON
#define LDBL_EPSILON (long double)(2e-105)
#endif

#define CMB_TRANSFORM_EQ_FIND(eq, op, x, seqt, seqp) \
    int                                                                      \
    x(struct cmb_config *config, seqt, uint32_t nitems, char *items[])       \
    {                                                                        \
    	uint8_t show_numbers = FALSE;                                        \
    	uint32_t n;                                                          \
    	long double ld;                                                      \
    	long double total = 0;                                               \
    	const char *delimiter = " ";                                         \
    	const char *prefix = NULL;                                           \
    	const char *suffix = NULL;                                           \
    	struct cmb_xitem *xitem = NULL;                                      \
    	                                                                     \
    	for (n = 1; n < nitems; n++) {                                       \
    		memcpy(&xitem, &items[n], sizeof(struct cmb_xitem *));       \
    		ld = xitem->as.ld;                                           \
    		total = eq;                                                  \
    	}                                                                    \
    	if (fabsl(total - cmb_transform_find->as.ld) <= LDBL_EPSILON *       \
    	    fmaxl(fabsl(total), fabsl(cmb_transform_find->as.ld)))           \
    		return (0);                                                  \
    	if (config != NULL) {                                                \
    		if (config->delimiter != NULL)                               \
    			delimiter = config->delimiter;                       \
    		if ((config->options & CMB_OPT_NUMBERS) != 0)                \
    			show_numbers = TRUE;                                 \
    		prefix = config->prefix;                                     \
    		suffix = config->suffix;                                     \
    	}                                                                    \
    	if (!opt_silent) {                                                   \
    		if (show_numbers)                                            \
    			seqp(seq);                                           \
    		if (prefix != NULL && !opt_quiet)                            \
    			printf("%s", prefix);                                \
    	}                                                                    \
    	if (nitems > 0) {                                                    \
    		memcpy(&xitem, &items[0], sizeof(struct cmb_xitem *));       \
    		total = xitem->as.ld;                                        \
    		if (!opt_silent && !opt_quiet) {                             \
    			printf("%s", xitem->cp);                             \
    			if (nitems > 1)                                      \
    				printf("%s" #op "%s", delimiter, delimiter); \
    		}                                                            \
    	}                                                                    \
    	for (n = 1; n < nitems; n++) {                                       \
    		if (!opt_silent && !opt_quiet) {                             \
    			printf("%s", xitem->cp);                             \
    			if (n < nitems - 1)                                  \
    				printf("%s" #op "%s", delimiter, delimiter); \
    		}                                                            \
    	}                                                                    \
    	if (!opt_silent) {                                                   \
    		if (suffix != NULL && !opt_quiet)                            \
    			printf("%s", suffix);                                \
    		printf("%s%.*Lf\n", opt_quiet ? "" : " = ",                  \
    			cmb_transform_precision, total);                     \
    	}                                                                    \
    	return (0);                                                          \
    }

#define CMB_TRANSFORM_OP_FIND(op, x) \
	CMB_TRANSFORM_EQ_FIND(total op ld, op, x, uint64_t seq, cmb_print_seq)
#define CMB_TRANSFORM_FN_FIND(op, fn, x) \
	CMB_TRANSFORM_EQ_FIND(fn(total, ld), op, x, uint64_t seq, \
	    cmb_print_seq)

#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
#define CMB_TRANSFORM_OP_FIND_BN(op, x) \
	CMB_TRANSFORM_EQ_FIND(total op ld, op, x, BIGNUM *seq, \
	    cmb_print_seq_bn)
#define CMB_TRANSFORM_FN_FIND_BN(op, fn, x) \
	CMB_TRANSFORM_EQ_FIND(fn(total, ld), op, x, BIGNUM *seq, \
	    cmb_print_seq_bn)
#endif

/*
 * Example transformations
 */

#if 0
CMB_TRANSFORM_OP(+, cmb_add);			/* creates cmb_add() */
CMB_TRANSFORM_FN(/, div, cmb_div);		/* creates cmb_div() */
#if defined(HAVE_LIBCRYPTO) && defined(HAVE_OPENSSL_BN_H)
CMB_TRANSFORM_OP_BN(+, cmb_add_bn);		/* creates cmb_add_bn() */
CMB_TRANSFORM_FN_BN(/, div, cmb_div_bn);	/* creates cmb_div_bn() */
#endif
#endif

#endif /* !_CMB_H_ */
