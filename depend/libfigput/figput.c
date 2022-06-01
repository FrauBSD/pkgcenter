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

#include <ctype.h>
#include <fcntl.h>
#include <libgen.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string_m.h>
#include <unistd.h>

#if 0 /* REFERENCE MATERIAL */
#include <err.h>
#include <limits.h>
#include <figpar.h>
#include <stdbool.h>
#include <stdint.h>
#include <sysexits.h>
#endif

#include "figput.h"
#if 0 /* REFERENCE MATERIAL */
#include "parser.h"
#include "util.h"
#endif

#if 0 /* REFERENCE MATERIAL */
#define STR_BUFSIZE 255
#define defcheck(type) do {                               \
	if (nodef                 &&                      \
	    options[i].value.type &&                      \
	    options[i].value.type == option.value.type)   \
		errx(EX_USAGE, "value == default value"); \
} while (0)
#endif

#if 0 /* REFERENCE MATERIAL */
extern uint8_t fline;
extern uint8_t found;
#endif

/*
 * Search for config option (struct figput_config) in the array of config
 * options and set the value of the struct whose directive matches the given
 * parameter. On success, returns 1, otherwise 0.
 */
int
set_config_option(struct figput_config options[], const char *directive,
    union figput_cfgvalue *value)
{
	uint32_t n;
	struct figput_config *found = NULL;

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
put_config(struct figput_config options[], const char *path,
    uint16_t processing_options, uint16_t put_options)
{
	uint8_t backup;
	uint8_t bequals;
	uint8_t bsemicolon;
	uint8_t case_sensitive;
	uint8_t comment;
	uint8_t emptyok;
	uint8_t end;
	uint8_t have_directive = 0;
	uint8_t have_equals;
	uint8_t have_value;
	uint8_t matched_directive;
	uint8_t missing;
	uint8_t nodflt;
	uint8_t nodup;
	uint8_t quote;
	uint8_t require_equals;
	uint8_t strict_equals;
	uint8_t unquoted;
	uint16_t result;
	char p[2];
	char *dcmp = NULL;
	char *directive = NULL;
	char *statement = NULL;
	char *t;
	const char *tmpdir;
	char *value = NULL;
	int error;
	int fd = -1;
	int rv = 0;
	int tmpfd = -1;
	ssize_t r = 1;
	struct figput_config *option;
	uint32_t dsize = 0;
	uint32_t line = 1;
	uint32_t n;
	uint32_t ssize = 0;
	uint32_t vsize = 0;
	off_t curpos;
	off_t endd;
	off_t endpos = 0;
	off_t endv;
	off_t startd;
	off_t startpos = 0;
	off_t startv;
	char rpath[PATH_MAX];
	char tpath[PATH_MAX];

	/* Sanity check: if no options, return */
	if (options == NULL)
		return (-1);

	/* Reading options from processing_options */
	bequals = (processing_options & FIGPAR_BREAK_ON_EQUALS) == 0 ? 0 : 1;
	bsemicolon =
	    (processing_options & FIGPAR_BREAK_ON_EQUALS) == 0 ? 0 : 1;
	case_sensitive =
	    (processing_options & FIGPAR_CASE_SENSITIVE) == 0 ? 0 : 1;
	require_equals =
	    (processing_options & FIGPAR_REQUIRE_EQUALS) == 0 ? 0 : 1;
	strict_equals =
	    (processing_options & FIGPAR_STRICT_EQUALS) == 0 ? 0 : 1;

	/* Writing options from put_options */
	backup   = (put_options & FIGPUT_SAVE_BACKUP)        == 0 ? 0 : 1;
	emptyok  = (put_options & FIGPUT_SAVE_ALLOW_EMPTY)   == 0 ? 0 : 1;
	nodflt   = (put_options & FIGPUT_NO_SAVE_DEFAULTS)   == 0 ? 0 : 1;
	nodup    = (put_options & FIGPUT_NO_SAVE_DUPLICATES) == 0 ? 0 : 1;
	unquoted = (put_options & FIGPUT_SAVE_UNQUOTED)      == 0 ? 0 : 1;

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

	/* Create temporary file */
	if ((tmpfd = mkstemp(tpath)) == -1)
		return (-1);

	/* Open original for reading */
	if ((fd = open(rpath, O_RDONLY)) < 0)
		return (-1);

	/* Read from the original and write to the temporary until EOF */
	while (r != 0) {
		comment = 0;
		have_directive = 0;
		have_equals = 0; /* NB: Only used when bequals set */
		have_value = 0;
		matched_directive = 0;

		/* Get the current offset as statement start */
		startpos = lseek(fd, 0, SEEK_CUR) - 1;
		if (startpos == -1) {
			rv = -1;
			goto put_config_cleanup;
		}
		endpos = startpos;

		/* Skip to the beginning of a directive */
		/* XXX Only # is supported for comments XXX */
		r = read(fd, p, 1);
		while (r != 0 && (isspace(*p) || *p == '#' || comment ||
		    (bsemicolon && *p == ';'))) {
			if (*p == '#')
				comment = 1;
			else if (*p == '\n') {
				line++;
			}
			r = read(fd, p, 1);
			endpos += 1;
		}
		/* Test for EOF before reaching directive */
		if (r == 0)
			goto put_config_eof;
		/* Test whether we read through a comment line */
		if (comment)
			goto put_config_end_statement;

		/* Get the current offset as start of directive */
		startd = lseek(fd, 0, SEEK_CUR) - 1;
		if (startd == -1) {
			rv = -1;
			goto put_config_cleanup;
		}

		/* Find the end of the directive */
		endd = startd;
		for (n = 0; r != 0; n++) {
			/* XXX ??? Literal # can be in directive ???! XXX */
			/* XXX Do not want, but I think figpar allows XXX */
			/* XXX !!! fix figpar and figput at same time XXX */
			if (isspace(*p))
				break;
			if (bequals && *p == '=') {
				have_equals = 1;
				break;
			}
			if (bsemicolon && *p == ';')
				break;
			r = read(fd, p, 1);
		}
		endd += n;

		/* Test for EOF with zero data read */
		if (n == 0 && r == 0)
			goto put_config_eof;

		/* Go back to the beginning of the directive */
		error = (int)lseek(fd, startd, SEEK_SET);
		if (error == (startd - 1)) {
			rv = -1;
			goto put_config_cleanup;
		}

		/* Allocate and read the directive into memory */
		if ((endd - startd) > dsize) {
			dsize = (endd - startd) + 1;
			if ((directive = realloc(directive, dsize)) == NULL) {
				rv = -1;
				goto put_config_cleanup;
			}
			if (case_sensitive) {
				dcmp = directive;
			} else if ((dcmp = realloc(dcmp, dsize)) == NULL) {
				rv = -1;
				goto put_config_cleanup;
			}
		}
		r = read(fd, directive, dsize - 1);
		directive[dsize] = '\0';
		have_directive = 1;

		/* Determine if directive matches */
		if (!case_sensitive) {
			strcpy(dcmp, directive);
			strtolower(dcmp);
		}
		for (n = 0; options[n].directive != NULL; n++) {
			option = &options[n];
			if (strcmp(option->directive, dcmp) != 0) {
				/* Not a match */
				continue;
			}
			matched_directive = 1;
			break;
		}

		/* Test for EOF with some data read */
		if (r == 0)
			goto put_config_eof;

		/*
		 * Read up to start of value (we need to know where the value
		 * ends but to do that we first need to know where it begins).
		 *
		 * NB: Regardless of whether the directive is known or NOT, we
		 * need to parse the whole line so even when it does not match
		 * a line we need to edit, we can parse up to the end of the
		 * full statement to contine at the next (which, if we the
		 * config supports semi-colons, may not necessarily be at the
		 * next newline character).
		 */

		/*
		 * If we are supposed to break-on-equals (vs simply requiring
		 * only whitespace separating directive and value) and the
		 * current character is an equal sign, seek past it and read
		 * the next character.
		 */
		if (bequals && *p == '=') {
			if (lseek(fd, 1, SEEK_CUR) != -1)
				r = read(fd, p, 1);
			/*
			 * Consider there to be no value to the directive if
			 * (1) equals are strict (zero whitespace allowed
			 * around them) and (2) the character immediately
			 * following is a space.
			 *
			 * NB: The newline acts as a terminator.
			 */
			if (strict_equals && isspace(*p))
				*p = '\n';
		}

		/*
		 * Read past whitespace to get to value start
		 * NB: If we are (a) told to break-on-semicolon and are
		 *     currently on a semi-colon, or (2) asked to treat equals
		 *     strictly (no surrounding whitespace) then do not advance
		 *     position.
		 */
		if (!(bsemicolon && *p == ';') &&
		    !(strict_equals && *p == '=')) {
			while (r != 0 && isspace(*p) && *p != '\n')
				r = read(fd, p, 1);
		}

		/*
		 * If we (1) have not hit EOF (r == 0) and (2) are supposed-to
		 * break-on-equals and (3) are on equals and (4) whitespace is
		 * allowed around the equals (strict_equals is unset) then read
		 * past any whitespace that follows said equals.
		 */
		if (r != 0 && bequals && *p == '=' && !strict_equals) {
			have_equals = 1;
			r = read(fd, p, 1);
			while (r != 0 && isspace(*p) && *p != '\n')
				r = read(fd, p, 1);
		}

		/* Get the current offset as start of value */
		startv = lseek(fd, 0, SEEK_CUR) - 1;
		if (startv == -1) {
			rv = -1;
			goto put_config_cleanup;
		}

		/* If no value, allocate a dummy value for below */
		/* XXX Only # is supported for comments XXX */
		if (r == 0 || *p == '\n' || *p == '#' ||
		    (bsemicolon && *p == ';')) {
			/* Initialize the value if not already done */
			if (value == NULL && (value = malloc(1)) == NULL) {
				rv = -1;
				goto put_config_cleanup;
			}
			value[0] = '\0';
			endv = startv;
			goto put_config_have_value;
		}

		/*
		 * Find the end of the value
		 */
		end = 0;
		quote = 0;
		while (r != 0 && end == 0) {
			/* Read until some condition prevents it */
			if (*p != '\"' && *p != '#' && *p != '\n' &&
			    (!bsemicolon || *p != ';')) {
				r = read(fd, p, 1);
				continue;
			}

			/* Get the current offset as last char read */
			curpos = lseek(fd, 0, SEEK_CUR) - 1;
			if (curpos == -1) {
				rv = -1;
				goto put_config_cleanup;
			}

			/*
			 * Seek back to test whether last char is escaped with
			 * backslash (making it part of value).
			 */
			error = (int)lseek(fd, -2, SEEK_CUR);
			if (error == -3) {
				rv = -1;
				goto put_config_cleanup;
			}
			r = read(fd, p, 1);

			/* Count how many backslashes there are (0 or more) */
			for (n = 0; *p == '\\'; n++) {
				/* Move back another byte */
				error = (int)lseek(fd, -2,
						   SEEK_CUR);
				if (error == -3) {
					rv = -1;
					goto put_config_cleanup;
				}
				r = read(fd, p, 1);
			}

			/* Return to previous position of last char read */
			error = (int)lseek(fd, curpos, SEEK_SET);
			if (error == (curpos - 1)) {
				rv = -1;
				goto put_config_cleanup;
			}
			r = read(fd, p, 1);

			/*
			 * If last character is NOT escaped
			 */
			if ((n & 1) == 0) {
				switch (*p) {
				case '\"':
					quote = !quote;
					break;
				case '#':
					if (!quote)
						end = 1;
					break;
				case '\n':
					end = 1;
					break;
				case ';':
					if (!quote &&
					    bsemicolon)
						end = 1;
					break;
				}
			} else { /* Escaped */
				switch (*p) {
				case '\n':
					line++;
				}
			}

			/* Advance to the next character */
			r = read(fd, p, 1);
		}

		/* Get the current offset as value end */
		endv = lseek(fd, 0, SEEK_CUR) - 1;
		if (endv == -1) {
			rv = -1;
			goto put_config_cleanup;
		}

		/* Move offset back to the beginning of the value */
		error = (int)lseek(fd, startv, SEEK_SET);
		if (error == (startv - 1)) {
			rv = -1;
			goto put_config_cleanup;
		}

		/* Calculate value length */
		n = (uint32_t)(endv - startv);
		if (r != 0) /* more to read, don't include last char read */
			n--;

		/* Allocate and read the value into memory */
		if (n > vsize) {
			vsize = n + 1;
			if ((value = realloc(value, vsize)) == NULL) {
				rv = -1;
				goto put_config_cleanup;
			}
		}
		r = read(fd, value, vsize - 1);
		value[vsize] = '\0';
		have_value = 1;

		/* Cut trailing whitespace off by termination */
		t = value + vsize - 1;
		while (isspace(*--t))
			*t = '\0';

put_config_have_value:
		/* Continue to read until end of statement */
		if (*p == '\n' || (bsemicolon && *p == ';')) {
			while (r != 0 && (isspace(*p) || *p == '#' ||
			    comment)) {
				if (*p == '#')
					comment = 1;
				else if (*p == '\n') {
					comment = 0;
					line++;
				}
				r = read(fd, p, 1);
			}
		}

		/* Get the current offset as statement end */
		endpos = lseek(fd, 0, SEEK_CUR) - 1;
		if (endpos == -1) {
			rv = -1;
			goto put_config_cleanup;
		}

put_config_end_statement:

		/*
		 * For no-match, simply put the line/statement and continue
		 */
		if (!matched_directive) {
			/* Read up to the end of the statement */
		}

#if 0 /* UPCOMING */
		result = option->result; /* NB: copy */
		missing =
		    (result & FIGPUT_DIRECTIVE_FOUND) == 0 ? 1 : 0;
		switch(option->action) {
		case FIGPUT_ACTION_CHECK:
		case FIGPUT_ACTION_REMOVE:
		case FIGPUT_ACTION_SET_VALUE:
		}
#endif

	} /* while (read) */

put_config_eof:
	/* Close read config (keep temp file open for additional writes) */
	close(fd);
	fd = -1;

	/*
	 * Write any remaining data trailing the last statement
	 */
	if (!have_directive && endpos > startpos) {
		/* Allocate space for the statement to be read */
		n = (endpos - startpos) + 1;
		if (n > ssize)
			ssize = n;
		if ((statement = realloc(statement, ssize)) == NULL) {
			rv = -1;
			goto put_config_cleanup;
		}

		/* Go back to the beginning of the statement */
		error = (int)lseek(fd, startpos, SEEK_SET);
		if (error == (startpos - 1)) {
			rv = -1;
			goto put_config_cleanup;
		}

		/* Read statement so we can then write it as-is */
		r = read(fd, statement, ssize - 1);
		statement[ssize] = '\0';

		/* XXX write statement to tmpfd XXX */
	}

	/* Loop through options we were told to put */
	for (n = 0; options[n].directive != NULL; n++) {
		option = &options[n];
		result = option->result; /* NB: copy */
		missing = (result & FIGPUT_DIRECTIVE_FOUND) == 0 ? 1 : 0;
		switch (option->action) {
		case FIGPUT_ACTION_CHECK:
			if (missing) {
				/* Checks failed, mark as different */
				option->result |= FIGPUT_VALUE_CHANGED;
				continue;
			}
			/* Checks done */
			continue;
		case FIGPUT_ACTION_REMOVE:
			/* Removed already */
			continue;
		case FIGPUT_ACTION_SET_VALUE:
			if (!missing) {
				/* Changed already */
				continue;
			}
			/* Add to output file */
			break;
		}

		/* XXX need to add directive to tmpfd XXX */
	}

	close(tmpfd);
	tmpfd = -1;

	/* XXX move file XXX */

put_config_cleanup:
	if (fd >= 0)
		close(fd);
	if (tmpfd >= 0)
		close(tmpfd);

	/* XXX free memory XXX */

	return (rv);
}

#if 0 /* REFERENCE MATERIAL */
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
