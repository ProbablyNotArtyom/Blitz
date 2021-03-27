#if 0
/* common.h  -  Common data structures and functions. */
/*
Copyright 1992-1998 Werner Almesberger.
Copyright 1999-2000 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

*/
#endif

#ifndef COMMON_H
#define COMMON_H

#ifndef LILO_ASM
#include <sys/stat.h>
#include <asm/types.h>
#include <linux/genhd.h>

#include "lilo.h"


#define O_NOACCESS 3  /* open a file for "no access" */

#endif

/*
;*/typedef struct {		/*
							block	0
;*/    unsigned char sector,track; /* CX 
						sa_sector:	.blkb	1
						sa_track:	.blkb	1
;*/    unsigned char device,head; /* DX
						sa_device:	.blkb	1
						sa_head:	.blkb	1
;*/    unsigned char num_sect; /* AL
						sa_num_sect:	.blkb	1
;*/} SECTOR_ADDR; /*
						sa_size:
							endb




;*/typedef struct {			/*
							block	0
;*/    char name[MAX_IMAGE_NAME+1];	/* image name, NUL terminated 
						id_name:	.blkb	MAX_IMAGE_NAME_asm+1
;*/    unsigned short password_crc[MAX_PW_CRC*(sizeof(long)/sizeof(short))];  /* 4 password CRC-32 values
						id_password_crc:.blkb	MAX_PW_CRC_asm*4
;*/    unsigned short rd_size[2]; /* RAM disk size in bytes, 0 if none
						id_rd_size:	.blkb	4		;don't change the order !!!
;*/    SECTOR_ADDR initrd,start;  /* start of initrd & kernel
						id_initrd:	.blkb	sa_size		;  **
						id_start:	.blkb	sa_size		;  **
;*/    unsigned short start_page; /* page at which the kernel is loaded high, 0
;*/				  /* if loading low
						id_start_page:	.blkb	2		;  **
;*/    unsigned short flags,vga_mode; /* image flags & video mode
						id_flags:	.blkb	2		;  **
						id_vga_mode:	.blkb	2		;  **
;*/} IMAGE_DESCR;		/*
						id_size:
							endb



;*/typedef struct {					/*
							block	0
;*/    unsigned char cli;     /* clear interrupt flag instruction
						par1_cli:	.blkb	1
;*/    unsigned char call_ins;  /* call instruction opcode
						par1_call_ins:	.blkb	1
;*/    short call_offset;  /* offset to destination of call instruction
						par1_call_offs:	.blkb	2
;*/    unsigned short code_length;  /* length of the first stage code
						par1_code_len:	.blkb	2
;*/    char signature[4]; /* "LILO"
						par1_signature:	.blkb	4
;*/    unsigned short stage,version;  /*
						par1_stage:	.blkb	2
						par1_version:	.blkb	2
;*/    unsigned char port; /* COM port. 0 = none, 1 = COM1, etc. !!! keep these two serial bytes together !!!
;*/    unsigned char ser_param; /* RS-232 parameters, must be 0 if unused
						par1_port:	.blkb	1	; referenced together
						par1_ser_param:	.blkb	1	; **
;*/    unsigned long raid_offset; /* raid partition/partition offset
						par1_raid_offset: .blkb	4
;*/    unsigned long timestamp; /* timestamp for restoration
						par1_timestamp:	.blkb	4
;*/    unsigned short timeout; /* 54 msec delay until input time-out,
			       0xffff: never 
						par1_timeout:	.blkb	2
;*/    unsigned short delay; /* delay: wait that many 54 msec units.
						par1_delay:	.blkb	2
;*/    unsigned short msg_len; /* 0 if none
						par1_msg_len:	.blkb	2
;*/    SECTOR_ADDR msg; /* initial greeting message
						par1_msg:	.blkb	sa_size
;*/    SECTOR_ADDR descr[MAX_DESCR_SECS+1]; /* 2 descriptors and default command line
						par1_descr:	.blkb	sa_size*MAX_DESCR_SECS_asm
						par1_dflcmd:	.blkb	sa_size
;*/    unsigned char prompt; /* FLAG_PROMPT=always, FLAG_RAID install
						par1_prompt:	.blkb	1
;*/    SECTOR_ADDR keytab; /* keyboard translation table
						par1_keytab:	.blkb	sa_size
;*/    SECTOR_ADDR secondary; /* sectors of the second stage loader
						par1_secondary:	.blkb	sa_size
;*/} BOOT_PARAMS_1; /* first stage boot loader 
								.align	4
						par1_size:
							endb


;*/typedef struct {					/*
							block	0
;*/	char menu_sig[4];	/* "MENU" or "BMP4" signature, or NULs if not present
						mt_sig:		.blkb	4
;*/	unsigned char at_text;	/* attribute for normal menu text
						mt_at_text:	.blkb	1
;*/	unsigned char at_highlight;	/* attribute for highlighted text
						mt_at_hilite:	.blkb	1
;*/	unsigned char at_border;	/* attribute for borders
						mt_at_border:	.blkb	1
;*/	unsigned char at_title;		/* attribute for title
						mt_at_title:	.blkb	1
;*/	unsigned char len_title;	/* length of the title string
						mt_len_title:	.blkb	1
;*/	char title[MAX_MENU_TITLE+2];	/* MENU title to override default
						mt_title:	.blkb	MAX_MENU_TITLE_asm+2
;*/	short row, col, ncol;		/* BMP row, col, and ncols
						mt_row:		.blkw	1
						mt_col:		.blkw	1
						mt_ncol:	.blkw	1
;*/	short maxcol, xpitch;		/* BMP max per col, xpitch between cols
						mt_maxcol:	.blkw	1
						mt_xpitch:	.blkw	1
;*/	short fg, bg, sh;		/* BMP normal text fore, backgr, shadow
						mt_fg:		.blkw	1
						mt_bg:		.blkw	1
						mt_sh:		.blkw	1
;*/	short h_fg, h_bg, h_sh;		/* highlight fg, bg, & shadow
						mt_h_fg:	.blkw	1
						mt_h_bg:	.blkw	1
						mt_h_sh:	.blkw	1
;*/	short t_fg, t_bg, t_sh;		/* timer fg, bg, & shadow colors
						mt_t_fg:	.blkw	1
						mt_t_bg:	.blkw	1
						mt_t_sh:	.blkw	1
;*/	short t_row, t_col;		/* timer position
						mt_t_row:	.blkw	1
						mt_t_col:	.blkw	1
;*/} MENUTABLE;		/* MENU and BITMAP parameters at KEYTABLE+256
						mt_size:
							endb

;*/

#ifndef LILO_ASM
typedef struct {
    char jump[6]; /* jump over the data */
    char signature[4]; /* "LILO" */
    unsigned short stage,version;
} BOOT_PARAMS_2; /* second stage boot loader */

typedef struct {
    char jump[6]; /* jump over the data */
    char signature[4]; /* "LILO" */
    unsigned short stage,version; /* stage is 0x10 */
    unsigned short offset; /* partition entry offset */
    unsigned char drive; /* BIOS drive code */
    unsigned char head; /* head; always 0 */
    unsigned short drvmap; /* offset of drive map */
    unsigned char ptable[PARTITION_ENTRY*PARTITION_ENTRIES]; /* part. table */
} BOOT_PARAMS_C; /* chain loader */

typedef union {
    BOOT_PARAMS_1 par_1;
    BOOT_PARAMS_2 par_2;
    BOOT_PARAMS_C par_c;
    unsigned char sector[SECTOR_SIZE];
} BOOT_SECTOR;

typedef union {
    struct {
	IMAGE_DESCR descr[MAX_IMAGES]; /* boot file descriptors */
    } d;
    unsigned char sector[SECTOR_SIZE*MAX_DESCR_SECS];
    struct {
    	unsigned long sector[SECTOR_SIZE/4*MAX_DESCR_SECS - 1];
    	unsigned long checksum;
    } l;
} DESCR_SECTORS;
#endif
/*
IMAGES_numerator = SECTOR_SIZE_asm*MAX_DESCR_SECS_asm - 4 - 1
IMAGES = IMAGES_numerator / id_size
;*/
#ifndef LILO_ASM
typedef struct {
    unsigned short jump;	/*  0: jump to startup code */
    char signature[4];		/*  2: "HdrS" */
    unsigned short version;	/*  6: header version */
    unsigned short x,y,z;	/*  8: LOADLIN hacks */
    unsigned short ver_offset;	/* 14: kernel version string */
    unsigned char loader;	/* 16: loader type */
    unsigned char flags;	/* 17: loader flags */
    unsigned short a;		/* 18: more LOADLIN hacks */
    unsigned long start;	/* 20: kernel start, filled in by loader */
    unsigned long ramdisk;	/* 24: RAM disk start address */
    unsigned long ramdisk_size;	/* 28: RAM disk size */
    unsigned short b,c;		/* 32: bzImage hacks */
    unsigned short heap_end_ptr;/* 36: end of free area after setup code */
} SETUP_HDR;

#define alloc_t(t) ((t *) alloc(sizeof(t)))


extern int verbose,test,compact,linear,nowarn,lba32,autoauto,passw,geometric;
extern int boot_dev_nr,raid_flags,do_md_install,zflag;
extern unsigned short drv_map[DRVMAP_SIZE+1]; /* needed for fixup maps */
extern int curr_drv_map;
extern unsigned long prt_map[PRTMAP_SIZE+1];
extern int curr_prt_map, config_read;
#if 0
extern unsigned long crc_polynomial[MAX_PW_CRC];
#endif
extern char *config_file;
extern FILE *errstd;


/*volatile*/ void pdie(char *msg);

/* Do a perror and then exit. */


/*volatile*/ void die(char *fmt,...);

/* fprintf an error message and then exit. */


void *alloc(int size);

/* Allocates the specified number of bytes. Dies on error. */


void *ralloc(void *old,int size);

/* Changes the size of an allocated memory area. Dies on error. */


char *stralloc(const char *str);

/* Like strdup, but dies on error. */


int to_number(char *num);

/* Converts a string to a number. Dies if the number is invalid. */


void check_version(BOOT_SECTOR *sect, int stage);

/* Verify that a boot sector has the correct version number. */


int stat_equal(struct stat *a, struct stat *b);

/* Compares two stat structures. Returns a non-zero integer if they describe
   the same file, zero if they don't. */


unsigned long crc32 (unsigned char *cp, int nsize, unsigned long polynomial);

/* calculate a CRC-32 polynomial */

#endif
#endif
