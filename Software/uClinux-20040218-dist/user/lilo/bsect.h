/* bsect.h  -  Boot sector handling */

/* Copyright 1992-1995 Werner Almesberger. See file COPYING for details. */


#ifndef BSECT_H
#define BSECT_H

void bsect_read(char *boot_dev,BOOT_SECTOR *buffer);

/* Read the boot sector stored on BOOT_DEV into BUFFER. */

void bsect_open(char *boot_dev,char *map_file,char *install,int delay,
  int timeout, long raid_offset);

/* Loads the boot sector of the specified device and merges it with a new
   boot sector (if install != NULL). Sets the boot delay to 'delay' 1/10 sec.
   Sets the input timeout to 'timeout' 1/10 sec (no timeout if -1). Creates a
   temporary map file. */

int bsect_number(void);

/* Returns the number of successfully defined boot images. */

void check_fallback(void);

/* Verifies that all fallback options specify valid images. */

void bsect_update(char *backup_file, int force_backup, int pass);

/* Updates the boot sector and the map file. */

void bsect_raid_update(char *boot_dev, unsigned long raid_offset, 
	char *backup_file, int force_backup, int pass);

/* Update the boot sector and the map file, with RAID considerations */

void bsect_cancel(void);

/* Cancels all changes. (Deletes the temporary map file and doesn't update
   the boot sector. */

void do_image(void);

/* Define a "classic" boot image. (Called from the configuration parser.) */

void do_unstripped(void);

/* Define an unstripped kernel. */

void do_other(void);

/* Define an other operating system. */

void bsect_uninstall(char *boot_dev_name,char *backup_file,int validate);

/* Restores the backed-up boot sector of the specified device. If
   'boot_dev_name' is NULL, the current root device is used. If 'backup_file'
   is NULL, the default backup file is used. A time stamp contained in the
   boot sector is verified if 'validate' is non-zero. */

#endif
