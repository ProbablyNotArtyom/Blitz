/*
 *  linux/mm/swapfile.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *  Swap reorganised 29.12.95, Stephen Tweedie
 *
 * uClinux revisions for NO_MM
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 1999, 2000   D. Jeff Dionne
 *                            Rt-Control, Inc. /Lineo Inc.
 */

#include <linux/config.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/stat.h>
#include <linux/swap.h>
#include <linux/fs.h>
#include <linux/swapctl.h>
#include <linux/blkdev.h> /* for blk_size */
#include <linux/shm.h>

/*
 * Compatibility functions mostly
 */
asmlinkage int sys_swapoff(const char * specialfile)
{
	return -ENOSYS;
}

asmlinkage int sys_swapon(const char * specialfile, int swap_flags)
{
	return -ENOSYS;
}

void si_swapinfo(struct sysinfo *val)
{
	val->freeswap = val->totalswap = 0;
	return;
}
