/*-
 * Copyright (c) 2006-2019 Devin Teske <dteske@FreeBSD.org>
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
__FBSDID("$FrauBSD: //github.com/FrauBSD/pkgcenter/depend/cputools/htt.c 2019-02-17 19:42:20 -0800 freebsdfrau $");
#endif

#include <sys/sysctl.h>
#include <sys/syslog.h>
#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* dmesg `Features' string to search for (from beginning of line) */
#define SEARCH		"  Features=0x"
#define SEARCH_LEN	13

/* CPUID features bit signifying HyperThreading support */
#define HTT_FLAG	0x10000000

int
main(int argc, char *argv[])
{
	int all = 0, pri = 0;
	int bufpos;
	int ch, newl, skip;
	char *p, *ep, *bp;
	size_t buflen;
	unsigned long features;
	char buf[SEARCH_LEN + 8 + 1];

	/* Running kernel. Use sysctl. */
	if (sysctlbyname("kern.msgbuf", NULL, &buflen, NULL, 0) == -1)
		err(1, "sysctl kern.msgbuf");
	if ((bp = malloc(buflen)) == NULL)
		errx(1, "malloc failed");
	if (sysctlbyname("kern.msgbuf", bp, &buflen, NULL, 0) == -1)
		err(1, "sysctl kern.msgbuf");

	memset(buf, 0, sizeof(buf));
	bufpos = 0;

	/*
	 * The message buffer is circular. If the buffer has wrapped, the
	 * write pointer points to the oldest data. Otherwise, the write
	 * pointer points to \0's following the data. Read the entire
	 * buffer starting at the write pointer and ignore nulls so that
	 * we effectively start at the oldest data.
	 */
	p = bp;
	ep = bp + buflen;
	newl = skip = 0;
	do {
		if (p == bp + buflen)
			p = bp;
		ch = *p;
		/* Skip "\n<.*>" syslog sequences. */
		if (skip) {
			if (ch == '\n') {
				skip = 0;
				newl = 1;
			} if (ch == '>') {
				if (LOG_FAC(pri) == LOG_KERN || all)
					newl = skip = 0;
			} else if (ch >= '0' && ch <= '9') {
				pri *= 10;
				pri += ch - '0';
			}
			continue;
		}
		if (newl && ch == '<') {
			pri = 0;
			skip = 1;
			continue;
		}
		if (ch == '\0')
			continue;
		newl = ch == '\n';
		if (ch == '\n') {
			memset(buf, 0, sizeof(buf));
			bufpos = 0;
		} else if (bufpos < SEARCH_LEN + 8) {
			buf[bufpos++] = ch;
			if ((bufpos == SEARCH_LEN + 8) && !strncmp(buf, SEARCH,
			    SEARCH_LEN)) {
				features = strtoul((char *)(buf + SEARCH_LEN),
				    0, 16);
				if (features & HTT_FLAG)
					printf("HyperThreading Status: YES\n");
				else
					printf("HyperThreading Status: NO\n");
				return (EXIT_SUCCESS);
			}
		}
	} while (++p != ep);

	printf("HyperThreading Status: ERROR\n");
	return (EXIT_FAILURE);
}
