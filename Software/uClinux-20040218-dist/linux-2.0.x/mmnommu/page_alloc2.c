/****************************************************************************/
/*
 *  linux/mmnommu/page_alloc2.c
 *
 *  	Copyright (C) 2000 Lineo Australia
 *  	davidm@lineo.com
 *
 *	A page allocator that attempts to be better than the
 *	standard power of 2 allocator.
 *
 *	This stuff is really simple,  we have the standard memory map
 *	and we have memory.  No bitmaps or fancy stuff.
 *	Whenever an allocation request comes in we search the mem_map
 *	for a run of free pages to satisfy the request.  Because of this
 *	we automatically coalesce adjacent blocks of free pages without
 *	even trying.
 *
 *	IDEAS/TODO:
 *
 *	  Some possible ways to correct problems if they appear in general
 *	  use are listed below.  Until I get more real world testing I
 *	  don't even know if they will be required.
 *
 *	  - implement a best fit rather than first fit scan of mem map
 *	    to help with fragmentation issues
 *	  - look at some simple ways to improve speed for free space search.
 * 	  - work out a realistic fragmentation percentage
 *
 *	Based in part on the following:
 *
 *	linux/mm/page_alloc:
 *		Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *		Swap reorganised 29.12.95, Stephen Tweedie
 *	linux/mmnommu/page_alloc:
 *		Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *                     The Silver Hammer Group, Ltd.
 *		Copyright (C) 1999  D. Jeff Dionne <jeff@uclinux.org>,
 *                     Rt-Control, Inc.
 */  
/****************************************************************************/

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
#include <linux/interrupt.h>

#include <asm/dma.h>
#include <asm/system.h> /* for cli()/sti() */
#include <asm/segment.h> /* for memcpy_to/fromfs */
#include <asm/bitops.h>
#include <asm/pgtable.h>

/****************************************************************************/
/*
 *	do we want nasty stuff checking enabled
 */
#undef SADISTIC_PAGE_ALLOC

/*
 *	Some accounting stuff
 */

int nr_swap_pages = 0;
int nr_free_pages = 0;

/*
 *	A simple method to save us searching all the reserved kernel
 *	pages every time is to remember where the first free page is
 */

static char *bit_map = NULL;
static int	 bit_map_size = 0;
static int	 first_usable_page = 0;

extern struct wait_queue *buffer_wait;

#define ADDRESS(x) (PAGE_OFFSET + ((x) << PAGE_SHIFT))

/****************************************************************************/
#ifdef SADISTIC_PAGE_ALLOC

static void
mem_set(unsigned char *p, int n)
{
	while (n-- > 0)
		*p++ = 0xd0;
}

static void
mem_test(unsigned char *p, int n)
{
	while (n-- > 0)
		if (*p++ != 0xd0)
			break;
	if (n >= 0)
		printk("free memory changed 0x%x, 0x%x = 0xd0\n",
				(unsigned int) p - 1, *(p - 1));
}

#endif
/****************************************************************************/

void __free_page(struct page *page)
{
	unsigned long flags;

	if (PageReserved(page))
		return;
	save_flags(flags);
	cli();
	if (atomic_dec_and_test(&page->count)) {
#ifdef SADISTIC_PAGE_ALLOC
		mem_set((char *) ADDRESS(page->map_nr), PAGE_SIZE);
#endif
		if (page->map_nr < first_usable_page)
			first_usable_page = page->map_nr;
		clear_bit(page->map_nr, bit_map);
		restore_flags(flags);
		nr_free_pages++;
		if (!waitqueue_active(&buffer_wait))
			return;
		wake_up(&buffer_wait);
	}
	restore_flags(flags);
}

/****************************************************************************/

void free_contiguous_pages(unsigned long addr, unsigned long num_adjpages)
{
	unsigned long map_nr = MAP_NR(addr);
	unsigned long flags;

	if (map_nr < MAP_NR(high_memory)) {
		int freed = 0;
		mem_map_t *p, *ep;

		for (p = mem_map + map_nr, ep = p + num_adjpages; p < ep; p++) {
			/* we never hand out reserved pages so ignore it */
			if (PageReserved(p))
				return;
		}
		save_flags(flags);
		cli();
		for (p = mem_map + map_nr, ep = p + num_adjpages; p < ep; p++) {
			if (atomic_dec_and_test(&p->count)) {
#ifdef SADISTIC_PAGE_ALLOC
				mem_set((char *) ADDRESS(p->map_nr), PAGE_SIZE);
#endif
				if (p->map_nr < first_usable_page)
					first_usable_page = p->map_nr;
				clear_bit(p->map_nr, bit_map);
				freed++;
			}
		}
		restore_flags(flags);

		if (freed) {
			nr_free_pages += freed;
			if (!waitqueue_active(&buffer_wait))
				return;
			wake_up(&buffer_wait);
		}
	}
}

/****************************************************************************/
/*
 *	We have to keep this interface as some parts of the kernel
 *	source reference them directly
 */

void free_pages(unsigned long addr, unsigned long order)
{
	free_contiguous_pages(addr, 1 << order);
}

/****************************************************************************/
/*
 *	look through the map for a run of consecutive pages that will
 *	hold order # or pages
 */

unsigned long
__get_contiguous_pages(int priority, unsigned long num_adjpages, int dma)
{
	unsigned long		 flags;
	int					 reserved_pages;
	register mem_map_t	*p;

	if (intr_count && priority != GFP_ATOMIC) {
		static int count = 0;
		if (++count < 5) {
			printk("gfp called nonatomically from interrupt %p\n",
				__builtin_return_address(0));
			priority = GFP_ATOMIC;
		}
	}

	reserved_pages = 5;
#ifndef CONFIG_REDUCED_MEMORY
	if (priority != GFP_NFS)
		reserved_pages = min_free_pages;
	if ((priority == GFP_BUFFER || priority == GFP_IO) && reserved_pages >= 48)
		reserved_pages -= (12 + (reserved_pages>>3));
#endif /* !CONFIG_REDUCED_MEMORY */

	save_flags(flags);

repeat:
	cli();
/*
 *	Don't bother trying to find pages unless there are enough
 *	for the given context
 */
	if (num_adjpages <= nr_free_pages &&
			(priority == GFP_ATOMIC ||
			 nr_free_pages - num_adjpages > reserved_pages)) {

		int n = 0, ff;

		p = NULL;
		ff = find_next_zero_bit(bit_map, bit_map_size, first_usable_page);

		while (ff + num_adjpages <= bit_map_size) {
			p = mem_map + ff;
			for (n = 0; n < num_adjpages; n++, p++) {
				if (test_bit(p->map_nr, bit_map))
					break;
				if (dma && !PageDMA(p))
					break;
			}
			if (n >= num_adjpages)
				break;
			ff = find_next_zero_bit(bit_map, bit_map_size, ff + n + 1);
		}

		if (p && n >= num_adjpages) {
			nr_free_pages -= num_adjpages;
			while (n-- > 0) {
				p--;
#ifdef SADISTIC_PAGE_ALLOC
				if (p->count)
					printk("allocated a non-free page\n");
#endif
				p->count = 1;
				p->age = PAGE_INITIAL_AGE;
				p->next = p->prev = NULL;
				set_bit(p->map_nr, bit_map);
			}
#ifdef SADISTIC_PAGE_ALLOC
			mem_test((char *) ADDRESS(p->map_nr), num_adjpages * PAGE_SIZE);
			mem_set((char *) ADDRESS(p->map_nr), num_adjpages * PAGE_SIZE);
#endif
			restore_flags(flags);
			return(ADDRESS(p->map_nr));
		}
	}
	restore_flags(flags);
	if (priority != GFP_BUFFER && try_to_free_page(priority, dma, 1))
		goto repeat;
	return(0);
}

/****************************************************************************/
/*
 *	as for free_pages,  we have to provide this one as well
 */

unsigned long __get_free_pages(int priority, unsigned long order, int dma)
{
	return(__get_contiguous_pages(priority, 1 << order, dma));
}

/****************************************************************************/
/*
 *	totals held by do_mmap to compute memory wastage
 */

unsigned long realalloc, askedalloc;

/****************************************************************************/
/*
 *	dump some stats on how we are doing
 */

static void
print_free_areas(char *buffer)
{
	mem_map_t	*p, *ep;
 	unsigned long	flags, slack;
 	unsigned long	min_free=high_memory,max_free=0,avg_free=0,free_blks=0;
 	unsigned long	min_used=high_memory,max_used=0,avg_used=0,used_blks=0;

#define	PRINTK(fmt, a...) \
	(buffer ? sprintf(&buffer[strlen(buffer)], fmt, ##a) : printk(fmt, ##a))

	if (realalloc)
		slack = (realalloc-askedalloc) * 100 / realalloc;
	else
		slack = 0;
	
	save_flags(flags);
	cli();

 	for (p = mem_map, ep = p + MAP_NR(high_memory); p < ep; ) {
		int n;
		
		n = 0;

		if (test_bit(p->map_nr, bit_map)) {
			while (p < ep && test_bit(p->map_nr, bit_map)) {
				n++;
				p++;
			}
			avg_used += n;
			if (n < min_used)
				min_used = n;
			if (n > max_used)
				max_used = n;
			used_blks++;
		} else {
			while (p < ep && !test_bit(p->map_nr, bit_map)) {
				n++;
				p++;
			}
			avg_free += n;
			if (n < min_free)
				min_free = n;
			if (n > max_free)
				max_free = n;
			free_blks++;
		}
	}
	
	PRINTK("Free pages:%8d (%dkB), %%%lu frag, %%%lu slack\n",
			nr_free_pages, nr_free_pages << (PAGE_SHIFT-10),
			(free_blks * 100) / nr_free_pages, slack);
	PRINTK("Free blks: %8lu min=%lu max=%lu avg=%lu\n",
			free_blks, min_free, max_free, avg_free / free_blks);
	PRINTK("Used blks: %8lu min=%lu max=%lu avg=%lu\n",
			used_blks, min_used, max_used, avg_used / used_blks);
	
	restore_flags(flags);
}

/****************************************************************************/

void
show_free_areas(void)
{
	print_free_areas(NULL);
#ifdef SWAP_CACHE_INFO
	show_swap_cache_info();
#endif	
}

void
page_alloc_stats(char *buffer)
{
	print_free_areas(buffer);
}

/****************************************************************************/
/*
 * set up the memory_map:
 *   - mark all pages reserved
 *   - mark all memory queues empty
 */

#define LONG_ALIGN(x) (((x)+(sizeof(long))-1)&~((sizeof(long))-1))

unsigned long
free_area_init(unsigned long start_mem, unsigned long end_mem)
{
	mem_map_t * p;
	int i;

	/*
	 * select nr of pages we try to keep free for important stuff
	 * with a minimum of 48 pages. This is totally arbitrary
	 */
	i = (end_mem - PAGE_OFFSET) >> (PAGE_SHIFT+7);
	if (i < 24)
		i = 24;
	i += 24;   /* The limit for buffer pages in __get_free_pages is
	   	    * decreased by 12+(i>>3) */
	min_free_pages = i;
	free_pages_low = i + (i>>1);
	free_pages_high = i + i;
/*
 *	make some space for the mem_map and our bit_map
 */
	bit_map = (char *) LONG_ALIGN(start_mem);
	bit_map_size = MAP_NR(end_mem);

	i = LONG_ALIGN(MAP_NR(end_mem) / 8);		/* how many bytes of bits */
	mem_map = (mem_map_t *) (bit_map + i);
	p = mem_map + MAP_NR(end_mem);
	start_mem = LONG_ALIGN((unsigned long) p);
	memset(mem_map, 0, start_mem - (unsigned long) mem_map);
	do {
		--p;
		p->flags = (1 << PG_DMA) | (1 << PG_reserved);
		p->map_nr = p - mem_map;
		set_bit(p->map_nr, bit_map);
	} while (p > mem_map);
/*
 *	as we free pages we mark the first page that is usable
 */
	first_usable_page = MAP_NR(end_mem);

	return start_mem;
}

/****************************************************************************/
