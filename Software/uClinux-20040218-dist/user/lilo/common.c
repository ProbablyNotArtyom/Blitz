/* common.c  -  Common data structures and functions. */
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
#include <stdarg.h>

#include "common.h"
#include "lilo.h"


int verbose = 0,test = 0,compact = 0,linear = 0, raid_flags = 0, zflag = 0,
      nowarn = 0, lba32 = 0, autoauto = 0, passw = 0, geometric = 0;

unsigned short drv_map[DRVMAP_SIZE+1]; /* fixup maps ... */
int curr_drv_map;
unsigned long prt_map[PRTMAP_SIZE+1];
int curr_prt_map;
#if 0
unsigned long crc_polynomial[MAX_PW_CRC] = { 
	CRC_POLY1, CRC_POLY2, CRC_POLY3, CRC_POLY4 };
#endif	

/*volatile*/ void pdie(char *msg)
{
    fflush(stdout);
    perror(msg);
    exit(1);
}


/*volatile*/ void die(char *fmt,...)
{
    va_list ap;

    fflush(stdout);
    fprintf(errstd,"Fatal: ");       /* JRC */
    va_start(ap,fmt);
    vfprintf(errstd,fmt,ap);
    va_end(ap);
    fputc('\n',errstd);
    exit(1);
}


void *alloc(int size)
{
    void *this;

    if ((this = malloc(size)) == NULL) pdie("Out of memory");
    return this;
}


void *ralloc(void *old,int size)
{
    void *this;

    if ((this = realloc(old,size)) == NULL) pdie("Out of memory");
    return this;
}


char *stralloc(const char *str)
{
    char *this;

    if ((this = strdup(str)) == NULL) pdie("Out of memory");
    return this;
}


int to_number(char *num)
{
    int number;
    char *end;

    number = strtol(num,&end,0);
    if (end && *end) die("Not a number: \"%s\"",num);
    return number;
}


static char *name(int stage)
{
    switch (stage) {
	case STAGE_FIRST:
	    return "First boot sector";
	case STAGE_SECOND:
	    return "Second boot sector";
	case STAGE_CHAIN:
	    return "Chain loader";
	default:
	    die("Internal error: Unknown stage code %d",stage);
    }
    return NULL; /* for GCC */
}


void check_version(BOOT_SECTOR *sect,int stage)
{
    int bs_major, bs_minor;

    if (!strncmp(sect->par_1.signature-4,"LILO",4))
	die("%s has a pre-21 LILO signature",name(stage));
    if (strncmp(sect->par_1.signature,"LILO",4))
	die("%s doesn't have a valid LILO signature",name(stage));
    if ((sect->par_1.stage&0xFF) != stage)
	die("%s has an invalid stage code (%d)",name(stage),sect->par_1.stage);

    bs_major = sect->par_1.version & 255;
    bs_minor = sect->par_1.version >> 8;
    if (sect->par_1.version != VERSION)
	die("%s is version %d.%d. Expecting version %d.%d.",name(stage),
	    bs_major,bs_minor, VERSION_MAJOR,VERSION_MINOR);
}


int stat_equal(struct stat *a,struct stat *b)
{
    return a->st_dev == b->st_dev && a->st_ino == b->st_ino;
}

/* calculate a CRC-32 polynomial */

unsigned long crc32 (unsigned char *cp, int nsize, unsigned long polynomial)
{
   unsigned long poly, crc;
   int i;
   unsigned char ch;

   crc = 0xFFFFFFFFUL;
   while (nsize--) {
      ch = *cp++;
      for (i=0; i<8; i++) {
         if ( ( (crc>>31) ^ (ch>>(7-i)) ) & 1) poly = polynomial;
         else poly = 0UL;
         crc = (crc<<1) ^ poly;
      }
   }
   return ~crc;
}

