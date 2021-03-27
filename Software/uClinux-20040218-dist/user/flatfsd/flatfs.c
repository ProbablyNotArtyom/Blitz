/*****************************************************************************/
/*
 *	flatfs.c -- flat FLASH file-system.
 *
 *	Copyright (C) 1999, Greg Ungerer (gerg@snapgear.com).
 *	Copyright (C) 2001-2002, SnapGear (www.snapgear.com)
 */
/*****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include <sys/mount.h>

#include <syslog.h>

#include "flatfs.h"
#include "flatfs_dev.h"

/*
 * DON'T CHANGE THIS!!!
 * It is required because of the broken way we calculate checksums.
 */
#define BUF_SIZE 1024

/*****************************************************************************/

/*
 *	Checksum the contents of FLASH file.
 *	Pretty bogus check-sum really, but better than nothing :-)
 */

unsigned int chksum(const unsigned char *sp, unsigned int len)
{
	unsigned int chksum = 0;

	while (len--) {
		chksum += *sp++;
	}
	return(chksum);
}

static unsigned int   flat_sum = 0;

/*
 *	Open the flat device for reading ("r") or writing ("w").
 *  The size of the device is available after opening, but
 *  an explicit flat_erase() needs to be done before writing
 *  anything with flat_write().
 */
static inline int
flat_open(const char *flatfs, const char *mode)
{
	return(flat_dev_open(flatfs, mode));
}

/**
 * Erase the flat device which has successfully been opened for writing.
 *
 * Sets the checksum to 0.
 */
static inline int
flat_erase(void)
{
	int rc = flat_dev_erase();

	if (rc == 0) {
		flat_sum = 0;
	}

	return(rc);
}

/**
 * Closes the flat device.
 * If 'abort' is not set and the device has been erased/written to,
 * then the changes are committed.
 */
static inline int
flat_close(int abort, off_t written)
{
	return(flat_dev_close(abort, written));
}

/**
 * Performs an lseek() on the flat device.
 */
static inline off_t
flat_seek(off_t offset, int whence)
{
	return(flat_dev_seek(offset, whence));
}

/**
 * Returns the total length of the flat device.
 */
static inline size_t
flat_length(void)
{
	return(flat_dev_length());
}

/**
 * Write bytes to an erased flat device.
 *
 * Writes at the given offset.
 * Updates the checksum.
 */
static inline int
flat_write(off_t offset, const char *buf, size_t len)
{
	int rc = flat_dev_write(offset, buf, len);

	if (rc < 0) {
		return(rc);
	}
	flat_sum += chksum(buf, len);

	return(len);
}

/**
 * Just like read() against the flat device.
 */
static inline int
flat_read(char *buf, size_t len)
{
	return(flat_dev_read(buf, len));
}


/*
 *	Read the contents of a flat file-system and dump them out as
 *	regular files. Mmap would be nice, but alas...
 */

int flatread(char *flatfs)
{
	struct flathdr	hdr;
	int		version;
	struct flatent	ent;
	unsigned int n = 0;
	unsigned int len, size, sum;
	int		fdfile;
	char		filename[128];
	unsigned char	buf[BUF_SIZE];
	mode_t		mode;
	char *confbuf, *confline, *confdata;
	time_t t;
	int rc;

	if (chdir(DSTDIR) < 0)
		return(ERROR_CODE());

	if ((rc = flat_open(flatfs, "r")) < 0) {
		return(rc);
	}

	/* Check that header is valid */
	if (flat_read((void *) &hdr, sizeof(hdr)) != sizeof(hdr)) {
		flat_close(1, 0);
		return(ERROR_CODE());
	}

	if (hdr.magic == FLATFS_MAGIC) {
		version = 1;
	} else if (hdr.magic == FLATFS_MAGIC_V2) {
		version = 2;
	} else {
		syslog(LOG_ERR, "invalid header magic");
		flat_close(1, 0);
		return(ERROR_CODE());
	}

	len = flat_length();

	/* Check contents are valid */

	/* XXX - mn
	 * We calculate the checksum wrongly here.  Be aware of this when
	 * working out the checksum we need to store.
	 *
	 * Also be aware that if you include the last 1008 bytes in the checksum
	 * calculation, then this checksum calculation will fail.
	 *
	 */
	for (sum = 0, size = sizeof(hdr); (size < len); size += sizeof(buf)) {
		n = (size > sizeof(buf)) ? sizeof(buf) :  size;
		if (flat_read((void *) &buf[0], n) != n) {
			flat_close(1, 0);
			return(ERROR_CODE());
		}
		sum += chksum(&buf[0], n);
	}

#ifdef DEBUG
	syslog(LOG_DEBUG, "flatread() calculated checksum over %d bytes = %u", len, sum);
#endif

#if 0
	/* Check contents are valid */
	for (sum = 0, size = sizeof(hdr); (size < len); size += sizeof(buf)) {
		struct flatent *flt;
		int chrs;

		/* read the header */
		n = sizeof(struct flatent);
		if (flat_read((void *) &buf[0], n) != n) {
			flat_close(1, 0);
			return(ERROR_CODE());
		}

		/* get the aligned fields */
		/* Also add in the sizeof the mode bits */
		flt = (struct flatent *)buf;
		filelen = flt->filelen;
		namelen = flt->namelen;

		if ((filelen == UINT_MAX) && (namelen == UINT_MAX)) {
			goto sum_finished;
		}

		filelen = (filelen+3) & ~0x3;
		namelen = ((namelen+3) & ~0x3) + 4;

		sum += chksum(&buf[0], n);

		chrs = namelen+filelen;

		/* checksum the data */
		while (chrs) {
			n = (chrs > sizeof(buf)) ? sizeof(buf) : chrs;

			if (flat_read((void *) &buf[0], n) != n) {
				flat_close(1, 0);
				return(ERROR_CODE());
			}

			sum += chksum(&buf[0], n);

			chrs -= n;
		}
	}

sum_finished:

	sum += chksum(&buf[0], n);
#endif

	if (sum != hdr.chksum) {
		flat_close(1, 0);
		syslog(LOG_ERR, "bad header checksum");
		return(ERROR_CODE());
	}

	if (flat_seek(sizeof(hdr), SEEK_SET) != sizeof(hdr)) {
		flat_close(1, 0);
		return(ERROR_CODE());
	}

	for (numfiles = 0, numbytes = 0; ; numfiles++) {
		/* Get the name of next file. */
		if (flat_read((void *) &ent, sizeof(ent)) != sizeof(ent)) {
			flat_close(1, 0);
			return(ERROR_CODE());
		}

		if (ent.filelen == FLATFS_EOF) {
			break;
		}

		n = ((ent.namelen + 3) & ~0x3);
		if (n > sizeof(filename)) {
			flat_close(1, 0);
			return(ERROR_CODE());
		}

		if (flat_read((void *) &filename[0], n) != n) {
			flat_close(1, 0);
			return(ERROR_CODE());
		}

		if (version >= 2) {
			if (flat_read((void *) &mode, sizeof(mode)) != sizeof(mode)) {
				flat_close(1, 0);
				return(ERROR_CODE());
			}
		} else {
			mode = 0644;
		}

		if (strcmp(filename, FLATFSD_CONFIG) == 0) {
			/* Read our special flatfsd config file into memory */
			if (ent.filelen == 0) {
				/* This file was not written correctly, so just ignore it */
				syslog(LOG_WARNING, "%s is zero length, ignoring", filename);
			}
			else if ((confbuf = malloc(ent.filelen)) == 0) {
				syslog(LOG_ERR, "Failed to allocate memory for %s -- ignoring it", filename);
			}
			else {
				if (flat_read(confbuf, ent.filelen) != ent.filelen) {
					flat_close(1, 0);
					return(ERROR_CODE());
				}

				confline = strtok(confbuf, "\n");
				while (confline) {
					confdata = strchr(confline, ' ');
					if (confdata) {
						*confdata = '\0';
						confdata++;
						if (!strcmp(confline, "time")) {
							t = atol(confdata);
							if (t > time(NULL))
								stime(&t);
						}
					}
					confline = strtok(NULL, "\n");
				}
				free(confbuf);
			}
		} else {
			/* Write contents of file out for real. */
			fdfile = open(filename, (O_WRONLY | O_TRUNC | O_CREAT), mode);
			if (fdfile < 0) {
				flat_close(1, 0);
				return(ERROR_CODE());
			}
			
			for (size = ent.filelen; (size > 0); size -= n) {
				n = (size > sizeof(buf)) ? sizeof(buf) : size;
				if (flat_read(&buf[0], n) != n) {
					flat_close(1, 0);
					return(ERROR_CODE());
				}
				if (write(fdfile, (void *) &buf[0], n) != n) {
					flat_close(1, 0);
					return(ERROR_CODE());
				}
			}

			close(fdfile);
		}

		/* Read alignment padding */
		n = ((ent.filelen + 3) & ~0x3) - ent.filelen;
		if (flat_read(&buf[0], n) != n) {
			flat_close(1, 0);
			return(ERROR_CODE());
		}

		numbytes += ent.filelen;
	}

	flat_close(0, 0);

	return(0);
}

/*****************************************************************************/

int writefile(char *name, unsigned int *ptotal, int dowrite)
{
	struct flatent ent;
	struct stat    st;
	unsigned int   size;
	int            fdfile, zero = 0;
	mode_t		   mode;
	char           buf[BUF_SIZE];
	int written;
	int n;

	/*
	 *	Write file entry into flat fs. Names and file
	 *	contents are aligned on long word boundaries.
	 *	They are padded to that length with zeros.
	 */
	if (stat(name, &st) < 0)
		return(ERROR_CODE());

	size = strlen(name) + 1;
	if (size > 128) {
		numdropped++;
		return(ERROR_CODE());
	}

	ent.namelen = size;
	ent.filelen = st.st_size;
	if (dowrite && flat_write(*ptotal, (char *) &ent, sizeof(ent)) < 0)
		return(ERROR_CODE());
	*ptotal += sizeof(ent);

	/* Write file name out, with padding to align */
	if (dowrite && flat_write(*ptotal, name, size) < 0)
		return(ERROR_CODE());
	*ptotal += size;
	size = ((size + 3) & ~0x3) - size;
	if (dowrite && flat_write(*ptotal, (char *)&zero, size) < 0)
		return(ERROR_CODE());
	*ptotal += size;

	/* Write out the permissions */
	mode = (mode_t) st.st_mode;
	size = sizeof(mode);
	if (dowrite && flat_write(*ptotal, (char *) &mode, size) < 0)
		return(ERROR_CODE());
	*ptotal += size;

	/* Write the contents of the file. */
	size = st.st_size;


	written = 0;

	if (size > 0) {
		if (dowrite) {
			if ((fdfile = open(name, O_RDONLY)) < 0) {
				return(ERROR_CODE());
			}
			while (size>written) {
				int bytes_read;
				n = ((size-written) > sizeof(buf))?sizeof(buf):(size-written);
				if ((bytes_read = read(fdfile, buf, n)) != n) {
					/* Somebody must have trunced the file - Log it. */
					syslog(LOG_WARNING, "File %s was shorter than expected.",
						name);
					if (bytes_read <= 0) {
						break;
					}
				}
				if (dowrite && flat_write(*ptotal, buf, bytes_read) < 0) {
					close(fdfile);
					return (ERROR_CODE());
				}
				*ptotal += bytes_read;
				written += bytes_read;
			}
			if (lseek(fdfile, 0, SEEK_END) != written) {
				/* 
				 * Log the file being longer than expected.
				 * We can't write more than expected because the size is already
				 * written.
				 */
				syslog(LOG_WARNING, "File %s was longer than expected.", name);
			}
			close(fdfile);
		}
		else {
			*ptotal += st.st_size;
		}

		/* Pad to align */
		written = ((st.st_size + 3) & ~0x3)- st.st_size;
		if (dowrite && flat_write(*ptotal, (char *)&zero, written) < 0)
			return(ERROR_CODE());
		*ptotal += written;
	}

	numfiles++;
	numbytes += ent.filelen;

	return 0;
}

/**
 * Writes out the contents of all files.
 * Does not actually do the write if 'dowrite'
 * is not set. In this case, it just checks
 * to see that the config will fit.
 * The total length of data written (or simulated) is stored
 * in *total.
 * Does not remove .flatfsd
 *
 * Note that if the flash has been erased, aborting
 * early will just lose data. So we try to work around
 * problems as much as possible.
 *
 * Returns 0 if OK, or < 0 if error.
 */
static int write_config(int dowrite, unsigned *total)
{
	DIR             *dirp;
	struct dirent	*dp;
	int ret = 0;
	int rc;
	struct flathdr	hdr;
	struct flatent	ent;

#ifdef DEBUG
	syslog(LOG_DEBUG, "write_config(dowrite=%d)", dowrite);
#endif

	/* Write out contents of all files, skip over header */
	numfiles = 0;
	numbytes = 0;
	numdropped = 0;
	*total = sizeof(hdr);

	rc = writefile(FLATFSD_CONFIG, total, dowrite);
	if (rc < 0 && !ret) {
		ret = rc;
	}

	/* Scan directory */
	if ((dirp = opendir(".")) == NULL) {
		rc = ERROR_CODE();
		if (rc < 0 && !ret) {
			ret = rc;
		}
		/* Really nothing we can do at this point */
		return(ret);
	}

	while ((dp = readdir(dirp)) != NULL) {

		if ((strcmp(dp->d_name, ".") == 0) ||
		    (strcmp(dp->d_name, "..") == 0) ||
		    (strcmp(dp->d_name, FLATFSD_CONFIG) == 0))
			continue;

		rc = writefile(dp->d_name, total, dowrite);
		if (rc < 0) {
			syslog(LOG_ERR, "Failed to write write file %s (%d): %m %d",
				dp->d_name, rc, errno);
			if (!ret) {
				ret = rc;
			}
		}
	}
	closedir(dirp);

	/* Write the terminating entry */
	if (dowrite) {
		ent.namelen = FLATFS_EOF;
		ent.filelen = FLATFS_EOF;
		rc = flat_write(*total, (char *) &ent, sizeof(ent));
		if (rc < 0 && !ret) {
			ret = rc;
		}
	}

	*total += sizeof(ent);

#ifdef USING_MTD_DEVICE
	/* We need to account for the fact that we checksum the entire device, not
	 * just the data we wrote. On MTD devices, this data is 0xFF
	 */
	{
		int checksum_len = flat_length() - (BUF_SIZE - (sizeof(struct flathdr) * 2));

		flat_sum += 0xFFu * (checksum_len - *total);

#ifdef DEBUG
		syslog(LOG_DEBUG, "flatwrite(): added %d 0xFF bytes to checksum -> flat_sum=%u", checksum_len - *total, flat_sum);
#endif
	}
#endif

	if (dowrite) {
		/* Construct header */
		hdr.magic = FLATFS_MAGIC_V2;
		hdr.chksum = flat_sum;

#ifdef DEBUG
		syslog(LOG_DEBUG, "flatwrite(): final checksum=%u, total=%d", flat_sum, *total);
#endif

		rc = flat_write(0L, (char *)&hdr, sizeof(hdr));
		if (rc < 0 && !ret) {
			ret = rc;
		}
	}

#ifdef DEBUG
	syslog(LOG_DEBUG, "write_config() returning ret=%d, total=%u", ret, *total);
#endif

	return(ret);
}

/*****************************************************************************/

/*
 *	Write out the contents of the local directory to flat file-system.
 *	The writing process is not quite as easy as read. Use the usual
 *	write system call so that FLASH programming is done properly.
 */

int flatwrite(char *flatfs)
{
	FILE            *hfile;
	unsigned int	total;
	int             rc = 0;
	time_t			start_time, flt_write_time;
	int log_level;

	start_time = time(NULL);

	if (chdir(SRCDIR) < 0) {
		return(ERROR_CODE());
	}

	/* Create a special config file */
	/* REVISIT: If we can't create this file, should we write everything else anyway
	 *          or should we give up with an error?
	 */
	hfile = fopen(FLATFSD_CONFIG, "w");
	if (!hfile) {
		return(ERROR_CODE());
	}
	fprintf(hfile, "time %ld\n", time(NULL));
	fflush(hfile);
	if (ferror(hfile)) {
		rc = ERROR_CODE();
	}
	if (fclose(hfile) != 0) {
		rc = ERROR_CODE();
	}
	if (rc < 0) {
		unlink(FLATFSD_CONFIG);
		return(rc);
	}

	rc = flat_open(flatfs, "w");
	if (rc < 0) {
		unlink(FLATFSD_CONFIG);
		return(rc);
	}
	flat_sum = 0;

	/* Check to see if the config will fit before we erase */
	rc = write_config(0, &total);
	if (rc < 0 || total > flat_length()) {
		syslog(LOG_ERR, "config will not fit in flash");
		unlink(FLATFSD_CONFIG);
		goto cleanup;
	}

	rc = flat_erase();
	if (rc < 0) {
		unlink(FLATFSD_CONFIG);
		goto cleanup;
	}

	rc = write_config(1, &total);
	unlink(FLATFSD_CONFIG);
	if (rc < 0) {
		goto cleanup;
	}

	rc = flat_close(0, total);

	flt_write_time = time(NULL) - start_time;

	log_level = LOG_ALERT;
	if (flt_write_time<=20) {
		log_level = LOG_DEBUG;
	} else if ((flt_write_time>20) && (flt_write_time<=40)) {
		log_level = LOG_NOTICE;
	} else if ((flt_write_time>40) && (flt_write_time<=100)){
		log_level = LOG_ERR;
	} else {
		log_level = LOG_ALERT;
	}
	syslog(log_level, "Wrote %d bytes to flash in %ld seconds", total, flt_write_time);

	return rc;

cleanup:
	flat_close(1, 0);
	return(rc);
}

/*****************************************************************************/
