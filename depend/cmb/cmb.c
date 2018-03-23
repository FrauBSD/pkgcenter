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
 * $FrauBSD: depend/cmb/cmb.c 2018-03-23 12:11:21 -0700 freebsdfrau $
 * $FreeBSD$
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmb.h"

#define CMB_COUNT 0

#if CMB_COUNT
int
main(int argc, char *argv[]__attribute__((unused)))
{
	int nitems = argc - 1;
	struct cmb_config *config = NULL;

	config = (struct cmb_config *)malloc(sizeof(struct cmb_config));
	bzero(config, sizeof(struct cmb_config));

	printf("%u\n", cmb_count(config, nitems));
	return (0);
}
#else
int
main(int argc, char *argv[])
{
	int nitems = argc - 1;
	char **items = nitems > 0 ? &argv[1] : NULL;
	struct cmb_config *config = NULL;

	config = (struct cmb_config *)malloc(sizeof(struct cmb_config));
	bzero(config, sizeof(struct cmb_config));

	cmb(config, nitems, items);
	return (0);
}
#endif
