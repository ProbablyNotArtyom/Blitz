/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/* See RFC1035 for details of the protocol this code talks. */

/* Author's email: simon@thekelleys.org.uk */

#include "config.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#if defined(__sun) || defined(__sun__)
#include <sys/sockio.h>
#endif
#include <sys/time.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>
#include <syslog.h>
#ifdef HAVE_GETOPT_LONG
#  include <getopt.h>
#endif
#include <time.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

struct all_addr {
  union {
    struct in_addr addr4;
#ifndef NO_IPV6
    struct in6_addr addr6;
#endif
  } addr;
};


struct crec { 
  char *name;
  struct all_addr addr;
  struct crec *next, *prev;
  time_t ttd; /* time to die */
  int flags;
  int nameSize;
};

#define F_IMMORTAL  1
#define F_NEW       2
#define F_REVERSE   4
#define F_FORWARD   8
#define F_DHCP      16 
#define F_NEG       32       
#define F_HOSTS     64
#define F_IPV4      128
#define F_IPV6      256

#ifndef AF_INET6
#define AF_INET6	10	/* IP version 6			*/
#endif

/* struct sockaddr is not large enough to hold any address,
   and specifically not big enough to hold and IPv6 address.
   Blech. Roll our own. */
union mysockaddr {
  struct sockaddr sa;
  struct sockaddr_in in;
#ifndef NO_IPV6 
#ifdef HAVE_BROKEN_SOCKADDR_IN6
  /* early versions of glibc don't include sin6_scope_id in sockaddr_in6
     but latest kernels _require_ it to be set. The choice is to have
     dnsmasq fail to compile on back-level libc or fail to run
     on latest kernels with IPv6. Or to do this: sorry that it's so gross. */
  struct my_sockaddr_in6 {
    sa_family_t     sin6_family;    /* AF_INET6 */
    uint16_t        sin6_port;      /* transport layer port # */
    uint32_t        sin6_flowinfo;  /* IPv6 traffic class & flow info */
    struct in6_addr sin6_addr;      /* IPv6 address */
    uint32_t        sin6_scope_id;  /* set of interfaces for a scope */
  } in6;
#else
  struct sockaddr_in6 in6;
#endif
#endif
};

struct server {
  union mysockaddr addr;
  struct server *next; /* circle */
};

/* linked list of all the interfaces in the system and 
   the sockets we have bound to each one. */
struct irec {
  union mysockaddr addr;
  int fd;
  struct irec *next;
};

struct iname {
  char *name;
  union mysockaddr addr;
  struct iname *next;
  int found;
};

struct frec {
  union mysockaddr source;
  struct server *sentto;
  unsigned short orig_id, new_id;
  int fd;
  time_t time;
  int response_count;
  HEADER *last_header; /*a pointer into the header*/
  void *buffer;		/*The buffer for caching data*/
  int pack_size;	/*the size of the received packet*/
  int buf_size;		/*the size of the allocated buffer*/
};

/* cache.c */
void cache_init(int cachesize);
void cache_mark_all_old(void);
struct crec *cache_find_by_addr(struct crec *crecp,
				struct all_addr *addr, time_t now, int prot);
struct crec *cache_find_by_name(struct crec *crecp, 
				char *name, time_t now, int prot);
void cache_mark_all_old(void);
void cache_remove_old_name(char *name, time_t now, int prot);
void cache_remove_old_addr(struct all_addr *addr, time_t now, int prot);
void cache_insert(struct crec *crecp);
void cache_name_insert(char *name, struct all_addr *addr, 
		       time_t now, unsigned long ttl, int prot);
void cache_addr_insert(char *name, struct all_addr *addr,
		       time_t now, unsigned long ttl, int prot);
void cache_reload(int use_hosts, int cachesize);
struct crec *cache_clear_dhcp(void);
void dump_cache(int daemon, int size);

/* rfc1035.c */
void extract_addresses(HEADER *header, int qlen);
int answer_request(HEADER *header, char *limit, int qlen, char *mxname, 
		   char *mxtarget, int boguspriv, int filterwin2k);

/* dhcp.c */
void load_dhcp(char *file, char *suffix);

#define DNS_WORST REFUSED+1
