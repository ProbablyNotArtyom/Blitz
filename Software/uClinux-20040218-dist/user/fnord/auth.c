/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Authorization "module" (c) 1998,99 Martin Hinner <martin@tdp.cz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifdef USE_AUTH
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#ifndef __UC_LIBC__
#include <crypt.h>
#endif
#include <syslog.h>
#ifdef SHADOW_AUTH
#include <shadow.h>
#elif !defined(EMBED)
#include "md5.h"
#endif
#ifdef OLD_CONFIG_PASSWORDS
#include <crypt_old.h>
#endif
#ifdef EMBED
#include <sys/types.h>
#include <pwd.h>
#include <config/autoconf.h>
#endif

#ifdef CONFIG_AMAZON
#include "../../login/logcnt.c"
#endif

#include "base64.h"

#undef DEBUG

#ifdef DEBUG
#define DBG(A) A
#else
#define DBG(A)
#endif

struct _auth_dir_ {
	char *directory;
	FILE *authfile;
	int dir_len;
	struct _auth_dir_ *next;
};

typedef struct _auth_dir_ auth_dir;

static auth_dir *auth_list = 0;
#ifdef OLD_CONFIG_PASSWORDS
static char auth_old_password[16];
#endif

/*
 * Name: auth_add
 *
 * Description: adds 
 */
void auth_add(char *directory,char *file)
{
	auth_dir *new_a,*old;
	
	old = auth_list;
	while (old)
	{
		if (!strcmp(directory,old->directory))
			return;
		old = old->next;
	}

#ifdef DEBUG
	syslog(LOG_DEBUG, "auth_add(dir=%s, file=%s)\n", directory, file);
#endif
	
	new_a = (auth_dir *)malloc(sizeof(auth_dir));
	/* success of this call will be checked later... */
	new_a->authfile = fopen(file,"rt");
	new_a->directory = strdup(directory);
	new_a->dir_len = strlen(directory);
	new_a->next = auth_list;
	auth_list = new_a;
}

#if 0
void auth_check()
{
	auth_dir *cur;
	
	cur = auth_list;
	while (cur) {
		if (!cur->authfile) {
			/*log_error_time();*/
			syslog(LOG_ERR,"Authentication password file for %s not found!\n", cur->directory);
		}
		cur = cur->next;
	}
}
#endif

/*
 * Name: auth_check_userpass
 *
 * Description: Checks user's password. Returns 0 when sucessful and password
 * 	is ok, else returns nonzero; As one-way function is used RSA's MD5 w/
 *  BASE64 encoding.
#ifdef EMBED
 * On embedded environments we use crypt(), instead of MD5.
#endif
 */
static int auth_check_userpass(char *user,char *pass,FILE *authfile)
{
#ifdef SHADOW_AUTH
	struct spwd *sp;

	sp = getspnam(user);
	if (!sp)
		return 2;

	if (!strcmp(crypt(pass, sp->sp_pwdp), sp->sp_pwdp))
		return 0;

#else

#ifndef EMBED
	char temps[0x100],*pwd;
	struct MD5Context mc;
 	unsigned char final[16];
	char encoded_passwd[0x40];
  /* Encode password ('pass') using one-way function and then use base64
		 encoding. */
	MD5Init(&mc);
	MD5Update(&mc, pass, strlen(pass));
	MD5Final(final, &mc);
	strcpy(encoded_passwd,"$1$");
	base64encode(final, encoded_passwd+3, 16);

	DBG(printf("auth_check_userpass(%s,%s,...);\n",user,pass);)

	fseek(authfile,0,SEEK_SET);
	while (fgets(temps,0x100,authfile))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;
		pwd = strchr(temps,':');
		if (pwd)
		{
			*pwd++=0;
			if (!strcmp(temps,user))
			{
				if (!strcmp(pwd,encoded_passwd))
					return 0;
			} else
				return 2;
		}
	}
#else
	struct passwd *pwp;

	pwp = getpwnam(user);
	if (pwp != NULL) {
		if (strcmp(crypt(pass, pwp->pw_passwd), pwp->pw_passwd) == 0)
			return 0;
	} else 
#ifdef OLD_CONFIG_PASSWORDS
	/* For backwards compatibility we allow the global root password to work too */
	if ((auth_old_password[0] != '\0') && 
			((*user == '\0') || (strcmp(user, "root") == 0))) {

		if (strcmp(crypt_old(pass,auth_old_password),auth_old_password) == 0 ||
				strcmp(crypt(pass,auth_old_password),auth_old_password) == 0) {
			strcpy(user, "root");
			return 0;
		}
	} else
#endif	/* OLD_CONFIG_PASSWORDS */
		return 2;

#endif	/* ! EMBED */
#endif	/* SHADOW_AUTH */
	return 1;
}

int auth_authorize(const char *host, const char *url, const char *remote_ip_addr, const char *authorization, char username[15])
{
	auth_dir *current;
	char *pwd;
	char auth_userpass[0x80];

	current = auth_list;

	while (current) {
		if (!memcmp(url, current->directory,
								current->dir_len)) {
			if (current->directory[current->dir_len - 1] != '/' &&
								url[current->dir_len] != '/' &&
								url[current->dir_len] != '\0') {
				break;
			}

			if (authorization) {
				int denied = 1;
				if (current->authfile==0) {
					return 0;
				}
				if (strncasecmp(authorization,"Basic ",6)) {
					syslog(LOG_ERR, "Can only handle Basic auth\n");
					return 0;
				}
				
				base64decode(auth_userpass,authorization+6,sizeof(auth_userpass));
				
				if ( (pwd = strchr(auth_userpass,':')) == 0 ) {
					syslog(LOG_ERR, "No user:pass in Basic auth\n");
					return 0;
				}
				
				*pwd++=0;

				denied = auth_check_userpass(auth_userpass,pwd,current->authfile);
#ifdef CONFIG_AMAZON
				access__attempted(denied, auth_userpass);
#endif
				if (denied) {
					switch (denied) {
						case 1:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Bad Password\n",
									auth_userpass, remote_ip_addr);
							break;
						case 2:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Invalid Username\n",
									auth_userpass, remote_ip_addr);
					}
					return 0;
				}
				/* Rely on syslogd to throw away duplicates */
				syslog(LOG_INFO, "Authentication successful for %s from %s\n", auth_userpass, remote_ip_addr);

				/* Copy user's name to request structure */
				snprintf(username, 15, "%s", auth_userpass);
				return 1;
			}
			else {
				/* No credentials were supplied. Tell them that some are required */
				return 0;
			}
		}
		current = current->next;
	}
						
	return 1;
}

#if 0
void dump_auth(void)
{
	auth_dir *temp;

	temp = auth_list;
	while (temp) {
		auth_dir *temp_next;

		if (temp->directory)
			free(temp->directory);
		if (temp->authfile)
			fclose(temp->authfile);
		temp_next = temp->next;
		free(temp);
		temp = temp_next;
	}
	auth_list = 0;
}
#endif

#endif
