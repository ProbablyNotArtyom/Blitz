/*
 *  nandwrite.c
 *
 *  Copyright (C) 2000 Steven J. Hill (sjhill@cotw.com)
 *
 * $Id: nandwrite.c,v 1.3 2001/03/07 09:34:19 ollie Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Overview:
 *   This utility writes a binary image directly to a NAND flash
 *   chip or NAND chips contained in DoC devices. This is the
 *   "inverse operation" of nanddump.
 *
 * Bug/ToDo:
 *   How are we going to do with ECC (esp. for "free" pages) ?
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <asm/types.h>
#include <linux/mtd/mtd.h>

/*
 * Buffer array used for writing data
 */
unsigned char writebuf[512];
unsigned char oobbuf[16];

/*
 * Main program
 */
int main(int argc, char **argv)
{
	unsigned long ofs;
	int cnt, fd, ifd;
	mtd_info_t meminfo;
	struct mtd_oob_buf oob = {0, 16, oobbuf};

	/* Make sure enough arguments were passed */
	if (argc < 4) {
		printf("usage: %s <mtdname> <input file> <start address>\n",
				argv[0]);
		exit(1);
	}

	/* Open the device */
	if ((fd = open(argv[1], O_RDWR)) == -1) {
		perror("open flash");
		exit(1);
	}

	/* Fill in MTD device capability structure */  
	if (ioctl(fd, MEMGETINFO, &meminfo) != 0) {
		perror("MEMGETINFO");
		close(fd);
		exit(1);
	}

	/* Make sure device page sizes are valid */
	if (!(meminfo.oobsize == 16 && meminfo.oobblock == 512) &&
	    !(meminfo.oobsize == 8 && meminfo.oobblock == 256)) {
		printf("Unknown flash (not normal NAND)\n");
		close(fd);
		exit(1);
	}

	/* Open the input file */
	if ((ifd = open(argv[2], O_RDONLY)) == -1) {
		perror("open input file");
		close(fd);
		close(ifd);
		exit(1);
	}

	/* Get start address */
	ofs = strtoul(argv[3], NULL, 0);

	/* Get data from input and write to the device */
	while(1) {
		/* Read Page Data from input file */
		if (!(cnt = read(ifd, writebuf, meminfo.oobblock)))
			/* EOF */
			break;
		else {
			/* Write out the Page data */
			if (pwrite(fd, writebuf, cnt, ofs) != cnt) {
				perror("pwrite");
				close(fd);
				close(ifd);
				exit(1);
			}
		}

		/* Read OOB data from input file, exit on failure */
		if (!read(ifd, oobbuf, meminfo.oobsize)) {
			fprintf(stderr, "Unexpected EOF in input file\n");
			exit(1);
		}

		/* Write OOB data */
		oob.start = ofs;
		if (ioctl(fd, MEMWRITEOOB, &oob) != 0) {
			perror("ioctl(MEMWRITEOOB)");
			close(fd);
			close(ifd);
			exit(1);
		}

		/* Increment address counter */
		ofs += meminfo.oobblock;
	}

	/* Close the output file and MTD device */
	close(fd);
	close(ifd);

	/* Return happy */
	return 0;
}
