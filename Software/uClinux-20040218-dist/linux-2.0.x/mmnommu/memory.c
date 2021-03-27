/*
 *  linux/mm/memory.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 */

/*
 * uClinux revisions for NO_MM
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 */  

#include <linux/config.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/swap.h>

#include <asm/system.h>
#include <asm/segment.h>
#include <asm/pgtable.h>
#include <asm/string.h>

unsigned long high_memory = 0;

mem_map_t * mem_map = NULL;

/*
 * oom() prints a message (so that the user knows why the process died),
 * and gives the process an untrappable SIGKILL.
 */
void oom(struct task_struct * task)
{
	printk("\nOut of memory for %s.\n", task->comm);
	task->sig->action[SIGKILL-1].sa_handler = NULL;
	task->blocked &= ~(1<<(SIGKILL-1));
	send_sig(SIGKILL,task,1);
}

#ifdef DEBUG_VERIFY_AREA
#undef verify_area
int verify_area_flf(int type, const void * addr, unsigned long size, char *file, int line, char * function)
{
	int result;
	result = verify_area(type, addr, size);
	if (result)
		printk("%s:%d %s> verify_area(%d,%p,%ld) = %d\n",file,line, function, type, addr, size, result);
	return result;
}
#endif /* DEBUG_VERIFY_AREA */

/* FIXME:  Need to check the architecture memory map
 * DJD.
 */
int verify_area(int type, const void * addr, unsigned long size)
{
#ifdef __i960__
#define HIMEM	0xa4000000UL
#warning this is really ugly...
#else
#define HIMEM	0x10f00000UL
#endif
    
#if defined(CONFIG_COLDFIRE)
	extern unsigned long _ramend;
	if ((unsigned long)addr > _ramend) {
#elif defined(CONFIG_LEON_2)
	extern int _ramend;
	if ((unsigned long)addr > &_ramend) {
#else
	if ((unsigned long)addr > HIMEM) {
#endif

		if((type == VERIFY_READ) && ((unsigned long)addr >= 0xf0000000))
#if defined(CONFIG_FLASH1MB)
			if((unsigned long)addr < 0xf0100000)
#elif defined(CONFIG_FLASH2MB)
			if((unsigned long)addr < 0xf0200000)
#elif defined(CONFIG_FLASH4MB)
			if((unsigned long)addr < 0xf0400000)
#endif
			return 0;
		printk("Bad verify_area in process %d: %lx\n",
			current->pid, (unsigned long)addr);
		return -EFAULT;
	}
	return 0;
}
