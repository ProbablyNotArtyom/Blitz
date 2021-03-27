/*
 *
 * fdisksgilabel.c
 *
 * Copyright (C) Andreas Neuper, Sep 1998.
 *	This file may be modified and redistributed under
 *	the terms of the GNU Public License.
 *
 * Sat Mar 20 EST 1999 Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *	Internationalization
 */
#include <stdio.h>              /* stderr */
#include <stdlib.h>             /* uint */
#include <string.h>             /* strstr */
#include <unistd.h>             /* write */
#include <sys/ioctl.h>          /* ioctl */
#include <sys/stat.h>           /* stat */
#include <assert.h>             /* assert */

#include <endian.h>
#include "nls.h"
#include <linux/major.h>        /* FLOPPY_MAJOR */
#include <linux/hdreg.h>        /* HDIO_GETGEO */

#include "common.h"
#include "fdisk.h"
#include "fdisksgilabel.h"

static	int     other_endian = 0;
static	int     debug = 0;
static  short volumes=1;

/*
 * only dealing with free blocks here
 */

typedef struct { int first; int last; } freeblocks;
static freeblocks freelist[17]; /* 16 partitions can produce 17 vacant slots */

static void
setfreelist( int i, int f, int l ) {
	freelist[i].first = f;
	freelist[i].last = l;
}

static void
add2freelist( int f, int l ) {
	int i = 0;
	for( ; i<17 ; i++ ) {
		if(freelist[i].last==0) break;
	}
	setfreelist( i, f, l );
}

static void
clearfreelist(void) {
	int i = 0;
	for( ; i<17 ; i++ ) {
		setfreelist( i, 0, 0 );
	}
}

static int
isinfreelist( int b ) {
	int i = 0;
	for( ; i<17 ; i++ ) {
		if (freelist[i].first <= b && freelist[i].last >= b) {
			return freelist[i].last;
		}
	}
	return 0;
}
	/* return last vacant block of this stride (never 0). */
	/* the '>=' is not quite correct, but simplifies the code */
/*
 * end of free blocks section
 */
struct systypes sgi_sys_types[] = {
    {SGI_VOLHDR,  N_("SGI volhdr")},
    {0x01,	  N_("SGI trkrepl")},
    {0x02,	  N_("SGI secrepl")},
    {SGI_SWAP,	  N_("SGI raw")},
    {0x04,	  N_("SGI bsd")},
    {0x05,	  N_("SGI sysv")},
    {ENTIRE_DISK, N_("SGI volume")},
    {SGI_EFS,	  N_("SGI efs")},
    {0x08,	  N_("SGI lvol")},
    {0x09,	  N_("SGI rlvol")},
    {0x0a,	  N_("SGI xfs")},
    {0x0b,	  N_("SGI xlvol")},
    {0x0c,	  N_("SGI rxlvol")},
    {LINUX_SWAP,  N_("Linux swap")},
    {LINUX_NATIVE,N_("Linux native")},
    {0x8e,	  N_("Linux LVM")},
    {0, NULL }
};

static inline unsigned short
__swap16(unsigned short x) {
        return (((__u16)(x) & 0xFF) << 8) | (((__u16)(x) & 0xFF00) >> 8);
}

static inline __u32
__swap32(__u32 x) {
        return (((__u32)(x) & 0xFF) << 24) | (((__u32)(x) & 0xFF00) << 8) | (((__u32)(x) & 0xFF0000) >> 8) | (((__u32)(x) & 0xFF000000) >> 24);
}

static int
sgi_get_nsect(void) {
    return SSWAP16(sgilabel->devparam.nsect);
}

static int
sgi_get_ntrks(void) {
    return SSWAP16(sgilabel->devparam.ntrks);
}

#if 0
static int
sgi_get_head_vol0(void) {
    return SSWAP16(sgilabel->devparam.head_vol0);
}

static int
sgi_get_bytes(void) {
    return SSWAP16(sgilabel->devparam.bytes);
}
#endif

static int
sgi_get_pcylcount(void) {
    return SSWAP16(sgilabel->devparam.pcylcount);
}

void
sgi_nolabel()
{
    sgilabel->magic = 0;
    sgi_label = 0;
    partitions = 4;
}

static unsigned int
two_s_complement_32bit_sum(unsigned int* base, int size /* in bytes */ ) {
    int i=0;
    unsigned int sum=0;

    size = size / sizeof( unsigned int );
    for( i=0; i<size; i++ )
	sum = sum - SSWAP32(base[i]);
    return sum;
}

int
check_sgi_label() {
    if (sizeof(sgilabel) > 512) {
	    fprintf(stderr,
		    _("According to MIPS Computer Systems, Inc the "
		    "Label must not contain more than 512 bytes\n"));
	    exit(1);
    }

    if (sgilabel->magic != SGI_LABEL_MAGIC &&
	sgilabel->magic != SGI_LABEL_MAGIC_SWAPPED) {
	sgi_label = 0;
	other_endian = 0;
	return 0;
    }

    other_endian = (sgilabel->magic == SGI_LABEL_MAGIC_SWAPPED);
    /*
     * test for correct checksum
     */
    if( two_s_complement_32bit_sum( (unsigned int*)sgilabel,
			sizeof(*sgilabel) ) )
    {
	fprintf( stderr, _("Detected sgi disklabel with wrong checksum.\n"));
    } else
    {
	heads = sgi_get_ntrks();
	cylinders = sgi_get_pcylcount();
	sectors = sgi_get_nsect();
    }
    update_units();
    sgi_label = 1;
    partitions= 16;
    volumes = 15;
    return 1;
}

void
sgi_list_table( int xtra ) {
    int i, w;
    char *type;

    w = strlen( disk_device );

    if( xtra )
    {
	printf(_("\nDisk %s (SGI disk label): %d heads, %d sectors\n"
	       "%d cylinders, %d physical cylinders\n"
	       "%d extra sects/cyl, interleave %d:1\n"
	       "%s\n"
	       "Units = %s of %d * 512 bytes\n\n"),
	       disk_device, heads, sectors, cylinders,
	       SSWAP16(sgiparam.pcylcount),
	       SSWAP16(sgiparam.sparecyl),
	       SSWAP16(sgiparam.ilfact),
	       (char *)sgilabel,
	       str_units(PLURAL), units_per_sector);
    } else
    {
	printf( _("\nDisk %s (SGI disk label): "
		"%d heads, %d sectors, %d cylinders\n"
		"Units = %s of %d * 512 bytes\n\n"),
	        disk_device, heads, sectors, cylinders,
	        str_units(PLURAL), units_per_sector );
    }
    printf(_("----- partitions -----\n"
	   "%*s  Info      Start       End   Sectors  Id  System\n"),
	   w + 1, _("Device"));
    for (i = 0 ; i < partitions; i++)
    {
	if( sgi_get_num_sectors(i) || debug )
	{
	    __u32 start = sgi_get_start_sector(i);
	    __u32 len = sgi_get_num_sectors(i);
	    printf(
		"%*s%-2d %4s %9ld %9ld %9ld  %2x  %s\n",
/* device */              w, disk_device, i + 1,
/* flags */               (sgi_get_swappartition() == i) ? "swap" :
/* flags */               (sgi_get_bootpartition() == i) ? "boot" : "    ", 
/* start */               (long) scround(start),
/* end */                 (long) scround(start+len)-1,
/* no odd flag on end */  (long) len, 
/* type id */             sgi_get_sysid(i),
/* type name */           (type = partition_type(sgi_get_sysid(i)))
		    ? type : _("Unknown"));
	}
    }
    printf(_("----- bootinfo -----\nBootfile: %s\n"
	     "----- directory entries -----\n"),
	   sgilabel->boot_file );
    for (i = 0 ; i < volumes; i++)
    {
	if (sgilabel->directory[i].vol_file_size)
	{
	    __u32 start = SSWAP32(sgilabel->directory[i].vol_file_start);
	    __u32 len = SSWAP32(sgilabel->directory[i].vol_file_size);
	    char*name = sgilabel->directory[i].vol_file_name;
	    printf(_("%2d: %-10s sector%5u size%8u\n"),
		    i, name, (unsigned int) start, (unsigned int) len);
	}
    }
}

int
sgi_get_start_sector( int i ) {
    return SSWAP32(sgilabel->partitions[i].start_sector);
}

int
sgi_get_num_sectors( int i ) {
    return SSWAP32(sgilabel->partitions[i].num_sectors);
}

int
sgi_get_sysid( int i )
{
    return SSWAP32(sgilabel->partitions[i].id);
}

int
sgi_get_bootpartition(void)
{
    return SSWAP16(sgilabel->boot_part);
}

int
sgi_get_swappartition(void)
{
    return SSWAP16(sgilabel->swap_part);
}

void
sgi_set_bootpartition( int i )
{
    sgilabel->boot_part = SSWAP16(((short)i));
}

static int
sgi_get_lastblock(void) {
    return heads * sectors * cylinders;
}

void
sgi_set_swappartition( int i ) {
    sgilabel->swap_part = SSWAP16(((short)i));
}

static int
sgi_check_bootfile( const char* aFile ) {
    if( strlen( aFile ) < 3 ) /* "/a\n" is minimum */
    {
	printf( _("\nInvalid Bootfile!\n"
		"\tThe bootfile must be an absolute non-zero pathname,\n"
		"\te.g. \"/unix\" or \"/unix.save\".\n") );
	return 0;
    } else
    if( strlen( aFile ) > 16 )
    {
	printf( _("\n\tName of Bootfile too long:  16 bytes maximum.\n") );
	return 0;
    } else
    if( aFile[0] != '/' )
    {
	printf( _("\n\tBootfile must have a fully qualified pathname.\n") );
	return 0;
    }
    if( strncmp( aFile, sgilabel->boot_file, 16 ) )
    {
	printf( _("\n\tBe aware, that the bootfile is not checked for existence.\n\t"
		"SGI's default is \"/unix\" and for backup \"/unix.save\".\n") );
	/* filename is correct and did change */
	return 1;
    }
    return 0;	/* filename did not change */
}

const char *
sgi_get_bootfile(void) {
	return sgilabel->boot_file;
}

void
sgi_set_bootfile( const char* aFile )
{
    int i = 0;
    if( sgi_check_bootfile( aFile ) )
    {
	while( i<16 )
	{
	    if( (aFile[i] != '\n')	/* in principle caught again by next line */
	    &&  (strlen( aFile ) > i ) )
		sgilabel->boot_file[i] = aFile[i];
	    else
		sgilabel->boot_file[i] = 0;
	    i++;
	}
	printf( _("\n\tBootfile is changed to \"%s\".\n"), sgilabel->boot_file );
    }
}

void
create_sgiinfo(void)
{
    /* I keep SGI's habit to write the sgilabel to the second block */
    sgilabel->directory[0].vol_file_start = SSWAP32( 2 );
    sgilabel->directory[0].vol_file_size = SSWAP32( sizeof( sgiinfo ) );
    strncpy( sgilabel->directory[0].vol_file_name, "sgilabel",8 );
}

sgiinfo * fill_sgiinfo(void);

void
sgi_write_table(void)
{
    sgilabel->csum = 0;
    sgilabel->csum = SSWAP32( two_s_complement_32bit_sum(
				 (unsigned int*)sgilabel, 
				 sizeof(*sgilabel) ) );
    assert( two_s_complement_32bit_sum(
	    (unsigned int*)sgilabel, sizeof(*sgilabel) ) == 0 );
    if( lseek(fd, 0, SEEK_SET) < 0 )
	fatal(unable_to_seek);
    if( write(fd, sgilabel, SECTOR_SIZE) != SECTOR_SIZE )
	fatal(unable_to_write);
    if( ! strncmp( sgilabel->directory[0].vol_file_name, "sgilabel",8 ) )
    {
	/*
	 * keep this habbit of first writing the "sgilabel".
	 * I never tested whether it works without (AN 981002).
	 */
	sgiinfo*info = fill_sgiinfo();	/* fills the block appropriately */
	int infostartblock = SSWAP32( sgilabel->directory[0].vol_file_start );
	if( ext2_llseek(fd, (ext2_loff_t)infostartblock*
	    SECTOR_SIZE, SEEK_SET) < 0 )
	    fatal(unable_to_seek);
	if( write(fd, info, SECTOR_SIZE) != SECTOR_SIZE )
	    fatal(unable_to_write);
	free( info );
    }
}

static int
compare_start( int *x, int *y ) {
    /*
     * sort according to start sectors
     * and prefers largest partition:
     * entry zero is entire disk entry
     */
    int i = *x;
    int j = *y;
    int a = sgi_get_start_sector(i);
    int b = sgi_get_start_sector(j);
    int c = sgi_get_num_sectors(i);
    int d = sgi_get_num_sectors(j);
    if( a == b )
    {
	return( d - c );
    }
    return( a - b );
}

static int
sgi_gaps(void) {
    /*
     * returned value is:
     *  = 0 : disk is properly filled to the rim
     *  < 0 : there is an overlap
     *  > 0 : there is still some vacant space
     */
    return verify_sgi(0);
}

int
verify_sgi( int verbose )
{
    int Index[16];	/* list of valid partitions */
    int sortcount = 0;	/* number of used partitions, i.e. non-zero lengths */
    int entire = 0, i = 0;	/* local counters */
    int start = 0;
    int gap = 0;	/* count unused blocks */
    int lastblock = sgi_get_lastblock();
    /*
     */
    clearfreelist();
    for( i=0; i<16; i++ )
    {
	if( sgi_get_num_sectors(i)!=0 )
	{
	    Index[sortcount++]=i;
	    if( sgi_get_sysid(i) == ENTIRE_DISK )
	    {
		if( entire++ == 1 )
		{
		    if(verbose)
			printf(_("More than one entire disk entry present.\n"));
		}
	    }
	}
    }
    if( sortcount == 0 )
    {
	if(verbose)
	    printf(_("No partitions defined\n"));
        return lastblock;
    }
    qsort( Index, sortcount, sizeof(Index[0]), (void*)compare_start );
    if( sgi_get_sysid( Index[0] ) == ENTIRE_DISK )
    {
	if( ( Index[0] != 10 ) && verbose )
	    printf( _("IRIX likes when Partition 11 covers the entire disk.\n") );
	if( ( sgi_get_start_sector( Index[0] ) != 0 ) && verbose )
	    printf( _("The entire disk partition should start at block 0,\nnot "
		    "at diskblock %d.\n"), sgi_get_start_sector(Index[0] ) );
    if(debug)	/* I do not understand how some disks fulfil it */
	if( ( sgi_get_num_sectors( Index[0] ) != lastblock ) && verbose )
	    printf( _("The entire disk partition is only %d diskblock large,\n"
		    "but the disk is %d diskblocks long.\n"),
		    sgi_get_num_sectors( Index[0] ), lastblock );
	lastblock = sgi_get_num_sectors( Index[0] );
    } else
    {
	if( verbose )
	    printf( _("One Partition (#11) should cover the entire disk.\n") );
	if(debug>2)
	    printf( "sysid=%d\tpartition=%d\n",
			sgi_get_sysid( Index[0] ), Index[0]+1 );
    }
    for( i=1, start=0; i<sortcount; i++ )
    {
	int cylsize = sgi_get_nsect() * sgi_get_ntrks();
	if( (sgi_get_start_sector( Index[i] ) % cylsize) != 0 )
	{
	if(debug)	/* I do not understand how some disks fulfil it */
	    if( verbose )
		printf( _("Partition %d does not start on cylinder boundary.\n"),
			Index[i]+1 );
	}
	if( sgi_get_num_sectors( Index[i] ) % cylsize != 0 )
	{
	if(debug)	/* I do not understand how some disks fulfil it */
	    if( verbose )
		printf( _("Partition %d does not end on cylinder boundary.\n"),
			Index[i]+1 );
	}
	/* We cannot handle several "entire disk" entries. */
	if( sgi_get_sysid( Index[i] ) == ENTIRE_DISK ) continue;
	if( start > sgi_get_start_sector( Index[i] ) )
	{
	    if( verbose )
		printf( _("The Partition %d and %d overlap by %d sectors.\n"),
			Index[i-1]+1, Index[i]+1,
			start - sgi_get_start_sector( Index[i] ) );
	    if( gap >  0 ) gap = -gap;
	    if( gap == 0 ) gap = -1;
	}
	if( start < sgi_get_start_sector( Index[i] ) )
	{
	    if( verbose )
		printf( _("Unused gap of %8d sectors - sectors %8d-%d\n"),
			sgi_get_start_sector( Index[i] ) - start,
			start, sgi_get_start_sector( Index[i] )-1 );
	    gap += sgi_get_start_sector( Index[i] ) - start;
	    add2freelist( start, sgi_get_start_sector( Index[i] ) );
	}
	start = sgi_get_start_sector( Index[i] )
	      + sgi_get_num_sectors(  Index[i] );
	if(debug>1)
	{
	    if( verbose )
		printf( "%2d:%12d\t%12d\t%12d\n", Index[i],
			sgi_get_start_sector(Index[i]),
			sgi_get_num_sectors(Index[i]),
			sgi_get_sysid(Index[i]) );
	}
    }
    if( ( start < lastblock ) )
    {
	if( verbose )
	    printf( _("Unused gap of %8d sectors - sectors %8d-%d\n"),
		    lastblock - start, start, lastblock-1 );
	gap += lastblock - start;
	add2freelist( start, lastblock );
    }
    /*
     * Done with arithmetics
     * Go for details now
     */
    if( verbose )
    {
	if( !sgi_get_num_sectors( sgi_get_bootpartition() ) )
	{
	    printf( _("\nThe boot partition does not exist.\n") );
	}
	if( !sgi_get_num_sectors( sgi_get_swappartition() ) )
	{
	    printf( _("\nThe swap partition does not exist.\n") );
	} else
	if( ( sgi_get_sysid( sgi_get_swappartition() ) != SGI_SWAP )
	&&  ( sgi_get_sysid( sgi_get_swappartition() ) != LINUX_SWAP ) )
	{
	    printf( _("\nThe swap partition has no swap type.\n") );
	}
	if( sgi_check_bootfile( "/unix" ) )
	{
	    printf( _("\tYou have chosen an unusual boot file name.\n") );
	}
    }
    return gap;
}

void
sgi_change_sysid( int i, int sys )
{
    if( sgi_get_num_sectors(i) == 0 ) /* caught already before, ... */
    {
	printf(_("Sorry You may change the Tag of non-empty partitions.\n"));
	return;
    }
    if( ((sys != ENTIRE_DISK ) && (sys != SGI_VOLHDR))
     && (sgi_get_start_sector(i)<1) )
    {
	read_chars(
	_("It is highly recommended that the partition at offset 0\n"
	"is of type \"SGI volhdr\", the IRIX system will rely on it to\n"
	"retrieve from its directory standalone tools like sash and fx.\n"
	"Only the \"SGI volume\" entire disk section may violate this.\n"
	"Type YES if you are sure about tagging this partition differently.\n"));
	if (strcmp (line_ptr, "YES\n"))
                    return;
    }
    sgilabel->partitions[i].id = SSWAP32(sys);
}

/* returns partition index of first entry marked as entire disk */
static int
sgi_entire(void) {
    int i=0;
    for( i=0; i<16; i++ )
	if( sgi_get_sysid(i) == SGI_VOLUME )
	    return i;
    return -1;
}

static void
sgi_set_partition( int i, uint start, uint length, int sys ) {
    sgilabel->partitions[i].id =
	    SSWAP32( sys );
    sgilabel->partitions[i].num_sectors =
	    SSWAP32( length );
    sgilabel->partitions[i].start_sector =
	    SSWAP32( start );
    set_changed(i);
    if( sgi_gaps() < 0 )	/* rebuild freelist */
	printf(_("Do You know, You got a partition overlap on the disk?\n"));
}

static void
sgi_set_entire(void) {
    int n;
    for( n=10; n<partitions; n++ ) {
	if(!sgi_get_num_sectors( n ) ) {
	    sgi_set_partition( n, 0, sgi_get_lastblock(), SGI_VOLUME );
	    break;
	}
    }
}

static
void
sgi_set_volhdr(void)
{
    int n;
    for( n=8; n<partitions; n++ )
    {
	if(!sgi_get_num_sectors( n ) )
	{
	    /*
	     * 5 cylinders is an arbitrary value I like
	     * IRIX 5.3 stored files in the volume header
	     * (like sash, symmon, fx, ide) with ca. 3200
	     * sectors.
	     */
	    if( heads * sectors * 5 < sgi_get_lastblock() )
		sgi_set_partition( n, 0, heads * sectors * 5, SGI_VOLHDR );
	    break;
	}
    }
}

void
sgi_delete_partition( int i )
{
    sgi_set_partition( i, 0, 0, 0 );
}

void
sgi_add_partition( int n, int sys )
{
    char mesg[256];
    int first=0, last=0;

    if( n == 10 ) {
	sys = SGI_VOLUME;
    } else if ( n == 8 ) {
	sys = 0;
    }
    if( sgi_get_num_sectors(n) )
    {
	printf(_("Partition %d is already defined.  Delete "
		"it before re-adding it.\n"), n + 1);
	return;
    }
    if( (sgi_entire() == -1)
    &&  (sys != SGI_VOLUME) )
    {
	printf(_("Attempting to generate entire disk entry automatically.\n"));
	sgi_set_entire();
	sgi_set_volhdr();
    }
    if( (sgi_gaps() == 0)
    &&  (sys != SGI_VOLUME) )
    {
	printf(_("The entire disk is already covered with partitions.\n"));
	return;
    }
    if( sgi_gaps() < 0 )
    {
	printf(_("You got a partition overlap on the disk. Fix it first!\n"));
	return;
    }
    sprintf(mesg, _("First %s"), str_units(SINGULAR));
    for(;;) {
	if(sys == SGI_VOLUME) {
	    last = sgi_get_lastblock();
	    first = read_int(0, 0, last-1, 0, mesg);
	    if( first != 0 ) {
		printf(_("It is highly recommended that eleventh partition\n"
		       "covers the entire disk and is of type `SGI volume'\n"));
	    }
	} else {
	    first = freelist[0].first;
	    last  = freelist[0].last;
	    first = read_int(scround(first), scround(first), scround(last)-1,
			     0, mesg);
	}
	if (display_in_cyl_units)
	    first *= units_per_sector;
	else
	    first = first; /* align to cylinder if you know how ... */
	if( !last )
	    last = isinfreelist(first);
	if( last == 0 ) {
	    printf(_("You will get a partition overlap on the disk. "
		    "Fix it first!\n"));
	} else
	    break;
    }
    sprintf(mesg, _(" Last %s"), str_units(SINGULAR));
    last = read_int(scround(first), scround(last)-1, scround(last)-1,
		    scround(first), mesg)+1;
    if (display_in_cyl_units)
	last *= units_per_sector;                                     
    else                                                             
	last = last; /* align to cylinder if You know how ... */
    if( (sys == SGI_VOLUME) && ( first != 0 || last != sgi_get_lastblock() ) )
	printf(_("It is highly recommended that eleventh partition\n"
		"covers the entire disk and is of type `SGI volume'\n"));
    sgi_set_partition( n, first, last-first, sys );
}

void
create_sgilabel(void)
{
    struct hd_geometry geometry;
    struct { int start;
	     int nsect;
	     int sysid; } old[4];
    int i=0;
    fprintf( stderr,
	_("Building a new SGI disklabel. Changes will remain in memory only,\n"
	"until you decide to write them. After that, of course, the previous\n"
	"content will be unrecoverably lost.\n\n"));
#if BYTE_ORDER == LITTLE_ENDIAN
    other_endian = 1;
#else
    other_endian = 0;
#endif
#ifdef HDIO_REQ
    if (!ioctl(fd, HDIO_REQ, &geometry))
#else
    if (!ioctl(fd, HDIO_GETGEO, &geometry))
#endif
    {
	heads = geometry.heads;
	sectors = geometry.sectors;
	cylinders = geometry.cylinders;
    }
    for (i = 0; i < 4; i++)
    {
	old[i].sysid = 0;
	if( valid_part_table_flag(MBRbuffer) )
	{
	    if( get_part_table(i)->sys_ind )
	    {
		old[i].sysid = get_part_table(i)->sys_ind;
		old[i].start = get_start_sect( get_part_table(i) );
		old[i].nsect = get_nr_sects( get_part_table(i) );
		printf( _("Trying to keep parameters of partition %d.\n"), i );
		if( debug )
		    printf( _("ID=%02x\tSTART=%d\tLENGTH=%d\n"),
			    old[i].sysid, old[i].start, old[i].nsect );
	    }
	}
    }
    memset(MBRbuffer, 0, sizeof(MBRbuffer));
    sgilabel->magic = SSWAP32(SGI_LABEL_MAGIC);
    sgilabel->boot_part = SSWAP16(0);
    sgilabel->swap_part = SSWAP16(1); strncpy(
    sgilabel->boot_file , "/unix\0\0\0\0\0\0\0\0\0\0\0", 16 );
    sgilabel->devparam.skew			= (0);
    sgilabel->devparam.gap1			= (0);
    sgilabel->devparam.gap2			= (0);
    sgilabel->devparam.sparecyl			= (0);
    sgilabel->devparam.pcylcount		= SSWAP16(geometry.cylinders);
    sgilabel->devparam.head_vol0		= SSWAP16(0);
    sgilabel->devparam.ntrks			= SSWAP16(geometry.heads);
						/* tracks/cylinder (heads) */
    sgilabel->devparam.cmd_tag_queue_depth	= (0);
    sgilabel->devparam.unused0			= (0);
    sgilabel->devparam.unused1			= SSWAP16(0);
    sgilabel->devparam.nsect			= SSWAP16(geometry.sectors);
						/* sectors/track */
    sgilabel->devparam.bytes			= SSWAP16(512);
    sgilabel->devparam.ilfact			= SSWAP16(1);
    sgilabel->devparam.flags			= SSWAP32(TRACK_FWD|\
							IGNORE_ERRORS|RESEEK);
    sgilabel->devparam.datarate			= SSWAP32(0);
    sgilabel->devparam.retries_on_error		= SSWAP32(1);
    sgilabel->devparam.ms_per_word		= SSWAP32(0);
    sgilabel->devparam.xylogics_gap1		= SSWAP16(0);
    sgilabel->devparam.xylogics_syncdelay	= SSWAP16(0);
    sgilabel->devparam.xylogics_readdelay	= SSWAP16(0);
    sgilabel->devparam.xylogics_gap2		= SSWAP16(0);
    sgilabel->devparam.xylogics_readgate	= SSWAP16(0);
    sgilabel->devparam.xylogics_writecont	= SSWAP16(0);
    memset( &(sgilabel->directory), 0, sizeof(struct volume_directory)*15 );
    memset( &(sgilabel->partitions), 0, sizeof(struct sgi_partition)*16 );
    sgi_label  =  1;
    partitions = 16;
    volumes    = 15;
    sgi_set_entire();
    sgi_set_volhdr();
    for (i = 0; i < 4; i++)
    {
	if( old[i].sysid )
	{
	    sgi_set_partition( i, old[i].start, old[i].nsect, old[i].sysid );
	}
    }
}

void
sgi_set_ilfact(void)
{
    /* do nothing in the beginning */
}

void
sgi_set_rspeed(void)
{
    /* do nothing in the beginning */
}

void
sgi_set_pcylcount(void)
{
    /* do nothing in the beginning */
}

void
sgi_set_xcyl(void)
{
    /* do nothing in the beginning */
}

void
sgi_set_ncyl(void)
{
    /* do nothing in the beginning */
}

/* _____________________________________________________________
 */

sgiinfo*
fill_sgiinfo(void)
{
    sgiinfo*info=calloc( 1, sizeof(sgiinfo) );
    info->magic=SSWAP32(SGI_INFO_MAGIC);
    info->b1=SSWAP32(-1);
    info->b2=SSWAP16(-1);
    info->b3=SSWAP16(1);
    /* You may want to replace this string !!!!!!! */
    strcpy( info->scsi_string, "IBM OEM 0662S12         3 30" );
    strcpy( info->serial, "0000" );
    info->check1816 = SSWAP16(18*256 +16 );
    strcpy( info->installer, "Sfx version 5.3, Oct 18, 1994" );
    return info;
}
