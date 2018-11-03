/*-
 * Copyright (c) 2002-2018 Devin Teske <dteske@FreeBSD.org>
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
__FBSDID("$FrauBSD: pkgcenter/depend/libcmb/cmb.c 2018-11-03 12:15:22 -0700 freebsdfrau $");
__FBSDID("$FreeBSD$");
#endif

#include <err.h>
#include <errno.h>
#include <limits.h>
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
uint64_t
cmb_count(struct cmb_config *config, uint32_t nitems)
{
	uint8_t show_empty = FALSE;
	int8_t nextset = 1;
	uint32_t curset;
	uint32_t i = nitems;
	uint32_t k;
	uint32_t p;
	uint32_t setdone = nitems;
	uint32_t setinit = 1;
	uint64_t count = 0;
	long double z = 1;
	uint64_t ncombos;

	errno = 0;
	if (nitems == 0)
		return (0);

	/* Process config options */
	if (config != NULL) {
		show_empty = config->show_empty;
		if (config->size_min != 0 || config->size_max != 0) {
			setinit = config->size_min;
			setdone = config->size_max;
		}
	}

	/* Adjust values to be non-zero (mathematical constraint) */
	if (setinit == 0)
		setinit = 1;
	if (setdone == 0)
		setdone = 1;

	/* Return zero if the request is out of range */
	if (setinit > nitems && setdone > nitems)
		return (0);

	/* Enforce limits so we don't run over bounds */
	if (setinit > nitems)
		setinit = nitems;
	if (setdone > nitems)
		setdone = nitems;

	/* Check for integer overflow */
	if ((setinit > setdone && setinit - setdone >= 64) ||
	    (setinit < setdone && setdone - setinit >= 64)) {
		errno = ERANGE;
		return (0);
	}

	/* If entire set is requested, return 2^N[-1] */
	if ((setinit == 1 && setdone == nitems) ||
	    (setinit == nitems && setdone == 1)) {
		if (show_empty) {
			if (nitems >= 64) {
				errno = ERANGE;
				return (0);
			}
			return (1 << nitems);
		} else
			return (ULLONG_MAX >> (64 - nitems));
	}

	/* Set the direction of flow (incrementing vs. decrementing) */
	if (setinit > setdone)
		nextset = -1;

	/*
	 * Loop over each `set' in the configured direction until we are done
	 */
	if (show_empty)
		count++;
	p = nextset > 0 ? setinit - 1 : setinit;
	for (k = 1; k <= p; k++)
		z = (z * i--) / k;
	for (curset = setinit;
	    nextset > 0 ? curset <= setdone : curset >= setdone;
	    curset += nextset)
	{
		/* Calculate number of combinations (incrementing) */
		if (nextset > 0)
			z = (z * i--) / k++;

		/* Add number of combinations in this set to total */
		if ((ncombos = z) == 0) {
			errno = ERANGE;
			return (0);
		}
		if (ncombos > ULLONG_MAX - count) {
			errno = ERANGE;
			return (0);
		}
		count += ncombos;

		/* Calculate number of combinations (decrementing) */
		if (nextset < 0)
			z = (z * --k) / ++i;
	}

	return (count);
}

/*
 * Takes pointer to `struct cmb_config' options, number of items, and array of
 * `char *' items. Calculates combinations according to options and either
 * prints combinations to stdout (default) or runs `action' if passed-in as
 * function pointer member of `config' argument.
 */
int
cmb(struct cmb_config *config, uint32_t nitems, char *items[])
{
	uint8_t docount = FALSE;
	uint8_t doseek = FALSE;
	uint8_t show_empty = FALSE;
	int8_t nextset = 1;
	int retval = 0;
	uint32_t curset;
	uint32_t i = nitems;
	uint32_t k;
	uint32_t n;
	uint32_t p;
	uint32_t seed;
	uint32_t setdone = nitems;
	uint32_t setinit = 1;
	uint32_t setmax;
	uint32_t setnums_last;
	uint32_t setpos;
	uint32_t setpos_backend;
	uint64_t combo;
	uint64_t count = 0;
	uint64_t ncombos;
	uint64_t seek = 0;
	long double z = 1;
	char **curitems;
	uint32_t *setnums;
	uint32_t *setnums_backend;
	int (*action)(struct cmb_config *config, uint32_t nitems,
	    char *items[]) = cmb_print;

	if (nitems == 0)
		return (0);
	else if (cmb_count(config, nitems) == 0)
		return (errno);

	/* Process config options */
	if (config != NULL) {
		if (config->action != NULL)
			action = config->action;
		if (config->count != 0) {
			docount = TRUE;
			count = config->count;
		}
		show_empty = config->show_empty;
		if (config->size_min != 0 || config->size_max != 0) {
			setinit = config->size_min;
			setdone = config->size_max;
		}
		if (config->start > 1) {
			doseek = TRUE;
			seek = config->start;
		}
	}

	/* Adjust values to be non-zero (mathematical constraint) */
	if (setinit == 0)
		setinit = 1;
	if (setdone == 0)
		setdone = 1;

	/* Enforce limits so we don't run over bounds */
	if (setinit > nitems)
		setinit = nitems;
	if (setdone > nitems)
		setdone = nitems;

	/* Set the direction of flow (incrementing vs. decrementing) */
	if (setinit > setdone)
		nextset = -1;

	/* Allocate memory */
	setmax = setdone > setinit ? setdone : setinit;
	if ((curitems = (char **)malloc(sizeof(char *) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	if ((setnums = (uint32_t *)malloc(sizeof(uint32_t) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	if ((setnums_backend =
	    (uint32_t *)malloc(sizeof(uint32_t) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");

	/* Show the empty set consisting of a single combination of no-items */
	if (nextset > 0 && show_empty) {
		if (!doseek) {
			retval = action(config, 0, NULL);
			if (retval != 0)
				goto cmb_return;
			if (docount && --count == 0)
				goto cmb_return;
		} else {
			seek--;
			if (seek == 1)
				doseek = FALSE;
		}
	}

	/*
	 * Loop over each `set' in the configured direction until we are done.
	 * NB: Each `set' can represent a single item or multiple items.
	 */
	p = nextset > 0 ? setinit - 1 : setinit;
	for (k = 1; k <= p; k++)
		z = (z * i--) / k;
	for (curset = setinit;
	    nextset > 0 ? curset <= setdone : curset >= setdone;
	    curset += nextset)
	{
		/* Calculate number of combinations (incrementing) */
		if (nextset > 0)
			z = (z * i--) / k++;

		/* Cast number of combinations in set to integer */
		if ((ncombos = z) == 0)
			return (errno = ERANGE);

		/* Jump to next set if requested start is beyond this one */
		if (doseek) {
			if (seek > ncombos) {
				seek -= ncombos;
				if (nextset < 0)
					z = (z * --k) / ++i;
				continue;
			} else if (seek == 1) {
				doseek = FALSE;
			}
		}

		/* Fill array with the initial positional arguments */
		for (n = 0; n < curset; n++)
			curitems[n] = items[n];

		/* Produce results with the first set of items */
		if (!doseek) {
			retval = action(config, curset, curitems);
			if (retval != 0)
				break;
			if (docount && --count == 0)
				break;
		}

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
		p = 0;
		for (n = 0; n < curset; n++)
			setnums[n] = n;
		for (n = curset; n > 0; n--)
			setnums_backend[p++] = nitems - n;

		/*
		 * Process remaining self-similar combinations in the set.
		 */
		for (combo = 1; combo < ncombos; combo++) {
			setnums_last = curset;

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
				if (setpos < setpos_backend)
					break;
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

			/* Now map new setnums into values stored in items */
			for (n = 0; n < curset; n++)
				curitems[n] = items[setnums[n]];

			/* Produce results with this set of items */
			if (doseek) {
				seek--;
				if (seek == 1)
					doseek = FALSE;
			}
			if (!doseek || seek == 1) {
				doseek = FALSE;
				retval = action(config, curset, curitems);
				if (retval != 0)
					goto cmb_return;
				if (docount && --count == 0)
					goto cmb_return;
			}

		} /* for combo */

		/* Calculate number of combinations (decrementing) */
		if (nextset < 0)
			z = (z * --k) / ++i;

	} /* for curset */

	/* Show the empty set consisting of a single combination of no-items */
	if (nextset < 0 && show_empty) {
		if ((!doseek || seek == 1) && (!docount || count > 0))
			retval = action(config, 0, NULL);
	}

cmb_return:
	free(curitems);
	free(setnums);
	free(setnums_backend);

	return (retval);
}

int
cmb_print(struct cmb_config *config, uint32_t nitems, char *items[])
{
	uint8_t cmb_print_nul = FALSE;
	uint32_t n;
	const char *cmb_print_delimiter = " ";
	const char *cmb_print_prefix = NULL;
	const char *cmb_print_suffix = NULL;

	/* Process config options */
	if (config != NULL) {
		if (config->delimiter != NULL)
			cmb_print_delimiter = config->delimiter;
		cmb_print_nul = config->nul_terminate;
		cmb_print_prefix = config->prefix;
		cmb_print_suffix = config->suffix;
	}

	if (cmb_print_prefix != NULL)
		printf("%s", cmb_print_prefix);
	for (n = 0; n < nitems; n++) {
		printf("%s", items[n]);
		if (n < nitems - 1)
			printf("%s", cmb_print_delimiter);
	}
	if (cmb_print_suffix != NULL)
		printf("%s", cmb_print_suffix);
	if (cmb_print_nul)
		printf("%c", 0);
	else
		printf("\n");

	return (0);
}

#ifdef HAVE_OPENSSL_BN_H
/*
 * Takes pointer to `struct cmb_config' options and number of items. Returns
 * total number of combinations according to config options. Numbers formatted
 * as openssl bn(3) BIGNUM type.
 */
BIGNUM *
cmb_count_bn(struct cmb_config *config, uint32_t nitems)
{
	uint8_t show_empty = FALSE;
	int8_t nextset = 1;
	uint32_t curset;
	uint32_t i = nitems;
	uint32_t k;
	uint32_t p;
	uint32_t setdone = nitems;
	uint32_t setinit = 1;
	BIGNUM *count = NULL;
	BIGNUM *ncombos = NULL;

	if (nitems == 0)
		return (NULL);

	/* Process config options */
	if (config != NULL) {
		show_empty = config->show_empty;
		if (config->size_min != 0 || config->size_max != 0) {
			setinit = config->size_min;
			setdone = config->size_max;
		}
	}

	/* Adjust values to be non-zero (mathematical constraint) */
	if (setinit == 0)
		setinit = 1;
	if (setdone == 0)
		setdone = 1;

	/* Return NULL if the request is out of range */
	if (setinit > nitems && setdone > nitems)
		return (NULL);

	/* Enforce limits so we don't run over bounds */
	if (setinit > nitems)
		setinit = nitems;
	if (setdone > nitems)
		setdone = nitems;

	/* Set the direction of flow (incrementing vs decrementing) */
	if (setinit > setdone)
		nextset = -1;

	/* Initialize count */
	if ((count = BN_new()) == NULL)
		return (NULL);
	if (!BN_zero(count))
		goto cmb_count_bn_return;

	/* If entire set is requested, return 2^N[-1] */
	if ((setinit == 1 && setdone == nitems) ||
	    (setinit == nitems && setdone == 1)) {
		if (show_empty) {
			BN_lshift(count, BN_value_one(), nitems);
			goto cmb_count_bn_return;
		} else {
			if (BN_lshift(count, BN_value_one(), nitems))
				BN_sub_word(count, 1);
			goto cmb_count_bn_return;
		}
	}

	/* Allocate memory */
	if ((ncombos = BN_new()) == NULL)
		goto cmb_count_bn_return;
	if (!BN_one(ncombos))
		goto cmb_count_bn_return;

	/*
	 * Loop over each `set' in the configured direction until we are done
	 */
	p = nextset > 0 ? setinit - 1 : setinit;
	for (k = 1; k <= p; k++) {
		if (!BN_mul_word(ncombos, i--))
			goto cmb_count_bn_return;
		if (BN_div_word(ncombos, k) == (BN_ULONG)-1)
			goto cmb_count_bn_return;
	}
	for (curset = setinit;
	    nextset > 0 ? curset <= setdone : curset >= setdone;
	    curset += nextset)
	{
		/* Calculate number of combinations (incrementing) */
		if (nextset > 0) {
			if (!BN_mul_word(ncombos, i--))
				break;
			if (BN_div_word(ncombos, k++) == (BN_ULONG)-1)
				break;
		}

		/* Add number of combinations in this set to total */
		if (!BN_add(count, count, ncombos))
			break;

		/* Calculate number of combinations (decrementing) */
		if (nextset < 0) {
			if (!BN_mul_word(ncombos, --k))
				break;
			if (BN_div_word(ncombos, ++i) == (BN_ULONG)-1)
				break;
		}
	}

cmb_count_bn_return:
	BN_free(ncombos);

	return (count);
}

/*
 * Takes pointer to `struct cmb_config' options, number of items, and array
 * of `char *' items. Calculates combinations according to options and either
 * prints combinations to stdout (default) or runs `action' if passed-in as
 * function pointer member of `config' argument. Numbers formatted as openssl
 * bn(3) BIGNUM type.
 */
int
cmb_bn(struct cmb_config *config, uint32_t nitems, char *items[])
{
	uint8_t docount = FALSE;
	uint8_t doseek = FALSE;
	uint8_t show_empty = FALSE;
	int8_t nextset = 1;
	int retval = 0;
	uint32_t curset;
	uint32_t i = nitems;
	uint32_t k;
	uint32_t n;
	uint32_t p;
	uint32_t seed;
	uint32_t setdone = nitems;
	uint32_t setinit = 1;
	uint32_t setmax;
	uint32_t setnums_last;
	uint32_t setpos;
	uint32_t setpos_backend;
	char **curitems;
	uint32_t *setnums;
	uint32_t *setnums_backend;
	BIGNUM *combo = NULL;
	BIGNUM *count = NULL;
	BIGNUM *ncombos = NULL;
	BIGNUM *seek = NULL;
	int (*action)(struct cmb_config *config, uint32_t nitems,
		char *items[]) = cmb_print;

	if (nitems == 0)
		return (0);

	/* Process config options */
	if (config != NULL) {
		if (config->action != NULL)
			action = config->action;
		if (config->count_bn != NULL &&
		    !BN_is_negative(config->count_bn) &&
		    !BN_is_zero(config->count_bn))
		{
			docount = TRUE;
			if ((count = BN_dup(config->count_bn)) == NULL)
				goto cmb_bn_return;
		}
		show_empty = config->show_empty;
		if (config->size_min != 0 || config->size_max != 0) {
			setinit = config->size_min;
			setdone = config->size_max;
		}
		if (config->start_bn != NULL &&
		    !BN_is_negative(config->start_bn) &&
		    !BN_is_zero(config->start_bn) &&
		    !BN_is_one(config->start_bn))
		{
			doseek = TRUE;
			if ((seek = BN_dup(config->start_bn)) == NULL)
				goto cmb_bn_return;
		}
	}

	/* Adjust values to be non-zero (mathematical constraint) */
	if (setinit == 0)
		setinit = 1;
	if (setdone == 0)
		setdone = 1;

	/* Enforce limits so we don't run over bounds */
	if (setinit > nitems)
		setinit = nitems;
	if (setdone > nitems)
		setdone = nitems;

	/* Set the direction of flow (incrementing vs. decrementing) */
	if (setinit > setdone)
		nextset = -1;

	/* Allocate memory */
	if ((combo = BN_new()) == NULL)
		goto cmb_bn_return;
	if ((ncombos = BN_new()) == NULL)
		goto cmb_bn_return;
	if (!BN_one(ncombos))
		goto cmb_bn_return;
	setmax = setdone > setinit ? setdone : setinit;
	if ((curitems = (char **)malloc(sizeof(char *) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	if ((setnums = (uint32_t *)malloc(sizeof(uint32_t) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");
	if ((setnums_backend =
	    (uint32_t *)malloc(sizeof(uint32_t) * setmax)) == NULL)
		errx(EXIT_FAILURE, "Out of memory?!");

	/* Show the empty set consisting of a single combination of no-items */
	if (nextset > 0 && show_empty) {
		if (!doseek) {
			retval = action(config, 0, NULL);
			if (retval != 0)
				goto cmb_bn_return;
			if (docount) {
				if (!BN_sub_word(count, 1))
					goto cmb_bn_return;
				if (BN_is_zero(count))
					goto cmb_bn_return;
			}
		} else {
			if (!BN_sub_word(seek, 1))
				goto cmb_bn_return;
			if (BN_is_one(seek))
				doseek = FALSE;
		}
	}

	/*
	 * Loop over each `set' in the configured direction until we are done.
	 * NB: Each `set' can represent a single item or multiple items.
	 */
	p = nextset > 0 ? setinit - 1 : setinit;
	for (k = 1; k <= p; k++) {
		if (!BN_mul_word(ncombos, i--))
			goto cmb_bn_return;
		if (BN_div_word(ncombos, k) == (BN_ULONG)-1)
			goto cmb_bn_return;
	}
	for (curset = setinit;
	    nextset > 0 ? curset <= setdone : curset >= setdone;
	    curset += nextset)
	{
		/* Calculate number of combinations (incrementing) */
		if (nextset > 0) {
			if (!BN_mul_word(ncombos, i--))
				break;
			if (BN_div_word(ncombos, k++) == (BN_ULONG)-1)
				break;
		}

		/* Jump to next set if requested start is beyond this one */
		if (doseek) {
			if (BN_ucmp(seek, ncombos) > 0) {
				if (!BN_sub(seek, seek, ncombos))
					break;
				if (nextset < 0) {
					if (!BN_mul_word(ncombos, --k))
						break;
					if (BN_div_word(ncombos, ++i) ==
					    (BN_ULONG)-1)
						break;
				}
				continue;
			} else if (BN_is_one(seek)) {
				doseek = FALSE;
			}
		}

		/* Fill array with the initial positional arguments */
		for (n = 0; n < curset; n++)
			curitems[n] = items[n];

		/* Produce results with the first set of items */
		if (!doseek) {
			retval = action(config, curset, curitems);
			if (retval != 0)
				break;
			if (docount) {
				if (!BN_sub_word(count, 1))
					break;
				if (BN_is_zero(count))
					break;
			}
		}

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
		p = 0;
		for (n = 0; n < curset; n++)
			setnums[n] = n;
		for (n = curset; n > 0; n--)
			setnums_backend[p++] = nitems - n;

		/*
		 * Process remaining self-similar combinations in the set.
		 */
		if (!BN_one(combo))
			break;
		for (; BN_ucmp(combo, ncombos) < 0; ) {
			setnums_last = curset;

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
				if (setpos < setpos_backend)
					break;
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

			/* Now map new setnums into values stored in items */
			for (n = 0; n < curset; n++)
				curitems[n] = items[setnums[n]];

			/* Produce results with this set of items */
			if (doseek) {
				if (!BN_sub_word(seek, 1))
					goto cmb_bn_return;
				if (BN_is_one(seek))
					doseek = FALSE;
			}
			if (!doseek || BN_is_one(seek)) {
				doseek = FALSE;
				retval = action(config, curset, curitems);
				if (retval != 0)
					goto cmb_bn_return;
				if (docount) {
					if (!BN_sub_word(count, 1))
						goto cmb_bn_return;
					if (BN_is_zero(count))
						goto cmb_bn_return;
				}
			}

			if (!BN_add_word(combo, 1))
				goto cmb_bn_return;

		} /* for combo */

		/* Calculate number of combinations (decrementing) */
		if (nextset < 0) {
			if (!BN_mul_word(ncombos, --k))
				break;
			if (BN_div_word(ncombos, ++i) == (BN_ULONG)-1)
				break;
		}

	} /* for curset */

	/* Show the empty set consisting of a single combination of no-items */
	if (nextset < 0 && show_empty) {
		if ((!doseek || BN_is_one(seek)) &&
		    (!docount || !BN_is_zero(count))) {
			retval = action(config, 0, NULL);
		}
	}

cmb_bn_return:
	BN_free(combo);
	BN_free(count);
	BN_free(ncombos);
	BN_free(seek);

	return (retval);
}
#endif /* HAVE_OPENSSL_BN_H */
