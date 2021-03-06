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
__FBSDID("$FrauBSD: pkgcenter/depend/libcmb/tests/test1.c 2019-04-10 15:27:36 -0700 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <cmb.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

static int num_calls = 0;
static int
afunc(struct cmb_config *config, uint64_t seq, uint32_t nitems, char *items[])
{
	num_calls++;
	return (0);
}

int
main(void)
{
	uint32_t nitems = 4;
	int retval;
	uint64_t seq = 1;
	char *items[] = {"a", "b", "c", "d"};
	char itemstr[] = "a, b, c, d";
	static struct cmb_config config = {
		.options = CMB_OPT_NUMBERS,
		.delimiter = ",",
		.prefix = "\t[",
		.suffix = "]",
		.size_min = 2,
		.size_max = 3,
		.count = 6,
		.start = 4,
	};

	printf("config = {\n");
	printf("\t.options = 0x%08x,\n", config.options);
	printf("\t.delimiter = \"%s\",\n", config.delimiter);
	printf("\t.prefix = \"%s\",\n", config.prefix);
	printf("\t.suffix = \"%s\",\n", config.suffix);
	printf("\t.size_min = %u,\n", config.size_min);
	printf("\t.size_max = %u,\n", config.size_max);
	printf("\t.count = %"PRIu64",\n", config.count);
	printf("\t.start = %"PRIu64",\n", config.start);
	printf("}\n");

	printf("cmb_version(%u): %s\n", CMB_VERSION, cmb_version(CMB_VERSION));
	printf("cmb_version(%u): %s\n", CMB_VERSION_LONG,
	    cmb_version(CMB_VERSION_LONG));
	printf("size_min=%u size_max=%u\n", config.size_min, config.size_max);
	printf("cmb_count(config, %u) = %"PRIu64"\n", nitems,
	    cmb_count(&config, nitems));
	printf("cmb_print(config, %"PRIu64", %u, [%s]):\n", seq, nitems,
	    itemstr);
	retval = cmb_print(&config, seq, nitems, items);
	printf("\tRESULT: %i\n", retval);
	printf("cmb(config, %u, [%s]):\n", nitems, itemstr);
	printf("NOTE: { .start = %"PRIu64", .count = %"PRIu64" }\n",
	    config.start, config.count);
	retval = cmb(&config, nitems, items);
	printf("\tRESULT: %i\n", retval);

	/*
	 * Callbacks
	 */
	config.options = 0;
	config.action = afunc;
	printf("cmb_callback(config, %u, [%s], afunc):\n", nitems, itemstr);
	retval = cmb(&config, nitems, items);
	printf("\tnum_calls: %i\n", num_calls);
	printf("\tRESULT: %i\n", retval);

	return (EXIT_SUCCESS);
}
