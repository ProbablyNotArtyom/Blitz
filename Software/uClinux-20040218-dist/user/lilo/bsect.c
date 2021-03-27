/* bsect.c  -  Boot sector handling */
/*
Copyright 1992-1998 Werner Almesberger.
Copyright 1999-2001 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

*/


#include <unistd.h>
#include <sys/types.h>
//#include <sys/statfs.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#ifdef	_SYS_STATFS_H
#define	_I386_STATFS_H	/* two versions of statfs is not good ... */
#endif

#include <linux/fs.h>
#include <linux/hdreg.h>
#include <linux/fd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>

#include "config.h"
#include "common.h"
#include "cfg.h"
#include "lilo.h"
#include "device.h"
#include "geometry.h"
#include "map.h"
#include "temp.h"
#include "partition.h"
#include "boot.h"
#include "bsect.h"
#include "bitmap.h"

#ifdef SHS_PASSWORDS
#include "shs2.h"
#endif


int boot_dev_nr;

static BOOT_SECTOR bsect,bsect_orig;
static DESCR_SECTORS descrs;
static char secondary_map[SECTOR_SIZE];
static DEVICE dev;
static char *boot_devnam,*map_name;
static int fd;
static int image_base = 0,image = 0;
static char temp_map[PATH_MAX+1];
static char *fallback[MAX_IMAGES];
static int fallbacks = 0;
static unsigned short stage_flags;
static int image_menu_space = MAX_IMAGES;
static char *getval_user;

typedef struct Pass {
    long crc[MAX_PW_CRC];
    char *unique;
    char *label;
    struct Pass *next;
    } PASSWORD;

static PASSWORD *pwsave = NULL;

static int getval(char **cp, int low, int high, int default_value, int factor)
{
    int temp;
    
    if (!**cp) {
	if (factor) {
	    if (low==1) default_value--;
	    default_value *= factor;
	}
    }
    else if (ispunct(**cp)) {
	(*cp)++;
	if (factor) {
	    if (low==1) default_value--;
	    default_value *= factor;
	}
    } else {
	temp = strtol(*cp, cp, 0);
	if (!factor) default_value = temp;
	else {
	    if (**cp == 'p' || **cp == 'P') {
		(*cp)++;
		default_value = temp;
		temp /= factor;
		if (low==1) temp++;
	    } else {
		default_value = (low==1 ? temp-1 : temp)*factor;
	    }
	}
	
	if (temp < low || temp > high)
		die("%s: value out of range [%d,%d]", getval_user, low, high);
	if (**cp && !ispunct(**cp)) die("Invalid character: \"%c\"", **cp);
	if (**cp) (*cp)++;
    }
    if (verbose>=5) printf("getval: %d\n", default_value);
    
    return default_value;
}

static void bmp_do_timer(char *cp, MENUTABLE *menu)
{
    getval_user = "bmp-timer";
    if (!cp) {
        menu->t_row = menu->t_col = -1;  /* there is none, if not specified */
    } else {
    	menu->t_col = getval(&cp, 1, 76, 64, 8);
    	menu->t_row = getval(&cp, 1, 80, 2, 16);
    	menu->t_fg = getval(&cp, 0, 15, menu->fg, 0);
    	menu->t_bg = getval(&cp, 0, 15, 0, 0);
    	menu->t_sh = getval(&cp, 0, 15, menu->t_fg, 0);
    }
}


static void bmp_do_table(char *cp, MENUTABLE *menu)
{
    if (!cp) cp = "";
    
    getval_user = "bmp-table";
    menu->col = getval(&cp, 1, 80-MAX_IMAGE_NAME, 12, 8);
    menu->row = getval(&cp, 1, 29, 5, 16);
    menu->ncol = getval(&cp, 1, 80/(MAX_IMAGE_NAME+2), 1, 0);
    menu->maxcol = getval(&cp, 3, MAX_IMAGES, (MAX_IMAGES+menu->ncol-1)/menu->ncol, 0);
    menu->xpitch = getval(&cp, MAX_IMAGE_NAME+2, 80/menu->ncol, MAX_IMAGE_NAME+6, 8);
    if ((menu->row + menu->maxcol > 480 || 
         menu->col + (MAX_IMAGE_NAME+1)*8 + (menu->ncol-1)*menu->xpitch > 640)
          && !nowarn)  
       fprintf(errstd,"Warning: 'bmp-table' may spill off screen\n");
    image_menu_space = menu->ncol * menu->maxcol;
    if (verbose>=5) printf("image_menu_space = %d\n", image_menu_space);
}


void bmp_do_colors(char *cp, MENUTABLE *menu)
{
    if (!cp) cp = "";

    getval_user = "bmp-colors";
    menu->fg = getval(&cp, 0, 15, 7, 0);
    menu->bg = getval(&cp, 0, 15, menu->fg, 0);
    menu->sh = getval(&cp, 0, 15, menu->fg, 0);

    menu->h_fg = getval(&cp, 0, 15, 15, 0);
    menu->h_bg = getval(&cp, 0, 15, menu->h_fg, 0);
    menu->h_sh = getval(&cp, 0, 15, menu->h_fg, 0);    	
}

void pw_file_update(int passw)
{
    PASSWORD *walk;
    int i;
    
    if (verbose>=4) printf("pw_file_update:  passw=%d\n", passw);

    if (passw & !test) {
	if (fseek(pw_file,0L,SEEK_SET)) perror("pw_file_update");
    
	for (walk=pwsave; walk; walk=walk->next) {
	    fprintf(pw_file, "label=<\"%s\">", walk->label);
	    for (i=0; i<MAX_PW_CRC; i++) fprintf(pw_file, " 0x%08lX", walk->crc[i]);
	    fprintf(pw_file, "\n");
	}
    }
    if (pw_file) fclose(pw_file);
}

void pw_fill_cache(void)
{
    char line[MAX_TOKEN+1];
    char *brace;
    char *label;
    PASSWORD *new;
    int i;
     
    if (verbose>=5) printf("pw_fill_cache\n");    
    if (fseek(pw_file,0L,SEEK_SET)) perror("pw_fill_cache");
    
    while (fgets(line,MAX_TOKEN,pw_file)) {
    	if (verbose>=5) printf("   %s\n", line);
    	brace = strrchr(line,'>');
    	label = strchr(line,'<');
    	if (label && label[1]=='"' && brace && brace[-1]=='"') {
	    brace[-1] = 0;
	    if ( !(new = alloc_t(PASSWORD)) ) pdie("Out of memory");
	    new->next = pwsave;
	    pwsave = new;
	    new->unique = NULL;
	    new->label = stralloc(label+2);
	    if (verbose>=2) printf("Password file: label=%s\n", new->label);
	    brace++;
    	    for (i=0; i<MAX_PW_CRC; i++) {
		new->crc[i] = strtoul(brace,&label,0);
		brace = label;
    	    }
    	}
    	else die("Ill-formed line in .crc file");
    }
    if (verbose >=5) printf("end pw_fill_cache\n");
}

static void hash_password(char *password, long crcval[])
{
#ifdef CRC_PASSWORDS
   static long poly[] = {CRC_POLY1, CRC_POLY2, CRC_POLY3, CRC_POLY4, CRC_POLY5};
#endif
	long crc;
	int j;
	int i = strlen(password);
	
#ifdef SHS_PASSWORDS
	shsInit();
	shsUpdate(password, i);
	shsFinal();
#endif
	for (j=0; j<MAX_PW_CRC; j++) {
	    crcval[j] = crc =
#ifdef CRC_PASSWORDS
	    			crc32(password, i, poly[j]);
#else
				shsInfo.digest[j];
#endif
	    if(verbose >= 2) {
		if (j==0) printf("Password "
#ifdef CRC_PASSWORDS
					"CRC-32"
#else
					"SHS-160"
#endif
						" =");
		printf(" %08lX", crc);
	    }
	}
	if (verbose >= 2) printf("\n");
}


void pw_wipe(char *pass)
{
    int i;
    
    if (!pass) return;
    i = strlen(pass);
    while (i) pass[--i]=0;
    free(pass);
}


static char *pw_input(void)
{
    char *pass;
    char buf[MAX_TOKEN+1];
    int i, ch;
    
    i = 0;
    while((ch=getchar())!='\n') if (i<MAX_TOKEN) buf[i++]=ch;
    buf[i]=0;
    pass = stralloc(buf);
    while (i) buf[--i]=0;
    return pass;
}

static void pw_get(char *pass, long crcval[], int option)
{
    PASSWORD *walk;
    char *pass2;
    char *label;
    
    label = cfg_get_strg(cf_all, "label");
    if (!label) label = cfg_get_strg(cf_top, "image");
    if (!label) label = cfg_get_strg(cf_top, "other");
    if (!label) die("Need label to get password");
    if ((pass2 = strrchr(pass,'/'))) label = pass2+1;

    for (walk=pwsave; walk; walk=walk->next) {
        if (pass == walk->unique ||
        	(!walk->unique && !strcmp(walk->label,label) && (walk->unique=pass)) ) {
            memcpy(crcval, walk->crc, MAX_PW_CRC*sizeof(long));
            return;
        }
    }
    walk = alloc_t(PASSWORD);
    if (!walk) die("Out of memory");
    walk->next = pwsave;
    pwsave = walk;
    walk->unique = pass;
    walk->label = stralloc(label);
    
    printf("\nEntry for  %s  used null password\n", label);
    pass = pass2 = NULL;
    do {
    	if (pass) {
    	    printf("   *** Phrases don't match ***\n");
    	    pw_wipe(pass);
    	    pw_wipe(pass2);
	}
	printf("Type passphrase: ");
	pass2 = pw_input();
	printf("Please re-enter: ");
	pass = pw_input();
    } while (strcmp(pass,pass2));
    printf("\n");
    pw_wipe(pass2);  
    hash_password(pass, walk->crc);
    pw_wipe(pass);
    memcpy(crcval, walk->crc, MAX_PW_CRC*sizeof(long));
}


static void retrieve_crc(long crcval[])
{
    int i;
    char *pass;
    
    if (!pwsave) {
	if (cfg_pw_open()) pw_fill_cache();
    }
    pass = cfg_get_strg(cf_all,"password");
    if (pass) pw_get(pass,crcval,0);
    else pw_get(cfg_get_strg(cf_options,"password"),crcval,1);

    if (verbose >= 1) {
        printf("Password found is");
        for (i=0; i<MAX_PW_CRC; i++) printf(" %08lX", crcval[i]);
        printf("\n");
    }
}



static void open_bsect(char *boot_dev)
{
    struct stat st;

    if (verbose > 0)
	printf("Reading boot sector from %s\n",boot_dev ? boot_dev :
	  "current root.");
    boot_devnam = boot_dev;
    if (boot_dev) {
	if ((fd = open(boot_dev,O_RDWR)) < 0)
	    die("open %s: %s",boot_dev,strerror(errno));
	if (fstat(fd,&st) < 0) die("stat %s: %s",boot_dev,strerror(errno));
	if (!S_ISBLK(st.st_mode)) boot_dev_nr = 0;
	else boot_dev_nr = st.st_rdev;
    }
    else {
	if (stat("/",&st) < 0) pdie("stat /");
#if 0
	if ((st.st_dev & PART_MASK) > PART_MAX)
#else
	if (MAJOR(st.st_dev) != MAJOR_MD &&
		(st.st_dev & P_MASK(st.st_dev)) > PART_MAX)
#endif
	    die("Can't put the boot sector on logical partition 0x%04X",
	      (int)st.st_dev);
	fd = dev_open(&dev,boot_dev_nr = st.st_dev,O_RDWR);
    }
    if (boot_dev_nr && !is_first(boot_dev_nr) && !nowarn)
	fprintf(errstd,"Warning: %s is not on the first disk\n",boot_dev ?
	  boot_dev : "current root");
    if (read(fd,(char *) &bsect,SECTOR_SIZE) != SECTOR_SIZE)
	die("read %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
    bsect_orig = bsect;
}


void bsect_read(char *boot_dev,BOOT_SECTOR *buffer)
{
    open_bsect(boot_dev);
    *buffer = bsect;
    (void) close(fd);
}


static void menu_do_scheme(char *scheme, MENUTABLE *menu)
{
    static char khar[] = "kbgcrmywKBGCRMYW";
    unsigned int fg, bg;
    int i;
    unsigned char *at;
    /* order of color attributes is:
          text, hilighted text, border, title
    */
#define color(c) ((int)strchr(khar,(int)(c))-(int)khar)
    bg = 0;
    at = &(menu->at_text);
    for (i=0; i<4 && *scheme; i++) {
    	fg = color(*scheme);
    	if (fg>=16) die("Invalid menu-scheme color: '%c'", *scheme);
    	if (*++scheme) bg = color(*scheme);
    	else {
	    die("Invalid menu-scheme syntax");
    	}
    	if (bg>=16) die("Invalid menu-scheme color: '%c'", *scheme);
    	if (*++scheme) {
    	    if (ispunct(*scheme)) scheme++;
    	    else die("Invalid menu-scheme punctuation");
    	}
    	if (bg>=8 && !nowarn)
    	    fprintf(errstd, "Warning: menu-scheme BG color may not be intensified\n");
    	*at++ = ((bg<<4) | fg) & 0x7F;
    }
    /* check on the TEXT color */
    if (menu->at_text == 0) {
        if (!nowarn)
            fprintf(errstd, "Warning: menu-scheme \"black on black\" changed to "
        	"\"white on black\"\n");
        menu->at_text = 0x07;
    }
    /* check on the HIGHLIGHT color */
    if (menu->at_highlight == 0)  menu->at_highlight = ((menu->at_text<<4)&0x70) | ((menu->at_text>>4)&0x0f);
    /* check on the BORDER color */
    if (menu->at_border == 0)  menu->at_border = menu->at_text;
    /* check on the TITLE color */
    if (menu->at_title == 0)  menu->at_title = menu->at_border;
    
    strncpy(menu->menu_sig, "MENU", 4);
    if (verbose>=5)
       printf("Menu attributes: text %02X  highlight %02X  border %02X  title %02X\n",
       		(int)menu->at_text, (int)menu->at_highlight,
       		(int)menu->at_border, (int)menu->at_title);
#undef color
}


void bsect_open(char *boot_dev,char *map_file,char *install,int delay,
  int timeout, long raid_offset)
{
    static char coms[] = "0123";
    static char parity[] = "NnOoEe";
    static char bps[] = 
        "110\000150\000300\000600\0001200\0002400\0004800\0009600\000"
        "19200\00038400\00057600\000115200\000?\000?\000?\000?\000"
        "56000\000";
    GEOMETRY geo;
    struct stat st;
    int i, speed, bitmap;
    int i_fd,m_fd,kt_fd,sectors;
    char *message,*colon,*serial,*walk,*this,*keytable,*scheme;
    unsigned char table[SECTOR_SIZE];
    MENUTABLE *menu;
    unsigned long timestamp;

    image = image_base = i = 0;
    if (stat(map_file,&st) >= 0 && !S_ISREG(st.st_mode))
	die("Map %s is not a regular file.",map_file);
    open_bsect(boot_dev);
    part_verify(boot_dev_nr,1);
#ifndef LCF_NOINSTDEF
    if (!install) install = DFL_BOOT;
#endif
    if (install) {
	if (verbose > 0) printf("Merging with %s\n",install);
	i_fd = geo_open(&geo,install,O_RDONLY);
	timestamp = bsect.par_1.timestamp; /* preserve timestamp */

	if (read(i_fd, table, MAX_BOOT_SIZE) != MAX_BOOT_SIZE)
	    die("read %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
	check_version ((BOOT_SECTOR*)table, STAGE_FIRST);
#if 0
	memmove((char*)&bsect, table, BEG_BPB);
	memmove((char*)&bsect+(END_BPB),table+(END_BPB),MAX_BOOT_SIZE-(END_BPB));
#else
	memmove((char*)&bsect, table, MAX_BOOT_SIZE);
#endif	
	bsect.par_1.timestamp = timestamp;
	if (fstat(i_fd,&st) < 0)
	    die("stat %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
	map_begin_section(); /* no access to the (not yet open) map file
		required, because this map is built in memory */
	map_add(&geo,1,(st.st_size+SECTOR_SIZE-1)/SECTOR_SIZE-1);
#if 0
	sectors = map_write(bsect.par_1.secondary,
				nelem(bsect.par_1.secondary), 1);
#else
	sectors = map_write((SECTOR_ADDR*)secondary_map, SECTOR_SIZE/sizeof(SECTOR_ADDR)-2, 1);
#endif
	if (verbose > 1)
	    printf("Secondary loader: %d sector%s.\n",sectors,sectors == 1 ?
	      "" : "s");
	if (lseek(i_fd, SECTOR_SIZE, SEEK_SET)!=SECTOR_SIZE)
	    die("lseek %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
	if (read(i_fd,table,SECTOR_SIZE) != SECTOR_SIZE)
	    die("read(2) %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
	stage_flags = ((BOOT_SECTOR*)table) -> par_2.stage;
	if ((stage_flags & 0xFF) != STAGE_SECOND)
	    die("Ill-formed boot loader; no second stage section");
	if (verbose>=4) printf("install(2) flags: 0x%04X\n", (int)stage_flags);
	geo_close(&geo);
    }
#if 0
    check_version(&bsect,STAGE_FIRST);
#endif
    if ((colon = strrchr(map_name = map_file,':')) == NULL)
	strcat(strcpy(temp_map,map_name),MAP_TMP_APP);
    else {
	*colon = 0;
	strcat(strcat(strcpy(temp_map,map_name),MAP_TMP_APP),colon+1);
	*colon = ':';
    }
    map_create(temp_map);
    temp_register(temp_map);

    

    *(unsigned short *) &bsect.sector[BOOT_SIG_OFFSET] = BOOT_SIGNATURE;
    message = cfg_get_strg(cf_options,"message");
    scheme = cfg_get_strg(cf_options,"bitmap");
    if (message && scheme) die("'bitmap' and 'message' are mutually exclusive");
    bsect.par_1.msg_len = 0;
    bitmap = scheme ? 1 : 0;
    if (bitmap) {
	message = scheme;
	if (!(stage_flags & STAGE_FLAG_BMP4))
	    die("Boot loader does not support 'bitmap='");
    }
    if (message) {
	if (verbose >= 1) printf("Mapping %s file %s\n", 
			bitmap ? "bitmap" : "message", message);
	m_fd = geo_open(&geo,message,O_RDONLY);
	if (fstat(m_fd,&st) < 0) die("stat %s: %s",message,strerror(errno));
	/* the -2 below is because of GCC's alignment requirements */
	i = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPHEADER);
	if (bitmap || st.st_size>i) {
	    if (read(m_fd,table,i) != i) die("read %s: %s",message,strerror(errno));
	    if (((BITMAPFILEHEADER*)table)->magic == 0x4D42 /* "BM" */ ) {
	        BITMAPHEADER *bmh = (void*)(table+sizeof(BITMAPFILEHEADER));
	        BITMAPHEADER2 *bmh2 = (void*)bmh;
	        if (verbose >= 3) {
	            if (bmh->size == sizeof(BITMAPHEADER))
	            printf("width=%d height=%d planes=%d bits/plane=%d\n",
	        	(int)bmh->width, (int)bmh->height,
	        	(int)bmh->numBitPlanes, (int)bmh->numBitsPerPlane);
	            if (bmh2->size == sizeof(BITMAPHEADER2))
	            printf("width=%d height=%d planes=%d bits/plane=%d\n",
	        	(int)bmh2->width, (int)bmh2->height,
	        	(int)bmh2->numBitPlanes, (int)bmh2->numBitsPerPlane);
	        }
	        if (bmh->size == sizeof(BITMAPHEADER) &&
	        	bmh->width==640 && bmh->height==480 && 
	            	bmh->numBitPlanes * bmh->numBitsPerPlane == 4) {
	            if (!bitmap) die("Message specifies a bitmap file");
	        }
	        else if (bmh2->size == sizeof(BITMAPHEADER2) &&
	        	bmh2->width==640 && bmh2->height==480 && 
	            	bmh2->numBitPlanes * bmh2->numBitsPerPlane == 4) {
	            if (!bitmap) die("Message specifies a bitmap file");
	        } else if (bitmap) die("Unsupported bitmap");
	    } else if (bitmap) die("Not a bitmap file");
	}
	i = bitmap ? MAX_KERNEL_SECS*SECTOR_SIZE : MAX_MESSAGE;
	if (st.st_size > i)
	    die("%s is too big (> %d bytes)",message,i);
	map_begin_section();
	bsect.par_1.msg_len = bitmap ? (st.st_size+15)/16 : st.st_size;
	map_add(&geo,0,((st.st_size)+SECTOR_SIZE-1)/SECTOR_SIZE);
	sectors = map_end_section(&bsect.par_1.msg,0);
	if (verbose >= 2)
	    printf("%s: %d sector%s.\n",bitmap?"Bitmap":"Message",
	    		sectors,sectors == 1 ?  "" : "s");
	geo_close(&geo);
    }
    serial = cfg_get_strg(cf_options,"serial");
    if (serial) {
    	if (!(stage_flags & STAGE_FLAG_SERIAL))
    	    die("Serial line not supported by boot loader");
	if (!*serial || !(this = strchr(coms,*serial)))
	    die("Invalid serial port in \"%s\" (should be 0-3)",serial);
	else bsect.par_1.port = (this-coms)+1;
	bsect.par_1.ser_param = SER_DFL_PRM;
	if (serial[1]) {
	    if (serial[1] != ',')
		die("Serial syntax is <port>[,<bps>[<parity>[<bits>]]]");
	    walk = bps;
	    speed = 0;
	    while (*walk && strncmp(serial+2,walk,(i=strlen(walk)))) {
	        speed++;
	        walk += i+1;
	    }
	    if (!*walk) die("Unsupported baud rate");
	    bsect.par_1.ser_param &= ~0xE4;
	    if (speed==16) speed -= 6;  /* convert 56000 to 57600 */
	    bsect.par_1.ser_param |= ((speed<<5) | (speed>>1)) & 0xE4;
	    serial += i+2;
	/* check for parity specified */
	    if (*serial) {
		if (!(this = strchr(parity,*serial)))
		    die("Valid parity values are N, O and E");
		i = (int)(this-parity)>>1;
		if (i==2) i++; /* N=00, O=01, E=11 */
		bsect.par_1.ser_param &= ~(i&1); /* 7 bits if parity specified */
		bsect.par_1.ser_param |= i<<3;   /* specify parity */
	/* check if number of bits is there */
		if (serial[1]) {
		    if (serial[1] != '7' && serial[1] != '8')
			die("Only 7 or 8 bits supported");
		    if (serial[1]=='7')	bsect.par_1.ser_param &= 0xFE;
		    else bsect.par_1.ser_param |= 0x01;
		    
		    if (serial[2]) die("Synax error in SERIAL");
		}
	    }
	    if (verbose>=4) printf("Serial Param = 0x%02X\n", 
	                                        (int)bsect.par_1.ser_param);
	}
	if (delay < 20 && !cfg_get_flag(cf_options,"prompt")) {
	    fprintf(errstd,"Setting DELAY to 20 (2 seconds)\n");
	    delay = 20;
	}
    }
    bsect.par_1.prompt = cfg_get_flag(cf_options,"prompt") ? FLAG_PROMPT : 0;
    bsect.par_1.prompt |= raid_flags;
    bsect.par_1.raid_offset = raid_offset;  /* to be modified in bsect_raid_update */
/* convert timeout in tenths of a second to clock ticks    */
/* tick interval is 54.925 ms  */
/*   54.925 * 40 -> 2197       */
/*  100 * 40 -> 4000	       */
#if 0
#define	tick(x) ((x)*100/55)
#else
#define tick(x) ((x)*4000/2197)
#endif
    if (tick(delay) > 0xffff) die("Maximum delay is 59:59 (3599.5secs).");
	else bsect.par_1.delay = tick(delay);
    if (timeout == -1) bsect.par_1.timeout = 0xffff;
    else if (tick(timeout) >= 0xffff) die("Maximum timeout is 59:59 (3599.5secs).");
	else bsect.par_1.timeout = tick(timeout);
    if (!(keytable = cfg_get_strg(cf_options,"keytable"))) {
	for (i = 0; i < 256; i++) table[i] = i;
    }
    else {
	if ((kt_fd = open(keytable,O_RDONLY)) < 0)
	    die("open %s: %s",keytable,strerror(errno));
	if (read(kt_fd,table,256) != 256)
	    die("%s: bad keyboard translation table",keytable);
	(void) close(kt_fd);
    }
    menu = (MENUTABLE*)&table[256];
    memset(menu, 0, 256);
    if ((scheme = cfg_get_strg(cf_options,"menu-scheme"))) {
	if (!(stage_flags & STAGE_FLAG_MENU) && !nowarn)
	    fprintf(errstd,"Warning: 'menu-scheme' not supported by boot loader\n");
    	menu_do_scheme(scheme, menu);
    }
    if ((scheme = cfg_get_strg(cf_options,"menu-title"))) {
	if (!(stage_flags & STAGE_FLAG_MENU) && !nowarn)
	    fprintf(errstd,"Warning: 'menu-title' not supported by boot loader\n");
	if (strlen(scheme) > MAX_MENU_TITLE && !nowarn)
	    fprintf(errstd,"Warning: menu-[Atitle is > %d characters\n", MAX_MENU_TITLE);
    	strncpy(menu->title, scheme, MAX_MENU_TITLE);
    	menu->len_title = strlen(menu->title);
    }
    if ((scheme = cfg_get_strg(cf_options,"bmp-table"))) {
	if (!(stage_flags & STAGE_FLAG_BMP4) && !nowarn)
	    fprintf(errstd,"Warning: 'bmp-table' not supported by boot loader\n");
    }
    bmp_do_table(scheme, menu);
    if ((scheme = cfg_get_strg(cf_options,"bmp-colors"))) {
	if (!(stage_flags & STAGE_FLAG_BMP4) && !nowarn)
	    fprintf(errstd,"Warning: 'bmp-colors' not supported by boot loader\n");
    }
    bmp_do_colors(scheme, menu);
    if ((scheme = cfg_get_strg(cf_options,"bmp-timer"))) {
	if (!(stage_flags & STAGE_FLAG_BMP4) && !nowarn)
	    fprintf(errstd,"Warning: 'bmp-timer' not supported by boot loader\n");
    }
    bmp_do_timer(scheme, menu);
    map_begin_section();
    map_add_sector(table);
    (void) map_write(&bsect.par_1.keytab,1,0);
    memset(&descrs,0,SECTOR_SIZE*MAX_DESCR_SECS);
    if (cfg_get_strg(cf_options,"default")) image = image_base = 1;
}


static int dev_number(char *dev)
{
    struct stat st;

    if (stat(dev,&st) >= 0) return st.st_rdev;
    return to_number(dev);
}


static int get_image(char *name,const char *label,IMAGE_DESCR *descr)
{
    char *here,*deflt;
    int this_image,other;

    if (!label) {
        here = strrchr(label = name,'/');
        if (here) label = here+1;
    }
    if (strlen(label) > MAX_IMAGE_NAME) die("Label \"%s\" is too long",label);
    for (other = image_base; other <= image; other++) {
#ifdef LCF_IGNORECASE
	if (!strcasecmp(label,descrs.d.descr[other].name))
#else
	if (!strcmp(label,descrs.d.descr[other].name))
#endif
	    die("Duplicate label \"%s\"",label);
	if ((((descr->flags & FLAG_SINGLE) && strlen(label) == 1) ||
          (((descrs.d.descr[other].flags) & FLAG_SINGLE) &&
	  strlen(descrs.d.descr[other].name) == 1)) &&
#ifdef LCF_IGNORECASE
	  toupper(*label) == toupper(*descrs.d.descr[other].name))
#else
	  *label == *descrs.d.descr[other].name)
#endif
	    die("Single-key clash: \"%s\" vs. \"%s\"",label,
	      descrs.d.descr[other].name);
    }

    if (image_base && (deflt = cfg_get_strg(cf_options,"default")) &&
#ifdef LCF_IGNORECASE
      !strcasecmp(deflt,label))
#else
      !strcmp(deflt,label))
#endif
	this_image = image_base = 0;
    else {
	if (image == MAX_IMAGES)
	    die("Only %d image names can be defined",MAX_IMAGES);
	if (image >= image_menu_space)
	    die("'bmp-table=' has space for only %d images",
	    			image_menu_space);
	this_image = image++;
    }
    descrs.d.descr[this_image] = *descr;
    strcpy(descrs.d.descr[this_image].name,label);
    return this_image;
}


static char options[SECTOR_SIZE]; /* this is ugly */


static void bsect_common(IMAGE_DESCR *descr)
{
    struct stat st;
    char *here,*root,*ram_disk,*vga,*password;
    char *literal,*append,*fback;
    char fallback_buf[SECTOR_SIZE];
    int i;

    for (i=0; i<nelem((descr->password_crc)); i++) descr->password_crc[i] = 0;
    descr->flags = 0;
    memset(fallback_buf,0,SECTOR_SIZE);
    memset(options,0,SECTOR_SIZE);
    if ((cfg_get_flag(cf_kernel,"read-only") && cfg_get_flag(cf_kernel,
      "read-write")) || (cfg_get_flag(cf_options,"read-only") && cfg_get_flag(
      cf_options,"read-write")))
	die("Conflicting READONLY and READ_WRITE settings.");
    if (cfg_get_flag(cf_kernel,"read-only") || cfg_get_flag(cf_options,
      "read-only")) strcat(options,"ro ");
    if (cfg_get_flag(cf_kernel,"read-write") || cfg_get_flag(cf_options,
      "read-write")) strcat(options,"rw ");
    if ((root = cfg_get_strg(cf_kernel,"root")) || (root = cfg_get_strg(
      cf_options,"root")))  {
	if (strcasecmp(root,"current")) 
	    sprintf(strchr(options,0),"root=%x ",dev_number(root));
	else {
	    if (stat("/",&st) < 0) pdie("stat /");
	    sprintf(strchr(options,0),"root=%x ",(unsigned int) st.st_dev);
	}
      }	
    if ((ram_disk = cfg_get_strg(cf_kernel,"ramdisk")) || (ram_disk =
      cfg_get_strg(cf_options,"ramdisk")))
	sprintf(strchr(options,0),"ramdisk=%d ",to_number(ram_disk));
    if (cfg_get_flag(cf_kernel,"lock") || cfg_get_flag(cf_options,"lock"))
#ifdef LCF_READONLY
	die("This LILO is compiled READONLY and doesn't support the LOCK "
	  "option");
#else
	descr->flags |= FLAG_LOCK;
#endif
    if ((vga = cfg_get_strg(cf_kernel,"vga")) || (vga = cfg_get_strg(cf_options,
      "vga"))) {
#ifndef NORMAL_VGA
	if (!nowarn)
	    fprintf(errstd,"Warning: VGA mode presetting is not supported; ignoring 'vga='\n");
#else
	descr->flags |= FLAG_VGA;
	     if (!strcasecmp(vga,"normal")) descr->vga_mode = NORMAL_VGA;
	else if (!strcasecmp(vga,"ext") || !strcasecmp(vga,"extended"))
		descr->vga_mode = EXTENDED_VGA;
	else if (!strcasecmp(vga,"ask")) descr->vga_mode = ASK_VGA;
	else descr->vga_mode = to_number(vga);
#endif
    }
    if ((cfg_get_flag(cf_options,"restricted") && 
             cfg_get_flag(cf_options,"mandatory")) ||
        (cfg_get_flag(cf_all,"restricted") && 
             cfg_get_flag(cf_all,"mandatory")))
         die("MANDATORY and RESTRICTED are mutually exclusive");
    if (cfg_get_flag(cf_all,"bypass")) {
        if (cfg_get_flag(cf_all,"mandatory"))
             die("MANDATORY and BYPASS are mutually exclusive");
        if (cfg_get_flag(cf_all,"restricted"))
             die("RESTRICTED and BYPASS are mutually exclusive");
        if (!cfg_get_strg(cf_options,"password"))
             die("BYPASS only valid if global PASSWORD is set");
    }
    if ((password = cfg_get_strg(cf_all,"password")) && cfg_get_flag(cf_all,"bypass"))
        die("PASSWORD and BYPASS not valid together");
    if (password || 
        ( (password = cfg_get_strg(cf_options,"password")) &&
          !cfg_get_flag(cf_all,"bypass")  ) ) {
	if (!*password) {	/* null password triggers interaction */
	    retrieve_crc((long*)descr->password_crc);
	} else {
	    hash_password(password, (long*)descr->password_crc );
	}
	descr->flags |= FLAG_PASSWORD;
    }
#if 1
    if (cfg_get_flag(cf_all,"mandatory") || cfg_get_flag(cf_options,
      "mandatory")) {
	if (!password) die("MANDATORY is only valid if PASSWORD is set.");
    }
    if (cfg_get_flag(cf_all,"restricted") || cfg_get_flag(cf_options,
      "restricted")) {
	if (!password) die("RESTRICTED is only valid if PASSWORD is set.");
	if ((descr->flags & FLAG_PASSWORD) && !cfg_get_flag(cf_all,"mandatory"))
	    descr->flags |= FLAG_RESTR;
    }
    if (password && *password && config_read) {
	fprintf(errstd,"Warning: %s should be readable only "
	  "for root if using PASSWORD\n",config_file);
	config_read = 0;	/* suppress further warnings */
    }
#else
    if (cfg_get_flag(cf_all,"restricted") || cfg_get_flag(cf_options,
      "restricted")) {
	if (!password) die("RESTRICTED is only valid if PASSWORD is set.");
	descr->flags |= FLAG_RESTR;
    }
#endif
    if (cfg_get_flag(cf_all,"single-key") ||
      cfg_get_flag(cf_options,"single-key")) descr->flags |= FLAG_SINGLE;
#ifdef LCF_BOOT_FILE
    if ((append = cfg_get_strg(cf_top, "image"))) {
	strcat(options, "BOOT_FILE=");
	strcat(options, append);
	strcat(options, " ");
    }
#endif
    if ((append = cfg_get_strg(cf_kernel,"append")) || (append =
      cfg_get_strg(cf_options,"append"))) strcat(options,append);
    literal = cfg_get_strg(cf_kernel,"literal");
    if (literal) strcpy(options,literal);
    if (*options) {
	here = strchr(options,0);
	if (here[-1] == ' ') here[-1] = 0;
    }
    fback = cfg_get_strg(cf_kernel,"fallback");
    if (fback) {
#ifdef LCF_READONLY
	die("This LILO is compiled READONLY and doesn't support the FALLBACK "
	  "option");
#else
	if (descr->flags & FLAG_LOCK)
	    die("LOCK and FALLBACK are mutually exclusive");
	else descr->flags |= FLAG_FALLBACK;
	*(unsigned short *) fallback_buf = DC_MAGIC;
	strcpy(fallback_buf+2,fback);
	fallback[fallbacks++] = stralloc(fback);
#endif
    }
    *(unsigned long *) descr->rd_size = 0; /* no RAM disk */
    descr->start_page = 0; /* load low */
    map_begin_section();
    map_add_sector(fallback_buf);
    map_add_sector(options);
}


static void bsect_done(char *name,IMAGE_DESCR *descr)
{
    char *alias;
    int this_image,this;

    if (!*name) die("Invalid image name.");
    alias = cfg_get_strg(cf_all,"alias");
    this = alias ? get_image(NULL,alias,descr) : -1;
    this_image = get_image(name,cfg_get_strg(cf_all,"label"),descr);
    if ((descr->flags & FLAG_SINGLE) &&
      strlen(descrs.d.descr[this_image].name) > 1 &&
      (!alias || strlen(alias) > 1))
	die("SINGLE-KEYSTROKE requires the label or the alias to be only "
	  "a single character");
    if (verbose >= 0) {
	printf("Added %s",descrs.d.descr[this_image].name);
	if (alias) printf(" (alias %s)",alias);
	if (this_image && this) putchar('\n');
	else printf(" *\n");
    }
    if (verbose >= 3) {
	printf("%4s<dev=0x%02x,hd=%d,cyl=%d,sct=%d>\n","",
	  descr->start.device,descr->start.head,descr->start.track,
	  descr->start.sector);
	if (*options) printf("%4s\"%s\"\n","",options);
    }
    if (verbose >= 1) putchar('\n');   /* makes for nicer spacing */
}


int bsect_number(void)
{
    return image_base ? 0 : image;
}


static void unbootable(void)
{
    fflush(stdout);
    fprintf(errstd,"\nWARNING: The system is unbootable !\n");
    fprintf(errstd,"%9sRun LILO again to correct this.","");
    exit(1);
}


void check_fallback(void)
{
    char *start,*end;
    int i,image;

    for (i = 0; i < fallbacks; i++) {
	for (start = fallback[i]; *start && *start == ' '; start++);
	if (*start) {
	    for (end = start; *end && *end != ' '; end++);
	    if (*end) *end = 0;
	    for (image = 0; image < MAX_IMAGES; image++)
#ifdef LCF_IGNORECASE
		if (!strcasecmp(descrs.d.descr[image].name,start)) break;
#else
		if (!strcmp(descrs.d.descr[image].name,start)) break;
#endif
	    if (image == MAX_IMAGES) die("No image \"%s\" is defined",start);
	}
    }
}


void bsect_update(char *backup_file, int force_backup, int pass)
{
    struct stat st;
    char temp_name[PATH_MAX+1];
    int bck_file;

    if (!backup_file) {
	sprintf(temp_name,BACKUP_DIR "/boot.%04X",boot_dev_nr);
	backup_file = temp_name;
    }
    bck_file = open(backup_file,O_RDONLY);
    if (bck_file >= 0 && force_backup) {
	(void) close(bck_file);
	bck_file = -1;
    }
    if (bck_file >= 0) {
	if (verbose > 0)
	    printf("%s exists - no backup copy made.\n",backup_file);
    }
    else {
	if ((bck_file = creat(backup_file,0644)) < 0)
	    die("creat %s: %s",backup_file,strerror(errno));
	if (write(bck_file,(char *) &bsect_orig,SECTOR_SIZE) != SECTOR_SIZE)
	    die("write %s: %s",backup_file,strerror(errno));
	if (verbose > 0)
	    printf("Backup copy of boot sector in %s\n",backup_file);
	if (fstat(bck_file,&st) < 0)
	    die("fstat %s: %s",backup_file,strerror(errno));
	bsect.par_1.timestamp = st.st_mtime;
    }
    if (close(bck_file) < 0) die("close %s: %s",backup_file,strerror(errno));
    if (pass==0) {
	map_begin_section();
	map_add_sector(secondary_map);
	(void) map_write(&bsect.par_1.secondary,1,0);

	map_done(&descrs,bsect.par_1.descr);
    }
    if (lseek(fd,0,0) < 0)
	die("lseek %s: %s",boot_devnam ? boot_devnam : dev.name,
	  strerror(errno));
    if (verbose > 0) printf("Writing boot sector.\n");
 /* failsafe check */
    if (memcmp(bsect.sector+MAX_BOOT_SIZE, bsect_orig.sector+MAX_BOOT_SIZE, 64+8))
    	die("LILO internal error:  Would overwrite Partition Table");
#if 1
    if ( MAJOR(boot_dev_nr)==MAJOR_FD  &&
    	  bsect.par_1.cli == 0xFA  &&  bsect.par_1.call_ins == 0xE8 ) {
/* perform the relocation of the boot sector */
	int len = bsect.par_1.code_length;
	int space = BOOT_SIG_OFFSET - len;
	space &= 0xFFF0;	/* roll back to paragraph boundary */
	memmove(&bsect_orig.sector[space], &bsect, len);
	if (space <= 0x80) {
	    bsect_orig.sector[0] = 0xEB;		/* jmp short */
	    bsect_orig.sector[1] = space - 2;
	    bsect_orig.sector[2] = 0x90;		/* nop */
	} else {
	    bsect_orig.sector[0] = 0xE9;		/* jmp near */
	    *(short*)&bsect_orig.sector[1] = space - 3;
	}
	bsect = bsect_orig;
	if (verbose >= 1) printf("Boot sector relocation performed\n");
    }
#endif    	
    if (write(fd,(char *) &bsect,SECTOR_SIZE) != SECTOR_SIZE)
	die("write %s: %s",boot_devnam ? boot_devnam : dev.name,
	  strerror(errno));
    if (!boot_devnam) dev_close(&dev);
    else if (close(fd) < 0) {
	    unbootable();
	    die("close %s: %s",boot_devnam,strerror(errno));
	}
    if (pass==0) {
	pw_file_update(passw);
	temp_unregister(temp_map);
	if (rename(temp_map,map_name) < 0) {
	    unbootable();
	    die("rename %s %s: %s",temp_map,map_name,strerror(errno));
	}
    }
    (void) sync();
}


void bsect_cancel(void)
{
    map_done(&descrs,bsect.par_1.descr);
    if (boot_devnam) (void) close(fd);
    else dev_close(&dev);
    temp_unregister(temp_map);
    if (verbose<9) (void) remove(temp_map);
}


static int present(char *var)
{
    char *path;

    if (!(path = cfg_get_strg(cf_top,var))) die("No variable \"%s\"",var);
    if (!access(path,F_OK)) return 1;
    if (!cfg_get_flag(cf_all,"optional") && !cfg_get_flag(cf_options,
      "optional")) return 1;
    if (verbose >= 0) printf("Skipping %s\n",path);
    return 0;
}


void do_image(void)
{
    IMAGE_DESCR descr;
    char *name;

    cfg_init(cf_image);
    (void) cfg_parse(cf_image);
    if (present("image")) {
	bsect_common(&descr);
	descr.flags |= FLAG_KERNEL;
	name = cfg_get_strg(cf_top,"image");
	if (!cfg_get_strg(cf_image,"range")) boot_image(name,&descr);
	else boot_device(name,cfg_get_strg(cf_image,"range"),&descr);
	bsect_done(name,&descr);
    }
    cfg_init(cf_top);
}


void do_other(void)
{
    IMAGE_DESCR descr;
    char *name, *loader;

    cfg_init(cf_other);
    cfg_init(cf_kernel); /* clear kernel parameters */
    curr_drv_map = curr_prt_map = 0;
    (void) cfg_parse(cf_other);
    if (present("other")) {
	bsect_common(&descr);
	name = cfg_get_strg(cf_top,"other");
	loader = cfg_get_strg(cf_other,"loader");
	if (!loader) loader = cfg_get_strg(cf_options,"loader");
	boot_other(loader,name,cfg_get_strg(cf_other,"table"),&descr);
	bsect_done(name,&descr);
    }
    cfg_init(cf_top);
}


void bsect_uninstall(char *boot_dev,char *backup_file,int validate)
{
    struct stat st;
    char temp_name[PATH_MAX+1];
    int bck_file;

    open_bsect(boot_dev);
    if (*(unsigned short *) &bsect.sector[BOOT_SIG_OFFSET] != BOOT_SIGNATURE)
	die("Boot sector of %s does not have a boot signature",boot_dev ?
	  boot_dev : dev.name);
    if (!strncmp(bsect.par_1.signature-4,"LILO",4))
	die("Boot sector of %s has a pre-21 LILO signature",boot_dev ?
	  boot_dev : dev.name);
    if (strncmp(bsect.par_1.signature,"LILO",4))
	die("Boot sector of %s doesn't have a LILO signature",boot_dev ?
	  boot_dev : dev.name);
    if (!backup_file) {
	sprintf(temp_name,BACKUP_DIR "/boot.%04X",boot_dev_nr);
	backup_file = temp_name;
    }
    if ((bck_file = open(backup_file,O_RDONLY)) < 0)
	die("open %s: %s",backup_file,strerror(errno));
    if (fstat(bck_file,&st) < 0)
	die("fstat %s: %s",backup_file,strerror(errno));
    if (validate && st.st_mtime != bsect.par_1.timestamp)
	die("Timestamp in boot sector of %s differs from date of %s\n"
	  "Try using the -U option if you know what you're doing.",boot_dev ?
	  boot_dev : dev.name,backup_file);
    if (verbose > 0) printf("Reading old boot sector.\n");
    if (read(bck_file,(char *) &bsect,PART_TABLE_OFFSET) != PART_TABLE_OFFSET)
	die("read %s: %s",backup_file,strerror(errno));
    if (lseek(fd,0,0) < 0)
	die("lseek %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
    if (verbose > 0) printf("Restoring old boot sector.\n");
    if (write(fd,(char *) &bsect,PART_TABLE_OFFSET) != PART_TABLE_OFFSET)
	die("write %s: %s",boot_dev ? boot_dev : dev.name,strerror(errno));
    if (!boot_devnam) dev_close(&dev);
    else if (close(fd) < 0) {
	    unbootable();
	    die("close %s: %s",boot_devnam,strerror(errno));
	}
    exit(0);
}


void bsect_raid_update(char *boot_dev, unsigned long raid_offset, 
	char *backup_file, int force_backup, int pass)
{
    BOOT_SECTOR bsect_save;

    if (pass > 0) {    
	bsect_save = bsect;			/* save the generated boot sector */
        open_bsect(boot_dev);
        memcpy(&bsect, &bsect_save, MAX_BOOT_SIZE);	/* update the subject boot sector */
        bsect.par_1.raid_offset = raid_offset;	/* put in the new partition offset */
        bsect.par_1.prompt &= FLAG_PROMPT;	/* clear all flags but PROMPT */
        bsect.par_1.prompt |= raid_flags;	/* update the raid flags */
        *(unsigned short *) &bsect.sector[BOOT_SIG_OFFSET] = BOOT_SIGNATURE;
    }
    bsect_update(backup_file, force_backup, pass);
}


