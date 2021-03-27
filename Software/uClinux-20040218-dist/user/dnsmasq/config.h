/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* Author's email: simon@thekelleys.org.uk */

#define VERSION "1.8"

#define FTABSIZ 20 /* max number of outstanding requests */
#ifdef EMBED
#define CACHESIZ 100
#else
#define CACHESIZ 300 /* default cache size */
#endif
#define MAXLIN 1024 /* line length in config files */
#define HOSTSFILE "/etc/config/hosts"
#define RESOLVFILE "/etc/config/resolv.conf"
#define RUNFILE "/var/run/dnsmasq.pid"
#define CHUSER "nobody"
#ifndef NO_IPV6
#define IP6INTERFACES "/proc/net/if_inet6"
#endif

/* Follows system specific switches. If you run on a 
   new system, you may want to edit these. 
   May replace this with Autoconf one day. 
  
HAVE_LINUX_IPV6_PROC
   define this to do IPv6 interface discovery using
   proc/net/if_inet6 ala LINUX. 

HAVE_GETOPT_LONG
   define this if you have GNU libc or GNU getopt. 

HAVE_BROKEN_SOCKADDR_IN6
   we provide our own declaration of sockaddr_in6,
   since old versions of glibc are broken. 

HAVE_ARC4RANDOM
   define this if you have arc4random() to get better security from DNS spoofs
   by using really random ids (OpenBSD) 

HAVE_SOCKADDR_SA_LEN
   define this if struct sockaddr has sa_len field (*BSD) 

NOTES:
   For Linux you should define 
      HAVE_LINUX_IPV6_PROC 
      HAVE_GETOPT_LONG
   you should NOT define 
      HAVE_ARC4RANDOM
      HAVE_SOCKADDR_SA_LEN
   and you MAY have to define 
     HAVE_BROKEN_SOCKADDR_IN6 - if you have an old libc6.

   For *BSD systems you should define 
     HAVE_SOCKADDR_SA_LEN
   you should NOT define  
     HAVE_LINUX_IPV6_PROC 
     HAVE_BROKEN_SOCKADDR_IN6
   and you MAY define  
     HAVE_ARC4RANDOM - OpenBSD and FreeBSD 
     HAVE_GETOPT_LONG - only if you link GNU getopt. 
*/


/* define this if you have GNU libc or GNU getopt. */
#ifdef __linux__
//#define HAVE_LINUX_IPV6_PROC
#define HAVE_GETOPT_LONG
#undef HAVE_ARC4RANDOM
#undef HAVE_SOCKADDR_SA_LEN
//#define HAVE_BROKEN_SOCKADDR_IN6
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__)
#undef HAVE_LINUX_IPV6_PROC
#undef HAVE_GETOPT_LONG
#define HAVE_ARC4RANDOM
#define HAVE_SOCKADDR_SA_LEN
#undef HAVE_BROKEN_SOCKADDR_IN6
#endif

#if defined(__NetBSD__)
#undef HAVE_LINUX_IPV6_PROC
#undef HAVE_GETOPT_LONG
#undef HAVE_ARC4RANDOM
#define HAVE_SOCKADDR_SA_LEN
#undef HAVE_BROKEN_SOCKADDR_IN6
#endif
 
/* env "LIBS=-lsocket -lnsl" make */
#if defined(__sun) || defined(__sun__)
#undef HAVE_LINUX_IPV6_PROC
#undef HAVE_GETOPT_LONG
#undef HAVE_ARC4RANDOM
#undef HAVE_SOCKADDR_SA_LEN
#undef HAVE_BROKEN_SOCKADDR_IN6
#endif
