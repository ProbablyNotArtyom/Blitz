/*
 * FILE unlock.c
 *
 * This utility unlock all sectors of flash device.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <string.h>

#include <linux/mtd/mtd.h>

int main(int argc, char *argv[])
{
  int fd;
  struct mtd_info_user mtdInfo;
  struct erase_info_user mtdLockInfo;

  /*
   * Parse command line options
   */
  if(argc != 2)
  {
    fprintf(stderr, "USAGE: %s <mtd device>\n", argv[0]);
    exit(1);
  }
  else if(strncmp(argv[1], "/dev/mtd", 8) != 0)
  {
    fprintf(stderr, "'%s' is not a MTD device.  Must specify mtd device: /dev/mtd?\n", argv[1]);
    exit(1);
  }

  fd = open(argv[1], O_RDWR);
  if(fd < 0)
  {
    fprintf(stderr, "Could not open mtd device: %s\n", argv[1]);
    exit(1);
  }

  if(ioctl(fd, MEMGETINFO, &mtdInfo))
  {
    fprintf(stderr, "Could not get MTD device info from %s\n", argv[1]);
    close(fd);
    exit(1);
  }

  mtdLockInfo.start = 0;
  mtdLockInfo.length = mtdInfo.size;
  if(ioctl(fd, MEMUNLOCK, &mtdLockInfo))
  {
    fprintf(stderr, "Could not unlock MTD device: %s\n", argv[1]);
    close(fd);
    exit(1);
  }

  return 0;
}

