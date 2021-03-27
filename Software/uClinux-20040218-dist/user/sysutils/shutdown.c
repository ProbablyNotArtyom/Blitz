/* shutdown.c:
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 1999  D. Jeff Dionne     <jeff@lineo.ca>
 * Copyright (C) 2000  Lineo, Inc.  (www.lineo.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/autoconf.h>

#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <signal.h>

#if __GNU_LIBRARY__ > 5
#include <sys/reboot.h>
#endif

int
main(argc, argv)
	int argc;
	char	**argv;
{
	if ((argc != 3) || (strcmp(argv[1], "-h") && strcmp(argv[1], "-r")) || strcmp(argv[2], "now")) {
		printf("Usage: %s -h|-r now\n", argv[0]);
		exit(0);
	}
	
	kill(1, SIGTSTP);
	sync();
	signal(SIGTERM,SIG_IGN);
	setpgrp();
	kill(-1, SIGTERM);
	sleep(1);
	kill(-1, SIGHUP); /* Force PPPD's down, too */
	sleep(1);
	kill(-1, SIGKILL);
	sync();
	sleep(1);
	
	if (strcmp(argv[1], "-h")==0) {
#if __GNU_LIBRARY__ > 5
		reboot(0xCDEF0123);
#else
		reboot(0xfee1dead, 672274793, 0xCDEF0123);
#endif
	} else {
#if __GNU_LIBRARY__ > 5
		reboot(0x01234567);
#else
		reboot(0xfee1dead, 672274793, 0x01234567);
#endif
	}
	
	exit(0); /* Shrug */
}

