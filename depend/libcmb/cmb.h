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
 * $FrauBSD: pkgcenter/depend/libcmb/cmb.h 2019-01-10 12:54:37 -0800 freebsdfrau $
 * $FreeBSD$
 */

#ifndef _CMB_H_
#define _CMB_H_

#include <sys/param.h>
#include <sys/types.h>

#include <stddef.h>
#include <stdint.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#elif defined(__FreeBSD_version)
#ifdef HAVE_LIBCRYPTO
#define HAVE_OPENSSL_BN_H 1
#else
#undef HAVE_OPENSSL_BN_H
#endif
#endif

#ifdef HAVE_OPENSSL_BN_H
#include <openssl/bn.h>
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
 * Anatomy of config option to pass as cmb*() config argument
 */
struct cmb_config {
	uint8_t debug;		/* Enable debugging if non-zero */
	uint8_t	nul_terminate;	/* Terminate combinations with ASCII NUL */
	uint8_t show_empty;	/* Show empty set with no items */
	uint8_t show_numbers;	/* Show combination sequence numbers */
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
	int (*action)(struct cmb_config *config, uint64_t seq, uint32_t nitems,
	    char *items[]);

#ifdef HAVE_OPENSSL_BN_H
	BIGNUM	*count_bn;	/* bn(3) number of combinations */
	BIGNUM	*start_bn;	/* bn(3) starting combination */

	/*
	 * cmb_bn(3) function callback; called for each combination (default is
	 * cmb_print_bn()). If the return from action_bn() is non-zero,
	 * cmb_bn() will stop calculation. The cmb_bn() return value is the
	 * first non-zero result from action_bn(), zero otherwise.
	 */
	int (*action_bn)(struct cmb_config *config, BIGNUM *seq,
	    uint32_t nitems, char *items[]);
#endif
};

__BEGIN_DECLS
int		cmb(struct cmb_config *_config, uint32_t _nitems, char *_items[]);
uint64_t	cmb_count(struct cmb_config *_config, uint32_t _nitems);
char **		cmb_parse(struct cmb_config *_config, int _fd, uint32_t *_nitems, uint32_t _max);
char **		cmb_parse_file(struct cmb_config *_config, char *_path, uint32_t *_nitems, uint32_t _max);
int		cmb_print(struct cmb_config *_config, uint64_t _seq, uint32_t _nitems, char *_items[]);
const char *	cmb_version(int _type);
#ifdef HAVE_OPENSSL_BN_H
int		cmb_bn(struct cmb_config *_config, uint32_t _nitems, char *_items[]);
BIGNUM *	cmb_count_bn(struct cmb_config *_config, uint32_t _nitems);
int		cmb_print_bn(struct cmb_config *_config, BIGNUM *_seq, uint32_t _nitems, char *_items[]);
#endif
__END_DECLS

#endif /* !_CMB_H_ */
