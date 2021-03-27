/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James W. Williams of NASA Goddard Space Flight Center.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)cksum.c	8.2 (Berkeley) 4/28/95";
#endif
static const char rcsid[] =
	"$Id: cksum.c,v 1.9 1998/03/10 05:03:31 jb Exp $";
#endif /* not lint */

#include <sys/cdefs.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "extern.h"

#ifndef warn
#define warn(format, arg...) fprintf(stderr, format, ##arg)
#endif

static void usage __P((void));

int binary = 0;

int
main(argc, argv)
	int argc;
	char **argv;
{
	register int ch, fd, rval;
	u_int32_t len, val;
	char *fn;
	unsigned char nbuf[4];
	int (*cfncn) __P((int, u_int32_t *, u_int32_t *));
	void (*pfncn) __P((char *, u_int32_t, u_int32_t));

	cfncn = crc;
	pfncn = pcrc;

	while ((ch = getopt(argc, argv, "bo:")) != -1)
		switch (ch) {
		case 'o':
			if (!strcmp(optarg, "1")) {
				cfncn = csum1;
					pfncn = psum1;
			} else if (!strcmp(optarg, "2")) {
				cfncn = csum2;
				pfncn = psum2;
			} else if (*optarg == '3') {
				cfncn = crc32;
				pfncn = pcrc;
			} else {
				fprintf(stderr, "illegal argument to -o option\n");
				usage();
			}
			break;
		case 'b':
			binary++;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	fd = STDIN_FILENO;
	fn = NULL;
	rval = 0;
	do {
		if (*argv) {
			fn = *argv++;
			if ((fd = open(fn, O_RDONLY, 0)) < 0) {
				warn("%s", fn);
				rval = 1;
				continue;
			}
		}
		if (cfncn(fd, &val, &len)) {
			warn("%s", fn ? fn : "stdin");
			rval = 1;
		} else {
			if (binary) {
				nbuf[0] = (val >> 24) & 0xff;
				nbuf[1] = (val >> 16) & 0xff;
				nbuf[2] = (val >> 8) & 0xff;
				nbuf[3] = val & 0xff;
				write(1,  &nbuf[0], 4);
			} else {
				pfncn(fn, val, len);
			}
		}
		(void)close(fd);
	} while (*argv);
	exit(rval);
}

static void
usage()
{
	(void)fprintf(stderr, "usage: cksum [-b] [-o 1 | 2] [file ...]\n");
	(void)fprintf(stderr, "       sum [file ...]\n");
	exit(1);
}
