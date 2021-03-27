/*
 *  nanddump.c
 *
 *  Copyright (C) 2000 David Woodhouse (dwmw2@infradead.org)
 *                     Steven J. Hill (sjhill@cotw.com)
 *
 * $Id: nanddump.c,v 1.11 2001/06/11 17:00:50 dwmw2 Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This utility dumps the contents of raw NAND chips or NAND
 *   chips contained in DoC devices. NOTE: If you are using raw
 *   NAND chips, disable NAND ECC in your kernel.
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <asm/types.h>
#include <linux/mtd/mtd.h>

/*
 * Buffers for reading data from flash
 */
unsigned char readbuf[512];
unsigned char oobbuf[16];

/*
 * Main program
 */
int main(int argc, char **argv)
{
	unsigned long ofs;
	int i, fd, ofd, bs, start_addr, end_addr, pretty_print;
	struct mtd_oob_buf oob = {0, 16, oobbuf};
	mtd_info_t meminfo;
	unsigned char pretty_buf[80];

	/* Make sure enough arguments were passed */ 
	if (argc < 3) {
		printf("usage: %s <mtdname> <dumpname> [start addr] [length]\n", argv[0]);
		exit(1);
	}

	/* Open MTD device */
	if ((fd = open(argv[1], O_RDONLY)) == -1) {
		perror("open flash");
		exit (1);
	}

	/* Fill in MTD device capability structure */   
	if (ioctl(fd, MEMGETINFO, &meminfo) != 0) {
		perror("MEMGETINFO");
		close(fd);
		exit (1);
	}

	/* Make sure device page sizes are valid */
	if (!(meminfo.oobsize == 16 && meminfo.oobblock == 512) &&
	    !(meminfo.oobsize == 8 && meminfo.oobblock == 256)) {
		printf("Unknown flash (not normal NAND)\n");
		close(fd);
		exit(1);
	}

	/* Open output file for writing */
	if ((ofd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0644)) == -1) {
		perror ("open outfile");
		close(fd);
		exit(1);
	}

	/* Initialize start/end addresses and block size */
	start_addr = 0;
	end_addr = meminfo.size;
	bs = meminfo.oobblock;

	/* See if start address and length were specified */
	if (argc == 4) {
		start_addr = strtoul(argv[3], NULL, 0) & ~(bs - 1);
		end_addr = meminfo.size;
	} else if (argc == 5) {
		start_addr = strtoul(argv[3], NULL, 0) & ~(bs - 1);
		end_addr = (strtoul(argv[3], NULL, 0) + strtoul(argv[4], NULL, 0)) & ~(bs - 1);
	}

	/* Ask user if they would like pretty output */
	printf("Would you like formatted output? ");
	if (tolower(getc(stdin)) != 'y')
		pretty_print = 0;
	else
		pretty_print = 1;

	/* Print informative message */
	printf("Dumping data starting at 0x%08x and ending at 0x%08x...\n",
	       start_addr, end_addr);

	/* Dump the flash contents */
	for (ofs = start_addr; ofs < end_addr ; ofs+=bs) {

		/* Read page data and exit on failure */
		if (pread(fd, readbuf, bs, ofs) != bs) {
			perror("pread");
			close(fd);
			close(ofd);
			exit(1);
		}

		/* Write out page data */
		if (pretty_print) {
			for (i = 0; i < bs; i += 16) {
				sprintf(pretty_buf,
					"0x%08x: %02x %02x %02x %02x %02x %02x %02x "
					"%02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					(unsigned int) (ofs + i),  readbuf[i],
					readbuf[i+1], readbuf[i+2],
					readbuf[i+3], readbuf[i+4],
					readbuf[i+5], readbuf[i+6],
					readbuf[i+7], readbuf[i+8],
					readbuf[i+9], readbuf[i+10],
					readbuf[i+11], readbuf[i+12],
					readbuf[i+13], readbuf[i+14],
					readbuf[i+15]);
				write(ofd, pretty_buf, 60);
			}
		} else
			write(ofd, readbuf, bs);

		/* Read OOB data and exit on failure */
		oob.start = ofs;
		printf("Dumping %lx\n", ofs);
		if (ioctl(fd, MEMREADOOB, &oob) != 0) {
			perror("ioctl(MEMREADOOB)");
			close(fd);
			close(ofd);
			exit(1);
		}

		/* Write out OOB data */
		if (pretty_print) {
			if (meminfo.oobsize == 16) {
				sprintf(pretty_buf, "  OOB Data: %02x %02x %02x %02x %02x %02x "
					"%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
					oobbuf[0], oobbuf[1], oobbuf[2],
					oobbuf[3], oobbuf[4], oobbuf[5],
					oobbuf[6], oobbuf[7], oobbuf[8],
					oobbuf[9], oobbuf[10], oobbuf[11],
					oobbuf[12], oobbuf[13], oobbuf[14],
					oobbuf[15]);
				write(ofd, pretty_buf, 60);
			} else {
				sprintf(pretty_buf, "  OOB Data: %02x %02x %02x %02x %02x %02x "
					"%02x %02x\n",
					oobbuf[0], oobbuf[1], oobbuf[2],
					oobbuf[3], oobbuf[4], oobbuf[5],
					oobbuf[6], oobbuf[7]);
				write(ofd, pretty_buf, 48);
			}
		} else
			write(ofd, oobbuf, meminfo.oobsize);
	}

	/* Close the output file and MTD device */
	close(fd);
	close(ofd);

	/* Exit happy */
	return 0;
}
