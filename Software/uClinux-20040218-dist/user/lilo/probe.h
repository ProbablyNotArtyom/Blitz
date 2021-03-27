/* probe.h  -- definitions for the LILO probe utility */

#ifndef __PROBE_H_
#define __PROBE_H_

#include "lilo.h"


struct disk_geom {
   unsigned long n_total_blocks;
   int n_sect;
   int n_head;
   int n_cyl;
   char type;
   char EDD_flags;
   char EDD_rev;
   char n_disks;
   struct partition *pt;
   int serial_no;		/* added at PROBE_VERSION==4 */
};

#if 0
/* structure used by int 0x13, AH=0x48 */

struct disk_param {
   short size;
   short flags;
   unsigned long n_cyl;
   unsigned long n_head;
   unsigned long n_sect;
   long long n_sectors;
   short n_byte;
   unsigned long edd_config_ptr;
};
#endif


#define EDD_DMA_BOUNDARY_TRANSP  01
#define EDD_PARAM_GEOM_VALID     02


/* the following structures are created by the biosdata.S codes */

typedef
struct Video {
   unsigned short equipment;
   unsigned short mem;

/* BD_GET_VIDEO >= 1 */
   struct {
      unsigned char  al;
      unsigned char  ah;
      unsigned char  bl;
      unsigned char  bh;
   } vid0F;

/* BD_GET_VIDEO >= 2  */
#if 1
   struct {
      unsigned short ax;
      unsigned char  bl;
      unsigned char  bh;
   } vid12;
#endif
   struct {
      unsigned char  al;
      unsigned char  ah;
      unsigned short bx;
   } vid1A;


/* BD_GET_VIDEO >= 3  */
   struct {
      unsigned short ax;
      unsigned char  sig[4];
   } vid4F00;
   struct {
      unsigned short ax;
      unsigned short bits;
   } vid101;
   struct {
      unsigned short ax;
      unsigned short bits;
   } vid103;
} video_t;

typedef
struct Floppy {
   struct {
      unsigned char  ah;		/* AL and AH were swapped */
      unsigned char  flags;
      unsigned short dx;
      unsigned short cx;
   } fn15;
   struct {
      unsigned char  ah;		/* AL and AH were swapped */
      unsigned char  flags;
      unsigned short cx;
      unsigned short dx;
      unsigned short di;
      unsigned short es;
   } fn08;
} floppy_t;

typedef
struct Hard {
   struct {
      unsigned char  ah;		/* AL and AH were swapped */
      unsigned char  flags;
      unsigned short dx;
      unsigned short cx;
   } fn15;
   struct {
      unsigned char  ah;		/* AL and AH were swapped */
      unsigned char  flags;
      unsigned short cx;
      unsigned short dx;
   } fn08;
   struct {
      unsigned char  ah;		/* AL and AH were swapped */
      unsigned char  flags;
      unsigned short bx;
      unsigned short cx;
   } fn41;
} hard_t;

typedef
struct Fn48 {
   unsigned char  ah;		/* AL and AH were swapped */
   unsigned char  flags;
} fn48_t;

typedef
struct Edd {
   unsigned short size;			/* 26 or 30 */
   unsigned short info;
   unsigned long  cylinders;
   unsigned long  heads;
   unsigned long  sectors;
   long long      total_sectors;
   unsigned short sector_size;

   unsigned short offset,
   		  segment;
           fn48_t reg;		/* AH & flags returned from the call */
} edd_t;				/* struct is 26; but may be 30 in mem */


void probe_tell (char *cmd);

int bios_max_devs(void);

int bios_device(GEOMETRY *geo, int device);


#endif
/* end probe.h */
