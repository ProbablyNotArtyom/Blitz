#
# Makefile for the uClinux memory manager.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now in the main makefile...

O_TARGET := mm.o
O_OBJS	 := memory.o mmap.o filemap.o mprotect.o mlock.o mremap.o \
		vmalloc.o swap.o vmscan.o swapfile.o

ifdef CONFIG_ALLOC2
	    O_OBJS += kmalloc2.o page_alloc2.o
else
	    O_OBJS += kmalloc.o page_alloc.o
endif

include $(TOPDIR)/Rules.make
