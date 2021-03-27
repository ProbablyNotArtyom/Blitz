/*****************************************************************************/

/*
 *	hd -- simple hexdump utility
 *
 *	(C) Copyright 2000-2001, Greg Ungerer (gerg@moreton.com.au)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 * 
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/*****************************************************************************/

char	*version = "1.0.0";

char	*progname;

/*
 *	Specify output types.
 */
#define	HEX	0
#define	DECIMAL	1
#define	OCTAL	2

#define	BIT8	1
#define	BIT16	2
#define	BIT32	4

int	obase = HEX;
int	osize = BIT32;

/*****************************************************************************/

void usage(int rc)
{
	printf("usage: %s [-?vodxbcwl] [-s offset] [<filename>]\n\n", progname);
	printf("\t-?\t-- this help\n"
		"\t-v\t-- print version\n"
		"\t-s\t-- skip offset bytes from start\n"
		"\t-o\t-- output in octal\n"
		"\t-d\t-- output in decimal\n"
		"\t-x\t-- output in hex (default)\n"
		"\t-b\t-- output as bytes\n"
		"\t-c\t-- output as bytes\n"
		"\t-w\t-- output as 16 bit words\n"
		"\t-l\t-- output as 32 bit words (default)\n");
	exit(rc);
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	int		fd, c;
	int		len, i, numsize, numperline;
	char		*addrfmt, *fill, *ostr;
	char		numspaces[32], numfmt[10];
	char		numprtc;
	char		obuf[160];
	unsigned char	ibuf[16];
	unsigned long	addr, val, offset = 0L;

	progname = argv[0];

	while ((c = getopt(argc, argv, "?hvodxbcwls:")) != EOF) {
		switch (c) {
		case 'v':
			printf("%s: version %s\n", progname, version);
			exit(0);
			break;
		case 'o':
			obase = OCTAL;
			break;
		case 's':
			if (sscanf(optarg, "%i", &offset) != 1)
				usage(1);
			break;
		case 'd':
			obase = DECIMAL;
			break;
		case 'x':
			obase = HEX;
			break;
		case 'b':
		case 'c':
			osize = BIT8;
			break;
		case 'w':
			osize = BIT16;
			break;
		case 'l':
			osize = BIT32;
			break;
		case 'h':
		case '?':
			usage(0);
			break;
		default:
			fprintf(stderr, "%s: unknown option '%c'\n",
				progname, c);
			usage(1);
			break;
		}
	}

	if (optind < argc) {
		if ((fd = open(argv[optind], O_RDONLY)) < 0) {
			fprintf(stderr, "ERROR: could not open %s\n",
				argv[optind]);
			exit(1);
		}
	} else {
		fd = 0;
	}

	/*
	 *	Calculate formatting strings, line sizes, etc
	 */
	numperline = 16;

	if (obase == DECIMAL) {
		addrfmt = "%06d: ";
		numprtc = 'u';
		fill = "";
		if (osize == BIT8) {
			numsize = 3;
			numperline = 8;
		} else if (osize == BIT16) {
			numsize = 5;
		} else {
			numsize = 10;
		}
	} else if (obase == OCTAL) {
		addrfmt = "%06o: ";
		numprtc = 'o';
		numsize = (8 * osize) / 3 + 1;
                fill = "0";
		if ((osize == BIT8) || (osize == BIT16))
			numperline = 8;
	} else {
		addrfmt = "%06x: ";
		numprtc = 'x';
		numsize = 2 * osize;
		fill = "0";
	}

	sprintf(&numfmt[0], "%%%s%d%c ", fill, numsize, numprtc);
	memcpy(&numspaces[0], "                    ", 20);
	numspaces[numsize + 1] = 0;

	if (offset)
		lseek(fd, offset, SEEK_SET);
	addr = lseek(fd, 0, SEEK_CUR);

	/*
	 *	Do the actual printing.
	 */
	while ((len = read(fd, &ibuf[0], numperline)) > 0) {
		if (len < numperline)
			memset(&ibuf[len], 0, (numperline - len));

		ostr = &obuf[0];
		ostr += sprintf(ostr, addrfmt, addr);

		for (i = 0; (i < len); i += osize) {
			if (osize == BIT8)
				val = (unsigned long) ibuf[i];
			else if (osize == BIT16)
				val = (unsigned long) 
					*((unsigned short *) &ibuf[i]);
			else
				val = (unsigned long) 
					*((unsigned long *) &ibuf[i]);

			ostr += sprintf(ostr, numfmt, val);
		}
		for (; (i < numperline); i += osize)
			ostr += sprintf(ostr, numspaces);

		ostr += sprintf(ostr, "    ");

		for (i = 0; (i < len); i++) {
			if ((ibuf[i] >= 0x20) && (ibuf[i] <= 0x7f))
				ostr += sprintf(ostr, "%c", ibuf[i]);
			else
				ostr += sprintf(ostr, ".");
		}

		printf("%s\n", &obuf[0]);
		addr += len;
	}

	close(fd);
	exit(0);
}

/*****************************************************************************/
