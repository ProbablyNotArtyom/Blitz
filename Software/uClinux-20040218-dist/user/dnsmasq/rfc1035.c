/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include "dnsmasq.h"

static int extract_name(HEADER *header, int plen, unsigned char **pp, char *name)
{
  char *cp = name;
  unsigned char *p = *pp, *p1 = NULL;
  unsigned int j, l, hops = 0;
    
  while ((l = *p++))
    {
      if ((l & 0xc0) == 0xc0) /* pointer */
	{ 
	  if (p - (unsigned char *)header + 1 >= plen)
	    return 0;
	      
	  /* get offset */
	  l = (l&0x3f) << 8;
	  l |= *p++;
	  if (l >= (unsigned int)plen) 
	    return 0;
	  
	  if (!p1) /* first jump, save location to go back to */
	    p1 = p;
	      
	  hops++; /* break malicious infinite loops */
	  if (hops > 255)
	    return 0;
	  
	  p = l + (unsigned char *)header;
	}
      else
	{
	  if (cp-name+l+1 >= MAXDNAME)
	    return 0;
	  if (p - (unsigned char *)header + l >= (unsigned int)plen)
	    return 0;
	  for(j=0; j<l; j++, p++)
	    *cp++ = tolower(*p);
	  *cp++ = '.';
	}
      if (p - (unsigned char *)header >= plen)
	return 0;
    }
  *--cp = 0; /* overwrite last period */
  
  if (p1) /* we jumped via compression */
    p = p1;
  
  if (p - (unsigned char *)header + 4 > plen)
    return 0;

  *pp = p;
  return 1;
}

static int in_arpa_name_2_addr(char *namein, struct all_addr *addrp)
{
  int j;
  static char name[MAXDNAME];
  char  *cp1;
  unsigned char *addr = (unsigned char *)addrp;
  char *lastchunk = NULL, *penchunk = NULL;
  
  memset(addrp, 0, sizeof(struct all_addr));

  /* turn name into a series of asciiz strings */
  /* j counts no of labels */
  for(j = 1,cp1 = name; *namein; cp1++, namein++)
    if (*namein == '.')
      {
	penchunk = lastchunk;
        lastchunk = cp1 + 1;
	*cp1 = 0;
	j++;
      }
    else
      *cp1 = *namein;
  
  *cp1 = 0;

  if (j<3)
    return 0;

  if (strcmp(lastchunk, "arpa") == 0 && strcmp(penchunk, "in-addr") == 0)
    {
      /* IP v4 */
      /* address arives as a name of the form
	 www.xxx.yyy.zzz.in-addr.arpa
	 some of the low order address octets might be missing
	 and should be set to zero. */
      for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	{
	  /* check for digits only (weeds out things like
	     50.0/24.67.28.64.in-addr.arpa which are used 
	     as CNAME targets according to RFC 2317 */
	  char *cp;
	  for (cp = cp1; *cp; cp++)
	    if (!isdigit((int)*cp))
	      return 0;
	  
	  addr[3] = addr[2];
	  addr[2] = addr[1];
	  addr[1] = addr[0];
	  addr[0] = atoi(cp1);
	}

      return F_IPV4;
    }
  else if ((strcmp(lastchunk, "arpa") == 0 || strcmp(lastchunk, "int") == 0) &&
	   strcmp(penchunk, "ip6") == 0)
    {
      /* IP v6 */
      /* address arrives as 0.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.ip6.int
         or 0.1.2.3.4.5.6.7.8.9.a.b.c.d.e.f.ip6.arpa
      */
      for (cp1 = name; cp1 != penchunk; cp1 += strlen(cp1)+1)
	{
	  if (*(cp1+1) || !isxdigit((int)*cp1))
	    return 0;
	  
	  for (j = sizeof(struct all_addr)-1; j>0; j--)
	    addr[j] = (addr[j] >> 4) | (addr[j-1] << 4);
	  addr[0] = (addr[0] >> 4) | (strtol(cp1, NULL, 16) << 4);
	}

      return F_IPV6;
    }
  
  return 0;
}

static unsigned char *skip_questions(HEADER *header, int plen)
{
  int q, qdcount = ntohs(header->qdcount);
  unsigned char *ansp = (unsigned char *)(header+1);

  for (q=0; q<qdcount; q++)
    {
      while (1)
	{
          if (ansp - (unsigned char *)header >= plen)
	    return NULL;
	  if (((*ansp) & 0xc0) == 0xc0) /* pointer for name compression */
	    {
              ansp += 2;	
	      break;
	    }
	  else if (*ansp) 
	    { /* another segment */
	      ansp += (*ansp) + 1;
	    }
	  else            /* end */
	    {
	      ansp++;
	      break;
	    }
	}
      ansp += 4; /* class and type */
    }
  if (ansp - (unsigned char *)header > plen) 
     return NULL;
  
  return ansp;
}

/* is addr in the non-globally-routed IP space? */ 
static int private_net(struct all_addr *addrp) 
{
  struct in_addr addr = *(struct in_addr *)addrp;
  if (inet_netof(addr) == 0xA ||
      (inet_netof(addr) >= 0xAC10 && inet_netof(addr) < 0xAC20) ||
      (inet_netof(addr) >> 8) == 0xC0A8) 
    return 1;
  else 
    return 0;
}
 
static unsigned char *add_text_record(unsigned int nameoffset, unsigned char *p, 
				      unsigned long ttl, unsigned short pref, 
				      unsigned short type, char *name)
{
  unsigned char *sav, *cp;
  int j;
  
  PUTSHORT(nameoffset | 0xc000, p); 
  PUTSHORT(type, p);
  PUTSHORT(C_IN, p);
  PUTLONG(ttl, p); /* TTL */
  
  sav = p;
  PUTSHORT(0, p); /* dummy RDLENGTH */

  if (pref)
    PUTSHORT(pref, p);

  while (*name) 
    {
      cp = p++;
      for (j=0; *name && (*name != '.'); name++, j++)
	*p++ = *name;
      *cp = j;
      if (*name)
	name++;
    }
  *p++ = 0;
  j = p - sav - 2;
  PUTSHORT(j, sav); /* Real RDLENGTH */
  
  return p;
}

/* On receiving an NXDOMAIN or NODATA reply, determine which names are known
   not to exist for negative caching */
static void extract_neg_addrs(HEADER *header, int qlen) 
{
  static char name[MAXDNAME];
  unsigned char *p;
  int i, found_soa = 0;
  int qtype, qclass, rdlen;
  unsigned long ttl, minttl = 0;
  time_t now = time(NULL);
  
  /* there may be more than one question with some questions
     answered. We don't generate negative entries from those. */
  if (ntohs(header->ancount) != 0)
    return;
  
  if (!(p = skip_questions(header, qlen)))
    return; /* bad packet */
  
  /* we first need to find SOA records, to get min TTL, then we
     add a NEG cache entry for each question. */

  for (i=0; i<ntohs(header->nscount); i++)
    {
      if (!extract_name(header, qlen, &p, name))
	return; /* bad packet */

      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p);
       
      if ((qclass == C_IN) && (qtype == T_SOA))
	{
	  int dummy;
	  /* MNAME */
	  if (!extract_name(header, qlen, &p, name))
	    return;
	  /* RNAME */
	  if (!extract_name(header, qlen, &p, name))
	    return;
	  GETLONG(dummy, p); /* SERIAL */
	  GETLONG(dummy, p); /* REFRESH */
	  GETLONG(dummy, p); /* RETRY */
	  GETLONG(dummy, p); /* EXPIRE */
	  if (!found_soa)
	    {
	      found_soa = 1;
	      minttl = ttl;
	    }
	  else if (ttl < minttl)
	    minttl = ttl;
	  GETLONG(ttl, p); /* minTTL */
	  if (ttl < minttl)
	    minttl = ttl;
	}
      else
	p += rdlen;

      if (p - (unsigned char *)header > qlen)
	return; /* bad packet */
    }
  
  if (!found_soa)
    return; /* failed to find SOA */
  
  p = (unsigned char *)(header+1);
  
  cache_mark_all_old();
  
  for (i=0; i<ntohs(header->qdcount); i++)
    {
      if (!extract_name(header, qlen, &p, name))
	return; /* bad packet */
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      
      if (qclass == C_IN && qtype == T_A) 
	{
	  cache_remove_old_name(name, now, F_IPV4);
	  cache_name_insert(name, NULL, now, minttl, F_IPV4);
	} 
      
      if (qclass == C_IN && qtype == T_AAAA) 
	{
	  cache_remove_old_name(name, now, F_IPV6);
	  cache_name_insert(name, NULL, now, minttl, F_IPV6);
	}
    }
}

void extract_addresses(HEADER *header, int qlen)
{
  static char name[MAXDNAME];
  unsigned char *p, *psave, *endrr;
  int qtype, qclass, rdlen;
  unsigned long ttl;
  int i;
  time_t now = time(NULL);

  /* detect errors or NODATA packets and cache negative answer */
  if (header->rcode == NXDOMAIN || ntohs(header->ancount) == 0)
    extract_neg_addrs(header, qlen);
  
   /* skip over questions */
  if (!(p = skip_questions(header, qlen)))
    return; /* bad packet */

  /* Strategy: mark all entries as old and suitable for replacement.
     new entries made here are not removed while processing this packet.
     Makes multiple addresses per name work. */
  
  cache_mark_all_old();
  
  psave = p;
  
  for (i=0; i<ntohs(header->ancount); i++)
    {
      if (!extract_name(header, qlen, &p, name))
	return; /* bad packet */

      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);
      GETLONG(ttl, p);
      GETSHORT(rdlen, p);
	
      endrr = p + rdlen;
      if (endrr - (unsigned char *)header > qlen)
	return; /* bad packet */
      
      if (qclass != C_IN)
	{
	  p = endrr;
	  continue;
	}

      if (qtype == T_A)
	{
	  /* A record. */
	  cache_remove_old_name(name, now, F_IPV4);
	  cache_name_insert(name, (struct all_addr *)p, now, ttl, F_IPV4);
	}
      else if (qtype == T_AAAA)
	{
	  /* IPV6 address record. */
	  cache_remove_old_name(name, now, F_IPV6);
	  cache_name_insert(name, (struct all_addr *)p, now, ttl, F_IPV6);
	} 
      else if (qtype == T_PTR)
	{
	  /* PTR record */
	  struct all_addr addr;
	  int name_encoding = in_arpa_name_2_addr(name, &addr);
	  if (name_encoding)
	    {
	      if (!extract_name(header, qlen, &p, name))
		return; /* bad packet */
	      cache_remove_old_addr(&addr, now, name_encoding);
	      cache_addr_insert(name, &addr, now, ttl, name_encoding); 
	    }
	}
      else if (qtype == T_CNAME)
	{
	  /* CNAME, search whole answer section again */
	  /* name is name in CNAME
	     rname is target of CNAME
	     newname is name of record we are processing */
	  static char rname[MAXDNAME];  static char newname[MAXDNAME];
	  unsigned char *endrr1;
	  unsigned long cttl;
	  int j;

	  /* CNAME target */
	  if (!extract_name(header, qlen, &p, rname))
	    return; /* bad packet */
	  
	  p = psave; /* rewind p */
	  for (j=0; j<ntohs(header->ancount); j++)
	    {
	      if (!extract_name(header, qlen, &p, newname))
		return;
	      
	      GETSHORT(qtype, p); 
	      GETSHORT(qclass, p);
	      GETLONG(cttl, p);
	      GETSHORT(rdlen, p);
	      
	      endrr1 = p+rdlen;
	      if (endrr1 - (unsigned char *)header > qlen)
		return; /* bad packet */

	      /* is this RR name same as target of CNAME */
	      if ((qclass != C_IN) || (strcmp(rname, newname) != 0))
		{ 
		  p = endrr1;
		  continue;
		}

	      /* match, use name of CNAME, data from this RR
		 use min TTL of two */

	      if (ttl < cttl)
		cttl = ttl;

	      if (qtype == T_A)
		{
		  /* A record. */
		  cache_remove_old_name(name, now, F_IPV4);
		  cache_name_insert(name, (struct all_addr *)p, now, cttl, F_IPV4);
		}
	      else if (qtype == T_AAAA)
		{
		  /* IPV6 address record. */
		  cache_remove_old_name(name, now, F_IPV6);
		  cache_name_insert(name, (struct all_addr *)p, now, cttl, F_IPV6);
		} 
	      else if (qtype == T_PTR)
		{
		  /* PTR record extract address from CNAME name */
		  struct all_addr addr;
		  int name_encoding = in_arpa_name_2_addr(name, &addr);
		  if (name_encoding)
		    {
		      if (!extract_name(header, qlen, &p, newname))
			return; /* bad packet */
		      cache_remove_old_addr(&addr, now, name_encoding);
		      cache_addr_insert(newname, &addr, now, cttl, name_encoding);
		    } 
		}
	      p = endrr1;
	    }
	} 
      p = endrr;
    }
}

/* return zero if we can't answer from cache, or packet size if we can */
int answer_request(HEADER *header, char *limit, int qlen,
		   char *mxname, char *mxtarget, int boguspriv, int filterwin2k)
{
  unsigned char *p, *ansp;
  int qtype, qclass, is_arpa;
  static char name[MAXDNAME];
  struct all_addr addr;
  unsigned int nameoffset;
  int q, qdcount = ntohs(header->qdcount); 
  int ans, anscount = 0;
  time_t now = time(NULL);
  struct crec *crecp;

  if (!qdcount || header->opcode != QUERY )
    return 0;

  /* determine end of question section (we put answers there) */
  if (!(ansp = skip_questions(header, qlen)))
    return 0; /* bad packet */
   
  /* now process each question, answers go in RRs after the question */
  p = (unsigned char *)(header+1);
  
  for (q=0; q<qdcount; q++)
    {
      /* save pointer to name for copying into answers */
      nameoffset = p - (unsigned char *)header;

      /* now extract name as .-concatenated string into name */
      if (!extract_name(header, qlen, &p, name))
	return 0; /* bad packet */
      
      /* see if it's w.z.y.z.in-addr.arpa format */

      is_arpa = in_arpa_name_2_addr(name, &addr);
      
      GETSHORT(qtype, p); 
      GETSHORT(qclass, p);

      if (qclass != C_IN)
	return 0; /* we can't answer non-inet queries */
  
      ans = 0; /* have we answered this question */
      
      if (filterwin2k && (qtype == T_SOA || qtype == T_SRV))
	ans = 1;
      
      if (((qtype == T_PTR) || (qtype == T_ANY)) && is_arpa)
	{
	  crecp = NULL;
	  while ((crecp = cache_find_by_addr(crecp, &addr, now, is_arpa)))
	    { 
	      unsigned long ttl = (crecp->flags & F_IMMORTAL) ? 0 : crecp->ttd - now;
	      
	      /* don't answer wildcard queries with data not from /etc/hosts 
		 or dhcp leases */
	      if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
		return 0;
	      
	      ans = 1;
	      ansp = add_text_record(nameoffset, ansp, ttl, 0, T_PTR, crecp->name);
	      anscount++;
	      
	      /* if last answer exceeded packet size, give up */
	      if (((unsigned char *)limit - ansp) < 0)
		return 0;
	    }
	  
	  /* if not in cache, enabled and private IPV4 address, fake up answer */
	  if (ans == 0 && is_arpa == F_IPV4 && boguspriv && private_net(&addr))
	    {
	      struct in_addr addr4 = *((struct in_addr *)&addr);
	      ansp = add_text_record(nameoffset, ansp, 0, 0, T_PTR, inet_ntoa(addr4));  
	      anscount++;
	      ans = 1;

	      if (((unsigned char *)limit - ansp) < 0)
		return 0;
	    }
	}
	  
      if (((qtype == T_A) || qtype == T_ANY) && !is_arpa)
	{
	  /* T_ANY queries for hostnames with underscores are spam
             from win2k - don't forward them. */
	  if (filterwin2k && qtype == T_ANY && (strchr(name, '_') != NULL))
	    ans = 1;
	  else
	    { 
	      crecp = NULL;
	      while ((crecp = cache_find_by_name(crecp, name, now, F_IPV4)))
		{ 
		  unsigned long ttl = (crecp->flags & F_IMMORTAL) ? 0 : crecp->ttd - now;
		  /* don't answer wildcard queries with data not from /etc/hosts
		     or DHCP leases */
		  if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
		    return 0;
		  
		  /* If we have negative cache entry, it's OK
		     to return no answer. */
		  ans = 1;
		  
		  if (!(crecp->flags & F_NEG))
		    {
		      /* copy question as first part of answer (use compression) */
		      PUTSHORT(nameoffset | 0xc000, ansp); 
		      PUTSHORT(T_A, ansp);
		      PUTSHORT(C_IN, ansp);
		      PUTLONG(ttl, ansp); /* TTL */
		      
		      PUTSHORT(INADDRSZ, ansp);
		      memcpy(ansp, &crecp->addr, INADDRSZ);
		      ansp += INADDRSZ;
		      anscount++;
		      
		      if (((unsigned char *)limit - ansp) < 0)
			return 0;
		    }
		  
		}
	    }
	}

      if (((qtype == T_AAAA) || qtype == T_ANY) && !is_arpa)
	{
	  /* T_ANY queries for hostnames with underscores are spam
             from win2k - don't forward them. */
	  if (filterwin2k && qtype == T_ANY && (strchr(name, '_') != NULL))
	    ans = 1;
	  else
	    { 
	      crecp = NULL;
	      while ((crecp = cache_find_by_name(crecp, name, now, F_IPV6)))
		{ 
		  unsigned long ttl = (crecp->flags & F_IMMORTAL) ? 0 : crecp->ttd - now;
		  /* don't answer wildcard queries with data not from /etc/hosts
		     or DHCP leases */
		  if (qtype == T_ANY && !(crecp->flags & (F_HOSTS | F_DHCP)))
		    return 0;
		  
		  /* If we have negative cache entry, it's OK
		     to return no answer. */
		  ans = 1;
		  
		  if (!(crecp->flags & F_NEG))
		    {
		      /* copy question as first part of answer (use compression) */
		      PUTSHORT(nameoffset | 0xc000, ansp); 
		      PUTSHORT(T_AAAA, ansp);
		      PUTSHORT(C_IN, ansp);
		      PUTLONG(ttl, ansp); /* TTL */
		      
		      PUTSHORT(IN6ADDRSZ, ansp);
		      memcpy(ansp, &crecp->addr, IN6ADDRSZ);
		      ansp += IN6ADDRSZ;
		      anscount++;
		      
		      if (((unsigned char *)limit - ansp) < 0)
			return 0;
		    }
		}
	    }
	}
      
      if (((qtype == T_MX) || (qtype == T_ANY)) && !is_arpa && 
	  mxname && (strcmp(name, mxname) == 0))
	{
	  ansp = add_text_record(nameoffset, ansp, 0, 1, T_MX, mxtarget);
	  anscount++;
	  ans = 1;
	  
	  if (((unsigned char *)limit - ansp) < 0)
	    return 0;
	}
      
      if (!ans)
	return 0; /* failed to answer a question */

    }
  
  /* done all questions, set up header and return length of result */
  header->qr = 1; /* response */
  header->aa = 0; /* authoritive - never */
  header->ra = 1; /* recursion if available */
  header->tc = 0; /* truncation */
  header->rcode = NOERROR; /* no error */
  header->ancount = htons(anscount);
  header->nscount = htons(0);
  header->arcount = htons(0);
  return ansp - (unsigned char *)header;
}





