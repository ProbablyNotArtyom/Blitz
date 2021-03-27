/*****************************************************************************
 *	flatfs_disk.c (c) 2003 SnapGear (www.snapgear.com)
 *
 *  Implements flatfs access to a disk-like device
 *
 *****************************************************************************/

#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <assert.h>

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
	int open_mode;
	assert(flatinfo.fd == -1);

	flatinfo.flags = 0;
	if (*mode == 'w') {
		flatinfo.flags |= FLAG_WRITING;
		open_mode = O_WRONLY;
	}
	else {
		open_mode = O_RDONLY;
	}

	/* Open and get the size of the disk file-system. */
	if ((flatinfo.fd = open(flatfs, open_mode)) < 0) {
		flatinfo.fd = -1;
		return(ERROR_CODE());
	}



	/* Now, let's see how bit this file is. If it is zero size and we are writing
	 * we try "erasing" it to find the length. This is no loss since it obviously
	 * contains nothing
	 */
	flatinfo.len = lseek(flatinfo.fd, 0, SEEK_END);
	if (flatinfo.len < 0) {
		close(flatinfo.fd);
		flatinfo.fd = -1;
		return(ERROR_CODE());
	}

	/* Reposition to the start */
	lseek(flatinfo.fd, 0L, SEEK_SET);

	if ((flatinfo.flags & FLAG_WRITING) && flatinfo.len == 0) {
		/* OK. The file has zero size anyway, so erase it and see how big it gets.
		 * flat_dev_erase() sets flatinfo.len
		 */
		return(flat_dev_erase());
	}

	return(0);
}

int flat_dev_length(void)
{
	assert(flatinfo.fd != -1);

	return(flatinfo.len);
}

int
flat_dev_erase(void)
{
	assert(flatinfo.fd != -1);
	assert(flatinfo.flags & FLAG_WRITING);

	if (flatinfo.flags & FLAG_ERASED) {
		return(0);
	}
	else {
		char	buf[32];
		int size = 0;

		flatinfo.flags |= FLAG_ERASED;

		memset(buf, 0, sizeof(buf));

		while (write(flatinfo.fd, buf, sizeof(buf)) == sizeof(buf)) {
			size += sizeof(buf);
		}

		flatinfo.len = lseek(flatinfo.fd, 0L, SEEK_CUR);

		/* Reposition to the start */
		lseek(flatinfo.fd, 0L, SEEK_SET);

		return(0);
	}
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
	(void)abort;
	(void)written;

	assert(flatinfo.fd != -1);

	close(flatinfo.fd);
	flatinfo.fd = -1;

	return(rc);
}
