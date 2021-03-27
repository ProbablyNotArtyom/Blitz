/*
 *  linux/kernel/panic.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Revisions for CONFIG_CONSOLE by Kenneth Albanowski
 *  Copyright (c) 1997, 1998 
 * 
 */

/*
 * This function is used through-out the kernel (including mm and fs)
 * to indicate a major problem.
 */
#include <stdarg.h>

#include <linux/config.h> /* CONFIG_SCSI_GDTH */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>

#ifdef CONFIG_LEDMAN
#include <linux/ledman.h>
#endif

asmlinkage void sys_sync(void);	/* it's really int */
extern void hard_reset_now(void);
extern void do_unblank_screen(void);
extern void DAC960_Finalize(void);
extern void gdth_halt(void);
extern int C_A_D;

int panic_timeout = 0;

void panic_setup(char *str, int *ints)
{
	if (ints[0] == 1)
		panic_timeout = ints[1];
}

NORET_TYPE void panic(const char * fmt, ...)
{
	static char buf[1024];
	va_list args;
	int i;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	printk(KERN_EMERG "Kernel panic: %s\n",buf);
	if (current == task[0])
		printk(KERN_EMERG "In swapper task - not syncing\n");
	else
		sys_sync();

#ifdef CONFIG_CONSOLE
	do_unblank_screen();
#endif /* CONFIG_CONSOLE */

	if (panic_timeout > 0)
	{
		/*
	 	 * Delay timeout seconds before rebooting the machine. 
		 * We can't use the "normal" timers since we just panicked..
	 	 */
		printk(KERN_EMERG "Rebooting in %d seconds..",panic_timeout);
		for(i = 0; i < (panic_timeout*1000); i++)
			udelay(1000);
#ifdef CONFIG_BLK_DEV_DAC960
		DAC960_Finalize();
#endif
#ifdef CONFIG_SCSI_GDTH
		gdth_halt();
#endif
		hard_reset_now();
	}

#ifdef CONFIG_LEDMAN
	ledman_cmd(LEDMAN_CMD_ON, LEDMAN_ALL);
#endif
#ifdef CONFIG_M68360
    printk(KERN_EMERG "Rebooting in 4 seconds....\n");
    cli();
#endif
	for(;;);
}

/*
 * GCC 2.5.8 doesn't always optimize correctly; see include/asm/segment.h
 */

int bad_user_access_length(void)
{
        panic("bad_user_access_length executed (not cool, dude)");
}
