/* Copyright (c) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* getent: get entries from administrative database.  */

#include <aliases.h>
#include <argp.h>
#include <grp.h>
#include <pwd.h>
#include <shadow.h>
#include <ctype.h>
#include <error.h>
#include <libintl.h>
#include <locale.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

/* Get libc version number.  */
#include <version.h>

#define PACKAGE _libc_intl_domainname

/* Name and version of program.  */
static void print_version (FILE *stream, struct argp_state *state);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = print_version;

/* Short description of parameters.  */
static const char args_doc[] = N_("database [key ...]");

/* Supported options. */
static const struct argp_option args_options[] =
  {
    { "service", 's', "CONFIG", 0, N_("Service configuration to be used") },
    { NULL, 0, NULL, 0, NULL },
  };

/* Prototype for option handler.  */
static error_t parse_option (int key, char *arg, struct argp_state *state);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
  {
    args_options, parse_option, args_doc, NULL,
  };

/* Print the version information.  */
static void
print_version (FILE *stream, struct argp_state *state)
{
  fprintf (stream, "getent (GNU %s) %s\n", PACKAGE, VERSION);
  fprintf (stream, gettext ("\
Copyright (C) %s Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
"), "2002");
  fprintf (stream, gettext ("Written by %s.\n"), "Thorsten Kukuk");
}

/* This is for aliases */
static inline void
print_aliases (struct aliasent *alias)
{
  unsigned int i = 0;

  printf ("%s: ", alias->alias_name);
  for  (i = strlen (alias->alias_name); i < 14; ++i)
    fputs_unlocked (" ", stdout);

  for (i = 0; i < alias->alias_members_len; ++i)
    printf ("%s%s",
	    alias->alias_members [i],
	    i + 1 == alias->alias_members_len ? "\n" : ", ");
}

static int
aliases_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct aliasent *alias;

  if (number == 0)
    {
      setaliasent ();
      while ((alias = getaliasent ()) != NULL)
	print_aliases (alias);
      endaliasent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      alias = getaliasbyname (key[i]);

      if (alias == NULL)
	result = 2;
      else
	print_aliases (alias);
    }

  return result;
}

/* This is for ethers */
static int
ethers_keys (int number, char *key[])
{
  int result = 0;
  int i;

  if (number == 0)
    {
      fprintf (stderr, _("Enumeration not supported on %s\n"), "ethers");
      return 3;
    }

  for (i = 0; i < number; ++i)
    {
      struct ether_addr *ethp, eth;
      char buffer [1024], *p;

      ethp = ether_aton (key[i]);
      if (ethp != NULL)
	{
	  if (ether_ntohost (buffer, ethp))
	    {
	      result = 2;
	      continue;
	    }
	  p = buffer;
	}
      else
	{
	  if (ether_hostton (key[i], &eth))
	    {
	      result = 2;
	      continue;
	    }
	  p = key[i];
	  ethp = &eth;
	}
      printf ("%s %s\n", ether_ntoa (ethp), p);
    }

  return result;
}

/* This is for group */
static inline void
print_group (struct group *grp)
{
  unsigned int i = 0;

  printf ("%s:%s:%ld:", grp->gr_name ? grp->gr_name : "",
	  grp->gr_passwd ? grp->gr_passwd : "",
	  (unsigned long int) grp->gr_gid);

  while (grp->gr_mem[i] != NULL)
    {
      fputs_unlocked (grp->gr_mem[i], stdout);
      ++i;
      if (grp->gr_mem[i] != NULL)
	putchar_unlocked (',');
    }
  putchar_unlocked ('\n');
}

static int
group_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct group *grp;

  if (number == 0)
    {
      setgrent ();
      while ((grp = getgrent ()) != NULL)
	print_group (grp);
      endgrent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      if (isdigit (key[i][0]))
	grp = getgrgid (atol (key[i]));
      else
	grp = getgrnam (key[i]);

      if (grp == NULL)
	result = 2;
      else
	print_group (grp);
    }

  return result;
}

/* This is for hosts */
static inline void
print_hosts (struct hostent *host)
{
  unsigned int i;
  char buf[INET6_ADDRSTRLEN];
  const char *ip = inet_ntop (host->h_addrtype, host->h_addr_list[0],
			      buf, sizeof (buf));

  printf ("%-15s %s", ip, host->h_name);

  i = 0;
  while (host->h_aliases[i] != NULL)
    {
      putchar_unlocked (' ');
      fputs_unlocked (host->h_aliases[i], stdout);
      ++i;
    }
  putchar_unlocked ('\n');
}

static int
hosts_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct hostent *host;

  if (number == 0)
    {
      sethostent (0);
      while ((host = gethostent ()) != NULL)
	print_hosts (host);
      endhostent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      struct hostent *host = NULL;

      if (strchr (key[i], ':') != NULL)
	{
	  char addr[IN6ADDRSZ];
	  if (inet_pton (AF_INET6, key[i], &addr))
	    host = gethostbyaddr (addr, sizeof (addr), AF_INET6);
	}
      else if (isdigit (key[i][0]))
	{
	  char addr[INADDRSZ];
	  if (inet_pton (AF_INET, key[i], &addr))
	    host = gethostbyaddr (addr, sizeof (addr), AF_INET);
	}
      else if ((host = gethostbyname2 (key[i], AF_INET6)) == NULL)
	host = gethostbyname2 (key[i], AF_INET);

      if (host == NULL)
	result = 2;
      else
	print_hosts (host);
    }

  return result;
}

/* This is for netgroup */
static int
netgroup_keys (int number, char *key[])
{
  int result = 0;
  int i;

  if (number == 0)
    {
      fprintf (stderr, _("Enumeration not supported on %s\n"), "netgroup");
      return 3;
    }

  for (i = 0; i < number; ++i)
    {
      if (!setnetgrent (key[i]))
	result = 2;
      else
	{
	  char *p[3];

	  printf ("%-21s", key[i]);

	  while (getnetgrent (p, p + 1, p + 2))
	    printf (" (%s, %s, %s)", p[0] ?: " ", p[1] ?: "", p[2] ?: "");
	  putchar_unlocked ('\n');
	}
    }

  return result;
}

/* This is for networks */
static inline void
print_networks (struct netent *net)
{
  unsigned int i;
  struct in_addr ip;
  ip.s_addr = htonl (net->n_net);

  printf ("%-21s %s", net->n_name, inet_ntoa (ip));

  i = 0;
  while (net->n_aliases[i] != NULL)
    {
      putchar_unlocked (' ');
      fputs_unlocked (net->n_aliases[i], stdout);
      ++i;
      if (net->n_aliases[i] != NULL)
	putchar_unlocked (',');
    }
  putchar_unlocked ('\n');
}

static int
networks_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct netent *net;

  if (number == 0)
    {
      setnetent (0);
      while ((net = getnetent ()) != NULL)
	print_networks (net);
      endnetent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      if (isdigit (key[i][0]))
	net = getnetbyaddr (inet_addr (key[i]), AF_UNIX);
      else
	net = getnetbyname (key[i]);

      if (net == NULL)
	result = 2;
      else
	print_networks (net);
    }

  return result;
}

/* Now is all for passwd */
static inline void
print_passwd (struct passwd *pwd)
{
  printf ("%s:%s:%ld:%ld:%s:%s:%s\n",
	  pwd->pw_name ? pwd->pw_name : "",
	  pwd->pw_passwd ? pwd->pw_passwd : "",
	  (unsigned long int) pwd->pw_uid,
	  (unsigned long int) pwd->pw_gid,
	  pwd->pw_gecos ? pwd->pw_gecos : "",
	  pwd->pw_dir ? pwd->pw_dir : "",
	  pwd->pw_shell ? pwd->pw_shell : "");
}

static int
passwd_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct passwd *pwd;

  if (number == 0)
    {
      setpwent ();
      while ((pwd = getpwent ()) != NULL)
	print_passwd (pwd);
      endpwent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      if (isdigit (key[i][0]))
	pwd = getpwuid (atol (key[i]));
      else
	pwd = getpwnam (key[i]);

      if (pwd == NULL)
	result = 2;
      else
	print_passwd (pwd);
    }

  return result;
}

/* This is for protocols */
static inline void
print_protocols (struct protoent *proto)
{
  unsigned int i;

  printf ("%-21s %d", proto->p_name, proto->p_proto);

  i = 0;
  while (proto->p_aliases[i] != NULL)
    {
      putchar_unlocked (' ');
      fputs_unlocked (proto->p_aliases[i], stdout);
      ++i;
    }
  putchar_unlocked ('\n');
}

static int
protocols_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct protoent *proto;

  if (number == 0)
    {
      setprotoent (0);
      while ((proto = getprotoent ()) != NULL)
	print_protocols (proto);
      endprotoent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      if (isdigit (key[i][0]))
	proto = getprotobynumber (atol (key[i]));
      else
	proto = getprotobyname (key[i]);

      if (proto == NULL)
	result = 2;
      else
	print_protocols (proto);
    }

  return result;
}

/* Now is all for rpc */
static inline void
print_rpc (struct rpcent *rpc)
{
  int i;

  printf ("%-15s %d%s",
	  rpc->r_name, rpc->r_number, rpc->r_aliases[0] ? " " : "");

  for (i = 0; rpc->r_aliases[i]; ++i)
    printf (" %s", rpc->r_aliases[i]);
  putchar_unlocked ('\n');
}

static int
rpc_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct rpcent *rpc;

  if (number == 0)
    {
      setrpcent (0);
      while ((rpc = getrpcent ()) != NULL)
	print_rpc (rpc);
      endrpcent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      if (isdigit (key[i][0]))
	rpc = getrpcbynumber (atol (key[i]));
      else
	rpc = getrpcbyname (key[i]);

      if (rpc == NULL)
	result = 2;
      else
	print_rpc (rpc);
    }

  return result;
}

/* for services */
static void
print_services (struct servent *serv)
{
  unsigned int i;

  printf ("%-21s %d/%s", serv->s_name, ntohs (serv->s_port), serv->s_proto);

  i = 0;
  while (serv->s_aliases[i] != NULL)
    {
      putchar_unlocked (' ');
      fputs_unlocked (serv->s_aliases[i], stdout);
      ++i;
    }
  putchar_unlocked ('\n');
}

static int
services_keys (int number, char *key[])
{
  int result = 0;
  int i;
  struct servent *serv;

  if (!number)
    {
      setservent (0);
      while ((serv = getservent ()) != NULL)
	print_services (serv);
      endservent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      struct servent *serv;
      char *proto = strchr (key[i], '/');

      if (proto == NULL)
	{
	  setservent (0);
	  if (isdigit (key[i][0]))
	    {
	      int port = htons (atol (key[i]));
	      while ((serv = getservent ()) != NULL)
		if (serv->s_port == port)
		  {
		    print_services (serv);
		    break;
		  }
	    }
	  else
	    {
	      int j;

	      while ((serv = getservent ()) != NULL)
		if (strcmp (serv->s_name, key[i]) == 0)
		  {
		    print_services (serv);
		    break;
		  }
		else
		  for (j = 0; serv->s_aliases[j]; ++j)
		    if (strcmp (serv->s_aliases[j], key[i]) == 0)
		      {
			print_services (serv);
			break;
		      }
	    }
	  endservent ();
	}
      else
	{
	  *proto++ = '\0';

	  if (isdigit (key[i][0]))
	    serv = getservbyport (htons (atol (key[i])), proto);
	  else
	    serv = getservbyname (key[i], proto);

	  if (serv == NULL)
	    result = 2;
	  else
	    print_services (serv);
	}
    }

  return result;
}

/* This is for shadow */
static inline void
print_shadow (struct spwd *sp)
{
  printf ("%s:%s:",
	  sp->sp_namp ? sp->sp_namp : "",
	  sp->sp_pwdp ? sp->sp_pwdp : "");

#define SHADOW_FIELD(n) \
  if (sp->n == -1)							      \
    putchar_unlocked (':');						      \
  else									      \
    printf ("%ld:", sp->n)

  SHADOW_FIELD (sp_lstchg);
  SHADOW_FIELD (sp_min);
  SHADOW_FIELD (sp_max);
  SHADOW_FIELD (sp_warn);
  SHADOW_FIELD (sp_inact);
  SHADOW_FIELD (sp_expire);
  if (sp->sp_flag == ~0ul)
    putchar_unlocked ('\n');
  else
    printf ("%lu\n", sp->sp_flag);
}

static int
shadow_keys (int number, char *key[])
{
  int result = 0;
  int i;

  if (number == 0)
    {
      struct spwd *sp;

      setspent ();
      while ((sp = getspent ()) != NULL)
	print_shadow (sp);
      endpwent ();
      return result;
    }

  for (i = 0; i < number; ++i)
    {
      struct spwd *sp;

      sp = getspnam (key[i]);

      if (sp == NULL)
	result = 2;
      else
	print_shadow (sp);
    }

  return result;
}

struct
  {
    const char *name;
    int (*func) (int number, char *key[]);
  } databases[] =
  {
#define D(name) { #name, name ## _keys },
D(aliases)
D(ethers)
D(group)
D(hosts)
D(netgroup)
D(networks)
D(passwd)
D(protocols)
D(rpc)
D(services)
D(shadow)
#undef D
    { NULL, NULL }
  };

/* Handle arguments found by argp. */
static error_t
parse_option (int key, char *arg, struct argp_state *state)
{
  int i;
  switch (key)
    {
    case 's':
      for (i = 0; databases[i].name; ++i)
	__nss_configure_lookup (databases[i].name, arg);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}

/* build doc */
static inline void
build_doc (void)
{
  int i, j, len;
  char *short_doc, *long_doc, *doc, *p;

  short_doc = _("getent - get entries from administrative database.");
  long_doc = _("Supported databases:");
  len = strlen (short_doc) + strlen (long_doc) + 3;

  for (i = 0; databases[i].name; ++i)
    len += strlen (databases[i].name) + 1;

  doc = (char *) malloc (len);
  if (doc == NULL)
    doc = short_doc;
  else
    {
      p = stpcpy (doc, short_doc);
      *p++ = '\v';
      p = stpcpy (p, long_doc);
      *p++ = '\n';

      for (i = 0, j = 0; databases[i].name; ++i)
	{
	  len = strlen (databases[i].name);
	  if (i != 0)
	    {
	      if (j + len > 72)
		{
		  j = 0;
		  *p++ = '\n';
		}
	      else
		*p++ = ' ';
	    }

	  p = mempcpy (p, databases[i].name, len);
	  j += len + 1;
	}
    }

  argp.doc = doc;
}

/* the main function */
int
main (int argc, char *argv[])
{
  int remaining, i;

  /* Set locale via LC_ALL.  */
  setlocale (LC_ALL, "");
  /* Set the text message domain.  */
  textdomain (PACKAGE);

  /* Build argp.doc.  */
  build_doc ();

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if ((argc - remaining) < 1)
    {
      error (0, 0, gettext ("wrong number of arguments"));
      argp_help (&argp, stdout, ARGP_HELP_SEE, program_invocation_short_name);
      return 1;
    }

  for (i = 0; databases[i].name; ++i)
    if (argv[remaining][0] == databases[i].name[0]
	&& !strcmp (argv[remaining], databases[i].name))
      return databases[i].func (argc - remaining - 1, &argv[remaining + 1]);

  fprintf (stderr, _("Unknown database: %s\n"), argv[remaining]);
  argp_help (&argp, stdout, ARGP_HELP_SEE, program_invocation_short_name);
  return 1;
}
