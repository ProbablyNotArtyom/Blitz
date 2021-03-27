/* partition.c  -  Partition table handling */
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
#include <sys/types.h>
#include <linux/unistd.h>
#include <linux/fs.h>
#include <limits.h>
#include <time.h>
#include "config.h"
#include "lilo.h"
#include "common.h"
#include "cfg.h"
#include "device.h"
#include "geometry.h"
#include "partition.h"
#include "boot.h"


#if 0
       
       _syscall5(int,  _llseek,  uint,  fd, ulong, hi, ulong, lo,
       loff_t *, res, uint, wh);

       int _llseek(unsigned int fd,  unsigned  long  offset_high,
       unsigned  long  offset_low,  loff_t * result, unsigned int
       whence);
       
       loff_t llseek(unsigned int fd, loff_t offs, unsigned int whence)
       { loff_t res;
       	return _llseek(fd, offs>>32, offs, &res, whence) < 0  ?
       			 (loff_t)(-1) : res;
       }
#endif


void part_verify(int dev_nr,int type)
{
    GEOMETRY geo;
    DEVICE dev;
    char backup_file[PATH_MAX+1];
    int fd, bck_file, part, size, lin_3d, cyl;
    unsigned long second, base;
    struct partition part_table[PART_MAX];
    int mask, i, pe;
    unsigned short boot_sig;
    
    if (!has_partitions(dev_nr) || !(mask = P_MASK(dev_nr)) || !(dev_nr & mask)
#if 0
     || (dev_nr & mask) > PART_MAX
#endif
     	) return;

    if (verbose >= 4) printf("part_verify:  dev_nr=%04x, type=%d\n", dev_nr, type);
    geo_get(&geo,dev_nr & ~mask,-1,1);
    fd = dev_open(&dev,dev_nr & ~mask,cfg_get_flag(cf_options,"fix-table")
      && !test ? O_RDWR : O_RDONLY);
    part = (pe = dev_nr & mask)-1;
    if (lseek(fd, PART_TABLE_OFFSET, SEEK_SET) < 0) pdie("lseek partition table");
    if (!(size = read(fd,(char *) part_table, sizeof(struct partition)*
      PART_MAX))) die("Short read on partition table");
    if (size < 0) pdie("read partition table");
    if ( read(fd, &boot_sig, sizeof(boot_sig)) != sizeof(boot_sig)  ||
	boot_sig != BOOT_SIGNATURE ) die("read boot signature failed");

    if (verbose>=5) printf("part_verify:  part#=%d\n", pe);

    second=base=0;
    for (i=0; i<PART_MAX; i++) {
	if (is_extd_part(part_table[i].sys_ind)) {
	    if (!base) base = part_table[i].start_sect;
	    else die("invalid partition table: second extended partition found");
	}
    }
    i=5;
    while (i<=pe && base) {
        if (llseek(fd, SECTORSIZE*(base+second) + PART_TABLE_OFFSET, SEEK_SET) < 0)
            die("secondary llseek failed");
	if (read(fd, part_table, sizeof(part_table)) != sizeof(part_table)) die("secondary read pt failed");
	if ( read(fd, &boot_sig, sizeof(boot_sig)) != sizeof(boot_sig)  ||
	    boot_sig != BOOT_SIGNATURE ) die("read second boot signature failed");
        if (is_extd_part(part_table[1].sys_ind)) second=part_table[1].start_sect;
        else base = 0;
        i++;
        part=0;
    }

    if (type && part_table[part].sys_ind != PART_LINUX_MINIX &&
	      part_table[part].sys_ind != PART_LINUX_NATIVE &&
	      !is_extd_part(part_table[part].sys_ind) ) {
	fflush(stdout);
	fprintf(errstd,"Device 0x%04X: Partition type 0x%02X does not seem "
	    "suitable for a LILO boot sector\n",dev_nr,part_table[part].sys_ind);
	if (!cfg_get_flag(cf_options,"ignore-table")) exit(1);
    }
    cyl = part_table[part].cyl+((part_table[part].sector >> 6) << 8);
    lin_3d = (part_table[part].sector & 63)-1+(part_table[part].head+
      cyl*geo.heads)*geo.sectors;
    if (pe <= PART_MAX &&
	    (lin_3d > part_table[part].start_sect || (lin_3d <
	    part_table[part].start_sect && cyl != BIOS_MAX_CYLS-1)) &&
	    !nowarn) {
	fflush(stdout);
	fprintf(errstd,"Device 0x%04X: Invalid partition table, %d%s entry\n",
	  dev_nr & ~mask,part+1,!part ? "st" : part == 1 ? "nd" : part ==
	  2 ? "rd" : "th");
	fprintf(errstd,"  3D address:     %d/%d/%d (%d)\n",part_table[part].
	  sector & 63,part_table[part].head,cyl,lin_3d);
	cyl = part_table[part].start_sect/geo.sectors/geo.heads;
	fprintf(errstd,"  Linear address: %d/%d/%d (%d)\n",part_table[part].
	  sector = (part_table[part].start_sect % geo.sectors)+1,part_table
	  [part].head = (part_table[part].start_sect/geo.sectors) %
	  geo.heads,cyl,part_table[part].start_sect);
	part_table[part].sector |= cyl >> 8;
	part_table[part].cyl = cyl & 0xff;
	if (!cfg_get_flag(cf_options,"fix-table") && !cfg_get_flag(cf_options,
	  "ignore-table")) exit(1);
	if (test || cfg_get_flag(cf_options,"ignore-table"))
	    fprintf(errstd,"The partition table is *NOT* being adjusted.\n");
	else {
	    sprintf(backup_file,BACKUP_DIR "/part.%04X",dev_nr & ~mask);
	    if ((bck_file = creat(backup_file,0644)) < 0)
		die("creat %s: %s",backup_file,strerror(errno));
	    if (!(size = write(bck_file,(char *) part_table,
	      sizeof(struct partition)*PART_MAX)))
		die("Short write on %s",backup_file);
	    if (size < 0) pdie(backup_file);
	    if (close(bck_file) < 0)
		die("close %s: %s",backup_file,strerror(errno));
	    if (verbose > 0)
		printf("Backup copy of partition table in %s\n",backup_file);
	    printf("Writing modified partition table to device 0x%04X\n",
	      dev_nr & ~mask);
	    if (lseek(fd,PART_TABLE_OFFSET,0) < 0)
		pdie("lseek partition table");
	    if (!(size = write(fd,(char *) part_table,sizeof(struct partition)*
	      PART_MAX))) die("Short write on partition table");
	    if (size < 0) pdie("write partition table");
	}
    }
    dev_close(&dev);
}


CHANGE_RULE *change_rules = NULL;


void do_cr_reset(void)
{
    CHANGE_RULE *next;

    while (change_rules) {
	next = change_rules->next;
	free((char *) change_rules->type);
	free(change_rules);
	change_rules = next;
    }
}


static unsigned char cvt_byte(const char *s)
{
    char *end;
    unsigned long value;

    value = strtoul(s,&end,0);
    if (value > 255 || *end) cfg_error("\"%s\" is not a byte value",s);
    return value;
}


static void add_type(const char *type,int normal,int hidden)
{
    CHANGE_RULE *rule;

    for (rule = change_rules; rule; rule = rule->next)
	if (!strcasecmp(rule->type,type))
	    die("Duplicate type name: \"%s\"",type);
    rule = alloc_t(CHANGE_RULE);
    rule->type = stralloc(type);
    rule->normal = normal == -1 ? hidden ^ HIDDEN_OFF : normal;
    rule->hidden = hidden == -1 ? normal ^ HIDDEN_OFF : hidden;
    rule->next = change_rules;
    change_rules = rule;
}


void do_cr_type(void)
{
    const char *normal,*hidden;

    cfg_init(cf_change_rule);
    (void) cfg_parse(cf_change_rule);
    normal = cfg_get_strg(cf_change_rule,"normal");
    hidden = cfg_get_strg(cf_change_rule,"hidden");
    if (normal)
	add_type(cfg_get_strg(cf_change_rules,"type"),cvt_byte(normal),
	  hidden ? cvt_byte(hidden) : -1);
    else {
	if (!hidden)
	    cfg_error("At least one of NORMAL and HIDDEN must be present");
	add_type(cfg_get_strg(cf_change_rules,"type"),cvt_byte(hidden),-1);
    }
    cfg_unset(cf_change_rules,"type");
}


void do_cr(void)
{
    cfg_init(cf_change_rules);
    (void) cfg_parse(cf_change_rules);
}


#if defined(LCF_REWRITE_TABLE) && !defined(LCF_READONLY)

/*
 * Rule format:
 *
 * +------+------+------+------+
 * |drive |offset|expect| set  |
 * +------+------+------+------+
 *     0      1      2      3
 */

static void add_rule(unsigned char bios,unsigned char offset,
  unsigned char expect,unsigned char set)
{
    int i;

    if (curr_prt_map == PRTMAP_SIZE)
	cfg_error("Too many change rules (more than %s)",PRTMAP_SIZE);
    if (verbose >= 3)
	printf("  Adding rule: disk 0x%02x, offset 0x%x, 0x%02x -> 0x%02x\n",
	    bios,PART_TABLE_OFFSET+offset,expect,set);
    prt_map[curr_prt_map] = (set << 24) | (expect << 16) | (offset << 8) | bios;
    for (i = 0; i < curr_prt_map; i++) {
	if (prt_map[i] == prt_map[curr_prt_map])
	  die("Repeated rule: disk 0x%02x, offset 0x%x, 0x%02x -> 0x%02x",
	    bios,PART_TABLE_OFFSET+offset,expect,set);
	if ((prt_map[i] & 0xffff) == ((offset << 8) | bios) &&
	  (prt_map[i] >> 24) == expect)
	    die("Redundant rule: disk 0x%02x, offset 0x%x: 0x%02x -> 0x%02x "
	      "-> 0x%02x",bios,PART_TABLE_OFFSET+offset,
	     (prt_map[i] >> 16) & 0xff,expect,set);
    }
    curr_prt_map++;
}

#endif


static int has_partition;

static CHANGE_RULE *may_change(unsigned char sys_ind)
{
    CHANGE_RULE *cr = change_rules;
    
    while (cr) {
        if (cr->normal == sys_ind || cr->hidden == sys_ind) return cr;
        cr = cr->next;
    }
    return NULL;
}


void do_cr_auto(void)
{
    GEOMETRY geo;
    struct stat st;
    char *table, *table2, *other;
    int partition, pfd, i, j;
    struct partition part_table[PART_MAX];

    if (autoauto) has_partition = 0;
    other = cfg_get_strg(cf_top, "other");
    if (verbose > 4) printf("do_cr_auto: other=%s has_partition=%d\n",
        other, has_partition);
#if 0
    i = other[strlen(other)-1] - '0';
    if (i>PART_MAX || i<1) return;
#endif
    table = cfg_get_strg(cf_other,"table");
    table2 = boot_mbr(other, 1);	/* get possible default */
    if (!table) table = table2;
    
    if (!table && autoauto) return;
    if (table && autoauto && !table2) cfg_error("TABLE may not be specified");
   
    if (has_partition) cfg_error("AUTOMATIC must be before PARTITION");
    if (!table) cfg_error("TABLE must be set to use AUTOMATIC");
    /*    
     */
    if (stat(table,&st) < 0) die("stat %s: %s",table,strerror(errno));
    geo_get(&geo,st.st_rdev & D_MASK(st.st_rdev),-1,1);
    partition = st.st_rdev & P_MASK(st.st_rdev);
    if (!S_ISBLK(st.st_mode) || partition)
	cfg_error("\"%s\" doesn't contain a primary partition table",table);
    pfd = open(table, O_RDONLY);
    if (pfd<0) die("Cannot open %s", table);
    if (lseek(pfd, PART_TABLE_OFFSET, SEEK_SET)!=PART_TABLE_OFFSET)
	die("Cannot seek to partition table of %s", table);
    if (read(pfd, part_table, sizeof(part_table))!=sizeof(part_table))
	die("Cannot read Partition Table of %s", table);
    close(pfd);
    partition = other[strlen(other)-1] - '0';
    if (verbose > 3) printf("partition = %d\n", partition);
    for (j=i=0; i<PART_MAX; i++)
	if (may_change(part_table[i].sys_ind)) j++;

    if (j>1)
#if defined(LCF_REWRITE_TABLE) && !defined(LCF_READONLY)
    for (i=0; i<PART_MAX; i++) {
    	CHANGE_RULE *cr;
	if ((cr=may_change(part_table[i].sys_ind))) {
	    j = i*PARTITION_ENTRY + PART_TYPE_ENT_OFF;
	    if (autoauto && !nowarn) {
	    	fflush(stdout);
	        fprintf(errstd, "Warning: CHANGE AUTOMATIC assumed after \"other=%s\"\n", other);
	        autoauto = 0;  /* suppress further warnings */
	    }    
	    if (i == partition-1)
		add_rule(geo.device, j, cr->hidden, cr->normal);
	    else
		add_rule(geo.device, j, cr->normal, cr->hidden);
	}
    }
#else
    if (!nowarn) fprintf(errstd,
       "Warning:  This LILO is compiled without REWRITE_TABLE;\n"
       "   unable to generate CHANGE/AUTOMATIC change-rules\n");
#endif
}



void do_cr_part(void)
{
    GEOMETRY geo;
    struct stat st;
    char *tmp;
    int partition,part_base;

    tmp = cfg_get_strg(cf_change,"partition");
    if (stat(tmp,&st) < 0) die("stat %s: %s",tmp,strerror(errno));
    geo_get(&geo,st.st_rdev & D_MASK(st.st_rdev),-1,1);
    partition = st.st_rdev & P_MASK(st.st_rdev);
    if (!S_ISBLK(st.st_mode) || !partition || partition > PART_MAX)
	cfg_error("\"%s\" isn't a primary partition",tmp);
    part_base = (partition-1)*PARTITION_ENTRY;
    has_partition = 1;   
    cfg_init(cf_change_dsc);
    (void) cfg_parse(cf_change_dsc);
    tmp = cfg_get_strg(cf_change_dsc,"set");
    if (tmp) {
#if defined(LCF_REWRITE_TABLE) && !defined(LCF_READONLY)
	CHANGE_RULE *walk;
	char *here;
	int hidden;

	here = (void*)NULL;	/* quiet GCC */
	hidden = 0;		/* quiet GCC */
	if (strlen(tmp) < 7 || !(here = strrchr(tmp,'_')) ||
	  ((hidden = strcasecmp(here+1,"normal")) &&
	  strcasecmp(here+1,"hidden")))
	    cfg_error("Type name must end with _normal or _hidden");
	*here = 0;
	for (walk = change_rules; walk; walk = walk->next)
	    if (!strcasecmp(walk->type,tmp)) break;
	if (!walk) cfg_error("Unrecognized type name");
	add_rule(geo.device,part_base+PART_TYPE_ENT_OFF,hidden ? walk->normal :
	  walk->hidden,hidden ? walk->hidden : walk->normal);
#else
	die("This LILO is compiled without REWRITE_TABLE and doesn't support "
	  "the SET option");
#endif
    }
    if (cfg_get_flag(cf_change_dsc,"activate")) {
#if defined(LCF_REWRITE_TABLE) && !defined(LCF_READONLY)
	add_rule(geo.device,part_base+PART_ACT_ENT_OFF,0x00,0x80);
	if (cfg_get_flag(cf_change_dsc,"deactivate"))
	    cfg_error("ACTIVATE and DEACTIVATE are incompatible");
#else
	die("This LILO is compiled without REWRITE_TABLE and doesn't support "
	  "the ACTIVATE option");
#endif
    }
    if (cfg_get_flag(cf_change_dsc,"deactivate"))
#if defined(LCF_REWRITE_TABLE) && !defined(LCF_READONLY)
	add_rule(geo.device,part_base+PART_ACT_ENT_OFF,0x80,0x00);
#else
	die("This LILO is compiled without REWRITE_TABLE and doesn't support "
	  "the DEACTIVATE option");
#endif
    cfg_unset(cf_change,"partition");
}


void do_change(void)
{
    cfg_init(cf_change);
    has_partition = 0;
    (void) cfg_parse(cf_change);
}


void preload_types(void)
{
#if 0 /* don't know if it makes sense to add these too */
    add_type("Netware", 0x64, 0x74);
    add_type("OS2_BM", 0x0a, 0x1a);
#endif
    add_type("OS2_HPFS", 0x07, 0x17);

    add_type("FAT16_lba", PART_FAT16_LBA, -1);
    add_type("FAT32_lba", PART_FAT32_LBA, -1);
    add_type("FAT32", PART_FAT32, -1);
    add_type("NTFS", PART_NTFS, -1);
    add_type("DOS16_big", PART_DOS16_BIG, -1);
    add_type("DOS16_small", PART_DOS16_SMALL, -1);
    add_type("DOS12", PART_DOS12, -1);
}



#define PART_BEGIN	0x1be
#define PART_NUM	4
#define PART_SIZE	16
#define PART_ACTIVE	0x80
#define PART_INACTIVE	0


void do_activate(char *where, char *which)
{
    struct stat st;
    int fd,number,count;
    unsigned char flag, ptype;

    if ((fd = open(where, !which ? O_RDONLY : O_RDWR)) < 0)
	die("open %s: %s",where,strerror(errno));
    if (fstat(fd,&st) < 0) die("stat %s: %s",where,strerror(errno));
    if (!S_ISBLK(st.st_mode)) die("%s: not a block device",where);
    if (verbose >= 1) {
       printf("st.st_dev = %04X, st.st_rdev = %04X\n",
       				(int)st.st_dev, (int)st.st_rdev);
    }
    if ((st.st_rdev & has_partitions(st.st_rdev)) != st.st_rdev)
        die("%s is not a master device with a primary partition table", where);
    if (!which) {	/* one argument: display active partition */
	for (count = 1; count <= PART_NUM; count++) {
	    if (lseek(fd,PART_BEGIN+(count-1)*PART_SIZE,0) < 0)
		die("lseek: %s",strerror(errno));
	    if (read(fd,&flag,1) != 1) die("read: %s",strerror(errno));
	    if (flag == PART_ACTIVE) {
		printf("%s%d\n",where,count);
		exit(0);
	    }
	}
	die("No active partition found on %s",where);
    }
    number = to_number(which);
    if (number < 1 || number > 4)
	die("%s: not a valid partition number (1-4)",which);
    for (count = 1; count <= PART_NUM; count++) {
	if (lseek(fd,PART_BEGIN+(count-1)*PART_SIZE+4,0) < 0)
	    die("lseek: %s",strerror(errno));
	if (read(fd,&ptype,1) != 1) die("read: %s",strerror(errno));
	if (count == number && ptype==0) die("Cannot activate an empty partition");
    }
    if (test) {
        printf("The partition table of  %s  has *NOT* been updated\n",where);
    }
    else for (count = 1; count <= PART_NUM; count++) {
	if (lseek(fd,PART_BEGIN+(count-1)*PART_SIZE,0) < 0)
	    die("lseek: %s",strerror(errno));
	flag = count == number ? PART_ACTIVE : PART_INACTIVE;
	if (write(fd,&flag,1) != 1) die("write: %s",strerror(errno));
    }
    exit(0);
}


void do_install_mbr(char *where, char *what)
{
    int fd, nfd, i;
    struct stat st;
    char buf[SECTOR_SIZE];
    
    if (!what) what = DFL_MBR;
    if ((fd=open(where,O_RDWR)) < 0) die("Cannot open %s: %s", where,strerror(errno));
    if (fstat(fd,&st) < 0) die("stat: %s : %s", where,strerror(errno));
    if (!S_ISBLK(st.st_mode)) die("%s not a block device",where);
    if (st.st_rdev != (st.st_rdev & has_partitions(st.st_rdev)))
	die("%s is not a master device with a primary parition table",where);
    if (read(fd,buf,SECTOR_SIZE) != SECTOR_SIZE) die("read %s: %s",where, strerror(errno));
    
    if ((nfd=open(what,O_RDONLY)) < 0) die("Cannot open %s: %s",what,strerror(errno));
    if (read(nfd,buf,MAX_BOOT_SIZE) != MAX_BOOT_SIZE) die("read %s: %s",what,strerror(errno));
    
    *(unsigned short*)&buf[BOOT_SIG_OFFSET] = BOOT_SIGNATURE;
    if (zflag) {
        char *p = buf+MAX_BOOT_SIZE;
        for (i=0; i<8; i++) *p++ = 0;
    } else if (*(int*)&buf[PART_TABLE_OFFSET - 6] == 0) {
    	i = st.st_rdev;
    	i %= 271;		/* modulo a prime number; eg, 2551, 9091 */
        srand(time(NULL));	/* seed the random number generator */
        while (i--) rand();
        *(int*)&buf[PART_TABLE_OFFSET - 6] = rand();  /* insert serial number */
        if (*(short*)&buf[PART_TABLE_OFFSET - 2] == 0)
            *(short*)&buf[PART_TABLE_OFFSET - 2] = MAGIC_SERIAL;
    }
    
    if (lseek(fd,0,SEEK_SET) != 0) die("seek %s; %s", where, strerror(errno));
    if (!test) {
	if (write(fd,buf,SECTOR_SIZE) != SECTOR_SIZE)
		die("write %s: %s",where,strerror(errno));
    }
    close(fd); close(nfd);
    printf("The Master Boot Record of  %s  has %sbeen updated.\n", where, test ? "*NOT* " : "");
    sync();
    exit(0);
}

