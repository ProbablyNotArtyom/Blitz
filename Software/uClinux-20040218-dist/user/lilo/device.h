/* device.h  -  Device access */

/* Copyright 1992-1996 Werner Almesberger. See file COPYING for details. */


#ifndef DEVICE_H
#define DEVICE_H

#include <sys/stat.h>


typedef struct {
    int fd;
    struct stat st;
    char *name;
    int delete;
} DEVICE;


int dev_open(DEVICE *dev,int number,int flags);

/* Searches /dev for a block device with the specified number. If no device
   can be found, a temporary device is created. The device is opened with
   the specified access mode and the file descriptor is returned. If flags
   are -1, the device is not opened. */

void dev_close(DEVICE *dev);

/* Closes a device that has previously been opened by dev_open. If the device
   had to be created, it is removed now. */

void preload_dev_cache(void);

/* Preloads the device number to name cache. */

#endif
