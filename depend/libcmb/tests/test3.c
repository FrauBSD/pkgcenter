/*-
 * Copyright (c) 2018-2026 Devin Teske <dteske@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <cmb.h>
#include <stdio.h>
#include <stdlib.h>

static int
afunc(struct cmb_config *config, uint64_t seq, uint32_t nitems, char *items[])
{
	int i;

	printf("\t");
	for (i = 0; i < nitems; i++)
		printf("%s%s", items[i], i < (nitems - 1) ? " " : "");
	printf("\n");

	return (0);
}

int
main(void)
{
	int nitems = 4;
	char *items[] = {"a", "b", "c", "d"};
	static struct cmb_config config = {
		.size_min = 2,
		.size_max = 2,
		.action = afunc,
	};

	printf("Enumerating choose-2 from %u:\n", nitems);
	(void)cmb(&config, nitems, items);

	return (EXIT_SUCCESS);
}
