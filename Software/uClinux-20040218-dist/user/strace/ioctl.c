/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2001 Wichert Akkerman <wichert@cistron.nl>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: ioctl.c,v 1.8 2001/08/03 21:52:13 wichert Exp $
 */

#include "defs.h"

struct ioctlent ioctlent0[] = {
/*
 * `ioctlent.h' may be generated from `ioctlent.raw' by the auxiliary
 * program `ioctlsort', such that the list is sorted by the `code' field.
 * This has the side-effect of resolving the _IO.. macros into
 * plain integers, eliminating the need to include here everything
 * in "/usr/include" .
 */
#include "ioctlent.h"
};

#ifdef LINUX
#include <asm/ioctl.h>
#endif

int nioctlents0 = sizeof ioctlent0 / sizeof ioctlent0[0];

#if SUPPORTED_PERSONALITIES >= 2
struct ioctlent ioctlent1[] = {
#include "ioctlent1.h"
};

int nioctlents1 = sizeof ioctlent1 / sizeof ioctlent1[0];
#endif /* SUPPORTED_PERSONALITIES >= 2 */

#if SUPPORTED_PERSONALITIES >= 3
struct ioctlent ioctlent2[] = {
#include "ioctlent2.h"
};

int nioctlents1 = sizeof ioctlent2 / sizeof ioctlent2[0];
#endif /* SUPPORTED_PERSONALITIES >= 3 */

struct ioctlent *ioctlent;
int nioctlents;

static int
compare(a, b)
const void *a;
const void *b;
{
	unsigned long code1 = ((struct ioctlent *) a)->code;
	unsigned long code2 = ((struct ioctlent *) b)->code;
	return (code1 > code2) ? 1 : (code1 < code2) ? -1 : 0;
}

char *
ioctl_lookup(code)
long code;
{
	struct ioctlent *iop, ioent;

	ioent.code = code;
#ifdef LINUX
	ioent.code &= (_IOC_NRMASK<<_IOC_NRSHIFT) | (_IOC_TYPEMASK<<_IOC_TYPESHIFT);
#endif
	iop = (struct ioctlent *) bsearch((char *) &ioent, (char *) ioctlent,
			nioctlents, sizeof(struct ioctlent), compare);
	return iop ? iop->symbol : NULL;
}

int
ioctl_decode(tcp, code, arg)
struct tcb *tcp;
long code, arg;
{
	switch ((code >> 8) & 0xff) {
#ifdef LINUX
#ifdef ALPHA
	case 'f': case 't': case 'T':
#else /* !ALPHA */
	case 0x54:
#endif /* !ALPHA */
#else /* !LINUX */
	case 'f': case 't': case 'T':
#endif /* !LINUX */
		return term_ioctl(tcp, code, arg);
#ifdef LINUX
	case 0x89:
#else /* !LINUX */
	case 'r': case 's': case 'i':
#ifndef FREEBSD		
	case 'p':
#endif		
#endif /* !LINUX */
		return sock_ioctl(tcp, code, arg);
#ifdef USE_PROCFS
#ifndef HAVE_MP_PROCFS
#ifndef FREEBSD
	case 'q':
#else
	case 'p':
#endif		
		return proc_ioctl(tcp, code, arg);
#endif
#endif /* USE_PROCFS */
#ifdef HAVE_SYS_STREAM_H
	case 'S':
		return stream_ioctl(tcp, code, arg);
#endif /* HAVE_SYS_STREAM_H */
	default:
		break;
	}
	return 0;
}

/*
 * Registry of ioctl characters, culled from
 *	@(#)ioccom.h 1.7 89/06/16 SMI; from UCB ioctl.h 7.1 6/4/86
 *
 * char	file where defined		notes
 * ----	------------------		-----
 *   F	sun/fbio.h
 *   G	sun/gpio.h
 *   H	vaxif/if_hy.h
 *   M	sundev/mcpcmd.h			*overlap*
 *   M	sys/modem.h			*overlap*
 *   S	sys/stropts.h
 *   T	sys/termio.h			-no overlap-
 *   T	sys/termios.h			-no overlap-
 *   V	sundev/mdreg.h
 *   a	vaxuba/adreg.h
 *   d	sun/dkio.h			-no overlap with sys/des.h-
 *   d	sys/des.h			(possible overlap)
 *   d	vax/dkio.h			(possible overlap)
 *   d	vaxuba/rxreg.h			(possible overlap)
 *   f	sys/filio.h
 *   g	sunwindow/win_ioctl.h		-no overlap-
 *   g	sunwindowdev/winioctl.c		!no manifest constant! -no overlap-
 *   h	sundev/hrc_common.h
 *   i	sys/sockio.h			*overlap*
 *   i	vaxuba/ikreg.h			*overlap*
 *   k	sundev/kbio.h
 *   m	sundev/msio.h			(possible overlap)
 *   m	sundev/msreg.h			(possible overlap)
 *   m	sys/mtio.h			(possible overlap)
 *   n	sun/ndio.h
 *   p	net/nit_buf.h			(possible overlap)
 *   p	net/nit_if.h			(possible overlap)
 *   p	net/nit_pf.h			(possible overlap)
 *   p	sundev/fpareg.h			(possible overlap)
 *   p	sys/sockio.h			(possible overlap)
 *   p	vaxuba/psreg.h			(possible overlap)
 *   q	sun/sqz.h
 *   r	sys/sockio.h
 *   s	sys/sockio.h
 *   t	sys/ttold.h			(possible overlap)
 *   t	sys/ttycom.h			(possible overlap)
 *   v	sundev/vuid_event.h		*overlap*
 *   v	sys/vcmd.h			*overlap*
 *
 * End of Registry
 */

