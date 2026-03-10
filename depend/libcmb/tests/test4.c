/*-
 * Copyright (c) 2018-2026 Devin Teske <dteske@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <cmb.h>
#include <err.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHOICE 2
#define NITEMS 10000

static int dpv[2] = { 0, 0 };

static int
afunc(struct cmb_config *config, uint64_t seq, uint32_t nitems, char *items[])
{
	uint32_t i;

	for (i = 0; i < nitems; i++)
		printf("%s%s", items[i], i < (nitems - 1) ? " " : "");
	printf("\n");

	return (0);
}

int
main(void)
{
	uint32_t i;
	pid_t pid;
	int status;
	static struct cmb_config config = {
		.size_min = CHOICE,
		.size_max = CHOICE,
		.action = afunc,
	};
	char *items[NITEMS];
	char count_arg[23];

	pipe(dpv);

	if ((pid = fork()) == 0) { /* child */
		dup2(dpv[0], STDIN_FILENO);
		close(dpv[0]);
		close(dpv[1]);
		sprintf(count_arg, "%"PRIu64":c", cmb_count(&config, NITEMS));
		execlp("dpv", "dpv", "-l", count_arg, NULL);
		err(EXIT_FAILURE, "dpv");
		/* NOTREACHED */
	} 
	/* parent */

	dup2(dpv[1], STDOUT_FILENO);
	close(dpv[0]);
	close(dpv[1]);

	for (i = 0; i < NITEMS; i++) {
		items[i] = (char *)calloc(1, 11);
		sprintf(items[i], "%u", i);
	}
	(void)cmb(&config, NITEMS, items);

	fflush(stdout);
	close(STDOUT_FILENO);
	waitpid(pid, &status, 0);

	return (EXIT_SUCCESS);
}
