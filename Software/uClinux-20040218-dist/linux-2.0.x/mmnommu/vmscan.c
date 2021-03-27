/*
 *  linux/mm/vmscan.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *
 *  Swap reorganised 29.12.95, Stephen Tweedie.
 *  kswapd added: 7.1.96  sct
 *  Version: $Id: vmscan.c,v 1.3 2000/08/04 05:45:09 gerg Exp $
 */

/*
 * uClinux revisions for NO_MM
 * Copyright (C) 1998        Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 1998, 1999  D. Jeff Dionne <jeff@uclinux.org>,
 *                     2000  Rt-Control, Inc. /Lineo Inc.
 */  

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
#include <linux/pagemap.h>
#include <linux/smp_lock.h>

/*
 * We can't swap, so all we can do is shrink mmap.
 */
int try_to_free_page(int priority, int dma, int wait)
{
	int i=6;
	int stop, can_do_io;

#if 0
	printk("trying to find free page within pid %d...\n", current->pid);
#endif
	/* we don't try as hard if we're not waiting.. */
	stop = 3;
	can_do_io = 1;
	if (wait)
		stop = 0;
	if (priority == GFP_IO)
		can_do_io = 0;

       	do {
#if 0
	       	printk("Attempting to shrink_mmap\n");
#endif
       	        if (shrink_mmap(i, dma, can_do_io))
       			return 1;
		i--;
        } while ((i - stop) >= 0);

	printk("Can't find a free page for pid %d\n", current->pid);
	return 0;
}

/* 
 * In case someone wants to re-implement swapping...
 */

void init_swap_timer(void)
{
}
