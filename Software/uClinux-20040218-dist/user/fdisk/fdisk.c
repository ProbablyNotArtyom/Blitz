/* fdisk.c -- Partition table manipulator for Linux.
 *
 * Copyright (C) 1992  A. V. Le Blanc (LeBlanc@mcc.ac.uk)
 *
 * This program is free software.  You can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation: either version 1 or
 * (at your option) any later version.
 *
 * For detailed old history, see older versions.
 * Contributions before 2000 by faith@cs.unc.edu, Michael Bischoff,
 * LeBlanc@mcc.ac.uk, martin@cs.unc.edu, leisner@sdsp.mc.xerox.com,
 * esr@snark.thyrsus.com, aeb@cwi.nl, quinlan@yggdrasil.com,
 * fasten@cs.bonn.edu, orschaer@cip.informatik.uni-erlangen.de,
 * jj@sunsite.mff.cuni.cz, fasten@shw.com, ANeuper@GUUG.de,
 * kgw@suse.de.
 * 
 * Modified, Sun Feb 20 2000, kalium@gmx.de
 * Added fix operation allowing to reorder primary/extended partition
 * entries within the partition table. Some programs or OSes have
 * problems using a partition table with entries not ordered
 * according to their positions on disk.
 * Munged this patch to also make it work for logical partitions.
 * aeb, 2000-02-20.
 *
 * Wed Mar 1 14:34:53 EST 2000 David Huggins-Daines <dhuggins@linuxcare.com>
 * Better support for OSF/1 disklabels on Alpha.
 *
 * 2000-04-06, Michal Jaegermann (michal@ellpspace.math.ualberta.ca)
 * fixed and added some alpha stuff.
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>

#include <linux/hdreg.h>       /* for HDIO_GETGEO */

#include "nls.h"
#include "common.h"
#include "fdisk.h"

#include "fdisksunlabel.h"
#include "fdisksgilabel.h"
#include "fdiskaixlabel.h"

#include "defines.h"
#ifdef HAVE_blkpg_h
#include <linux/blkpg.h>
#endif

#define hex_val(c)	({ \
				char _c = (c); \
				isdigit(_c) ? _c - '0' : \
				tolower(_c) + 10 - 'a'; \
			})


#define LINE_LENGTH	80
#define pt_offset(b, n)	((struct partition *)((b) + 0x1be + \
				(n) * sizeof(struct partition)))
#define sector(s)	((s) & 0x3f)
#define cylinder(s, c)	((c) | (((s) & 0xc0) << 2))

#define hsc2sector(h,s,c) (sector(s) - 1 + sectors * \
				((h) + heads * cylinder(s,c)))
#define set_hsc(h,s,c,sector) { \
				s = sector % sectors + 1;	\
				sector /= sectors;	\
				h = sector % heads;	\
				sector /= heads;	\
				c = sector & 0xff;	\
				s |= (sector >> 2) & 0xc0;	\
			}

/* A valid partition table sector ends in 0x55 0xaa */
static unsigned int
part_table_flag(char *b) {
	return ((uint) b[510]) + (((uint) b[511]) << 8);
}

int
valid_part_table_flag(unsigned char *b) {
	return (b[510] == 0x55 && b[511] == 0xaa);
}

static void
write_part_table_flag(char *b) {
	b[510] = 0x55;
	b[511] = 0xaa;
}

/* start_sect and nr_sects are stored little endian on all machines */
/* moreover, they are not aligned correctly */
static void
store4_little_endian(unsigned char *cp, unsigned int val) {
	cp[0] = (val & 0xff);
	cp[1] = ((val >> 8) & 0xff);
	cp[2] = ((val >> 16) & 0xff);
	cp[3] = ((val >> 24) & 0xff);
}

static unsigned int
read4_little_endian(unsigned char *cp) {
	return (uint)(cp[0]) + ((uint)(cp[1]) << 8)
		+ ((uint)(cp[2]) << 16) + ((uint)(cp[3]) << 24);
}

static void
set_start_sect(struct partition *p, unsigned int start_sect) {
	store4_little_endian(p->start4, start_sect);
}

unsigned int
get_start_sect(struct partition *p) {
	return read4_little_endian(p->start4);
}

static void
set_nr_sects(struct partition *p, unsigned int nr_sects) {
	store4_little_endian(p->size4, nr_sects);
}

unsigned int
get_nr_sects(struct partition *p) {
	return read4_little_endian(p->size4);
}

/* normally O_RDWR, -l option gives O_RDONLY */
static int type_open = O_RDWR;

/*
 * Raw disk label. For DOS-type partition tables the MBR,
 * with descriptions of the primary partitions.
 */
char MBRbuffer[MAX_SECTOR_SIZE];

/*
 * per partition table entry data
 *
 * The four primary partitions have the same sectorbuffer (MBRbuffer)
 * and have NULL ext_pointer.
 * Each logical partition table entry has two pointers, one for the
 * partition and one link to the next one.
 */
struct pte {
	struct partition *part_table;	/* points into sectorbuffer */
	struct partition *ext_pointer;	/* points into sectorbuffer */
	char changed;		/* boolean */
	uint offset;		/* disk sector number */
	char *sectorbuffer;	/* disk sector contents */
} ptes[MAXIMUM_PARTS];

char	*disk_device,			/* must be specified */
	*line_ptr,			/* interactive input */
	line_buffer[LINE_LENGTH];

int	fd,				/* the disk */
	ext_index,			/* the prime extended partition */
	listing = 0,			/* no aborts for fdisk -l */
	nowarn = 0,			/* no warnings for fdisk -l/-s */
	dos_compatible_flag = ~0,
	partitions = 4;			/* maximum partition + 1 */

uint	user_cylinders, user_heads, user_sectors;

uint	heads,
	sectors,
	cylinders,
	sector_size = DEFAULT_SECTOR_SIZE,
	sector_offset = 1,
	units_per_sector = 1,
	display_in_cyl_units = 1,
	extended_offset = 0;		/* offset of link pointers */

#define dos_label (!sun_label && !sgi_label && !aix_label && !osf_label)
int     sun_label = 0;                  /* looking at sun disklabel */
int	sgi_label = 0;			/* looking at sgi disklabel */
int	aix_label = 0;			/* looking at aix disklabel */
int	osf_label = 0;			/* looking at osf disklabel */
jmp_buf listingbuf;

void fatal(enum failure why) {
	char	error[LINE_LENGTH],
		*message = error;

	if (listing) {
		close(fd);
		longjmp(listingbuf, 1);
	}

	switch (why) {
		case usage: message = _(
"Usage: fdisk [-b SSZ] [-u] DISK     Change partition table\n"
"       fdisk -l [-b SSZ] [-u] DISK  List partition table(s)\n"
"       fdisk -s PARTITION           Give partition size(s) in blocks\n"
"       fdisk -v                     Give fdisk version\n"
"Here DISK is something like /dev/hdb or /dev/sda\n"
"and PARTITION is something like /dev/hda7\n"
"-u: give Start and End in sector (instead of cylinder) units\n"
"-b 2048: (for certain MO drives) use 2048-byte sectors\n");
			break;
		case usage2:
		  /* msg in cases where fdisk used to probe */
			message = _(
"Usage: fdisk [-l] [-b SSZ] [-u] device\n"
"E.g.: fdisk /dev/hda  (for the first IDE disk)\n"
"  or: fdisk /dev/sdc  (for the third SCSI disk)\n"
"  or: fdisk /dev/eda  (for the first PS/2 ESDI drive)\n"
"  or: fdisk /dev/rd/c0d0  or: fdisk /dev/ida/c0d0  (for RAID devices)\n"
"  ...\n");
			break;
		case unable_to_open:
			sprintf(error, _("Unable to open %s\n"), disk_device);
			break;
		case unable_to_read:
			sprintf(error, _("Unable to read %s\n"), disk_device);
			break;
		case unable_to_seek:
			sprintf(error, _("Unable to seek on %s\n"),disk_device);
			break;
		case unable_to_write:
			sprintf(error, _("Unable to write %s\n"), disk_device);
			break;
		case ioctl_error:
			sprintf(error, _("BLKGETSIZE ioctl failed on %s\n"),
				disk_device);
			break;
		case out_of_memory:
			message = _("Unable to allocate any more memory\n");
			break;
		default: message = _("Fatal error\n");
	}

	fputc('\n', stderr);
	fputs(message, stderr);
	exit(1);
}

static void
seek_sector(int fd, uint secno) {
	ext2_loff_t offset = (ext2_loff_t) secno * sector_size;
	if (ext2_llseek(fd, offset, SEEK_SET) == (ext2_loff_t) -1)
		fatal(unable_to_seek);
}

static void
read_sector(int fd, uint secno, char *buf) {
	seek_sector(fd, secno);
	if (read(fd, buf, sector_size) != sector_size)
		fatal(unable_to_read);
}

static void
write_sector(int fd, uint secno, char *buf) {
	seek_sector(fd, secno);
	if (write(fd, buf, sector_size) != sector_size)
		fatal(unable_to_write);
}

/* Allocate a buffer and read a partition table sector */
static void
read_pte(int fd, int pno, uint offset) {
	struct pte *pe = &ptes[pno];

	pe->offset = offset;
	pe->sectorbuffer = (char *) malloc(sector_size);
	if (!pe->sectorbuffer)
		fatal(out_of_memory);
	read_sector(fd, offset, pe->sectorbuffer);
	pe->changed = 0;
	pe->part_table = pe->ext_pointer = NULL;
}
	
static unsigned int
get_partition_start(struct pte *pe) {
	return pe->offset + get_start_sect(pe->part_table);
}

struct partition *
get_part_table(int i) {
	return ptes[i].part_table;
}

void
set_all_unchanged(void) {
	int i;

	for (i = 0; i < MAXIMUM_PARTS; i++)
		ptes[i].changed = 0;
}

void
set_changed(int i) {
	ptes[i].changed = 1;
}

static void
menu(void) {
	if (sun_label) {
	   puts(_("Command action"));
	   puts(_("   a   toggle a read only flag")); 		/* sun */
	   puts(_("   b   edit bsd disklabel"));
	   puts(_("   c   toggle the mountable flag"));		/* sun */
	   puts(_("   d   delete a partition"));
	   puts(_("   l   list known partition types"));
	   puts(_("   m   print this menu"));
	   puts(_("   n   add a new partition"));
	   puts(_("   o   create a new empty DOS partition table"));
	   puts(_("   p   print the partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   s   create a new empty Sun disklabel"));	/* sun */
	   puts(_("   t   change a partition's system id"));
	   puts(_("   u   change display/entry units"));
	   puts(_("   v   verify the partition table"));
	   puts(_("   w   write table to disk and exit"));
	   puts(_("   x   extra functionality (experts only)"));
	}
	else if(sgi_label) {
	   puts(_("Command action"));
	   puts(_("   a   select bootable partition"));    /* sgi flavour */
	   puts(_("   b   edit bootfile entry"));          /* sgi */
	   puts(_("   c   select sgi swap partition"));    /* sgi flavour */
	   puts(_("   d   delete a partition"));
	   puts(_("   l   list known partition types"));
	   puts(_("   m   print this menu"));
	   puts(_("   n   add a new partition"));
	   puts(_("   o   create a new empty DOS partition table"));
	   puts(_("   p   print the partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   s   create a new empty Sun disklabel"));	/* sun */
	   puts(_("   t   change a partition's system id"));
	   puts(_("   u   change display/entry units"));
	   puts(_("   v   verify the partition table"));
	   puts(_("   w   write table to disk and exit"));
	}
	else if(aix_label) {
	   puts(_("Command action"));
	   puts(_("   m   print this menu"));
	   puts(_("   o   create a new empty DOS partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   s   create a new empty Sun disklabel"));	/* sun */
	}
	else {
	   puts(_("Command action"));
	   puts(_("   a   toggle a bootable flag"));
	   puts(_("   b   edit bsd disklabel"));
	   puts(_("   c   toggle the dos compatibility flag"));
	   puts(_("   d   delete a partition"));
	   puts(_("   l   list known partition types"));
	   puts(_("   m   print this menu"));
	   puts(_("   n   add a new partition"));
	   puts(_("   o   create a new empty DOS partition table"));
	   puts(_("   p   print the partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   s   create a new empty Sun disklabel"));	/* sun */
	   puts(_("   t   change a partition's system id"));
	   puts(_("   u   change display/entry units"));
	   puts(_("   v   verify the partition table"));
	   puts(_("   w   write table to disk and exit"));
	   puts(_("   x   extra functionality (experts only)"));
	}
}

static void
xmenu(void) {
	if (sun_label) {
	   puts(_("Command action"));
	   puts(_("   a   change number of alternate cylinders"));      /*sun*/
	   puts(_("   c   change number of cylinders"));
	   puts(_("   d   print the raw data in the partition table"));
	   puts(_("   e   change number of extra sectors per cylinder"));/*sun*/
	   puts(_("   h   change number of heads"));
	   puts(_("   i   change interleave factor"));			/*sun*/
	   puts(_("   o   change rotation speed (rpm)"));		/*sun*/
	   puts(_("   m   print this menu"));
	   puts(_("   p   print the partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   r   return to main menu"));
	   puts(_("   s   change number of sectors/track"));
	   puts(_("   v   verify the partition table"));
	   puts(_("   w   write table to disk and exit"));
	   puts(_("   y   change number of physical cylinders"));	/*sun*/
	}
	else if(sgi_label) {
	   puts(_("Command action"));
	   puts(_("   b   move beginning of data in a partition")); /* !sun */
	   puts(_("   c   change number of cylinders"));
	   puts(_("   d   print the raw data in the partition table"));
	   puts(_("   e   list extended partitions"));		/* !sun */
	   puts(_("   g   create an IRIX partition table"));	/* sgi */
	   puts(_("   h   change number of heads"));
	   puts(_("   m   print this menu"));
	   puts(_("   p   print the partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   r   return to main menu"));
	   puts(_("   s   change number of sectors/track"));
	   puts(_("   v   verify the partition table"));
	   puts(_("   w   write table to disk and exit"));
	}
	else if(aix_label) {
	   puts(_("Command action"));
	   puts(_("   b   move beginning of data in a partition")); /* !sun */
	   puts(_("   c   change number of cylinders"));
	   puts(_("   d   print the raw data in the partition table"));
	   puts(_("   e   list extended partitions"));		/* !sun */
	   puts(_("   g   create an IRIX partition table"));	/* sgi */
	   puts(_("   h   change number of heads"));
	   puts(_("   m   print this menu"));
	   puts(_("   p   print the partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   r   return to main menu"));
	   puts(_("   s   change number of sectors/track"));
	   puts(_("   v   verify the partition table"));
	   puts(_("   w   write table to disk and exit"));
	}
	else {
	   puts(_("Command action"));
	   puts(_("   b   move beginning of data in a partition")); /* !sun */
	   puts(_("   c   change number of cylinders"));
	   puts(_("   d   print the raw data in the partition table"));
	   puts(_("   e   list extended partitions"));		/* !sun */
	   puts(_("   f   fix partition order"));		/* !sun, !aix, !sgi */
	   puts(_("   g   create an IRIX partition table"));	/* sgi */
	   puts(_("   h   change number of heads"));
	   puts(_("   m   print this menu"));
	   puts(_("   p   print the partition table"));
	   puts(_("   q   quit without saving changes"));
	   puts(_("   r   return to main menu"));
	   puts(_("   s   change number of sectors/track"));
	   puts(_("   v   verify the partition table"));
	   puts(_("   w   write table to disk and exit"));
	}
}

static int
get_sysid(int i) {
	return (
		sun_label ? sunlabel->infos[i].id :
		sgi_label ? sgi_get_sysid(i) : ptes[i].part_table->sys_ind);
}

static struct systypes *
get_sys_types(void) {
	return (
		sun_label ? sun_sys_types :
		sgi_label ? sgi_sys_types : i386_sys_types);
}

char *partition_type(unsigned char type)
{
	int i;
	struct systypes *types = get_sys_types();

	for (i=0; types[i].name; i++)
		if (types[i].type == type)
			return _(types[i].name);

	return NULL;
}

void list_types(struct systypes *sys)
{
	uint last[4], done = 0, next = 0, size;
	int i;

	for (i = 0; sys[i].name; i++);
	size = i;

	for (i = 3; i >= 0; i--)
		last[3 - i] = done += (size + i - done) / (i + 1);
	i = done = 0;

	do {
		printf("%c%2x  %-15.15s", i ? ' ' : '\n',
		        sys[next].type, _(sys[next].name));
 		next = last[i++] + done;
		if (i > 3 || next >= last[i]) {
			i = 0;
			next = ++done;
		}
	} while (done < last[0]);
	putchar('\n');
}

static void
clear_partition(struct partition *p) {
	if (!p)
		return;
	p->boot_ind = 0;
	p->head = 0;
	p->sector = 0;
	p->cyl = 0;
	p->sys_ind = 0;
	p->end_head = 0;
	p->end_sector = 0;
	p->end_cyl = 0;
	set_start_sect(p,0);
	set_nr_sects(p,0);
}

static void
set_partition(int i, struct partition *p, uint start, uint stop,
	      int sysid, uint offset) {
	p->boot_ind = 0;
	p->sys_ind = sysid;
	set_start_sect(p, start - offset);
	set_nr_sects(p, stop - start + 1);
	if (dos_compatible_flag && (start/(sectors*heads) > 1023))
		start = heads*sectors*1024 - 1;
	set_hsc(p->head, p->sector, p->cyl, start);
	if (dos_compatible_flag && (stop/(sectors*heads) > 1023))
		stop = heads*sectors*1024 - 1;
	set_hsc(p->end_head, p->end_sector, p->end_cyl, stop);
	ptes[i].changed = 1;
}

static int
test_c(char **m, char *mesg) {
	int val = 0;
	if (!*m)
		fprintf(stderr, _("You must set"));
	else {
		fprintf(stderr, " %s", *m);
		val = 1;
	}
	*m = mesg;
	return val;
}

static int
warn_geometry(void) {
	char *m = NULL;
	int prev = 0;
	if (!heads)
		prev = test_c(&m, _("heads"));
	if (!sectors)
		prev = test_c(&m, _("sectors"));
	if (!cylinders)
		prev = test_c(&m, _("cylinders"));
	if (!m)
		return 0;
	fprintf(stderr,
		_("%s%s.\nYou can do this from the extra functions menu.\n"),
		prev ? _(" and ") : " ", m);
	return 1;
}

void update_units(void)
{
	int cyl_units = heads * sectors;

	if (display_in_cyl_units && cyl_units)
		units_per_sector = cyl_units;
	else
		units_per_sector = 1; 	/* in sectors */
}

static void
warn_cylinders(void) {
	if (dos_label && cylinders > 1024 && !nowarn)
		fprintf(stderr, "\n\
The number of cylinders for this disk is set to %d.\n\
There is nothing wrong with that, but this is larger than 1024,\n\
and could in certain setups cause problems with:\n\
1) software that runs at boot time (e.g., old versions of LILO)\n\
2) booting and partitioning software from other OSs\n\
   (e.g., DOS FDISK, OS/2 FDISK)\n",
			cylinders);
}

static void
read_extended(int ext) {
	int i;
	struct pte *pex;
	struct partition *p, *q;

	ext_index = ext;
	pex = &ptes[ext];
	pex->ext_pointer = pex->part_table;

	p = pex->part_table;
	if (!get_start_sect(p))
		fprintf(stderr, _("Bad offset in primary extended partition\n"));
	else while (IS_EXTENDED (p->sys_ind)) {
		struct pte *pe = &ptes[partitions];

		if (partitions >= MAXIMUM_PARTS) {
			/* This is not a Linux restriction, but
			   this program uses arrays of size MAXIMUM_PARTS.
			   Do not try to `improve' this test. */
			struct pte *pre = &ptes[partitions-1];

			fprintf(stderr,
				_("Warning: deleting partitions after %d\n"),
				partitions);
			clear_partition(pre->ext_pointer);
			pre->changed = 1;
			return;
		}

		read_pte(fd, partitions, extended_offset + get_start_sect(p));

		if (!extended_offset)
			extended_offset = get_start_sect(p);

		q = p = pt_offset(pe->sectorbuffer, 0);
		for (i = 0; i < 4; i++, p++) {
			if (IS_EXTENDED (p->sys_ind)) {
				if (pe->ext_pointer)
					fprintf(stderr, _("Warning: extra link"
							  " pointer in partition table "
							  "%d\n"), partitions + 1);
				else
					pe->ext_pointer = p;
			} else if (p->sys_ind) {
				if (pe->part_table)
					fprintf(stderr,
						_("Warning: ignoring extra data "
						  "in partition table %d\n"),
						partitions + 1);
				else
					pe->part_table = p;
			}
		}

		/* very strange code here... */
		if (!pe->part_table) {
			if (q != pe->ext_pointer)
				pe->part_table = q;
			else
				pe->part_table = q + 1;
		}
		if (!pe->ext_pointer) {
			if (q != pe->part_table)
				pe->ext_pointer = q;
			else
				pe->ext_pointer = q + 1;
		}

		p = pe->ext_pointer;
		partitions++;
	}
}

static void
create_doslabel(void) {
	int i;

	fprintf(stderr,
	_("Building a new DOS disklabel. Changes will remain in memory only,\n"
	  "until you decide to write them. After that, of course, the previous\n"
	  "content won't be recoverable.\n\n"));

	sun_nolabel();  /* otherwise always recognised as sun */
	sgi_nolabel();  /* otherwise always recognised as sgi */

	write_part_table_flag(MBRbuffer);
	for (i = 0; i < 4; i++)
	    if (ptes[i].part_table)
		clear_partition(ptes[i].part_table);
	set_all_unchanged();
	set_changed(0);
	get_boot(create_empty);
}

static void
get_sectorsize(int fd) {
#if defined(BLKSSZGET) && defined(HAVE_blkpg_h)
	/* For a short while BLKSSZGET gave a wrong sector size */
	{ int arg;
	if (ioctl(fd, BLKSSZGET, &arg) == 0)
		sector_size = arg;
	if (sector_size != DEFAULT_SECTOR_SIZE)
		printf(_("Note: sector size is %d (not %d)\n"),
		       sector_size, DEFAULT_SECTOR_SIZE);
	}
#else
	sector_size = DEFAULT_SECTOR_SIZE;
#endif
}

/*
 * Ask kernel about geometry. Invent something reasonable
 * in case the kernel has no opinion.
 */
void
get_geometry(int fd, struct geom *g) {
	int sec_fac;
	long longsectors;
	struct hd_geometry geometry;
	int res1, res2;

	get_sectorsize(fd);
	sec_fac = sector_size / 512;

	guess_device_type(fd);

	res1 = ioctl(fd, BLKGETSIZE, &longsectors);
#ifdef HDIO_REQ
	res2 = ioctl(fd, HDIO_REQ, &geometry);
#else
	res2 = ioctl(fd, HDIO_GETGEO, &geometry);
#endif

	/* never use geometry.cylinders - it is truncated */
	heads = cylinders = sectors = 0;
	sector_offset = 1;
	if (res2 == 0) {
		heads = geometry.heads;
		sectors = geometry.sectors;
		if (heads * sectors == 0)
			res2 = -1;
		else if (dos_compatible_flag)
			sector_offset = sectors;
	}
	if (res1 == 0 && res2 == 0) { 	/* normal case */
		cylinders = longsectors / (heads * sectors);
		cylinders /= sec_fac;   /* do not round up */
	} else if (res1 == 0) {		/* size but no geometry */
		heads = cylinders = 1;
		sectors = longsectors / sec_fac;
	}

	if (!sectors)
		sectors = user_sectors;
	if (!heads)
		heads = user_heads;
	if (!cylinders)
		cylinders = user_cylinders;

	if (g) {
		g->heads = heads;
		g->sectors = sectors;
		g->cylinders = cylinders;
	}
}

/*
 * Read MBR.  Returns:
 *   -1: no 0xaa55 flag present (possibly entire disk BSD)
 *    0: found or created label
 */
int
get_boot(enum action what) {
	int i;

	partitions = 4;

	if (what == create_empty)
		goto got_table;		/* skip reading disk */

	if ((fd = open(disk_device, type_open)) < 0) {
	    if ((fd = open(disk_device, O_RDONLY)) < 0)
		fatal(unable_to_open);
	    else
		printf(_("You will not be able to write the partition table.\n"));
	}

	get_geometry(fd, NULL);

	update_units();

	if (sector_size != read(fd, MBRbuffer, sector_size))
		fatal(unable_to_read);

got_table:

	if (check_sun_label())
		return 0;

	if (check_sgi_label())
		return 0;

	if (check_aix_label())
		return 0;

	if (check_osf_label())
		return 0;

	if (!valid_part_table_flag(MBRbuffer)) {
		switch(what) {
		case fdisk:
			fprintf(stderr,
				_("Device contains neither a valid DOS partition"
			 	  " table, nor Sun, SGI or OSF disklabel\n"));
#ifdef __sparc__
			create_sunlabel();
#else
			create_doslabel();
#endif
			return 0;
		case require:
			return -1;
		case try_only:
		        return -1;
		case create_empty:
			break;
		}

		fprintf(stderr, _("Internal error\n"));
		exit(1);
	}

	warn_cylinders();
	warn_geometry();

	for (i = 0; i < 4; i++) {
		struct pte *pe = &ptes[i];

		pe->part_table = pt_offset(MBRbuffer, i);
		pe->ext_pointer = NULL;
		pe->changed = 0;
		pe->offset = 0;
		pe->sectorbuffer = MBRbuffer;
	}

	for (i = 0; i < 4; i++) {
		struct pte *pe = &ptes[i];

		if (IS_EXTENDED (pe->part_table->sys_ind)) {
			if (partitions != 4)
				fprintf(stderr, _("Ignoring extra extended "
					"partition %d\n"), i + 1);
			else
				read_extended(i);
		}
	}

	for (i = 3; i < partitions; i++) {
		struct pte *pe = &ptes[i];

		if (!valid_part_table_flag(pe->sectorbuffer)) {
			fprintf(stderr,
				_("Warning: invalid flag 0x%04x of partition "
				"table %d will be corrected by w(rite)\n"),
				part_table_flag(pe->sectorbuffer), i + 1);
			pe->changed = 1;
		}
	}

	return 0;
}

/* read line; return 0 or first char */
int
read_line(void)
{
	static int got_eof = 0;

	line_ptr = line_buffer;
	if (!fgets(line_buffer, LINE_LENGTH, stdin)) {
		if (feof(stdin))
			got_eof++; 	/* user typed ^D ? */
		if (got_eof >= 3) {
			fflush(stdout);
			fprintf(stderr, _("\ngot EOF thrice - exiting..\n"));
			exit(1);
		}
		return 0;
	}
	while (*line_ptr && !isgraph(*line_ptr))
		line_ptr++;
	return *line_ptr;
}

char
read_char(char *mesg)
{
	do {
		fputs(mesg, stdout);
		fflush(stdout);
	} while (!read_line());
	return *line_ptr;
}

char
read_chars(char *mesg)
{
        fputs(mesg, stdout);
	fflush(stdout);
        if (!read_line()) {
		*line_ptr = '\n';
		line_ptr[1] = 0;
	}
	return *line_ptr;
}

int
read_hex(struct systypes *sys)
{
        int hex;

        while (1)
        {
           read_char(_("Hex code (type L to list codes): "));
           if (tolower(*line_ptr) == 'l')
               list_types(sys);
	   else if (isxdigit (*line_ptr))
	   {
	      hex = 0;
	      do
		 hex = hex << 4 | hex_val(*line_ptr++);
	      while (isxdigit(*line_ptr));
	      return hex;
	   }
        }
}

/*
 * Print the message MESG, then read an integer between LOW and HIGH (inclusive).
 * If the user hits Enter, DFLT is returned.
 * Answers like +10 are interpreted as offsets from BASE.
 *
 * There is no default if DFLT is not between LOW and HIGH.
 */
uint
read_int(uint low, uint dflt, uint high, uint base, char *mesg)
{
	uint i;
	int default_ok = 1;
	static char *ms = NULL;
	static int mslen = 0;

	if (!ms || strlen(mesg)+50 > mslen) {
		mslen = strlen(mesg)+100;
		if (!(ms = realloc(ms,mslen)))
			fatal(out_of_memory);
	}

	if (dflt < low || dflt > high)
		default_ok = 0;

	if (default_ok)
		sprintf(ms, _("%s (%d-%d, default %d): "), mesg, low, high, dflt);
	else
		sprintf(ms, "%s (%d-%d): ", mesg, low, high);

	while (1) {
		int use_default = default_ok;

		/* ask question and read answer */
		while (read_chars(ms) != '\n' && !isdigit(*line_ptr)
		       && *line_ptr != '-' && *line_ptr != '+')
			continue;

		if (*line_ptr == '+' || *line_ptr == '-') {
			i = atoi(line_ptr+1);
			if (*line_ptr == '-')
				i = -i;
 			while (isdigit(*++line_ptr))
				use_default = 0;
			switch (*line_ptr) {
				case 'c':
				case 'C':
					if (!display_in_cyl_units)
						i *= heads * sectors;
					break;
				case 'k':
				case 'K':
					i *= 2;
					i /= (sector_size / 512);
					i /= units_per_sector;
					break;
				case 'm':
				case 'M':
					i *= 2048;
					i /= (sector_size / 512);
					i /= units_per_sector;
					break;
				case 'g':
				case 'G':
					i *= 2048000;
					i /= (sector_size / 512);
					i /= units_per_sector;
					break;
				default:
					break;
			}
			i += base;
		} else {
			i = atoi(line_ptr);
			while (isdigit(*line_ptr)) {
				line_ptr++;
				use_default = 0;
			}
		}
		if (use_default)
			printf(_("Using default value %d\n"), i = dflt);
		if (i >= low && i <= high)
			break;
		else
			printf(_("Value out of range.\n"));
	}
	return i;
}

int
get_partition(int warn, int max) {
	struct pte *pe;
	int i;

	i = read_int(1, 0, max, 0, _("Partition number")) - 1;
	pe = &ptes[i];

	if (warn && (
	     (!sun_label && !sgi_label && !pe->part_table->sys_ind)
	     || (sun_label &&
		(!sunlabel->partitions[i].num_sectors ||
		 !sunlabel->infos[i].id))
	     || (sgi_label && (!sgi_get_num_sectors(i)))
	)) fprintf(stderr, _("Warning: partition %d has empty type\n"), i+1);
	return i;
}

char * const
str_units(int n) {	/* n==1: use singular */
	if (n == 1)
		return display_in_cyl_units ? _("cylinder") : _("sector");
	else
		return display_in_cyl_units ? _("cylinders") : _("sectors");
}

void change_units(void)
{
	display_in_cyl_units = !display_in_cyl_units;
	update_units();
	printf(_("Changing display/entry units to %s\n"),
		str_units(PLURAL));
}

static void
toggle_active(int i) {
	struct pte *pe = &ptes[i];
	struct partition *p = pe->part_table;

	if (IS_EXTENDED (p->sys_ind) && !p->boot_ind)
		fprintf(stderr,
			_("WARNING: Partition %d is an extended partition\n"),
			i + 1);
	p->boot_ind = (p->boot_ind ? 0 : ACTIVE_FLAG);
	pe->changed = 1;
}

static void
toggle_dos_compatibility_flag(void) {
	dos_compatible_flag = ~dos_compatible_flag;
	if (dos_compatible_flag) {
		sector_offset = sectors;
		printf(_("DOS Compatibility flag is set\n"));
	}
	else {
		sector_offset = 1;
		printf(_("DOS Compatibility flag is not set\n"));
	}
}

static void
delete_partition(int i) {
	struct pte *pe = &ptes[i];
	struct partition *p = pe->part_table;
	struct partition *q = pe->ext_pointer;

/* Note that for the fifth partition (i == 4) we don't actually
 * decrement partitions.
 */

	if (warn_geometry())
		return;
	pe->changed = 1;

	if (sun_label) {
		sun_delete_partition(i);
		return;
	}
	if (sgi_label) {
		sgi_delete_partition(i);
		return;
	}
	if (i < 4) {
		if (IS_EXTENDED (p->sys_ind) && i == ext_index) {
			partitions = 4;
			ptes[ext_index].ext_pointer = NULL;
			extended_offset = 0;
		}
		clear_partition(p);
	}
	else if (!q->sys_ind && i > 4) {
		--partitions;
		--i;
		clear_partition(ptes[i].ext_pointer);
		ptes[i].changed = 1;
	}
	else if (i > 3) {
		if (i > 4) {
			p = ptes[i-1].ext_pointer;
			p->boot_ind = 0;
			p->head = q->head;
			p->sector = q->sector;
			p->cyl = q->cyl;
			p->sys_ind = EXTENDED;
			p->end_head = q->end_head;
			p->end_sector = q->end_sector;
			p->end_cyl = q->end_cyl;
			set_start_sect(p, get_start_sect(q));
			set_nr_sects(p, get_nr_sects(q));
			ptes[i-1].changed = 1;
		} else {
			struct pte *pe = &ptes[5];

			if(pe->part_table) /* prevent SEGFAULT */
				set_start_sect(pe->part_table,
					       get_partition_start(pe) -
					       extended_offset);
			pe->offset = extended_offset;
			pe->changed = 1;
		}
		if (partitions > 5) {
			partitions--;
			while (i < partitions) {
				ptes[i] = ptes[i+1];
				i++;
			}
		} else
			clear_partition(ptes[i].part_table);
	}
}

static void
change_sysid(void) {
	char *temp;
	int i = get_partition(0, partitions), sys, origsys;
	struct partition *p = ptes[i].part_table;

	origsys = sys = get_sysid(i);

	if (!sys && !sgi_label)
                printf(_("Partition %d does not exist yet!\n"), i + 1);
        else while (1) {
		sys = read_hex (get_sys_types());

		if (!sys && !sgi_label) {
			printf(_("Type 0 means free space to many systems\n"
			       "(but not to Linux). Having partitions of\n"
			       "type 0 is probably unwise. You can delete\n"
			       "a partition using the `d' command.\n"));
			/* break; */
		}

		if (!sun_label && !sgi_label) {
			if (IS_EXTENDED (sys) != IS_EXTENDED (p->sys_ind)) {
				printf(_("You cannot change a partition into"
				       " an extended one or vice versa\n"
				       "Delete it first.\n"));
				break;
			}
		}

                if (sys < 256) {
			if (sun_label && i == 2 && sys != WHOLE_DISK)
				printf(_("Consider leaving partition 3 "
				       "as Whole disk (5),\n"
				       "as SunOS/Solaris expects it and "
				       "even Linux likes it.\n\n"));
			if (sgi_label && ((i == 10 && sys != ENTIRE_DISK)
					  || (i == 8 && sys != 0)))
				printf(_("Consider leaving partition 9 "
				       "as volume header (0),\nand "
				       "partition 11 as entire volume (6)"
				       "as IRIX expects it.\n\n"));
                        if (sys == origsys)
                            break;

			if (sun_label) {
				sun_change_sysid(i, sys);
			} else
			if (sgi_label) {
				sgi_change_sysid(i, sys);
			} else
				p->sys_ind = sys;
                        printf (_("Changed system type of partition %d "
                                "to %x (%s)\n"), i + 1, sys,
                                (temp = partition_type(sys)) ? temp :
                                _("Unknown"));
                        ptes[i].changed = 1;
                        break;
                }
        }
}

/* check_consistency() and long2chs() added Sat Mar 6 12:28:16 1993,
 * faith@cs.unc.edu, based on code fragments from pfdisk by Gordon W. Ross,
 * Jan.  1990 (version 1.2.1 by Gordon W. Ross Aug. 1990; Modified by S.
 * Lubkin Oct.  1991). */

static void long2chs(ulong ls, uint *c, uint *h, uint *s) {
	int	spc = heads * sectors;

	*c = ls / spc;
	ls = ls % spc;
	*h = ls / sectors;
	*s = ls % sectors + 1;	/* sectors count from 1 */
}

static void check_consistency(struct partition *p, int partition) {
	uint	pbc, pbh, pbs;		/* physical beginning c, h, s */
	uint	pec, peh, pes;		/* physical ending c, h, s */
	uint	lbc, lbh, lbs;		/* logical beginning c, h, s */
	uint	lec, leh, les;		/* logical ending c, h, s */

	if (!heads || !sectors || (partition >= 4))
		return;		/* do not check extended partitions */

/* physical beginning c, h, s */
	pbc = (p->cyl & 0xff) | ((p->sector << 2) & 0x300);
	pbh = p->head;
	pbs = p->sector & 0x3f;

/* physical ending c, h, s */
	pec = (p->end_cyl & 0xff) | ((p->end_sector << 2) & 0x300);
	peh = p->end_head;
	pes = p->end_sector & 0x3f;

/* compute logical beginning (c, h, s) */
	long2chs(get_start_sect(p), &lbc, &lbh, &lbs);

/* compute logical ending (c, h, s) */
	long2chs(get_start_sect(p) + get_nr_sects(p) - 1, &lec, &leh, &les);

/* Same physical / logical beginning? */
	if (cylinders <= 1024 && (pbc != lbc || pbh != lbh || pbs != lbs)) {
		printf(_("Partition %d has different physical/logical "
			"beginnings (non-Linux?):\n"), partition + 1);
		printf(_("     phys=(%d, %d, %d) "), pbc, pbh, pbs);
		printf(_("logical=(%d, %d, %d)\n"),lbc, lbh, lbs);
	}

/* Same physical / logical ending? */
	if (cylinders <= 1024 && (pec != lec || peh != leh || pes != les)) {
		printf(_("Partition %d has different physical/logical "
			"endings:\n"), partition + 1);
		printf(_("     phys=(%d, %d, %d) "), pec, peh, pes);
		printf(_("logical=(%d, %d, %d)\n"),lec, leh, les);
	}

#if 0
/* Beginning on cylinder boundary? */
	if (pbh != !pbc || pbs != 1) {
		printf(_("Partition %i does not start on cylinder "
			"boundary:\n"), partition + 1);
		printf(_("     phys=(%d, %d, %d) "), pbc, pbh, pbs);
		printf(_("should be (%d, %d, 1)\n"), pbc, !pbc);
	}
#endif

/* Ending on cylinder boundary? */
	if (peh != (heads - 1) || pes != sectors) {
		printf(_("Partition %i does not end on cylinder boundary:\n"),
			partition + 1);
		printf(_("     phys=(%d, %d, %d) "), pec, peh, pes);
		printf(_("should be (%d, %d, %d)\n"),
		pec, heads - 1, sectors);
	}
}

static void
list_disk_geometry(void) {
	printf(_("\nDisk %s: %d heads, %d sectors, %d cylinders\nUnits = "
	       "%s of %d * %d bytes\n\n"), disk_device, heads, sectors,
	       cylinders, str_units(PLURAL), units_per_sector, sector_size);
}

/*
 * Check whether partition entries are ordered by their starting positions.
 * Return 0 if OK. Return i if partition i should have been earlier.
 * Two separate checks: primary and logical partitions.
 */
static int
wrong_p_order(int *prev) {
	struct pte *pe;
	struct partition *p;
	uint last_p_start_pos = 0, p_start_pos;
	int i, last_i = 0;

	for (i = 0 ; i < partitions; i++) {
		if (i == 4) {
			last_i = 4;
			last_p_start_pos = 0;
		}
		pe = &ptes[i];
		if ((p = pe->part_table)->sys_ind) {
			p_start_pos = get_partition_start(pe);

			if (last_p_start_pos > p_start_pos) {
				if (prev)
					*prev = last_i;
				return i;
			}

			last_p_start_pos = p_start_pos;
			last_i = i;
		}
	}
	return 0;
}

static void
fix_partition_table_order(void) {
	struct pte *pei, *pek, ptebuf;
	int i,k;

	if(!wrong_p_order(NULL)) {
		printf(_("Nothing to do. Ordering is correct already.\n\n"));
		return;
	}

	while ((i = wrong_p_order(&k)) != 0) {
		/* partition i should have come earlier, move it */
		pei = &ptes[i];
		pek = &ptes[k];

		if (i < 4) {
			/* We have to move data in the MBR */
			struct partition *pi, *pk, *pe, pbuf;

			pe = pei->ext_pointer;
			pei->ext_pointer = pek->ext_pointer;
			pek->ext_pointer = pe;

			pi = pei->part_table;
			pk = pek->part_table;

			memmove(&pbuf, pi, sizeof(struct partition));
			memmove(pi, pk, sizeof(struct partition));
			memmove(pk, &pbuf, sizeof(struct partition));
		} else {
			/* Only change is change in numbering */
			ptebuf = *pei;
			*pei = *pek;
			*pek = ptebuf;
		}
		pei->changed = pek->changed = 1;

	}
}

static void
list_table(int xtra) {
	struct partition *p;
	char *type;
	int i, w;

	if (sun_label) {
		sun_list_table(xtra);
		return;
	}

	if (sgi_label) {
		sgi_list_table(xtra);
		return;
	}

	list_disk_geometry();

	if (osf_label) {
		xbsd_print_disklabel(xtra);
		return;
	}

	/* Heuristic: we list partition 3 of /dev/foo as /dev/foo3,
	   but if the device name ends in a digit, say /dev/foo1,
	   then the partition is called /dev/foo1p3. */
	w = strlen(disk_device);
	if (w && isdigit(disk_device[w-1]))
		w++;
	if (w < 5)
		w = 5;

	printf(_("%*s Boot    Start       End    Blocks   Id  System\n"),
	       w+1, _("Device"));

	for (i = 0 ; i < partitions; i++) {
		struct pte *pe = &ptes[i];

		p = pe->part_table;
		if (p->sys_ind) {
			unsigned int psects = get_nr_sects(p);
			unsigned int pblocks = psects;
			unsigned int podd = 0;

			if (sector_size < 1024) {
				pblocks /= (1024 / sector_size);
				podd = psects % (1024 / sector_size);
			}
			if (sector_size > 1024)
				pblocks *= (sector_size / 1024);
                        printf(
			    "%s  %c %9ld %9ld %9ld%c  %2x  %s\n",
			partname(disk_device, i+1, w+2),
/* boot flag */		!p->boot_ind ? ' ' : p->boot_ind == ACTIVE_FLAG
			? '*' : '?',
/* start */		(long) cround(get_partition_start(pe)),
/* end */		(long) cround(get_partition_start(pe) + psects
				- (psects ? 1 : 0)),
/* odd flag on end */	(long) pblocks, podd ? '+' : ' ',
/* type id */		p->sys_ind,
/* type name */		(type = partition_type(p->sys_ind)) ?
			type : _("Unknown"));
			check_consistency(p, i);
		}
	}
	
	/* Is partition table in disk order? It need not be, but... */
	/* partition table entries are not checked for correct order if this
	   is a sgi, sun or aix labeled disk... */
	if (dos_label && wrong_p_order(NULL)) {
		printf(_("\nPartition table entries are not in disk order\n"));
	}
}

static void
x_list_table(int extend) {
	struct pte *pe;
	struct partition *p;
	int i;

	printf(_("\nDisk %s: %d heads, %d sectors, %d cylinders\n\n"),
		disk_device, heads, sectors, cylinders);
        printf(_("Nr AF  Hd Sec  Cyl  Hd Sec  Cyl    Start     Size ID\n"));
	for (i = 0 ; i < partitions; i++) {
		pe = &ptes[i];
		p = (extend ? pe->ext_pointer : pe->part_table);
		if (p != NULL) {
                        printf("%2d %02x%4d%4d%5d%4d%4d%5d%9d%9d %02x\n",
				i + 1, p->boot_ind, p->head,
				sector(p->sector),
				cylinder(p->sector, p->cyl), p->end_head,
				sector(p->end_sector),
				cylinder(p->end_sector, p->end_cyl),
				get_start_sect(p), get_nr_sects(p), p->sys_ind);
			if (p->sys_ind)
				check_consistency(p, i);
		}
	}
}

static void
fill_bounds(uint *first, uint *last) {
	int i;
	struct pte *pe = &ptes[0];
	struct partition *p;

	for (i = 0; i < partitions; pe++,i++) {
		p = pe->part_table;
		if (!p->sys_ind || IS_EXTENDED (p->sys_ind)) {
			first[i] = 0xffffffff;
			last[i] = 0;
		} else {
			first[i] = get_partition_start(pe);
			last[i] = first[i] + get_nr_sects(p) - 1;
		}
	}
}

static void
check(int n, uint h, uint s, uint c, uint start) {
	uint total, real_s, real_c;

	real_s = sector(s) - 1;
	real_c = cylinder(s, c);
	total = (real_c * sectors + real_s) * heads + h;
	if (!total)
		fprintf(stderr, _("Warning: partition %d contains sector 0\n"), n);
	if (h >= heads)
		fprintf(stderr,
			_("Partition %d: head %d greater than maximum %d\n"),
			n, h + 1, heads);
	if (real_s >= sectors)
		fprintf(stderr, _("Partition %d: sector %d greater than "
			"maximum %d\n"), n, s, sectors);
	if (real_c >= cylinders)
		fprintf(stderr, _("Partitions %d: cylinder %d greater than "
			"maximum %d\n"), n, real_c + 1, cylinders);
	if (cylinders <= 1024 && start != total)
		fprintf(stderr,
			_("Partition %d: previous sectors %d disagrees with "
			"total %d\n"), n, start, total);
}

static void
verify(void) {
	int i, j;
	uint total = 1;
	uint first[partitions], last[partitions];
	struct partition *p = part_table[0];

	if (warn_geometry())
		return;

	if (sun_label) {
		verify_sun();
		return;
	}

	if (sgi_label) {
		verify_sgi(1);
		return;
	}

	fill_bounds(first, last);
	for (i = 0; i < partitions; i++) {
		struct pte *pe = &ptes[i];

		p = pe->part_table;
		if (p->sys_ind && !IS_EXTENDED (p->sys_ind)) {
			check_consistency(p, i);
			if (get_partition_start(pe) < first[i])
				printf(_("Warning: bad start-of-data in "
					"partition %d\n"), i + 1);
			check(i + 1, p->end_head, p->end_sector, p->end_cyl,
				last[i]);
			total += last[i] + 1 - first[i];
			for (j = 0; j < i; j++)
			if ((first[i] >= first[j] && first[i] <= last[j])
			 || ((last[i] <= last[j] && last[i] >= first[j]))) {
				printf(_("Warning: partition %d overlaps "
					"partition %d.\n"), j + 1, i + 1);
				total += first[i] >= first[j] ?
					first[i] : first[j];
				total -= last[i] <= last[j] ?
					last[i] : last[j];
			}
		}
	}

	if (extended_offset) {
		struct pte *pex = &ptes[ext_index];
		uint e_last = get_start_sect(pex->part_table) +
			get_nr_sects(pex->part_table) - 1;

		for (i = 4; i < partitions; i++) {
			total++;
			p = ptes[i].part_table;
			if (!p->sys_ind) {
				if (i != 4 || i + 1 < partitions)
					printf(_("Warning: partition %d "
						"is empty\n"), i + 1);
			}
			else if (first[i] < extended_offset ||
					last[i] > e_last)
				printf(_("Logical partition %d not entirely in "
					"partition %d\n"), i + 1, ext_index + 1);
		}
	}

	if (total > heads * sectors * cylinders)
		printf(_("Total allocated sectors %d greater than the maximum "
			"%d\n"), total, heads * sectors * cylinders);
	else if ((total = heads * sectors * cylinders - total) != 0)
		printf(_("%d unallocated sectors\n"), total);
}

static void
add_partition(int n, int sys) {
	char mesg[256];		/* 48 does not suffice in Japanese */
	int i, read = 0;
	struct partition *p = ptes[n].part_table;
	struct partition *q = ptes[ext_index].part_table;
	uint start, stop = 0, limit, temp,
		first[partitions], last[partitions];

	if (p && p->sys_ind) {
		printf(_("Partition %d is already defined.  Delete "
			"it before re-adding it.\n"), n + 1);
		return;
	}
	fill_bounds(first, last);
	if (n < 4) {
		start = sector_offset;
		limit = heads * sectors * cylinders - 1;
		if (extended_offset) {
			first[ext_index] = extended_offset;
			last[ext_index] = get_start_sect(q) +
				get_nr_sects(q) - 1;
		}
	} else {
		start = extended_offset + sector_offset;
		limit = get_start_sect(q) + get_nr_sects(q) - 1;
	}
	if (display_in_cyl_units)
		for (i = 0; i < partitions; i++)
			first[i] = (cround(first[i]) - 1) * units_per_sector;

	sprintf(mesg, _("First %s"), str_units(SINGULAR));
	do {
		temp = start;
		for (i = 0; i < partitions; i++) {
			int lastplusoff;

			if (start == ptes[i].offset)
				start += sector_offset;
			lastplusoff = last[i] + ((n<4) ? 0 : sector_offset);
			if (start >= first[i] && start <= lastplusoff)
				start = lastplusoff + 1;
		}
		if (start > limit)
			break;
		if (start >= temp+units_per_sector && read) {
			printf(_("Sector %d is already allocated\n"), temp);
			temp = start;
			read = 0;
		}
		if (!read && start == temp) {
			uint i;
			i = start;
			start = read_int(cround(i), cround(i), cround(limit),
					 0, mesg);
			if (display_in_cyl_units) {
				start = (start - 1) * units_per_sector;
				if (start < i) start = i;
			}
			read = 1;
		}
	} while (start != temp || !read);
	if (n > 4) {			/* NOT for fifth partition */
		struct pte *pe = &ptes[n];

		pe->offset = start - sector_offset;
		if (pe->offset == extended_offset) { /* must be corrected */
		     pe->offset++;
		     if (sector_offset == 1)
			  start++;
		}
	}

	for (i = 0; i < partitions; i++) {
		struct pte *pe = &ptes[i];

		if (start < pe->offset && limit >= pe->offset)
			limit = pe->offset - 1;
		if (start < first[i] && limit >= first[i])
			limit = first[i] - 1;
	}
	if (start > limit) {
		printf(_("No free sectors available\n"));
		if (n > 4)
			partitions--;
		return;
	}
	if (cround(start) == cround(limit)) {
		stop = limit;
	} else {
		sprintf(mesg, _("Last %s or +size or +sizeM or +sizeK"),
			str_units(SINGULAR));
		stop = read_int(cround(start), cround(limit), cround(limit),
				cround(start), mesg);
		if (display_in_cyl_units) {
			stop = stop * units_per_sector - 1;
			if (stop >limit)
				stop = limit;
		}
	}

	set_partition(n, p, start, stop, sys, ptes[n].offset);

	if (IS_EXTENDED (sys)) {
		struct pte *pe4 = &ptes[4];
		struct pte *pen = &ptes[n];

		ext_index = n;
		pen->ext_pointer = p;
		pe4->offset = extended_offset = start;
		if (!(pe4->sectorbuffer = calloc(1, sector_size)))
			fatal(out_of_memory);
		pe4->part_table = pt_offset(pe4->sectorbuffer, 0);
		pe4->ext_pointer = pe4->part_table + 1;
		pe4->changed = 1;
		partitions = 5;
	} else {
		if (n > 4)
			set_partition(n - 1, ptes[n-1].ext_pointer,
				ptes[n].offset, stop, EXTENDED,
				extended_offset);
#if 0
		if ((limit = get_nr_sects(p)) & 1)
			printf(_("Warning: partition %d has an odd "
				"number of sectors.\n"), n + 1);
#endif
	}
}

static void
add_logical(void) {
	if (partitions > 5 || ptes[4].part_table->sys_ind) {
		struct pte *pe = &ptes[partitions];

		if (!(pe->sectorbuffer = calloc(1, sector_size)))
			fatal(out_of_memory);
		pe->part_table = pt_offset(pe->sectorbuffer, 0);
		pe->ext_pointer = pe->part_table + 1;
		pe->offset = 0;
		pe->changed = 1;
		partitions++;
	}
	add_partition(partitions - 1, LINUX_NATIVE);
}

static void
new_partition(void) {
	int i, free_primary = 0;

	if (warn_geometry())
		return;

	if (sun_label) {
		add_sun_partition(get_partition(0, partitions), LINUX_NATIVE);
		return;
	}

	if (sgi_label) {
		sgi_add_partition(get_partition(0, partitions), LINUX_NATIVE);
		return;
	}

	if (partitions >= MAXIMUM_PARTS) {
		printf(_("The maximum number of partitions has been created\n"));
		return;
	}

	for (i = 0; i < 4; i++)
		free_primary += !ptes[i].part_table->sys_ind;
	if (!free_primary) {
		if (extended_offset)
			add_logical();
		else
			printf(_("You must delete some partition and add "
				"an extended partition first\n"));
	} else {
		char c, line[LINE_LENGTH];
		sprintf(line, _("Command action\n   %s\n   p   primary "
			"partition (1-4)\n"), extended_offset ?
			_("l   logical (5 or over)") : _("e   extended"));
		while (1)
			if ((c = read_char(line)) == 'p' || c == 'P') {
				add_partition(get_partition(0, 4),
					LINUX_NATIVE);
				return;
			}
			else if ((c == 'L' || c == 'l') && extended_offset) {
				add_logical();
				return;
			}
			else if ((c == 'E' || c == 'e') && !extended_offset) {
				add_partition(get_partition(0, 4),
					EXTENDED);
				return;
			}
			else
				printf(_("Invalid partition number "
				       "for type `%c'\n"), c);
		
	}
}

static void
write_table(void) {
	int i;

	if (dos_label) {
		for (i=0; i<3; i++)
			if(ptes[i].changed)
				ptes[3].changed = 1;
		for (i = 3; i < partitions; i++) {
			struct pte *pe = &ptes[i];

			if (pe->changed) {
				write_part_table_flag(pe->sectorbuffer);
				write_sector(fd, pe->offset, pe->sectorbuffer);
			}
		}
	} else if (sgi_label) {
		/* no test on change? the printf below might be mistaken */
		sgi_write_table();
	} else if (sun_label) {
		int needw = 0;

		for (i=0; i<8; i++)
			if(ptes[i].changed)
				needw = 1;
		if (needw)
			sun_write_table();
	}

	printf(_("The partition table has been altered!\n\n"));
	reread_partition_table(1);
}

void
reread_partition_table(int leave) {
	int error = 0;
	int i;

	printf(_("Calling ioctl() to re-read partition table.\n"));
	sync();
	sleep(2);
	if ((i = ioctl(fd, BLKRRPART)) != 0) {
                error = errno;
        } else {
                /* some kernel versions (1.2.x) seem to have trouble
                   rereading the partition table, but if asked to do it
		   twice, the second time works. - biro@yggdrasil.com */
                sync();
                sleep(2);
                if((i = ioctl(fd, BLKRRPART)) != 0)
                        error = errno;
        }

	if (i < 0)
		printf(_("Re-read table failed with error %d: %s.\nReboot your "
			"system to ensure the partition table is updated.\n"),
			error, strerror(error));

	if (!sun_label && !sgi_label)
	    printf(
		_("\nWARNING: If you have created or modified any DOS 6.x\n"
		"partitions, please see the fdisk manual page for additional\n"
		"information.\n"));

	if (leave) {
		close(fd);

		printf(_("Syncing disks.\n"));
		sync();
		sleep(4);		/* for sync() */
		exit(!!i);
	}
}

#define MAX_PER_LINE	16
static void
print_buffer(char pbuffer[]) {
	int	i,
		l;

	for (i = 0, l = 0; i < sector_size; i++, l++) {
		if (l == 0)
			printf("0x%03X:", i);
		printf(" %02X", (unsigned char) pbuffer[i]);
		if (l == MAX_PER_LINE - 1) {
			printf("\n");
			l = -1;
		}
	}
	if (l > 0)
		printf("\n");
	printf("\n");
}

static void
print_raw(void) {
	int i;

	printf(_("Device: %s\n"), disk_device);
	if (sun_label || sgi_label)
		print_buffer(MBRbuffer);
	else for (i = 3; i < partitions; i++)
		print_buffer(ptes[i].sectorbuffer);
}

static void
move_begin(int i) {
	struct pte *pe = &ptes[i];
	struct partition *p = pe->part_table;
	uint new, first;

	if (warn_geometry())
		return;
	if (!p->sys_ind || !get_nr_sects(p) || IS_EXTENDED (p->sys_ind)) {
		printf(_("Partition %d has no data area\n"), i + 1);
		return;
	}
	first = get_partition_start(pe);
	new = read_int(first, first, first + get_nr_sects(p) - 1, first,
		       _("New beginning of data")) - pe->offset;

	if (new != get_nr_sects(p)) {
		first = get_nr_sects(p) + get_start_sect(p) - new;
		set_nr_sects(p, first);
		set_start_sect(p, new);
		pe->changed = 1;
	}
}

static void
xselect(void) {
	char c;

	while(1) {
		putchar('\n');
		c = read_char(_("Expert command (m for help): "));
		c = tolower(c);
		switch (c) {
		case 'a':
			if (sun_label)
				sun_set_alt_cyl();
			break;
		case 'b':
			if (dos_label)
				move_begin(get_partition(0, partitions));
			break;
		case 'c':
			user_cylinders = cylinders =
				read_int(1, cylinders, 131071, 0,
					 _("Number of cylinders"));
			if (sun_label)
				sun_set_ncyl(cylinders);
			if (dos_label)
				warn_cylinders();
			break;
		case 'd':
			print_raw();
			break;
		case 'e':
			if (sgi_label)
				sgi_set_xcyl();
			else if (sun_label)
				sun_set_xcyl();
			else if (dos_label)
				x_list_table(1);
			break;
		case 'f':
			if(dos_label)
				fix_partition_table_order();
			break;
		case 'g':
			create_sgilabel();
			break;
		case 'h':
			user_heads = heads = read_int(1, heads, 256, 0,
					 _("Number of heads"));
			update_units();
			break;
		case 'i':
			if (sun_label)
				sun_set_ilfact();
			break;
		case 'o':
			if (sun_label)
				sun_set_rspeed();
			break;
		case 'p':
			if (sun_label)
				list_table(1);
			else
				x_list_table(0);
			break;
		case 'q':
			close(fd);
			printf("\n");
			exit(0);
		case 'r':
			return;
		case 's':
			user_sectors = sectors = read_int(1, sectors, 63, 0,
					   _("Number of sectors"));
			if (dos_compatible_flag) {
				sector_offset = sectors;
				fprintf(stderr, _("Warning: setting "
					"sector offset for DOS "
					"compatiblity\n"));
			}
			update_units();
			break;
		case 'v':
			verify();
			break;
		case 'w':
			write_table(); 	/* does not return */
			break;
		case 'y':
			if (sun_label)
				sun_set_pcylcount();
			break;
		default:
			xmenu();
		}
	}
}

static int
is_ide_cdrom(char *device) {
	/* No device was given explicitly, and we are trying some
       likely things.  But opening /dev/hdc may produce errors like
           "hdc: tray open or drive not ready"
       if it happens to be a CD-ROM drive. It even happens that
       the process hangs on the attempt to read a music CD.
       So try to be careful. This only works since 2.1.73. */

    FILE *procf;
    char buf[100];
    struct stat statbuf;

    if (strncmp("/dev/hd", device, 7))
	return 0;
    sprintf(buf, "/proc/ide/%s/media", device+5);
    procf = fopen(buf, "r");
    if (procf != NULL && fgets(buf, sizeof(buf), procf))
        return  !strncmp(buf, "cdrom", 5);

    /* Now when this proc file does not exist, skip the
       device when it is read-only. */
    if (stat(device, &statbuf) == 0)
        return (statbuf.st_mode & 0222) == 0;

    return 0;
}

static void
try(char *device, int user_specified) {
	disk_device = device;
	if (!setjmp(listingbuf)) {
		if (!user_specified)
			if (is_ide_cdrom(device))
				return;
		if ((fd = open(disk_device, type_open)) >= 0) {
			if (get_boot(try_only) < 0) {
				list_disk_geometry();
				if (btrydev(device) < 0)
					fprintf(stderr,
						_("Disk %s doesn't contain a valid "
						"partition table\n"), device);
				close(fd);
			} else {
				close(fd);
				list_table(0);
				if (!sun_label && partitions > 4)
					delete_partition(ext_index);
			}
		} else {
				/* Ignore other errors, since we try IDE
				   and SCSI hard disks which may not be
				   installed on the system. */
			if(errno == EACCES) {
			    fprintf(stderr, _("Cannot open %s\n"), device);
			    return;
			}
		}
	}
}

/* for fdisk -l: try all things in /proc/partitions
   that look like a partition name (do not end in a digit) */
static void
tryprocpt(void) {
	FILE *procpt;
	char line[100], ptname[100], devname[120], *s;
	int ma, mi, sz;

	procpt = fopen(PROC_PARTITIONS, "r");
	if (procpt == NULL) {
		fprintf(stderr, _("cannot open %s\n"), PROC_PARTITIONS);
		return;
	}

	while (fgets(line, sizeof(line), procpt)) {
		if (sscanf (line, " %d %d %d %[^\n]\n",
			    &ma, &mi, &sz, ptname) != 4)
			continue;
		for(s = ptname; *s; s++);
		if (isdigit(s[-1]))
			continue;
		sprintf(devname, "/dev/%s", ptname);
		try(devname, 1);
	}
}

static void
dummy(int *kk) {}

static void
unknown_command(int c) {
	printf(_("%c: unknown command\n"), c);
}

int
main(int argc, char **argv) {
	int j, c;
	int optl = 0, opts = 0;
	int user_set_sector_size = 0;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	/*
	 * Calls:
	 *  fdisk -v
	 *  fdisk -l [-b sectorsize] [-u] device ...
	 *  fdisk -s [partition] ...
	 *  fdisk [-b sectorsize] [-u] device
	 */
	while ((c = getopt(argc, argv, "b:lsuvV")) != EOF) {
		switch (c) {
		case 'b':
			/* ugly: this sector size is really per device,
			   so cannot be combined with multiple disks */
			sector_size = atoi(optarg);
			if (sector_size != 512 && sector_size != 1024 &&
			    sector_size != 2048)
				fatal(usage);
			sector_offset = 2;
			user_set_sector_size = 1;
			break;
		case 'l':
			optl = 1;
			break;
		case 's':
			opts = 1;
			break;
		case 'u':
			display_in_cyl_units = 0;
			break;
		case 'V':
		case 'v':
			printf("fdisk v" UTIL_LINUX_VERSION "\n");
			exit(0);
		default:
			fatal(usage);
		}
	}

#if 0
	printf(_("This kernel finds the sector size itself - -b option ignored\n"));
#else
	if (user_set_sector_size && argc-optind != 1)
		printf(_("Warning: the -b (set sector size) option should"
			 " be used with one specified device\n"));
#endif

	if (optl) {
		nowarn = 1;
		type_open = O_RDONLY;
		if (argc > optind) {
			int k;
			/* avoid gcc warning:
			   variable `k' might be clobbered by `longjmp' */
			dummy(&k);
			listing = 1;
			for(k=optind; k<argc; k++)
				try(argv[k], 1);
		} else {
			/* we no longer have default device names */
			/* but, we can use /proc/partitions instead */
			tryprocpt();
		}
		exit(0);
	}

	if (opts) {
		long size;

		nowarn = 1;
		type_open = O_RDONLY;

		opts = argc - optind;
		if (opts <= 0)
			fatal(usage);

		for (j = optind; j < argc; j++) {
			disk_device = argv[j];
			if ((fd = open(disk_device, type_open)) < 0)
				fatal(unable_to_open);
			if (ioctl(fd, BLKGETSIZE, &size))
				fatal(ioctl_error);
			close(fd);
			if (opts == 1)
				printf("%ld\n", size/2);
			else
				printf("%s: %ld\n", argv[j], size/2);
		}
		exit(0);
	}

	if (argc-optind == 1)
		disk_device = argv[optind];
	else if (argc-optind != 0)
		fatal(usage);
	else
		fatal(usage2);

	get_boot(fdisk);

#ifdef __alpha__
	/* On alpha, if we detect a disklabel, go directly to
	   disklabel mode (typically you'll be switching from DOS
	   partition tables to disklabels, not the other way around)
	   - dhuggins@linuxcare.com */
	if (osf_label) {
		printf(_("Detected an OSF/1 disklabel on %s, entering disklabel mode.\n"
			 "To return to DOS partition table mode, use the 'r' command.\n"),
		       disk_device);
		bselect();
	}
#endif

	while (1) {
		putchar('\n');
		c = read_char(_("Command (m for help): "));
		c = tolower(c);
		switch (c) {
		case 'a':
			if (dos_label)
				toggle_active(get_partition(1, partitions));
			else if (sun_label)
				toggle_sunflags(get_partition(1, partitions),
						0x01);
			else if (sgi_label)
				sgi_set_bootpartition(
					get_partition(1, partitions));
			else
				unknown_command(c);
			break;
		case 'b':
			if (sgi_label) {
				printf(_("\nThe current boot file is: %s\n"),
				       sgi_get_bootfile());
				if (read_chars(_("Please enter the name of the "
					       "new boot file: ")) == '\n')
					printf(_("Boot file unchanged\n"));
				else
					sgi_set_bootfile(line_ptr);
			} else
				bselect();
			break;
		case 'c':
			if (dos_label)
				toggle_dos_compatibility_flag();
			else if (sun_label)
				toggle_sunflags(get_partition(1, partitions),
						0x10);
			else if (sgi_label)
				sgi_set_swappartition(
						get_partition(1, partitions));
			else
				unknown_command(c);
			break;
		case 'd':
			delete_partition(
				get_partition(1, partitions));
			break;
		case 'i':
			if (sgi_label)
				create_sgiinfo();
			else
				unknown_command(c);
		case 'l':
			list_types(get_sys_types());
			break;
		case 'm':
			menu();
			break;
		case 'n':
			new_partition();
			break;
		case 'o':
			create_doslabel();
			break;
		case 'p':
			list_table(0);
			break;
		case 'q':
			close(fd);
			printf("\n");
			exit(0);
		case 's':
			create_sunlabel();
			break;
		case 't':
			change_sysid();
			break;
		case 'u':
			change_units();
			break;
		case 'v':
			verify();
			break;
		case 'w':
			write_table(); 		/* does not return */
			break;
		case 'x':
			if(sgi_label) {
				fprintf(stderr,
					_("\n\tSorry, no experts menu for SGI "
					"partition tables available.\n\n"));
			} else
				xselect();
			break;
		default:
			unknown_command(c);
			menu();
		}
	}
	return 0;
}
