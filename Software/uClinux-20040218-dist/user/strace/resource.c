/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id: resource.c,v 1.7 2001/03/08 13:59:00 hughesj Exp $
 */

#include "defs.h"

#include <sys/resource.h>
#ifdef LINUX
#include <sys/times.h>
#include <linux/kernel.h>
#ifdef HAVE_SYS_QUOTA_H
#include <sys/quota.h>
#else
#include <linux/types.h>
#include <linux/quota.h>
#endif
#endif /* LINUX */
#ifdef SUNOS4
#include <ufs/quota.h>
#endif /* SUNOS4 */
#if defined(SVR4) || defined(FREEBSD)
#include <sys/times.h>
#include <sys/time.h>
#endif

#if HAVE_LONG_LONG_RLIM_T
/*
 * Hacks for systems that have a long long rlim_t
 */

#define rlimit64 rlimit			/* Ugly hack */
#define rlim64_t rlim_t			/* Ugly hack */
#define RLIM64_INFINITY RLIM_INFINITY	/* You guessed it */

#define sys_getrlimit64	sys_getrlimit
#define sys_setrlimit64	sys_setrlimit
#endif

static struct xlat resources[] = {
#ifdef RLIMIT_CPU
	{ RLIMIT_CPU,	"RLIMIT_CPU"	},
#endif
#ifdef RLIMIT_FSIZE
	{ RLIMIT_FSIZE,	"RLIMIT_FSIZE"	},
#endif
#ifdef RLIMIT_DATA
	{ RLIMIT_DATA,	"RLIMIT_DATA"	},
#endif
#ifdef RLIMIT_STACK
	{ RLIMIT_STACK,	"RLIMIT_STACK"	},
#endif
#ifdef RLIMIT_CORE
	{ RLIMIT_CORE,	"RLIMIT_CORE"	},
#endif
#ifdef RLIMIT_RSS
	{ RLIMIT_RSS,	"RLIMIT_RSS"	},
#endif
#ifdef RLIMIT_NPROC
	{ RLIMIT_NPROC,"RLIMIT_NPROC"	},
#endif
#ifdef RLIMIT_NOFILE
	{ RLIMIT_NOFILE,"RLIMIT_NOFILE"	},
#endif
#ifdef RLIMIT_MEMLOCK
	{ RLIMIT_MEMLOCK,	"RLIMIT_MEMLOCK"	},
#endif
#ifdef RLIMIT_VMEM
	{ RLIMIT_VMEM,	"RLIMIT_VMEM"	},
#endif
#ifdef RLIMIT_AS
	{ RLIMIT_AS,	"RLIMIT_AS"	},
#endif
	{ 0,		NULL		},
};

#if !HAVE_LONG_LONG_RLIM_T
static char *
sprintrlim(lim)
long lim;
{
	static char buf[32];

	if (lim == RLIM_INFINITY)
		sprintf(buf, "RLIM_INFINITY");
	else if (lim > 1024 && lim%1024 == 0)
		sprintf(buf, "%ld*1024", lim/1024);
	else
		sprintf(buf, "%ld", lim);
	return buf;
}

int
sys_getrlimit(tcp)
struct tcb *tcp;
{
	struct rlimit rlim;

	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprintf(", ");
	}
	else {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &rlim) < 0)
			tprintf("{...}");
		else {
			tprintf("{rlim_cur=%s,", sprintrlim(rlim.rlim_cur));
			tprintf(" rlim_max=%s}", sprintrlim(rlim.rlim_max));
		}
	}
	return 0;
}

int
sys_setrlimit(tcp)
struct tcb *tcp;
{
	struct rlimit rlim;

	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprintf(", ");
		if (!verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &rlim) < 0)
			tprintf("{...}");
		else {
			tprintf("{rlim_cur=%s,", sprintrlim(rlim.rlim_cur));
			tprintf(" rlim_max=%s}", sprintrlim(rlim.rlim_max));
		}
	}
	return 0;
}
#endif /* !HAVE_LONG_LONG_RLIM_T */

#if _LFS64_LARGEFILE || HAVE_LONG_LONG_RLIM_T
static char *
sprintrlim64(lim)
rlim64_t lim;
{
	static char buf[64];

	if (lim == RLIM64_INFINITY)
		sprintf(buf, "RLIM64_INFINITY");
	else if (lim > 1024 && lim%1024 == 0)
		sprintf(buf, "%lld*1024", lim/1024);
	else
		sprintf(buf, "%lld", lim);
	return buf;
}

int
sys_getrlimit64(tcp)
struct tcb *tcp;
{
	struct rlimit64 rlim;

	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprintf(", ");
	}
	else {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &rlim) < 0)
			tprintf("{...}");
		else {
			tprintf("{rlim_cur=%s,", sprintrlim64(rlim.rlim_cur));
			tprintf(" rlim_max=%s}", sprintrlim64(rlim.rlim_max));
		}
	}
	return 0;
}

int
sys_setrlimit64(tcp)
struct tcb *tcp;
{
	struct rlimit64 rlim;

	if (entering(tcp)) {
		printxval(resources, tcp->u_arg[0], "RLIMIT_???");
		tprintf(", ");
		if (!verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &rlim) < 0)
			tprintf("{...}");
		else {
			tprintf("{rlim_cur=%s,", sprintrlim64(rlim.rlim_cur));
			tprintf(" rlim_max=%s}", sprintrlim64(rlim.rlim_max));
		}
	}
	return 0;
}
#endif /* _LFS64_LARGEFILES || HAVE_LONG_LONG_RLIM_T */

#ifndef SVR4

static struct xlat usagewho[] = {
	{ RUSAGE_SELF,		"RUSAGE_SELF"		},
	{ RUSAGE_CHILDREN,	"RUSAGE_CHILDREN"	},
#ifdef RUSAGE_BOTH
	{ RUSAGE_BOTH,		"RUSAGE_BOTH"		},
#endif
	{ 0,			NULL			},
};

#ifdef ALPHA
void
printrusage32(tcp, addr)
struct tcb *tcp;
long addr;
{
    struct timeval32
    {
	unsigned tv_sec;
	unsigned tv_usec;
    };
    struct rusage32
    {
	struct timeval32 ru_utime;	/* user time used */
	struct timeval32 ru_stime;	/* system time used */
	long	ru_maxrss;		/* maximum resident set size */
	long	ru_ixrss;		/* integral shared memory size */
	long	ru_idrss;		/* integral unshared data size */
	long	ru_isrss;		/* integral unshared stack size */
	long	ru_minflt;		/* page reclaims */
	long	ru_majflt;		/* page faults */
	long	ru_nswap;		/* swaps */
	long	ru_inblock;		/* block input operations */
	long	ru_oublock;		/* block output operations */
	long	ru_msgsnd;		/* messages sent */
	long	ru_msgrcv;		/* messages received */
	long	ru_nsignals;		/* signals received */
	long	ru_nvcsw;		/* voluntary context switches */
	long	ru_nivcsw;		/* involuntary " */
    } ru;

    if (!addr)
	tprintf("NULL");
    else if (syserror(tcp) || !verbose(tcp))
	tprintf("%#lx", addr);
    else if (umove(tcp, addr, &ru) < 0)
	tprintf("{...}");
    else if (!abbrev(tcp)) {
	tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ",
		(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
		(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
	tprintf("ru_maxrss=%lu, ru_ixrss=%lu, ",
		ru.ru_maxrss, ru.ru_ixrss);
	tprintf("ru_idrss=%lu, ru_isrss=%lu, ",
		ru.ru_idrss, ru.ru_isrss);
	tprintf("ru_minflt=%lu, ru_majflt=%lu, ru_nswap=%lu, ",
		ru.ru_minflt, ru.ru_majflt, ru.ru_nswap);
	tprintf("ru_inblock=%lu, ru_oublock=%lu, ",
		ru.ru_inblock, ru.ru_oublock);
	tprintf("ru_msgsnd=%lu, ru_msgrcv=%lu, ",
		ru.ru_msgsnd, ru.ru_msgrcv);
	tprintf("ru_nsignals=%lu, ru_nvcsw=%lu, ru_nivcsw=%lu}",
		ru.ru_nsignals, ru.ru_nvcsw, ru.ru_nivcsw);
    }
    else {
	tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ...}",
		(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
		(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
    }
}
#endif

void
printrusage(tcp, addr)
struct tcb *tcp;
long addr;
{
	struct rusage ru;

	if (!addr)
		tprintf("NULL");
	else if (syserror(tcp) || !verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &ru) < 0)
		tprintf("{...}");
	else if (!abbrev(tcp)) {
		tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ",
			(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
			(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
		tprintf("ru_maxrss=%lu, ru_ixrss=%lu, ",
			ru.ru_maxrss, ru.ru_ixrss);
		tprintf("ru_idrss=%lu, ru_isrss=%lu, ",
			ru.ru_idrss, ru.ru_isrss);
		tprintf("ru_minflt=%lu, ru_majflt=%lu, ru_nswap=%lu, ",
			ru.ru_minflt, ru.ru_majflt, ru.ru_nswap);
		tprintf("ru_inblock=%lu, ru_oublock=%lu, ",
			ru.ru_inblock, ru.ru_oublock);
		tprintf("ru_msgsnd=%lu, ru_msgrcv=%lu, ",
			ru.ru_msgsnd, ru.ru_msgrcv);
		tprintf("ru_nsignals=%lu, ru_nvcsw=%lu, ru_nivcsw=%lu}",
			ru.ru_nsignals, ru.ru_nvcsw, ru.ru_nivcsw);
	}
	else {
		tprintf("{ru_utime={%lu, %lu}, ru_stime={%lu, %lu}, ...}",
			(long) ru.ru_utime.tv_sec, (long) ru.ru_utime.tv_usec,
			(long) ru.ru_stime.tv_sec, (long) ru.ru_stime.tv_usec);
	}
}

int
sys_getrusage(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
		tprintf(", ");
	}
	else
		printrusage(tcp, tcp->u_arg[1]);
	return 0;
}

#ifdef ALPHA
int
sys_osf_getrusage(tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	printxval(usagewho, tcp->u_arg[0], "RUSAGE_???");
	tprintf(", ");
    }
    else
	printrusage32(tcp, tcp->u_arg[1]);
    return 0;
}
#endif /* ALPHA */

#endif /* !SVR4 */

#ifdef LINUX

int
sys_sysinfo(tcp)
struct tcb *tcp;
{
	struct sysinfo si;

	if (exiting(tcp)) {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &si) < 0)
			tprintf("{...}");
		else {
			tprintf("{uptime=%lu, loads=[%lu, %lu, %lu] ",
				si.uptime, si.loads[0], si.loads[1],
				si.loads[2]);
			tprintf("totalram=%lu, freeram=%lu, ",
				si.totalram, si.freeram);
			tprintf("sharedram=%lu, bufferram=%lu} ",
				si.sharedram, si.bufferram);
			tprintf("totalswap=%lu, freeswap=%lu, procs=%hu}",
				si.totalswap, si.freeswap, si.procs);
		}
	}
	return 0;
}

#endif /* LINUX */

static struct xlat priorities[] = {
	{ PRIO_PROCESS,	"PRIO_PROCESS"	},
	{ PRIO_PGRP,	"PRIO_PGRP"	},
	{ PRIO_USER,	"PRIO_USER"	},
	{ 0,		NULL		},
};

int
sys_getpriority(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(priorities, tcp->u_arg[0], "PRIO_???");
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setpriority(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(priorities, tcp->u_arg[0], "PRIO_???");
		tprintf(", %lu, %ld", tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_nice(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%ld", tcp->u_arg[0]);
	return 0;
}

#ifndef SUNOS4

int
sys_times(tcp)
struct tcb *tcp;
{
	struct tms tbuf;

	if (exiting(tcp)) {
		if (tcp->u_arg[0] == 0)
			tprintf("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &tbuf) < 0)
			tprintf("{...}");
		else {
			tprintf("{tms_utime=%lu, tms_stime=%lu, ",
				tbuf.tms_utime, tbuf.tms_stime);
			tprintf("tms_cutime=%lu, tms_cstime=%lu}",
				tbuf.tms_cutime, tbuf.tms_cstime);
		}
	}
	return 0;
}

#endif /* !SUNOS4 */

#ifdef LINUX

static struct xlat quotacmds[] = {
	{ Q_QUOTAON,	"Q_QUOTAON"	},
	{ Q_QUOTAOFF,	"Q_QUOTAOFF"	},
	{ Q_GETQUOTA,	"Q_GETQUOTA"	},
	{ Q_SETQUOTA,	"Q_SETQUOTA"	},
#ifdef Q_SETUSE
	{ Q_SETUSE,	"Q_SETUSE"	},
#endif
	{ Q_SYNC,	"Q_SYNC"	},
#ifdef Q_SETQLIM
	{ Q_SETQLIM,	"Q_SETQLIM"	},
#endif
#ifdef Q_GETSTATS
	{ Q_GETSTATS,	"Q_GETSTATS"	},
#endif
#ifdef Q_RSQUASH
	{ Q_RSQUASH,	"Q_RSQUASH"	},
#endif
	{ 0,		NULL		},
};

static struct xlat quotatypes[] = {
	{ USRQUOTA,	"USRQUOTA"	},
	{ GRPQUOTA,	"GRPQUOTA"	},
	{ 0,		NULL		},
};

int
sys_quotactl(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(quotacmds, tcp->u_arg[0] >> SUBCMDSHIFT, "Q_???");
		tprintf("|");
		printxval(quotatypes, tcp->u_arg[0] & SUBCMDMASK, "???QUOTA");
		tprintf(", ");
		printstr(tcp, tcp->u_arg[1], -1);
		tprintf(", %lu, ", tcp->u_arg[2]);
	}
	else {
		struct dqblk dq;

		if (!tcp->u_arg[3])
			tprintf("NULL");
		else if (!verbose(tcp))
			tprintf("%#lx", tcp->u_arg[3]);
                else if (umoven(tcp, tcp->u_arg[3], sizeof(struct dqblk),   
                    (char *) &dq) < 0)
                        tprintf("???");
		else {
                        tprintf("{");
			tprintf("%u, ", dq.dqb_bhardlimit);
			tprintf("%u, ", dq.dqb_bsoftlimit);
			tprintf("%u, ", dq.dqb_curblocks);
			tprintf("%u, ", dq.dqb_ihardlimit);
			tprintf("%u, ", dq.dqb_isoftlimit);
			tprintf("%u, ", dq.dqb_curinodes);
			tprintf("%lu, ", dq.dqb_btime);
			tprintf("%lu", dq.dqb_itime);
                        tprintf("}");
		}

	}
	return 0;
}

#endif /* Linux */

#if defined(SUNOS4) || defined(FREEBSD)

#ifdef FREEBSD
#include <ufs/ufs/quota.h>
#endif

static struct xlat quotacmds[] = {
	{ Q_QUOTAON,	"Q_QUOTAON"	},
	{ Q_QUOTAOFF,	"Q_QUOTAOFF"	},
	{ Q_GETQUOTA,	"Q_GETQUOTA"	},
	{ Q_SETQUOTA,	"Q_SETQUOTA"	},
#ifdef Q_SETQLIM
	{ Q_SETQLIM,	"Q_SETQLIM"	},
#endif
#ifdef Q_SETUSE
	{ Q_SETUSE,	"Q_SETUSE"	},
#endif
	{ Q_SYNC,	"Q_SYNC"	},
	{ 0,		NULL		},
};

int
sys_quotactl(tcp)
struct tcb *tcp;
{
	/* fourth arg (addr) not interpreted here */
	if (entering(tcp)) {
#ifdef SUNOS4
		printxval(quotacmds, tcp->u_arg[0], "Q_???");
		tprintf(", ");
		printstr(tcp, tcp->u_arg[1], -1);
#endif
#ifdef FREEBSD
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", ");
		printxval(quotacmds, tcp->u_arg[1], "Q_???");
#endif		
		tprintf(", %lu, %#lx", tcp->u_arg[2], tcp->u_arg[3]);
	}
	return 0;
}

#endif /* SUNOS4 || FREEBSD */
