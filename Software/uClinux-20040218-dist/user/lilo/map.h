/* map.h  -  Map file creation */

/* Copyright 1992-1996 Werner Almesberger. See file COPYING for details. */


#ifndef MAP_H
#define MAP_H

#include "common.h"
#include "geometry.h"


void map_patch_first(char *name,char *str);

/* Puts str into the first sector of a map file. */

void map_create(char *name);

/* Create and initialize the specified map file. */

void map_done(DESCR_SECTORS *descr,SECTOR_ADDR addr[3]);

/* Updates and closes the map file. */

void map_add_sector(void *sector);

/* Adds the specified sector to the map file and registers it in the map
   section. */

void map_begin_section(void);

/* Begins a map section. Note: maps can also be written to memory with 
   map_write. Thus, the map routines can be used even without map_create. */

void map_add(GEOMETRY *geo,int from,int num_sect);

/* Adds pointers to sectors from the specified file to the map file, starting
   "from" sectors from the beginning. */

void map_add_zero(void);

/* Adds a zero-filled sector to the current section. */

int map_end_section(SECTOR_ADDR *addr,int dont_compact);

/* Writes a map section to the map file and returns the address of the first
   sector of that section. The first DONT_COMPACT sectors are never compacted.
   Returns the number of sectors that have been mapped. */

int map_write(SECTOR_ADDR *list,int max_len,int terminate);

/* Writes a map section to an array. If terminate is non-zero, a terminating
   zero entry is written. If the section (including the terminating zero entry)
   exceeds max_len sectors, map_write dies. */

#endif
