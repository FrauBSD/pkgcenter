/*-
 * Copyright (c) 2015-2022 Devin Teske <dteske@FreeBSD.org>
 * Copyright (c) 2021-2022 Faraz Vahedi <kfv@kfv.io>
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
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <fcntl.h>
#include <libgen.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if 0
#include <err.h>
#include <limits.h>
#include <figpar.h>
#include <stdbool.h>
#include <stdint.h>
#include <sysexits.h>
#endif

#include "figput.h"
#if 0
#include "parser.h"
#include "util.h"
#endif

#if 0
#define STR_BUFSIZE 255
#define defcheck(type) do {                               \
	if (nodef                 &&                      \
	    options[i].value.type &&                      \
	    options[i].value.type == option.value.type)   \
		errx(EX_USAGE, "value == default value"); \
} while (0)
#endif

#if 0
extern uint8_t fline;
extern uint8_t found;
#endif

/*
 * Search for config option (struct fp_config) in the array of config
 * options and set the value of the struct whose directive matches the given
 * parameter. On success, returns 1, otherwise 0.
 */
int
set_config_option(struct fp_config options[], const char *directive,
    union fp_cfgvalue *value)
{
	uint32_t n;
	struct fp_config *found = NULL;

	/* Check arguments */
	if (options == NULL || directive == NULL)
		return (0);

	/* Loop through the array, find the first match */
	for (n = 0; options[n].directive != NULL; n++) {
		if (strcmp(options[n].directive, directive) == 0) {
			found = &options[n];
			break;
		}
	}
	if (found == NULL)
		return (0);

	found->value.data = value;

	return (1);
}

int
put_config(struct fp_config options[], const char *path, uint8_t put_options)
{
	uint8_t backup;
	uint8_t emptyok;
	uint8_t nodflt;
	uint8_t nodup;
	uint8_t unquoted;
	const char *tmpdir;
	int fd;
	int tmpfd;
	ssize_t r = 1;
	char rpath[PATH_MAX];
	char tpath[PATH_MAX];

	/* Sanity check: if no options, return */
	if (options == NULL)
		return (-1);

	/* Writing options from put_options */
	backup   = (put_options & FP_SAVE_BACKUP)        == 0 ? 0 : 1;
	emptyok  = (put_options & FP_SAVE_ALLOW_EMPTY)   == 0 ? 0 : 1;
	nodflt   = (put_options & FP_NO_SAVE_DEFAULTS)   == 0 ? 0 : 1;
	nodup    = (put_options & FP_NO_SAVE_DUPLICATES) == 0 ? 0 : 1;
	unquoted = (put_options & FP_SAVE_UNQUOTED)      == 0 ? 0 : 1;

	/* Resolve the file path */
	if (realpath(path, rpath) == 0)
		return (-1);

	/* Determine path to temporary file */
	if ((tmpdir = getenv("FIGPUT_TMPDIR")) == NULL || *tmpdir == '\0' ||
	    (tmpdir = getenv("TMPDIR")) == NULL || *tmpdir == '\0') {
		tmpdir = _PATH_TMP;
	}
	if ((snprintf(tpath, PATH_MAX, "%s/%s.XXXXXXXXXX", tmpdir,
	    basename(rpath))) < 0)
		return (-1);
	if ((tmpfd = mkstemp(tpath)) == -1)
		return (-1);

	/* Open original for reading */
	if ((fd = open(rpath, O_RDONLY)) < 0)
		return (-1);

	/* Read from the original and write to the temporary until EOF */
	while (r != 0) {
		break; /* XXX */
	}

	close(fd);
	close(tmpfd);

	/* XXX move XXX */

	return (0);
}

#if 0
int
emit_config(struct figpar_config options[static 1], const char *path,
    const char *directive, const char *value, uint16_t par_options,
    uint16_t put_options)
{
	int oldconfig;
	ssize_t r = 1;
	ssize_t dlen = strlen(directive);
	ssize_t vlen = strlen(value);
	ssize_t tlen = 0;
	char buf[2] = {0};
	char vbuf[STR_BUFSIZE]      = {0};
	char tbuf[STR_BUFSIZE * 2]  = {0};
	char bpath[PATH_MAX]        = {0};
	enum figpar_cfgtype  optype = 0;
	struct figpar_config config = {0}; /* In place configuration */
	struct figpar_config option = {0}; /* Provided configuration */

	if (value && !noval) {
		if (quote)
			snprintf(vbuf, vlen + 3, "\"%s\"", value);
		else
			snprintf(vbuf, vlen + 1, "%s", value);
	}

	/* Sanity check */
	if (!directive)
		errx(EX_USAGE, "Directive cannot be NULL");

	for (size_t i = 0; options[i].directive; i++) {
		/* Break if we have already filled the option */
		if (option.directive)
			break;
		/* Continue if directive is not found */
		if (strcmp(options[i].directive, directive))
			continue;

		option.type = options[i].type;
		if (!(option.directive = strndup(options[i].directive,
		    STR_BUFSIZE)))
			err(EXIT_FAILURE, NULL);

		switch (options[i].type) {
		case FIGPAR_TYPE_NONE:
			optype = FIGPAR_TYPE_NONE;
			option.action = &setnone;
			if (!noval) noval = 1;
			if (value)
				errx(1, "Directive does not accept a value");
			break;
		case FIGPAR_TYPE_BOOL:
			/* XXX: Incomplete */
			optype = FIGPAR_TYPE_BOOL;
			option.action = &setbool;
			if (noval && options[i].value.boolean == true)
				errx(1, "Default value is already set true");
			defcheck(boolean);
			break;
		case FIGPAR_TYPE_INT:
			optype = FIGPAR_TYPE_INT;
			option.action = &setnum;
			if (!(option.value.num = strtol(value, 0, 10)))
				err(EX_USAGE, "Value is not a number");
			defcheck(num);
			break;
		case FIGPAR_TYPE_UINT:
			optype = FIGPAR_TYPE_UINT;
			option.action = &setnum;
			if (!(option.value.u_num = strtol(value, 0, 10)))
				err(EX_USAGE, "Value is not a number");
			defcheck(u_num);
			break;
		case FIGPAR_TYPE_STR:
			optype = FIGPAR_TYPE_STR;
			option.action = &setstr;
			if (!(option.value.str = strndup(value, STR_BUFSIZE)))
				errx(1, "Could not use the value");
			defcheck(str);
			break;
		/* XXX: Add FIGPAR_TYPE_STRARRAY case */
		default:
			errx(1, "Don't know how to handle type");
		}
	}

	config = option;
	parse_config(&config, path, NULL, par_options);

	if ((found > 1) && nodup)
		errx(EX_USAGE, "Resolve duplicates in the config file");

	if (backup) {
		if (snprintf(bpath, PATH_MAX, "%s.bak", rpath) < 0)
			err(EXIT_FAILURE, "snprintf failed");
		if (fig_backup(rpath, bpath))
			err(EX_IOERR, "Could not backup the config file");
	}

	if ((oldconfig = open(rpath, O_RDWR)) < 0)
		err(EX_IOERR, "Could not open the config file");

	/*
	 * Write to the temporary file up to the match line
	 *
	 * N.B. This means up to the EOF if either the
	 * type is FIGPAR_TYPE_NONE or the directive is not
	 * yet set (whether the default value's in use or
	 * the option is not set at all.)
	 */
	unsigned l = 1;
	while (r && l != fline) {
		if ((r = read(oldconfig, buf, 1)) > 0)
			if ((write(tmpfd, buf, 1)) != r)
				err(EX_IOERR, "Write error");
		if (r < 0)
			err(EX_IOERR, "Read error");
		if (*buf == '\n')
			l++;
	}

	/* In case the directive stands without a value */
	if (noval) {
		if (found)
			errx(EX_USAGE, "Value is already set");

		if (write(tmpfd, option.directive, dlen) != dlen)
			err(EX_USAGE, "Cannot write to the config file");

		close(oldconfig);
		close(tmpfd);

		fig_move(tpath, rpath);

		return EXIT_SUCCESS;
	}

	/* Format the directive/value string as requested */
	if (par_options & FIGPAR_STRICT_EQUALS)
		snprintf(tbuf, dlen + strlen(vbuf) + 2,
		    "%s=%s", directive, vbuf);
	else {
		if (par_options & FIGPAR_REQUIRE_EQUALS)
			snprintf(tbuf, dlen + strlen(vbuf) + 4,
			    "%s = %s", directive, vbuf);
		else
			snprintf(tbuf, dlen + strlen(vbuf) + 2,
			    "%s %s", directive, vbuf);
	}
	tlen = strlen(tbuf);

	/*
	 * If options is not set in the config file, append.
	 * Otherwise, if FIGPAR_BREAK_ON_SEMICOLON is not set,
	 * overwrite the line if found, or else find the position,
	 * write the formatted directive/value, read from the
	 * oldconfig up to either a semicolon or a '\n', and
	 * finally copy the rest of the lines.
	 */
	if (!found) {
		if (write(tmpfd, tbuf, strlen(tbuf)) != tlen)
			err(EX_USAGE, "Write error");

		close(oldconfig);
		close(tmpfd);

		fig_move(tpath, rpath);

		return EXIT_SUCCESS;
	}

	/* Err if new value == old value from the config */
	if (((optype == FIGPAR_TYPE_INT)  &&
	     (option.value.num == config.value.num))     ||
	    ((optype == FIGPAR_TYPE_UINT) &&
	     (option.value.u_num == config.value.u_num)) ||
	    ((optype == FIGPAR_TYPE_STR)  &&
	     !strcmp(vbuf, config.value.str)))
		errx(EX_USAGE,
		    "new value == old value");

	/*
	 * XXX: Body of the following condition could be probably
	 * written as a function.
	 */
	if (par_options & FIGPAR_BREAK_ON_SEMICOLON) {
		ssize_t vlen = 0;
		size_t counter = 0, idx = 0, pos = 0;
		char lbuf[STR_BUFSIZE] = {0};
		fig_node_t *head = NULL;
		fig_node_t *last = NULL;
		fig_node_t *tail = NULL;
		fig_node_t *test = NULL;

		while ((r = read(oldconfig, buf, 1)) > 0) {
			if ((buf[0] == ';') || (buf[0] == '\n')) {
				if (buf[0] == ';')
					counter++;
				if ((counter == 0 || counter == 1) && !head) {
					test = tail = last = head =
					    malloc(sizeof(fig_node_t) +
					    strlen(lbuf) + 1);
				}
				else {
					tail = malloc(sizeof(fig_node_t) +
						strlen(lbuf) + 1);
					last->next = tail;
				}
				snprintf(tail->val, strlen(lbuf) + 1, "%s",
				    lbuf);
				last = tail;
				tail = tail->next;
				memset(lbuf, 0, sizeof lbuf);
				idx = 0;
				if (buf[0] == '\n')
					break;
				continue;
			}
			lbuf[idx] = buf[0];
			idx++;
		}
		last->next = NULL;

		while (head && strlen(head->val) > 0) {
			char *ptr = strstr(head->val, directive);
			if (ptr) {
				if (pos >= 1)
					if ((write(tmpfd, " ", 1) != 1))
						err(EX_USAGE, "Write error");
				if (strncmp(ptr, directive, dlen + 1))
					if ((write(tmpfd, tbuf,
					    strlen(tbuf)) !=
					    tlen))
						err(EX_USAGE, "Write error");
				if ((pos <= counter) &&
				    ((write(tmpfd, ";", 1)) != 1))
					err(EX_IOERR, "Write error");
				head = head->next;
				continue;
			}
			vlen = strlen(head->val);
			if ((write(tmpfd, head->val, strlen(head->val))) !=
			    vlen)
				err(EX_IOERR, "Write error");
			if ((write(tmpfd, ";", 1)) != 1)
				err(EX_IOERR, "Write error");
			head = head->next;
			pos++;
		}

		/* Write '\n' if one is left */
		if (r && (write(tmpfd, buf, 1)) != r)
				err(EX_IOERR, "Write error");

		/* Free the linked list */
		fig_freelist(test);
	} else {
		if (write(tmpfd, tbuf, strlen(tbuf)) != tlen)
			err(EX_USAGE, "Cannot write to the config file");
		while ((r = read(oldconfig, buf, 1)) > 0)
			if (buf[0] == '\n')
				break;
		if (r && write(tmpfd, buf, 1) != r)
				err(EX_IOERR, "Write error");
	}

	/* Write the remaining lines from the old file */
	while ((r = read(oldconfig, buf, 1)) > 0) {
			if ((write(tmpfd, buf, 1)) != r)
				err(EX_IOERR, "Write error");
		if (r < 0)
			err(EX_IOERR, "Read error");
	}

	close(oldconfig);
	close(tmpfd);

	fig_move(tpath, rpath);

	return EXIT_SUCCESS;
}
#endif
