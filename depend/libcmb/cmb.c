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
 */

#include <sys/cdefs.h>
#ifdef __FBSDID
__FBSDID("$FrauBSD: depend/libcmb/cmb.c 2018-03-24 14:41:30 -0700 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <err.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmb.h"

/*
 * Takes pointer to `struct cmb_config' options and number of items. Returns
 * total number of combinations according to config options.
 */
uint cmb_count(struct cmb_config *config, int nitems)
{
	int nextset = 1;
	uint count = 0;
	uint curset;
	uint setinit = 1;
	uint setdone = nitems;

	if (config != NULL) {
		if (config->range_min == 0 && config->range_max == 0) {
			setinit = 1;
			setdone = nitems;
		} else {
			setinit = config->range_min;
			setdone = config->range_max;
		}
	}

	/* Adjust values to be non-zero (mathematical constraint) */
	if (setinit == 0) setinit = 1;
	if (setdone == 0) setdone = 1;

	/* Enforce limits so we don't run over bounds */
	if (setinit > (uint)nitems) setinit = nitems;
	if (setdone > (uint)nitems) setdone = nitems;

	/* Set the direction of flow (incrementing vs. decrementing) */
	if (setinit > setdone) nextset = -1;

	/*
	 * Loop over each `set' in the configured direction until we are done
	 */
	for (curset = setinit;
	    nextset > 0 ? curset <= setdone : curset >= setdone;
	    curset += nextset) {
		uint n;
		uint ncombos;
		uint nsubsets;
		uint64_t d;
		uint64_t z;

		/*
		 * Calculate number of subsets, based on the number of items in
		 * the current set we're working on.
		 */
		nsubsets = nitems - curset + 1;

		/*
		 * Calculate number of combinations based on number of subsets
		 */
		z = d = 1;
		for (n = 0; n < curset; n++) {
			z *= nsubsets + n;
			d *= n + 1;
		}
		ncombos = z / d;

		/*
		 * Add the number of combinations in this set to the total
		 */
		count += ncombos;
	}

	return (count);
}

/*
 * Takes pointer to `struct cmb_config' options, number of items, and array of
 * `char *'. Calculates combinations according to options and either prints
 * combinations to stdout (default) or runs `action' if passed-in as function
 * pointer member of `config' argument.
 */
int cmb(struct cmb_config *config, int nitems, char *items[])
{
	int nextset = 1;
	int retval = 0;
	uint curset;
	uint setinit = 0;
	uint setdone = nitems;
	uint setmax;
	char **curitems;
	uint *setnums;
	uint *setnums_backend;
	int (*action)(int nitems, char *items[]) = cmb_print;

	if (config != NULL) {
		if (config->action != NULL) action = config->action;
		if (config->delimiter != NULL)
			cmb_print_delimiter = config->delimiter;
		if (config->prefix != NULL) cmb_print_prefix = config->prefix;
		if (config->range_min == 0 && config->range_max == 0) {
			setinit = 1;
			setdone = nitems;
		} else {
			setinit = config->range_min;
			setdone = config->range_max;
		}
		if (config->suffix != NULL) cmb_print_suffix = config->suffix;
	}

	/* Adjust values to be non-zero (mathematical constraint) */
	if (setinit == 0) setinit = 1;
	if (setdone == 0) setdone = 1;

	/* Enforce limits so we don't run over bounds */
	if (setinit > (uint)nitems) setinit = nitems;
	if (setdone > (uint)nitems) setdone = nitems;

	/* Set the direction of flow (incrementing vs. decrementing) */
	if (setinit > setdone) nextset = -1;

	/*
	 * Allocate memory
	 */
	setmax = setdone > setinit ? setdone : setinit;
	if ((curitems = (char **)malloc(sizeof(char *) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	if ((setnums = (uint *)malloc(sizeof(uint) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	if ((setnums_backend = (uint *)malloc(sizeof(uint) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");

	/*
	 * Loop over each `set' in the configured direction until we are done
	 * NB: Each `set' can represent a single item or multiple items
	 */
	for (curset = setinit;
	    nextset > 0 ?  curset <= setdone : curset >= setdone;
	    curset += nextset) {
		uint combo;
		uint i;
		uint n;
		uint ncombos;
		uint nsubsets;
		uint setpos;
		uint setpos_backend;
		uint64_t d;
		uint64_t z;

		/*
		 * Calculate number of subsets, based on the number of items in
		 * the current set we're working on.
		 */
		nsubsets = nitems - curset + 1;

		/*
		 * Calculate number of combinations based on number of subsets
		 */
		z = d = 1;
		for (n = 0; n < curset; n++) {
			z *= nsubsets + n;
			d *= n + 1;
		}
		ncombos = z / d;

		/*
		 * Fill array with the initial positional arguments
		 */
		for (n = 0; n < curset; n++) curitems[n] = items[n];

		/*
		 * Produce results with the first set of items
		 */
		if ((retval = action(curset, curitems)) != 0) goto cmb_return;

		/*
		 * Prefill two arrays used for matrix calculations.
		 *
		 * The first array (setnums) is a linear sequence starting at
		 * one (1) and ending at N (where N is the same integer as the
		 * current set we're operating on). For example, if we are
		 * currently on a set-of-2, setnums is 1, 2.
		 *
		 * The second array (setnums_backend) is a linear sequence
		 * starting at nitems-N and ending at nitems (again, N is the
		 * same integer as the current set we are operating on; nitems
		 * is the total number of items). For example, if we are
		 * operating on a set-of-2, and nitems is 8, setnums_backend is
		 * set to 7, 8.
		 */
		i = 0;
		for (n = 0; n < curset; n++) setnums[n] = n;
		for (n = curset; n > 0; n--) setnums_backend[i++] = nitems - n;

		/*
		 * Process remaining self-similar combinations in the set
		 */
		for (combo = 1; combo < ncombos; combo++) {
			uint seed;
			uint setnums_last;

			/*
			 * Using self-similarity (matrix) theorem, determine
			 * (by comparing the [sliding] setnums to the stored
			 * setnums_backend) the number of arguments that remain
			 * available for shifting into a new setnums value
			 * (for later mapping into curitems).
			 *
			 * In essence, determine when setnums has slid into
			 * setnums_backend in which case we can mathematically
			 * use the last item to find the next-new item.
			 */
			for (n = curset; n > 0; n--) {
				setpos = setnums[n - 1];
				setpos_backend = setnums_backend[n - 1];
				/*
				 * If setpos is equal to or greater than
				 * setpos_backend then we keep iterating over
				 * the current set's list of argument positions
				 * until otherwise; each time incrementing the
				 * amount of numbers we must produce from
				 * formulae rather than stored position.
				 */
				setnums_last = n - 1;
				if (setpos < setpos_backend) break;
			}

			/*
			 * The next few stanzas are dedicated to rebuilding the
			 * setnums array for mapping positional items
			 * [immediately following] into curitems.
			 */

			/*
			 * Get the generator number used to populate unknown
			 * positions in the matrix (using self-similarity).
			 */
			seed = setnums[setnums_last];

			/*
			 * Use the generator number to populate any position
			 * numbers that weren't carried over from previous
			 * combination run -- using self-similarity theorem.
			 */
			for (n = setnums_last; n <= curset; n++)
				setnums[n] = seed + n - setnums_last + 1;

			/*
			 * Now map the new setnums into values stored in items
			 */
			for (n = 0; n < curset; n++)
				curitems[n] = items[setnums[n]];

			/*
			 * Produce results with this set of items
			 */
			if ((retval = action(curset, curitems)) != 0)
				goto cmb_return;

		} /* for combo */

	} /* for curset */

cmb_return:
	free(curitems);
	free(setnums);
	free(setnums_backend);
	return (retval);
}

int
cmb_print(int nitems, char *items[])
{
	int n;

	if (cmb_print_prefix != NULL) printf("%s", cmb_print_prefix);
	for (n = 0; n < nitems; n++) {
		printf("%s", items[n]);
		if (n < nitems - 1 && cmb_print_delimiter != NULL)
			printf("%s", cmb_print_delimiter);
	}
	if (cmb_print_suffix != NULL) printf("%s", cmb_print_suffix);
	printf("\n");

	return (0);
}
