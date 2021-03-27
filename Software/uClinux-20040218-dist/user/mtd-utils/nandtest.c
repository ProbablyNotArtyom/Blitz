/*
 *  nandtest.c
 *
 *  Copyright (C) 2000 Miguel Freitas (miguel@cetuc.puc-rio.br)
 *                     Steven J. Hill (sjhill@cotw.com)
 *
 * $Id: nandtest.c,v 1.4 2000/10/23 17:42:43 miguel Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Overview:
 *   This utility tests MTD devices that are raw NAND chips or NAND
 *   chips contained in DoC devices.
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
#include <linux/config.h>
#include <linux/mtd/mtd.h>

/*
 * Buffer arrays used for running tests
 */
unsigned char readbuf[512];
unsigned char writebuf[512];
unsigned char oobbuf[16];

/*
 * Array used for partial page writes which is only
 * possible if ECC is not being done
 */
#ifndef CONFIG_MTD_NAND_ECC
static int subsector[][2] = {
	{400, 112},
	{100, 100},
	{300, 100},
	{200, 100},
	{0, 100},
	{-1, -1}
};
#endif

/*
 * Main program
 */
int main(int argc, char **argv)
{
	unsigned long ofs;
	int bs, fd, i;
	struct mtd_oob_buf oob = {0, 16, (unsigned char *) &oobbuf};
	mtd_info_t meminfo;
	erase_info_t erase;

	/* Make sure a device was specified */
	if(argc < 2) {
		printf("usage: %s <mtdname>\n", argv[0]);
		exit(1);
	}

	/* Open the device */
	if((fd = open(argv[1], O_RDWR)) == -1) {
		perror("open flash");
		exit(1);
	}

	/* Fill in MTD device capability structure */  
	if(ioctl(fd, MEMGETINFO, &meminfo) != 0) {
		perror("MEMGETINFO");
		close(fd);
		exit(1);
	}

	/* Make sure device page sizes are valid */
	if( !(meminfo.oobsize == 16 && meminfo.oobblock == 512) &&
		!(meminfo.oobsize == 8 && meminfo.oobblock == 256) ) {
		printf("Unknown flash (not normal NAND)\n");
		close(fd);
		exit(1);
	}

	/* Get the size of a page */
	bs = meminfo.oobblock;

	/* Ask to perform destructive test */
	printf("This will erase the first block of device, continue? ");
	if(tolower(getc(stdin)) != 'y')
		exit(1);

	/* Erase first block of flash */
	erase.length = meminfo.erasesize;
	erase.start = 0;

	printf("\nPerforming Flash Erase of length %lu at offset %lu...\n",
		erase.length, erase.start);
	fflush(stdout);

	if(ioctl(fd, MEMERASE, &erase) != 0) {
		perror("\nMTD Erase failure\n");
		close(fd);
		exit(1);
	}

	/* See if block erased properly */
	printf("OK\nTest if block was erased...");
	fflush(stdout);

	for( ofs = 0; ofs < meminfo.erasesize; ofs+=512 ) 
	{
		memset(readbuf,0,512);
		memset(oobbuf,0,16);

		/* Read in a full page of data */
		if (pread(fd, readbuf, 512, ofs) != 512) {
				perror("pread");
				close(fd);
				exit(1);
		}
		oob.start = ofs;

		/* Read in the OOB data for the page */
		if (ioctl(fd, MEMREADOOB, &oob) != 0) {
			 perror("ioctl(MEMREADOOB)");
			 close(fd);
			 exit(1);
		}

		/* Check to make sure page is erased */
		for( i=0; i<512; i++)
			if(readbuf[i] != 0xff) {
				printf("Memory not erased correctly\n");
				close(fd);
				exit(1);
			}

		/* Check to make sure OOB data is erased */
		for( i=0; i<16; i++)
			if(oobbuf[i] != 0xff) {
				printf("OOB not erased correctly\n");
				close(fd);
				exit(1);
			}
	}

	/* Create master data pattern for testing */
	for( i = 0; i < 512; i++ )
		writebuf[i] = 'A' + (i%26); 

	/* Write out four full pages of data */
	printf("OK\nWriting %i byte sectors...", bs);
	fflush(stdout);

	for( ofs = (bs*4); ofs < (bs*8); ofs+=bs ) 
		if ( pwrite (fd, writebuf, bs, ofs) != bs) {
			perror("pwrite");
			close(fd);
			exit(1);
		}

	/* Read back and check each page */
	printf("OK\nReading %i byte sectors...", bs);
	fflush(stdout);

	for( ofs = (bs*4); ofs < (bs*8); ofs+=bs )
	{
		memset(readbuf, 0, bs);
		if ( pread (fd, readbuf, bs, ofs) != bs) {
			perror("pread");
			close(fd);
			exit(1);
		}

		for( i=0; i < bs; i++ )
			if( readbuf[i] != writebuf[i] )	{
				printf("0x%04x: (0x%02x) != (0x%02x)\n",
					i, readbuf[i], writebuf[i]);
				close(fd);
				exit(1);
			}	
	}

/*
 * This test is only available for DoC and raw NAND flash devices
 * where the NAND driver is not using ECC.
 */
#ifndef CONFIG_MTD_NAND_ECC
	printf("OK\nTesting subsector writing...");
	fflush(stdout);

	/* Write out sub-sectors */
	for( i=0; subsector[i][0] != -1; i++ ) 
		if (pwrite(fd, &writebuf[subsector[i][0]], subsector[i][1], 
			subsector[i][0]) != subsector[i][1]) {
			perror("pwrite");
			close(fd);
			exit(1);
		}

	/* Read back the sub-sectors page at a time */
	printf("OK\nReading back...");
	fflush(stdout);

	memset(readbuf, 0, bs);
	if (pread(fd, readbuf, bs, 0) != bs) {
		perror("pread1");
		close(fd);
		exit(1);
	}
	for( i=0; i < bs; i++ )
		if(readbuf[i] != writebuf[i]) {
			printf("0x%04x: (0x%02x) != (0x%02x)\n",
				i, readbuf[i], writebuf[i]);
			close(fd);
			exit(1);
		}

	/* Read back the sub-sectors a sub-sector at a time */
	fflush(stdout);
	memset(readbuf, 0, bs);
	for( i=0; subsector[i][0] != -1; i++ ) 
		if (pread(fd, &readbuf[subsector[i][0]], subsector[i][1], 
			subsector[i][0]) != subsector[i][1]) {
			perror("pread2");
			close(fd);
			exit(1);
		}

	/* Compare the complete read page with master */
	for( i=0; i < bs; i++) 
		if(readbuf[i] != writebuf[i]) {
			printf("0x%04x: (0x%02x) != (0x%02x)\n",
				i, readbuf[i], writebuf[i]);
			close(fd);
			exit(1);
		}
#endif

	/* Erase the block again before next test */
	erase.length = meminfo.erasesize;
	erase.start = 0;

	printf("\nPerforming Flash Erase of length %lu at offset %lu...\n",
		erase.length, erase.start);
	fflush(stdout);

	if(ioctl(fd, MEMERASE, &erase) != 0) {
		perror("\nMTD Erase failure\n");
		close(fd);
		exit(1);
	}

	/* Write test OOB data */
	printf("OK\nWriting OOB...");
	fflush(stdout);

	memcpy(oobbuf, writebuf, meminfo.oobsize);
	oob.start = 0;
	if (ioctl(fd, MEMWRITEOOB, &oob) != 0) {
		perror("ioctl(MEMWRITEOOB)");
		close(fd);
		exit(1);
	}

	/* Read back the OOB data */
	printf("OK\nReading OOB...");
	fflush(stdout);

	memset(oobbuf, 0, meminfo.oobsize);
	if (ioctl(fd, MEMREADOOB, &oob) != 0) {
		perror("ioctl(MEMREADOOB)");
		close(fd);
		exit(1);
	}

	/* Check OOB data */  
	for( i=0; i < meminfo.oobsize; i++ )
	if(oobbuf[i] != writebuf[i] ) {
		printf("0x%04x: (0x%02x) != (0x%02x)\n",
			i, oobbuf[i], writebuf[i]);
		close(fd);
		exit(1);
	}

	/* All the tests succeeded */
	printf("OK\n");
	close(fd);
	return 0;
}
