#
# Makefile for the linux memory manager.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := mmnommu.o
obj-y	 := bootmem.o filemap.o mlock.o mmap.o \
			mprotect.o mremap.o numa.o \
			slab.o swapfile.o vmalloc.o vmscan.o memory.o \
			swap.o oom_kill.o

ifdef CONFIG_CONTIGUOUS_PAGE_ALLOC
obj-y    += page_alloc2.o
else
obj-y    += page_alloc.o
endif

export-objs := filemap.o memory.o page_alloc.o page_alloc2.o

include $(TOPDIR)/Rules.make
