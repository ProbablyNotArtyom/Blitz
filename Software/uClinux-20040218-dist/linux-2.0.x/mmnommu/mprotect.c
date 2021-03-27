/*
 *	linux/mm/mprotect.c
 *
 *  (C) Copyright 1994 Linus Torvalds
 */

/*
 * uClinux revisions for NO_MM
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 1999,2000  D. Jeff Dionne <jeff@uclinux.org>,
 *                          Rt-Control, Inc. /Lineo Inc.
 */  

#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/shm.h>
#include <linux/errno.h>
#include <linux/mman.h>
#include <linux/string.h>
#include <linux/malloc.h>

asmlinkage int sys_mprotect(unsigned long start, size_t len, unsigned long prot)
{
	return -ENOSYS;
}
