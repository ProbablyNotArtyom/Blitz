/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
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
 *	$Id: ipc.c,v 1.3 2000/09/01 21:03:06 wichert Exp $
 */

#include "defs.h"

#if (defined(LINUX) && LINUX_VERSION_CODE >= 0x020100) || defined(SUNOS4) || defined(FREEBSD)

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>

#ifndef MSG_STAT
#define MSG_STAT 11
#endif
#ifndef MSG_INFO
#define MSG_INFO 12
#endif
#ifndef SHM_STAT
#define SHM_STAT 13
#endif
#ifndef SHM_INFO
#define SHM_INFO 14
#endif
#ifndef SEM_STAT
#define SEM_STAT 18
#endif
#ifndef SEM_INFO
#define SEM_INFO 19
#endif

static struct xlat msgctl_flags[] = {
	{ IPC_RMID,	"IPC_RMID"	},
	{ IPC_SET,	"IPC_SET"	},
	{ IPC_STAT,	"IPC_STAT"	},
#ifdef LINUX
	{ IPC_INFO,	"IPC_INFO"	},
	{ MSG_STAT,	"MSG_STAT"	},
	{ MSG_INFO,	"MSG_INFO"	},
#endif /* LINUX */
	{ 0,		NULL		},
};

static struct xlat semctl_flags[] = {
	{ IPC_RMID,	"IPC_RMID"	},
	{ IPC_SET,	"IPC_SET"	},
	{ IPC_STAT,	"IPC_STAT"	},
#ifdef LINUX
	{ IPC_INFO,	"IPC_INFO"	},
	{ SEM_STAT,	"SEM_STAT"	},
	{ SEM_INFO,	"SEM_INFO"	},
#endif /* LINUX */
	{ GETPID,	"GETPID"	},
	{ GETVAL,	"GETVAL"	},
	{ GETALL,	"GETALL"	},
	{ GETNCNT,	"GETNCNT"	},
	{ GETZCNT,	"GETZCNT"	},
	{ SETVAL,	"SETVAL"	},
	{ SETALL,	"SETALL"	},
	{ 0,		NULL		},
};

static struct xlat shmctl_flags[] = {
	{ IPC_RMID,	"IPC_RMID"	},
	{ IPC_SET,	"IPC_SET"	},
	{ IPC_STAT,	"IPC_STAT"	},
#ifdef LINUX
	{ IPC_INFO,	"IPC_INFO"	},
	{ SHM_STAT,	"SHM_STAT"	},
	{ SHM_INFO,	"SHM_INFO"	},
#endif /* LINUX */
#ifdef SHM_LOCK	
	{ SHM_LOCK,	"SHM_LOCK"	},
#endif
#ifdef SHM_UNLOCK	
	{ SHM_UNLOCK,	"SHM_UNLOCK"	},
#endif	
	{ 0,		NULL		},
};

static struct xlat resource_flags[] = {
	{ IPC_CREAT,	"IPC_CREAT"	},
	{ IPC_EXCL,	"IPC_EXCL"	},
	{ IPC_NOWAIT,	"IPC_NOWAIT"	},
	{ 0,		NULL		},
};

static struct xlat shm_flags[] = {
#ifdef LINUX
	{ SHM_REMAP,	"SHM_REMAP"	},
#endif /* LINUX */
	{ SHM_RDONLY,	"SHM_RDONLY"	},
	{ SHM_RND,	"SHM_RND"	},
	{ 0,		NULL		},
};

static struct xlat msg_flags[] = {
	{ MSG_NOERROR,	"MSG_NOERROR"	},
#ifdef LINUX
	{ MSG_EXCEPT,	"MSG_EXCEPT"	},
#endif /* LINUX */
	{ IPC_NOWAIT,	"IPC_NOWAIT"	},
	{ 0,		NULL		},
};

int sys_msgget(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		if (tcp->u_arg[0])
			tprintf("%lu", tcp->u_arg[0]);
		else
			tprintf("IPC_PRIVATE");
		tprintf(", ");
		if (printflags(resource_flags, tcp->u_arg[1]) != 0)
			tprintf("|");
		tprintf("%#lo", tcp->u_arg[1] & 0666);
	}
	return 0;
}

int sys_msgctl(tcp)
struct tcb *tcp;
{
	char *cmd = xlookup(msgctl_flags, tcp->u_arg[1]);

	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
		tprintf(", %s", cmd == NULL ? "MSG_???" : cmd);
#ifdef LINUX
		tprintf(", %#lx", tcp->u_arg[3]);
#else /* !LINUX */
		tprintf(", %#lx", tcp->u_arg[2]);
#endif /* !LINUX */
	}
	return 0;
}

int sys_msgsnd(tcp)
struct tcb *tcp;
{
	long mtype;

	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
#ifdef LINUX
		umove(tcp, tcp->u_arg[3], &mtype);
		tprintf(", {%lu, ", mtype);
		printstr(tcp, tcp->u_arg[3] + sizeof(long),
			tcp->u_arg[1]);
		tprintf("}, %lu", tcp->u_arg[1]);
		tprintf(", ");
		if (printflags(msg_flags, tcp->u_arg[2]) == 0)
			tprintf("0");
#else /* !LINUX */
		umove(tcp, tcp->u_arg[1], &mtype);
		tprintf(", {%lu, ", mtype);
		printstr(tcp, tcp->u_arg[1] + sizeof(long),
			tcp->u_arg[2]);
		tprintf("}, %lu", tcp->u_arg[2]);
		tprintf(", ");
		if (printflags(msg_flags, tcp->u_arg[3]) == 0)
			tprintf("0");
#endif /* !LINUX */
	}
	return 0;
}

int sys_msgrcv(tcp)
struct tcb *tcp;
{
	long mtype;
#ifdef LINUX
	struct ipc_wrapper {
		struct msgbuf *msgp;
		long msgtyp;
	} tmp;
#endif


	if (exiting(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
#ifdef LINUX
		umove(tcp, tcp->u_arg[3], &tmp);
		umove(tcp, (long) tmp.msgp, &mtype);
		tprintf(", {%lu, ", mtype);
		printstr(tcp, (long) (tmp.msgp) + sizeof(long),
			tcp->u_arg[1]);
		tprintf("}, %lu", tcp->u_arg[1]);
		tprintf(", %ld", tmp.msgtyp);
		tprintf(", ");
		if (printflags(msg_flags, tcp->u_arg[2]) == 0)
			tprintf("0");
#else /* !LINUX */
		umove(tcp, tcp->u_arg[1], &mtype);
		tprintf(", {%lu, ", mtype);
		printstr(tcp, tcp->u_arg[1] + sizeof(long),
			tcp->u_arg[2]);
		tprintf("}, %lu", tcp->u_arg[2]);
		tprintf(", %ld", tcp->u_arg[3]);
		tprintf(", ");
		if (printflags(msg_flags, tcp->u_arg[4]) == 0)
			tprintf("0");
#endif /* !LINUX */
	}
	return 0;
}

int sys_semop(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
#ifdef LINUX
		tprintf(", %#lx", tcp->u_arg[3]);
		tprintf(", %lu", tcp->u_arg[1]);
#else /* !LINUX */
		tprintf(", %#lx", tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
#endif /* !LINUX */
	}
	return 0;
}

int sys_semget(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		if (tcp->u_arg[0])
			tprintf("%lu", tcp->u_arg[0]);
		else
			tprintf("IPC_PRIVATE");
		tprintf(", %lu", tcp->u_arg[1]);
		tprintf(", ");
		if (printflags(resource_flags, tcp->u_arg[2]) != 0)
			tprintf("|");
		tprintf("%#lo", tcp->u_arg[2] & 0666);
	}
	return 0;
}

int sys_semctl(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
		tprintf(", %lu, ", tcp->u_arg[1]);
		printxval(semctl_flags, tcp->u_arg[2], "SEM_???");
		tprintf(", %#lx", tcp->u_arg[3]);
	}
	return 0;
}

int sys_shmget(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		if (tcp->u_arg[0])
			tprintf("%lu", tcp->u_arg[0]);
		else
			tprintf("IPC_PRIVATE");
		tprintf(", %lu", tcp->u_arg[1]);
		tprintf(", ");
		if (printflags(resource_flags, tcp->u_arg[2]) != 0)
			tprintf("|");
		tprintf("%#lo", tcp->u_arg[2] & 0666);
	}
	return 0;
}

int sys_shmctl(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
		printxval(shmctl_flags, tcp->u_arg[1], "SHM_???");
#ifdef LINUX
		tprintf(", %#lx", tcp->u_arg[3]);
#else /* !LINUX */
		tprintf(", %#lx", tcp->u_arg[2]);
#endif /* !LINUX */
	}
	return 0;
}

int sys_shmat(tcp)
struct tcb *tcp;
{
#ifdef LINUX
	unsigned long raddr;
#endif /* LINUX */

	if (exiting(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
#ifdef LINUX
		tprintf(", %#lx", tcp->u_arg[3]);
		tprintf(", ");
		if (printflags(shm_flags, tcp->u_arg[1]) == 0)
			tprintf("0");
#else /* !LINUX */
		tprintf(", %#lx", tcp->u_arg[1]);
		tprintf(", ");
		if (printflags(shm_flags, tcp->u_arg[2]) == 0)
			tprintf("0");
#endif /* !LINUX */
		if (syserror(tcp))
			return 0;
#ifdef LINUX
		if (umove(tcp, tcp->u_arg[2], &raddr) < 0)
			return RVAL_NONE;
		tcp->u_rval = raddr;
#endif /* LINUX */
		return RVAL_HEX;
	}
	return 0;
}

int sys_shmdt(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
#ifdef LINUX
		tprintf("%#lx", tcp->u_arg[3]);
#else /* !LINUX */
		tprintf("%#lx", tcp->u_arg[0]);
#endif /* !LINUX */
	return 0;
}

#endif /* defined(LINUX) || defined(SUNOS4) || defined(FREEBSD) */
