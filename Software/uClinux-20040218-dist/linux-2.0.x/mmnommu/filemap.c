/*
 *	linux/mm/filemap.c
 *
 * Copyright (C) 1994, 1995  Linus Torvalds
 *
 * uClinux revisions
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 1999,2000  D. Jeff Dionne <jeff@uclinux.org>,
 *                          Rt-Control, Inc./ Lineo, Inc.
 */

/*
 * This file handles the generic file mmap semantics used by
 * most "normal" filesystems (but you don't /have/ to use this:
 * the NFS filesystem does this differently, for example)
 */
#include <linux/config.h> /* CONFIG_READA_SMALL */
#include <linux/stat.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/shm.h>
#include <linux/errno.h>
#include <linux/mman.h>
#include <linux/string.h>
#include <linux/malloc.h>
#include <linux/fs.h>
#include <linux/locks.h>
#include <linux/pagemap.h>
#include <linux/swap.h>

#include <asm/segment.h>
#include <asm/system.h>
#include <asm/pgtable.h>

/*
 * Shared mappings implemented 30.11.1994. It's not fully working yet,
 * though.
 *
 * Shared mappings now work. 15.8.1995  Bruno.
 */

unsigned long page_cache_size = 0;
struct page * page_hash_table[PAGE_HASH_SIZE];

/*
 * Simple routines for both non-shared and shared mappings.
 */

#define release_page(page) __free_page((page))

/*
 * Invalidate the pages of an inode, removing all pages that aren't
 * locked down (those are sure to be up-to-date anyway, so we shouldn't
 * invalidate them).
 */
void invalidate_inode_pages(struct inode * inode)
{
	struct page ** p;
	struct page * page;

	p = &inode->i_pages;
	while ((page = *p) != NULL) {
		if (PageLocked(page)) {
			p = &page->next;
			continue;
		}
		inode->i_nrpages--;
		if ((*p = page->next) != NULL)
			(*p)->prev = page->prev;
		page->dirty = 0;
		page->next = NULL;
		page->prev = NULL;
		remove_page_from_hash_queue(page);
		page->inode = NULL;
		__free_page(page);
		continue;
	}
}

/*
 * Truncate the page cache at a set offset, removing the pages
 * that are beyond that offset (and zeroing out partial pages).
 */
void truncate_inode_pages(struct inode * inode, unsigned long start)
{
	struct page ** p;
	struct page * page;

repeat:
	p = &inode->i_pages;
	while ((page = *p) != NULL) {
		unsigned long offset = page->offset;

		/* page wholly truncated - free it */
		if (offset >= start) {
			if (PageLocked(page)) {
				__wait_on_page(page);
				goto repeat;
			}
			inode->i_nrpages--;
			if ((*p = page->next) != NULL)
				(*p)->prev = page->prev;
			page->dirty = 0;
			page->next = NULL;
			page->prev = NULL;
			remove_page_from_hash_queue(page);
			page->inode = NULL;
			__free_page(page);
			continue;
		}
		p = &page->next;
		offset = start - offset;
		/* partial truncate, clear end of page */
		if (offset < PAGE_SIZE) {
			unsigned long address = page_address(page);
			memset((void *) (offset + address), 0, PAGE_SIZE - offset);
			flush_page_to_ram(address);
		}
	}
}

/*
 * This is called from try_to_swap_out() when we try to get rid of some
 * pages..  If we're unmapping the last occurrence of this page, we also
 * free it from the page hash-queues etc, as we don't want to keep it
 * in-core unnecessarily.
 */
unsigned long page_unuse(unsigned long page)
{
	struct page * p = mem_map + MAP_NR(page);
	int count = p->count;

	if (count != 2)
		return count;
	if (!p->inode)
		return count;
	remove_page_from_hash_queue(p);
	remove_page_from_inode_queue(p);
	free_page(page);
	return 1;
}

/*
 * Update a page cache copy, when we're doing a "write()" system call
 * See also "update_vm_cache()".
 */
void update_vm_cache(struct inode * inode, unsigned long pos, const char * buf, int count)
{
	unsigned long offset, len;

	offset = (pos & ~PAGE_MASK);
	pos = pos & PAGE_MASK;
	len = PAGE_SIZE - offset;
	do {
		struct page * page;

		if (len > count)
			len = count;
		page = find_page(inode, pos);
		if (page) {
			wait_on_page(page);
			memcpy((void *) (offset + page_address(page)), buf, len);
			release_page(page);
		}
		count -= len;
		buf += len;
		len = PAGE_SIZE;
		offset = 0;
		pos += PAGE_SIZE;
	} while (count);
}

static inline void add_to_page_cache(struct page * page,
	struct inode * inode, unsigned long offset,
	struct page **hash)
{
	page->count++;
	page->flags &= ~((1 << PG_uptodate) | (1 << PG_error));
	page->offset = offset;
	add_page_to_inode_queue(inode, page);
	__add_page_to_hash_queue(page, hash);
}

/*
 * Try to read ahead in the file. "page_cache" is a potentially free page
 * that we could use for the cache (if it is 0 we can try to create one,
 * this is all overlapped with the IO on the previous page finishing anyway)
 */
static unsigned long try_to_read_ahead(struct inode * inode, unsigned long offset, unsigned long page_cache)
{
	struct page * page;
	struct page ** hash;

	offset &= PAGE_MASK;
	switch (page_cache) {
	case 0:
		page_cache = __get_free_page(GFP_KERNEL);
		if (!page_cache)
			break;
	default:
		if (offset >= inode->i_size)
			break;
		hash = page_hash(inode, offset);
		page = __find_page(inode, offset, *hash);
		if (!page) {
			/*
			 * Ok, add the new page to the hash-queues...
			 */
			page = mem_map + MAP_NR(page_cache);
			add_to_page_cache(page, inode, offset, hash);
			inode->i_op->readpage(inode, page);
			page_cache = 0;
		}
		release_page(page);
	}
	return page_cache;
}

/* 
 * Wait for IO to complete on a locked page.
 *
 * This must be called with the caller "holding" the page,
 * ie with increased "page->count" so that the page won't
 * go away during the wait..
 */
void __wait_on_page(struct page *page)
{
	struct wait_queue wait = { current, NULL };

	add_wait_queue(&page->wait, &wait);
repeat:
	run_task_queue(&tq_disk);
	current->state = TASK_UNINTERRUPTIBLE;
	if (PageLocked(page)) {
		schedule();
		goto repeat;
	}
	remove_wait_queue(&page->wait, &wait);
	current->state = TASK_RUNNING;
}

#if 0
#define PROFILE_READAHEAD
#define DEBUG_READAHEAD
#endif

/*
 * Read-ahead profiling information
 * --------------------------------
 * Every PROFILE_MAXREADCOUNT, the following information is written 
 * to the syslog:
 *   Percentage of asynchronous read-ahead.
 *   Average of read-ahead fields context value.
 * If DEBUG_READAHEAD is defined, a snapshot of these fields is written 
 * to the syslog.
 */

#ifdef PROFILE_READAHEAD

#define PROFILE_MAXREADCOUNT 1000

static unsigned long total_reada;
static unsigned long total_async;
static unsigned long total_ramax;
static unsigned long total_ralen;
static unsigned long total_rawin;

static void profile_readahead(int async, struct file *filp)
{
	unsigned long flags;

	++total_reada;
	if (async)
		++total_async;

	total_ramax	+= filp->f_ramax;
	total_ralen	+= filp->f_ralen;
	total_rawin	+= filp->f_rawin;

	if (total_reada > PROFILE_MAXREADCOUNT) {
		save_flags(flags);
		cli();
		if (!(total_reada > PROFILE_MAXREADCOUNT)) {
			restore_flags(flags);
			return;
		}

		printk("Readahead average:  max=%ld, len=%ld, win=%ld, async=%ld%%\n",
			total_ramax/total_reada,
			total_ralen/total_reada,
			total_rawin/total_reada,
			(total_async*100)/total_reada);
#ifdef DEBUG_READAHEAD
		printk("Readahead snapshot: max=%ld, len=%ld, win=%ld, raend=%ld\n",
			filp->f_ramax, filp->f_ralen, filp->f_rawin, filp->f_raend);
#endif

		total_reada	= 0;
		total_async	= 0;
		total_ramax	= 0;
		total_ralen	= 0;
		total_rawin	= 0;

		restore_flags(flags);
	}
}
#endif  /* defined PROFILE_READAHEAD */

/*
 * Read-ahead context:
 * -------------------
 * The read ahead context fields of the "struct file" are the following:
 * - f_raend : position of the first byte after the last page we tried to
 *             read ahead.
 * - f_ramax : current read-ahead maximum size.
 * - f_ralen : length of the current IO read block we tried to read-ahead.
 * - f_rawin : length of the current read-ahead window.
 *             if last read-ahead was synchronous then
 *                  f_rawin = f_ralen
 *             otherwise (was asynchronous)
 *                  f_rawin = previous value of f_ralen + f_ralen
 *
 * Read-ahead limits:
 * ------------------
 * MIN_READAHEAD   : minimum read-ahead size when read-ahead.
 * MAX_READAHEAD   : maximum read-ahead size when read-ahead.
 *
 * Synchronous read-ahead benefits:
 * --------------------------------
 * Using reasonable IO xfer length from peripheral devices increase system 
 * performances.
 * Reasonable means, in this context, not too large but not too small.
 * The actual maximum value is:
 *	MAX_READAHEAD + PAGE_SIZE = 76k is CONFIG_READA_SMALL is undefined
 *      and 32K if defined (4K page size assumed).
 *
 * Asynchronous read-ahead benefits:
 * ---------------------------------
 * Overlapping next read request and user process execution increase system 
 * performance.
 *
 * Read-ahead risks:
 * -----------------
 * We have to guess which further data are needed by the user process.
 * If these data are often not really needed, it's bad for system 
 * performances.
 * However, we know that files are often accessed sequentially by 
 * application programs and it seems that it is possible to have some good 
 * strategy in that guessing.
 * We only try to read-ahead files that seems to be read sequentially.
 *
 * Asynchronous read-ahead risks:
 * ------------------------------
 * In order to maximize overlapping, we must start some asynchronous read 
 * request from the device, as soon as possible.
 * We must be very careful about:
 * - The number of effective pending IO read requests.
 *   ONE seems to be the only reasonable value.
 * - The total memory pool usage for the file access stream.
 *   This maximum memory usage is implicitly 2 IO read chunks:
 *   2*(MAX_READAHEAD + PAGE_SIZE) = 156K if CONFIG_READA_SMALL is undefined,
 *   64k if defined (4K page size assumed).
 */

#define PageAlignSize(size) (((size) + PAGE_SIZE -1) & PAGE_MASK)

#ifdef CONFIG_READA_SMALL  /* small readahead */
#define MAX_READAHEAD PageAlignSize(4096*7)
#define MIN_READAHEAD PageAlignSize(4096*2)
#else /* large readahead */
#define MAX_READAHEAD PageAlignSize(4096*18)
#define MIN_READAHEAD PageAlignSize(4096*3)
#endif

static inline unsigned long generic_file_readahead(int reada_ok, struct file * filp, struct inode * inode,
	unsigned long ppos, struct page * page,
	unsigned long page_cache)
{
	unsigned long max_ahead, ahead;
	unsigned long raend;

	raend = filp->f_raend & PAGE_MASK;
	max_ahead = 0;

/*
 * The current page is locked.
 * If the current position is inside the previous read IO request, do not
 * try to reread previously read ahead pages.
 * Otherwise decide or not to read ahead some pages synchronously.
 * If we are not going to read ahead, set the read ahead context for this 
 * page only.
 */
	if (PageLocked(page)) {
		if (!filp->f_ralen || ppos >= raend || ppos + filp->f_ralen < raend) {
			raend = ppos;
			if (raend < inode->i_size)
				max_ahead = filp->f_ramax;
			filp->f_rawin = 0;
			filp->f_ralen = PAGE_SIZE;
			if (!max_ahead) {
				filp->f_raend  = ppos + filp->f_ralen;
				filp->f_rawin += filp->f_ralen;
			}
		}
	}
/*
 * The current page is not locked.
 * If we were reading ahead and,
 * if the current max read ahead size is not zero and,
 * if the current position is inside the last read-ahead IO request,
 *   it is the moment to try to read ahead asynchronously.
 * We will later force unplug device in order to force asynchronous read IO.
 */
	else if (reada_ok && filp->f_ramax && raend >= PAGE_SIZE &&
	         ppos <= raend && ppos + filp->f_ralen >= raend) {
/*
 * Add ONE page to max_ahead in order to try to have about the same IO max size
 * as synchronous read-ahead (MAX_READAHEAD + 1)*PAGE_SIZE.
 * Compute the position of the last page we have tried to read in order to 
 * begin to read ahead just at the next page.
 */
		raend -= PAGE_SIZE;
		if (raend < inode->i_size)
			max_ahead = filp->f_ramax + PAGE_SIZE;

		if (max_ahead) {
			filp->f_rawin = filp->f_ralen;
			filp->f_ralen = 0;
			reada_ok      = 2;
		}
	}
/*
 * Try to read ahead pages.
 * We hope that ll_rw_blk() plug/unplug, coalescence, requests sort and the
 * scheduler, will work enough for us to avoid too bad actuals IO requests.
 */
	ahead = 0;
	while (ahead < max_ahead) {
		ahead += PAGE_SIZE;
		page_cache = try_to_read_ahead(inode, raend + ahead, page_cache);
	}
/*
 * If we tried to read ahead some pages,
 * If we tried to read ahead asynchronously,
 *   Try to force unplug of the device in order to start an asynchronous
 *   read IO request.
 * Update the read-ahead context.
 * Store the length of the current read-ahead window.
 * Double the current max read ahead size.
 *   That heuristic avoid to do some large IO for files that are not really
 *   accessed sequentially.
 */
	if (ahead) {
		if (reada_ok == 2) {
			run_task_queue(&tq_disk);
		}

		filp->f_ralen += ahead;
		filp->f_rawin += filp->f_ralen;
		filp->f_raend = raend + ahead + PAGE_SIZE;

		filp->f_ramax += filp->f_ramax;

		if (filp->f_ramax > MAX_READAHEAD)
			filp->f_ramax = MAX_READAHEAD;

#ifdef PROFILE_READAHEAD
		profile_readahead((reada_ok == 2), filp);
#endif
	}

	return page_cache;
}


/*
 * This is a generic file read routine, and uses the
 * inode->i_op->readpage() function for the actual low-level
 * stuff.
 *
 * This is really ugly. But the goto's actually try to clarify some
 * of the logic when it comes to error handling etc.
 */

int generic_file_read(struct inode * inode, struct file * filp, char * buf, int count)
{
	int error, read;
	unsigned long pos, ppos, page_cache;
	int reada_ok;

	error = 0;
	read = 0;
	page_cache = 0;

	pos = filp->f_pos;
	ppos = pos & PAGE_MASK;

#ifdef MAGIC_ROM_PTR
	/* Logic: if romptr f_op is available, try to get a pointer into ROM
	 * for the data, bypassing the buffer cache entirely. This is only a
	 * win if the ROM is reasonably fast, of course.
	 *
	 * Note that this path only requires that the pointer (and the data
	 * it points to) to be valid until the memcpy_tofs is complete.
	 *
	 *	-- Kenneth Albanowski
	 */
	   
        if (filp->f_op->romptr) {
                struct vm_area_struct vma;

		/* the romptr routine below returns 0 even if
		 * we are off the end of the file,  so don't even
		 * try if we are past the end,  otherwise user apps
		 * read past the end of the file.
		 */
		if (pos >= inode->i_size)
			return(0);

                vma.vm_start = 0;
                vma.vm_offset = pos;
                vma.vm_flags = VM_READ;
                if (!filp->f_op->romptr(inode, filp, &vma)) {
			if (count > inode->i_size - pos)
				count = inode->i_size - pos;
                        memcpy_tofs(buf, (void*)vma.vm_start, count);
                        filp->f_pos += count;
                        return count;
                }
	}
#endif /* MAGIC_ROM_PTR */

/*
 * If the current position is outside the previous read-ahead window, 
 * we reset the current read-ahead context and set read ahead max to zero
 * (will be set to just needed value later),
 * otherwise, we assume that the file accesses are sequential enough to
 * continue read-ahead.
 */
	if (ppos > filp->f_raend || ppos + filp->f_rawin < filp->f_raend) {
		reada_ok = 0;
		filp->f_raend = 0;
		filp->f_ralen = 0;
		filp->f_ramax = 0;
		filp->f_rawin = 0;
	} else {
		reada_ok = 1;
	}
/*
 * Adjust the current value of read-ahead max.
 * If the read operation stay in the first half page, force no readahead.
 * Otherwise try to increase read ahead max just enough to do the read request.
 * Then, at least MIN_READAHEAD if read ahead is ok,
 * and at most MAX_READAHEAD in all cases.
 */
	if (pos + count <= (PAGE_SIZE >> 1)) {
		filp->f_ramax = 0;
	} else {
		unsigned long needed;

		needed = ((pos + count) & PAGE_MASK) - ppos;

		if (filp->f_ramax < needed)
			filp->f_ramax = needed;

		if (reada_ok && filp->f_ramax < MIN_READAHEAD)
				filp->f_ramax = MIN_READAHEAD;
		if (filp->f_ramax > MAX_READAHEAD)
			filp->f_ramax = MAX_READAHEAD;
	}

	for (;;) {
		struct page *page, **hash;

		if (pos >= inode->i_size)
			break;

		/*
		 * Try to find the data in the page cache..
		 */
		hash = page_hash(inode, pos & PAGE_MASK);
		page = __find_page(inode, pos & PAGE_MASK, *hash);
		if (!page)
			goto no_cached_page;

found_page:
/*
 * Try to read ahead only if the current page is filled or being filled.
 * Otherwise, if we were reading ahead, decrease max read ahead size to
 * the minimum value.
 * In this context, that seems to may happen only on some read error or if 
 * the page has been rewritten.
 */
		if (PageUptodate(page) || PageLocked(page))
			page_cache = generic_file_readahead(reada_ok, filp, inode, pos & PAGE_MASK, page, page_cache);
		else if (reada_ok && filp->f_ramax > MIN_READAHEAD)
				filp->f_ramax = MIN_READAHEAD;

		wait_on_page(page);

		if (!PageUptodate(page))
			goto page_read_error;

success:
		/*
		 * Ok, we have the page, it's up-to-date and ok,
		 * so now we can finally copy it to user space...
		 */
	{
		unsigned long offset, nr;
		offset = pos & ~PAGE_MASK;
		nr = PAGE_SIZE - offset;
		if (nr > count)
			nr = count;

		if (nr > inode->i_size - pos)
			nr = inode->i_size - pos;
		memcpy_tofs(buf, (void *) (page_address(page) + offset), nr);
		release_page(page);
		buf += nr;
		pos += nr;
		read += nr;
		count -= nr;
		if (count) {
			/*
			 * to prevent hogging the CPU on well-cached systems,
			 * schedule if needed, it's safe to do it here:
			 */
			if (need_resched)
				schedule();
			continue;
		}
		break;
	}

no_cached_page:
		/*
		 * Ok, it wasn't cached, so we need to create a new
		 * page..
		 */
		if (!page_cache) {
			page_cache = __get_free_page(GFP_KERNEL);
			/*
			 * That could have slept, so go around to the
			 * very beginning..
			 */
			if (page_cache)
				continue;
			error = -ENOMEM;
			break;
		}

		/*
		 * Ok, add the new page to the hash-queues...
		 */
		page = mem_map + MAP_NR(page_cache);
		page_cache = 0;
		add_to_page_cache(page, inode, pos & PAGE_MASK, hash);

		/*
		 * Error handling is tricky. If we get a read error,
		 * the cached page stays in the cache (but uptodate=0),
		 * and the next process that accesses it will try to
		 * re-read it. This is needed for NFS etc, where the
		 * identity of the reader can decide if we can read the
		 * page or not..
		 */
/*
 * We have to read the page.
 * If we were reading ahead, we had previously tried to read this page,
 * That means that the page has probably been removed from the cache before 
 * the application process needs it, or has been rewritten.
 * Decrease max readahead size to the minimum value in that situation.
 */
		if (reada_ok && filp->f_ramax > MIN_READAHEAD)
			filp->f_ramax = MIN_READAHEAD;

		error = inode->i_op->readpage(inode, page);
		if (!error)
			goto found_page;
		release_page(page);
		break;

page_read_error:
		/*
		 * We found the page, but it wasn't up-to-date.
		 * Try to re-read it _once_. We do this synchronously,
		 * because this happens only if there were errors.
		 */
		error = inode->i_op->readpage(inode, page);
		if (!error) {
			wait_on_page(page);
			if (PageUptodate(page) && !PageError(page))
				goto success;
			error = -EIO; /* Some unspecified error occurred.. */
		}
		release_page(page);
		break;
	}

	filp->f_pos = pos;
	filp->f_reada = 1;
	if (page_cache)
		free_page(page_cache);
	UPDATE_ATIME(inode)
	if (!read)
		read = error;
	return read;
}

int shrink_mmap(int priority, int dma, int free_buf)
{
	static int clock = 0;
	struct page * page;
	unsigned long limit = MAP_NR(high_memory);
	struct buffer_head *tmp, *bh;
	int count_max, count_min;

	count_max = (limit<<1) >> (priority>>1);
	count_min = (limit<<1) >> (priority);

	page = mem_map + clock;
	
	do {
		count_max--;
		if (page->inode || page->buffers)
			count_min--;

		if (PageLocked(page))
			goto next;
		if (dma && !PageDMA(page))
			goto next;
		/* First of all, regenerate the page's referenced bit
                   from any buffers in the page */
		bh = page->buffers;
		if (bh) {
			tmp = bh;
			do {
				if (buffer_touched(tmp)) {
					clear_bit(BH_Touched, &tmp->b_state);
					set_bit(PG_referenced, &page->flags);
				}
				tmp = tmp->b_this_page;
			} while (tmp != bh);
		}

		/* We can't throw away shared pages, but we do mark
		   them as referenced.  This relies on the fact that
		   no page is currently in both the page cache and the
		   buffer cache; we'd have to modify the following
		   test to allow for that case. */

		switch (page->count) {
			case 1:
				/* If it has been referenced recently, don't free it */
				if (clear_bit(PG_referenced, &page->flags)) {
					/* age this page potential used */
					if (priority < 4)
						age_page(page);
					break;
				}
				
				/* is it a page cache page? */
				if (page->inode) {
					remove_page_from_hash_queue(page);
					remove_page_from_inode_queue(page);
					__free_page(page);
					return 1;
				}
                                
				/* is it a buffer cache page? */
				if (free_buf && bh && try_to_free_buffer(bh, &bh, 6))
					return 1;
				break;

			default:
				/* more than one users: we can't throw it away */
				set_bit(PG_referenced, &page->flags);
				/* fall through */
			case 0:
				/* nothing */
		}
next:
		page++;
		clock++;
		if (clock >= limit) {
			clock = 0;
			page = mem_map;
		}
	} while (count_max > 0 && count_min > 0);
	return 0;
}

asmlinkage int sys_msync(unsigned long start, size_t len, int flags)
{
	return 0;
}

int generic_file_mmap(struct inode * inode, struct file * file, struct vm_area_struct * vma)
{
	return -ENOSYS;
}
