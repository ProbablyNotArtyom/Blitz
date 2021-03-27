/* vi: set sw=4 ts=4: */
/*
 * nfsmount.c -- Linux NFS mount
 * Copyright (C) 1993 Rick Sladkey <jrs@world.std.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Wed Feb  8 12:51:48 1995, biro@yggdrasil.com (Ross Biro): allow all port
 * numbers to be specified on the command line.
 *
 * Fri, 8 Mar 1996 18:01:39, Swen Thuemmler <swen@uni-paderborn.de>:
 * Omit the call to connect() for Linux version 1.3.11 or later.
 *
 * Wed Oct  1 23:55:28 1997: Dick Streefland <dick_streefland@tasking.com>
 * Implemented the "bg", "fg" and "retry" mount options for NFS.
 *
 * 1999-02-22 Arkadiusz Mi�kiewicz <misiek@misiek.eu.org>
 * - added Native Language Support
 * 
 * Modified by Olaf Kirch and Trond Myklebust for new NFS code,
 * plus NFSv3 stuff.
 */

/*
 * nfsmount.c,v 1.1.1.1 1993/11/18 08:40:51 jrs Exp
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "busybox.h"
#undef TRUE
#undef FALSE
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#include <linux/nfs.h>  /* For the kernels nfs stuff */
#include "nfsmount.h"

#ifndef NFS_FHSIZE
static const int NFS_FHSIZE = 32;
#endif
#ifndef NFS_PORT
static const int NFS_PORT = 2049;
#endif

/* Disable the nls stuff */
# undef bindtextdomain
# define bindtextdomain(Domain, Directory) /* empty */
# undef textdomain
# define textdomain(Domain) /* empty */
# define _(Text) (Text)
# define N_(Text) (Text)

static const int MS_MGC_VAL = 0xc0ed0000; /* Magic number indicatng "new" flags */
static const int MS_RDONLY = 1;      /* Mount read-only */
static const int MS_NOSUID = 2;      /* Ignore suid and sgid bits */
static const int MS_NODEV = 4;      /* Disallow access to device special files */
static const int MS_NOEXEC = 8;      /* Disallow program execution */
static const int MS_SYNCHRONOUS = 16;      /* Writes are synced at once */
static const int MS_REMOUNT = 32;      /* Alter flags of a mounted FS */
static const int MS_MANDLOCK = 64;      /* Allow mandatory locks on an FS */
static const int S_QUOTA = 128;     /* Quota initialized for file/directory/symlink */
static const int S_APPEND = 256;     /* Append-only file */
static const int S_IMMUTABLE = 512;     /* Immutable file */
static const int MS_NOATIME = 1024;    /* Do not update access times. */
static const int MS_NODIRATIME = 2048;    /* Do not update directory access times */


/*
 * We want to be able to compile mount on old kernels in such a way
 * that the binary will work well on more recent kernels.
 * Thus, if necessary we teach nfsmount.c the structure of new fields
 * that will come later.
 *
 * Moreover, the new kernel includes conflict with glibc includes
 * so it is easiest to ignore the kernel altogether (at compile time).
 */

/* NOTE: Do not make this into a 'static const int' because the pre-processor
 * needs to test this value in some #if statements. */
#define NFS_MOUNT_VERSION 4

struct nfs2_fh {
        char                    data[32];
};
struct nfs3_fh {
        unsigned short          size;
        unsigned char           data[64];
};

struct nfs_mount_data {
	int		version;		/* 1 */
	int		fd;			/* 1 */
	struct nfs2_fh	old_root;		/* 1 */
	int		flags;			/* 1 */
	int		rsize;			/* 1 */
	int		wsize;			/* 1 */
	int		timeo;			/* 1 */
	int		retrans;		/* 1 */
	int		acregmin;		/* 1 */
	int		acregmax;		/* 1 */
	int		acdirmin;		/* 1 */
	int		acdirmax;		/* 1 */
	struct sockaddr_in addr;		/* 1 */
	char		hostname[256];		/* 1 */
	int		namlen;			/* 2 */
	unsigned int	bsize;			/* 3 */
	struct nfs3_fh	root;			/* 4 */
};

/* bits in the flags field */

static const int NFS_MOUNT_SOFT = 0x0001;	/* 1 */
static const int NFS_MOUNT_INTR = 0x0002;	/* 1 */
static const int NFS_MOUNT_SECURE = 0x0004;	/* 1 */
static const int NFS_MOUNT_POSIX = 0x0008;	/* 1 */
static const int NFS_MOUNT_NOCTO = 0x0010;	/* 1 */
static const int NFS_MOUNT_NOAC = 0x0020;	/* 1 */
static const int NFS_MOUNT_TCP = 0x0040;	/* 2 */
static const int NFS_MOUNT_VER3 = 0x0080;	/* 3 */
static const int NFS_MOUNT_KERBEROS = 0x0100;	/* 3 */
static const int NFS_MOUNT_NONLM = 0x0200;	/* 3 */


#define UTIL_LINUX_VERSION "2.10m"
#define util_linux_version "util-linux-2.10m"

#define HAVE_inet_aton
#define HAVE_scsi_h
#define HAVE_blkpg_h
#define HAVE_kd_h
#define HAVE_termcap
#define HAVE_locale_h
#define HAVE_libintl_h
#define ENABLE_NLS
#define HAVE_langinfo_h
#define HAVE_progname
#define HAVE_openpty
#define HAVE_nanosleep
#define HAVE_personality
#define HAVE_tm_gmtoff

static char *nfs_strerror(int status);

#define MAKE_VERSION(p,q,r)	(65536*(p) + 256*(q) + (r))
#define MAX_NFSPROT ((nfs_mount_version >= 4) ? 3 : 2)

static const int EX_FAIL = 32;       /* mount failure */
static const int EX_BG = 256;       /* retry in background (internal only) */


/*
 * nfs_mount_version according to the sources seen at compile time.
 */
static int nfs_mount_version;

/*
 * Unfortunately, the kernel prints annoying console messages
 * in case of an unexpected nfs mount version (instead of
 * just returning some error).  Therefore we'll have to try
 * and figure out what version the kernel expects.
 *
 * Variables:
 *	KERNEL_NFS_MOUNT_VERSION: kernel sources at compile time
 *	NFS_MOUNT_VERSION: these nfsmount sources at compile time
 *	nfs_mount_version: version this source and running kernel can handle
 */
static void
find_kernel_nfs_mount_version(void)
{
	static int kernel_version = 0;

	if (kernel_version)
		return;

	nfs_mount_version = NFS_MOUNT_VERSION; /* default */

	kernel_version = get_kernel_revision();
	if (kernel_version) {
		if (kernel_version < MAKE_VERSION(2,1,32))
			nfs_mount_version = 1;
		else if (kernel_version < MAKE_VERSION(2,2,18) ||
				(kernel_version >=   MAKE_VERSION(2,3,0) &&
				 kernel_version < MAKE_VERSION(2,3,99)))
			nfs_mount_version = 3;
		else
			nfs_mount_version = 4; /* since 2.3.99pre4 */
	}
	if (nfs_mount_version > NFS_MOUNT_VERSION)
		nfs_mount_version = NFS_MOUNT_VERSION;
}

static struct pmap *
get_mountport(struct sockaddr_in *server_addr,
      long unsigned prog,
      long unsigned version,
      long unsigned proto,
      long unsigned port)
{
struct pmaplist *pmap;
static struct pmap p = {0, 0, 0, 0};

server_addr->sin_port = PMAPPORT;
pmap = pmap_getmaps(server_addr);

if (version > MAX_NFSPROT)
	version = MAX_NFSPROT;
if (!prog)
	prog = MOUNTPROG;
p.pm_prog = prog;
p.pm_vers = version;
p.pm_prot = proto;
p.pm_port = port;

while (pmap) {
	if (pmap->pml_map.pm_prog != prog)
		goto next;
	if (!version && p.pm_vers > pmap->pml_map.pm_vers)
		goto next;
	if (version > 2 && pmap->pml_map.pm_vers != version)
		goto next;
	if (version && version <= 2 && pmap->pml_map.pm_vers > 2)
		goto next;
	if (pmap->pml_map.pm_vers > MAX_NFSPROT ||
	    (proto && p.pm_prot && pmap->pml_map.pm_prot != proto) ||
	    (port && pmap->pml_map.pm_port != port))
		goto next;
	memcpy(&p, &pmap->pml_map, sizeof(p));
next:
	pmap = pmap->pml_next;
}
if (!p.pm_vers)
	p.pm_vers = MOUNTVERS;
if (!p.pm_port)
	p.pm_port = MOUNTPORT;
if (!p.pm_prot)
	p.pm_prot = IPPROTO_TCP;
return &p;
}

int nfsmount(const char *spec, const char *node, int *flags,
	     char **extra_opts, char **mount_opts, int running_bg)
{
	static char *prev_bg_host;
	char hostdir[1024];
	CLIENT *mclient;
	char *hostname;
	char *pathname;
	char *old_opts;
	char *mounthost=NULL;
	char new_opts[1024];
	struct timeval total_timeout;
	enum clnt_stat clnt_stat;
	static struct nfs_mount_data data;
	char *opt, *opteq;
	int val;
	struct hostent *hp;
	struct sockaddr_in server_addr;
	struct sockaddr_in mount_server_addr;
	struct pmap* pm_mnt;
	int msock, fsock;
	struct timeval retry_timeout;
	union {
		struct fhstatus nfsv2;
		struct mountres3 nfsv3;
	} status;
	struct stat statbuf;
	char *s;
	int port;
	int mountport;
	int proto;
	int bg;
	int soft;
	int intr;
	int posix;
	int nocto;
	int noac;
	int nolock;
	int retry;
	int tcp;
	int mountprog;
	int mountvers;
	int nfsprog;
	int nfsvers;
	int retval;
	time_t t;
	time_t prevt;
	time_t timeout;

	find_kernel_nfs_mount_version();

	retval = EX_FAIL;
	msock = fsock = -1;
	mclient = NULL;
	if (strlen(spec) >= sizeof(hostdir)) {
		error_msg("excessively long host:dir argument");
		goto fail;
	}
	strcpy(hostdir, spec);
	if ((s = strchr(hostdir, ':'))) {
		hostname = hostdir;
		pathname = s + 1;
		*s = '\0';
		/* Ignore all but first hostname in replicated mounts
		   until they can be fully supported. (mack@sgi.com) */
		if ((s = strchr(hostdir, ','))) {
			*s = '\0';
			error_msg("warning: multiple hostnames not supported");
		}
	} else {
		error_msg("directory to mount not in host:dir format");
		goto fail;
	}

	server_addr.sin_family = AF_INET;
#ifdef HAVE_inet_aton
	if (!inet_aton(hostname, &server_addr.sin_addr))
#endif
	{
		if ((hp = gethostbyname(hostname)) == NULL) {
			herror_msg("%s", hostname);
			goto fail;
		} else {
			if (hp->h_length > sizeof(struct in_addr)) {
				error_msg("got bad hp->h_length");
				hp->h_length = sizeof(struct in_addr);
			}
			memcpy(&server_addr.sin_addr,
			       hp->h_addr, hp->h_length);
		}
	}

	memcpy (&mount_server_addr, &server_addr, sizeof (mount_server_addr));

	/* add IP address to mtab options for use when unmounting */

	s = inet_ntoa(server_addr.sin_addr);
	old_opts = *extra_opts;
	if (!old_opts)
		old_opts = "";
	if (strlen(old_opts) + strlen(s) + 10 >= sizeof(new_opts)) {
		error_msg("excessively long option argument");
		goto fail;
	}
	sprintf(new_opts, "%s%saddr=%s",
		old_opts, *old_opts ? "," : "", s);
	*extra_opts = xstrdup(new_opts);

	/* Set default options.
	 * rsize/wsize (and bsize, for ver >= 3) are left 0 in order to
	 * let the kernel decide.
	 * timeo is filled in after we know whether it'll be TCP or UDP. */
	memset(&data, 0, sizeof(data));
	data.retrans	= 3;
	data.acregmin	= 3;
	data.acregmax	= 60;
	data.acdirmin	= 30;
	data.acdirmax	= 60;
#if NFS_MOUNT_VERSION >= 2
	data.namlen	= NAME_MAX;
#endif

	bg = 0;
	soft = 0;
	intr = 0;
	posix = 0;
	nocto = 0;
	nolock = 0;
	noac = 0;
	retry = 10000;		/* 10000 minutes ~ 1 week */
	tcp = 0;

	mountprog = MOUNTPROG;
	mountvers = 0;
	port = 0;
	mountport = 0;
	nfsprog = NFS_PROGRAM;
	nfsvers = 0;

	/* parse options */

	for (opt = strtok(old_opts, ","); opt; opt = strtok(NULL, ",")) {
		if ((opteq = strchr(opt, '='))) {
			val = atoi(opteq + 1);	
			*opteq = '\0';
			if (!strcmp(opt, "rsize"))
				data.rsize = val;
			else if (!strcmp(opt, "wsize"))
				data.wsize = val;
			else if (!strcmp(opt, "timeo"))
				data.timeo = val;
			else if (!strcmp(opt, "retrans"))
				data.retrans = val;
			else if (!strcmp(opt, "acregmin"))
				data.acregmin = val;
			else if (!strcmp(opt, "acregmax"))
				data.acregmax = val;
			else if (!strcmp(opt, "acdirmin"))
				data.acdirmin = val;
			else if (!strcmp(opt, "acdirmax"))
				data.acdirmax = val;
			else if (!strcmp(opt, "actimeo")) {
				data.acregmin = val;
				data.acregmax = val;
				data.acdirmin = val;
				data.acdirmax = val;
			}
			else if (!strcmp(opt, "retry"))
				retry = val;
			else if (!strcmp(opt, "port"))
				port = val;
			else if (!strcmp(opt, "mountport"))
			        mountport = val;
			else if (!strcmp(opt, "mounthost"))
			        mounthost=xstrndup(opteq+1,
						  strcspn(opteq+1," \t\n\r,"));
			else if (!strcmp(opt, "mountprog"))
				mountprog = val;
			else if (!strcmp(opt, "mountvers"))
				mountvers = val;
			else if (!strcmp(opt, "nfsprog"))
				nfsprog = val;
			else if (!strcmp(opt, "nfsvers") ||
				 !strcmp(opt, "vers"))
				nfsvers = val;
			else if (!strcmp(opt, "proto")) {
				if (!strncmp(opteq+1, "tcp", 3))
					tcp = 1;
				else if (!strncmp(opteq+1, "udp", 3))
					tcp = 0;
				else
					printf(_("Warning: Unrecognized proto= option.\n"));
			} else if (!strcmp(opt, "namlen")) {
#if NFS_MOUNT_VERSION >= 2
				if (nfs_mount_version >= 2)
					data.namlen = val;
				else
#endif
				printf(_("Warning: Option namlen is not supported.\n"));
			} else if (!strcmp(opt, "addr"))
				/* ignore */;
			else {
				printf(_("unknown nfs mount parameter: "
				       "%s=%d\n"), opt, val);
				goto fail;
			}
		}
		else {
			val = 1;
			if (!strncmp(opt, "no", 2)) {
				val = 0;
				opt += 2;
			}
			if (!strcmp(opt, "bg")) 
				bg = val;
			else if (!strcmp(opt, "fg")) 
				bg = !val;
			else if (!strcmp(opt, "soft"))
				soft = val;
			else if (!strcmp(opt, "hard"))
				soft = !val;
			else if (!strcmp(opt, "intr"))
				intr = val;
			else if (!strcmp(opt, "posix"))
				posix = val;
			else if (!strcmp(opt, "cto"))
				nocto = !val;
			else if (!strcmp(opt, "ac"))
				noac = !val;
			else if (!strcmp(opt, "tcp"))
				tcp = val;
			else if (!strcmp(opt, "udp"))
				tcp = !val;
			else if (!strcmp(opt, "lock")) {
				if (nfs_mount_version >= 3)
					nolock = !val;
				else
					printf(_("Warning: option nolock is not supported.\n"));
			} else {
				printf(_("unknown nfs mount option: "
					   "%s%s\n"), val ? "" : "no", opt);
				goto fail;
			}
		}
	}
	proto = (tcp) ? IPPROTO_TCP : IPPROTO_UDP;

	data.flags = (soft ? NFS_MOUNT_SOFT : 0)
		| (intr ? NFS_MOUNT_INTR : 0)
		| (posix ? NFS_MOUNT_POSIX : 0)
		| (nocto ? NFS_MOUNT_NOCTO : 0)
		| (noac ? NFS_MOUNT_NOAC : 0);
#if NFS_MOUNT_VERSION >= 2
	if (nfs_mount_version >= 2)
		data.flags |= (tcp ? NFS_MOUNT_TCP : 0);
#endif
#if NFS_MOUNT_VERSION >= 3
	if (nfs_mount_version >= 3)
		data.flags |= (nolock ? NFS_MOUNT_NONLM : 0);
#endif
	if (nfsvers > MAX_NFSPROT) {
		error_msg("NFSv%d not supported!", nfsvers);
		return 0;
	}
	if (mountvers > MAX_NFSPROT) {
		error_msg("NFSv%d not supported!", nfsvers);
		return 0;
	}
	if (nfsvers && !mountvers)
		mountvers = (nfsvers < 3) ? 1 : nfsvers;
	if (nfsvers && nfsvers < mountvers) {
		mountvers = nfsvers;
	}

	/* Adjust options if none specified */
	if (!data.timeo)
		data.timeo = tcp ? 70 : 7;

#ifdef NFS_MOUNT_DEBUG
	printf("rsize = %d, wsize = %d, timeo = %d, retrans = %d\n",
		data.rsize, data.wsize, data.timeo, data.retrans);
	printf("acreg (min, max) = (%d, %d), acdir (min, max) = (%d, %d)\n",
		data.acregmin, data.acregmax, data.acdirmin, data.acdirmax);
	printf("port = %d, bg = %d, retry = %d, flags = %.8x\n",
		port, bg, retry, data.flags);
	printf("mountprog = %d, mountvers = %d, nfsprog = %d, nfsvers = %d\n",
		mountprog, mountvers, nfsprog, nfsvers);
	printf("soft = %d, intr = %d, posix = %d, nocto = %d, noac = %d\n",
		(data.flags & NFS_MOUNT_SOFT) != 0,
		(data.flags & NFS_MOUNT_INTR) != 0,
		(data.flags & NFS_MOUNT_POSIX) != 0,
		(data.flags & NFS_MOUNT_NOCTO) != 0,
		(data.flags & NFS_MOUNT_NOAC) != 0);
#if NFS_MOUNT_VERSION >= 2
	printf("tcp = %d\n",
		(data.flags & NFS_MOUNT_TCP) != 0);
#endif
#endif

	data.version = nfs_mount_version;
	*mount_opts = (char *) &data;

	if (*flags & MS_REMOUNT)
		return 0;

	/*
	 * If the previous mount operation on the same host was
	 * backgrounded, and the "bg" for this mount is also set,
	 * give up immediately, to avoid the initial timeout.
	 */
	if (bg && !running_bg &&
	    prev_bg_host && strcmp(hostname, prev_bg_host) == 0) {
		if (retry > 0)
			retval = EX_BG;
		return retval;
	}

	/* create mount deamon client */
	/* See if the nfs host = mount host. */
	if (mounthost) {
	  if (mounthost[0] >= '0' && mounthost[0] <= '9') {
	    mount_server_addr.sin_family = AF_INET;
	    mount_server_addr.sin_addr.s_addr = inet_addr(hostname);
	  } else {
		  if ((hp = gethostbyname(mounthost)) == NULL) {
			  herror_msg("%s", mounthost);
			  goto fail;
		  } else {
			  if (hp->h_length > sizeof(struct in_addr)) {
				  error_msg("got bad hp->h_length?");
				  hp->h_length = sizeof(struct in_addr);
			  }
			  mount_server_addr.sin_family = AF_INET;
			  memcpy(&mount_server_addr.sin_addr,
				 hp->h_addr, hp->h_length);
		  }
	  }
	}

	/*
	 * The following loop implements the mount retries. On the first
	 * call, "running_bg" is 0. When the mount times out, and the
	 * "bg" option is set, the exit status EX_BG will be returned.
	 * For a backgrounded mount, there will be a second call by the
	 * child process with "running_bg" set to 1.
	 *
	 * The case where the mount point is not present and the "bg"
	 * option is set, is treated as a timeout. This is done to
	 * support nested mounts.
	 *
	 * The "retry" count specified by the user is the number of
	 * minutes to retry before giving up.
	 *
	 * Only the first error message will be displayed.
	 */
	retry_timeout.tv_sec = 3;
	retry_timeout.tv_usec = 0;
	total_timeout.tv_sec = 20;
	total_timeout.tv_usec = 0;
	timeout = time(NULL) + 60 * retry;
	prevt = 0;
	t = 30;
	val = 1;
	for (;;) {
		if (bg && stat(node, &statbuf) == -1) {
			if (running_bg) {
				sleep(val);	/* 1, 2, 4, 8, 16, 30, ... */
				val *= 2;
				if (val > 30)
					val = 30;
			}
		} else {
			/* be careful not to use too many CPU cycles */
			if (t - prevt < 30)
				sleep(30);

			pm_mnt = get_mountport(&mount_server_addr,
				       mountprog,
				       mountvers,
				       proto,
 				       mountport);

			/* contact the mount daemon via TCP */
			mount_server_addr.sin_port = htons(pm_mnt->pm_port);
			msock = RPC_ANYSOCK;

			switch (pm_mnt->pm_prot) {
			case IPPROTO_UDP:
				mclient = clntudp_create(&mount_server_addr,
						 pm_mnt->pm_prog,
						 pm_mnt->pm_vers,
						 retry_timeout,
						 &msock);
		  if (mclient)
			  break;
		  mount_server_addr.sin_port = htons(pm_mnt->pm_port);
		  msock = RPC_ANYSOCK;
		case IPPROTO_TCP:
			mclient = clnttcp_create(&mount_server_addr,
						 pm_mnt->pm_prog,
						 pm_mnt->pm_vers,
						 &msock, 0, 0);
			break;
		default:
			mclient = 0;
			}
			if (mclient) {
				/* try to mount hostname:pathname */
				mclient->cl_auth = authunix_create_default();

			/* make pointers in xdr_mountres3 NULL so
			 * that xdr_array allocates memory for us
			 */
			memset(&status, 0, sizeof(status));

			if (pm_mnt->pm_vers == 3)
				clnt_stat = clnt_call(mclient, MOUNTPROC3_MNT,
						      (xdrproc_t) xdr_dirpath,
						      (caddr_t) &pathname,
						      (xdrproc_t) xdr_mountres3,
						      (caddr_t) &status,
					total_timeout);
			else
				clnt_stat = clnt_call(mclient, MOUNTPROC_MNT,
						      (xdrproc_t) xdr_dirpath,
						      (caddr_t) &pathname,
						      (xdrproc_t) xdr_fhstatus,
						      (caddr_t) &status,
						      total_timeout);

				if (clnt_stat == RPC_SUCCESS)
					break;		/* we're done */
				if (errno != ECONNREFUSED) {
					clnt_perror(mclient, "mount");
					goto fail;	/* don't retry */
				}
				if (!running_bg && prevt == 0)
					clnt_perror(mclient, "mount");
				auth_destroy(mclient->cl_auth);
				clnt_destroy(mclient);
				mclient = 0;
				close(msock);
			} else {
				if (!running_bg && prevt == 0)
					clnt_pcreateerror("mount");
			}
			prevt = t;
		}
		if (!bg)
		        goto fail;
		if (!running_bg) {
			prev_bg_host = xstrdup(hostname);
			if (retry > 0)
				retval = EX_BG;
			goto fail;
		}
		t = time(NULL);
		if (t >= timeout)
			goto fail;
	}
	nfsvers = (pm_mnt->pm_vers < 2) ? 2 : pm_mnt->pm_vers;

	if (nfsvers == 2) {
		if (status.nfsv2.fhs_status != 0) {
			error_msg("%s:%s failed, reason given by server: %s",
				hostname, pathname,
				nfs_strerror(status.nfsv2.fhs_status));
			goto fail;
		}
		memcpy(data.root.data,
		       (char *) status.nfsv2.fhstatus_u.fhs_fhandle,
		       NFS_FHSIZE);
#if NFS_MOUNT_VERSION >= 4
		data.root.size = NFS_FHSIZE;
		memcpy(data.old_root.data,
		       (char *) status.nfsv2.fhstatus_u.fhs_fhandle,
		       NFS_FHSIZE);
#endif
	} else {
#if NFS_MOUNT_VERSION >= 4
		fhandle3 *my_fhandle;
		if (status.nfsv3.fhs_status != 0) {
			error_msg("%s:%s failed, reason given by server: %s",
				hostname, pathname,
				nfs_strerror(status.nfsv3.fhs_status));
			goto fail;
		}
		my_fhandle = &status.nfsv3.mountres3_u.mountinfo.fhandle;
		memset(data.old_root.data, 0, NFS_FHSIZE);
		memset(&data.root, 0, sizeof(data.root));
		data.root.size = my_fhandle->fhandle3_len;
		memcpy(data.root.data,
		       (char *) my_fhandle->fhandle3_val,
		       my_fhandle->fhandle3_len);

		data.flags |= NFS_MOUNT_VER3;
#endif
	}

	/* create nfs socket for kernel */

	if (tcp) {
		if (nfs_mount_version < 3) {
	     		printf(_("NFS over TCP is not supported.\n"));
			goto fail;
		}
		fsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	} else
		fsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fsock < 0) {
		perror(_("nfs socket"));
		goto fail;
	}
	if (bindresvport(fsock, 0) < 0) {
		perror(_("nfs bindresvport"));
		goto fail;
	}
	if (port == 0) {
		server_addr.sin_port = PMAPPORT;
		port = pmap_getport(&server_addr, nfsprog, nfsvers,
			tcp ? IPPROTO_TCP : IPPROTO_UDP);
		if (port == 0)
			port = NFS_PORT;
#ifdef NFS_MOUNT_DEBUG
		else
			printf(_("used portmapper to find NFS port\n"));
#endif
	}
#ifdef NFS_MOUNT_DEBUG
	printf(_("using port %d for nfs deamon\n"), port);
#endif
	server_addr.sin_port = htons(port);
	 /*
	  * connect() the socket for kernels 1.3.10 and below only,
	  * to avoid problems with multihomed hosts.
	  * --Swen
	  */
	if (get_kernel_revision() <= 66314
	    && connect(fsock, (struct sockaddr *) &server_addr,
		       sizeof (server_addr)) < 0) {
		perror(_("nfs connect"));
		goto fail;
	}

	/* prepare data structure for kernel */

	data.fd = fsock;
	memcpy((char *) &data.addr, (char *) &server_addr, sizeof(data.addr));
	strncpy(data.hostname, hostname, sizeof(data.hostname));

	/* clean up */

	auth_destroy(mclient->cl_auth);
	clnt_destroy(mclient);
	close(msock);
	return 0;

	/* abort */

fail:
	if (msock != -1) {
		if (mclient) {
			auth_destroy(mclient->cl_auth);
			clnt_destroy(mclient);
		}
		close(msock);
	}
	if (fsock != -1)
		close(fsock);
	return retval;
}	

/*
 * We need to translate between nfs status return values and
 * the local errno values which may not be the same.
 *
 * Andreas Schwab <schwab@LS5.informatik.uni-dortmund.de>: change errno:
 * "after #include <errno.h> the symbol errno is reserved for any use,
 *  it cannot even be used as a struct tag or field name".
 */

#ifndef EDQUOT
#define EDQUOT	ENOSPC
#endif

static struct {
	enum nfs_stat stat;
	int errnum;
} nfs_errtbl[] = {
	{ NFS_OK,		0		},
	{ NFSERR_PERM,		EPERM		},
	{ NFSERR_NOENT,		ENOENT		},
	{ NFSERR_IO,		EIO		},
	{ NFSERR_NXIO,		ENXIO		},
	{ NFSERR_ACCES,		EACCES		},
	{ NFSERR_EXIST,		EEXIST		},
	{ NFSERR_NODEV,		ENODEV		},
	{ NFSERR_NOTDIR,	ENOTDIR		},
	{ NFSERR_ISDIR,		EISDIR		},
#ifdef NFSERR_INVAL
	{ NFSERR_INVAL,		EINVAL		},	/* that Sun forgot */
#endif
	{ NFSERR_FBIG,		EFBIG		},
	{ NFSERR_NOSPC,		ENOSPC		},
	{ NFSERR_ROFS,		EROFS		},
	{ NFSERR_NAMETOOLONG,	ENAMETOOLONG	},
	{ NFSERR_NOTEMPTY,	ENOTEMPTY	},
	{ NFSERR_DQUOT,		EDQUOT		},
	{ NFSERR_STALE,		ESTALE		},
#ifdef EWFLUSH
	{ NFSERR_WFLUSH,	EWFLUSH		},
#endif
	/* Throw in some NFSv3 values for even more fun (HP returns these) */
	{ 71,			EREMOTE		},

	{ -1,			EIO		}
};

static char *nfs_strerror(int status)
{
	int i;
	static char buf[256];

	for (i = 0; nfs_errtbl[i].stat != -1; i++) {
		if (nfs_errtbl[i].stat == status)
			return strerror(nfs_errtbl[i].errnum);
	}
	sprintf(buf, _("unknown nfs status return value: %d"), status);
	return buf;
}

static bool_t
xdr_fhandle (XDR *xdrs, fhandle objp)
{
	//register int32_t *buf;

	 if (!xdr_opaque (xdrs, objp, FHSIZE))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_fhstatus (XDR *xdrs, fhstatus *objp)
{
	//register int32_t *buf;

	 if (!xdr_u_int (xdrs, &objp->fhs_status))
		 return FALSE;
	switch (objp->fhs_status) {
	case 0:
		 if (!xdr_fhandle (xdrs, objp->fhstatus_u.fhs_fhandle))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

bool_t
xdr_dirpath (XDR *xdrs, dirpath *objp)
{
	//register int32_t *buf;

	 if (!xdr_string (xdrs, objp, MNTPATHLEN))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_fhandle3 (XDR *xdrs, fhandle3 *objp)
{
	//register int32_t *buf;

	 if (!xdr_bytes (xdrs, (char **)&objp->fhandle3_val, (u_int *) &objp->fhandle3_len, FHSIZE3))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_mountres3_ok (XDR *xdrs, mountres3_ok *objp)
{
	//register int32_t *buf;

	 if (!xdr_fhandle3 (xdrs, &objp->fhandle))
		 return FALSE;
	 if (!xdr_array (xdrs, (char **)&objp->auth_flavours.auth_flavours_val, (u_int *) &objp->auth_flavours.auth_flavours_len, ~0,
		sizeof (int), (xdrproc_t) xdr_int))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_mountstat3 (XDR *xdrs, mountstat3 *objp)
{
	//register int32_t *buf;

	 if (!xdr_enum (xdrs, (enum_t *) objp))
		 return FALSE;
	return TRUE;
}

bool_t
xdr_mountres3 (XDR *xdrs, mountres3 *objp)
{
	//register int32_t *buf;

	 if (!xdr_mountstat3 (xdrs, &objp->fhs_status))
		 return FALSE;
	switch (objp->fhs_status) {
	case MNT_OK:
		 if (!xdr_mountres3_ok (xdrs, &objp->mountres3_u.mountinfo))
			 return FALSE;
		break;
	default:
		break;
	}
	return TRUE;
}

