/* vi: set sw=4 ts=4: */
/*
 * Mini halt implementation for busybox
 *
 * Copyright (C) 1995, 1996 by Bruce Perens <bruce@pixar.com>.
 * Copyright (C) 1999-2002 by Erik Andersen <andersee@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "busybox.h"
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <config/autoconf.h>

#if __GNU_LIBRARY__ > 5
#include <sys/reboot.h>
#endif

extern int halt_main(int argc, char **argv)
{
	int delay = 0; /* delay in seconds before rebooting */
	int rc;

	while ((rc = getopt(argc, argv, "d:")) > 0) {
		switch (rc) {
		case 'd':
			delay = atoi(optarg);
			break;

		default:
			show_usage();
			break;
		}
	}

	if(delay > 0)
		sleep(delay);

#ifdef CONFIG_USER_INIT_INIT
	/* Culled from the sash reboot code */
	kill(1, SIGTSTP);
	sync();
	signal(SIGTERM,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	setpgrp();
	kill(-1, SIGTERM);
	sleep(1);
	kill(-1, SIGHUP);
	sleep(1);
	kill(-1, SIGKILL);
	sync();
	sleep(1);
#if __GNU_LIBRARY__ > 5
	reboot(0xCDEF0123);
#else
	reboot(0xfee1dead, 672274793, 0xCDEF0123);
#endif
	exit(0); /* Shrug */
#else
#ifdef BB_FEATURE_LINUXRC
	/* don't assume init's pid == 1 */
	long *pid = find_pid_by_name("init");
	if (!pid || *pid<=0) {
		pid = find_pid_by_name("linuxrc");
		if (!pid || *pid<=0)
			error_msg_and_die("no process killed");
	}
	return(kill(*pid, SIGUSR1));
#else
	return(kill(1, SIGUSR1));
#endif
#endif
}
