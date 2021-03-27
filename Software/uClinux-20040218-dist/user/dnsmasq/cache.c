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

static struct crec *cache_head, *cache_tail;
static int cache_inserted, cache_live_freed;

static struct crec *cache_get_free(time_t now);
static void cache_free(struct crec *crecp);
static void cache_unlink (struct crec *crecp);

void cache_init(int cachesize)
{
  struct crec *crecp;
  int i;

  if (!(crecp = malloc(cachesize*sizeof(struct crec))))
    {
      fprintf(stderr, "dnsmasq: could not get memory");
      exit(1);
    }

  cache_head = NULL;
  cache_tail = crecp;
  for (i=0; i<cachesize; i++, crecp++) {
	crecp->nameSize = 0;
	crecp->name = NULL;
    cache_insert(crecp);
  }
  cache_inserted = cache_live_freed = 0;
}


/* get a cache entry to re-cycle from the tail of the list (oldest entry) */
static struct crec *cache_get_free(time_t now)
{
  struct crec *ret = cache_tail;

  cache_tail = cache_tail->prev;
  cache_tail->next = NULL;

  /* just push immortal entries back to the top and try again. */
  if (ret->flags & (F_HOSTS | F_DHCP))
    {
      cache_insert(ret);
      return cache_get_free(now);
    }

  /* record still-live cache entries we have to blow away */
  if ((ret->flags & (F_FORWARD | F_REVERSE)) &&
      ret->ttd >= now)
    cache_live_freed++;

  cache_inserted++;
	
  /* The next bit ensures that if there is more than one entry
     for a name or address, they all get removed at once */
  if (ret->flags & F_FORWARD)
    cache_remove_old_name(ret->name, now, ret->flags & (F_IPV4 | F_IPV6));
  else if (ret->flags & F_REVERSE)
    cache_remove_old_addr(&ret->addr, now, ret->flags & (F_IPV4 | F_IPV6));
    
  return ret;
}

/* Note that it's OK to free slots with F_DHCP set */
/* They justfloat around unused until the new dhcp.leases load */
static void cache_free(struct crec *crecp)
{
  cache_unlink(crecp);
  crecp->flags &= ~F_FORWARD;
  crecp->flags &= ~F_REVERSE;
  cache_tail->next = crecp;
  crecp->prev = cache_tail;
  crecp->next = NULL;
  cache_tail = crecp;
}

/* insert a new cache entry at the head of the list (youngest entry) */
void cache_insert(struct crec *crecp)
{
  if (cache_head) /* check needed for init code */
    cache_head->prev = crecp;
  crecp->next = cache_head;
  crecp->prev = NULL;
  cache_head = crecp;
}

/* remove an arbitrary cache entry for promotion */ 
static void cache_unlink (struct crec *crecp)
{
  if (crecp->prev)
    crecp->prev->next = crecp->next;
  else
    cache_head = crecp->next;

  if (crecp->next)
    crecp->next->prev = crecp->prev;
  else
    cache_tail = crecp->prev;
}

void cache_mark_all_old(void)
{
  struct crec *crecp;
  
  for (crecp = cache_head; crecp; crecp = crecp->next)
    crecp->flags &= ~F_NEW;
}

void cache_remove_old_name(char *name, time_t now, int prot)
{
  struct crec *crecp = cache_head;
  
  while (crecp)
    {
      struct crec *tmp = crecp->next;
      if ((crecp->flags & F_FORWARD) && (crecp->flags & prot)) 
	{
	  if (strcmp(crecp->name, name) == 0 && 
	      !(crecp->flags & (F_HOSTS | F_NEW | F_DHCP)))
	    cache_free(crecp);
	  
	  if ((crecp->ttd < now) && !(crecp->flags & F_IMMORTAL))
	    cache_free(crecp);
	}
      crecp = tmp;
    }
  
}

void cache_remove_old_addr(struct all_addr *addr, time_t now, int prot)
{
  int addrlen = (prot == F_IPV6) ? IN6ADDRSZ : INADDRSZ;
  struct crec *crecp = cache_head;
  while (crecp)
    {
      struct crec *tmp = crecp->next;
      if ((crecp->flags & F_REVERSE) && (crecp->flags & prot))
	{
	  
	  if (memcmp(&crecp->addr, addr, addrlen) == 0 && 
	      !(crecp->flags & (F_HOSTS | F_NEW | F_DHCP )))
	    cache_free(crecp);
	  
	  /* remove expired entries too. */
	  if ((crecp->ttd < now) && !(crecp->flags & F_IMMORTAL))
	    cache_free(crecp);
	}
      crecp = tmp;
    }
  
}

void cache_name_insert(char *name, struct all_addr *addr, 
		       time_t now, unsigned long ttl, int prot)
{
  int addrlen = (prot == F_IPV6) ? IN6ADDRSZ : INADDRSZ;
  struct crec *crecp = cache_get_free(now);
  int len;
  crecp->flags = F_NEW | F_FORWARD | prot;
  if ((len = strlen(name)) >= crecp->nameSize) {
    if(crecp->name) 
      free(crecp->name);
    crecp->name = strdup(name);
    crecp->nameSize = len + 1;
  } else
    strcpy(crecp->name, name);
  if (addr)
    memcpy(&crecp->addr, addr, addrlen);
  else
    crecp->flags |= F_NEG;
  crecp->ttd = ttl + now;
  cache_insert(crecp);
}

void cache_addr_insert(char *name, struct all_addr *addr, 
		       time_t now, unsigned long ttl, int prot)
{
  struct crec *crecp = cache_get_free(now);
  int addrlen = (prot == F_IPV6) ? IN6ADDRSZ : INADDRSZ;
  int len;
  crecp->flags = F_NEW | F_REVERSE | prot;
  if ((len = strlen(name)) >= crecp->nameSize) {
    if (crecp->name)
      free(crecp->name);
	crecp->name = strdup(name);
	crecp->nameSize = len + 1;
  } else
    strcpy(crecp->name, name);
  memcpy(&crecp->addr, addr, addrlen);
  crecp->ttd = ttl + now;
  cache_insert(crecp);
}

struct crec *cache_find_by_name(struct crec *crecp, char *name, time_t now, int prot)
{
  if (crecp) /* iterating */
    {
      if (crecp->next && 
	  (crecp->next->flags & F_FORWARD) && 
	  (crecp->next->flags & prot) &&
	  strcmp(crecp->next->name, name) == 0)
	return crecp->next;
      else
	return NULL;
    }
  
  /* first search, look for relevant entries and push to top of list
     also free anything which has expired */
  
  crecp = cache_head;
  while (crecp)
    {
      struct crec *tmp = crecp->next;
      if ((crecp->flags & F_FORWARD) && 
	  (crecp->flags & prot) &&
	  (strcmp(crecp->name, name) == 0))
	{
	  if ((crecp->flags & F_IMMORTAL) || crecp->ttd > now)
	    {
	      cache_unlink(crecp);
	      cache_insert(crecp);
	    }
	  else
	    cache_free(crecp);
	}
      crecp = tmp;
    }

  /* if there's anything relevant, it will be at the head of the cache now. */

  if (cache_head && 
      (cache_head->flags & F_FORWARD) &&
      (cache_head->flags & prot) &&
      (strcmp(cache_head->name, name) == 0))
    return cache_head;

  return NULL;
}

struct crec *cache_find_by_addr(struct crec *crecp, struct all_addr *addr, 
				time_t now, int prot)
{
  int addrlen = (prot == F_IPV6) ? IN6ADDRSZ : INADDRSZ;
  if (crecp) /* iterating */
    {
      if (crecp->next && 
	  (crecp->next->flags & F_REVERSE) && 
	  (crecp->next->flags & prot) &&
	  memcmp(&crecp->next->addr, addr, addrlen) == 0)
	return crecp->next;
      else
	return NULL;
    }
  
  /* first search, look for relevant entries and push to top of list
     also free anything which has expired */
  
  crecp = cache_head;
  while (crecp)
    {
      struct crec *tmp = crecp->next;
      if ((crecp->flags & F_REVERSE) && 
	  (crecp->flags & prot) &&
	  memcmp(&crecp->addr, addr, addrlen) == 0)
	{	    
	  if ((crecp->flags & F_IMMORTAL) || crecp->ttd > now)
	    {
	      cache_unlink(crecp);
	      cache_insert(crecp);
	    }
	  else
	    cache_free(crecp);
	}
      crecp = tmp;
    }

  /* if there's anything relevant, it will be at the head of the cache now. */

  if (cache_head && 
      (cache_head->flags & F_REVERSE) &&
      (cache_head->flags & prot) &&
      memcmp(&cache_head->addr, addr, addrlen) == 0)
    return cache_head;
  
  return NULL;
}

void cache_reload(int use_hosts, int cachesize)
{
  struct crec *cache, *tmp;
  FILE *f;
  char *line, buff[MAXLIN];

  for (cache=cache_head; cache; cache=tmp)
    {
      tmp = cache->next;
      if (cache->flags & F_HOSTS)
	{
	  cache_unlink(cache);
	  free(cache);
	}
      else if (!(cache->flags & F_DHCP))
	cache->flags = 0;
    }

  if (!use_hosts && (cachesize > 0))
    {
      syslog(LOG_INFO, "cleared cache");
      return;
    }
  
  f = fopen(HOSTSFILE, "r");
  
  if (!f)
    {
      syslog(LOG_WARNING, "failed to load names from %s: %m", HOSTSFILE);
      return;
    }
  
  syslog(LOG_INFO, "reading %s", HOSTSFILE);
  
  while ((line = fgets(buff, MAXLIN, f)))
    {
      struct all_addr addr;
      char *token = strtok(line, " \t\n");
      int addrlen, flags;
          
      if (!token || (*token == '#')) 
	continue;
      
      if (inet_pton(AF_INET, token, &addr) == 1)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV4;
	  addrlen = INADDRSZ;
	}
      else if(inet_pton(AF_INET6, token, &addr) == 1)
	{
	  flags = F_HOSTS | F_IMMORTAL | F_FORWARD | F_REVERSE | F_IPV6;
	  addrlen = IN6ADDRSZ;
	}
      else
	continue;

      while ((token = strtok(NULL, " \t\n")) && (*token != '#'))
	if ((cache = malloc(sizeof(struct crec))))
	  {
	    char *cp;
		int len;

		len = strlen(token);
		cache->nameSize = len + 1;
		cache->name = malloc(cache->nameSize);
		if (!(cache->name)) {
			free(cache);
			continue;
		}
	    for (cp = cache->name; *token; token++, cp++)
	      *cp = tolower(*token);
	    *cp = 0;
	    cache->flags = flags;
	    memcpy(&cache->addr, &addr, addrlen);
	    cache_insert(cache);
	    /* Only the first name is canonical, and should be 
	       returned to reverse queries */
	    flags &=  ~F_REVERSE;
	  }
    }

  fclose(f);
}
	    

struct crec *cache_clear_dhcp(void)
{
  struct crec *cache = cache_head, *ret = NULL;
  
  while (cache)
    {
      struct crec *tmp = cache->next;
      if (cache->flags & F_DHCP)
	{
	  cache_unlink(cache);
	  cache->next = ret;
	  ret = cache;
	}
      cache = tmp;
    }
  return ret;
}

void dump_cache(int daemon, int cache_size)
{
  if (daemon)
    syslog(LOG_INFO, "Cache size %d, %d/%d cache insertions re-used unexpired cache entries.", 
	   cache_size, cache_live_freed, cache_inserted); 
  else
    {
      struct crec *cache ;
      char addrbuff[INET6_ADDRSTRLEN];
      printf("Dnsmasq: Cache size %d, %d/%d cache insertions re-used unexpired cache entries.\n",
	     cache_size, cache_live_freed, cache_inserted); 
      printf("Host                                     Address                        Flags   Expires\n");
      
      for(cache = cache_head ; cache ; cache = cache->next)
	if (cache->flags & (F_FORWARD | F_REVERSE))
	  {
	    if (cache->flags & F_NEG)
	      addrbuff[0] = 0;
	    else if (cache->flags & F_IPV4)
	      inet_ntop(AF_INET, &cache->addr, addrbuff, INET_ADDRSTRLEN);
	    else if (cache->flags & F_IPV6)
	      inet_ntop(AF_INET6, &cache->addr, addrbuff, INET6_ADDRSTRLEN);
	    
	    printf("%-40.40s %-30.30s %s%s%s%s%s%s%s%s  %s",
		   cache->name, addrbuff,
		   cache->flags & F_IPV4 ? "4" : "",
		   cache->flags & F_IPV6 ? "6" : "",
		   cache->flags & F_FORWARD ? "F" : " ",
		   cache->flags & F_REVERSE ? "R" : " ",
		   cache->flags & F_IMMORTAL ? "I" : " ",
		   cache->flags & F_DHCP ? "D" : " ",
		   cache->flags & F_NEG ? "N" : " ",
		   cache->flags & F_HOSTS ? "H" : " ",
		   cache->flags & F_IMMORTAL ? "\n" : ctime(&(cache->ttd))) ;
	  }
    }
}

