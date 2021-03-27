/* vi: set sw=4 ts=4: */
/*
 * mkswap.c - set up a linux swap device
 *
 * (C) 1991 Linus Torvalds. This file may be redistributed as per
 * the Linux copyright.
 */

/*
 * 20.12.91  -	time began. Got VM working yesterday by doing this by hand.
 *
 * Usage: mkswap [-c] [-vN] [-f] device [size-in-blocks]
 *
 *	-c   for readability checking. (Use it unless you are SURE!)
 *	-vN  for swap areas version N. (Only N=0,1 known today.)
 *      -f   for forcing swap creation even if it would smash partition table.
 *
 * The device may be a block device or an image of one, but this isn't
 * enforced (but it's not much fun on a character device :-).
 *
 * Patches from jaggy@purplet.demon.co.uk (Mike Jagdis) to make the
 * size-in-blocks parameter optional added Wed Feb  8 10:33:43 1995.
 *
 * Version 1 swap area code (for kernel 2.1.117), aeb, 981010.
 *
 * Sparc fixes, jj@ultra.linux.cz (Jakub Jelinek), 981201 - mangled by aeb.
 * V1_MAX_PAGES fixes, jj, 990325.
 *
 * 1999-02-22 Arkadiusz Mi�kiewicz <misiek@misiek.eu.org>
 * - added Native Language Support
 *
 *  from util-linux -- adapted for busybox by
 *  Erik Andersen <andersee@debian.org>. I ripped out Native Language
 *  Support, made some stuff smaller, and fitted for life in busybox.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>			/* for _IO */
#include <sys/utsname.h>
#include <asm/page.h>			/* for PAGE_SIZE and PAGE_SHIFT */
				/* we also get PAGE_SIZE via getpagesize() */
#include "busybox.h"

#ifndef _IO
/* pre-1.3.45 */
static const int BLKGETSIZE = 0x1260;
#else
/* same on i386, m68k, arm; different on alpha, mips, sparc, ppc */
#define BLKGETSIZE _IO(0x12,96)
#endif

static char *device_name = NULL;
static int DEV = -1;
static long PAGES = 0;
static int check = 0;
static int badpages = 0;
static int version = -1;

#define MAKE_VERSION(p,q,r)	(65536*(p) + 256*(q) + (r))

/*
 * The definition of the union swap_header uses the constant PAGE_SIZE.
 * Unfortunately, on some architectures this depends on the hardware model,
 * and can only be found at run time -- we use getpagesize().
 */

static int pagesize;
static int *signature_page;

static struct swap_header_v1 {
	char bootbits[1024];		/* Space for disklabel etc. */
	unsigned int version;
	unsigned int last_page;
	unsigned int nr_badpages;
	unsigned int padding[125];
	unsigned int badpages[1];
} *p;

static inline void init_signature_page()
{
	pagesize = getpagesize();

#ifdef PAGE_SIZE
	if (pagesize != PAGE_SIZE)
		error_msg("Assuming pages of size %d", pagesize);
#endif
	signature_page = (int *) xmalloc(pagesize);
	memset(signature_page, 0, pagesize);
	p = (struct swap_header_v1 *) signature_page;
}

static inline void write_signature(char *sig)
{
	char *sp = (char *) signature_page;

	strncpy(sp + pagesize - 10, sig, 10);
}

#define V0_MAX_PAGES	(8 * (pagesize - 10))
/* Before 2.2.0pre9 */
#define V1_OLD_MAX_PAGES	((0x7fffffff / pagesize) - 1)
/* Since 2.2.0pre9:
   error if nr of pages >= SWP_OFFSET(SWP_ENTRY(0,~0UL))
   with variations on
	#define SWP_ENTRY(type,offset) (((type) << 1) | ((offset) << 8))
	#define SWP_OFFSET(entry) ((entry) >> 8)
   on the various architectures. Below the result - yuk.

   Machine	pagesize	SWP_ENTRY	SWP_OFFSET	bound+1	oldbound+2
   i386		2^12		o<<8		e>>8		1<<24	1<<19
   mips		2^12		o<<15		e>>15		1<<17	1<<19
   alpha	2^13		o<<40		e>>40		1<<24	1<<18
   m68k		2^12		o<<12		e>>12		1<<20	1<<19
   sparc	2^{12,13}	(o&0x3ffff)<<9	(e>>9)&0x3ffff	1<<18	1<<{19,18}
   sparc64	2^13		o<<13		e>>13		1<<51	1<<18
   ppc		2^12		o<<8		e>>8		1<<24	1<<19
   armo		2^{13,14,15}	o<<8		e>>8		1<<24	1<<{18,17,16}
   armv		2^12		o<<9		e>>9		1<<23	1<<19

   assuming that longs have 64 bits on alpha and sparc64 and 32 bits elsewhere.

   The bad part is that we need to know this since the kernel will
   refuse a swap space if it is too large.
*/
/* patch from jj - why does this differ from the above? */
#if defined(__alpha__)
#define V1_MAX_PAGES           ((1 << 24) - 1)
#elif defined(__mips__)
#define V1_MAX_PAGES           ((1 << 17) - 1)
#elif defined(__sparc_v9__)
#define V1_MAX_PAGES           ((3 << 29) - 1)
#elif defined(__sparc__)
#define V1_MAX_PAGES           (pagesize == 8192 ? ((3 << 29) - 1) : ((1 << 18) - 1))
#else
#define V1_MAX_PAGES           V1_OLD_MAX_PAGES
#endif
/* man page now says:
The maximum useful size of a swap area now depends on the architecture.
It is roughly 2GB on i386, PPC, m68k, ARM, 1GB on sparc, 512MB on mips,
128GB on alpha and 3TB on sparc64.
*/

#define MAX_BADPAGES	((pagesize-1024-128*sizeof(int)-10)/sizeof(int))

static inline void bit_set(unsigned int *addr, unsigned int nr)
{
	unsigned int r, m;

	addr += nr / (8 * sizeof(int));

	r = *addr;
	m = 1 << (nr & (8 * sizeof(int) - 1));

	*addr = r | m;
}

static int bit_test_and_clear(unsigned int *addr, unsigned int nr)
{
	unsigned int r, m;

	addr += nr / (8 * sizeof(int));

	r = *addr;
	m = 1 << (nr & (8 * sizeof(int) - 1));

	*addr = r & ~m;
	return (r & m) != 0;
}


static void page_ok(int page)
{
	if (version == 0)
		bit_set(signature_page, page);
}

static inline void page_bad(int page)
{
	if (version == 0)
		bit_test_and_clear(signature_page, page);
	else {
		if (badpages == MAX_BADPAGES)
			error_msg_and_die("too many bad pages");
		p->badpages[badpages] = page;
	}
	badpages++;
}

static void check_blocks(void)
{
	unsigned int current_page;
	int do_seek = 1;
	char *buffer;

	buffer = xmalloc(pagesize);
	current_page = 0;
	while (current_page < PAGES) {
		if (!check) {
			page_ok(current_page++);
			continue;
		}
		if (do_seek && lseek(DEV, current_page * pagesize, SEEK_SET) !=
			current_page * pagesize)
			error_msg_and_die("seek failed in check_blocks");
		if ((do_seek = (pagesize != read(DEV, buffer, pagesize)))) {
			page_bad(current_page++);
			continue;
		}
		page_ok(current_page++);
	}
	if (badpages == 1)
		printf("one bad page\n");
	else if (badpages > 1)
		printf("%d bad pages\n", badpages);
}

static long valid_offset(int fd, int offset)
{
	char ch;

	if (lseek(fd, offset, 0) < 0)
		return 0;
	if (read(fd, &ch, 1) < 1)
		return 0;
	return 1;
}

static int find_size(int fd)
{
	unsigned int high, low;

	low = 0;
	for (high = 1; high > 0 && valid_offset(fd, high); high *= 2)
		low = high;
	while (low < high - 1) {
		const int mid = (low + high) / 2;

		if (valid_offset(fd, mid))
			low = mid;
		else
			high = mid;
	}
	return (low + 1);
}

/* return size in pages, to avoid integer overflow */
static long get_size(const char *file)
{
	int fd;
	long size;

	if ((fd = open(file, O_RDONLY)) < 0)
		perror_msg_and_die("%s", file);
	if (ioctl(fd, BLKGETSIZE, &size) >= 0) {
		int sectors_per_page = pagesize / 512;

		size /= sectors_per_page;
	} else {
		size = find_size(fd) / pagesize;
	}
	close(fd);
	return size;
}

int mkswap_main(int argc, char **argv)
{
	char *tmp;
	struct stat statbuf;
	int sz;
	int maxpages;
	int goodpages;
	int offset;
	int force = 0;

	init_signature_page();		/* get pagesize */

	while (argc-- > 1) {
		argv++;
		if (argv[0][0] != '-') {
			if (device_name) {
				int blocks_per_page = pagesize / 1024;

				PAGES = strtol(argv[0], &tmp, 0) / blocks_per_page;
				if (*tmp)
					show_usage();
			} else
				device_name = argv[0];
		} else {
			switch (argv[0][1]) {
			case 'c':
				check = 1;
				break;
			case 'f':
				force = 1;
				break;
			case 'v':
				version = atoi(argv[0] + 2);
				break;
			default:
				show_usage();
			}
		}
	}
	if (!device_name) {
		error_msg("error: Nowhere to set up swap on?");
		show_usage();
	}
	sz = get_size(device_name);
	if (!PAGES) {
		PAGES = sz;
	} else if (PAGES > sz && !force) {
		error_msg("error: size %ld is larger than device size %d",
				PAGES * (pagesize / 1024), sz * (pagesize / 1024));
		return EXIT_FAILURE;
	}

	if (version == -1) {
		if (PAGES <= V0_MAX_PAGES)
			version = 0;
		else if (get_kernel_revision() < MAKE_VERSION(2, 1, 117))
			version = 0;
		else if (pagesize < 2048)
			version = 0;
		else
			version = 1;
	}
	if (version != 0 && version != 1) {
		error_msg("error: unknown version %d", version);
		show_usage();
	}
	if (PAGES < 10) {
		error_msg("error: swap area needs to be at least %ldkB",
				(long) (10 * pagesize / 1024));
		show_usage();
	}
#if 0
	maxpages = ((version == 0) ? V0_MAX_PAGES : V1_MAX_PAGES);
#else
	if (!version)
		maxpages = V0_MAX_PAGES;
	else if (get_kernel_revision() >= MAKE_VERSION(2, 2, 1))
		maxpages = V1_MAX_PAGES;
	else {
		maxpages = V1_OLD_MAX_PAGES;
		if (maxpages > V1_MAX_PAGES)
			maxpages = V1_MAX_PAGES;
	}
#endif
	if (PAGES > maxpages) {
		PAGES = maxpages;
		error_msg("warning: truncating swap area to %ldkB",
				PAGES * pagesize / 1024);
	}

	DEV = open(device_name, O_RDWR);
	if (DEV < 0 || fstat(DEV, &statbuf) < 0)
		perror_msg_and_die("%s", device_name);
	if (!S_ISBLK(statbuf.st_mode))
		check = 0;
	else if (statbuf.st_rdev == 0x0300 || statbuf.st_rdev == 0x0340)
		error_msg_and_die("Will not try to make swapdevice on '%s'", device_name);

#ifdef __sparc__
	if (!force && version == 0) {
		/* Don't overwrite partition table unless forced */
		unsigned char *buffer = (unsigned char *) signature_page;
		unsigned short *q, sum;

		if (read(DEV, buffer, 512) != 512)
			error_msg_and_die("fatal: first page unreadable");
		if (buffer[508] == 0xDA && buffer[509] == 0xBE) {
			q = (unsigned short *) (buffer + 510);
			for (sum = 0; q >= (unsigned short *) buffer;)
				sum ^= *q--;
			if (!sum) {
				error_msg("Device '%s' contains a valid Sun disklabel.\n"
"This probably means creating v0 swap would destroy your partition table\n"
"No swap created. If you really want to create swap v0 on that device, use\n"
"the -f option to force it.", device_name);
				return EXIT_FAILURE;
			}
		}
	}
#endif

	if (version == 0 || check)
		check_blocks();
	if (version == 0 && !bit_test_and_clear(signature_page, 0))
		error_msg_and_die("fatal: first page unreadable");
	if (version == 1) {
		p->version = version;
		p->last_page = PAGES - 1;
		p->nr_badpages = badpages;
	}

	goodpages = PAGES - badpages - 1;
	if (goodpages <= 0)
		error_msg_and_die("Unable to set up swap-space: unreadable");
	printf("Setting up swapspace version %d, size = %ld bytes\n",
		   version, (long) (goodpages * pagesize));
	write_signature((version == 0) ? "SWAP-SPACE" : "SWAPSPACE2");

	offset = ((version == 0) ? 0 : 1024);
	if (lseek(DEV, offset, SEEK_SET) != offset)
		error_msg_and_die("unable to rewind swap-device");
	if (write(DEV, (char *) signature_page + offset, pagesize - offset)
		!= pagesize - offset)
		error_msg_and_die("unable to write signature page");

	/*
	 * A subsequent swapon() will fail if the signature
	 * is not actually on disk. (This is a kernel bug.)
	 */
	if (fsync(DEV))
		error_msg_and_die("fsync failed");
	return EXIT_SUCCESS;
}
