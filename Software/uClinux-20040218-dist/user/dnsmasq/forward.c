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

#include "dnsmasq.h"

static struct frec *ftab;

static struct frec *get_new_frec(time_t now);
static struct frec *lookup_frec(unsigned short id);
static struct frec *lookup_frec_by_sender(unsigned short id,
					  union mysockaddr *addr);
static unsigned short get_id(void);

/* May be called more than once. */
void forward_init(int first)
{
  int i;

  if (first)
    ftab = safe_malloc(FTABSIZ*sizeof(struct frec));
  for (i=0; i<FTABSIZ; i++)
    ftab[i].new_id = 0;
}

/* returns new last_server */	
struct server *forward_query(int udpfd, int peerfd, int peerfd6,
			     union mysockaddr *udpaddr, HEADER *header, 
			     int plen, int strict_order, char *dnamebuff, 
			     struct server *servers, struct server *last_server)
{
  time_t now = time(NULL);
  struct frec *forward;
  char *domain = NULL;
  struct server *serv;
  int gotname = extract_request(header, (unsigned int)plen, dnamebuff);

  /* may be  recursion not speced or no servers available. */
  if (!header->rd || !servers)
    forward = NULL;
  else if ((forward = lookup_frec_by_sender(ntohs(header->id), udpaddr)))
    {
      /* retry on existing query, send to next server */
      domain = forward->sentto->domain;
      if (!(forward->sentto = forward->sentto->next))
	forward->sentto = servers; /* at end of list, recycle */
      header->id = htons(forward->new_id);
    }
  else
    {
      /* new query, pick nameserver and send */
      forward = get_new_frec(now);
      
      /* If the query ends in the domain in one of our servers, set
	 domain to point to that name. We find the largest match to allow both
	 domain.org and sub.domain.org to exist. */
      
      if (gotname)
	{
	  unsigned int namelen = strlen(dnamebuff);
	  unsigned int matchlen = 0;
	  for (serv=servers; serv; serv=serv->next)
	    if (serv->domain)
	      {
		unsigned int domainlen = strlen(serv->domain);
		if (namelen >= domainlen &&
		    strcmp(dnamebuff + namelen - domainlen, serv->domain) == 0 &&
		    domainlen > matchlen)
		  {
		    domain = serv->domain;
		    matchlen = domainlen;
		  }
	      }
	}
      
      /* In strict_order mode, or when using domain specific servers
	 always try servers in the order specified in resolv.conf,
	 otherwise, use the one last known to work. */
      
      if (domain || strict_order)
	forward->sentto = servers;
      else
	forward->sentto = last_server;
	
      forward->source = *udpaddr;
      forward->new_id = get_id();
      forward->fd = udpfd;
      forward->orig_id = ntohs(header->id);
      header->id = htons(forward->new_id);
    }
  
  /* check for send errors here (no route to host) 
     if we fail to send to all nameservers, send back an error
     packet straight away (helps modem users when offline)  */
  
  if (forward)
    {
      struct server *firstsentto = forward->sentto;
            
      while (1)
	{ 
	  int af = forward->sentto->addr.sa.sa_family; 
	  int fd = af == AF_INET ? peerfd : peerfd6;
	  
	  /* only send to servers dealing with our domain.
	     domain may be NULL, in which case server->domain 
	     must be NULL also. */
	  
	  if ((!domain && !forward->sentto->domain) ||
	      (domain && forward->sentto->domain && strcmp(domain, forward->sentto->domain) == 0))
	    {
	      if (sendto(fd, (char *)header, plen, 0,
			 &forward->sentto->addr.sa,
			 sa_len(&forward->sentto->addr)) != -1)
		{
		  if (af == AF_INET)
		    log_query(F_SERVER | F_IPV4 | F_FORWARD, gotname ? dnamebuff : "query", 
			      (struct all_addr *)&forward->sentto->addr.in.sin_addr);
#ifdef HAVE_IPV6
		  else
		    log_query(F_SERVER | F_IPV6 | F_FORWARD, gotname ? dnamebuff : "query", 
			      (struct all_addr *)&forward->sentto->addr.in6.sin6_addr);
#endif
		  /* for no-domain, dont't update last_server */
		  return domain ? last_server : (forward->sentto->next ? forward->sentto->next : servers);
		}
	    } 
	  
	  if (!(forward->sentto = forward->sentto->next))
	    forward->sentto = servers;
	  
	  /* check if we tried all without success */
	  if (forward->sentto == firstsentto)
	    break;
	}
      
      /* could not send on, prepare to return */ 
      header->id = htons(forward->orig_id);
      forward->new_id = 0; /* cancel */
    }	  
  
  /* could not send on, return empty answer */
  header->qr = 1; /* response */
  header->aa = 0; /* authoritive - never */
  header->ra = 1; /* recursion if available */
  header->tc = 0; /* not truncated */
  header->rcode = NOERROR; /* no error */
  header->ancount = htons(0); /* no answers */
  header->nscount = htons(0);
  header->arcount = htons(0);
  sendto(udpfd, (char *)header, plen, 0, &udpaddr->sa, sa_len(udpaddr));

  return last_server;
}

/* returns new last_server */
struct server *reply_query(int fd, char *packet, char *dnamebuff, struct server *last_server)
{
  /* packet from peer server, extract data for cache, and send to
     original requester */
  struct frec *forward;
  HEADER *header;
  int n = recv(fd, packet, PACKETSZ, 0);
  
  header = (HEADER *)packet;
  if (n >= (int)sizeof(HEADER) && header->qr)
    {
      if ((forward = lookup_frec(ntohs(header->id))))
	{
	  if (header->rcode == NOERROR || header->rcode == NXDOMAIN)
	    {
	      if (!forward->sentto->domain)
		last_server = forward->sentto; /* known good */
	      if (header->opcode == QUERY)
		extract_addresses(header, (unsigned int)n, dnamebuff);
	    }
	  
	  header->id = htons(forward->orig_id);
	  /* There's no point returning an upstream reply marked as truncated,
	     since that will prod the resolver into moving to TCP - which we
	     don't support. */
	  header->tc = 0; /* goodbye truncate */
	  sendto(forward->fd, packet, n, 0, 
		 &forward->source.sa, sa_len(&forward->source));
	  forward->new_id = 0; /* cancel */
	}
    }

  return last_server;
}
      

static struct frec *get_new_frec(time_t now)
{
  int i;
  struct frec *oldest = &ftab[0];
  time_t oldtime = now;

  for(i=0; i<FTABSIZ; i++)
    {
      struct frec *f = &ftab[i];
      if (f->time <= oldtime)
	{
	  oldtime = f->time;
	  oldest = f;
	}
      if (f->new_id == 0)
	{
	  f->time = now;
	  return f;
	}
    }

  /* table full, use oldest */

  oldest->time = now;
  return oldest;
}
 
static struct frec *lookup_frec(unsigned short id)
{
  int i;
  for(i=0; i<FTABSIZ; i++)
    {
      struct frec *f = &ftab[i];
      if (f->new_id == id)
	return f;
    }
  return NULL;
}

static struct frec *lookup_frec_by_sender(unsigned short id,
					  union mysockaddr *addr)
{
  int i;
  for(i=0; i<FTABSIZ; i++)
    {
      struct frec *f = &ftab[i];
      if (f->new_id &&
	  f->orig_id == id && 
	  sockaddr_isequal(&f->source, addr))
	return f;
    }
  return NULL;
}


/* return unique random ids between 1 and 65535 */
static unsigned short get_id(void)
{
  unsigned short ret = 0;

  while (ret == 0)
    {
      ret = rand16();
      
      /* scrap ids already in use */
      if ((ret != 0) && lookup_frec(ret))
	ret = 0;
    }

  return ret;
}





