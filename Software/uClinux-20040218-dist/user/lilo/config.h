/* config.h  -  Configurable parameters */

/* Copyright 1992-1995 Werner Almesberger. See file COPYING for details. */


#ifndef CONFIG_H
#define CONFIG_H

#define TMP_DEV     "/tmp/dev.%d" /* temporary devices are created here */
#define MAX_TMP_DEV 50 /* highest temp. device number */

#ifdef LCF_OLD_DIRSTR
#define LILO_DIR    "/etc/lilo" /* base directory for LILO files */
#define BACKUP_DIR  LILO_DIR /* boot sector and partition table backups */
#define DFL_CONFIG  LILO_DIR "/config" /* default configuration file */
#define DFL_DISKTAB LILO_DIR "/disktab" /* LILO's disk parameter table */
#define MAP_FILE    LILO_DIR "/map" /* default map file */
#define MAP_TMP_APP "~" /* temporary file appendix */
#define DFL_BOOT    LILO_DIR "/boot.b" /* default boot loader */
#define DFL_CHAIN   LILO_DIR "/chain.b" /* default chain loader */
#define DFL_MBR	    LILO_DIR "/mbr.b"	/* default MBR */
#else
#define CFG_DIR	    "/etc"		/* location of configuration files */
#define BOOT_DIR    "/boot"		/* location of boot files */
#define BACKUP_DIR  BOOT_DIR /* boot sector and partition table backups */
#define DFL_CONFIG  CFG_DIR "/lilo.conf"/* default configuration file */
#define DFL_DISKTAB CFG_DIR "/disktab"	/* LILO's disk parameter table */
#define MAP_FILE    BOOT_DIR "/map"	/* default map file */
#define MAP_TMP_APP "~"			/* temporary file appendix */
#define	DFL_BOOT    BOOT_DIR "/boot.b"	/* default boot loader */
#define DFL_CHAIN   BOOT_DIR "/chain.b"	/* default chain loader */
#define DFL_MBR	    BOOT_DIR "/mbr.b"	/* default MBR */
#endif

#define DEV_DIR	    "/dev" /* devices directory */

#define MAX_LINE    1024 /* maximum disk parameter table line length */

#endif
