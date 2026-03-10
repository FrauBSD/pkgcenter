/*-
 * Copyright (c) 2018-2026 Devin Teske <dteske@FreeBSD.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _CMB_PRIVATE_H_
#define _CMB_PRIVATE_H_

/* Limits */
#define BUFSIZE_MAX		(2 * 1024 * 1024)
					/* Buffer size for read(2) input */
#ifndef MAXPHYS
#define MAXPHYS			(128 * 1024)
					/* max raw I/O transfer size */
#endif

/*
 * Memory strategy threshold, in pages: if physmem is larger than this,
 * use a large buffer.
 */
#define PHYSPAGES_THRESHOLD	(32 * 1024)

/*
 * Small (default) buffer size in bytes. It's inefficient for this to be
 * smaller than MAXPHYS.
 */
#define BUFSIZE_SMALL		(MAXPHYS)

/*
 * Math macros
 */
#undef  MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#undef  MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#endif /* !_CMB_PRIVATE_H_ */
