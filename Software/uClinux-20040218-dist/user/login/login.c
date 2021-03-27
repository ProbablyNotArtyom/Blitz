/*****************************************************************************/

/*
 *	login.c -- simple login program.
 *
 *	(C) Copyright 1999-2001, Greg Ungerer (gerg@snapgear.com).
 * 	(C) Copyright 2001, SnapGear Inc. (www.snapgear.com) 
 * 	(C) Copyright 2000, Lineo Inc. (www.lineo.com) 
 *
 *	Made some changes and additions Nick Brok (nick@nbrok.iaehv.nl).
 */

/*****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <getopt.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <sys/utsname.h>
#include <config/autoconf.h>
#ifndef __UC_LIBC__
#include <crypt.h>
#endif
#ifdef OLD_CONFIG_PASSWORDS
#include <crypt_old.h>
#endif
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>

#ifdef CONFIG_AMAZON
#include "logcnt.c"
#endif

/*****************************************************************************/

#ifdef CONFIG_USER_FLATFSD_FLATFSD
#define PATH_OLD_PASSWD	"/etc/config/config"
#else
#define PATH_OLD_PASSWD	"/etc/passwd"
#endif

/* Delay bad password exit.
 * 
 * This doesn't really accomplish anything I guess..
 * as other connections can be made in the meantime.. and
 * someone attempting a brute force attack could kill their
 * connection if a delay is detected etc.
 *
 * -m2 (20000201)
 */
#define DELAY_EXIT	1

/*****************************************************************************/

char *version = "v1.0.2";

char usernamebuf[128];

/*****************************************************************************/

#ifdef OLD_CONFIG_PASSWORDS
static inline char *getoldpass(const char *pfile)
{
	static char	tmpline[128];
	FILE		*fp;
	char		*spass;
	int		len;

	if ((fp = fopen(pfile, "r")) == NULL) {
		fprintf(stderr, "ERROR: failed to open(%s), errno=%d \n",
			pfile, errno);
		return((char *) NULL);
	}

	while (fgets(tmpline, sizeof(tmpline), fp)) {
		spass = strchr(tmpline, ' ');
		if (spass) {
			*spass++ = 0;
			if (strcmp(tmpline, "passwd") == 0) {
				len = strlen(spass);
				if (spass[len-1] == '\n')
					spass[len-1] = 0;
				fclose(fp);
				return(spass);
			}
		}
	}

	fclose(fp);
	return((char *) NULL);
}
#endif

static inline char *getrealpass(const char *user) {
	struct passwd *pwp;
	
	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp->pw_passwd;
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	char	*user;
	char	*realpwd, *gotpwd, *cpwd;
	char *host = NULL;
	int flag;

    while ((flag = getopt(argc, argv, "h:")) != EOF) {
        switch (flag) {
        case 'h':
            host = optarg;
            break;
        default:
			fprintf(stderr,
			"login [OPTION]... [username]\n"
			"\nBegin a new session on the system\n\n"
			"Options:\n"
			"\t-h\t\tName of the remote host for this login.\n"
			);
        }
    }

	chdir("/");

	if (optind < argc) {
		user = argv[optind];
	} else {
		printf("login: ");
		fflush(stdout);
		if (fgets(usernamebuf, sizeof(usernamebuf), stdin) == NULL)
			exit(0);
		if ((user = strchr(usernamebuf, '\n')) != 0) {
			*user = '\0';
		}
		user = &usernamebuf[0];
	}

	gotpwd = getpass("Password: ");
	realpwd = getrealpass(user);
#ifdef OLD_CONFIG_PASSWORDS
	if ((realpwd == NULL) && 
			((*user == '\0') || (strcmp(user, "root") == 0)))
		realpwd = getoldpass(PATH_OLD_PASSWD);
#endif
	openlog("login", LOG_PID, LOG_AUTHPRIV);
	if (gotpwd && realpwd
#ifdef ONLY_ALLOW_ROOT
			&& strcmp(user, "root") == 0
#endif
			) {
		int good = 0;


		cpwd = crypt(gotpwd, realpwd);
		if (strcmp(cpwd, realpwd) == 0) 
			good++;

#ifdef OLD_CONFIG_PASSWORDS
		cpwd = crypt_old(gotpwd, realpwd);
		if (strcmp(cpwd, realpwd) == 0)
			good++;
#endif

#ifdef CONFIG_AMAZON
		access__attempted(!good, user);
#endif
		if (good) {
			syslog(LOG_INFO, "Authentication successful for %s from %s\n",
					user, host ? host : "unknown");
#ifdef EMBED
			execlp("sh", "sh", NULL);
#else
			execlp("sh", "sh", "-t", NULL);
#endif
		} else {
			syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Bad Password\n",
					user, host ? host : "unknown");
			sleep(DELAY_EXIT);
		}
	} else {
#ifdef CONFIG_AMAZON
		access__attempted(1, user);
#endif
		syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Invalid Username\n",
					user, host ? host : "unknown");
		sleep(DELAY_EXIT);
	}

	return(0);
}

/*****************************************************************************/
