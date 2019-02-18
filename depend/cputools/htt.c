/* -*- tab-width:  4 -*- ;; Emacs  */
/* vi: set tabstop=4     :: Vi/ViM */

/* Devin Teske (c)2006, April 19, 09:21:12. All Rights Reserved. */

/* system includes */
#include <sys/types.h>	/* standard types */
#include <stdio.h>	/* printf() */
#include <sys/sysctl.h>	/* sysctlbyname() */
#include <stdlib.h>	/* malloc() EXIT_SUCCESS EXIT_FAILURE strtoul() */
#include <err.h>	/* err() errx() */
#include <sys/syslog.h>	/* LOG_FAC() LOG_KERN */
#include <string.h>     /* strncmp() */


/* Preprocessor Macros */

/* dmesg `Features' string to search for (from beginning of line) */
#define SEARCH		"  Features=0x"
#define SEARCH_LEN	13

/* CPUID features bit signifying HyperThreading support */
#define HTT_FLAG	0x10000000


int
main (int argc, char *argv[])
{
   int ch, newl, skip;
   char *p, *ep, *bp;
   char buf[SEARCH_LEN + 8 + 1];
   int all = 0, pri = 0;
   size_t buflen;
   int bufpos;

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
    * The message buffer is circular.  If the buffer has wrapped, the
    * write pointer points to the oldest data.  Otherwise, the write
    * pointer points to \0's following the data.  Read the entire
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
         if ((bufpos == SEARCH_LEN + 8) && !strncmp(buf, SEARCH, SEARCH_LEN)) {
            unsigned long features;
            features = strtoul((char *)(buf + SEARCH_LEN),0,16);
            if (features & HTT_FLAG)
               printf("HyperThreading Status: YES\n");
            else
               printf("HyperThreading Status: NO\n");
            return(EXIT_SUCCESS);
         }
      }
   } while (++p != ep);

   printf("HyperThreading Status: ERROR\n");
   return(EXIT_FAILURE);
}

/*
 * $Header: /cvsroot/druidbsd/druidbsd/druid/dep/freebsd/util/htt.c,v 1.2 2012/09/02 16:04:27 devinteske Exp $
 *
 * $Copyright: 2006-2012 Devin Teske. All rights reserved. $
 *
 * $Log: htt.c,v $
 * Revision 1.2  2012/09/02 16:04:27  devinteske
 * Update copyright
 *
 * Revision 1.1  2012/01/28 06:59:47  devinteske
 * Commit initial public beta release (beta 56)
 *
 *
 */
