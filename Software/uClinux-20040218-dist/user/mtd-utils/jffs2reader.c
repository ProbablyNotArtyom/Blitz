/* vi: set sw=4 ts=4: */
/* 
 * jffs2reader v0.0.18 A jffs2 image reader 
 *
 * Copyright (c) 2001 Jari Kirma <Jari.Kirma@hut.fi>
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the author be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must
 * not claim that you wrote the original software. If you use this
 * software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 * 
 * 2. Altered source versions must be plainly marked as such, and must
 * not be misrepresented as being the original software.
 * 
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 *
 *********
 *  This code was altered September 2001 
 *  Changes are Copyright (c) Erik Andersen <andersen@codepoet.org>
 *
 * In compliance with (2) above, this is hereby marked as an altered
 * version of this software.  It has been altered as follows:
 *      *) Listing a directory now mimics the behavior of 'ls -l'
 *      *) Support for recursive listing has been added
 *      *) Without options, does a recursive 'ls' on the whole filesystem
 *      *) option parsing now uses getopt()
 *      *) Now uses printf, and error messages go to stderr.
 *      *) The copyright notice has been cleaned up and reformatted
 *      *) The code has been reformatted
 *      *) Several twisty code paths have been fixed so I can understand them.
 *  -Erik, 1 September 2001
 *
 *      *) Made it show major/minor numbers for device nodes 
 *      *) Made it show symlink targets
 *  -Erik, 13 September 2001
 *
 * $Id: jffs2reader.c,v 1.4 2001/09/13 22:47:51 andersen Exp $
*/


/*
  TODO:

  - Add CRC checking code to places marked with XXX.
  - Add support for other node compression types.

  - Test with real life images.
  - Maybe port into bootloader.
*/

/*
  BUGS:

  - Doesn't check CRC checksums.
*/


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <zlib.h>
#include <linux/jffs2.h>

#define SCRATCH_SIZE (5*1024*1024)

#ifndef MAJOR
/* FIXME:  I am using illicit insider knowledge of 
 * kernel major/minor representation...  */
#define MAJOR(dev) (((dev)>>8)&0xff)
#define MINOR(dev) ((dev)&0xff)
#endif


#define DIRENT_INO(dirent) ((dirent)!=NULL?(dirent)->ino:0)
#define DIRENT_PINO(dirent) ((dirent)!=NULL?(dirent)->pino:0)

struct dir {
	struct dir *next;
	__u8 type;
	__u8 nsize;
	__u32 ino;
	char name[256];
};

void putblock(char *, size_t, size_t *, struct jffs2_raw_inode *);
struct dir *putdir(struct dir *, struct jffs2_raw_dirent *);
void printdir(char *o, size_t size, struct dir *d, char *path,
			  int recurse);
void freedir(struct dir *);

struct jffs2_raw_inode *find_raw_inode(char *o, size_t size, __u32 ino);
struct jffs2_raw_dirent *resolvedirent(char *, size_t, __u32, __u32,
									   char *, __u8);
struct jffs2_raw_dirent *resolvename(char *, size_t, __u32, char *, __u8);
struct jffs2_raw_dirent *resolveinode(char *, size_t, __u32);

struct jffs2_raw_dirent *resolvepath0(char *, size_t, __u32, char *,
									  __u32 *, int);
struct jffs2_raw_dirent *resolvepath(char *, size_t, __u32, char *,
									 __u32 *);

void lsdir(char *, size_t, char *, int);
void catfile(char *, size_t, char *, char *, size_t, size_t *);

int main(int, char **);

/* writes file node into buffer, to the proper position. */
/* reading all valid nodes in version order reconstructs the file. */

/*
  b       - buffer
  bsize   - buffer size
  rsize   - result size
  n       - node
*/

void putblock(char *b, size_t bsize, size_t * rsize,
			  struct jffs2_raw_inode *n)
{
	uLongf dlen = n->dsize;

	if (n->isize > bsize || (n->offset + dlen) > bsize) {
		fprintf(stderr, "File does not fit into buffer!\n");
		exit(EXIT_FAILURE);
	}

	if (*rsize < n->isize)
		bzero(b + *rsize, n->isize - *rsize);

	switch (n->compr) {
	case JFFS2_COMPR_ZLIB:
		uncompress((Bytef *) b + n->offset, &dlen,
				   (Bytef *) ((char *) n) + sizeof(struct jffs2_raw_inode),
				   (uLongf) n->csize);
		break;

	case JFFS2_COMPR_NONE:
		memcpy(b + n->offset,
			   ((char *) n) + sizeof(struct jffs2_raw_inode), dlen);
		break;

	case JFFS2_COMPR_ZERO:
		bzero(b + n->offset, dlen);
		break;

		/* [DYN]RUBIN support required! */

	default:
		fprintf(stderr, "Unsupported compression method!\n");
		exit(EXIT_FAILURE);
	}

	*rsize = n->isize;
}

/* adds/removes directory node into dir struct. */
/* reading all valid nodes in version order reconstructs the directory. */

/*
  dd      - directory struct being processed
  n       - node

  return value: directory struct value replacing dd
*/

struct dir *putdir(struct dir *dd, struct jffs2_raw_dirent *n)
{
	struct dir *o, *d, *p;

	o = dd;

	if (n->ino) {
		if (dd == NULL) {
			d = malloc(sizeof(struct dir));
			d->type = n->type;
			memcpy(d->name, n->name, n->nsize);
			d->nsize = n->nsize;
			d->ino = n->ino;
			d->next = NULL;

			return d;
		}

		while (1) {
			if (n->nsize == dd->nsize &&
				!memcmp(n->name, dd->name, n->nsize)) {
				dd->type = n->type;
				dd->ino = n->ino;

				return o;
			}

			if (dd->next == NULL) {
				dd->next = malloc(sizeof(struct dir));
				dd->next->type = n->type;
				memcpy(dd->next->name, n->name, n->nsize);
				dd->next->nsize = n->nsize;
				dd->next->ino = n->ino;
				dd->next->next = NULL;

				return o;
			}

			dd = dd->next;
		}
	} else {
		if (dd == NULL)
			return NULL;

		if (n->nsize == dd->nsize && !memcmp(n->name, dd->name, n->nsize)) {
			d = dd->next;
			free(dd);
			return d;
		}

		while (1) {
			p = dd;
			dd = dd->next;

			if (dd == NULL)
				return o;

			if (n->nsize == dd->nsize &&
				!memcmp(n->name, dd->name, n->nsize)) {
				p->next = dd->next;
				free(dd);

				return o;
			}
		}
	}
}


#define TYPEINDEX(mode) (((mode) >> 12) & 0x0f)
#define TYPECHAR(mode)  ("0pcCd?bB-?l?s???" [TYPEINDEX(mode)])

/* The special bits. If set, display SMODE0/1 instead of MODE0/1 */
static const mode_t SBIT[] = {
	0, 0, S_ISUID,
	0, 0, S_ISGID,
	0, 0, S_ISVTX
};

/* The 9 mode bits to test */
static const mode_t MBIT[] = {
	S_IRUSR, S_IWUSR, S_IXUSR,
	S_IRGRP, S_IWGRP, S_IXGRP,
	S_IROTH, S_IWOTH, S_IXOTH
};

static const char MODE1[] = "rwxrwxrwx";
static const char MODE0[] = "---------";
static const char SMODE1[] = "..s..s..t";
static const char SMODE0[] = "..S..S..T";

/*
 * Return the standard ls-like mode string from a file mode.
 * This is static and so is overwritten on each call.
 */
const char *mode_string(int mode)
{
	static char buf[12];

	int i;

	buf[0] = TYPECHAR(mode);
	for (i = 0; i < 9; i++) {
		if (mode & SBIT[i])
			buf[i + 1] = (mode & MBIT[i]) ? SMODE1[i] : SMODE0[i];
		else
			buf[i + 1] = (mode & MBIT[i]) ? MODE1[i] : MODE0[i];
	}
	return buf;
}

/* prints contents of directory structure */

/*
  d       - dir struct
*/

void printdir(char *o, size_t size, struct dir *d, char *path, int recurse)
{
	char m;
	char *filetime;
	time_t age;
	struct jffs2_raw_inode *ri;

	if (!path)
		return;
	if (strlen(path) == 1 && *path == '/')
		path++;

	while (d != NULL) {
		switch (d->type) {
		case DT_REG:
			m = ' ';
			break;

		case DT_FIFO:
			m = '|';
			break;

		case DT_CHR:
			m = ' ';
			break;

		case DT_BLK:
			m = ' ';
			break;

		case DT_DIR:
			m = '/';
			break;

		case DT_LNK:
			m = ' ';
			break;

		case DT_SOCK:
			m = '=';
			break;

		default:
			m = '?';
		}
		ri = find_raw_inode(o, size, d->ino);
		if (!ri) {
			fprintf(stderr, "bug: raw_inode missing!\n");
			d = d->next;
			continue;
		}

		filetime = ctime((const time_t *) &(ri->ctime));
		age = time(NULL) - ri->ctime;
		printf("%s %-4d %-8d %-8d ", mode_string(ri->mode),
			   1, ri->uid, ri->gid);
		if ( d->type==DT_BLK || d->type==DT_CHR ) {
			dev_t rdev;
			size_t devsize;
			putblock((char*)&rdev, sizeof(rdev), &devsize, ri);
			printf("%4d, %3d ", (int)MAJOR(rdev), (int)MINOR(rdev));
		} else {
			printf("%9ld ", (long)ri->dsize);
		}
		d->name[d->nsize]='\0';
		if (age < 3600L * 24 * 365 / 2 && age > -15 * 60) {
			/* hh:mm if less than 6 months old */
			printf("%6.6s %5.5s %s/%s%c", filetime + 4, filetime + 11, path, d->name, m);
		} else {
			printf("%6.6s %4.4s %s/%s%c", filetime + 4, filetime + 20, path, d->name, m);
		}
		if (d->type == DT_LNK) {
			char symbuf[1024];
			size_t symsize;
			putblock(symbuf, sizeof(symbuf), &symsize, ri);
			symbuf[symsize] = 0;
			printf(" -> %s", symbuf);
		}
		printf("\n");

		if (d->type == DT_DIR && recurse) {
			char *tmp;
			tmp = malloc(BUFSIZ);
			if (!tmp) {
				fprintf(stderr, "memory exhausted\n");
				exit(EXIT_FAILURE);
			}
			sprintf(tmp, "%s/%s", path, d->name);
			lsdir(o, size, tmp, recurse);		/* Go recursive */
			free(tmp);
		}

		d = d->next;
	}
}

/* frees memory used by directory structure */

/*
  d       - dir struct
*/

void freedir(struct dir *d)
{
	struct dir *t;

	while (d != NULL) {
		t = d->next;
		free(d);
		d = t;
	}
}

/* collects directory/file nodes in version order. */

/*
  f       - file flag.
            if zero, collect file, compare ino to inode
            otherwise, collect directory, compare ino to parent inode
  o       - filesystem image pointer
  size    - size of filesystem image
  ino     - inode to compare against. see f.

  return value: a jffs2_raw_inode that corresponds the the specified
    inode, or NULL
*/

struct jffs2_raw_inode *find_raw_inode(char *o, size_t size, __u32 ino)
{
	/* aligned! */
	union jffs2_node_union *n;
	union jffs2_node_union *e = (union jffs2_node_union *) (o + size);
	union jffs2_node_union *lr;	/* last block position */
	union jffs2_node_union *mp = NULL;	/* minimum position */

	__u32 vmin, vmint, vmaxt, vmax, vcur, v;

	vmin = 0;					/* next to read */
	vmax = ~((__u32) 0);		/* last to read */
	vmint = ~((__u32) 0);
	vmaxt = 0;					/* found maximum */
	vcur = 0;					/* XXX what is smallest version number used? */
	/* too low version number can easily result excess log rereading */

	n = (union jffs2_node_union *) o;
	lr = n;

	do {
		while (n < e && n->u.magic != JFFS2_MAGIC_BITMASK)
			((char *) n) += 4;

		if (n < e && n->u.magic == JFFS2_MAGIC_BITMASK) {
			if (n->u.nodetype == JFFS2_NODETYPE_INODE &&
				n->i.ino == ino && (v = n->i.version) > vcur) {
				/* XXX crc check */

				if (vmaxt < v)
					vmaxt = v;
				if (vmint > v) {
					vmint = v;
					mp = n;
				}

				if (v == (vcur + 1))
					return (&(n->i));
			}

			((char *) n) += ((n->u.totlen + 3) & ~3);
		} else
			n = (union jffs2_node_union *) o;	/* we're at the end, rewind to the beginning */

		if (lr == n) {			/* whole loop since last read */
			vmax = vmaxt;
			vmin = vmint;
			vmint = ~((__u32) 0);

			if (vcur < vmax && vcur < vmin)
				return (&(mp->i));
		}
	} while (vcur < vmax);

	return NULL;
}

/* collects dir struct for selected inode */

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  pino    - inode of the specified directory
  d       - input directory structure

  return value: result directory structure, replaces d.
*/

struct dir *collectdir(char *o, size_t size, __u32 ino, struct dir *d)
{
	/* aligned! */
	union jffs2_node_union *n;
	union jffs2_node_union *e = (union jffs2_node_union *) (o + size);
	union jffs2_node_union *lr;	/* last block position */
	union jffs2_node_union *mp = NULL;	/* minimum position */

	__u32 vmin, vmint, vmaxt, vmax, vcur, v;

	vmin = 0;					/* next to read */
	vmax = ~((__u32) 0);		/* last to read */
	vmint = ~((__u32) 0);
	vmaxt = 0;					/* found maximum */
	vcur = 0;					/* XXX what is smallest version number used? */
	/* too low version number can easily result excess log rereading */

	n = (union jffs2_node_union *) o;
	lr = n;

	do {
		while (n < e && n->u.magic != JFFS2_MAGIC_BITMASK)
			((char *) n) += 4;

		if (n < e && n->u.magic == JFFS2_MAGIC_BITMASK) {
			if (n->u.nodetype == JFFS2_NODETYPE_DIRENT &&
					n->d.pino == ino && (v = n->d.version) > vcur) {
				/* XXX crc check */

				if (vmaxt < v)
					vmaxt = v;
				if (vmint > v) {
					vmint = v;
					mp = n;
				}

				if (v == (vcur + 1)) {
					d = putdir(d, &(n->d));

					lr = n;
					vcur++;
					vmint = ~((__u32) 0);
				}
			}

			((char *) n) += ((n->u.totlen + 3) & ~3);
		} else
			n = (union jffs2_node_union *) o;	/* we're at the end, rewind to the beginning */

		if (lr == n) {			/* whole loop since last read */
			vmax = vmaxt;
			vmin = vmint;
			vmint = ~((__u32) 0);

			if (vcur < vmax && vcur < vmin) {
				d = putdir(d, &(mp->d));

				lr = n =
					(union jffs2_node_union *) (((char *) mp) +
												((mp->u.totlen + 3) & ~3));

				vcur = vmin;
			}
		}
	} while (vcur < vmax);

	return d;
}



/* resolve dirent based on criteria */

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  ino     - if zero, ignore,
            otherwise compare against dirent inode
  pino    - if zero, ingore,
            otherwise compare against parent inode
            and use name and nsize as extra criteria
  name    - name of wanted dirent, used if pino!=0
  nsize   - length of name of wanted dirent, used if pino!=0

  return value: pointer to relevant dirent structure in
                filesystem image or NULL
*/

struct jffs2_raw_dirent *resolvedirent(char *o, size_t size,
									   __u32 ino, __u32 pino,
									   char *name, __u8 nsize)
{
	/* aligned! */
	union jffs2_node_union *n;
	union jffs2_node_union *e = (union jffs2_node_union *) (o + size);

	struct jffs2_raw_dirent *dd = NULL;

	__u32 vmax, v;

	if (!pino && ino <= 1)
		return dd;

	vmax = 0;

	n = (union jffs2_node_union *) o;

	do {
		while (n < e && n->u.magic != JFFS2_MAGIC_BITMASK)
			((char *) n) += 4;

		if (n < e && n->u.magic == JFFS2_MAGIC_BITMASK) {
			if (n->u.nodetype == JFFS2_NODETYPE_DIRENT &&
				(!ino || n->d.ino == ino) &&
				(v = n->d.version) > vmax &&
				(!pino || (n->d.pino == pino &&
						   nsize == n->d.nsize &&
						   !memcmp(name, n->d.name, nsize)))) {
				/* XXX crc check */

				if (vmax < v) {
					vmax = v;
					dd = &(n->d);
				}
			}

			((char *) n) += ((n->u.totlen + 3) & ~3);
		} else
			return dd;
	} while (1);
}

/* resolve name under certain parent inode to dirent */

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  pino    - requested parent inode
  name    - name of wanted dirent
  nsize   - length of name of wanted dirent

  return value: pointer to relevant dirent structure in
                filesystem image or NULL
*/

struct jffs2_raw_dirent *resolvename(char *o, size_t size, __u32 pino,
									 char *name, __u8 nsize)
{
	return resolvedirent(o, size, 0, pino, name, nsize);
}

/* resolve inode to dirent */

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  ino     - compare against dirent inode

  return value: pointer to relevant dirent structure in
                filesystem image or NULL
*/

struct jffs2_raw_dirent *resolveinode(char *o, size_t size, __u32 ino)
{
	return resolvedirent(o, size, ino, 0, NULL, 0);
}

/* resolve slash-style path into dirent and inode.
   slash as first byte marks absolute path (root=inode 1).
   . and .. are resolved properly, and symlinks are followed.
*/

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  ino     - root inode, used if path is relative
  p       - path to be resolved
  inos    - result inode, zero if failure
  recc    - recursion count, to detect symlink loops

  return value: pointer to dirent struct in file system image.
                note that root directory doesn't have dirent struct
                (return value is NULL), but it has inode (*inos=1)
*/

struct jffs2_raw_dirent *resolvepath0(char *o, size_t size, __u32 ino,
									  char *p, __u32 * inos, int recc)
{
	struct jffs2_raw_dirent *dir = NULL;

	int d = 1;
	__u32 tino;

	char *next;

	char *path, *pp;

	char symbuf[1024];
	size_t symsize;

	if (recc > 16) {
		/* probably symlink loop */
		*inos = 0;
		return NULL;
	}

	pp = path = strdup(p);

	if (*path == '/') {
		path++;
		ino = 1;
	}

	if (ino > 1) {
		dir = resolveinode(o, size, ino);

		ino = DIRENT_INO(dir);
	}

	next = path - 1;

	while (ino && next != NULL && next[1] != 0 && d) {
		path = next + 1;
		next = strchr(path, '/');

		if (next != NULL)
			*next = 0;

		if (*path == '.' && path[1] == 0)
			continue;
		if (*path == '.' && path[1] == '.' && path[2] == 0) {
			if (DIRENT_PINO(dir) == 1) {
				ino = 1;
				dir = NULL;
			} else {
				dir = resolveinode(o, size, DIRENT_PINO(dir));
				ino = DIRENT_INO(dir);
			}

			continue;
		}

		dir = resolvename(o, size, ino, path, (__u8) strlen(path));

		if (DIRENT_INO(dir) == 0 ||
			(next != NULL &&
			 !(dir->type == DT_DIR || dir->type == DT_LNK))) {
			free(pp);

			*inos = 0;

			return NULL;
		}

		if (dir->type == DT_LNK) {
			struct jffs2_raw_inode *ri;
			ri = find_raw_inode(o, size, DIRENT_INO(dir));
			putblock(symbuf, sizeof(symbuf), &symsize, ri);
			symbuf[symsize] = 0;

			tino = ino;
			ino = 0;

			dir = resolvepath0(o, size, tino, symbuf, &ino, ++recc);

			if (dir != NULL && next != NULL &&
				!(dir->type == DT_DIR || dir->type == DT_LNK)) {
				free(pp);

				*inos = 0;
				return NULL;
			}
		}
		if (dir != NULL)
			ino = DIRENT_INO(dir);
	}

	free(pp);

	*inos = ino;

	return dir;
}

/* resolve slash-style path into dirent and inode.
   slash as first byte marks absolute path (root=inode 1).
   . and .. are resolved properly, and symlinks are followed.
*/

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  ino     - root inode, used if path is relative
  p       - path to be resolved
  inos    - result inode, zero if failure

  return value: pointer to dirent struct in file system image.
                note that root directory doesn't have dirent struct
                (return value is NULL), but it has inode (*inos=1)
*/

struct jffs2_raw_dirent *resolvepath(char *o, size_t size, __u32 ino,
									 char *p, __u32 * inos)
{
	return resolvepath0(o, size, ino, p, inos, 0);
}

/* lists files on directory specified by path */

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  p       - path to be resolved
*/

void lsdir(char *o, size_t size, char *path, int recurse)
{
	struct jffs2_raw_dirent *dd;
	struct dir *d = NULL;

	__u32 ino;

	dd = resolvepath(o, size, 1, path, &ino);

	if (ino == 0 ||
		(dd == NULL && ino == 0) || (dd != NULL && dd->type != DT_DIR)) {
		fprintf(stderr, "jffs2reader: %s: No such file or directory\n",
				path);
		exit(EXIT_FAILURE);
	}

	d = collectdir(o, size, ino, d);
	printdir(o, size, d, path, recurse);
	freedir(d);
}

/* writes file specified by path to the buffer */

/*
  o       - filesystem image pointer
  size    - size of filesystem image
  p       - path to be resolved
  b       - file buffer
  bsize   - file buffer size
  rsize   - file result size
*/

void catfile(char *o, size_t size, char *path, char *b, size_t bsize,
			 size_t * rsize)
{
	struct jffs2_raw_dirent *dd;
	struct jffs2_raw_inode *ri;
	__u32 ino;

	dd = resolvepath(o, size, 1, path, &ino);

	if (ino == 0) {
		fprintf(stderr, "%s: No such file or directory\n", path);
		exit(EXIT_FAILURE);
	}

	if (dd == NULL || dd->type != DT_REG) {
		fprintf(stderr, "%s: Not a regular file\n", path);
		exit(EXIT_FAILURE);
	}

	ri = find_raw_inode(o, size, ino);
	putblock(b, bsize, rsize, ri);

	write(1, b, *rsize);
}

/* usage example */

int main(int argc, char **argv)
{
	int fd, opt, recurse = 0;
	struct stat st;

	char *scratch, *dir = NULL, *file = NULL;
	size_t ssize = 0;

	char *buf;

	while ((opt = getopt(argc, argv, "rd:f:")) > 0) {
		switch (opt) {
		case 'd':
			dir = optarg;
			break;
		case 'f':
			file = optarg;
			break;
		case 'r':
			recurse++;
			break;
		default:
			fprintf(stderr,
					"Usage: jffs2reader <image> [-d|-f] < path > \n");
			exit(EXIT_FAILURE);
		}
	}

	fd = open(argv[optind], O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "%s: %s\n", argv[optind], strerror(errno));
		exit(2);
	}

	if (fstat(fd, &st)) {
		fprintf(stderr, "%s: %s\n", argv[optind], strerror(errno));
		exit(3);
	}

	buf = malloc((size_t) st.st_size);
	if (buf == NULL) {
		fprintf(stderr, "%s: memory exhausted\n", argv[optind]);
		exit(4);
	}

	if (read(fd, buf, st.st_size) != (ssize_t) st.st_size) {
		fprintf(stderr, "%s: %s\n", argv[optind], strerror(errno));
		exit(5);
	}

	if (dir)
		lsdir(buf, st.st_size, dir, recurse);

	if (file) {
		scratch = malloc(SCRATCH_SIZE);
		if (scratch == NULL) {
			fprintf(stderr, "%s: memory exhausted\n", argv[optind]);
			exit(6);
		}

		catfile(buf, st.st_size, file, scratch, SCRATCH_SIZE, &ssize);
		free(scratch);
	}

	if (!dir && !file)
		lsdir(buf, st.st_size, "/", 1);


	free(buf);
	exit(EXIT_SUCCESS);
}
