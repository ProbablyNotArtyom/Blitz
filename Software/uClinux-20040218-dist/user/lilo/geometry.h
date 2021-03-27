/* geometry.h  -  Device and file geometry computation */

/* Copyright 1992-1995 Werner Almesberger. See file COPYING for details. */


#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "lilo.h"


typedef struct {
    int device,heads;
    int cylinders,sectors;
    int start; /* partition offset */
    int spb; /* sectors per block */
    int fd,file;
    int boot; /* non-zero after geo_open_boot */
} GEOMETRY;

typedef struct _dt_entry {
    int device,bios;
    int sectors;
    int heads; /* 0 if inaccessible */
    int cylinders;
    int start;
    struct _dt_entry *next;
} DT_ENTRY;

extern DT_ENTRY *disktab;


int has_partitions(dev_t dev);

/* indicates that the specified device is a block hard disk device */
/* returns the partition mask or 0 */

void geo_init(char *name);

/* Loads the disk geometry table. */

int is_first(int device);

/* Returns a non-zero integer if the specified device could be the first (i.e.
   boot) disk, zero otherwise. */

void geo_get(GEOMETRY *geo,int device,int user_device,int all);

/* Obtains geometry information of the specified device. Sets the BIOS device
   number to user_device unless -1. If ALL is zero, only the BIOS device number
   is retrieved and the other geometry information is undefined. */

int geo_open(GEOMETRY *geo,char *name,int flags);

/* Opens the specified file or block device, obtains the necessary geometry
   information and returns the file descriptor. If the name contains a BIOS
   device specification (xxx:yyy), it is removed and stored in the geometry
   descriptor. Returns the file descriptor of the opened object. */

int geo_open_boot(GEOMETRY *geo,char *name);

/* Like get_open, but only the first sector of the device is accessed. This
   way, no geometry information is needed. */

void geo_close(GEOMETRY *geo);

/* Closes a file or device that has previously been opened by geo_open. */

int geo_comp_addr(GEOMETRY *geo,int offset,SECTOR_ADDR *addr);

/* Determines the address of the disk sector that contains the offset'th
   byte of the specified file or device. Returns a non-zero value if such
   a sector exists, zero if it doesn't. */

int geo_find(GEOMETRY *geo,SECTOR_ADDR addr);

/* lseeks in the file associated with GEO for the sector at address ADDR.
   Returns a non-zero integer on success, zero on failure. */

/* static void geo_query_dev(GEOMETRY *geo,int device,int all);  */

/* opens the specified device and gets the geometry information.  That
   information is then stored in *geo */

#endif
