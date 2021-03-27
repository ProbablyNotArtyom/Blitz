/* probe.c -- BIOS probes */
/*
Copyright 1999-2001 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

*/

/*#define DEBUG*/
#define VIDEO 1

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
#include "common.h"
#include "device.h"
#include "geometry.h"
#include "partition.h"
#include "bdata.h"
#include "probe.h"
/*#include "lrmi.h"*/

#ifdef LCF_BDATA
#if BD_MAX_FLOPPY > 4
#error "too many floppies in  bdata.h"
#endif
#if BD_MAX_HARD > 16
#error "too many hard disks in  bdata.h"
#endif
#if BD_GET_VIDEO > 3
#error "video get level set too high in  bdata.h"
#endif
#endif


static union Buf {
   unsigned char b[4*SECTOR_SIZE];
   struct {
      short checksum[2];	/* prevent alignment on *4 boundary */
      char signature[4];
      short version;
      short length;
      unsigned char disk;	/* device code of last good disk */
      unsigned char vid, mflp, mhrd;
      short floppy;		/* byte offset to floppy data    */
      short hard;		/* byte offset to hard disk data */
      short partitions;		/* byte offset to partition info */
      video_t v;
      floppy_t f[4];
      hard_t d;
/*      edd_t edd; */
   } s;
} buf;
static int buf_valid = 0;
static hard_t *hdp[16+1] =		/* pointers to all the hard disks */
	{	NULL, NULL, NULL, NULL, 
		NULL, NULL, NULL, NULL, 
		NULL, NULL, NULL, NULL, 
		NULL, NULL, NULL, NULL,  NULL	};
static char warned[16];

static void do_ebda(void);
static void do_cr_pr(void);
static void do_help(void);
static void do_geom(char *bios);
static void do_geom_all(void);
static void do_table(char *part);
#if VIDEO
static void do_video(void);
#endif

static char dev[] = "<device>";

extern CHANGE_RULE *change_rules;	/* defined in partition.c */


#if 0   /* def LCF_ALL_PARTITIONS */

#define SECTORSIZE ((long long)SECTOR_SIZE)

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


static
struct Probes {
	char *cmd;
	void (*prc)();
	char *str;
	char *help;
	}
	list[] = {
{ "help",  do_help,  NULL,  "Print list of -T(ell) options"	},
{ "ChRul", do_cr_pr, NULL,  "List partition change-rules"  },
{ "EBDA",  do_ebda,  NULL,  "Extended BIOS Data Area information" },
{ "geom=", do_geom,  "<bios>", "Geometry CHS data for BIOS code 0x80, etc." },
{ "geom" , do_geom_all, NULL, "Geometry for all BIOS drives" },
{ "table=", do_table, dev,   "Partition table information for /dev/hda, etc." },
#if VIDEO
{ "video", do_video, NULL,  "Graphic mode information" },
#endif
{ NULL,    NULL,     NULL,   NULL}
	};


static struct partitions {
	char *name;
	unsigned char type;
	unsigned char hide;
	} ptab [] = {		/* Not complete, by any means */

    { "DOS12", PART_DOS12, HIDDEN_OFF },
    { "DOS16_small", PART_DOS16_SMALL, HIDDEN_OFF },
    { "DOS16_big", PART_DOS16_BIG, HIDDEN_OFF },
    { "NTFS or OS2_HPFS", PART_NTFS, HIDDEN_OFF },	/* same as HPFS; keep these two together */
/*  { "HPFS", PART_HPFS, HIDDEN_OFF },	*/	/* same as NTFS */
    { "FAT32", PART_FAT32, HIDDEN_OFF },
    { "FAT32_lba", PART_FAT32_LBA, HIDDEN_OFF },
    { "FAT16_lba", PART_FAT16_LBA, HIDDEN_OFF },
    { "OS/2 BootMgr", PART_OS2_BOOTMGR, 0 },
    { "DOS extended", PART_DOS_EXTD, 0 },
    { "WIN extended", PART_WIN_EXTD_LBA, 0 },
    { "Linux ext'd", PART_LINUX_EXTD, 0 },
    { "Linux Swap", PART_LINUX_SWAP, 0 },
    { "Linux Native", PART_LINUX_NATIVE, 0 },
    { "Minix", PART_LINUX_MINIX, 0 },
    { "Linux RAID", 0xfd, 0 },
    { NULL, 0, 0 }   };

static char phead[] = "\t\t Type  Boot      Start           End      Sector    #sectors";


/* load the low memory bios data area */
/*  0 = no error, !0 = error on get */
static int fetch(void)
{
    int fd;
    int got, get;
    int at = 0, seek = PROBESEG*16;
    
    if (buf_valid) return 0;
    
    if ((fd=open("/dev/mem", O_RDONLY)) < 0) return 1;
#if 0
    while (at<seek) {
	get = seek - at > sizeof(buf.b) ? sizeof(buf.b) : seek - at;
	got = read(fd, &buf.b, get);
	if (got != get) return 1;
	at += got;
    }
#else
    at = lseek(fd, seek, SEEK_SET);
    if (at != seek) return 1;
#endif
    get = sizeof(buf.b);
    if (read(fd, &buf.b, get) != get) return 1;
    close(fd);
    
    if (strncmp(buf.s.signature, PROBE_SIGNATURE,4)) return 2;
/*    got = buf.s.version; */	/* quiet GCC */
    if (buf.s.version < 3 ||
        buf.s.version > (short)(PROBE_VERSION)) return 3;
    got = buf.s.length;
    if (got > sizeof(buf.b) || got < sizeof(buf.s)) return 4;
    if (*(long*)buf.s.checksum != crc32((char*)&buf.s + 4, got-4, CRC_POLY1))
    	return 5;
    buf_valid = 1;
    
    return 0;
}



/* print out the help page for -T flag */
static void do_help(void)
{
    struct Probes *pr;
    
    printf("usage:");
    for (pr=list; pr->cmd; pr++) {
    	printf("\tlilo -T %s%s\t%s\n", 
    			pr->cmd, 
    			pr->str ? pr->str : "        ",
    			pr->help);
    }
#ifdef DEBUG
    printf("    In some cases, the '-v' flag will produce more output.\n");
    printf("sizeof(video_t) = %d  sizeof(floppy_t) = %d  sizeof(hard_t) = %d\n"
           "sizeof(edd_t) = %d  sizeof(buf.s) = %d\n",
            sizeof(video_t),  sizeof(floppy_t), sizeof(hard_t), sizeof(edd_t),
            sizeof(buf.s) );
	
    printf("fetch returns %d\n", fetch());            
#endif
}


/* get the old BIOS disk geometry */
static int get_geom(unsigned int drive, struct disk_geom *geom)
{
    hard_t *hd;
    floppy_t *fd;
    int i;
    struct partition *pt_base;

    if((i=fetch())) {
        printf("No drive geometry information is available.\n\n");
        exit(0);
    }

#ifdef DEBUG
	printf("get_geom: drive = 0x%02X\n", drive);
	fflush(stdout);
#endif
    if (drive >= 0 && drive < buf.s.mflp) {
	fd = (floppy_t*)&buf.b[buf.s.floppy] + drive;
	hd = (hard_t*)fd;
    }
    else if (drive == 0x80) {
	hdp[drive-0x80] = hd = (hard_t*)&buf.b[buf.s.hard];
    }    
    else if (drive >= 0x81 && drive < 0x80+buf.s.mhrd) {
	if (drive > buf.s.disk) return 1;
	if (!hdp[drive-0x80]) {
	    i = get_geom(drive-1, geom);
#ifdef DEBUG
		printf("get_geom recursive return = %d  AH=0x%02X\n", i, i-1);
		fflush(stdout);
#endif
	    if (i) return i;
	}
	hd = hdp[drive-0x80];
    } else return 1;
#ifdef DEBUG
	printf("get_geom:  hd = %08X\n", (int)hd);
	fflush(stdout);
#endif
    
    memset(geom, 0, sizeof(*geom));


    if (drive >= 0x80)
        hdp[drive-0x80 + 1] = (void*)hd + sizeof(hard_t);		/* simplest increment, but may be wrong */
    
    /* regs.eax = 0x1500;           check drive type */
    /* regs.edx = drive;			*/

#ifdef DEBUG
	printf("get_geom: int13, fn=15\n");
	fflush(stdout);
#endif
   
   if (hd->fn15.flags & 1)   return 1;	/* carry was set */
   geom->type = hd->fn15.ah;
   if (geom->type == 0) return 1;
   if (geom->type == 3)
     geom->n_total_blocks = ((int)hd->fn15.cx << 16) + hd->fn15.dx;
   
   /* regs.eax = 0x0800;		*/
   /* regs.edx = drive;			*/
   
#ifdef DEBUG
	printf("get_geom: int13, fn=08\n");
	fflush(stdout);
#endif
   
   if (hd->fn08.flags&1 || hd->fn08.ah || hd->fn08.cx==0)
     return 1 + hd->fn08.ah;
   
   geom->n_sect = hd->fn08.cx & 0x3F;
   geom->n_head = ((hd->fn08.dx>>8)&0xFF)+1;
   geom->n_cyl  = (((hd->fn08.cx>>8)&0xFF)|((hd->fn08.cx&0xC0)<<2))+1;
   geom->n_disks = hd->fn08.dx & 0xFF;
   geom->pt = NULL;

   if (drive < 4)  return 0;
   
   pt_base = NULL;
   if (buf.s.disk) {
	pt_base = (struct partition *)&buf.b[buf.s.partitions];
   }
   if (pt_base && drive <= (int)buf.s.disk) {
#if 0
   				geom->pt = &pt_base[(drive&15)*4];
#else
	void *p = (void*)pt_base;
	int i = buf.s.version >= 4 ? 8 : 0;
	
	p += (drive & 15) * (PART_TABLE_SIZE + i) + i;
	geom->pt = (struct partition *)p;
	if (i) geom->serial_no = *(int*)(p-6);
#endif
   }

#ifdef DEBUG
   printf("get_geom:  PT->%08X  S/N=%08X\n", (int)geom->pt, geom->serial_no);
#endif
      
   /* regs.eax = 0x4100;      check EDD extensions present */
   /* regs.edx = drive;				*/
   /* regs.ebx = 0x55AA;			*/
#ifdef DEBUG
	printf("get_geom: int13, fn=41\n");
	fflush(stdout);
#endif
   if ((hd->fn41.flags&1)==0 && (hd->fn41.bx)==(unsigned short)0xAA55) {
      geom->EDD_flags = hd->fn41.cx;
      geom->EDD_rev = hd->fn41.ah;
   }
   
   if ((geom->EDD_flags) & EDD_SUBSET) {
      edd_t *dp;

      dp = (edd_t*)hdp[drive-0x80 + 1];
#ifdef DEBUG
	printf("get_geom:  EDD  dp = %08X\n", (int)dp);
	fflush(stdout);
#endif
    /* update the pointer to the next drive */
      hdp[drive-0x80 + 1] = (void*)dp + sizeof(edd_t);

      /* regs.eax = 0x4800;		*/
      /* regs.edx = drive;		*/
      
#ifdef DEBUG
	printf("get_geom: int13, fn=48\n");
	fflush(stdout);
#endif
      
      if ((dp->info) & EDD_PARAM_GEOM_VALID) {
         if ((geom->n_sect != dp->sectors ||
             geom->n_head != dp->heads) && !nowarn && !(warned[drive-0x80]++))
                fprintf(errstd,"Warning: Int 0x13 function 8 and function 0x48 return different\n"
                               "head/sector geometries for BIOS drive 0x%02X\n", drive);
         geom->n_cyl  = dp->cylinders;
         geom->n_head = dp->heads;
         geom->n_sect = dp->sectors;
         geom->n_total_blocks = dp->total_sectors;
      }

   }
   
   return 0;
}


/* get the conventional memory size in Kb */
static int get_conv_mem(void)
{
    if(fetch()) {
        printf("No memory information is available.\n\n");
        exit(0);
    }
    return (int)buf.s.v.mem;
}


/* print the conventional memory size */
static void do_ebda(void)
{
    int m, n;
    static char EBDA[]="Extended BIOS Data Area (EBDA)";
    
    m = get_conv_mem();
    if (m==640) printf("    no %s\n", EBDA);
    else printf("    %s = %dK\n", EBDA, 640-m);
    printf("    Conventional Memory = %dK    0x%06X\n", m, m<<10);
    m <<= 10;
    m -= 0x200;
    n = m - (MAX_SECONDARY+4+MAX_DESCR_SECS)*SECTOR_SIZE;
#if 0
    if (!(verbose>0)) return;
#else
    printf("\n");
#endif
    printf("    The First stage loader boots at:  0x%08X  (%04X:0000)\n",
    			FIRSTSEG<<4, FIRSTSEG);
    printf("    The Second stage loader runs at:  0x%08X  (%04X:%04X)\n",
    			n, n>>4, n&15);
    printf("    The kernel cmdline is passed at:  0x%08X  (%1X000:%04X)\n",
    			m, m>>16, m&0xFFFF);

}


/* print the CHS geometry information for the specified disk */
static void print_geom(int dr, struct disk_geom geom)
{
	 printf("    bios=0x%02x, cylinders=%d, heads=%d, sectors=%d\n", 
		dr, geom.n_cyl, geom.n_head, geom.n_sect);
	 if (geom.EDD_flags & EDD_PACKET)
	    printf("\tEDD packet calls allowed\n");
}


/* print disk drive geometry for all drives */
static void do_geom_all(void)
{
   int d, hd, dr;
   struct disk_geom geom;
   
   for (hd=0; hd<0x81; hd+=0x80)
   for (d=0; d<16; d++) {
      dr = d+hd;
      if (get_geom(dr, &geom)==0) {
         if (dr==0x80) printf("\nBIOS reports %d drive%s\n", (int) geom.n_disks,
                                      (int)geom.n_disks==1 ? "" : "s");
      	 print_geom(dr, geom);
      }
   }
}


/* print disk drive geometry information for a particular drive */
static void do_geom(char *bios)
{
    int dr;
    struct disk_geom geom;

#if 0    
    dr = my_atoi(bios);
#endif
    dr = to_number(bios);
    if (get_geom(dr, &geom)==0) print_geom(dr, geom);
    else printf("Unrecognized BIOS device code 0x%02x\n", dr);
    
}


/* print an individual partition table entry */
static void print_pt(int index, struct partition pt)
{
    char bt[4], *ty, start[32], end[32], type[8];
    char x;
    int i;
    
    for (x=i=0; i<sizeof(pt); i++) x |= ((char*)&pt)[i];
    if (!x) {
    	printf("%4d\t\t\t     ** empty **\n", index);
    	return;
    }
    strcpy(bt,"   ");
    sprintf(type, "0x%02x", (int)pt.sys_ind);
    sprintf(start, "%4d:%d:%d",
    	(int)pt.cyl+((pt.sector&0xC0)<<2),
    	(int)pt.head,
    	(int)pt.sector & 0x3f );
    sprintf(end, "%4d:%d:%d",
    	(int)pt.end_cyl+((pt.end_sector&0xC0)<<2),
    	(int)pt.end_head,
    	(int)pt.end_sector & 0x3f );
    ty = type;
    if (pt.boot_ind==0x80) bt[1]='*';
    else if (pt.boot_ind==0) ; /* do nothing */
    else {
    	sprintf(bt+1,"%02x", (int)pt.boot_ind);
    }
    for (i=0; ptab[i].name; i++) {
	if (ptab[i].type == pt.sys_ind) {
	    ty = ptab[i].name;
	    break;
	} else if ((ptab[i].type|ptab[i].hide) == pt.sys_ind) {
	    bt[0] = 'H';
	    ty = ptab[i].name;
	    break;
	}
    }
    printf("%4d%18s%5s%11s%14s%12u%12u\n", index, ty, bt,
    	start, end, pt.start_sect, pt.nr_sects);
}


/* partition table display */
static void do_table(char *part)
{
    int fd, i;
    struct partition pt [PART_MAX];
    unsigned int second, base;
    unsigned short boot_sig;
    
    if (!strncmp(part, "/dev/", 5)) {
	if (strncmp(part+5, "md", 2) && isdigit(part[strlen(part)-1]) )
	    die("Not a device: '%s'", part);
	fd = open(part, O_RDONLY);
    } else
	fd = -1;
    if (fd<0) die("Unable to open '%s'", part);
    if (lseek(fd, PART_TABLE_OFFSET, SEEK_SET)<0) die("lseek failed");
    if (read(fd, pt, sizeof(pt)) != sizeof(pt)) die("read pt failed");
    if ( read(fd, &boot_sig, sizeof(boot_sig)) != sizeof(boot_sig)  ||
	boot_sig != BOOT_SIGNATURE ) die("read boot signature failed");
    {
	if (lseek(fd, MAX_BOOT_SIZE+2, SEEK_SET)<0) die("lseek s/n failed");
	if (read(fd, &second, sizeof(second)) != sizeof(second))
	    die("read s/n failed");
	printf(" S/N = %08X\n", second);
    }
    printf("%s\n", phead);
    second=base=0;
    for (i=0; i<PART_MAX; i++) {
	print_pt(i+1, pt[i]);
	if (is_extd_part(pt[i].sys_ind)) {
	    if (!base) base = pt[i].start_sect;
	    else die("invalid partition table: second extended partition found");
	}
    }
    i=5;
    while (verbose>0 && base) {
        if (llseek(fd, SECTORSIZE*(base+second) + PART_TABLE_OFFSET, SEEK_SET) < 0)
            die("secondary llseek failed");
	if (read(fd, pt, sizeof(pt)) != sizeof(pt)) die("secondary read pt failed");
	if ( read(fd, &boot_sig, sizeof(boot_sig)) != sizeof(boot_sig)  ||
	    boot_sig != BOOT_SIGNATURE ) die("read second boot signature failed");
        print_pt(i++, pt[0]);
        if (is_extd_part(pt[1].sys_ind)) second=pt[1].start_sect;
        else base = 0;
    }
        
    close(fd);
}


/* list partition change-rules */
static void do_cr_pr(void)
{
    CHANGE_RULE *cr;
    
    cr = change_rules;
    printf("\t\tType Normal Hidden\n");
    if (!cr) printf("\t **** no change-rules defined ****\n");
    while (cr) {
	printf ("%20s  0x%02x  0x%02x\n", cr->type, (int)cr->normal, (int)cr->hidden);
	cr = cr->next;
    }
}

#if VIDEO
static int egamem;
static int mode, col, row, page;

int get_video(void)	/* return -1 on error, or adapter type [0..7] */
{
    int adapter = 0;    /* 0=unknown, 1=MDA,HGC, 2=CGA, 3=EGA, 4=MCGA, 5=VGA,
		    			 6=VESA (640), 7=VESA (800) */
    int okay = 1;
    int monitor;
        
    if(fetch()) return -1;

    if ((okay=buf.s.vid)) {
    /* get current video mode */
    /* reg.eax = 0x0F00;		*/
	if (verbose >= 2)
	    printf("get video mode\n");
	mode = buf.s.v.vid0F.al;
	col = buf.s.v.vid0F.ah;
	page = buf.s.v.vid0F.bh;
	row = buf.s.v.vid0F.bl + 1;
	if (mode==7) {
	    adapter=1;
	    row = 25;
	    okay = 0;
	} else if (col<80) {
	    adapter=2;
	    okay=0;
	} else adapter=2;	/* at least CGA */
    }
    if (okay>=2) {
#if 1
    /* determine video adapter type */
    /*    reg.eax = 0x1200;	  call valid on EGA/VGA */
    /*    reg.ebx = 0xFF10;				*/
	if (verbose >= 2)
	    printf("determine adapter type\n");
	if ((unsigned)(monitor = buf.s.v.vid12.bh) <= 1U)  {
    	    adapter = 3; /* at least EGA */
	    egamem = buf.s.v.vid12.bl;
	}
	else {
	    okay = 0;
	}
    }
#endif
    if (okay>=2) {
	/* check for VGA */
    	/* reg.eax = 0x1A00;	   get display combination */
    	if (verbose >= 2)
	    printf("get display combination\n");
	if ( buf.s.v.vid1A.al==0x1A ) {
    	    monitor = (buf.s.v.vid1A.bx >> ((verbose>=3)*8) ) & 0xFF;
    	    switch (monitor) {
    	      case 1:
    	        adapter = 1;
    	        break;
    	      case 2:
    	        adapter = 2;
    	        break;
	      case 7:
	      case 8:
	        adapter = 5;	/* VGA */
    	        break;
    	      case 0xA:
    	      case 0xB:
    	      case 0xC:
    	        adapter = 4;	/* MCGA */
    	        break;
    	      default:
    	        okay = 0;
    	        break;
    	    }
    	} else {
    	    okay = 0;
    	}
    }
    if (okay>=3 && adapter==5) {
	/* check for VESA extensions */
    	if (verbose >= 2)
	    printf("check VESA present\n");
	    
	if ((buf.s.v.vid4F00.ax == 0x4f) && strncmp("VESA", buf.s.v.vid4F00.sig, 4)==0)  adapter++;
	
	if (adapter > 5) {
	    /* reg.eax = 0x4f01;
	       reg.ecx = 0x0101;	   640x480x256 */
	    if ((buf.s.v.vid101.ax == 0x4f) && (buf.s.v.vid101.bits & 0x19) == 0x19) adapter ++;
	    else adapter--;
	}
	if (adapter > 6) {
	    /* reg.eax = 0x4f01;
	       reg.ecx = 0x0103;	   800x600x256 */
	    if ((buf.s.v.vid103.ax == 0x4f) && (buf.s.v.vid103.bits & 0x19) == 0x19) ;
	    else adapter--;
	}
    }
    if (verbose>=2)
    	printf ("mode = 0x%02x,  columns = %d,  rows = %d,  page = %d\n\n",
    		       mode, col, row, page);
    
    return adapter;
}

/* print VGA/VESA mode information */
void do_video(void)
{
static char *display[] = { "unknown", "MDA", "CGA", "EGA", 
		"MCGA", "VGA", "VGA/VESA", "VGA/VESA" };
    int adapter; /* 0=unknown, 1=MDA,HGC, 2=CGA, 3=EGA, 4=MCGA, 5=VGA,
    			 6=VESA (640), 7=VESA (800) */

    adapter = get_video();

    if(adapter<0) {
        printf("No video mode information is available.\n");
        return;
    }
    
    if (verbose>=1)
    	printf ("mode = 0x%02x,  columns = %d,  rows = %d,  page = %d\n\n",
    		       mode, col, row, page);

    printf("%s adapter:\n\n", display[adapter]);
    if (adapter < 3 || (adapter == 3 && egamem < 1) ) {
        printf("No graphic modes are supported\n");
    } else {
	if (adapter != 4)
	    printf("    640x350x16    mode 0x0010\n");
	if (adapter >= 5) {
	    printf("    640x480x16    mode 0x0012\n\n");
	    printf("    320x200x256   mode 0x0013\n");
	}
	if (adapter >= 6)
	    printf("    640x480x256   mode 0x0101\n");
	if (adapter >= 7)
	    printf("    800x600x256   mode 0x0103\n");
    }

}
#endif

/* entry from lilo.c for the '-T' (tell) switch */
void probe_tell (char *cmd)
{
    struct Probes *pr = list;
    int n;
    char *arg;
    
    if (!(verbose>0)) printf("\n");
    for (; pr->cmd; pr++) {
	n = strlen(pr->cmd);
	arg = NULL;
	if (pr->cmd[n-1] == '=') arg = cmd+n;
	if (!strncasecmp(cmd, pr->cmd, n)) {
	    pr->prc(arg);
	    printf("\n");
	    exit(0);
	}
    }
    printf("Unrecognized option to '-T' flag\n");
    do_help();
    
    exit(1);
}


int bios_max_devs(void)
{
    struct disk_geom geom;
    int i;
    
    if (!fetch() && !get_geom(0x80, &geom)) {
	i = (buf.s.disk & 0x7f) + 1;
	if (geom.n_disks == i) return i;
    }
    return BIOS_MAX_DEVS;    
}

#ifdef DEBUG
static void dump_pt(unsigned char *pt)
{
    int i, j;
    for (j=0; j<4; j++) {
	for (i=0; i<16; i++) {
	    printf(" %02X", (int)(*pt++));
	}
	printf("\n");
    }
    printf("\n");
}
#endif

/* 
 *  return the bios device code of the disk, based on the geometry
 * 	match with the probe data
 *   side effect is to place the device code in geo->device
 * return -1 if indeterminate
 */
int bios_device(GEOMETRY *geo, int device)
{
    int bios1, bios, match, fd;
    int mbios[BD_MAX_HARD];
    struct disk_geom bdata;
    DEVICE dev;
    unsigned char part[PART_TABLE_SIZE];
    unsigned char extra[8];
    int serial;
    
        
    if (fetch()) return -1;
    
    if (verbose>=3) printf("bios_dev:  device %04X\n", device);
    
    match = 0;
    bios1 = -1;		/* signal error */
    for (bios=0x80; bios<=buf.s.disk; bios++) {
	mbios[bios-0x80] = 0;
	if (get_geom(bios, &bdata)) break;
	if (geo->cylinders == bdata.n_cyl &&
	    geo->heads == bdata.n_head &&
	    geo->sectors == bdata.n_sect) {
	    	match++;
	    	mbios[bios-0x80] = bios1 = bios;
	}
    }
    if (match == 1) {
    	if (verbose>=2) printf("bios_dev: match on geometry alone (0x%02X)\n",
    		bios1);
	return (geo->device = bios1);
    }

    device &= D_MASK(device);	/* mask to primary device */
    fd = dev_open(&dev,device,O_RDONLY);
    if (verbose>=3) printf("bios_dev:  masked device %04X, which is %s\n", 
    			device, dev.name);
    if (lseek(fd, PART_TABLE_OFFSET-8, SEEK_SET)!=PART_TABLE_OFFSET-8)
	die("bios_device: seek to partition table - 8");
    if (read(fd,extra,sizeof(extra))!= sizeof(extra))
	die("bios_device: read partition table - 8");
    serial = *(int*)(extra+2);
    if (lseek(fd, PART_TABLE_OFFSET, SEEK_SET)!=PART_TABLE_OFFSET)
	die("bios_device: seek to partition table");
    if (read(fd,part,sizeof(part))!= sizeof(part))
	die("bios_device: read partition table");
    dev_close(&dev);

#ifdef DEBUG
    if (verbose>=4) {
        printf("serial number = %08X\n", serial);
        dump_pt(part);
    }
#endif

    if (verbose>=3) printf("bios_dev: geometry check found %d matches\n", match);
    match = 0;
    bios1 = -1;
    while (--bios >= 0x80) {
	get_geom(bios, &bdata);
	if (verbose>=4) {
		printf("bios_dev: (0x%02X)  S/N=%08X  *PT=%08X\n",
			bios, bdata.serial_no, (int)bdata.pt);
#ifdef DEBUG
		dump_pt((void*)bdata.pt);
#endif
	}
	if (!memcmp(part,bdata.pt,sizeof(part)) &&
		!(bdata.serial_no && serial!=bdata.serial_no)
	   ) {
	    match++;
	    bios1 = bios;
	}
    }
    if (verbose>=2) printf("bios_dev: PT match found %d match%s (0x%02X)\n",
    		match, match==1 ? "" : "es", bios1);

    if (match != 1) bios1 = -1;

    return (geo->device = bios1);
}


