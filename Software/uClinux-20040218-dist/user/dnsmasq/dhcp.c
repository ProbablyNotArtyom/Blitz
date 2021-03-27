/* dnsmasq is Copyright (c) 2000 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/


/* Code in this file is based on contributions by John Volpe. */

#include "dnsmasq.h"

static char *next_token (char *token, FILE * fp);
static void process_lease(struct crec **empty_cache, char *host_name, 
			  struct in_addr host_address, time_t ttd, int flags);

void load_dhcp(char *file, char *suffix)
{
  struct crec *spares;
  static char token[MAXLIN]; 
  static char hostname[MAXDNAME];
  struct in_addr host_address;
  time_t ttd, tts;
  FILE *fp = fopen (file, "r");
  
  if (!fp)
    {
      syslog (LOG_ERR, "failed to load %s: %m", file);
      return;
    }
  
  syslog (LOG_INFO, "reading %s", file);

  /* remove all existing DHCP cache entries onto temp. freelist */
  spares = cache_clear_dhcp();
  
  while ((next_token(token, fp)))
    {
      if (strcmp(token, "lease") == 0)
        {
          hostname[0] = '\0';
	  ttd = tts = (time_t)(-1);

          if (next_token(token, fp) && inet_pton(AF_INET, token, &host_address))
            {
              if (next_token(token, fp) && *token == '{')
                {
                  while (next_token(token, fp) && *token != '}')
                    {
                      if ((strcmp(token, "client-hostname") == 0) ||
			  (strcmp(token, "hostname") == 0))
                        {
			  if (next_token(token, fp))
			    {
			      if (*token == '"')
				{
				  *(token + strlen(token) - 1) = 0;
				  strncpy(hostname, token+1, MAXDNAME);
				}
			      else 
				strncpy(hostname, token, MAXDNAME);
			      hostname[MAXDNAME - 1] = 0;
			    }
			}

                      else if ((strcmp(token, "ends") == 0) ||
			       (strcmp(token, "starts") == 0))
                        {
                          struct tm lease_time;
			  int is_ends = (strcmp(token, "ends") == 0);
			  if (next_token(token, fp) &&  /* skip weekday */
			      next_token(token, fp) &&  /* Get date from lease file */
			      sscanf (token, "%d/%d/%d", 
				      &lease_time.tm_year,
				      &lease_time.tm_mon,
				      &lease_time.tm_mday) == 3 &&
			      next_token(token, fp) &&
			      sscanf (token, "%d:%d:%d:", 
				      &lease_time.tm_hour,
				      &lease_time.tm_min, 
				      &lease_time.tm_sec) == 3)
			    {
			      /* There doesn't seem to be a universally available library function
				 which converts broken-down _GMT_ time to seconds-in-epoch.
				 The following was borrowed from ISC dhcpd sources, where
                                 it is noted that it might not be entirely accurate for odd seconds.
				 Since we're trying to get the same answer as dhcpd, that's just
				 fine here. */
			      static int months [11] = { 31, 59, 90, 120, 151, 181,
							 212, 243, 273, 304, 334 };
			      time_t time = ((((((365 * (lease_time.tm_year - 1970) + /* Days in years since '70 */
						  (lease_time.tm_year - 1969) / 4 +   /* Leap days since '70 */
						  (lease_time.tm_mon > 1                /* Days in months this year */
						   ? months [lease_time.tm_mon - 2]
						   : 0) +
						  (lease_time.tm_mon > 2 &&         /* Leap day this year */
						   !((lease_time.tm_year - 1972) & 3)) +
						  lease_time.tm_mday - 1) * 24) +   /* Day of month */
						lease_time.tm_hour) * 60) +
					      lease_time.tm_min) * 60) + lease_time.tm_sec;
			      if (is_ends)
				ttd = time;
			      else
				tts = time;			    }
                        }
		    }

		  if (*hostname && ttd != (time_t)(-1))
		    {
		      char *dot;
		      /* infinite lease to is represented by -1 */
		      /* This makes is to the lease file as 
                         start time one less than end time. */
		      /* We use -1 as infinite in ttd */
		      if ((tts != -1) && (ttd == tts - 1))
			ttd = (time_t)(-1);
		      
		      dot = strchr(hostname, '.');
		      if (suffix)
			{ 
			  if (dot) 
			    { /* suffix and lease has ending: must match */
			      if (strcmp(dot+1, suffix) != 0)
				syslog(LOG_WARNING, 
				       "Ignoring DHCP lease for %s because it has an illegal domain part", hostname);
			      else
				process_lease(&spares, hostname, host_address, ttd, F_REVERSE);
			    }
			  else
			    { /* suffix exists but lease has no ending - add lease and lease.suffix */
			      process_lease(&spares, hostname, host_address, ttd, 0);
			      strncat(hostname, ".", MAXDNAME);
			      strncat(hostname, suffix, MAXDNAME);
			      hostname[MAXDNAME-1] = 0; /* in case strncat hit limit */
			      /* Make FQDN canonical for reverse lookups */
			      process_lease(&spares, hostname, host_address, ttd, F_REVERSE);
			    }
			}
		      else
			{ /* no suffix */
			  if (dot) /* no lease ending allowed */
			    syslog(LOG_WARNING, 
				   "Ignoring DHCP lease for %s because it has a domain part", hostname);
			  else
			    process_lease(&spares, hostname, host_address, ttd, F_REVERSE);
			}
		    }
		}
	    }
	}
    }
  fclose(fp);
  
  /* free any still-unused cache structs */
  while (spares)
    { 
      struct crec *tmp = spares->next;
      free(spares);
      spares = tmp;
    }
}

static char *next_token (char *token, FILE * fp)
{
  int count = 0;
  int c;
  char *cp = token;
  
  while((c = getc(fp)) != EOF)
    {
      if (c == '#')
	do { c = getc(fp); } while (c != '\n' && c != EOF);
      
      if (c == ' ' || c == '\t' || c == '\n' || c == ';')
	{
	  if (count)
	    {
	      *cp = 0;
	      return token;
	    }
	}
      else if (count<MAXLIN-1)
	{
	  *cp++ = c;
	  count++;
	}
      
    }
  
  if (count)
    {
      *cp = 0;
      return token;
    }
  
  return NULL;
}

static void process_lease(struct crec **empty_cache, char *host_name, 
			  struct in_addr host_address, time_t ttd, int flags) 
{
  struct crec *crec;
  int len;
  char *cp;
  
  if ((crec = cache_find_by_name(NULL, host_name, 0, F_IPV4)))
    {
      if (crec->flags & F_HOSTS)
	syslog(LOG_WARNING, "Ignoring DHCP lease for %s because it clashes with an /etc/hosts entry.", crec->name);
      else if (!(crec->flags & F_DHCP))
	{
	  if (crec->flags & F_NEG)
	    {
	      /* name may have been searched for before being allocated to DHCP and 
		 therefore got a negative cache entry. If so delete it and continue. */
	      crec->flags &= ~F_FORWARD;
	      goto newrec;
	    }
	  else
	    syslog(LOG_WARNING, "Ignoring DHCP lease for %s because it clashes with a cached name.", crec->name);
	}
      else if (ttd > crec->ttd || ttd == (time_t)-1)
	{
	  /* simply a later entry in the leases file which supercedes and earlier one. */
	  memcpy(&crec->addr, &host_address, INADDRSZ);
	  if (ttd == (time_t)-1)
	    crec->flags |= F_IMMORTAL;
	  else
	    crec->ttd = ttd;
	}
    }
  else
    { /* can't find it in cache. */
    newrec:
      crec = *empty_cache;
      if (crec) /* old entries to reuse */
	*empty_cache =(*empty_cache)->next;
      else /* need new one */
	crec = malloc(sizeof(struct crec));
      
      if (crec) /* malloc may fail */
	{
      crec->nameSize = 0;
	  crec->name = NULL;
	  crec->flags = F_DHCP | F_FORWARD | F_IPV4 | flags;
	  if (ttd == (time_t)-1)
	    crec->flags |= F_IMMORTAL;
	  else
	    crec->ttd = ttd;
	  memcpy(&crec->addr, &host_address, INADDRSZ);
      crec->name = malloc((len = strlen(host_name)) + 1);
	  if (!crec->name) {
        free (crec);
        return;
	  }
      crec->nameSize = len;
      for (cp = crec->name; *host_name; host_name++, cp++)
	    *cp = tolower(*host_name);
	  *cp = 0;
	  cache_insert(crec);
	}
    }
}









