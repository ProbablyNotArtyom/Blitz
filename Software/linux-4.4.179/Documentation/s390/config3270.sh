/*
 *  arch/arm/include/asm/uaccess.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _ASMARM_UACCESS_H
#define _ASMARM_UACCESS_H

/*
 * User space memory access functions
 */
#include <linux/string.h>
#include <asm/memory.h>
#include <asm/domain.h>
#include <asm/unified.h>
#include <asm/compiler.h>

#include <asm/extable.h>

/*
 * These two functions allow hooking accesses to userspace to increase
 * system integrity by ensuring that the kernel can not inadvertantly
 * perform such accesses (eg, via list poison values) which could then
 * be exploited for priviledge escalation.
 */
static inline unsigned int uaccess_save_and_enable(void)
{
#ifdef CONFIG_CPU_SW_DOMAIN_PAN
	unsigned int old_domain = get_domain();

	/* Set the current domain access to permit user accesses */
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
		   domain_val(DOMAIN_USER, DOMAIN_CLIENT));

	return old_domain;
#else
	return 0;
#endif
}

static inline void uaccess_restore(unsigned int flags)
{
#ifdef CONFIG_CPU_SW_DOMAIN_PAN
	/* Restore the user access mask */
	set_domain(flags);
#endif
}

/*
 * These two are intentionally not defined anywhere - if the kernel
 * code generates any references to them, that's a bug.
 */
extern int __get_user_bad(void);
extern int __put_user_bad(void);

/*
 * Note that this is actually 0x1,0000,0000
 */
#define KERNEL_DS	0x00000000
#define get_ds()	(KERNEL_DS)

#ifdef CONFIG_MMU

#define USER_DS		TASK_SIZE
#define get_fs()	(current_thread_info()->addr_limit)

static inline void set_fs(mm_segment_t fs)
{
	current_thread_info()->addr_limit = fs;

	/*
	 * Prevent a mispredicted conditional call to set_fs from forwarding
	 * the wrong address limit to access_ok under speculation.
	 */
	dsb(nsh);
	isb();

	modify_domain(DOMAIN_KERNEL, fs ? DOMAIN_CLIENT : DOMAIN_MANAGER);
}

#define segment_eq(a, b)	((a) == (b))

/* 