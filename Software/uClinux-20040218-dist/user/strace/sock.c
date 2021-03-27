/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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
 *	$Id: sock.c,v 1.2 1999/05/09 00:29:59 wichert Exp $
 */

#include "defs.h"

#ifdef linux
#include <sys/socket.h>
#else
#include <sys/sockio.h>
#endif

#if defined(ALPHA) || defined(SH)
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#elif defined(HAVE_IOCTLS_H)
#include <ioctls.h>
#endif
#endif

int
sock_ioctl(tcp, code, arg)
struct tcb *tcp;
long code, arg;
{
	if (entering(tcp))
		return 0;

	switch (code) {
#ifdef SIOCSHIWAT
	case SIOCSHIWAT:
#endif
#ifdef SIOCGHIWAT
	case SIOCGHIWAT:
#endif
#ifdef SIOCSLOWAT
	case SIOCSLOWAT:
#endif
#ifdef SIOCGLOWAT
	case SIOCGLOWAT:
#endif
#ifdef FIOSETOWN
	case FIOSETOWN:
#endif
#ifdef FIOGETOWN
	case FIOGETOWN:
#endif
#ifdef SIOCSPGRP
	case SIOCSPGRP:
#endif
#ifdef SIOCGPGRP
	case SIOCGPGRP:
#endif
#ifdef SIOCATMARK
	case SIOCATMARK:
#endif
		printnum(tcp, arg, ", %#d");
		return 1;
	default:
		return 0;
	}
}
