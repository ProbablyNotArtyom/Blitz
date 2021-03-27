/*
 *  linux/mm/vmalloc.c
 *
 *  Copyright (C) 1993  Linus Torvalds
 *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999
 *  SMP-safe vmalloc/vfree/ioremap, Tigran Aivazian <tigran@veritas.com>, May 2000
 *  Copyright (c) 2001 Lineo Inc., David McCullough <davidm@lineo.com>
 *  Copyright (c) 2000-2001 D Jeff Dionne <jeff@uClinux.org>
 */

#include <linux/config.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include <asm/uaccess.h>
#include <asm/pgalloc.h>

rwlock_t vmlist_lock = RW_LOCK_UNLOCKED;
struct vm_struct * vmlist;

void vfree(void * addr)
{
	kfree(addr);
}

void * __vmalloc (unsigned long size, int gfp_mask, pgprot_t prot)
{
	/*
	 * kmalloc doesn't like __GFP_HIGHMEM for some reason
	 * I doubt we need it - DAVIDM
	 */
	return kmalloc(size, gfp_mask & ~__GFP_HIGHMEM);
}

long vread(char *buf, char *addr, unsigned long count)
{
	memcpy(buf, addr, count);
	return count;
}

long vwrite(char *buf, char *addr, unsigned long count)
{
	/* Don't allow overflow */
	if ((unsigned long) addr + count < count)
		count = -(unsigned long) addr;
	
	memcpy(addr, buf, count);
	return(count);
}

