/*-
 * Copyright (c) 2018-2026 Devin Teske <dteske@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <cmb.h>
#include <inttypes.h>
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
	printf("%u of %"PRIu64" callbacks executed\n",
	    total, cmb_count(&config, nitems));

	return (EXIT_SUCCESS);
}
