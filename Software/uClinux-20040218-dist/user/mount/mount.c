/*
 * A mount(8) for Linux 0.99.
 * mount.c,v 1.1.1.1 1993/11/18 08:40:51 jrs Exp
 *
 * Thu Jul 14 07:32:40 1994: faith@cs.unc.edu added changes from Adam
 * J. Richter (adam@adam.yggdrasil.com) so that /proc/filesystems is used
 * if no -t option is given.  I modified his patches so that, if
 * /proc/filesystems is not available, the behavior of mount is the same as
 * it was previously.
 *
 * Wed Sep 14 22:43:00 1994: Mitchum DSouza
 * (mitch@mrc-applied-psychology.cambridge.ac.uk) added support for mounting
 * the "loop" device.
 *
 * Wed Sep 14 22:55:10 1994: Sander van Malssen (svm@kozmix.hacktic.nl)
 * added support for remounting readonly file systems readonly.
 *
 * Wed Feb 8 09:23:18 1995: Mike Grupenhoff <kashmir@umiacs.UMD.EDU> added
 * a probe of the superblock for the type before /proc/filesystems is
 * checked.
 *
 * Wed Feb  8 12:27:00 1995: Andries.Brouwer@cwi.nl fixed up error messages.
 * Sat Jun  3 20:44:38 1995: Patches from Andries.Brouwer@cwi.nl applied.
 * Tue Sep 26 22:38:20 1995: aeb@cwi.nl, many changes
 * Fri Feb 23 13:47:00 1996: aeb@cwi.nl, loop device related changes
 *
 * Fri Apr  5 01:13:33 1996: quinlan@bucknell.edu, fixed up iso9660 autodetect
 *
 * Since then, many changes - aeb.
 *
 * Wed Oct  1 23:55:28 1997: Dick Streefland <dick_streefland@tasking.com>
 * Implemented the "bg", "fg" and "retry" mount options for NFS.
 *
 * Tue Aug  4 15:54:31 1998: aeb@cwi.nl:
 * Open fd 0,1,2 so that printf's do not clobber /etc/mtab or so.
 * Mangle filenames with embedded spaces. Add ufsmagic. Add locking.
 * Avoid unnecessary error messages about /proc.
 * Improve support for noncanonical names in /etc/fstab.
 */

#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>

#include "mount_constants.h"
#include "sundries.h"
#include "mntent.h"
#include "fstab.h"
#include "lomount.h"
#include "loop.h"
#include "linux_fs.h"

#define PROC_FILESYSTEMS	"/proc/filesystems"
#define SIZE(a) (sizeof(a)/sizeof(a[0]))

#define DO_PS_FIDDLING

#ifdef DO_PS_FIDDLING
#define PROC_NAME		"mount: "
static int argc0;
static char** argv0;
static char** envp0;
extern char** environ;
#endif

/* True for fake mount (-f).  */
int fake = 0;

/* Don't write a entry in /etc/mtab (-n).  */
#ifdef EMBED
int nomtab = 1;
#else
int nomtab = 0;
#endif

/* True for explicit readonly (-r).  */
int readonly = 0;

/* Nonzero for chatty (-v).  */
int verbose = 0;

/* Nonzero for sloppy (-s).  */
int sloppy = 0;

/* True for explicit read/write (-w).  */
int readwrite = 0;

/* True for all mount (-a).  */
int all = 0;

/* True for fork() during all mount (-F).  */
int optfork = 0;

/* True if ruid != euid.  */
int suid = 0;

/* Map from -o and fstab option strings to the flag argument to mount(2).  */
struct opt_map
{
  const char *opt;		/* option name */
  int  skip;			/* skip in mtab option string */
  int  inv;			/* true if flag value should be inverted */
  int  mask;			/* flag mask value */
};

/* Custom mount options for our own purposes.  */
/* We can use the high-order 16 bits, since the mount call
   has MS_MGC_VAL there. */
#define MS_NOAUTO	0x80000000
#define MS_USER		0x40000000
#define MS_LOOP		0x00010000

/* Options that we keep the mount system call from seeing.  */
#define MS_NOSYS	(MS_NOAUTO|MS_USER|MS_LOOP)

/* Options that we keep from appearing in the options field in the mtab.  */
#define MS_NOMTAB	(MS_REMOUNT|MS_NOAUTO|MS_USER)

/* OPTIONS that we make ordinary users have by default.  */
#define MS_SECURE	(MS_NOEXEC|MS_NOSUID|MS_NODEV)

const struct opt_map opt_map[] = {
  { "defaults",	0, 0, 0		},	/* default options */
  { "ro",	1, 0, MS_RDONLY	},	/* read-only */
  { "rw",	1, 1, MS_RDONLY	},	/* read-write */
  { "exec",	0, 1, MS_NOEXEC	},	/* permit execution of binaries */
  { "noexec",	0, 0, MS_NOEXEC	},	/* don't execute binaries */
  { "suid",	0, 1, MS_NOSUID	},	/* honor suid executables */
  { "nosuid",	0, 0, MS_NOSUID	},	/* don't honor suid executables */
  { "dev",	0, 1, MS_NODEV	},	/* interpret device files  */
  { "nodev",	0, 0, MS_NODEV	},	/* don't interpret devices */
  { "sync",	0, 0, MS_SYNCHRONOUS},	/* synchronous I/O */
  { "async",	0, 1, MS_SYNCHRONOUS},	/* asynchronous I/O */
  { "remount",  0, 0, MS_REMOUNT},      /* Alter flags of mounted FS */
  { "auto",	0, 1, MS_NOAUTO	},	/* Can be mounted using -a */
  { "noauto",	0, 0, MS_NOAUTO	},	/* Can  only be mounted explicitly */
  { "user",	0, 0, MS_USER	},	/* Allow ordinary user to mount */
  { "nouser",	0, 1, MS_USER	},	/* Forbid ordinary user to mount */
  /* add new options here */
#ifdef MS_NOSUB
  { "sub",	0, 1, MS_NOSUB	},	/* allow submounts */
  { "nosub",	0, 0, MS_NOSUB	},	/* don't allow submounts */
#endif
#ifdef MS_SILENT
  { "quiet",	0, 0, MS_SILENT    },	/* be quiet  */
  { "loud",	0, 1, MS_SILENT    },	/* print out messages. */
#endif
#ifdef MS_MANDLOCK
  { "mand",	0, 0, MS_MANDLOCK },	/* Allow mandatory locks on this FS */
  { "nomand",	0, 1, MS_MANDLOCK },	/* Forbid mandatory locks on this FS */
#endif
  { "loop",	1, 0, MS_LOOP	},	/* use a loop device */
#ifdef MS_NOATIME
  { "atime",	0, 1, MS_NOATIME },	/* Update access time */
  { "noatime",	0, 0, MS_NOATIME },	/* Do not update access time */
#endif
#ifdef MS_NODIRATIME
  { "diratime",	0, 1, MS_NODIRATIME },	/* Update dir access times */
  { "nodiratime", 0, 0, MS_NODIRATIME },/* Do not update dir access times */
#endif
  { NULL,	0, 0, 0		}
};

char *opt_loopdev, *opt_vfstype, *opt_offset, *opt_encryption;

struct string_opt_map {
  char *tag;
  int skip;
  char **valptr;
} string_opt_map[] = {
  { "loop=",	0, &opt_loopdev },
  { "vfs=",	1, &opt_vfstype },
  { "offset=",	0, &opt_offset },
  { "encryption=", 0, &opt_encryption },
  { NULL, 0, NULL }
};

static void
clear_string_opts(void) {
  struct string_opt_map *m;

  for (m = &string_opt_map[0]; m->tag; m++)
    *(m->valptr) = NULL;
}

static int
parse_string_opt(char *s) {
  struct string_opt_map *m;
  int lth;

  for (m = &string_opt_map[0]; m->tag; m++) {
    lth = strlen(m->tag);
    if (!strncmp(s, m->tag, lth)) {
      *(m->valptr) = xstrdup(s + lth);
      return 1;
    }
  }
  return 0;
}

int mount_quiet=0;

/* Report on a single mount.  */
static void
print_one (const struct mntentchn *mc) {
     if (mount_quiet)
	  return;
     printf ("%s on %s", mc->mnt_fsname, mc->mnt_dir);
     if (mc->mnt_type != NULL && *(mc->mnt_type) != '\0')
	  printf (" type %s", mc->mnt_type);
     if (mc->mnt_opts != NULL)
	  printf (" (%s)", mc->mnt_opts);
     printf ("\n");
}

/* Report on everything in mtab (of the specified types if any).  */
static int
print_all (string_list types)
{
     struct mntentchn *mc;

     for (mc = mtab_head()->nxt; mc; mc = mc->nxt) {
	  if (matching_type (mc->mnt_type, types))
	       print_one (mc);
     }
     exit (0);
}


/* Look for OPT in opt_map table and return mask value.  If OPT isn't found,
   tack it onto extra_opts (which is non-NULL).  */
static inline void
parse_opt (const char *opt, int *mask, char *extra_opts)
{
  const struct opt_map *om;

  for (om = opt_map; om->opt != NULL; om++)
    if (streq (opt, om->opt))
      {
	if (om->inv)
	  *mask &= ~om->mask;
	else
	  *mask |= om->mask;
	if (om->mask == MS_USER)
	  *mask |= MS_SECURE;
#ifdef MS_SILENT
        if (om->mask == MS_SILENT && om->inv)  {
          mount_quiet = 1;
          verbose = 0;
        }
#endif
	return;
      }
  if (*extra_opts)
    strcat(extra_opts, ",");
  strcat(extra_opts, opt);
}
  
/* Take -o options list and compute 4th and 5th args to mount(2).  flags
   gets the standard options and extra_opts anything we don't recognize.  */
static void
parse_opts (char *opts, int *flags, char **extra_opts)
{
  char *opt;

  *flags = 0;
  *extra_opts = NULL;

  clear_string_opts();

  if (opts != NULL)
    {
      *extra_opts = xmalloc (strlen (opts) + 1); 
      **extra_opts = '\0';

      for (opt = strtok (opts, ","); opt; opt = strtok (NULL, ","))
	if (!parse_string_opt (opt))
	  parse_opt (opt, flags, *extra_opts);
    }

  if (readonly)
    *flags |= MS_RDONLY;
  if (readwrite)
    *flags &= ~MS_RDONLY;
}

/* Try to build a canonical options string.  */
static char *
fix_opts_string (int flags, char *extra_opts)
{
  const struct opt_map *om;
  const struct string_opt_map *m;
  char *new_opts;

  new_opts = (flags & MS_RDONLY) ? "ro" : "rw";
  for (om = opt_map; om->opt != NULL; om++) {
      if (om->skip)
	continue;
      if (om->inv || !om->mask || (flags & om->mask) != om->mask)
	continue;
      new_opts = xstrconcat3(new_opts, ",", om->opt);
      flags &= ~om->mask;
  }
  for (m = &string_opt_map[0]; m->tag; m++) {
      if (!m->skip && *(m->valptr))
	   new_opts = xstrconcat4(new_opts, ",", m->tag, *(m->valptr));
  }
  if (extra_opts && *extra_opts) {
      new_opts = xstrconcat3(new_opts, ",", extra_opts);
  }
  return new_opts;
}

/* Most file system types can be recognized by a `magic' number
   in the superblock.  Note that the order of the tests is
   significant: by coincidence a filesystem can have the
   magic numbers for several file system types simultaneously.
   For example, the romfs magic lives in the 1st sector;
   xiafs does not touch the 1st sector and has its magic in
   the 2nd sector; ext2 does not touch the first two sectors. */

static inline unsigned short
swapped(unsigned short a) {
     return (a>>8) | (a<<8);
}

/*
    char *fstype(const char *device);

    Probes the device and attempts to determine the type of filesystem
    contained within.

    Original routine by <jmorriso@bogomips.ww.ubc.ca>; made into a function
    for mount(8) by Mike Grupenhoff <kashmir@umiacs.umd.edu>.
    Read the superblock only once - aeb
    Added a test for iso9660 - aeb
    Added a test for high sierra (iso9660) - quinlan@bucknell.edu
    Corrected the test for xiafs - aeb
    Added romfs - aeb
    Added ufs from a patch by jj. But maybe there are several types of ufs?

    Currently supports: minix, ext, ext2, xiafs, iso9660, romfs, ufs
*/
char *magic_known[] = { "minix", "ext", "ext2", "xiafs", "iso9660", "romfs",
			"ufs" };

static int
tested(const char *device) {
    char **m;

    for (m = magic_known; m - magic_known < SIZE(magic_known); m++)
        if (!strcmp(*m, device))
	    return 1;
    return 0;
}

static char *
fstype(const char *device)
{
    int fd;
    char *type = NULL;
    union {
	struct minix_super_block ms;
	struct ext_super_block es;
	struct ext2_super_block e2s;
    } sb;
    union {
	struct xiafs_super_block xiasb;
	char romfs_magic[8];
    } xsb;
    struct ufs_super_block ufssb;
    union {
	struct iso_volume_descriptor iso;
	struct hs_volume_descriptor hs;
    } isosb;
    struct stat statbuf;

    /* opening and reading an arbitrary unknown path can have
       undesired side effects - first check that `device' refers
       to a block device */
    if (stat (device, &statbuf) || !S_ISBLK(statbuf.st_mode))
      return 0;

    fd = open(device, O_RDONLY);
    if (fd < 0)
      return 0;

    if (lseek(fd, 1024, SEEK_SET) != 1024
	|| read(fd, (char *) &sb, sizeof(sb)) != sizeof(sb))
	 goto io_error;

    if (ext2magic(sb.e2s) == EXT2_SUPER_MAGIC
	|| ext2magic(sb.e2s) == EXT2_PRE_02B_MAGIC
	|| ext2magic(sb.e2s) == swapped(EXT2_SUPER_MAGIC))
	 type = "ext2";

    else if (minixmagic(sb.ms) == MINIX_SUPER_MAGIC
	     || minixmagic(sb.ms) == MINIX_SUPER_MAGIC2)
	 type = "minix";

    else if (extmagic(sb.es) == EXT_SUPER_MAGIC)
	 type = "ext";

    if (!type) {
	 if (lseek(fd, 0, SEEK_SET) != 0
	     || read(fd, (char *) &xsb, sizeof(xsb)) != sizeof(xsb))
	      goto io_error;

	 if (xiafsmagic(xsb.xiasb) == _XIAFS_SUPER_MAGIC)
	      type = "xiafs";
	 else if(!strncmp(xsb.romfs_magic, "-rom1fs-", 8))
	      type = "romfs";
    }

    if (!type) {
	 if (lseek(fd, 8192, SEEK_SET) != 8192
	     || read(fd, (char *) &ufssb, sizeof(ufssb)) != sizeof(ufssb))
	      goto io_error;

	 if (ufsmagic(ufssb) == UFS_SUPER_MAGIC) /* also test swapped version? */
	      type = "ufs";
    }

    if (!type) {
	 if (lseek(fd, 0x8000, SEEK_SET) != 0x8000
	     || read(fd, (char *) &isosb, sizeof(isosb)) != sizeof(isosb))
	      goto io_error;

	 if(strncmp(isosb.iso.id, ISO_STANDARD_ID, sizeof(isosb.iso.id)) == 0
	    || strncmp(isosb.hs.id, HS_STANDARD_ID, sizeof(isosb.hs.id)) == 0)
	      type = "iso9660";
    }

    close (fd);
    return(type);

io_error:
    perror(device);
    close(fd);
    return 0;
}

FILE *procfs;

static void
procclose(void) {
    if (procfs)
        fclose (procfs);
    procfs = 0;
}

static int
procopen(void) {
    return ((procfs = fopen(PROC_FILESYSTEMS, "r")) != NULL);
}

static char *
procnext(void) {
   char line[100];
   static char fsname[50];

   while (fgets(line, sizeof(line), procfs)) {
      if (sscanf (line, "nodev %[^\n]\n", fsname) == 1) continue;
      if (sscanf (line, " %[^ \n]\n", fsname) != 1) continue;
      return fsname;
   }
   return 0;
}

static int
is_in_proc(char *type) {
    char *fsname;

    if (procopen()) {
	while ((fsname = procnext()) != NULL)
	  if (!strcmp(fsname, type))
	    return 1;
    }
    return 0;
}

static int
already (char *spec, char *node) {
    struct mntentchn *mc;
    int ret = 1;

    if ((mc = getmntfile(node)) != NULL)
        error ("mount: according to mtab, %s is already mounted on %s",
	       mc->mnt_fsname, node);
    else if ((mc = getmntfile(spec)) != NULL)
        error ("mount: according to mtab, %s is mounted on %s",
	       spec, mc->mnt_dir);
    else
        ret = 0;
    return ret;
}

/* Create mtab with a root entry.  */
static void
create_mtab (void) {
  struct mntentchn *fstab;
  struct mntent mnt;
  int flags;
  char *extra_opts;
  mntFILE *mfp;

  lock_mtab();

  mfp = my_setmntent (MOUNTED, "a+");
  if (mfp == NULL || mfp->mntent_fp == NULL)
    die (EX_FILEIO, "mount: can't open %s for writing: %s",
	 MOUNTED, strerror (errno));

  /* Find the root entry by looking it up in fstab */
  if ((fstab = getfsfile ("/")) || (fstab = getfsfile ("root"))) {
      parse_opts (xstrdup (fstab->mnt_opts), &flags, &extra_opts);
      mnt.mnt_dir = "/";
      mnt.mnt_fsname = canonicalize (fstab->mnt_fsname);
      mnt.mnt_type = fstab->mnt_type;
      mnt.mnt_opts = fix_opts_string (flags, extra_opts);
      mnt.mnt_freq = mnt.mnt_passno = 0;

      if (my_addmntent (mfp, &mnt) == 1)
	die (EX_FILEIO, "mount: error writing %s: %s",
	     MOUNTED, strerror (errno));
  }
  if (fchmod (fileno (mfp->mntent_fp), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) < 0)
    if (errno != EROFS)
      die (EX_FILEIO, "mount: error changing mode of %s: %s",
	   MOUNTED, strerror (errno));
  my_endmntent (mfp);

  unlock_mtab();
}

/* count successful mount system calls */
static int mountcount = 0;

static int
mount5 (char *special, char *dir, char *type, int flags, void *data) {
     int ret = mount (special, dir, type, 0xC0ED0000 | (flags), data);
     if (ret == 0)
	  mountcount++;
     return ret;
}

/* Mount a single file system.  Return status,
   so don't exit on non-fatal errors.  */
 
static int
try_mount5 (char *spec, char *node, char **type, int flags, char *mount_opts) {
   char *fsname;
   
   if (*type && strcasecmp (*type, "auto") == 0)
      *type = NULL;

   if (!*type && !(flags & MS_REMOUNT)) {
      *type = fstype(spec);
      if (verbose) {
	  printf ("mount: you didn't specify a filesystem type for %s\n",
		  spec);
	  if (*type)
	    printf ("       I will try type %s\n", *type);
	  else
	    printf ("       I will try all types mentioned in %s\n",
		    PROC_FILESYSTEMS);
      }
   }

   if (*type || (flags & MS_REMOUNT))
      return mount5 (spec, node, *type, flags & ~MS_NOSYS, mount_opts);

   if (!procopen())
     return -1;
   while ((fsname = procnext()) != NULL) {
      if (tested (fsname))
	 continue;
      if (mount5 (spec, node, fsname, flags & ~MS_NOSYS, mount_opts) == 0) {
	 *type = xstrdup(fsname);
	 procclose();
	 return 0;
      } else if (errno != EINVAL) {
         *type = "guess";
	 procclose();
	 return 1;
      }
   }
   procclose();
   *type = NULL;

   return -1;
}

/*
 * try_mount_one()
 *	Try to mount one file system. When "bg" is 1, this is a retry
 *	in the background. One additional exit code EX_BG is used here.
 *	It is used to instruct the caller to retry the mount in the
 *	background.
 */
static int
try_mount_one (char *spec0, char *node0, char *type0, char *opts0,
	       int freq, int pass, int bg)
{
  struct mntentchn mcn;
  struct mntent mnt;
  int mnt_err;
  int flags;
  char *extra_opts;		/* written in mtab */
  char *mount_opts;		/* actually used on system call */
  static int added_ro = 0;
  int loop, looptype, offset;
  char *spec, *node, *type, *opts, *loopdev, *loopfile;
  struct stat statbuf;

  spec = xstrdup(spec0);
  node = xstrdup(node0);
  type = xstrdup(type0);
  opts = xstrdup(opts0);

  parse_opts (xstrdup (opts), &flags, &extra_opts);

  /* root may allow certain types of mounts by ordinary users */
  if (suid && !(flags & MS_USER)) {
      if (already (spec, node))
	die (EX_USAGE, "mount failed");
      else
        die (EX_USAGE, "mount: only root can mount %s on %s", spec, node);
  }

  /* quietly succeed for fstab entries that don't get mounted automatically */
  if (all && (flags & MS_NOAUTO))
    return 0;

  mount_opts = extra_opts;

  /*
   * In the case of a loop mount, either type is of the form lo@/dev/loop5
   * or the option "-o loop=/dev/loop5" or just "-o loop" is given, or
   * mount just has to figure things out for itself from the fact that
   * spec is not a block device. We do not test for a block device
   * immediately: maybe later other types of mountable objects will occur.
   */

  loopdev = opt_loopdev;

  looptype = (type && strncmp("lo@", type, 3) == 0);
  if (looptype) {
    if (loopdev)
      error("mount: loop device specified twice");
    loopdev = type+3;
    type = opt_vfstype;
  }
  else if (opt_vfstype) {
    if (type)
      error("mount: type specified twice");
    else
      type = opt_vfstype;
  }

  loop = ((flags & MS_LOOP) || loopdev || opt_offset || opt_encryption);
  loopfile = spec;

  if (loop) {
    flags |= MS_LOOP;
    if (fake) {
      if (verbose)
	printf("mount: skipping the setup of a loop device\n");
    } else {
      int loopro = (flags & MS_RDONLY);

      if (!loopdev || !*loopdev)
	loopdev = find_unused_loop_device();
      if (!loopdev)
	return EX_SYSERR;	/* no more loop devices */
      if (verbose)
	printf("mount: going to use the loop device %s\n", loopdev);
      offset = opt_offset ? strtoul(opt_offset, NULL, 0) : 0;
      if (set_loop (loopdev, loopfile, offset, opt_encryption, &loopro))
	return EX_FAIL;
      spec = loopdev;
      if (loopro)
	flags |= MS_RDONLY;
    }
  }

  if (!fake && type && streq (type, "nfs")) {
#ifdef HAVE_NFS
    mnt_err = nfsmount (spec, node, &flags, &extra_opts, &mount_opts, bg);
    if (mnt_err)
      return mnt_err;
#else
    die (EX_SOFTWARE, "mount: this version was compiled "
	              "without support for the type `nfs'");
#endif
  }

#ifndef EMBED
  /*
   * Call mount.TYPE for types that require a separate
   * mount program.  For the moment these types are ncp and smb.
   */
  if (type)
#ifndef ALWAYS_STAT
  if (streq (type, "smb") || streq (type, "ncp"))
#else
  if (strlen (type) < 100)
#endif
  {
       char mountprog[120];

       sprintf(mountprog, "/sbin/mount.%s", type);
       if (stat(mountprog, &statbuf) == 0) {
	    if (fork() == 0) {
		 char *oo, *mountargs[10];
		 int i = 0;

		 setuid(getuid());
		 setgid(getgid());
		 oo = fix_opts_string (flags, extra_opts);
		 mountargs[i++] = mountprog;
		 mountargs[i++] = spec;
		 mountargs[i++] = node;
		 if (nomtab)
		      mountargs[i++] = "-n";
		 if (verbose)
		      mountargs[i++] = "-v";
		 if (oo && *oo) {
		      mountargs[i++] = "-o";
		      mountargs[i++] = oo;
		 }
		 mountargs[i] = NULL;
		 execv(mountprog, mountargs);
		 exit(1);	/* exec failed */
	    } else if (fork() != -1) {
		 int status;
		 wait(&status);
		 return status;
	    } else
		 error("cannot fork: %s", strerror(errno));
       }
  }
#endif

  block_signals (SIG_BLOCK);

  if (fake
      || (try_mount5 (spec, node, &type, flags & ~MS_NOSYS, mount_opts)) == 0)
    /* Mount succeeded, report this (if verbose) and write mtab entry.  */
    {
      if (loop)
	  opt_loopdev = loopdev;

      mcn.mnt_fsname = mnt.mnt_fsname = canonicalize (loop ? loopfile : spec);
      mcn.mnt_dir = mnt.mnt_dir = canonicalize (node);
      mcn.mnt_type = mnt.mnt_type = type ? type : "unknown";
      mcn.mnt_opts = mnt.mnt_opts = fix_opts_string (flags & ~MS_NOMTAB, extra_opts);
      mcn.nxt = 0;
      mnt.mnt_freq = freq;
      mnt.mnt_passno = pass;
      
      /* We get chatty now rather than after the update to mtab since the
	 mount succeeded, even if the write to /etc/mtab should fail.  */
      if (verbose)
	   print_one (&mcn);

      if (!nomtab && mtab_is_writable()) {
	  if (flags & MS_REMOUNT)
	       update_mtab (mnt.mnt_dir, &mnt);
	  else {
	       mntFILE *mfp;

	       lock_mtab();
	       mfp = my_setmntent(MOUNTED, "a+");
	       if (mfp == NULL || mfp->mntent_fp == NULL) {
		    error("mount: can't open %s: %s", MOUNTED,
			  strerror (errno));
	       } else {
		    if ((my_addmntent (mfp, &mnt)) == 1)
			 error("mount: error writing %s: %s", MOUNTED,
			       strerror (errno));
		    my_endmntent(mfp);
	       }
	       unlock_mtab();
	  }
      }

      block_signals (SIG_UNBLOCK);
      return 0;
    }

  mnt_err = errno;

  if (loop)
	del_loop(spec);

  block_signals (SIG_UNBLOCK);

  /* Mount failed, complain, but don't die.  */

  if (type == 0)
    error ("mount: you must specify the filesystem type");
  else
  switch (mnt_err)
    {
    case EPERM:
      if (geteuid() == 0) {
	   if (stat (node, &statbuf) || !S_ISDIR(statbuf.st_mode))
		error ("mount: mount point %s is not a directory", node);
	   else
		error ("mount: permission denied");
      } else
	error ("mount: must be superuser to use mount");
      break;
    case EBUSY:
      if (flags & MS_REMOUNT) {
	error ("mount: %s is busy", node);
      } else if (!strcmp(type, "proc") && !strcmp(node, "/proc")) {
	/* heuristic: if /proc/version exists, then probably proc is mounted */
	if (stat ("/proc/version", &statbuf))   /* proc mounted? */
	   error ("mount: %s is busy", node);   /* no */
	else if(!all || verbose)                /* yes, don't mention it */
	   error ("mount: proc already mounted");
      } else {
	error ("mount: %s already mounted or %s busy", spec, node);
	already (spec, node);
      }
      break;
    case ENOENT:
      if (lstat (node, &statbuf))
	   error ("mount: mount point %s does not exist", node);
      else if (stat (node, &statbuf))
	   error ("mount: mount point %s is a symbolic link to nowhere",
		  node);
      else if (stat (spec, &statbuf))
	   error ("mount: special device %s does not exist", spec);
      else {
           errno = mnt_err;
           perror("mount");
      }
      break;
    case ENOTDIR:
      error ("mount: mount point %s is not a directory", node);
      break;
    case EINVAL:
    { int fd, size;

      if (flags & MS_REMOUNT) {
	error ("mount: %s not mounted already, or bad option", node);
      } else {
	error ("mount: wrong fs type, bad option, bad superblock on %s,\n"
	       "       or too many mounted file systems",
	       spec);

	if (stat (spec, &statbuf) == 0 && S_ISBLK(statbuf.st_mode)
	   && (fd = open(spec, O_RDONLY)) >= 0) {
	  if(ioctl(fd, BLKGETSIZE, &size) == 0 && size <= 2)
	  error ("       (aren't you trying to mount an extended partition,\n"
		 "       instead of some logical partition inside?)");
	  close(fd);
	}
      }
      break;
    }
    case EMFILE:
      error ("mount table full"); break;
    case EIO:
      error ("mount: %s: can't read superblock", spec); break;
    case ENODEV:
      if (is_in_proc(type) || !strcmp(type, "guess"))
        error("mount: %s has wrong major or minor number", spec);
      else if (procfs) {
	char *lowtype, *p;
	int u;

	error ("mount: fs type %s not supported by kernel", type);

	/* maybe this loser asked for FAT or ISO9660 or isofs */
	lowtype = xstrdup(type);
	u = 0;
	for(p=lowtype; *p; p++) {
	  if(tolower(*p) != *p) {
	    *p = tolower(*p);
	    u++;
	  }
	}
	if (u && is_in_proc(lowtype))
	  error ("mount: probably you meant %s", lowtype);
	else if (!strncmp(lowtype, "iso", 3) && is_in_proc("iso9660"))
	  error ("mount: maybe you meant iso9660 ?");
	free(lowtype);
      } else
	error ("mount: %s has wrong device number or fs type %s not supported",
	       spec, type);
      break;
    case ENOTBLK:
      if (stat (spec, &statbuf)) /* strange ... */
	error ("mount: %s is not a block device, and stat fails?", spec);
      else if (S_ISBLK(statbuf.st_mode))
        error ("mount: the kernel does not recognize %s as a block device\n"
	       "       (maybe `insmod driver'?)", spec);
      else if (S_ISREG(statbuf.st_mode))
	error ("mount: %s is not a block device (maybe try `-o loop'?)",
		 spec);
      else
	error ("mount: %s is not a block device", spec);
      break;
    case ENXIO:
      error ("mount: %s is not a valid block device", spec); break;
    case EACCES:  /* pre-linux 1.1.38, 1.1.41 and later */
    case EROFS:   /* linux 1.1.38 and later */
      if (added_ro) {
          error ("mount: block device %s is not permitted on its filesystem",
		 spec);
          break;
      } else {
         added_ro = 1;
	 if (loop) {
	     opts = opts0;
	     type = type0;
	 }
         if (opts) {
             opts = realloc(xstrdup(opts), strlen(opts)+4);
             strcat(opts, ",ro");
         } else
             opts = "ro";
	 if (type && !strcmp(type, "guess"))
	     type = 0;
         error ("mount: %s%s is write-protected, mounting read-only",
		loop ? "" : "block device ", spec0);
	 return try_mount_one (spec0, node0, type, opts, freq, pass, bg);
      }
      break;
    default:
      error ("mount: %s", strerror (mnt_err)); break;
    }
  return EX_FAIL;
}

/*
 * set_proc_name()
 *	Update the argument vector, so that this process may be easily
 *	identified in a "ps" listing.
 */
static void
set_proc_name (char *spec)
{
#ifdef DO_PS_FIDDLING
  int i, l;

  /*
   * Move the environment so we can reuse the memory.
   * (Code borrowed from sendmail.)
   * WARNING: ugly assumptions on memory layout here; if this ever causes
   *          problems, #undef DO_PS_FIDDLING
   */
  for (i = 0; envp0[i] != NULL; i++)
    continue;
  environ = (char **) xmalloc(sizeof(char *) * (i + 1));
  for (i = 0; envp0[i] != NULL; i++)
    environ[i] = xstrdup(envp0[i]);
  environ[i] = NULL;

  if (i > 0)
    l = envp0[i-1] + strlen(envp0[i-1]) - argv0[0];
  else
    l = argv0[argc0-1] + strlen(argv0[argc0-1]) - argv0[0];
  if (l > sizeof(PROC_NAME)) {
    strcpy(argv0[0], PROC_NAME);
    strncpy(argv0[0] + sizeof(PROC_NAME) - 1, spec, l - sizeof(PROC_NAME) - 1);
    argv0[1] = NULL;
  }
#endif
}

static int
mount_one (char *spec, char *node, char *type, char *opts, char *cmdlineopts,
	   int freq, int pass)
{
  int status;
  int status2;

  /* Merge the fstab and command line options.  */
  if (opts == NULL)
       opts = cmdlineopts;
  else if (cmdlineopts != NULL)
       opts = xstrconcat3(opts, ",", cmdlineopts);

  if (type == NULL) {
      if (strchr (spec, ':') != NULL) {
	type = "nfs";
	if (verbose)
	  printf("mount: no type was given - "
		 "I'll assume nfs because of the colon\n");
      }
  }

  /*
   * Try to mount the file system. When the exit status is EX_BG,
   * we will retry in the background. Otherwise, we're done.
   */
  status = try_mount_one (spec, node, type, opts, freq, pass, 0);
  if (status != EX_BG)
    return status;

  /*
   * Retry in the background.
   */
  printf ("mount: backgrounding \"%s\"\n", spec);
  fflush( stdout );		/* prevent duplicate output */
#ifdef EMBED
  printf("mount: not forking...\n");
#else
  if (fork() > 0)
    return 0;			/* parent returns "success" */
#endif
  spec = xstrdup(spec);		/* arguments will be destroyed */
  node = xstrdup(node);		/* by set_proc_name()          */
  type = xstrdup(type);
  opts = xstrdup(opts);
  set_proc_name (spec);		/* make a nice "ps" listing */
  status2 = try_mount_one (spec, node, type, opts, freq, pass, 1);
  if (verbose && status2)
    printf ("mount: giving up \"%s\"\n", spec);
  exit (0);			/* child stops here */
}

/* Check if an fsname/dir pair was already in the old mtab.  */
static int
mounted (char *spec, char *node) {
     struct mntentchn *mc;

     spec = canonicalize (spec);
     node = canonicalize (node);

     for (mc = mtab_head()->nxt; mc; mc = mc->nxt)
	  if (streq (spec, mc->mnt_fsname) && streq (node, mc->mnt_dir))
	       return 1;
     return 0;
}

/* Mount all filesystems of the specified types except swap and root.  */
/* With the --fork option: fork and let different incarnations of
   mount handle different filesystems.  However, try to avoid several
   simultaneous mounts on the same physical disk, since that is very slow. */
#define DISKMAJOR(m)	(((int) m) & ~0xf)

static int
mount_all (string_list types, char *options) {
     struct mntentchn *mc, *mtmp;
     int status = 0;
     struct stat statbuf;
     struct child {
	  pid_t pid;
	  char *group;
	  struct mntentchn *mec;
	  struct mntentchn *meclast;
	  struct child *nxt;
     } childhead, *childtail, *cp;
     char major[22];
     char *g, *colon;

     /* build a chain of what we have to do, or maybe
	several chains, one for each major or NFS host */
     childhead.nxt = 0;
     childtail = &childhead;
     for (mc = fstab_head()->nxt; mc; mc = mc->nxt) {
	  if (matching_type (mc->mnt_type, types)
	      && !streq (mc->mnt_dir, "/")
	      && !streq (mc->mnt_dir, "root")) {
	       if (mounted (mc->mnt_fsname, mc->mnt_dir)) {
		    if (verbose)
			 printf("mount: %s already mounted on %s\n",
				mc->mnt_fsname, mc->mnt_dir);
	       } else {
		    mtmp = (struct mntentchn *) xmalloc(sizeof(*mtmp));
		    *mtmp = *mc;
		    mtmp->nxt = 0;
		    g = NULL;
		    if (optfork) {
			 if (stat(mc->mnt_fsname, &statbuf) == 0 &&
			     S_ISBLK(statbuf.st_mode)) {
			      sprintf(major, "#%x", DISKMAJOR(statbuf.st_rdev));
			      g = major;
			 }
#ifdef HAVE_NFS
			 if (strcmp(mc->mnt_type, "nfs") == 0) {
			      g = xstrdup(mc->mnt_fsname);
			      colon = strchr(g, ':');
			      if (colon)
				   *colon = '\0';
			 }
#endif
		    }
		    if (g) {
			 for (cp = childhead.nxt; cp; cp = cp->nxt)
			      if (cp->group && strcmp(cp->group, g) == 0) {
				   cp->meclast->nxt = mtmp;
				   cp->meclast = mtmp;
				   goto fnd;
			      }
		    }
		    cp = (struct child *) xmalloc(sizeof *cp);
		    cp->nxt = 0;
		    cp->mec = cp->meclast = mtmp;
		    cp->group = xstrdup(g);
		    cp->pid = 0;
		    childtail->nxt = cp;
		    childtail = cp;
	       fnd:;
	       }
	  }
     }
			      
     /* now do everything */
     for (cp = childhead.nxt; cp; cp = cp->nxt) {
	  pid_t p = -1;
	  if (optfork) {
#ifndef EMBED
	       p = fork();
#endif
	       if (p == -1)
		    error("mount: cannot fork: %s", strerror (errno));
	       else if (p != 0)
		    cp->pid = p;
	  }

	  /* if child, or not forked, do the mounting */
	  if (p == 0 || p == -1) {
	       for (mc = cp->mec; mc; mc = mc->nxt)
		    status |= mount_one (mc->mnt_fsname, mc->mnt_dir,
					 mc->mnt_type, mc->mnt_opts,
					 options, 0, 0);
	       if (mountcount)
		    status |= EX_SOMEOK;
	       if (p == 0)
		    exit(status);
	  }
     }

     /* wait for children, if any */
     while ((cp = childhead.nxt) != NULL) {
	  childhead.nxt = cp->nxt;
	  if (cp->pid) {
	       int ret;
	  keep_waiting:
	       if(waitpid(cp->pid, &ret, 0) == -1) {
		    if (errno == EINTR)
			 goto keep_waiting;
		    perror("waitpid");
	       } else if (WIFEXITED(ret))
		    status |= WEXITSTATUS(ret);
	       else
		    status |= EX_SYSERR;
	  }
     }
     if (mountcount)
	  status |= EX_SOMEOK;
     return status;
}

extern char version[];
#ifdef EMBED
#define	getopt_long(a,b,c,d,e)	getopt(a,b,c)
#else
static struct option longopts[] =
{
  { "all", 0, 0, 'a' },
  { "fake", 0, 0, 'f' },
  { "fork", 0, 0, 'F' },
  { "help", 0, 0, 'h' },
  { "no-mtab", 0, 0, 'n' },
  { "read-only", 0, 0, 'r' },
  { "ro", 0, 0, 'r' },
  { "verbose", 0, 0, 'v' },
  { "version", 0, 0, 'V' },
  { "read-write", 0, 0, 'w' },
  { "rw", 0, 0, 'w' },
  { "options", 1, 0, 'o' },
  { "types", 1, 0, 't' },
  { NULL, 0, 0, 0 }
};
#endif

const char *usage_string = "\
usage: mount [-hV]\n\
       mount -a [-nfFrsvw] [-t vfstypes]\n\
       mount [-nfrsvw] [-o options] special | node\n\
       mount [-nfrsvw] [-t vfstype] [-o options] special node\n\
";

static void
usage (FILE *fp, int n)
{
  fprintf (fp, "%s", usage_string);
  unlock_mtab();
  exit (n);
}

int
main (int argc, char *argv[]) {
  int c, result = 0;
  char *options = NULL, *spec;
  string_list types = NULL;
  struct mntentchn *mc;
  int fd;

  /* People report that a mount called from init without console
     writes error messages to /etc/mtab
     Let us try to avoid getting fd's 0,1,2 */
  while((fd = open("/dev/null", O_RDWR)) == 0 || fd == 1 || fd == 2) ;
  if (fd > 2)
     close(fd);

#ifdef DO_PS_FIDDLING
  argc0 = argc;
  argv0 = argv;
  envp0 = environ;
#endif

  while ((c = getopt_long (argc, argv, "afFhno:rsvVwt:", longopts, NULL))
	 != EOF)
    switch (c) {
      case 'a':			/* mount everything in fstab */
	++all;
	break;
      case 'f':			/* fake (don't actually do mount(2) call) */
	++fake;
	break;
      case 'F':
	++optfork;
	break;
      case 'h':			/* help */
	usage (stdout, 0);
	break;
      case 'n':			/* mount without writing in /etc/mtab */
	++nomtab;
	break;
      case 'o':			/* specify mount options */
	if (options)
	     options = xstrconcat3(options, ",", optarg);
	else
	     options = xstrdup(optarg);
	break;
      case 'r':			/* mount readonly */
	readonly = 1;
	readwrite = 0;
	break;
      case 's':			/* allow sloppy mount options */
	sloppy = 1;
	break;
      case 't':			/* specify file system types */
	types = parse_list (optarg);
	break;
      case 'v':			/* be chatty - very chatty if repeated */
	++verbose;
	break;
      case 'V':			/* version */
	printf ("mount: %s\n", version);
	exit (0);
      case 'w':			/* mount read/write */
	readwrite = 1;
	readonly = 0;
	break;
      case 0:
	break;
      case '?':
      default:
	usage (stderr, EX_USAGE);
    }

  argc -= optind;
  argv += optind;

  if (argc == 0 && !all) {
      if (options)
	usage (stderr, EX_USAGE);
      return print_all (types);
  }

  if (getuid () != geteuid ()) {
      suid = 1;
      if (types || options || readwrite || nomtab || all || fake || argc != 1)
	die (EX_USAGE, "mount: only root can do that");
  }

  if (!nomtab && mtab_does_not_exist()) {
       if (verbose > 1)
	    printf("mount: no %s found - creating it..\n", MOUNTED);
       create_mtab ();
  }

  switch (argc) {
    case 0:
      /* mount -a */
      result = mount_all (types, options);
      if (result == 0 && verbose)
	   error("not mounted anything");
      break;

    case 1:
      /* mount [-nfrvw] [-o options] special | node */
      if (types != NULL)
	usage (stderr, EX_USAGE);

      /* Try to find the other pathname in fstab.  */
      spec = canonicalize (*argv);
      if ((mc = getmntfile (spec)) == NULL &&
	  (mc = getfsspec (spec)) == NULL &&
	  (mc = getfsfile (spec)) == NULL &&
	     /* Try noncanonical name in fstab
	        perhaps /dev/cdrom or /dos is a symlink */
	  (mc = getfsspec (*argv)) == NULL &&
	  (mc = getfsfile (*argv)) == NULL)
	   die (EX_USAGE, "mount: can't find %s in %s or %s",
		spec, MOUNTED, _PATH_FSTAB);

      result = mount_one (xstrdup (mc->mnt_fsname), xstrdup (mc->mnt_dir),
			  xstrdup (mc->mnt_type), mc->mnt_opts, options, 0, 0);
      break;

    case 2:
      /* mount [-nfrvw] [-t vfstype] [-o options] special node */
      if (types == NULL)
	result = mount_one (argv[0], argv[1],
			    NULL, NULL, options, 0, 0);
      else if (cdr (types) == NULL)
	result = mount_one (argv[0], argv[1],
			    car (types), NULL, options, 0, 0);
      else
	usage (stderr, EX_USAGE);
      break;
      
    default:
      usage (stderr, EX_USAGE);
  }

  if (result == EX_SOMEOK)
       result = 0;
  exit (result);
}

#ifdef EMBED

int getdtablesize()
{
	return(64);
}

#endif
