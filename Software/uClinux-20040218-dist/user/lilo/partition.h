/* partition.h  -  Partition table handling */

/* Copyright 1992-1996 Werner Almesberger. See file COPYING for details. */


#ifndef PARTITION_H
#define PARTITION_H

#include <unistd.h>
#include <linux/unistd.h>

typedef struct _change_rule {
    const char *type;
    unsigned char normal;
    unsigned char hidden;
    struct _change_rule *next;
} CHANGE_RULE;

#if 1

#define SECTORSIZE ((long long)SECTOR_SIZE)

       loff_t llseek(unsigned int fd, loff_t offs, unsigned int whence);

#endif

#define is_extd_part(x) ((x)==PART_DOS_EXTD||(x)==PART_WIN_EXTD_LBA||(x)==PART_LINUX_EXTD)

void part_verify(int dev_nr,int type);
/* Verify the partition table of the disk of which dev_nr is a partition. May
   also try to "fix" a partition table. Fail on non-Linux partitions if the
   TYPE flag is non-zero (unless IGNORE-TABLE is set too). */

void do_cr_auto(void);
/* do automatic change-rules */

void preload_types(void);
/* Preload some partition types for convenience */

void do_activate(char *where, char *which);
/* Activate the specified partition */

void do_install_mbr(char *where, char *what);
/* Install a new MBR (Master Boot Record) */


#endif
