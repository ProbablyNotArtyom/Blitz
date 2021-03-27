# Path to parent kernel include files directory
KERNEL_INCLUDE=$(ROOTDIR)/$(LINUXDIR)/include
LIBC_INCLUDE=$(ROOTDIR)/lib/$(LIBCDIR)/include

DEFINES= -DRESOLVE_HOSTNAMES -DNO_DL

#options if you have a bind>=4.9.4 libresolv (or, maybe, glibc)
LDLIBS := -lresolv $(LDLIBS)
ADDLIB=

#options if you compile with libc5, and without a bind>=4.9.4 libresolv
#LDLIBS=
#ADDLIB=inet_ntop.o inet_pton.o

#options for decnet
ADDLIB+=dnet_ntop.o dnet_pton.o

#options for ipx
ADDLIB+=ipx_ntop.o ipx_pton.o

ifeq ($(findstring uClibc,$(LIBCDIR)),)
ifeq ($(LIBC_INCLUDE)/socketbits.h,$(wildcard $(LIBC_INCLUDE)/socketbits.h))
  ifeq ($(LIBC_INCLUDE)/net/if_packet.h,$(wildcard $(LIBC_INCLUDE)/net/if_packet.h))
    GLIBCFIX=-I../include-glibc -include ../include-glibc/glibc-bugs.h
  endif
endif
ifeq ($(LIBC_INCLUDE)/bits/socket.h,$(wildcard $(LIBC_INCLUDE)/bits/socket.h))
  GLIBCFIX=-I../include-glibc -include ../include-glibc/glibc-bugs.h
endif
endif

# CC = gcc
# CCOPTS = -D_GNU_SOURCE -O2 -Wstrict-prototypes -Wall -Werror -g
CFLAGS := $(CCOPTS) $(GLIBCFIX) -I$(KERNEL_INCLUDE) -I../include $(DEFINES) $(CFLAGS)

LDLIBS := -L../lib -lnetlink -lip2util $(LDLIBS)

SUBDIRS=lib ip tc

LIBNETLINK=../lib/libnetlink.a ../lib/libip2util.a

all: check-kernel
	@set -e; \
	for i in $(SUBDIRS); \
	do $(MAKE) -C $$i; done

check-kernel:
ifeq ($(KERNEL_INCLUDE),)
	@echo "Please, set correct KERNEL_INCLUDE"; false
else
	@set -e; \
	if [ ! -r $(KERNEL_INCLUDE)/linux/autoconf.h ]; then \
		echo "Please, Compile the kernel first"; false; fi
endif

clean:
	for i in $(SUBDIRS) doc; \
	do $(MAKE) -C $$i clean; done

romfs:
	$(ROMFSINST) etc /etc
	for i in $(SUBDIRS); \
	do $(MAKE) -C $$i romfs; done

.EXPORT_ALL_VARIABLES:
