/*
 * Build a JFFS2 image in a file, from a given directory tree.
 *
 * Copyright 2001 XXX
 *           2001 David A. Schleef <ds@lineo.com>
 *           2001 Erik Andersen <andersen@codepoet.org>
 *
 * Cross-endian support added by David Schleef <ds@schleef.org>.
 *
 * Major architectural rewrite by Erik Andersen <andersen@codepoet.org>
 * to allow support for making hard links (though hard links support is
 * not yet implemented), and for munging file permissions and ownership 
 * on the fly using --faketime, --squash, --devtable.   And I plugged a
 * few memory leaks, adjusted the error handling and fixed some little 
 * nits here and there.
 *
 * I also added a sample device table file.  See device_table.txt
 *  -Erik, Septermber 2001
 */

/* $Id: mkfs.jffs2.c,v 1.20 2002/02/08 00:51:52 dwmw2 Exp $ */

//#define DMALLOC
//#define mkfs_debug_msg	error_msg
#define mkfs_debug_msg(a...)	{ }


#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <libgen.h>
#include <endian.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "jffs2.h"
#include "crc32.h"
#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define min(x,y) ({ typeof((x)) _x = (x); typeof((y)) _y = (y); (_x>_y)?_y:_x; })


/* FIXME:  MKDEV uses illicit insider knowledge of kernel 
 * major/minor representation...  */
#define MINORBITS	8
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))


struct directory_entry {
	char *name;
	char *path;
	char *base;
	struct stat sb;
	uint32_t jffs2_ino;
	struct directory_entry *parent;
	struct directory_entry *next;
};
struct directory_entry *dir_list = NULL;

struct filesystem_entry {
	char *name;
	char *path;
	char type;
	struct stat sb;
	uint32_t jffs2_ino;
	struct directory_entry *parent;
	struct filesystem_entry *next;
	struct filesystem_entry *prev;
};
struct filesystem_entry *file_list = NULL;


#define JFFS2_MAX_FILE_SIZE 0xFFFFFFFF
#ifndef JFFS2_MAX_SYMLINK_LEN
#define JFFS2_MAX_SYMLINK_LEN 254
#endif

static int erase_block_size = 65536;
static int pad_fs_size = 0;
static int page_size = 4096;
static int out_fd = 1;
static int out_ofs = 0;
static uint32_t highest_ino = 1;
static uint32_t version;
static char default_rootdir[]=".";
static char *rootdir = default_rootdir;

static unsigned char ffbuf[16] =
	{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff };

/* fake_times will set all the timestamps to 0, making regression
 * testing easier. */
static int fake_times = 0;

/* Squash all permissions and make everything be owned by root */
static int squash = 0;

static int host_endian = __BYTE_ORDER;
static int target_endian = __BYTE_ORDER;

/* some byte swabbing stuff from include/linux/byteorder/ */
#define swab16(x) \
	((uint16_t)( \
		(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
		(((uint16_t)(x) & (uint16_t)0xff00U) >> 8) ))
#define swab32(x) \
	((uint32_t)( \
		(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
		(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
		(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
		(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24) ))

#define cpu_to_target16(x) \
	((host_endian==target_endian)?(x):(swab16(x)))
#define cpu_to_target32(x) \
	((host_endian==target_endian)?(x):(swab32(x)))



extern int zlib_compress(unsigned char *data_in, unsigned char *cpage_out, uint32_t *sourcelen, uint32_t *dstlen);
extern int jffs2_rtime_compress(unsigned char *data_in, unsigned char *cpage_out, uint32_t *sourcelen, uint32_t *dstlen);

unsigned char jffs2_compress(unsigned char *data_in, unsigned char *cpage_out, 
		    uint32_t *datalen, uint32_t *cdatalen)
{
	int ret;

	ret = zlib_compress(data_in, cpage_out, datalen, cdatalen);
	if (!ret) {
		return JFFS2_COMPR_ZLIB;
	}
	/* rtime does manage to recompress already-compressed data */
	ret = jffs2_rtime_compress(data_in, cpage_out, datalen, cdatalen);
	if (!ret) {
		return JFFS2_COMPR_RTIME;
	}
	return JFFS2_COMPR_NONE; /* We failed to compress */
}


/* These are all stolen from busybox's libbb to make
 * error handling simpler (and since I maintain busybox, 
 * I'm rather partial to these for error handling). 
 *  -Erik
 */
static const char *const app_name = "mkfs.jffs2";
static const char *const memory_exhausted = "memory exhausted";

static void verror_msg(const char *s, va_list p)
{
	fflush(stdout);
	fprintf(stderr, "%s: ", app_name);
	vfprintf(stderr, s, p);
}
static void error_msg(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
}

static void error_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	verror_msg(s, p);
	va_end(p);
	putc('\n', stderr);
	exit(EXIT_FAILURE);
}

static void vperror_msg(const char *s, va_list p)
{
	int err = errno;

	if (s == 0)
		s = "";
	verror_msg(s, p);
	if (*s)
		s = ": ";
	fprintf(stderr, "%s%s\n", s, strerror(err));
}

#if 0
static void perror_msg(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	vperror_msg(s, p);
	va_end(p);
}
#endif

static void perror_msg_and_die(const char *s, ...)
{
	va_list p;

	va_start(p, s);
	vperror_msg(s, p);
	va_end(p);
	exit(EXIT_FAILURE);
}

#ifndef DMALLOC
static void *xmalloc(size_t size)
{
	void *ptr = malloc(size);

	if (!ptr)
		error_msg_and_die(memory_exhausted);
	return ptr;
}

static char *xstrdup(const char *s)
{
	char *t;

	if (s == NULL)
		return NULL;

	t = strdup(s);

	if (t == NULL)
		error_msg_and_die(memory_exhausted);

	return t;
}
#endif

static FILE *xfopen(const char *path, const char *mode)
{
	FILE *fp;

	if ((fp = fopen(path, mode)) == NULL)
		perror_msg_and_die("%s", path);
	return fp;
}

static void full_write(int fd, const void *buf, int len)
{
	int ret;

	while (len > 0) {
		ret = write(fd, buf, len);

		if (ret < 0)
			perror_msg_and_die("write");

		if (ret == 0)
			perror_msg_and_die("write returned zero");

		len -= ret;
		buf += ret;
		out_ofs += ret;
	}
}

static void new_file_list_entry(struct filesystem_entry *new_entry)
{
	/* Add the new file to the file_list */
	if (!file_list) {
		file_list = new_entry;
		file_list->prev = NULL;
		file_list->next = NULL;
	} else {
		struct filesystem_entry *tmp_file, *prev;

		tmp_file = prev = file_list;
		while (tmp_file && tmp_file->next) {
			prev = tmp_file;
			tmp_file = tmp_file->next;
		}
		tmp_file->prev = prev;
		tmp_file->next = new_entry;
	}

	/* If tmp_file->parent is NULL (which happens with 
	 * device table entries), try and find our parent now) */
	if (new_entry->parent == NULL) {
		char *base, *tmp;
		struct directory_entry *tmp_dir;

		tmp = xstrdup(new_entry->name);
		base = xstrdup(dirname(tmp));
		free(tmp);

		tmp_dir = dir_list;
		while (tmp_dir) {
			if (strcmp(base, tmp_dir->name) == 0) {
				new_entry->parent = tmp_dir;
				break;
			}
			tmp_dir = tmp_dir->next;
		}
		free(base);
		if (!new_entry->parent)
			error_msg_and_die("%s: has no parent directory!", new_entry->name);
	}
}

static void add_new_file(char *name, char *path, unsigned long uid,
				  unsigned long gid, unsigned long mode,
				  struct directory_entry *parent)
{
	int status;
	struct stat sb;
	time_t timestamp = time(NULL);
	struct filesystem_entry *new_entry;

	memset(&sb, 0, sizeof(struct stat));
	status = lstat(path, &sb);

	if (status >= 0) {
		/* It is ok for some types of files to not exit on disk (such as
		 * device nodes), but if they _do_ exist the specified mode had
		 * better match the actual file or strange things will happen.... */
		if ((mode & S_IFMT) != (sb.st_mode & S_IFMT))
			error_msg_and_die("%s: file type does not match specified type!", path);
		timestamp = sb.st_mtime;
	} else {
		/* If this is a regular file, it _must_ exist on disk */
		if ((mode & S_IFMT) == S_IFREG) {
			error_msg_and_die("%s: file is must exist on disk!", path);
		}
	}

	if (squash) {
		/* Squash all permissions so files are owned by
		 * root, all timestamps are _right now_, and file
		 * permissions have group and other write removed */
		uid = gid = 0;
		if (!S_ISLNK(mode)) {
			mode &= ~(S_IWGRP | S_IWOTH);
			mode &= ~(S_ISUID | S_ISGID);
		}
	}
	if (fake_times) {
		timestamp = 0;
	}

	new_entry = xmalloc(sizeof(struct filesystem_entry));
	memset(new_entry, 0, sizeof(struct filesystem_entry));
	new_entry->name = xstrdup(name);
	new_entry->path = xstrdup(path);
	if (sb.st_ino != 0)
		memcpy(&(new_entry->sb), &sb, sizeof(struct stat));
	new_entry->jffs2_ino = 1;
	new_entry->sb.st_uid = uid;
	new_entry->sb.st_gid = gid;
	new_entry->sb.st_mode = mode;
	new_entry->parent = parent;
	new_entry->sb.st_atime = new_entry->sb.st_ctime =
		new_entry->sb.st_mtime = timestamp;

	new_file_list_entry(new_entry);
}

static void add_new_device(char *name, char *path, unsigned long uid, 
	unsigned long gid, unsigned long mode, dev_t rdev, 
	struct directory_entry *parent)
{
	int status;
	struct stat sb;
	time_t timestamp = time(NULL);
	struct filesystem_entry *new_entry;

	memset(&sb, 0, sizeof(struct stat));
	status = lstat(path, &sb);

	if (status >= 0) {
		/* It is ok for some types of files to not exit on disk (such as
		 * device nodes), but if they _do_ exist the specified mode had
		 * better match the actual file or strange things will happen.... */
#if 0
		if ((mode & S_IFMT) != (sb.st_mode & S_IFMT))
			error_msg_and_die("%s: file type does not match specified type!", path);
#endif
		timestamp = sb.st_mtime;
	}

	if (squash) {
		/* Squash all permissions so files are owned by
		 * root, all timestamps are _right now_, and file
		 * permissions have group and other write removed */
		uid = gid = 0;
		mode &= ~(S_IWGRP | S_IWOTH);
		mode &= ~(S_ISUID | S_ISGID);
	}
	if (fake_times) {
		timestamp = 0;
	}

	new_entry = xmalloc(sizeof(struct filesystem_entry));
	memset(new_entry, 0, sizeof(struct filesystem_entry));
	new_entry->name = xstrdup(name);
	new_entry->path = xstrdup(path);
	new_entry->jffs2_ino = 1;
	new_entry->sb.st_uid = uid;
	new_entry->sb.st_gid = gid;
	new_entry->sb.st_mode = mode;
	new_entry->parent = parent;
	new_entry->sb.st_rdev = rdev;
	new_entry->sb.st_atime = new_entry->sb.st_ctime =
		new_entry->sb.st_mtime = timestamp;

	new_file_list_entry(new_entry);
}

static struct directory_entry *add_new_directory(char *name, char *path, 
		unsigned long uid, unsigned long gid, unsigned long mode)
{
	int status;
	char *tmp;
	struct stat sb;
	time_t timestamp = time(NULL);
	struct directory_entry *new_entry;

	memset(&sb, 0, sizeof(struct stat));
	status = lstat(path, &sb);

	if (status >= 0) {
		/* If a file is in fact on the disk, it had better be a 
		 * directory or strange things will happen.... */
		if ((mode & S_IFMT) != (sb.st_mode & S_IFMT))
			error_msg_and_die("%s: is not a directory!", path);
		timestamp = sb.st_mtime;
	}

	if (squash) {
		/* Squash all permissions so files are owned by
		 * root, all timestamps are _right now_, and file
		 * permissions have group and other write removed */
		uid = gid = 0;
		mode &= ~(S_ISUID | S_ISGID);
	}
	if (fake_times) {
		timestamp = 0;
	}

	new_entry = malloc(sizeof(struct directory_entry));
	memset(new_entry, 0, sizeof(struct directory_entry));
	new_entry->name = xstrdup(name);
	new_entry->path = xstrdup(path);
	tmp = xstrdup(name);
	new_entry->base = xstrdup(dirname(tmp));
	free(tmp);
	if (sb.st_ino != 0)
		memcpy(&(new_entry->sb), &sb, sizeof(struct stat));
	new_entry->jffs2_ino = 1;
	new_entry->sb.st_uid = uid;
	new_entry->sb.st_gid = gid;
	new_entry->sb.st_mode = mode;
	new_entry->sb.st_atime = new_entry->sb.st_ctime =
		new_entry->sb.st_mtime = timestamp;

	/* Add the new directory to the dir_list */
	if (!dir_list) {
		new_entry->parent = NULL;
		dir_list = new_entry;
	} else {
		struct directory_entry *tmp_dir, *prev;

		tmp_dir = prev = dir_list;
		while (tmp_dir) {
			/* Connect directories with their parents */
			if (strcmp(new_entry->base, tmp_dir->name) == 0) {
				new_entry->parent = tmp_dir;
			}
			prev = tmp_dir;
			tmp_dir = tmp_dir->next;
		}
		prev->next = new_entry;
		if (!new_entry->parent) {
			error_msg_and_die("%s: has no parent directory!",
							  new_entry->name);
		}
	}
	return (new_entry);
}

static void padblock(void)
{
	while (out_ofs % erase_block_size) {
		full_write(out_fd, ffbuf, 
			min(16, erase_block_size - (out_ofs % erase_block_size)));
	}
}

static inline void pad_block_if_less_than(int req)
{
	if ((out_ofs % erase_block_size) + req > erase_block_size) {
		padblock();
	}
}

static inline void padword(void)
{
	if (out_ofs % 4) {
		full_write(out_fd, ffbuf, 4 - (out_ofs % 4));
	}
}

static void write_dirent(uint32_t pino, uint32_t ver, uint32_t ino, uint32_t mctime,
	uint32_t type, unsigned char *name)
{
	struct jffs2_raw_dirent rd;

	memset(&rd, 0, sizeof(rd));

	rd.magic = cpu_to_target16(JFFS2_MAGIC_BITMASK);
	rd.nodetype = cpu_to_target16(JFFS2_NODETYPE_DIRENT);
	rd.totlen = cpu_to_target32(sizeof(rd) + strlen(name));
	rd.hdr_crc = cpu_to_target32(crc32(0, &rd, 
		    sizeof(struct jffs2_unknown_node) - 4));
	rd.pino = cpu_to_target32(pino);
	rd.version = cpu_to_target32(ver);
	rd.ino = cpu_to_target32(ino);
	rd.mctime = cpu_to_target32(mctime);
	rd.nsize = strlen(name);
	rd.type = type;
	//rd.unused[0] = 0;
	//rd.unused[1] = 0;
	rd.node_crc = cpu_to_target32(crc32(0, &rd, sizeof(rd) - 8));
	rd.name_crc = cpu_to_target32(crc32(0, name, strlen(name)));

	pad_block_if_less_than(sizeof(rd) + rd.nsize);

	full_write(out_fd, &rd, sizeof(rd));
	full_write(out_fd, name, rd.nsize);
	padword();
}

static void output_reg(int fd, uint32_t ino, struct stat *statbuf)
{
	unsigned char *buf, *cbuf, *wbuf;
	struct jffs2_raw_inode ri;
	unsigned int version, offset;
	int len;

	buf = xmalloc(page_size);
	cbuf = xmalloc(page_size);

	version = 0;
	offset = 0;

	memset(&ri, 0, sizeof(ri));
	ri.magic = cpu_to_target16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_target16(JFFS2_NODETYPE_INODE);

	ri.ino = cpu_to_target32(ino);
	ri.mode = cpu_to_target32(statbuf->st_mode);
	ri.uid = cpu_to_target32(statbuf->st_uid);
	ri.gid = cpu_to_target32(statbuf->st_gid);
	ri.atime = cpu_to_target32(statbuf->st_atime);
	ri.ctime = cpu_to_target32(statbuf->st_ctime);
	ri.mtime = cpu_to_target32(statbuf->st_mtime);
	ri.isize = cpu_to_target32(statbuf->st_size);

	while ((len = read(fd, buf, page_size))) {
		unsigned char *tbuf = buf;

		if (len < 0) {
			perror_msg_and_die("read");
		}

		while (len) {
			uint32_t dsize, space;

			pad_block_if_less_than(sizeof(ri) + JFFS2_MIN_DATA_LEN);

			dsize = len;
			space = erase_block_size - (out_ofs % erase_block_size) - sizeof(ri);
			if (space > dsize)
				space = dsize;

			ri.compr = jffs2_compress(tbuf, cbuf, &dsize, &space);
			if (ri.compr) {
				wbuf = cbuf;
			} else {
				wbuf = tbuf;
				dsize = space;
			}

			ri.totlen = cpu_to_target32(sizeof(ri) + space);
			ri.hdr_crc = cpu_to_target32(crc32(0, &ri, 
				    sizeof(struct jffs2_unknown_node) - 4));

			version++;
			ri.version = cpu_to_target32(version);
			ri.offset = cpu_to_target32(offset);
			ri.csize = cpu_to_target32(space);
			ri.dsize = cpu_to_target32(dsize);
			ri.node_crc = cpu_to_target32(crc32(0, &ri, sizeof(ri) - 8));
			ri.data_crc = cpu_to_target32(crc32(0, wbuf, space));

			full_write(out_fd, &ri, sizeof(ri));
			full_write(out_fd, wbuf, space);
			padword();

			tbuf += dsize;
			len -= dsize;
			offset += dsize;
		}
	}
	if (!ri.version) {
		/* Was empty file */
		version++;
		ri.version = cpu_to_target32(version);
		ri.totlen = cpu_to_target32(sizeof(ri));
		ri.hdr_crc = cpu_to_target32(crc32(0, &ri, 
			    sizeof(struct jffs2_unknown_node) - 4));
		ri.csize = cpu_to_target32(0);
		ri.dsize = cpu_to_target32(0);
		ri.node_crc = cpu_to_target32(crc32(0, &ri, sizeof(ri) - 8));

		full_write(out_fd, &ri, sizeof(ri));
		padword();
	}
	free(buf);
	free(cbuf);
	close(fd);
}

static void output_dev(uint32_t ino, struct stat *statbuf)
{
	struct jffs2_raw_inode ri;

	/* FIXME:  I am using illicit insider knowledge of kernel 
	 * major/minor representation...  */
	unsigned short kdev;

	kdev = cpu_to_target16((major(statbuf->st_rdev) << 8) +
		minor(statbuf->st_rdev));

	memset(&ri, 0, sizeof(ri));

	ri.magic = cpu_to_target16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_target16(JFFS2_NODETYPE_INODE);
	ri.totlen = cpu_to_target32(sizeof(ri) + sizeof(kdev));
	ri.hdr_crc = cpu_to_target32(crc32(0, &ri, 
		    sizeof(struct jffs2_unknown_node) - 4));

	ri.ino = cpu_to_target32(ino);
	ri.mode = cpu_to_target32(statbuf->st_mode);
	ri.uid = cpu_to_target32(statbuf->st_uid);
	ri.gid = cpu_to_target32(statbuf->st_gid);
	ri.atime = cpu_to_target32(statbuf->st_atime);
	ri.ctime = cpu_to_target32(statbuf->st_ctime);
	ri.mtime = cpu_to_target32(statbuf->st_mtime);
	ri.isize = cpu_to_target32(statbuf->st_size);
	ri.version = cpu_to_target32(1);
	ri.csize = cpu_to_target32(sizeof(kdev));
	ri.dsize = cpu_to_target32(sizeof(kdev));
	ri.node_crc = cpu_to_target32(crc32(0, &ri, sizeof(ri) - 8));
	ri.data_crc = cpu_to_target32(crc32(0, &kdev, sizeof(kdev)));

	pad_block_if_less_than(sizeof(ri) + sizeof(kdev));

	full_write(out_fd, &ri, sizeof(ri));
	full_write(out_fd, &kdev, sizeof(kdev));
	padword();
}


static void output_pipe(uint32_t ino, struct stat *statbuf)
{
	struct jffs2_raw_inode ri;

	memset(&ri, 0, sizeof(ri));

	ri.magic = cpu_to_target16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_target16(JFFS2_NODETYPE_INODE);
	ri.totlen = cpu_to_target32(sizeof(ri));
	ri.hdr_crc = cpu_to_target32(crc32(0, &ri, 
		    sizeof(struct jffs2_unknown_node) - 4));

	ri.ino = cpu_to_target32(ino);
	ri.mode = cpu_to_target32(statbuf->st_mode);
	ri.uid = cpu_to_target32(statbuf->st_uid);
	ri.gid = cpu_to_target32(statbuf->st_gid);
	ri.atime = cpu_to_target32(statbuf->st_atime);
	ri.ctime = cpu_to_target32(statbuf->st_ctime);
	ri.mtime = cpu_to_target32(statbuf->st_mtime);
	ri.isize = cpu_to_target32(0);
	ri.version = cpu_to_target32(1);
	ri.csize = cpu_to_target32(0);
	ri.dsize = cpu_to_target32(0);
	ri.node_crc = cpu_to_target32(crc32(0, &ri, sizeof(ri) - 8));
	ri.data_crc = cpu_to_target32(0);

	pad_block_if_less_than(sizeof(ri));

	full_write(out_fd, &ri, sizeof(ri));
	padword();
}


static void output_symlink(const char *target, int len, uint32_t ino, 
	struct stat *statbuf)
{
	struct jffs2_raw_inode ri;

	memset(&ri, 0, sizeof(ri));

	ri.magic = cpu_to_target16(JFFS2_MAGIC_BITMASK);
	ri.nodetype = cpu_to_target16(JFFS2_NODETYPE_INODE);
	ri.totlen = cpu_to_target32(sizeof(ri) + len);
	ri.hdr_crc = cpu_to_target32(crc32(0, &ri, 
		    sizeof(struct jffs2_unknown_node) - 4));

	ri.ino = cpu_to_target32(ino);
	ri.mode = cpu_to_target32(statbuf->st_mode);
	ri.uid = cpu_to_target32(statbuf->st_uid);
	ri.gid = cpu_to_target32(statbuf->st_gid);
	ri.atime = cpu_to_target32(statbuf->st_atime);
	ri.ctime = cpu_to_target32(statbuf->st_ctime);
	ri.mtime = cpu_to_target32(statbuf->st_mtime);
	ri.isize = cpu_to_target32(statbuf->st_size);
	ri.version = cpu_to_target32(1);
	ri.csize = cpu_to_target32(len);
	ri.dsize = cpu_to_target32(len);
	ri.node_crc = cpu_to_target32(crc32(0, &ri, sizeof(ri) - 8));
	ri.data_crc = cpu_to_target32(crc32(0, target, len));

	pad_block_if_less_than(sizeof(ri) + len);

	full_write(out_fd, &ri, sizeof(ri));
	full_write(out_fd, target, len);
	padword();
}

static void recursive_add_directory(char *dname)
{
	DIR *dir;
	char path[BUFSIZ];
	struct dirent *dp;
	struct stat sb;
	struct directory_entry *parent;


	sprintf(path, "%s", dname);

	if (lstat(path, &sb)) {
		perror_msg_and_die("%s", path);
	}

	parent =
		add_new_directory(dname, path, sb.st_uid, sb.st_gid, sb.st_mode);
	dir = opendir(path);
	if (!dir) {
		perror_msg_and_die("opening directory %s", path);
	}
	while ((dp = readdir(dir))) {

		if (dp->d_name[0] == '.' && (dp->d_name[1] == 0 || 
			    (dp->d_name[1] == '.' && dp->d_name[2] == 0))) {
			continue;
		}
		if (strcmp(dname, ".") == 0)
			sprintf(path, "%s", dp->d_name);
		else
			sprintf(path, "%s/%s", dname, dp->d_name);
		if (lstat(path, &sb)) {
			perror_msg_and_die("%s", path);
		}

		switch (sb.st_mode & S_IFMT) {
		case S_IFDIR:
			recursive_add_directory(path);
			break;

		case S_IFREG:
			if (sb.st_size==0) {
				char devname[32];
				char type;
				int major;
				int minor;

				if (sscanf(dp->d_name,
						"@%31[-a-zA-Z0-9_+],%c,%d,%d",
						devname, &type, &major, &minor) == 4) {
					strcpy(dp->d_name, devname);
					sb.st_rdev = makedev(major, minor);
					sb.st_mode &= ~S_IFMT;
					switch (type) {
					case 'c':
					case 'u':
						sb.st_mode |= S_IFCHR;
						goto charnode;
					case 'b':
						sb.st_mode |= S_IFBLK;
						goto blocknode;
					case 'p':
						sb.st_mode |= S_IFIFO;
						goto pipenode;
					default:
						error_msg_and_die("%s: invalid special device type '%c'", dp->d_name, type);
						
						break;
					}
				}
			}
			/* fallthrough */
		case S_IFSOCK:
		case S_IFIFO:
pipenode:
		case S_IFLNK:
			add_new_file(dp->d_name, path, sb.st_uid, sb.st_gid,
						 sb.st_mode, parent);
			break;
		case S_IFCHR:
charnode:
		case S_IFBLK:
blocknode:
			add_new_device(dp->d_name, path, sb.st_uid, sb.st_gid,
						   sb.st_mode, sb.st_rdev, parent);
			break;

		default:
			error_msg("Unknown file type %o for %s", sb.st_mode, path);
			break;
		}
	}

	closedir(dir);
}

/*  device table entries take the form of:
    <path>	<type> <mode>	<uid>	<gid>	<major>	<minor>	<start>	<inc>	<count>
    /dev/mem    c      640      0       0       1       1       0        0        -

    type can be one of: 
	f	A regular file
	d	Directory
	c	Character special device file
	b	Block special device file
	p	Fifo (named pipe)

    I don't bother with symlinks (permissions are irrelevant), hard
    links (special cases of regular files), or sockets (why bother).

    Regular files must exist in the target root directory.  If a char,
    block, fifo, or directory does not exist, it will be created.
*/
static int interpret_table_entry(char *line)
{
	char *name;
	char path[BUFSIZ], type;
	unsigned long mode = 0755, uid = 0, gid = 0, major = 0, minor = 0;
	unsigned long start = 0, increment = 1, count = 0;

	if (0 > sscanf(line, "%40s %c %lo %lu %lu %lu %lu %lu %lu %lu", path,
		    &type, &mode, &uid, &gid, &major, &minor, &start,
		    &increment, &count)) 
	{
		return 1;
	}

	if (!strcmp(path, "/")) {
		error_msg_and_die("Device table entries require absolute paths");
	}
	name = xstrdup(path + 1);
	sprintf(path, "%s/%s", rootdir, name);

	switch (type) {
	case 'd':
		mode |= S_IFDIR;
		add_new_directory(name, path, uid, gid, mode);
		break;
	case 'f':
		mode |= S_IFREG;
		add_new_file(name, path, uid, gid, mode, NULL);
		break;
	case 'p':
		mode |= S_IFIFO;
		add_new_file(name, path, uid, gid, mode, NULL);
		break;
	case 'c':
	case 'b':
		mode |= (type == 'c') ? S_IFCHR : S_IFBLK;
		if (count > 0) {
			int i;
			dev_t rdev;
			char buf[80];

			for (i = start; i < count; i++) {
				sprintf(buf, "%s%d", name, i);
				/* FIXME:  MKDEV uses illicit insider knowledge of kernel 
				 * major/minor representation...  */
				rdev = MKDEV(major, minor + (i * increment - start));
				add_new_device(buf, path, uid, gid, mode, rdev, NULL);
			}
		} else {
			/* FIXME:  MKDEV uses illicit insider knowledge of kernel 
			 * major/minor representation...  */
			dev_t rdev = MKDEV(major, minor);

			add_new_device(name, path, uid, gid, mode, rdev, NULL);
		}
		break;
	default:
		error_msg_and_die("Unsupported file type");
	}
	free(name);
	return 0;
}

static int parse_device_table(FILE * file)
{
	char *line;
	int status = 0;
	size_t length = 0;

	/* Turn off squash, since we must ensure that values
	 * entered via the device table are not squashed */
	squash = 0;

	/* Looks ok so far.  The general plan now is to read in one
	 * line at a time, check for leading comment delimiters ('#'),
	 * then try and parse the line as a device table.  If we fail
	 * to parse things, try and help the poor fool to fix their
	 * device table with a useful error msg... */
	line = NULL;
	while (getline(&line, &length, file) != -1) {
		/* First trim off any whitespace */
		int len = strlen(line);

		/* trim trailing whitespace */
		while (len > 0 && isspace(line[len - 1]))
			line[--len] = '\0';
		/* trim leading whitespace */
		memmove(line, &line[strspn(line, " \n\r\t\v")], len);

		/* If this is NOT a comment line, try to interpret it */
		if (length && *line != '#') {
			if (interpret_table_entry(line))
				status = 1;
		}

		free(line);
		line = NULL;
	}
	fclose(file);

	return status;
}

static void create_target_filesystem(void)
{
	char *name;
	struct stat sb;
	time_t timestamp;
	struct directory_entry *tmp_dir;
	struct filesystem_entry *tmp_file;
	uint32_t parent_ino = 1, ino;


	tmp_dir = dir_list;
	while (tmp_dir) {
		parent_ino = 1;
		timestamp = tmp_dir->sb.st_mtime;
		name = tmp_dir->name;
		sb = tmp_dir->sb;
		if (!tmp_dir->parent) {
			/* Cope with the root directory */
			ino = highest_ino++;
			mkfs_debug_msg("writing '%s' ino=%lu", name, (unsigned long) ino);
			/* The root directory need not have a inode */
			//output_pipe(ino, &sb);
			/* The root directory must not have a dirent */
			//write_dirent(ino, version++, ino, timestamp, DT_DIR, name);
			tmp_dir = tmp_dir->next;
			continue;
		}

		parent_ino = tmp_dir->parent->jffs2_ino;
		tmp_dir->jffs2_ino = ino = highest_ino++;

		mkfs_debug_msg("writing dir '%s'  ino=%lu  parent_ino=%lu", name,
				  (unsigned long) ino, (unsigned long) parent_ino);
		name = basename(tmp_dir->name);
		write_dirent(parent_ino, version++, ino, timestamp, DT_DIR, name);
		output_pipe(ino, &sb);

		tmp_dir = tmp_dir->next;
	}

	tmp_file = file_list;
	while (tmp_file) {
		parent_ino = 1;
		sb = tmp_file->sb;
		name = basename(tmp_file->name);
		timestamp = tmp_file->sb.st_mtime;
		ino = tmp_file->jffs2_ino = highest_ino++;
		if (tmp_file->parent)
			parent_ino = tmp_file->parent->jffs2_ino;

		switch (sb.st_mode & S_IFMT) {
		case S_IFSOCK:
			write_dirent(parent_ino, version++, ino, timestamp, DT_SOCK, name);
			output_pipe(ino, &sb);
			break;

		case S_IFIFO:
			write_dirent(parent_ino, version++, ino, timestamp, DT_FIFO, name);
			output_pipe(ino, &sb);
			break;

		case S_IFLNK:
		{
			int len;
			char target[JFFS2_MAX_SYMLINK_LEN + 1];

			len = readlink(tmp_file->path, target, JFFS2_MAX_SYMLINK_LEN);
			if (len < 0) {
				perror_msg_and_die("readlink");
			}
			if (len > JFFS2_MAX_SYMLINK_LEN) {
				error_msg("symlink too large. Truncated to %d chars.",
					JFFS2_MAX_SYMLINK_LEN);
				len = JFFS2_MAX_SYMLINK_LEN;
			}
			mkfs_debug_msg("writing symlink '%s'  ino=%lu  parent_ino=%lu", 
				name, (unsigned long) ino, (unsigned long) parent_ino);
			write_dirent(parent_ino, version++, ino, timestamp, DT_LNK, name);
			output_symlink(target, len, ino, &sb);
		}
			break;

		case S_IFREG:
		{
			FILE *file;

			if (sb.st_size >= JFFS2_MAX_FILE_SIZE) {
				error_msg_and_die("File \"%s\" too large.", tmp_file->path);
			}
			file = xfopen(tmp_file->path, "r");
			mkfs_debug_msg("writing file '%s'  ino=%lu  parent_ino=%lu", name,
					  (unsigned long) ino, (unsigned long) parent_ino);
			write_dirent(parent_ino, version++, ino, timestamp, DT_REG, name);
			output_reg(fileno(file), ino, &sb);
			fclose(file);
		}
			break;

		case S_IFCHR:
			write_dirent(parent_ino, version++, ino, timestamp, DT_CHR, name);
			output_dev(ino, &sb);
			break;

		case S_IFBLK:
			write_dirent(parent_ino, version++, ino, timestamp, DT_BLK, name);
			output_dev(ino, &sb);
			break;

		case S_IFDIR:
			error_msg_and_die("There are not supposed to be directories in the file list!");

		default:
			error_msg("Unknown mode %o for %s", sb.st_mode, tmp_file->path);
			break;
		}
		tmp_file = tmp_file->next;
	}
}

static void cleanup(void)
{
	struct directory_entry *tmp_dir, *next;
	struct filesystem_entry *tmp_file, *fnext;

	tmp_dir = next = dir_list;
	while (tmp_dir) {
		next = tmp_dir->next;
		free(tmp_dir->name);
		free(tmp_dir->path);
		free(tmp_dir->base);
		free(tmp_dir);
		tmp_dir = next;
	}

	tmp_file = fnext = file_list;
	while (tmp_file) {
		fnext = tmp_file->next;
		free(tmp_file->name);
		free(tmp_file->path);
		free(tmp_file);
		tmp_file = fnext;
	}
	if (rootdir != default_rootdir)
		free(rootdir);
}

static int go(char *dname, FILE * devtable)
{
	struct stat sb;

	if (lstat(dname, &sb)) {
		perror_msg_and_die("%s", dname);
	}
	if (chdir(dname))
		perror_msg_and_die("%s", dname);

	recursive_add_directory(".");
	if (devtable)
		parse_device_table(devtable);
	create_target_filesystem();

	cleanup();
	return 0;
}


static struct option long_options[] = {
	{"pad", 2, NULL, 'p'},
	{"root", 1, NULL, 'r'},
	{"pagesize", 1, NULL, 's'},
	{"eraseblock", 1, NULL, 'e'},
	{"output", 1, NULL, 'o'},
	{"help", 0, NULL, 'h'},
	{"version", 0, NULL, 'v'},
	{"big-endian", 0, NULL, 'b'},
	{"little-endian", 0, NULL, 'l'},
	{"squash", 0, NULL, 'q'},
	{"faketime", 0, NULL, 'f'},
	{"devtable", 1, NULL, 'D'},
	{NULL, 0, NULL, 0}
};

static char *helptext =
	"Usage: mkfs.jffs2 [OPTIONS]\n"
	"Make a JFFS2 filesystem image from an existing directory tree\n\n"
	"Options:\n"
	"  -p, --pad[=SIZE]       Pad output to SIZE bytes with 0xFF. If SIZE is\n"
	"                         not specified, the output is padded to the end of\n"
	"                         the final erase block\n"
	"  -r, -d, --root=DIR     Build filesystem from directory DIR (default: cwd)\n"
	"  -s, --pagesize=SIZE    Use page size (max data node size) SIZE (default: 4KiB)\n"
	"  -e, --eraseblock=SIZE  Use erase block size SIZE (default: 64KiB)\n"
	"  -o, --output=FILE      Output to FILE (default: stdout)\n"
	"  -l, --little-endian    Create a little-endian filesystem\n"
	"  -b, --big-endian       Create a big-endian filesystem\n"
	"  -D, --devtable=FILE    Use the named FILE as a device table file\n"
	"  -f, --faketime         Change all file times to '0' for regression testing\n"
	"  -q, --squash           Squash permissions and owners making all files be owned by root\n"
	"  -h, --help             Display this help text\n"
	"  -v, --version          Display version information\n\n";


static char *revtext = "$Revision: 1.20 $";

int main(int argc, char **argv)
{
	int c, opt;
	extern char *optarg;
	struct stat statbuf;
	FILE *devtable = NULL;

	while ((opt = getopt_long(argc, argv, "D:p::d:r:s:e:o:blqfh?v", 
			long_options, &c)) >= 0) {
		switch (opt) {
		case 'D':
			devtable = xfopen(optarg, "r");
			if (fstat(fileno(devtable), &statbuf) < 0)
				perror_msg_and_die(optarg);
			if (statbuf.st_size < 10)
				error_msg_and_die("%s: not a proper device table file", optarg);
			break;
		case 'p':
			if (optarg)
				pad_fs_size = strtol(optarg, NULL, 0);
			else
				pad_fs_size = -1;
			break;

		case 'r':
		case 'd':				/* for compatibility with mkfs.jffs, genext2fs, etc... */
			if (rootdir != default_rootdir) {
				error_msg_and_die("root directory specified more than once");
			}
			rootdir = xstrdup(optarg);
			break;

		case 's':
			page_size = strtol(optarg, NULL, 0);
			break;

		case 'e':
			erase_block_size = strtol(optarg, NULL, 0);
			break;

		case 'o':
			if (out_fd != 1) {
				error_msg_and_die("output filename specified more than once");
			}
			out_fd = open(optarg, O_CREAT | O_TRUNC | O_RDWR, 0644);
			if (out_fd == -1) {
				perror_msg_and_die("open output file");
			}
			break;

		case 'l':
			target_endian = __LITTLE_ENDIAN;
			break;

		case 'b':
			target_endian = __BIG_ENDIAN;
			break;

		case 'q':
			squash = 1;
			break;

		case 'f':
			fake_times = 1;
			break;

		case 'h':
		case '?':
			fprintf(stderr, helptext);
			exit(1);

		case 'v':
			fprintf(stderr, "mkfs.jffs2 revision %.*s\n",
					(int) strlen(revtext) - 13, revtext + 11);
			exit(1);
		}
	}

	go(rootdir, devtable);
	if (pad_fs_size == -1) {
		padblock();
	} else {
		while (out_ofs < pad_fs_size) {
			full_write(out_fd, ffbuf, min(16, pad_fs_size - out_ofs));
		}
	}

	return 0;
}
