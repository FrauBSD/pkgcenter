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
 * $FrauBSD: depend/libcmb/cmb.h 2018-03-24 14:41:30 -0700 freebsdfrau $
 * $FreeBSD$
 */

#ifndef _CMB_H_
#define _CMB_H_

#include <sys/types.h>

#include <stddef.h>

static const char *cmb_print_delimiter = " ";
static const char *cmb_print_prefix = NULL;
static const char *cmb_print_suffix = NULL;

/*
 * Anatomy of config option to pass as cmb() config argument
 */
struct cmb_config {
	char	*delimiter;	/* Item separator (default is " ") */
	char	*prefix;	/* Prefix for each combination */
	char	*suffix;	/* Suffix for each combination */
	uint	range_min;	/* Minimum number of elements in combination */
	uint	range_max;	/* Maximum number of elements in combination */

	/*
	 * Function pointer; action to perform for each combination (default is
	 * cmb_items_print()). If the return from action() is non-zero, cmb()
	 * will stop calculation. The cmb() return value is the first non-zero
	 * result from action(), zero otherwise.
	 */
	int (*action)(int nitems, char *items[]);
};

__BEGIN_DECLS
int	cmb(struct cmb_config *_config, int _nitems, char *_items[]);
uint	cmb_count(struct cmb_config *_config, int _nitems);
int	cmb_print(int _nitems, char *_items[]);
__END_DECLS

#endif /* !_CMB_H_ */
