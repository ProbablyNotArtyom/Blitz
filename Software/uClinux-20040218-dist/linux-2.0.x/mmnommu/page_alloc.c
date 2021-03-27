/*
 *  linux/mm/page_alloc.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *  Swap reorganised 29.12.95, Stephen Tweedie
 */

/*
 * uClinux revisions for NO_MM
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 1999,2000  D. Jeff Dionne <jeff@uclinux.org>,
 *                          Rt-Control, Inc. /Lineo Inc.
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
#include <linux/interrupt.h>

#include <asm/dma.h>
#include <asm/system.h> /* for cli()/sti() */
#include <asm/segment.h> /* for memcpy_to/fromfs */
#include <asm/bitops.h>
#include <asm/pgtable.h>

int nr_swap_pages = 0;
int nr_free_pages = 0;

extern struct wait_queue *buffer_wait;

/*
 * Free area management
 *
 * The free_area_list arrays point to the queue heads of the free areas
 * of different sizes
 */

#ifdef BIGALLOCS
#define NR_MEM_LISTS 11
#else
#define NR_MEM_LISTS 9
#endif

/* The start of this MUST match the start of "struct page" */
struct free_area_struct {
	struct page *next;
	struct page *prev;
	unsigned int * map;
};

#define memory_head(x) ((struct page *)(x))

static struct free_area_struct free_area[NR_MEM_LISTS];

static inline void init_mem_queue(struct free_area_struct * head)
{
	head->next = memory_head(head);
	head->prev = memory_head(head);
}

static inline void add_mem_queue(struct free_area_struct * head, struct page * entry)
{
	struct page * next = head->next;

	entry->prev = memory_head(head);
	entry->next = next;
	next->prev = entry;
	head->next = entry;
}

static inline void remove_mem_queue(struct page * entry)
{
	struct page * next = entry->next;
	struct page * prev = entry->prev;
	next->prev = prev;
	prev->next = next;
}

/*
 * Free_page() adds the page to the free lists. This is optimized for
 * fast normal cases (no error jumps taken normally).
 *
 * The way to optimize jumps for gcc-2.2.2 is to:
 *  - select the "normal" case and put it inside the if () { XXX }
 *  - no else-statements if you can avoid them
 *
 * With the above two rules, you get a straight-line execution path
 * for the normal case, giving better asm-code.
 *
 * free_page() may sleep since the page being freed may be a buffer
 * page or present in the swap cache. It will not sleep, however,
 * for a freshly allocated page (get_free_page()).
 */

/*
 * Buddy system. Hairy. You really aren't expected to understand this
 *
 * Hint: -mask = 1+~mask
 */
static inline void free_pages_ok(unsigned long map_nr, unsigned long order)
{
	struct free_area_struct *area = free_area + order;
	unsigned long index = map_nr >> (1 + order);
	unsigned long mask = (~0UL) << order;
	unsigned long flags;

	save_flags(flags);
	cli();

#define list(x) (mem_map+(x))

	map_nr &= mask;
	nr_free_pages -= mask;
	while (mask + (1 << (NR_MEM_LISTS-1))) {
		if (!change_bit(index, area->map))
			break;
		remove_mem_queue(list(map_nr ^ -mask));
		mask <<= 1;
		area++;
		index >>= 1;
		map_nr &= mask;
	}
	add_mem_queue(area, list(map_nr));

#undef list

	restore_flags(flags);
	if (!waitqueue_active(&buffer_wait))
		return;
	wake_up(&buffer_wait);
}

#ifdef DEBUG_FREE_PAGES

#undef __free_page
void __free_page_flf(struct page *page, char*file, int line, char*function)
{
	printk("Freeing page %p from %s @%s:%d\n", page, function, file, line);
	__free_page(page);
}

#undef free_pages
void free_pages_flf(unsigned long addr, unsigned long order, char*file, int line, char*function)
{
	printk("Freeing %lu byte page %lx from %s @%s:%d\n", 4096 << order, addr, function, file, line);
	free_pages(addr, order);
}

#undef __get_free_pages
unsigned long __get_free_pages_flf(int priority, unsigned long order, int dma, char * file, int line, char * function)
{
	printk("Allocating %d byte page from %s @%s:%d\n", 4096 << order, function, file, line);
	return __get_free_pages(priority, order, dma);
}

#undef get_free_page
unsigned long get_free_page_flf(int priority, char * file, int line, char * function)
{
	void * result = (void*)__get_free_pages_flf(priority, 0, 0, file, line, function);
	if (result)
		memset(result, 0, PAGE_SIZE);
	return result;
}

#endif /* DEBUG_FREE_PAGES */

void __free_page(struct page *page)
{
	if (!PageReserved(page) && atomic_dec_and_test(&page->count)) {
		unsigned long map_nr = page->map_nr;

		free_pages_ok(map_nr, 0);
	}
}

void free_pages(unsigned long addr, unsigned long order)
{
	unsigned long map_nr = MAP_NR(addr);

	if (map_nr < MAP_NR(high_memory)) {
		mem_map_t * map = mem_map + map_nr;
		if (PageReserved(map))
			return;
		if (atomic_dec_and_test(&map->count)) {

			free_pages_ok(map_nr, order);
			return;
		}
	}
}

/*
 * Some ugly macros to speed up __get_free_pages()..
 */
#define MARK_USED(index, order, area) \
	change_bit((index) >> (1+(order)), (area)->map)
#define CAN_DMA(x) (PageDMA(x))
#define ADDRESS(x) (PAGE_OFFSET + ((x) << PAGE_SHIFT))
#define RMQUEUE(order, dma) \
do { struct free_area_struct * area = free_area+order; \
     unsigned long new_order = order; \
	do { struct page *prev = memory_head(area), *ret; \
		while (memory_head(area) != (ret = prev->next)) { \
			if (!dma || CAN_DMA(ret)) { \
				unsigned long map_nr = ret->map_nr; \
				(prev->next = ret->next)->prev = prev; \
				MARK_USED(map_nr, new_order, area); \
				nr_free_pages -= 1 << order; \
				EXPAND(ret, map_nr, order, new_order, area); \
				restore_flags(flags); \
				return ADDRESS(map_nr); \
			} \
			prev = ret; \
		} \
		new_order++; area++; \
	} while (new_order < NR_MEM_LISTS); \
} while (0)

#define EXPAND(map,index,low,high,area) \
do { unsigned long size = 1 << high; \
	while (high > low) { \
		area--; high--; size >>= 1; \
		add_mem_queue(area, map); \
		MARK_USED(index, high, area); \
		index += size; \
		map += size; \
	} \
	map->count = 1; \
	map->age = PAGE_INITIAL_AGE; \
} while (0)

unsigned long __get_free_pages(int priority, unsigned long order, int dma)
{
	unsigned long flags;
	int reserved_pages;

	if (order >= NR_MEM_LISTS)
		return 0;
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
	if ((priority==GFP_ATOMIC) || nr_free_pages > reserved_pages) {
		RMQUEUE(order, dma);
		restore_flags(flags);
#if 0
		printk("fragmentation preventing allocation, re-attempting to free\n");
#endif
	}
	restore_flags(flags);
	if (priority != GFP_BUFFER && try_to_free_page(priority, dma, 1))
		goto repeat;
	return 0;
}

/*
 * Show free area list (used inside shift_scroll-lock stuff)
 * We also calculate the percentage fragmentation. We do this by counting the
 * memory on each free list with the exception of the first item on the list.
 */
 
/* 
 * That's as may be, but I added an explicit fragmentation percentage, just
 * to make it obvious. -kja
 */
 
/* totals held by do_mmap to compute memory wastage */
unsigned long realalloc, askedalloc;

static void print_free_areas(char *buffer)
{
 	unsigned long order, flags;
 	unsigned long total = 0;
 	unsigned long fragmented = 0;
 	unsigned long slack;

#define	PRINTK(fmt, a...) \
	(buffer ? sprintf(&buffer[strlen(buffer)], fmt, ##a) : printk(fmt, ##a))

	PRINTK("Free pages:%8dkB\n ( ",nr_free_pages<<(PAGE_SHIFT-10));
	save_flags(flags);
	cli();
 	for (order=0 ; order < NR_MEM_LISTS; order++) {
		struct page * tmp;
		unsigned long nr = 0;
		for (tmp = free_area[order].next ; tmp != memory_head(free_area+order) ; tmp = tmp->next) {
			nr ++;
		}
		total += nr * ((PAGE_SIZE>>10) << order);
		if ((nr > 1) && (order < (NR_MEM_LISTS-1)))
			fragmented += (nr-1) * (1 << order);
		PRINTK("%lu*%lukB ", nr, (PAGE_SIZE>>10) << order);
	}
	restore_flags(flags);
	fragmented *= 100;
	fragmented /= nr_free_pages;
	
	if (realalloc)
		slack = (realalloc-askedalloc) * 100 / realalloc;
	else
		slack = 0;
	
	PRINTK("= %lukB, %%%lu frag, %%%lu slack)\n", total, fragmented, slack);
}

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

#define LONG_ALIGN(x) (((x)+(sizeof(long))-1)&~((sizeof(long))-1))

/*
 * set up the free-area data structures:
 *   - mark all pages reserved
 *   - mark all memory queues empty
 *   - clear the memory bitmaps
 */
unsigned long free_area_init(unsigned long start_mem, unsigned long end_mem)
{
	mem_map_t * p;
	unsigned long mask = PAGE_MASK;
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

	mem_map = (mem_map_t *) start_mem;
	p = mem_map + MAP_NR(end_mem);
	start_mem = LONG_ALIGN((unsigned long) p);
	memset(mem_map, 0, start_mem - (unsigned long) mem_map);
	do {
		--p;
		p->flags = (1 << PG_DMA) | (1 << PG_reserved);
		p->map_nr = p - mem_map;
	} while (p > mem_map);

	for (i = 0 ; i < NR_MEM_LISTS ; i++) {
		unsigned long bitmap_size;
		init_mem_queue(free_area+i);
		mask += mask;
		end_mem = (end_mem + ~mask) & mask;
		bitmap_size = (end_mem - PAGE_OFFSET) >> (PAGE_SHIFT + i);
		bitmap_size = (bitmap_size + 7) >> 3;
		bitmap_size = LONG_ALIGN(bitmap_size);
		free_area[i].map = (unsigned int *) start_mem;
		memset((void *) start_mem, 0, bitmap_size);
		start_mem += bitmap_size;
	}
	return start_mem;
}
