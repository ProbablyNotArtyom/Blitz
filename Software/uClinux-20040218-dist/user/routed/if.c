/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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

/*
 * From: @(#)if.c	5.6 (Berkeley) 6/1/90
 * From: @(#)if.c	8.1 (Berkeley) 6/5/93
 */
char if_rcsid[] = 
  "$Id: if.c,v 1.5 1999/08/01 19:19:16 dholland Exp $";

/*
 * Routing Table Management Daemon
 */

#include "defs.h"

extern	struct interface *ifnet;

static
int
same(struct sockaddr *a1, struct sockaddr *a2)
{
	return !memcmp(a1->sa_data, a2->sa_data, 14);
}

/*
 * Find the interface with address addr.
 */

struct interface *if_ifwithaddr(struct sockaddr *addr)
{
	struct interface *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_REMOTE)
			continue;
		if (ifp->int_addr.sa_family != addr->sa_family)
			continue;
		if (same(&ifp->int_addr, addr))
			break;
		if ((ifp->int_flags & IFF_BROADCAST) &&
		    same(&ifp->int_broadaddr, addr))
			break;
	}
	return (ifp);
}

/*
 * Find the point-to-point interface with destination address addr.
 */
struct interface *if_ifwithdstaddr(struct sockaddr *addr)
{
	struct interface *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if ((ifp->int_flags & IFF_POINTOPOINT) == 0)
			continue;
		if (same(&ifp->int_dstaddr, addr))
			break;
	}
	return (ifp);
}

/*
 * Find the interface on the network 
 * of the specified address.
 */

struct interface *if_ifwithnet(struct sockaddr *addr)
{
	struct interface *ifp;
	int af = addr->sa_family;
	int (*netmatch)(struct sockaddr *, struct sockaddr *);

	if (af >= af_max)
		return (0);
	netmatch = afswitch[af].af_netmatch;
	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_REMOTE)
			continue;
		if (af != ifp->int_addr.sa_family)
			continue;
		if ((*netmatch)(addr, &ifp->int_addr))
			break;
	}
	return (ifp);
}

/*
 * Find an interface from which the specified address
 * should have come from.  Used for figuring out which
 * interface a packet came in on -- for tracing.
 */
struct interface *if_iflookup(struct sockaddr *addr)
{
	struct interface *ifp, *maybe;
	int af = addr->sa_family;
	int (*netmatch)(struct sockaddr *, struct sockaddr *);

	if (af >= af_max)
		return (0);
	maybe = 0;
	netmatch = afswitch[af].af_netmatch;
	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_addr.sa_family != af)
			continue;
		if (same(&ifp->int_addr, addr))
			break;
		if ((ifp->int_flags & IFF_BROADCAST) &&
		    same(&ifp->int_broadaddr, addr))
			break;
		if ((ifp->int_flags & IFF_POINTOPOINT) &&
		    same(&ifp->int_dstaddr, addr))
			break;
		if (maybe == 0 && (*netmatch)(addr, &ifp->int_addr))
			maybe = ifp;
	}
	if (ifp == 0)
		ifp = maybe;
	return (ifp);
}
