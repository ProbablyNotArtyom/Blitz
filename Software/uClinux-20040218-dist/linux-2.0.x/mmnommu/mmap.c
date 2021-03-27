/*
 *	linux/mm/mmap.c
 *
 * Written by obz.
 */

/*
 * uClinux revisions for NO_MM
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 1999, 2000   D. Jeff Dionne <jeff@uclinux.org>,
 *                            Rt-Control, Inc. / Lineo Inc.
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
#include <linux/pagemap.h>
#include <linux/swap.h>

#include <asm/segment.h>
#include <asm/system.h>
#include <asm/pgtable.h>

/* If defined, warn whenever someone mmaps a block that has more then this
   number of bytes in slack space (due to kmalloc granularity). */

/*#define WARN_ON_SLACK 1024*/

/*
 * Combine the mmap "prot" and "flags" argument into one "vm_flags" used
 * internally. Essentially, translate the "PROT_xxx" and "MAP_xxx" bits
 * into "VM_xxx".
 */
static inline unsigned long vm_flags(unsigned long prot, unsigned long flags)
{
#define _trans(x,bit1,bit2) \
((bit1==bit2)?(x&bit1):(x&bit1)?bit2:0)

	unsigned long prot_bits, flag_bits;
	prot_bits =
		_trans(prot, PROT_READ, VM_READ) |
		_trans(prot, PROT_WRITE, VM_WRITE) |
		_trans(prot, PROT_EXEC, VM_EXEC);
	flag_bits =
		_trans(flags, MAP_GROWSDOWN, VM_GROWSDOWN) |
		_trans(flags, MAP_DENYWRITE, VM_DENYWRITE) |
		_trans(flags, MAP_EXECUTABLE, VM_EXECUTABLE);
	return prot_bits | flag_bits;
#undef _trans
}

/*
 *	use any slack space at the end of the process space due to memory
 *	allocator inefficiencies to implement a limited brk() function and
 *	maybe save some memory by using what we already have.
 *
 *	This does require a user space malloc that knows about brk() but
 *	doesn't rely on it exclusively.
 *
 *		davidm@lineo.com
 */

asmlinkage unsigned long sys_brk(unsigned long brk)
{
#ifdef maybe_later
	unsigned long rlim;
#endif
	struct mm_struct *mm = current->mm;

	if (brk < mm->end_code || brk < mm->start_brk || brk > mm->end_brk)
		return mm->brk;

	if (mm->brk == brk)
		return mm->brk;

	/*
	 * Always allow shrinking brk
	 */
	if (brk <= mm->brk) {
		mm->brk = brk;
		return brk;
	}

#ifdef maybe_later
	/*
	 * Check against rlimit and stack..
	 */
	rlim = current->rlim[RLIMIT_DATA].rlim_cur;
	if (rlim >= RLIM_INFINITY)
		rlim = ~0;
	if (brk - mm->end_code > rlim)
		return mm->brk;
#endif

	/*
	 * Ok, looks good - let it rip.
	 */
	return mm->brk = brk;
}

#ifdef DEBUG_MMAP

#undef do_mmap
unsigned long do_mmap_flf(struct file * file, unsigned long addr, unsigned long len,
	unsigned long prot, unsigned long flags, unsigned long off, char*filename, int line, char*function)
{
	unsigned long result;
	printk("%s @%s:%d: do_mmap by pid %d of %lu", function, filename, line, current->pid, len);

	result=do_mmap(file,addr,len,prot,flags,off);
	
	if (result >= -4096) /* so to speak... */
		printk(" = %lx\n", result);
	else
	if (is_in_rom(result))
		printk("-%lu = %lx\n", len, result);
	else
	printk("+%lu+%lu+%lu = %lx\n",
		ksize(result)-len,
		ksize(current->mm->tblock.next),
		ksize(current->mm->tblock.next->rblock),
		result);
	return result;
}
#endif /* DEBUG_MMAP */

#ifdef DEBUG
static void show_process_blocks(void)
{
	struct mm_tblock_struct * tblock, *tmp;
	
	printk("Process blocks %d:", current->pid);
	
	tmp = &current->mm->tblock;
	while (tmp) {
		printk(" %p: %p", tmp, tmp->rblock);
		if (tmp->rblock)
			printk(" (%d @%p #%d)", ksize(tmp->rblock->kblock), tmp->rblock->kblock, tmp->rblock->refcount);
		if (tmp->next)
			printk(" ->");
		else
			printk(".");
		tmp = tmp->next;
	}
	printk("\n");
}
#endif /* DEBUG */

extern unsigned long askedalloc, realalloc;

unsigned long do_mmap(struct file * file, unsigned long addr, unsigned long len,
	unsigned long prot, unsigned long flags, unsigned long off)
{
	void * result;
	struct mm_tblock_struct * tblock;

	if ((flags & MAP_SHARED) && (prot & PROT_WRITE) && (file)) {
		printk("MAP_SHARED not supported (cannot write mappings to disk)\n");
		return -EINVAL;
	}
	
	if ((prot & PROT_WRITE) && (flags & MAP_PRIVATE)) {
		printk("Private writable mappings not supported\n");
		return -EINVAL;
	}
	
	/*
	 * determine the object being mapped and call the appropriate
	 * specific mapper. 
	 */

	if (file) {
		struct vm_area_struct vma;
		int error;
		
		
		if (!file->f_op)
			return -ENODEV;

		vma.vm_start = addr;
		vma.vm_end = addr + len;
		vma.vm_flags = vm_flags(prot,flags) /*| mm->def_flags*/;

		if (file->f_mode & 1)
			vma.vm_flags |= VM_MAYREAD | VM_MAYWRITE | VM_MAYEXEC;
		if (flags & MAP_SHARED) {
			vma.vm_flags |= VM_SHARED | VM_MAYSHARE;
			/*
			 * This looks strange, but when we don't have the file open
			 * for writing, we can demote the shared mapping to a simpler
			 * private mapping. That also takes care of a security hole
			 * with ptrace() writing to a shared mapping without write
			 * permissions.
			 *
			 * We leave the VM_MAYSHARE bit on, just to get correct output
			 * from /proc/xxx/maps..
			 */
			if (!(file->f_mode & 2))
				vma.vm_flags &= ~(VM_MAYWRITE | VM_SHARED);
		}
		vma.vm_offset = off;

#ifdef MAGIC_ROM_PTR
		/* First, try simpler routine designed to give us a ROM pointer. */
		
		if (file->f_op->romptr && !(prot & PROT_WRITE)) {
			error = file->f_op->romptr(file->f_inode, file, &vma);
			/*printk("romptr mmap returned %d /%x\n", error, vma.vm_start);*/
					   
			if (!error)
				return vma.vm_start;
			else if (error != -ENOSYS)
				return error;
		} else
#endif /* MAGIC_ROM_PTR */
		/* Then try full mmap routine, which might return a RAM pointer,
		   or do something truly complicated. */
		   
		if (file->f_op->mmap) {
			error = file->f_op->mmap(file->f_inode, file, &vma);
				   
			/*printk("mmap mmap returned %d /%x\n", error, vma.vm_start);*/
			if (!error)
				return vma.vm_start;
			else if (error != -ENOSYS)
				return error;
		} else
			return -ENODEV; /* No mapping operations defined */

		/* An ENOSYS error indicates that mmap isn't possible (as opposed to
		   tried but failed) so we'll fall through to the copy. */
	}

	tblock = (struct mm_tblock_struct *)
                        kmalloc(sizeof(struct mm_tblock_struct), GFP_KERNEL);
	if (!tblock) {
		printk("Allocation of tblock for %lu byte allocation from process %d failed\n", len, current->pid);
		show_buffers();
		show_free_areas();
		return -ENOMEM;
	}

        tblock->rblock = (struct mm_rblock_struct *)
                        kmalloc(sizeof(struct mm_rblock_struct), GFP_KERNEL);

	if (!tblock->rblock) {
		printk("Allocation of rblock for %lu byte allocation from process %d failed\n", len, current->pid);
		show_buffers();
		show_free_areas();
		kfree(tblock);
		return -ENOMEM;
	}

	
	result = kmalloc(len, GFP_KERNEL);
	if (!result) {
		printk("Allocation of length %lu from process %d failed\n", len, current->pid);
		show_buffers();
		show_free_areas();
		kfree(tblock->rblock);
		kfree(tblock);
		return -ENOMEM;
	}

        tblock->rblock->refcount = 1;
        tblock->rblock->kblock = result;
        tblock->rblock->size = len;
	
	realalloc += ksize(result);
	askedalloc += len;

#ifdef WARN_ON_SLACK	
	if ((len+WARN_ON_SLACK) <= ksize(result))
		printk("Allocation of %lu bytes from process %d has %lu bytes of slack\n", len, current->pid, ksize(result)-len);
#endif
	
	if (file) {
		int error;
		int old_fs = get_fs();
        	set_fs(KERNEL_DS);
                error = file->f_op->read(file->f_inode, file, (char*)result, len);
                set_fs(old_fs);
                if (error < 0) {
                	kfree(result);
                	kfree(tblock->rblock);
                	kfree(tblock);
                	return error;
                }
                if (error<len)
	                memset(result+error, '\0', len-error);
        } else {
        	memset(result, '\0', len);
        }

        
        realalloc += ksize(tblock);
        askedalloc += sizeof(struct mm_tblock_struct);
        
        realalloc += ksize(tblock->rblock);
        askedalloc += sizeof(struct mm_rblock_struct);
        
        tblock->next = current->mm->tblock.next;
	current->mm->tblock.next = tblock;

#ifdef DEBUG
        printk("do_mmap:\n");
	show_process_blocks();
#endif	  

	return (unsigned long)result;
}


asmlinkage int sys_munmap(unsigned long addr, size_t len)
{
	return do_munmap(addr, len);
}

#ifdef DEBUG_MMAP
#undef do_munmap
int do_munmap_flf(unsigned long addr, size_t len, char * filename, int line, char*function)
{
	printk("do_munmap of %lx bytes at %x invoked from %s @%s:%d\n", ksize(addr), addr, function, filename, line);
	return do_munmap(addr,len);
}

#endif


/*
 * Munmap is split into 2 main parts -- this part which finds
 * what needs doing, and the areas themselves, which do the
 * work.  This now handles partial unmappings.
 * Jeremy Fitzhardine <jeremy@sw.oz.au>
 */
int do_munmap(unsigned long addr, size_t len)
{
	struct mm_tblock_struct * tblock, *tmp;

#ifdef MAGIC_ROM_PTR
	/* For efficiency's sake, if the pointer is obviously in ROM,
	   don't bother walking the lists to free it */
	if (is_in_rom(addr))
		return 0;
#endif

#ifdef DEBUG
        printk("do_munmap:\n");
#endif

	tmp = &current->mm->tblock;
	while ((tblock=tmp->next) && (tblock->rblock) && (tblock->rblock->kblock != (void*)addr)) 
		tmp = tblock;
		
	if (!tblock) {
	        printk("munmap of non-mmaped memory by process %d (%s): %p\n", current->pid, current->comm, (void*)addr);
	        return -EINVAL;
	}
	if(tblock->rblock)
		if(!--tblock->rblock->refcount) {
			if (tblock->rblock->kblock) {
				realalloc -= ksize(tblock->rblock->kblock);
				askedalloc -= tblock->rblock->size;
				kfree(tblock->rblock->kblock);
			}
			
			realalloc -= ksize(tblock->rblock);
			askedalloc -= sizeof(struct mm_rblock_struct);
			kfree(tblock->rblock);
		}
	tmp->next = tblock->next;
	realalloc -= ksize(tblock);
	askedalloc -= sizeof(struct mm_tblock_struct);
	kfree(tblock);

#ifdef DEBUG
	show_process_blocks();
#endif	  

	return -EINVAL;
}

/* Release all mmaps. */
void exit_mmap(struct mm_struct * mm)
{
        struct mm_tblock_struct * tmp;
        /*unsigned long flags;*/
        
        if (!mm)
                return;
        
	if (mm->executable)
		iput(mm->executable);
	mm->executable = 0;

        /*save_flags(flags); cli();*/
        
        if (mm->count > 1) {
                /*restore_flags(flags);*/
                return;
        }
        
#ifdef DEBUG
        printk("Exit_mmap:\n");
#endif
        
        while((tmp = mm->tblock.next)) {
                if (tmp->rblock) {
                        if (!--tmp->rblock->refcount) {
                                if (tmp->rblock->kblock) {
                                        realalloc -= ksize(tmp->rblock->kblock);
                                        askedalloc -= tmp->rblock->size;
                                        kfree(tmp->rblock->kblock);
                                }
                                realalloc -= ksize(tmp->rblock);
                                askedalloc -= sizeof(struct mm_rblock_struct);
                                kfree(tmp->rblock);
                        }
                        tmp->rblock = 0;
                }
                mm->tblock.next = tmp->next;
                realalloc -= ksize(tmp);
                askedalloc -= sizeof(struct mm_tblock_struct);
                kfree(tmp);
        }

#ifdef DEBUG
	show_process_blocks();
#endif	  

        /*restore_flags(flags);*/

}
