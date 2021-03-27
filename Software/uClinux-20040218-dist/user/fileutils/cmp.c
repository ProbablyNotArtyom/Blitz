/*
 * Copyright (c) 1993 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Most simple built-in commands are here.
 */

#include "futils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <utime.h>
#include <errno.h>


int
main(argc, argv)
	char	**argv;
{
	int		fd1;
	int		fd2;
	int		cc1;
	int		cc2;
	long		pos;
	char		*srcname;
	char		*destname;
	char		*lastarg;
	char		*bp1;
	char		*bp2;
	char		buf1[512];
	char		buf2[512];
	struct	stat	statbuf1;
	struct	stat	statbuf2;

	if (stat(argv[1], &statbuf1) < 0) {
		perror(argv[1]);
		exit(2);
	}

	if (stat(argv[2], &statbuf2) < 0) {
		perror(argv[2]);
		exit(2);
	}

	if ((statbuf1.st_dev == statbuf2.st_dev) &&
		(statbuf1.st_ino == statbuf2.st_ino))
	{
		printf("Files are links to each other\n");
		exit(0);
	}

	if (statbuf1.st_size != statbuf2.st_size) {
		printf("Files are different sizes\n");
		exit(1);
	}

	fd1 = open(argv[1], 0);
	if (fd1 < 0) {
		perror(argv[1]);
		exit(2);
	}

	fd2 = open(argv[2], 0);
	if (fd2 < 0) {
		perror(argv[2]);
		close(fd1);
		exit(2);
	}

	pos = 0;
	while (TRUE) {
		cc1 = read(fd1, buf1, sizeof(buf1));
		if (cc1 < 0) {
			perror(argv[1]);
			exit(2);
		}

		cc2 = read(fd2, buf2, sizeof(buf2));
		if (cc2 < 0) {
			perror(argv[2]);
			goto differ;
		}

		if ((cc1 == 0) && (cc2 == 0)) {
			printf("Files are identical\n");
			goto same;
		}

		if (cc1 < cc2) {
			printf("First file is shorter than second\n");
			goto differ;
		}

		if (cc1 > cc2) {
			printf("Second file is shorter than first\n");
			goto differ;
		}

		if (memcmp(buf1, buf2, cc1) == 0) {
			pos += cc1;
			continue;
		}

		bp1 = buf1;
		bp2 = buf2;
		while (*bp1++ == *bp2++)
			pos++;

		printf("Files differ at byte position %ld\n", pos);
		goto differ;
	}
same:
	exit(0);
differ:
	exit(1);
}
