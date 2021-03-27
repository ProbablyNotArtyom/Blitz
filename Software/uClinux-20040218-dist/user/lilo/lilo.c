/* lilo.c  -  LILO command-line parameter processing */
/*
Copyright 1992-1998 Werner Almesberger.
Copyright 1999-2001 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

*/


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include <asm/page.h>

#include "config.h"
#include "common.h"
#include "lilo.h"
#include "boot.h"
#include "temp.h"
#include "device.h"
#include "geometry.h"
#include "map.h"
#include "bsect.h"
#include "cfg.h"
#include "identify.h"
#include "partition.h"
#include "probe.h"
#include "md-int.h"

static int lowest;
static md_array_info_t md_array_info;
static DT_ENTRY *md_disk;
static DT_ENTRY *disk;
static unsigned long raid_base, raid_offset[MAX_RAID];
static char *raid_mbr[MAX_RAID];
static int raid_device[MAX_RAID+1];
static int raid_bios[MAX_RAID+1];
static int device, md_bios;
static enum {X_NULL=0, X_NONE, X_AUTO, X_MBR_ONLY, X_SPEC} extra;
enum {MD_NULL=0, MD_PARALLEL, MD_MIXED, MD_SKEWED};
int do_md_install;
char *config_file;		/* actual name of the config file */
int config_read;		/* readable by other than root */
FILE *errstd;
static char *raid_list[MAX_RAID];
static int list_index[MAX_RAID];
static int ndisk, nlist, faulty;


#define IS_COVERED    0x1000
#define TO_BE_COVERED 0x2000
#define COVERED   (IS_COVERED|TO_BE_COVERED)

static int is_primary(int device)
{
    int mask;
    
    mask = has_partitions(device);
    if (!mask) die("is_primary:  Not a valid device  0x%04X", device);
    mask = device & ~mask;
    return (mask && mask<=PART_MAX);
}


#if 0
static int is_master(int device)
{
    int mask;
    
    mask = has_partitions(device);
    if (!mask) die("is_master:  Not a valid device  0x%04X", device);
    mask = device & ~mask;
    return (!mask);
}
#endif


static int master(int device)
{
    int mask;
    
    mask = has_partitions(device);
    if (!mask) die("master:  Not a valid device  0x%04X", device);
    return device & mask;
}


static int is_accessible(int device)
{
    int mask;
    
    mask = has_partitions(device);
    if (!mask) die("is_accessible:  Not a valid device  0x%04X", device);
    mask = device & ~mask;
    return (mask<=PART_MAX);
}


static void raid_final(void)
{
    int pass, force;
    char *cp;

	if (verbose>=2)
	printf("do_md_install: %s\n", do_md_install == MD_PARALLEL ? "MD_PARALLEL" :
		do_md_install == MD_MIXED ? "MD_MIXED" :
		do_md_install == MD_SKEWED ? "MD_SKEWED" : "unknown");
		
    if (extra == X_MBR_ONLY) {    
        pass = 0;
        while (pass < ndisk) {
	    if (!test) {
	        force = 0;
	    
  /* we need to re-visit the logic about backups */
	        if ((cp=cfg_get_strg(cf_options,"force-backup"))) force=1;
	        else cp=cfg_get_strg(cf_options,"backup");
	    
	        bsect_raid_update(raid_mbr[pass], raid_offset[pass],
			cp, force, pass);
	        if (!nowarn) fprintf(errstd, "The Master boot record of  %s  has been updated.\n", raid_mbr[pass]);

	    } else {
	        if (pass==0) {
	    	    bsect_cancel();
		    if (passw) fprintf(errstd,"The password crc file has *NOT* been updated.\n");
		    fprintf(errstd, "The map file has *NOT* been altered.\n");
	        }

	        fprintf(errstd,"The Master boot record of  %s  has *NOT* been altered.\n", 
	    		raid_mbr[pass]);
	    }
	    pass++;
        }
    } 
    else {	/*  extra != X_MBR_ONLY   */
    	char *boot = cfg_get_strg(cf_options,"boot");
    	
	raid_flags &= ~FLAG_RAID_DEFEAT;    /* change won't affect /dev/mdX */
	if (!test) {
	    force = 0;
	    
	    if ((cp=cfg_get_strg(cf_options,"force-backup"))) force=1;
	    else cp=cfg_get_strg(cf_options,"backup");

	/* write out the /dev/mdX boot records */	    
	    bsect_raid_update(boot, 0L, cp, force, 0);
	    if (!nowarn) fprintf(errstd, "The boot record of  %s  has been updated.\n", boot);
	}
	else {
	    bsect_cancel();
	    if (passw) fprintf(errstd,"The password crc file has *NOT* been updated.\n");
	    fprintf(errstd, "The map file has *NOT* been updated.\n");
	    fprintf(errstd,"The boot record of  %s  has *NOT* been updated.\n", 
	    		boot);
	}

	if (extra == X_NONE || (extra == X_AUTO && do_md_install == MD_PARALLEL) ) return;

	if (extra == X_SPEC)
	for (pass = 0; pass < nlist; pass++) {
	    int index;
	    
	    if (raid_bios[list_index[pass]] & 0xFF) {
	    	index = list_index[pass];	/* within RAID set */
	    }
	    else {  /* not in the RAID set */
	    	raid_flags |= FLAG_RAID_DEFEAT;  /* make outsider invisible */
	    	index = lowest;
	    }

	    if (verbose>=2) printf("Specifed partition:  %s  raid offset = %08lX\n",
					raid_list[pass], raid_offset[index]);

	    if (!test) {
	        bsect_raid_update(raid_list[pass], raid_offset[index],
	    		NULL, 0, 1);
	    }
	    if (test || !nowarn)
	        fprintf(errstd,"The boot record of  %s  has%s been updated.\n",
			raid_list[pass], (test ? " *NOT*" : ""));

	    raid_flags &= ~FLAG_RAID_DEFEAT; /* restore DEFEAT flag to 0 */
	}
	else {		/* extra = X_AUTO */
	    for (pass = 0; pass < ndisk; pass++)
	    if (!(raid_bios[pass] & IS_COVERED) && (raid_bios[pass] & 0xFF) != 0x80) {
		if (!test)
		    bsect_raid_update(raid_mbr[pass], raid_offset[pass],
		    	NULL, 0, 1);
		if (test || !nowarn)
	            fprintf(errstd,"The Master boot record of  %s  has%s been updated.\n",
			raid_mbr[pass], (test ? " *NOT*" : ""));
	    }
	}
    }


    if (raid_flags & FLAG_RAID_NOWRITE) {
	if (!nowarn) {
		fprintf(errstd, "Warning: FLAG_RAID_NOWRITE has been set.\n");
		if (verbose >= 1)
		fprintf(errstd,   "  The boot loader will be unable to update the stored command line;\n"
		                  "  'lock' and 'fallback' are not operable; the 'lilo -R' boot command\n"
		                  "  line will be locked.\n\n");
	}
    }

}


static long raid_setup(void)
{
    int pass, mask;
    struct stat st;
    int md_fd;
    md_disk_info_t md_disk_info;
    GEOMETRY geo;
    char *boot, *extrap;
    int ro_set, all_pri_eq, pri_index;
    long pri_offset;
    int raid_limit;
    
    if ((boot=cfg_get_strg(cf_options,"boot")) != NULL &&
        strncmp("/dev/md",boot,7) == 0) {
	if ((md_fd=open(boot,O_NOACCESS)) < 0)
	    die("Unable to open %s",boot);
	if (fstat(md_fd,&st) < 0)
	    die("Unable to stat %s",boot);
	if (!S_ISBLK(st.st_mode))
	    die("%s is not a block device",boot);
	if (ioctl(md_fd,GET_ARRAY_INFO,&md_array_info) < 0)
	    die("Unable to get RAID info on %s",boot);
	if ((md_array_info.major_version == 0) && (md_array_info.minor_version < 90))
	    die("Raid versions < 0.90 are not supported");
	if (md_array_info.level != 1)
	    die("Only RAID1 devices are supported as boot devices");
	if (!linear && !lba32) {
#if 0
	    cfg_set(cf_options,"lba32",NULL,NULL);
#endif
	    lba32 = 1;
	    if (!nowarn)
		fprintf(errstd,"Warning: RAID install requires LBA32 or LINEAR;"
			" LBA32 assumed.\n");
	}
	extrap = cfg_get_strg(cf_options, RAID_EXTRA_BOOT);
	extra = !extrap ? X_AUTO :
		!strcasecmp(extrap,"none") ? X_NONE :
		!strcasecmp(extrap,"auto") ? X_AUTO :
		!strcasecmp(extrap,"mbr-only") ? X_MBR_ONLY :
		    X_SPEC;
	
	do_md_install = MD_PARALLEL;

	all_pri_eq = 1;
	ro_set = pri_index = pri_offset = 0;
	raid_flags = FLAG_RAID;
	md_bios = 0xFF;			/* we want to find the minimum */
	ndisk = 0;			/* count the number of disks on-line */
	nlist = 0;
	faulty = 0;
	
	device = (MD_MAJOR << 8) | md_array_info.md_minor;
	
    /* search the disk table for a definition */
	md_disk = disktab;
	while (md_disk && md_disk->device != device)
	    md_disk = md_disk->next;
	    
	if (!md_disk) {
	    md_disk = alloc_t(DT_ENTRY);
	    md_disk->device = (MD_MAJOR << 8) | md_array_info.md_minor;
	    md_disk->bios = -1;	/* use the default */
	    md_disk->next = disktab;
	    disktab = md_disk;
	}

	if (verbose >= 2) {
	   printf("RAID info:  nr=%d, raid=%d, active=%d, working=%d, failed=%d, spare=%d\n",
		md_array_info.nr_disks,
		md_array_info.raid_disks,
		md_array_info.active_disks,
		md_array_info.working_disks,
		md_array_info.failed_disks,
		md_array_info.spare_disks );
	}

    /* scan through all the RAID devices */
	raid_limit = md_array_info.raid_disks + md_array_info.spare_disks;
   	for (pass=0; pass < raid_limit; pass++) {
	    DEVICE dev;
	    int disk_fd;
	    char new_name[MAX_TOKEN+1];
	    char *np;
	    
	    md_disk_info.number = pass;
	    if (ioctl(md_fd,GET_DISK_INFO,&md_disk_info) < 0)
		die("main: GET_DISK_INFO: %s", strerror(errno));
	    device = (md_disk_info.major << 8) | md_disk_info.minor;
            if(verbose>=2) printf("md: RAIDset device %d = 0x%04X\n", pass, device);	    
	    if (device == 0) { /* empty slot left over from recovery process */
	        faulty++;
		continue;
	    }
	    disk_fd = dev_open(&dev,device,O_NOACCESS);
	    if (md_disk_info.state & (1 << MD_DISK_FAULTY)) {
		printf("disk %s marked as faulty, skipping\n",dev.name);
		faulty++;
		continue;
	    }
	    geo_get(&geo, device, -1, 1);
	    disk = alloc_t(DT_ENTRY);
	    if (verbose>=2)
		printf("RAID scan: geo_get: returns geo->device = 0x%02X"
		      " for device %04X\n", geo.device, device);
	      
	    disk->bios = geo.device;	/* will be overwritten */
	    disk->device = device;
	      /* used to mask above with 0xFFF0; forces MBR; sloppy, mask may be: 0xFFF8 */
	    disk->sectors = geo.sectors;
	    disk->heads = geo.heads;
	    disk->cylinders = geo.cylinders;
	    disk->start = geo.start;
	    if (ndisk==0) raid_base = geo.start;
	    raid_offset[ndisk] = geo.start - raid_base;
	    raid_device[ndisk] = device;

	    if (raid_offset[ndisk]) {
	        do_md_install = MD_SKEWED;	 /* flag non-zero raid_offset */
	    }

	    if (all_pri_eq && is_primary(device)) {
		if (ro_set) {
		    all_pri_eq &= (pri_offset == raid_offset[ndisk]);
		} else {
		    pri_offset = raid_offset[ndisk];
		    ro_set = 1;
		    pri_index = ndisk;
		}
	    }

	    if (geo.device < md_bios) {
	        md_bios = geo.device;	/* find smallest device code, period */
	        lowest = ndisk;		/* record where */
	    }
	    raid_bios[ndisk] = geo.device;  /* record device code */

	    disk->next = disktab;
	    disktab = disk;

	    if (verbose >= 2 && do_md_install) {
		printf("disk->start = %d\t\traid_offset = %ld (%08lX)\n",
		   disk->start, (long)raid_offset[ndisk], (long)raid_offset[ndisk]);
	    }
   	
	/* derive the MBR name, which may be needed later */
	    strncpy(new_name,dev.name,MAX_TOKEN);
	    new_name[MAX_TOKEN] = '\0';
	    np = boot_mbr(dev.name, 0);
	    if (!np) np = stralloc(new_name);
	    raid_mbr[ndisk] = np;

	    if (ndisk==0) {	/* use the first disk geometry */
		md_disk->sectors = geo.sectors;
		md_disk->heads = geo.heads;
		md_disk->cylinders = geo.cylinders;
		md_disk->start = geo.start;
	    }
	    
	    ndisk++;  /* count the disk */
   	}  /* for (pass=...    */

   	if(close(md_fd) < 0) die("Error on close of %s", boot);
   	raid_bios[ndisk] = 0;		/* mark the end */
   	raid_device[ndisk] = 0;

	all_pri_eq &= ro_set;
	if (all_pri_eq && do_md_install == MD_SKEWED) {
	    do_md_install = MD_MIXED;
	}
	else pri_index = lowest;

	/* check that all devices have an accessible block for writeback info */
	for (pass=0; pass < ndisk; pass++) {
	    if (extra == X_MBR_ONLY)
		raid_bios[pass] |= TO_BE_COVERED;

	    if (extra == X_AUTO && raid_bios[pass] != 0x80) {
		if (do_md_install == MD_SKEWED)  raid_bios[pass] |= TO_BE_COVERED;
		if (do_md_install == MD_MIXED) {
		    if (is_primary(raid_device[pass])) raid_bios[pass] |= IS_COVERED;
		    else  raid_bios[pass] |= TO_BE_COVERED;
		}
	    }
		
	    if ((do_md_install == MD_PARALLEL && is_accessible(raid_device[pass]))
		|| (do_md_install == MD_MIXED && pri_offset == raid_offset[pass]
		        && is_primary(raid_device[pass]))
		)    
		raid_bios[pass] |= IS_COVERED;
	}
	   	
	nlist = 0;
	if (extra==X_SPEC) {
	    char *next, *scan;
	    
	    scan = next = extrap;
	    while (next && *next) {
		scan = next;
		while (isspace(*scan)) scan++;	/* deblank the line */
		next = strchr(scan, ',');	/* find the separator */
		if (next) *next++ = 0;		/* NUL terminate  scan */
		    
		if ((md_fd=open(scan,O_NOACCESS)) < 0)
		    die("Unable to open %s", scan);
		if (fstat(md_fd,&st) < 0)
		    die("Unable to stat %s",scan);
		if (!S_ISBLK(st.st_mode))
		    die("%s is not a block device",scan);
	    	mask = has_partitions(st.st_rdev);
	    	if (!mask) die("%s (%04X) not a block device", scan, (int)st.st_rdev);
		if (verbose>=2) printf("RAID list: %s is device 0x%04X\n",
				scan, (int)st.st_rdev);	    	
		close(md_fd);
		
		list_index[nlist] = ndisk;  /* raid_bios==0 here */
		for (pass=0; pass < ndisk; pass++) {
		    if (master(st.st_rdev) == master(raid_device[pass])) {
		    	list_index[nlist] = pass;
		    	if (st.st_rdev == raid_device[pass])
			    die("Cannot write to a partition within a RAID set:  %s", scan);
		    	else if (is_accessible(st.st_rdev))
		    	    raid_bios[pass] |= IS_COVERED;
		    	break;
		    }
		}
		if (list_index[nlist] == ndisk) {
#if 0
		    raid_flags |= FLAG_RAID_NOWRITE;  /* disk is outside RAID set */
#endif
		    if (!nowarn) printf("Warning: device outside of RAID set  %s  0x%04X\n", 
		    				scan, (int)st.st_rdev);
		}
		raid_list[nlist++] = stralloc(scan);
	    }
	    
	}
	
	   	
    /* if the install is to MBRs, then change the boot= name */
	if (extra == X_MBR_ONLY) {
	    if (cfg_get_strg(cf_options,"boot")) cfg_unset(cf_options,"boot");
	    cfg_set(cf_options, "boot", (boot=raid_mbr[0]), NULL);
	}
	else {	/* if skewed install, disable mdX boot records as 
							source of writeback info */
	    if (do_md_install == MD_SKEWED) raid_flags |= FLAG_RAID_DEFEAT | 
	    	(extra == X_NONE ? FLAG_RAID_NOWRITE : 0);
	}

	mask = 1;
	for (pass=0; pass < ndisk; pass++) {
	    mask &= !!(raid_bios[pass] & COVERED);
	}
	if (!mask) {
	    raid_flags |= FLAG_RAID_NOWRITE;
	}

	if (raid_flags & FLAG_RAID_NOWRITE) {
	    if (!nowarn) {
		fprintf(errstd, "\nWarning: FLAG_RAID_NOWRITE has been set.\n");
#if 0
		if (verbose >= 1)
		fprintf(errstd,   "  The boot loader will be unable to update the stored command line;\n"
		                  "  'lock' and 'fallback' are not operable; the 'lilo -R' boot command\n"
		                  "  line will be locked.\n\n");
#endif
	    }
	}

    /* if the disk= bios= did not specify the bios, then this is the default */
	if (md_disk->bios < 0) {
	    md_disk->bios = md_bios;
	}
	if (md_disk->bios < 0x80 || md_disk->bios > DEV_MASK)
	   die("Unusual RAID bios device code: 0x%02X", md_disk->bios);
	disk = disktab;

	for (pass=0; pass < ndisk; pass++) {
	    disk->bios = md_disk->bios;	  /* all disks in the array are */
	    disk = disk->next;		  /*  assigned the same bios code */
	}

	if (!nowarn) {
	    fprintf(errstd,
	       "Warning: using BIOS device code 0x%02X for RAID boot blocks\n",
	    	                            md_disk->bios);
	}
	return raid_offset[pri_index];
    }	/* IF (test for a raid installation */
    else {	/* not raid at all */
	if (cfg_get_strg(cf_options, RAID_EXTRA_BOOT))
	    die("Not a RAID install, '" RAID_EXTRA_BOOT "=' not allowed");
	return 0L;
    }
}  /* void raid_setup(void) */
#undef COVERED

static void show_other(int fd)
{
    BOOT_SECTOR buf[SETUPSECS-1];
    const unsigned char *drvmap;
    const unsigned char *prtmap;

    if (read(fd,buf,sizeof(buf)) != sizeof(buf))
	die("Read on map file failed (access conflict ?)");
    if (!strncmp(buf[0].par_c.signature-4,"LILO",4)) {
	printf("    Pre-21 signature (0x%02x,0x%02x,0x%02x,0x%02x)\n",
	  buf[0].par_c.signature[0],buf[0].par_c.signature[1],
	  buf[0].par_c.signature[2],buf[0].par_c.signature[3]);
	return;
    }
    if (strncmp(buf[0].par_c.signature,"LILO",4)) {
	printf("    Bad signature (0x%02x,0x%02x,0x%02x,0x%02x)\n",
	  buf[0].par_c.signature[0],buf[0].par_c.signature[1],
	  buf[0].par_c.signature[2],buf[0].par_c.signature[3]);
	return;
    }
    drvmap = ((unsigned char *) buf+buf[0].par_c.drvmap);
    prtmap = drvmap+2*(DRVMAP_SIZE+1);
    while (drvmap[0] && drvmap[1]) {
	printf("    BIOS drive 0x%02x is mapped to 0x%02x\n",drvmap[0],
	  drvmap[1]);
	drvmap += 2;
    }
    while (prtmap[0] && prtmap[1]) {
	printf("    BIOS drive 0x%02x, offset 0x%x: 0x%02x -> 0x%02x\n",
	  prtmap[0],prtmap[1]+PART_TABLE_OFFSET,prtmap[2],prtmap[3]);
	prtmap += 4;
    }
}


static void show_images(char *map_file)
{
    DESCR_SECTORS descrs;
    BOOT_SECTOR boot;
    GEOMETRY geo;
    SECTOR_ADDR addr[4];
    char buffer[SECTOR_SIZE];
    char *name;
    int fd,image;
    int tsecs;
    int tlinear, tlba32;
    unsigned short flags;

    fd = geo_open(&geo,map_file,O_RDONLY);
    if (read(fd,buffer,SECTOR_SIZE) != SECTOR_SIZE)
	die("read %s: %s",map_file,strerror(errno));
    if (read(fd,(char *) &descrs,sizeof(descrs)) != sizeof(descrs))
	die("read %s: %s",map_file,strerror(errno));
    tlba32  = (descrs.d.descr[0].start.device & LBA32_FLAG) != 0;
    tlinear = !tlba32 && (descrs.d.descr[0].start.device & LINEAR_FLAG);
    if (tlinear != linear  ||  tlba32 != lba32) {
        printf("Warning: mapfile created with %s option\n",
	       tlinear?"linear":tlba32?"lba32":"no linear/lba32");
        linear = tlinear;  lba32 = tlba32;
    }
    if (verbose > 0) {
	bsect_read(cfg_get_strg(cf_options,"boot"),&boot);
	printf("Global settings:\n");
	tsecs = (boot.par_1.delay*2197+2000)/4000;
	printf("  Delay before booting: %d.%d seconds\n",tsecs/10,tsecs % 10);
	if (boot.par_1.timeout == 0xffff) printf("  No command-line timeout\n");
	else {
	    tsecs = (boot.par_1.timeout*2197+2000)/4000;
	    printf("  Command-line timeout: %d.%d seconds\n",tsecs/10,
	      tsecs % 10);
	}
	if (boot.par_1.prompt & FLAG_PROMPT) printf("  Always enter boot prompt\n");
	else printf("  Enter boot prompt only on demand\n");
	if (boot.par_1.prompt & FLAG_RAID) printf("  RAID installation\n");
	else printf("  Non-RAID installation\n");
	if (!boot.par_1.port) printf("  Serial line access is disabled\n");
	else printf("  Boot prompt can be accessed from COM%d\n",
	      boot.par_1.port);
	if (!boot.par_1.msg_len) printf("  No message for boot prompt\n");
	else if (!cfg_get_strg(cf_options,"bitmap"))
	    printf("  Boot prompt message is %d bytes\n",boot.par_1.msg_len);
	else printf("  Bitmap file is %d paragraphs (%d bytes)\n",
			boot.par_1.msg_len, 16*boot.par_1.msg_len);
	if (*(unsigned short *) buffer != DC_MAGIC || !buffer[2])
	    printf("  No default boot command line\n");
	else printf("  Default boot command line: \"%s\"\n",buffer+2);
	printf("Images:\n");
    }
    for (image = 0; image < MAX_IMAGES; image++)
	if (*(name = descrs.d.descr[image].name)) {
	    printf("%s%-" S(MAX_IMAGE_NAME) "s%s",verbose > 0 ? "  " : "",name,
	      image ? "  " : " *");
	    if (verbose >= 2) {
	        if (descrs.d.descr[image].start.device & (LINEAR_FLAG|LBA32_FLAG)) {
		   unsigned int sector;
		   sector = (descrs.d.descr[image].start.device & LBA32_FLAG)
		      && (descrs.d.descr[image].start.device & LBA32_NOCOUNT)
		        ? descrs.d.descr[image].start.num_sect : 0;
		   sector = (sector<<8)+descrs.d.descr[image].start.head;
	           sector = (sector<<8)+descrs.d.descr[image].start.track;
		   sector = (sector<<8)+descrs.d.descr[image].start.sector;
		   printf(" <dev=0x%02x,linear=%d>",
		     descrs.d.descr[image].start.device,
		     sector);
		}
	        else { /*  CHS addressing */
		    printf(" <dev=0x%02x,hd=%d,cyl=%d,sct=%d>",
		      descrs.d.descr[image].start.device,
		      descrs.d.descr[image].start.head,
		      descrs.d.descr[image].start.track,
		      descrs.d.descr[image].start.sector);
		}
	    }
	    printf("\n");
	    if (verbose >= 1) {
		flags = descrs.d.descr[image].flags;
		if ( !(flags & FLAG_PASSWORD) )
		    printf("    No password\n");
		else printf("    Password is required for %s\n",flags &
		      FLAG_RESTR ? "specifying options" : "booting this image");
		printf("    Boot command-line %s be locked\n",flags &
		  FLAG_LOCK ? "WILL" : "won't");
		printf("    %single-key activation\n",flags & FLAG_SINGLE ?
		  "S" : "No s");
		if (flags & FLAG_KERNEL) {
#ifdef NORMAL_VGA
		    if (!(flags & FLAG_VGA))
		       printf("    VGA mode is taken from boot image\n");
		    else {
			printf("    VGA mode: ");
			switch (descrs.d.descr[image].vga_mode) {
			    case NORMAL_VGA:
				printf("NORMAL\n");
				break;
			    case EXTENDED_VGA:
				printf("EXTENDED\n");
				break;
			    case ASK_VGA:
				printf("ASK\n");
				break;
			    default:
				printf("%d (0x%04x)\n",
				  descrs.d.descr[image].vga_mode,
				  descrs.d.descr[image].vga_mode);
			}
		    }
#endif
		    if (!descrs.d.descr[image].start_page)
			printf("    Kernel is loaded \"low\"\n");
		    else printf("    Kernel is loaded \"high\", at 0x%08lx\n",
			  (unsigned long) descrs.d.descr[image].start_page*
			  PAGE_SIZE);
		    if (!*(unsigned long *) descrs.d.descr[image].rd_size)
			printf("    No initial RAM disk\n");
		    else printf("    Initial RAM disk is %ld bytes\n",
			  *(unsigned long *) descrs.d.descr[image].rd_size);
		}
		if (!geo_find(&geo,descrs.d.descr[image].start)) {
		    printf("    Map sector not found\n");
		    continue;
		}
		if (read(fd,addr,4*sizeof(SECTOR_ADDR)) !=
		  4*sizeof(SECTOR_ADDR))
			die("Read on map file failed (access conflict ?)");
		if (!geo_find(&geo,addr[0]))
		    printf("    Fallback sector not found\n");
		else {
		    if (read(fd,buffer,SECTOR_SIZE) != SECTOR_SIZE)
			die("Read on map file failed (access conflict ?)");
		    if (*(unsigned short *) buffer != DC_MAGIC)
			printf("    No fallback\n");
		    else printf("    Fallback: \"%s\"\n",buffer+2);
		}
		if (flags & FLAG_KERNEL)
		    if (!geo_find(&geo,addr[1]))
			printf("    Options sector not found\n");
		    else {
			if (read(fd,buffer,SECTOR_SIZE) != SECTOR_SIZE)
			    die("Read on map file failed (access conflict ?)");
			printf("    Options: \"%s\"\n",buffer);
		    }
		else if (geo_find(&geo,addr[3])) show_other(fd);
		   else printf("    Image data not found\n");
	    }
	}
    (void) close(fd);
#if 0
    checksum = INIT_CKS;
    for (i = 0; i < sizeof(descrs)/sizeof(unsigned short); i++)
	checksum ^= ((unsigned short *) &descrs)[i];
    if (!checksum) exit(0);
#else
    if (descrs.l.checksum ==
    	  crc32(descrs.sector, sizeof(descrs.l.sector), CRC_POLY1) )  exit(0);
#endif
    fflush(stdout);
    fprintf(errstd,"Checksum error\n");
    exit(1);
}


static void usage(char *name)
{
    char *here;

    here = strrchr(name,'/');
    if (here) name = here+1;
    fprintf(errstd,"usage: %s [ -C config_file ] -q [ -m map_file ] "
      "[ -v N | -v ... ]\n",name);
    fprintf(errstd,"%7s%s [ -C config_file ] [ -b boot_device ] [ -c ] "
      "[ -g | -l | -L ]\n","",name);
    fprintf(errstd,"%12s[ -i boot_loader ] [ -m map_file ] [ -d delay ]\n","");
    fprintf(errstd,"%12s[ -v N | -v ... ] [ -t ] [ -s save_file | -S save_file ]\n",
      "");
    fprintf(errstd,"%12s[ -p ][ -P fix | -P ignore ] [ -r root_dir ] [ -w ]\n","");
    fprintf(errstd,"%7s%s [ -C config_file ] [ -m map_file ] "
      "-R [ word ... ]\n","",name);
    fprintf(errstd,"%7s%s [ -C config_file ] -I name [ options ]\n","",name);
    fprintf(errstd,"%7s%s [ -C config_file ] [ -s save_file ] "
      "-u | -U [ boot_device ]\n","",name);
    fprintf(errstd,"%7s%s -A /dev/XXX [ N ]\t\tactivate a partition\n","",name);
    fprintf(errstd,"%7s%s -M /dev/XXX [ mbr_file ]\tinstall master boot record\n","",name);
    fprintf(errstd,"%7s%s -T help \t\t\tlist additional options\n", "", name);
    fprintf(errstd,"%7s%s -V [ -v ]\t\t\tversion information\n\n","",name);
    exit(1);
}


int main(int argc,char **argv)
{
    char *name,*reboot_arg,*identify,*ident_opt,*new_root;
    char *tell_param, *uninst_dev, *param, *act1, *act2, ch;
    int query,more,version,uninstall,validate,activate,instmbr,geom;
    struct stat st;
    int fd;
    long raid_offset;

    errstd = stderr;
    config_file = DFL_CONFIG;
    act1 = act2 = tell_param = 
	    reboot_arg = identify = ident_opt = new_root = uninst_dev = NULL;
    lowest = do_md_install = zflag =
	    query = version = uninstall = validate = activate = instmbr = 0;
    verbose = -1;
    name = *argv;
    argc--;
    cfg_init(cf_options);
    while (argc && **++argv == '-') {
	argc--;
      /* first those options with a mandatory parameter */
      /* Notably absent are "RuUv" */
	if (strchr("AbCdDfiImMPrsSTx", ch=(*argv)[1])) {
	    if ((*argv)[2]) param = (*argv)+2;
	    else {
		param = *++argv;
		if(argc-- <= 0) usage(name);
	    }
	} else { 
	    param = NULL;
	}
#if 0
fprintf(errstd,"argc=%d, *argv=%s, ch=%c param=%s\n", argc, *argv, ch, param);
#endif
	switch (ch) {
	    case 'A':
		activate = 1;
		act1 = param;
		if (argc && argv[1][0] != '-') {
		    act2 = *++argv;
		    argc--;
		}
		break;
	    case 'b':
		cfg_set(cf_options,"boot",param,NULL);
		break;
	    case 'c':
		cfg_set(cf_options,"compact",NULL,NULL);
		compact = 1;
		break;
	    case 'C':
		config_file = param;
		break;
	    case 'd':
		cfg_set(cf_options,"delay",param,NULL);
		break;
	    case 'D':
		cfg_set(cf_options,"default",param,NULL);
		break;
	    case 'f':
		cfg_set(cf_options,"disktab",param,NULL);
		break;
	    case 'g':
		geometric |= 1;
		break;
	    case 'i':
		cfg_set(cf_options,"install",param,NULL);
		break;
	    case 'I':
		identify = param;
		if (argc && *argv[1] != '-') {
		    ident_opt = *++argv;
		    argc--;
		} else {
		    ident_opt = "i";
		}
		break;
	    case 'l':
		geometric |= 2;
		break;
	    case 'L':
		geometric |= 4;
		break;
	    case 'm':
		cfg_set(cf_options,"map",param,NULL);
		break;
	    case 'M':
		instmbr = 1;
		act1 = param;
		if (argc && argv[1][0] != '-') {
		    act2 = *++argv;
		    argc--;
		}
		break;
	    case 'p':
		passw = 1;	/* force re-gen of password file */
		break;
	    case 'P':
		if (!strcmp(param,"fix"))
		    cfg_set(cf_options,"fix-table",NULL,NULL);
		else if (!strcmp(param,"ignore"))
		    cfg_set(cf_options,"ignore-table",NULL,NULL);
		else usage(name);
		break;
	    case 'q':
		query = 1;
		break;
	    case 'r':
		new_root = param;
		break;
	    case 'R':
	        if (*(param = (*argv)+2)) argc++;
	        else if (argc) param = *++argv;
	        else reboot_arg = "";
	        
		while (argc) {
			if (!reboot_arg)
			    *(reboot_arg = alloc(strlen(param)+1)) = 0;
			else {
			    param = *++argv;
			    strcat(reboot_arg = ralloc(reboot_arg,
			        strlen(reboot_arg)+strlen(param)+2)," ");
			}
			strcat(reboot_arg, param);
			argc--;
		    }
#if 0
fprintf(errstd,"REBOOT=\"%s\"\n", reboot_arg);		    
#endif
		break;
	    case 's':
		cfg_set(cf_options,"backup",param,NULL);
		break;
	    case 'S':
		cfg_set(cf_options,"force-backup",param,NULL);
		break;
	    case 't':
		test = 1;
		break;
	    case 'T':
	        tell_param = param;
	    	break;
	    case 'u':
		validate = 1;
		/* fall through */
	    case 'U':	/* argument to -u or -U is optional */
		uninstall = 1;
		if ((*argv)[2]) param = (*argv)+2;
		else if (argc && argv[1][0] != '-') {
		    param = *++argv;
		    argc--;
		}
		uninst_dev = param;
		break;
	    case 'v':
	        if ((*argv)[2]) param = (*argv)+2;
	        else if (argc && argv[1][0]>='0' && argv[1][0]<='9') {
	            param = *++argv;
	            argc--;
	        }
	        if (param) 
		    verbose = to_number(param);
		else
	            if (verbose<0) verbose = 1;
	            else verbose++;
	        if (verbose) errstd = stdout;
		break;
	    case 'V':
		version = 1;
		break;
	    case 'w':
		cfg_set(cf_options,"nowarn",NULL,NULL);
		nowarn = 1;
		break;
	    case 'x':
		if (!strcmp(param,"none"))
		    cfg_set(cf_options,RAID_EXTRA_BOOT,param,NULL);
		else if (!strcmp(param,"auto"))
		    cfg_set(cf_options,RAID_EXTRA_BOOT,param,NULL);
		else if (!strcmp(param,"mbr-only"))
		    cfg_set(cf_options,RAID_EXTRA_BOOT,param,NULL);
		else
		    cfg_set(cf_options,RAID_EXTRA_BOOT,param,NULL);
		break;
	    case 'X':
#ifndef PAR1_PARAMS
		printf(
		"-DCODE_START_1=%d -DCODE_START_2=%d "
		  "\n"
		  ,
		  sizeof(BOOT_PARAMS_1),
		  sizeof(BOOT_PARAMS_2)
		     );
#else
		printf(
		"-DIMAGES=%d "
		"-DCODE_START_1=%d -DCODE_START_2=%d "
		  "-DDESCR_SIZE=%d "
		  "-DDSC_OFF=%d -DDSC_OFF2=%d -DDFCMD_OFF=%d -DMSG_OFF=%d "
		  "-DFLAGS_OFF=%d"
		  "\n"
		  ,
		  MAX_IMAGES,
		  sizeof(BOOT_PARAMS_1),
		  sizeof(BOOT_PARAMS_2),
		  sizeof(IMAGE_DESCR),
		  (void *) &dummy.par_1.descr[0]-(void *) &dummy,
		  (void *) &dummy.par_1.descr[1]-(void *) &dummy,
		  (void *) &dummy.par_1.descr[2]-(void *) &dummy,
		  (void *) &dummy.par_1.msg_len-(void *) &dummy,
		  (void *) &dummy2.flags-(void *) &dummy2
		     );
#endif
		exit(0);
	    case 'z':
		zflag++;	/* force zero of MBR 8-byte area */
		break;
	    default:
		usage(name);
	}
    }
    if (argc) usage(name);
    if (!new_root) new_root = getenv("ROOT");
    if (new_root && *new_root) {
	if (chroot(new_root) < 0) die("chroot %s: %s",new_root,strerror(errno));
	if (chdir("/dev") < 0 && !nowarn)
	    fprintf(errstd, "Warning: root at %s has no /dev directory\n", new_root);
	if (chdir("/") < 0) die("chdir /: %s",strerror(errno));
    }
    if (atexit(temp_remove)) die("atexit() failed");
    if (version+activate+instmbr+(tell_param!=NULL) > 1) usage(name);
    if (activate) do_activate(act1, act2);
    if (verbose > 0 || version) {
       printf("LILO version %d.%d%s%s", VERSION_MAJOR, VERSION_MINOR,
	      VERSION_EDIT, test ? " (test mode)" : "");
	if (version && verbose<=0) {
	    printf("\n");
	    return 0;
	}
	printf(", Copyright (C) 1992-1998 Werner Almesberger\n"
	       "Development beyond version 21 Copyright (C) 1999-2001 John Coffman\n"
	       );
        if (verbose>0) printf("Released %s and compiled at %s on %s.\n",
	        VERSION_DATE, __TIME__, __DATE__);
#ifdef LCF_DSECS
	printf("MAX_IMAGES = %d\n", MAX_IMAGES);
#endif
        printf("\n");
        if (version) return 0;
    }
    preload_types();
    if (geometric & (geometric-1))
	die ("Only one of '-g', '-l', or '-L' may be specified");
    fd = cfg_open(config_file);
    more = cfg_parse(cf_options);
    if (verbose > 0) errstd = stdout;
    if (verbose>=6) printf("main: cfg_parse returns %d\n", more);
    if (tell_param) probe_tell(tell_param);
    if (instmbr) do_install_mbr(act1, act2);
    
    if (!nowarn) {
	if (fstat(fd,&st) < 0) {
	    fprintf(errstd,"fstat %s: %s\n",config_file,strerror(errno));
	    exit(1);
	}
	if (S_ISREG(st.st_mode)) {
	    if (st.st_uid)
		fprintf(errstd,"Warning: %s should be owned by root\n",
		  config_file);
	    else if (st.st_mode & (S_IWGRP | S_IWOTH))
		    fprintf(errstd,"Warning: %s should be writable only for "
		      "root\n",config_file);
#if 0
		else {
		    char *p = cfg_get_strg(cf_all,"password");
		    char *pp = cfg_get_strg(cf_options,"password");
		    
		    if ( ((p && *p) || (pp && *pp))  &&
			     (st.st_mode & (S_IRGRP | S_IROTH)) )
			fprintf(errstd,"Warning: %s should be readable only "
			  "for root if using PASSWORD\n",config_file);
		}
#else
	    config_read = !!(st.st_mode & (S_IRGRP | S_IROTH));
#endif
	}
    }
    preload_dev_cache();
    
    compact = cfg_get_flag(cf_options,"compact");
    geom = cfg_get_flag(cf_options,"geometric");
    linear = cfg_get_flag(cf_options,"linear");
    lba32  = cfg_get_flag(cf_options,"lba32");
    nowarn = cfg_get_flag(cf_options,"nowarn");
    if (geom+linear+lba32 > 1)
	die("May specify only one of GEOMETRIC, LINEAR or LBA32");
    if (geometric) {
	if (!nowarn && (geom+linear+lba32 > 0))  
	    fprintf(errstd,"Ignoring entry '%s'\n", geom ? "geometric" :
	    	linear ? "linear" : "lba32");
	geom = linear = lba32 = 0;
	if (geometric==4) lba32 = 1;
	else if (geometric==2) linear = 1;
	else if (geometric==1) geom = 1;
    }    
#ifdef LCF_LBA32
    if (geom+linear+lba32 == 0) {
	if (!nowarn) fprintf(errstd,"Warning: LBA32 addressing assumed\n");
	lba32 = 1;
    }
#endif
    if (verbose<0 && cfg_get_strg(cf_options,"verbose"))
	verbose = to_number(cfg_get_strg(cf_options,"verbose"));
    if (verbose<0) verbose = 0;
    
    if (identify) identify_image(identify,ident_opt);

/* test for a RAID installation */
    raid_offset = raid_setup();
    if (verbose >= 2)
        printf("raid_setup returns offset = %08lX\n", raid_offset);
	    
	if (uninstall)
	    bsect_uninstall(uninst_dev ? uninst_dev : cfg_get_strg(cf_options,
	      "boot"),cfg_get_strg(cf_options,"backup"),validate);
	if (!nowarn && compact && (linear || lba32))
	    fprintf(errstd,"Warning: COMPACT may conflict with %s on some "
		"systems\n", lba32 ? "LBA32" : "LINEAR");
	if (reboot_arg) {
	    map_patch_first(cfg_get_strg(cf_options,"map") ? cfg_get_strg(
	      cf_options,"map") : MAP_FILE, reboot_arg);
	    sync();
	    exit(0);
	}
	if (argc) usage(name);
	geo_init(cfg_get_strg(cf_options,"disktab"));
	if (query)
	    show_images(!cfg_get_strg(cf_options,"map") ? MAP_FILE :
	      cfg_get_strg(cf_options,"map"));

	if (verbose >=2 && do_md_install)
	    printf("raid flags: at bsect_open  0x%02X\n", raid_flags);

	bsect_open(cfg_get_strg(cf_options,"boot"),cfg_get_strg(cf_options,"map") ?
	  cfg_get_strg(cf_options,"map") : MAP_FILE,cfg_get_strg(cf_options,
	  "install"),cfg_get_strg(cf_options,"delay") ? to_number(cfg_get_strg(
	  cf_options,"delay")) : 0,cfg_get_strg(cf_options,"timeout") ?
	  to_number(cfg_get_strg(cf_options,"timeout")) : -1, raid_offset);
	if (more) {
	    cfg_init(cf_top);
	    if (cfg_parse(cf_top)) cfg_error("Syntax error");
	}
	if (!bsect_number())
	    die("No images have been defined or default image doesn't exist.");
	check_fallback();
	
	if (do_md_install) raid_final();
	else if (!test)
	    if (cfg_get_strg(cf_options,"force-backup"))
		bsect_update(cfg_get_strg(cf_options,"force-backup"),1,0);
	    else bsect_update(cfg_get_strg(cf_options,"backup"),0,0);
	else {
	    bsect_cancel();
	    if (passw)
	        fprintf(errstd,"The password crc file has *NOT* been updated.\n");
	    fprintf(errstd,"The boot sector and the map file have *NOT* been "
	      "altered.\n");
	}

	
    return 0;
}
