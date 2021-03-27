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
 *	$Id: bjm.c,v 1.9 2000/04/10 22:22:31 wakkerma Exp $
 */
#include "defs.h"

#if defined(LINUX)

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#ifdef HAVE_SYS_USER_H
#include <sys/user.h>
#else
#undef PTRACE_SYSCALL
#include <linux/user.h>
#endif
#include <sys/syscall.h>
#include <signal.h>
#include <linux/module.h>

#ifdef __NR_query_module
static struct xlat which[] = {
	{ 0,		"0"		},
	{ QM_MODULES,	"QM_MODULES"	},
	{ QM_DEPS,	"QM_DEPS"	},
	{ QM_REFS,	"QM_REFS"	},
	{ QM_SYMBOLS,	"QM_SYMBOLS"	},
	{ QM_INFO,	"QM_INFO"	},
	{ 0,		NULL		},
};
#endif

#ifdef __NR_query_module
static struct xlat modflags[] = {
	{ MOD_UNINITIALIZED,	"MOD_UNINITIALIZED"	},
	{ MOD_RUNNING,		"MOD_RUNNING"		},
	{ MOD_DELETED,		"MOD_DELETED"		},
	{ MOD_AUTOCLEAN,	"MOD_AUTOCLEAN"		},
	{ MOD_VISITED,		"MOD_VISITED"		},
#ifdef MOD_USER_ONCE
	{ MOD_USED_ONCE,	"MOD_USED_ONCE"		},
#endif
#ifdef MOD_JUST_FREED
	{ MOD_JUST_FREED,	"MOD_JUST_FREED"	},
#endif
	{ 0,			NULL			},
};
#endif /* __NR_query_module */

#ifdef __NR_query_module
int
sys_query_module(tcp)
struct tcb *tcp;
{

	if (exiting(tcp)) {
		printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", ");
		printxval(which, tcp->u_arg[1], "QM_???");
		tprintf(", ");
		if (!verbose(tcp)) {
			tprintf("%#lx, %lu, %#lx", tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[4]);
		} else if (tcp->u_rval!=0) {
			size_t	ret;
			umove(tcp, tcp->u_arg[4], &ret);
			tprintf("%#lx, %lu, %Zu", tcp->u_arg[2], tcp->u_arg[3], ret);
		} else if (tcp->u_arg[1]==QM_INFO) {
			struct module_info	mi;
			size_t			ret;
			umove(tcp, tcp->u_arg[2], &mi);
			tprintf("{address=%#lx, size=%lu, flags=", mi.addr, mi.size);
			printflags(modflags, mi.flags);
			tprintf(", usecount=%lu}", mi.usecount);
			umove(tcp, tcp->u_arg[4], &ret);
			tprintf(", %Zu", ret);
		} else if ((tcp->u_arg[1]==QM_MODULES) ||
			       	(tcp->u_arg[1]==QM_DEPS) ||
			       	(tcp->u_arg[1]==QM_REFS)) {
			size_t	ret;

			umove(tcp, tcp->u_arg[4], &ret);
			tprintf("{");
			if (!abbrev(tcp)) {
				char*	data	= (char*)malloc(tcp->u_arg[3]);
				char*	mod	= data;
				size_t	idx;

				if (data==NULL) {
					fprintf(stderr, "sys_query_module: No memory\n");
					tprintf(" /* %Zu entries */ ", ret);
				} else {
					umoven(tcp, tcp->u_arg[2], tcp->u_arg[3], data);
					for (idx=0; idx<ret; idx++) {
						if (idx!=0)
							tprintf(",");
						tprintf(mod);
						mod+=strlen(mod)+1;
					}
					free(data);
				}
			} else 
				tprintf(" /* %Zu entries */ ", ret);
			tprintf("}, %Zu", ret);
		} else if (tcp->u_arg[1]==QM_SYMBOLS) {
			size_t	ret;
			umove(tcp, tcp->u_arg[4], &ret);
			tprintf("{");
			if (!abbrev(tcp)) {
				char*			data	= (char *)malloc(tcp->u_arg[3]);
				struct module_symbol*	sym	= (struct module_symbol*)data;
				size_t			idx;

				if (data==NULL) {
					fprintf(stderr, "sys_query_module: No memory\n");
					tprintf(" /* %Zu entries */ ", ret);
				} else {
					umoven(tcp, tcp->u_arg[2], tcp->u_arg[3], data);
					for (idx=0; idx<ret; idx++) {
						tprintf("{name=%s, value=%lu} ", data+(long)sym->name, sym->value);
						sym++;
					}
					free(data);
				}
			} else
				tprintf(" /* %Zu entries */ ", ret);
			tprintf("}, %Zd", ret);
		} else {
			printstr(tcp, tcp->u_arg[2], tcp->u_arg[3]);
			tprintf(", %#lx", tcp->u_arg[4]);
		}
	}
	return 0;
}
#endif

int
sys_create_module(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return RVAL_HEX;
}

int
sys_init_module(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %#lx", tcp->u_arg[1]);
	}
	return 0;
}
#endif /* LINUX */

