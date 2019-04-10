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
__FBSDID("$FrauBSD: pkgcenter/depend/libcmb/perl/Cmb.xs 2019-04-10 14:13:36 -0700 freebsdfrau $");
#endif

#include <cmb.h>

#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"
#include "const-c.inc"

int
g_callback(struct cmb_config *config, uint64_t seq, uint32_t nitems,
    char *items[])
{
	uint32_t i;
	int result = 0;
	dTHX;
	dSP;
	dMULTICALL;
	HV *stash;
	GV *gv;
	U8 gimme = G_SCALAR;
	CV *cv;
	SV *perlfunc;
	AV *perlargs;

#ifdef __linux__
	(void)newsp;
#endif

	perlfunc = (SV *)((void **)config->data)[0];
	perlargs = (AV *)((void **)config->data)[1];

	cv = sv_2cv(perlfunc, &stash, &gv, 0);
	PUSH_MULTICALL(cv);

	av_clear(perlargs);
	if ((config->options & CMB_OPT_NUMBERS) != 0)
		av_push(perlargs, (SV *)newSViv(seq));
	for (i = 1; i <= nitems; i++) {
		av_push(perlargs, (SV *)items[i-1]);
		SvREFCNT_inc((SV *)items[i-1]);
	}

	{
		GvAV(PL_defgv) = perlargs;
		MULTICALL;
	}

	result = SvIV(*PL_stack_sp);
	POP_MULTICALL;

	return (result);
}

void
_config(struct cmb_config *c, SV *hash)
{
	HV *config = (HV *)SvRV(hash);
	HE *entry;
	char *k;
	SV *v;
	I32 len;
	dTHX;

	(void)hv_iterinit(config);
	while ((entry = hv_iternext(config))) {
		k = hv_iterkey(entry, &len);
		v = hv_iterval(config, entry);
		if (strEQ(k, "count")) {
			c->count = SvIV(v);
		} else if (strEQ(k, "data")) {
			c->data = v;
		} else if (strEQ(k, "debug")) {
			if (SvIV(v) == 0) {
				c->options &= ~CMB_OPT_DEBUG;
			} else {
				c->options |= CMB_OPT_DEBUG;
			}
		} else if (strEQ(k, "delimiter")) {
			c->delimiter = SvPV(v, PL_na);
		} else if (strEQ(k, "nul_terminate")) {
			if (SvIV(v) == 0) {
				c->options &= ~CMB_OPT_NULPRINT;
			} else {
				c->options |= CMB_OPT_NULPRINT;
			}
		} else if (strEQ(k, "show_empty")) {
			if (SvIV(v) == 0) {
				c->options &= ~CMB_OPT_EMPTY;
			} else {
				c->options |= CMB_OPT_EMPTY;
			}
		} else if (strEQ(k, "show_numbers")) {
			if (SvIV(v) == 0) {
				c->options &= ~CMB_OPT_NUMBERS;
			} else {
				c->options |= CMB_OPT_NUMBERS;
			}
		} else if (strEQ(k, "prefix")) {
			c->prefix = SvPV(v, PL_na);
		} else if (strEQ(k, "size_max")) {
			c->size_max = SvIV(v);
		} else if (strEQ(k, "size_min")) {
			c->size_min = SvIV(v);
		} else if (strEQ(k, "start")) {
			c->start = SvIV(v);
		} else if (strEQ(k, "suffix")) {
			c->suffix = SvPV(v, PL_na);
		}
	}
}

typedef struct cmb_config * Cmb;

MODULE = Cmb		PACKAGE = Cmb		

INCLUDE: const-xs.inc

TYPEMAP: <<EOF
struct cmb_config *	T_PTROBJ
Cmb			T_PTROBJ
uint32_t		T_U_LONG
unsigned long long	T_U_LONG_LONG
uint64_t		T_U_LONG_LONG

INPUT
T_U_LONG_LONG
	$var = (unsigned long long)SvUv($arg)

OUTPUT
T_U_LONG_LONG
	sv_setuv($arg, (UV)$var);
EOF

Cmb
new(char *class, ...)
CODE:
	(void)class;
	struct cmb_config *c = calloc(1, sizeof(struct cmb_config));
	if (items > 1)
		_config(c, ST(1));
	RETVAL = c;
OUTPUT:
	RETVAL

void
DESTROY(c)
	Cmb c;
CODE:
	free(c);

void
config(c, hash)
	Cmb c
	SV *hash
CODE:
	_config(c, hash);

const char *
cmb_version(...)
CODE:
	RETVAL = cmb_version(items > 0 ? SvIV(ST(0)) : 0);
OUTPUT:
	RETVAL

const char *
version(c, ...)
	Cmb c
CODE:
	(void)c;
	RETVAL = cmb_version(items > 1 ? SvIV(ST(1)) : 0);
OUTPUT:
	RETVAL

uint64_t
cmb_count(c, nitems)
	Cmb c
	uint32_t nitems

uint64_t
count(c, nitems)
	Cmb c
	uint32_t nitems
CODE:
	RETVAL = cmb_count(c, nitems);
OUTPUT:
	RETVAL

int
print(c, seq, nitems, arrayref)
PREINIT:
	char **array;
	int inum;
	SV **item;
	int i;
INPUT:
	Cmb c;
	IV seq;
	uint32_t nitems;
	AV *arrayref;
CODE:
	inum = av_len(arrayref);
	array = (char **)calloc(inum + 1, sizeof(char *));
	for (i = 0; i <= inum; i++) {
		if ((item = av_fetch(arrayref, i, 0)) && SvOK(*item)) {
			array[i] = SvPV(*item, PL_na);
		}
	}
	RETVAL = cmb_print(c, seq, nitems, array);
	free(array);
OUTPUT:
	RETVAL

int
cmb(c, nitems, arrayref)
PREINIT:
	char **array;
	int inum;
	SV **item;
	int i;
INPUT:
	Cmb c;
	uint32_t nitems;
	AV *arrayref;
CODE:
	inum = av_len(arrayref);
	array = (char **)calloc(inum + 1, sizeof(char *));
	for (i = 0; i <= inum; i++) {
		if ((item = av_fetch(arrayref, i, 0)) && SvOK(*item)) {
			array[i] = SvPV(*item, PL_na);
		}
	}
	RETVAL = cmb(c, nitems, array);
	free(array);
OUTPUT:
	RETVAL

int
cmb_callback(c, nitems, arrayref, name)
PREINIT:
	int inum;
	char **array;
	int i;
	SV **item;
	void *_data;
	CMB_ACTION((*_action));
INPUT:
	Cmb c;
	uint32_t nitems;
	AV *arrayref;
	SV *name;
CODE:
	/* Set custom callback to call referenced subroutine */
	_action = c->action; /* save current callback */
	c->action = g_callback;

	/* Configure which Perl subroutine g_callback() should call */
	_data = c->data;
	c->data = (void *)calloc(2, sizeof(void *));
	((void **)c->data)[0] = name;

	/* Initialize globals used by g_callback() */
	((void **)c->data)[1] = (void *)sv_2mortal((SV *)newAV());

	/* Copy elements from arrayref to C array of SV pointers */
	inum = av_len(arrayref);
	array = (char **)calloc(inum + 1, sizeof(char *));
	for (i = 0; i <= inum; i++) {
		if ((item = av_fetch(arrayref, i, 0)) && SvOK(*item)) {
			array[i] = (char *)*item;
		}
	}

	/* Combine scalar value reference pointers using g_callback() */
	RETVAL = cmb(c, nitems, array);

	free(array);
	free(c->data);

	/* restore previous data */
	c->action = _action;
	c->data = _data;
OUTPUT:
	RETVAL
