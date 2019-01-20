/*-
 * Copyright (c) 2013-2015 Devin Teske <dteske@FreeBSD.org>
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
 *
 * $FreeBSD: head/lib/libdpv/dialogrc.h 290275 2015-11-02 20:03:59Z dteske $
 */

#ifndef _DIALOGRC_H_
#define _DIALOGRC_H_

#include <sys/types.h>

#include <figpar.h>

/* dialog(3) dlg_color_table[] attributes */
#define GAUGE_ATTR	33	/* entry used for gauge_color */

/* dialog(1) characteristics */
#define DIALOGRC	".dialogrc"
#define ENV_DIALOGRC	"DIALOGRC"
#define ENV_HOME	"HOME"

/* dialog(1) `.dialogrc' characteristics */
extern uint8_t use_colors;
extern uint8_t use_shadow;
extern char gauge_color[];
extern char separator[];

__BEGIN_DECLS
void			 dialogrc_free(void);
int			 parse_dialogrc(void);
struct figpar_config	*dialogrc_config_option(const char *_directive);
__END_DECLS

#endif /* !_DIALOGRC_H_ */
