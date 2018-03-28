/*-
 * Copyright (c) 2002-2018 Devin Teske <dteske@FreeBSD.org>
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
 * $FrauBSD: depend/libcmb/cmb.h 2018-03-27 17:00:01 -0700 freebsdfrau $
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
#define HAVE_OPENSSL_BN_H 1
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

static uint8_t cmb_print_nul = FALSE;
static const char *cmb_print_delimiter = " ";
static const char *cmb_print_prefix = NULL;
static const char *cmb_print_suffix = NULL;

/*
 * Anatomy of config option to pass as cmb*() config argument
 */
struct cmb_config {
	uint8_t	nul_terminate;	/* Terminate combinations with ASCII NUL */
	char	*delimiter;	/* Item separator (default is " ") */
	char	*prefix;	/* Prefix for each combination */
	char	*suffix;	/* Suffix for each combination */
	uint64_t range_min;	/* Minimum number of elements in combination */
	uint64_t range_max;	/* Maximum number of elements in combination */

	uint64_t count;		/* Number of combinations */
	uint64_t start;		/* Starting combination */
#ifdef HAVE_OPENSSL_BN_H
	BIGNUM	*count_bn;	/* bn(3) number of combinations */
	BIGNUM	*start_bn;	/* bn(3) starting combination */
#endif

	/*
	 * Function pointer; action to perform for each combination (default is
	 * cmb_print()). If the return from action() is non-zero, cmb() will
	 * stop calculation. The cmb() return value is the first non-zero
	 * result from action(), zero otherwise.
	 */
	int (*action)(uint64_t nitems, char *items[]);
};

__BEGIN_DECLS
int		cmb(struct cmb_config *_config, uint64_t _nitems, char *_items[]);
int		cmb_print(uint64_t _nitems, char *_items[]);
uint64_t	cmb_count(struct cmb_config *_config, uint64_t _nitems);
#ifdef HAVE_OPENSSL_BN_H
int		cmb_bn(struct cmb_config *_config, uint64_t _nitems, char *_items[]);
BIGNUM *	cmb_count_bn(struct cmb_config *_config, uint64_t _nitems);
#endif
__END_DECLS

#endif /* !_CMB_H_ */
