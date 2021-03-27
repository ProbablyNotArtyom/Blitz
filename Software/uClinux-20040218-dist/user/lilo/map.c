/* map.c  -  Map file creation */
/*
Copyright 1992-1998 Werner Almesberger.
Copyright 1999-2001 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

*/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "lilo.h"
#include "common.h"
#include "geometry.h"
#include "map.h"


typedef struct _map_entry {
    SECTOR_ADDR addr;
    struct _map_entry *next;
} MAP_ENTRY;


static MAP_ENTRY *map,*last;
static GEOMETRY map_geo;
static SECTOR_ADDR zero_addr;
static int map_file;


void map_patch_first(char *name,char *str)
{
    DESCR_SECTORS descrs;
    int fd,size,image;
    unsigned short magic;
    char *start,*end;

    if (strlen(str) >= SECTOR_SIZE-2)
	die("map_patch_first: String is too long");
    if ((fd = open(name,O_RDWR)) < 0) die("open %s: %s",name,strerror(errno));
    if (lseek(fd,SECTOR_SIZE,0) < 0) die("lseek %s: %s",name,strerror(errno));
    if (read(fd,(char *) &descrs,sizeof(descrs)) != sizeof(descrs))
	die("read %s: %s",name,strerror(errno));
    for (start = str; *start && *start == ' '; start++);
    if (*start) {
	for (end = start; *end && *end != ' '; end++);
	if (*end) *end = 0;
	else end = NULL;
	for (image = 0; image < MAX_IMAGES; image++)
#ifdef LCF_IGNORECASE
	    if (!strcasecmp(descrs.d.descr[image].name,start)) break;
#else
	    if (!strcmp(descrs.d.descr[image].name,start)) break;
#endif
	if (image == MAX_IMAGES) die("No image \"%s\" is defined",start);
	if (end) *end = ' ';
    }
    if (lseek(fd,0L,0) < 0) die("lseek %s: %s",name,strerror(errno));
    magic = *str ? DC_MAGIC : 0;
    if ((size = write(fd,(char *) &magic,2)) < 0)
	die("write %s: %s",name,strerror(errno));
    if (size != 2) die("map_patch_first: Bad write ?!?");
    if ((size = write(fd,str,strlen(str)+1)) < 0)
	die("write %s: %s",name,strerror(errno));
    if (size != strlen(str)+1) die("map_patch_first: Bad write ?!?");
    if (close(fd) < 0) die("close %s: %s",name,strerror(errno));
}


void map_create(char *name)
{
    char buffer[SECTOR_SIZE*(MAX_DESCR_SECS+2)];
    int fd;

    if ((fd = creat(name,0600)) < 0) die("creat %s: %s",name,strerror(errno));
    (void) close(fd);
    memset(buffer,0,SECTOR_SIZE*(MAX_DESCR_SECS+2));
    *(unsigned short *) buffer = DC_MGOFF;
    map_file = geo_open(&map_geo,name,O_RDWR);
    if (do_md_install) {
	struct stat st;
	if(fstat(map_file,&st)) die("map_create: cannot fstat map file");
	if (verbose >= 2)
	    printf("map_create:  boot=%04X  map=%04X\n", 
					boot_dev_nr, (int)st.st_dev);
	if (boot_dev_nr != st.st_dev) {
#if 1
	    die("map file must be on the boot RAID partition");
#else
	    raid_flags |= FLAG_RAID_NOWRITE;
	    fprintf(errstd,
	    	"Error: map file must be on the boot RAID partition\n"
	    	"    This installation provides no redundancy\n"	);
#endif
	}
    }
    if (write(map_file,buffer,SECTOR_SIZE*(MAX_DESCR_SECS+2)) != SECTOR_SIZE*(MAX_DESCR_SECS+2))
	die("write %s: %s",name,strerror(errno));
    if (!geo_comp_addr(&map_geo,SECTOR_SIZE*(MAX_DESCR_SECS+1),&zero_addr))
	die("Hole found in map file (zero sector)");
}


void map_done(DESCR_SECTORS *descr,SECTOR_ADDR* addr)
{
    struct stat st;
    int i, pos;

    descr->l.checksum =
                crc32(descr->sector, sizeof((*descr).l.sector), CRC_POLY1);

    if (lseek(map_file,SECTOR_SIZE,0) < 0) pdie("lseek map file");
    if (write(map_file,(char *) descr,SECTOR_SIZE*MAX_DESCR_SECS) != SECTOR_SIZE*MAX_DESCR_SECS)
	pdie("write map file");
#if 0
    if (!geo_comp_addr(&map_geo,SECTOR_SIZE,&addr[0]))
	die("Hole found in map file (first descr. sector)");
    if (!geo_comp_addr(&map_geo,SECTOR_SIZE*2,&addr[1]))
	die("Hole found in map file (second descr. sector)");
    if (!geo_comp_addr(&map_geo,0,&addr[2]))
	die("Hole found in map file (default command line)");
#else
    pos = SECTOR_SIZE;
    for (i=0; i<MAX_DESCR_SECS; i++) {
	if (!geo_comp_addr(&map_geo,pos,addr))
	    die("Hole found in map file (descr. sector %d)", i);
	addr++;
	pos += SECTOR_SIZE;
    }
    if (!geo_comp_addr(&map_geo,0,addr))
	die("Hole found in map file (default command line)");
#endif
    if (verbose >= 2) {
	if (fstat(map_file,&st) < 0) pdie("fstat map file");
	printf("Map file size: %ld bytes.\n",(long) st.st_size);
    }
    geo_close(&map_geo);
}


void map_register(SECTOR_ADDR *addr)
{
    MAP_ENTRY *new;

    new = alloc_t(MAP_ENTRY);
    new->addr = *addr;
    new->next = NULL;
    if (last) last->next = new;
    else map = new;
    last = new;
}


void map_add_sector(void *sector)
{
    int here;
    SECTOR_ADDR addr;

    if ((here = lseek(map_file,0L,1)) < 0) pdie("lseek map file");
    if (write(map_file,sector,SECTOR_SIZE) != SECTOR_SIZE)
	pdie("write map file");
    if (!geo_comp_addr(&map_geo,here,&addr))
	die("Hole found in map file (app. sector)");
    map_register(&addr);
}


void map_begin_section(void)
{
    map = last = NULL;
}


void map_add(GEOMETRY *geo,int from,int num_sect)
{
    int count;
    SECTOR_ADDR addr;

    for (count = 0; count < num_sect; count++) {
	if (geo_comp_addr(geo,SECTOR_SIZE*(count+from),&addr))
	    map_register(&addr);
	else {
	    map_register(&zero_addr);
	    if (verbose > 3) printf("Covering hole at sector %d.\n",count);
	}
    }
}


void map_add_zero(void)
{
    map_register(&zero_addr);
}


static void map_compact(int dont_compact)
{
    MAP_ENTRY *walk,*next;
    int count, removed, offset, adj, hinib, noffset, maxcount;

    removed = 0;
    hinib = 257;
    maxcount = lba32 ? 127 : 128;  /* JRC: max LBA transfer is 127 sectors,
	      per the EDD spec, v1.1, not 128 (unfortunately) */
/* JRC: for testing the hinib save: */
#ifdef DEBUG
	maxcount = lba32 ? 3 : maxcount;
#endif
    walk = map;
    for (count = 0; walk && count <= dont_compact; count++) walk = walk->next;
    offset = 0;
    noffset = 0;
    while (walk && walk->next) {
	adj = ((walk->addr.device ^ walk->next->addr.device) & ~LBA32_NOCOUNT) == 0;
        if (adj && (walk->addr.device & LBA32_FLAG)) {
	    if ((walk->addr.device & LBA32_NOCOUNT)==0) {
	        if ( (adj = (hinib==walk->next->addr.num_sect)) ) {
		    walk->next->addr.num_sect = 1;
		    walk->next->addr.device &= ~LBA32_NOCOUNT;
		}
	    }
	    else {
	        adj = 0;
	        hinib = walk->addr.num_sect;
	        if ((walk->next->addr.device&LBA32_NOCOUNT) &&
		    (walk->next->addr.num_sect == hinib)) {
		   walk->next->addr.num_sect = 1;
		   walk->next->addr.device &= ~LBA32_NOCOUNT;
		}
	    }
	}
	if (adj && walk->addr.device & (LINEAR_FLAG|LBA32_FLAG))
	    adj = ((walk->addr.head << 16) | (walk->addr.track << 8) |
	      walk->addr.sector)+walk->addr.num_sect == ((walk->next->addr.head
	      << 16) | (walk->next->addr.track << 8) | walk->next->addr.sector);
	else adj = adj && walk->addr.track == walk->next->addr.track &&
	      walk->addr.head == walk->next->addr.head &&
	      walk->addr.sector+walk->addr.num_sect == walk->next->addr.sector;
	noffset += SECTOR_SIZE;
	adj = adj && (offset>>16 == noffset>>16) &&
	             (walk->addr.num_sect < maxcount);    
	if (!adj) {
	    offset = noffset;
	    walk = walk->next;
	}
	else {
	    walk->addr.num_sect++;
	    next = walk->next->next;
	    free(walk->next);
	    removed++;
	    walk->next = next;
	}
    }
    if (verbose > 1)
	printf("Compaction removed %d BIOS call%s.\n",removed,removed == 1 ?
	  "" : "s");
}


static void map_alloc_page(int offset,SECTOR_ADDR *addr)
{
    int here;

    if ((here = lseek(map_file,offset,1)) < 0) pdie("lseek map file");
    if (write(map_file,"",1) != 1) pdie("write map file");
    if (fsync(map_file)) pdie("fsync map file");
    if (!geo_comp_addr(&map_geo,here,addr))
	die("Hole found in map file (alloc_page)");
    if (lseek(map_file,-offset-1,1) < 0) pdie("lseek map file");
}


int map_end_section(SECTOR_ADDR *addr,int dont_compact)
{
    int first,offset,sectors;
    char buffer[SECTOR_SIZE];
    MAP_ENTRY *walk,*next;
    int hinib;

    first = 1;
    memset(buffer,0,SECTOR_SIZE);
    offset = sectors = 0;
    if (compact) map_compact(dont_compact);
    if (!map) die("Empty map section");
    hinib = 0;
    for (walk = map; walk; walk = next) {
	next = walk->next;
	if (verbose > 3) {
	    if ((walk->addr.device&LBA32_FLAG) && (walk->addr.device&LBA32_NOCOUNT)) hinib = walk->addr.num_sect;
	    printf("  Mapped AL=0x%02x CX=0x%04x DX=0x%04x",walk->addr.num_sect,
	      (walk->addr.track << 8) | walk->addr.sector,(walk->addr.head << 8)
	      | walk->addr.device);
	    if (linear||lba32)
		printf(", %s=%d",
		  lba32 ? "LBA" : "linear",
		  (walk->addr.head << 16) | (walk->addr.track << 8) | walk->addr.sector | hinib<<24);
	    printf("\n");
	}
	if (first) {
	    first = 0;
	    map_alloc_page(0,addr);
	}
	if (offset+sizeof(SECTOR_ADDR)*2 > SECTOR_SIZE) {
	    map_alloc_page(SECTOR_SIZE,(SECTOR_ADDR *) (buffer+offset));
    	    if (write(map_file,buffer,SECTOR_SIZE) != SECTOR_SIZE)
		pdie("write map file");
	    memset(buffer,0,SECTOR_SIZE);
	    offset = 0;
	}
	memcpy(buffer+offset,&walk->addr,sizeof(SECTOR_ADDR));
	offset += sizeof(SECTOR_ADDR);
	sectors += (walk->addr.device&LBA32_FLAG) && (walk->addr.device&LBA32_NOCOUNT)
		? 1 : walk->addr.num_sect;
	free(walk);
    }
    if (offset)
	if (write(map_file,buffer,SECTOR_SIZE) != SECTOR_SIZE)
	    pdie("write map file");
    return sectors;
}


int map_write(SECTOR_ADDR *list,int max_len,int terminate)
{
    MAP_ENTRY *walk,*next;
    int sectors;

    sectors = 0;
    for (walk = map; walk; walk = next) {
	next = walk->next;
	if (--max_len < (terminate ? 1 : 0)) die("Map segment is too big.");
	*list++ = walk->addr;
	free(walk);
	sectors++;
    }
    if (terminate) memset(list,0,sizeof(SECTOR_ADDR));
    return sectors;
}
