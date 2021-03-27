/* device.c  -  Device access */
/*
Copyright 1992-1997 Werner Almesberger.
Copyright 1999-2001 John Coffman.
All rights reserved.

Licensed under the terms contained in the file 'COPYING' in the 
source directory.

*/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "config.h"
#include "common.h"
#include "temp.h"
#include "device.h"


typedef struct _cache_entry {
    const char *name;
    int number;
    struct _cache_entry *next;
} CACHE_ENTRY;

typedef struct _st_buf {
    struct _st_buf *next;
    struct stat st;
} ST_BUF;


static CACHE_ENTRY *cache = NULL;


static int scan_dir(ST_BUF *next,DEVICE *dev,char *parent,int number)
{
    DIR *dp;
    struct dirent *dir;
    ST_BUF st,*walk;
    char *start;

    st.next = next;
    if ((dp = opendir(parent)) == NULL)
	die("opendir %s: %s",parent,strerror(errno));
    *(start = strchr(parent,0)) = '/';
    while ((dir = readdir(dp))) {
	strcpy(start+1,dir->d_name);
	if (stat(parent,&st.st) >= 0) {
	    dev->st = st.st;
	    if (S_ISBLK(dev->st.st_mode) && dev->st.st_rdev == number) {
		(void) closedir(dp);
		return 1;
	    }
	    if (S_ISDIR(dev->st.st_mode) && strcmp(dir->d_name,".") &&
	      strcmp(dir->d_name,"..")) {
		for (walk = next; walk; walk = walk->next)
		    if (stat_equal(&walk->st,&st.st)) break;
		if (!walk && scan_dir(&st,dev,parent,number)) {
		    (void) closedir(dp);
		    return 1;
		}
	    }
	}
    }
    (void) closedir(dp);
    *start = 0;
    return 0;
}


static int lookup_dev(char *name,DEVICE *dev,int number)
{
    CACHE_ENTRY **walk,*here;

    for (walk = &cache; *walk; walk = &(*walk)->next)
	if ((*walk)->number == number) {
	    if (stat((*walk)->name,&dev->st) >= 0)
		if (S_ISBLK(dev->st.st_mode) && dev->st.st_rdev == number) {
		    strcpy(name,(*walk)->name);
		    return 1;
		}
	    here = *walk; /* remove entry from cache */
	    if (verbose >= 2)
		printf("Invalidating cache entry for %s (0x%04X)\n",here->name,
		  here->number);
	    *walk = here->next;
	    free((char *) here->name);
	    free(here);
	    return 0;
	}
    return 0;
}


static void cache_add(const char *name,int number)
{
    CACHE_ENTRY *entry;

    if (verbose >= 4) printf("Caching device %s (0x%04X)\n",name,number);
    entry = alloc_t(CACHE_ENTRY);
    entry->name = stralloc(name);
    entry->number = number;
    entry->next = cache;
    cache = entry;
}


int dev_open(DEVICE *dev,int number,int flags)
{
    char name[PATH_MAX+1];
    ST_BUF st;
    int count;

    if (lookup_dev(name,dev,number)) dev->delete = 0;
    else {
	if (stat(DEV_DIR,&st.st) < 0)
	    die("stat " DEV_DIR ": %s",strerror(errno));
	st.next = NULL;
	dev->delete = !scan_dir(&st,dev,strcpy(name,DEV_DIR),number);
	if (dev->delete) {
	    for (count = 0; count <= MAX_TMP_DEV; count++) {
#ifdef LCF_USE_TMPDIR
		if (!strncmp(TMP_DEV,"/tmp/",5) && getenv("TMPDIR")) {
		    strcpy(name,getenv("TMPDIR"));
		    sprintf(name+strlen(name),TMP_DEV+4,count);
		}
		else
#endif
		    sprintf(name,TMP_DEV,count);
		if (stat(name,&dev->st) < 0) break;
	    }
	    if (count > MAX_TMP_DEV)
		die("Failed to create a temporary device");
	    if (mknod(name,0600 | S_IFBLK,number) < 0)
		die("mknod %s: %s",name,strerror(errno));
	    if (stat(name,&dev->st) < 0)
		die("stat %s: %s",name,strerror(errno));
	    if (verbose > 1)
		printf("Created temporary device %s (0x%04X)\n",name,number);
	    temp_register(name);
	}
	else cache_add(name,number);
    }
    if (flags == -1) dev->fd = -1;
    else if ((dev->fd = open(name,flags)) < 0)
	    die("open %s: %s",name,strerror(errno));
    dev->name = stralloc(name);
    return dev->fd;
}


void dev_close(DEVICE *dev)
{
    if (dev->fd != -1)
	if (close(dev->fd) < 0) die("close %s: %s",dev->name,strerror(errno));
    if (dev->delete) {
	if (verbose > 1)
	    printf("Removed temporary device %s (0x%04X)\n",dev->name,
	      (unsigned int) dev->st.st_rdev);
	(void) remove(dev->name);
	temp_unregister(dev->name);
    }
    free(dev->name);
}

#define MAX 15

void cache_scsi (char *name, int major)
{
    char tem[12], format[12];
    int i, j, dev;
    
    for (i=15; i>=0; i--) {
	dev = (major<<8) | (i<<4);
	strcpy(format, name);
	format[7] += i;
	cache_add(format, dev);
	strcat(format, "%d");
	for (j = MAX; j; j--) {
	    sprintf(tem, format, j);
	    cache_add(tem, dev | j);
	}
    }
}


void cache_ide (char *name, int major)
{
    char tem[12];
    char format[12];
    int i, dev, minor;
    
    minor = name[7]&1 ? 0 : 64;
    strcat(strcpy(format, name), "%d");
    dev = (major<<8) | minor;
    for (i = MAX; i; i--) {
        sprintf(tem, format, i);
        cache_add(tem, dev | i);
    }
    cache_add(name, dev);
}


void preload_dev_cache(void)
{
    char tmp[12];
    int i, vsave;

#define QUIET 1

#ifdef QUIET
    vsave = verbose;
    verbose -= QUIET;	/* quiet useless output */
#endif

    cache_add("/dev/fd0", 0x0200);
    cache_add("/dev/fd1", 0x0201);
    
#if 0
    cache_ide("/dev/hdt", MAJOR_IDE10);
    cache_ide("/dev/hds", MAJOR_IDE10);
    cache_ide("/dev/hdr", MAJOR_IDE9);
    cache_ide("/dev/hdq", MAJOR_IDE9);

    cache_ide("/dev/hdp", MAJOR_IDE8);
    cache_ide("/dev/hdo", MAJOR_IDE8);
    cache_ide("/dev/hdn", MAJOR_IDE7);
    cache_ide("/dev/hdm", MAJOR_IDE7);
    
    cache_ide("/dev/hdl", MAJOR_IDE6);
    cache_ide("/dev/hdk", MAJOR_IDE6);
    cache_ide("/dev/hdj", MAJOR_IDE5);
    cache_ide("/dev/hdi", MAJOR_IDE5);
#endif
    
    cache_scsi("/dev/sda", MAJOR_SD);
    
    cache_ide("/dev/hdh", MAJOR_IDE4);
    cache_ide("/dev/hdg", MAJOR_IDE4);
    cache_ide("/dev/hdf", MAJOR_IDE3);
    cache_ide("/dev/hde", MAJOR_IDE3);

    for (i = 0; i <= 7; i++) {
	sprintf(tmp,"/dev/loop%d",i);
	cache_add(tmp,0x700+i);
    }

    cache_ide("/dev/hdd", MAJOR_IDE2);
    cache_ide("/dev/hdc", MAJOR_IDE2);
    cache_ide("/dev/hdb", MAJOR_HD);
    cache_ide("/dev/hda", MAJOR_HD);
#ifdef QUIET
    verbose = vsave;
#endif
}
