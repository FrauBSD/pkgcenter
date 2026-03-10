/*-
 * Copyright (c) 2018-2026 Devin Teske <dteske@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <cmb.h>
#include <stdio.h>
#include <stdlib.h>

#define CHOICE 2
#define NITEMS 10000

static int total = 0;

static int
afunc(struct cmb_config *config, uint64_t seq, uint32_t nitems, char *items[])
{
	total++;
	return (0);
}

int
main(void)
{
	static struct cmb_config config = {
		.size_min = CHOICE,
		.size_max = CHOICE,
		.action = afunc,
	};
	char *items[NITEMS];

	printf("Silently enumerating choose-%u from %u:\n", CHOICE, NITEMS);
	(void)cmb(&config, NITEMS, items);

	return (EXIT_SUCCESS);
}
