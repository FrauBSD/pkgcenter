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
__FBSDID("$FrauBSD: pkgcenter/depend/libcmb/tests/test4.c 2019-01-19 16:48:37 -0800 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

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

	close(STDOUT_FILENO);
	waitpid(pid, &status, 0);

	return (EXIT_SUCCESS);
}
