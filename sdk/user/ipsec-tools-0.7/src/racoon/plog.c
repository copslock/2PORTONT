/*	$NetBSD: plog.c,v 1.4 2006/09/09 16:22:10 manu Exp $	*/

/* Id: plog.c,v 1.11 2006/06/20 09:57:31 vanhu Exp */

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/param.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <ctype.h>
#include <err.h>

#include "var.h"
#include "misc.h"
#include "plog.h"
#include "logger.h"
#include "debug.h"
#include "gcmalloc.h"

#ifndef VA_COPY
# define VA_COPY(dst,src) memcpy(&(dst), &(src), sizeof(va_list))
#endif

char *pname = NULL;
u_int32_t loglevel = LLV_BASE;
int f_foreground = 0;

int print_location = 0;

static struct log *logp = NULL;
static char *logfile = "/tmp/racoon.log";

static char *plog_common __P((int, const char *, const char *));

static struct plogtags {
	char *name;
	int priority;
} ptab[] = {
	{ "(not defined)",	0, },
	{ "ERROR",		LOG_INFO, },
	{ "WARNING",		LOG_INFO, },
	{ "NOTIFY",		LOG_INFO, },
	{ "INFO",		LOG_INFO, },
	{ "DEBUG",		LOG_DEBUG, },
	{ "DEBUG2",		LOG_DEBUG, },
};

static char *
plog_common(pri, fmt, func)
	int pri;
	const char *fmt, *func;
{
	static char buf[800];	/* XXX shoule be allocated every time ? */
	char *p;
	int reslen, len;

	p = buf;
	reslen = sizeof(buf);

	if (logfile || f_foreground) {
		time_t t;
		struct tm *tm;

		t = time(0);
		tm = localtime(&t);
		len = strftime(p, reslen, "%Y-%m-%d %T: ", tm);
		p += len;
		reslen -= len;
	}

	if (pri < ARRAYLEN(ptab)) {
		len = snprintf(p, reslen, "%s: ", ptab[pri].name);
		if (len >= 0 && len < reslen) {
			p += len;
			reslen -= len;
		} else
			*p = '\0';
	}

	if (print_location)
		snprintf(p, reslen, "%s: %s", func, fmt);
	else
		snprintf(p, reslen, "%s", fmt);
#ifdef BROKEN_PRINTF
	while ((p = strstr(buf,"%z")) != NULL)
		p[1] = 'l';
#endif

	return buf;
}

void
plog(int pri, const char *func, struct sockaddr *sa, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	plogv(pri, func, sa, fmt, ap);
	va_end(ap);
}

void
plogv(int pri, const char *func, struct sockaddr *sa,
	const char *fmt, va_list ap)
{
	char *newfmt;
	va_list ap_bak;

	if (pri > loglevel)
		return;

	newfmt = plog_common(pri, fmt, func);

	VA_COPY(ap_bak, ap);
	
	if (f_foreground)
		vprintf(newfmt, ap);

	if (logfile)
		log_vaprint(logp, newfmt, ap_bak);
	else {
		if (pri < ARRAYLEN(ptab))
			vsyslog(ptab[pri].priority, newfmt, ap_bak);
		else
			vsyslog(LOG_ALERT, newfmt, ap_bak);
	}
}

void
plogdump(pri, data, len)
	int pri;
	void *data;
	size_t len;
{
	caddr_t buf;
	size_t buflen;
	int i, j;

	if (pri > loglevel)
		return;

	/*
	 * 2 words a bytes + 1 space 4 bytes + 1 newline 32 bytes
	 * + 2 newline + '\0'
	 */
	buflen = (len * 2) + (len / 4) + (len / 32) + 3;
	buf = racoon_malloc(buflen);

	i = 0;
	j = 0;
	while (j < len) {
		if (j % 32 == 0)
			buf[i++] = '\n';
		else
		if (j % 4 == 0)
			buf[i++] = ' ';
		snprintf(&buf[i], buflen - i, "%02x",
			((unsigned char *)data)[j] & 0xff);
		i += 2;
		j++;
	}
	if (buflen - i >= 2) {
		buf[i++] = '\n';
		buf[i] = '\0';
	}
	plog(pri, LOCATION, NULL, "%s", buf);

	racoon_free(buf);
}

void
ploginit()
{
	if (logfile) {
		logp = log_open(250, logfile);
		if (logp == NULL)
			errx(1, "ERROR: failed to open log file %s.", logfile);
		return;
	}

        openlog(pname, LOG_NDELAY, LOG_DAEMON);
}

void
plogset(file)
	char *file;
{
	if (logfile != NULL)
		racoon_free(logfile);
	logfile = racoon_strdup(file);
	STRDUP_FATAL(logfile);
}

/*
   Returns a printable string from (possibly) binary data ;
   concatenates all unprintable chars to one space.
   XXX Maybe the printable chars range is too large...
 */
char*
binsanitize(binstr, n)
	char *binstr;
	size_t n;
{
	int p,q;
	char* d;
	for (p = 0, q = 0; p < n; p++) {
                 if (isgraph((int)binstr[p])) {
			binstr[q++] = binstr[p];
		} else {
			if (q && binstr[q - 1] != ' ')
				 binstr[q++] = ' ';
		}
	}
	binstr[q++] = '\0';
	return binstr;
}
	
