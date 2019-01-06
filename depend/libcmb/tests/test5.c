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
__FBSDID("$FrauBSD: pkgcenter/depend/libcmb/tests/test5.c 2019-01-06 08:54:10 -0800 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <cmb.h>
#include <stdio.h>
#include <stdlib.h>

static int total = 0;

static int
afunc(struct cmb_config *config, uint64_t seq, uint32_t nitems, char *items[])
{
	return (total++);
}

int
main(void)
{
	static struct cmb_config config = {
		.action = afunc,
	};
	int nitems = 3;
	char *items[] = {"a", "b", "c"};

	printf("Testing non-zero callback return:\n");
	(void)cmb(&config, nitems, items);
	printf("%u of %lu callbacks executed\n",
	    total, cmb_count(&config, nitems));

	return (EXIT_SUCCESS);
}
