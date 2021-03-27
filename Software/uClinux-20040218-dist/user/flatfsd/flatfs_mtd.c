/*****************************************************************************
 *	flatfs_mtd.c (c) 2003 SnapGear (www.snapgear.com)
 *
 *  Implements flatfs access to an MTD flash device
 *
 *****************************************************************************/

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/mtd/mtd.h>

#include "flatfs.h"
#include "flatfs_dev.h"

#define FLAG_WRITING    0x0001
#define FLAG_ERASED     0x0002

struct flatinfo_s {
	int fd;
	size_t len;          /* total length, in bytes */
	int flags;
};

static struct flatinfo_s flatinfo = { fd: -1 };

int
flat_dev_open(const char *flatfs, const char *mode)
{
	mtd_info_t      mtd_info;

	assert(flatinfo.fd == -1);

	/* Open and get the size of the FLASH file-system. */
	if ((flatinfo.fd = open(flatfs, O_RDWR)) < 0) {
		flatinfo.fd = -1;
		return(ERROR_CODE());
	}

	if (ioctl(flatinfo.fd, MEMGETINFO, &mtd_info) < 0) {
		close(flatinfo.fd);
		flatinfo.fd = -1;
		return(ERROR_CODE());
	}
	flatinfo.len = mtd_info.size;
	flatinfo.flags = 0;

	if (*mode == 'w') {
		flatinfo.flags |= FLAG_WRITING;
	}

	return(0);
}

int flat_dev_length(void)
{
	assert(flatinfo.fd != -1);

	return(flatinfo.len);
}

int flat_dev_erase(void)
{
	erase_info_t    erase_info;

	assert(flatinfo.fd != -1);
	assert(flatinfo.flags & FLAG_WRITING);

	flatinfo.flags |= FLAG_ERASED;

	erase_info.start = 0;
	erase_info.length = flatinfo.len;
	if (ioctl(flatinfo.fd, MEMERASE, &erase_info) < 0) {
		return(ERROR_CODE());
	}
	return(0);
}

int flat_dev_write(off_t offset, const char *buf, size_t len)
{
	assert(flatinfo.fd != -1);
	assert(flatinfo.flags & FLAG_WRITING);
	assert(flatinfo.flags & FLAG_ERASED);

	if (lseek(flatinfo.fd, offset, SEEK_SET) != offset) {
		return(ERROR_CODE());
	}
	if (write(flatinfo.fd, buf, len) != len) {
		return(ERROR_CODE());
	}

	return(len);
}

int flat_dev_read(char *buf, size_t len)
{
	assert(flatinfo.fd != -1);
	assert(!(flatinfo.flags & FLAG_WRITING));

	return(read(flatinfo.fd, buf, len));
}

off_t flat_dev_seek(off_t offset, int whence)
{
	assert(flatinfo.fd != -1);

	return(lseek(flatinfo.fd, offset, whence));
}

int flat_dev_close(int abort, off_t written)
{
	int rc = 0;
	(void)written;

	assert(flatinfo.fd != -1);

#if 0
	if (!abort && (flatinfo.flags & FLAG_ERASED)) {
		/*
		 * This has been removed for the time being because we have decided to
		 * not support interoperability (it can slow things down a LOT).
		 */
		/*
		 * zero out the rest of the device.
		 */
		char	buf[32];

		if (lseek(flatinfo.fd, written, SEEK_SET) != written) {
			rc = ERROR_CODE();
		}
		else {
			memset(buf, 0, sizeof(buf));
			while (write(flat_fd, buf, sizeof(buf)) == sizeof(buf)) {
			}
		}
	}
#endif	

	close(flatinfo.fd);
	flatinfo.fd = -1;

	return(rc);
}
