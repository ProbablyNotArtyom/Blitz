# Generated automatically from Makefile.in by configure.
# Makefile.in -- strace Makefile prototype                     -*- Makefile -*-
#
# $Id: Makefile,v 1.5 2003/12/04 06:51:26 steveb Exp $
#

srcdir		= .

#CC		= gcc

INSTALL		= /usr/bin/install -c
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644

DEFS		= -DHAVE_CONFIG_H
#LDLIBS		= 

CFLAGS		+= -D_GNU_SOURCE -D__USE_BSD -D__USE_POSIX199309
ifdef CONFIG_SUPERH
CFLAGS += -DSH=1
endif
ifdef CONFIG_ARM
CFLAGS += -DARM=1 -DHAVE_SYS_USER_H=1
endif
ifdef CONFIG_USERMODE
CFLAGS += -DHAVE_SYS_USER_H=1 -DHAVE_SYS_REG_H=1
endif

#CPPFLAGS	= 
#LDFLAGS		= 
WARNFLAGS	= -Wall

# OS is one of `linux', `sunos4', `svr4', or `freebsd'.
OS		= linux
# ARCH is one of `i386', `m68k', `sparc', `arm', `mips' or `ia64'.
#ARCH		= i386
# OSARCH is OS/ARCH if it exists, otherwise just OS.
OSARCH		= linux

# You may define any of MAX_PROCS, DEFAULT_STRLEN, DEFAULT_ACOLUMN,
# or DEFAULT_SORTBY here.
EXTRA_DEFS	=

ifeq ($(ARCH),i386)
EXTRA_DEFS += -DI386
endif
ifeq ($(ARCH),m68knommu)
EXTRA_DEFS += -DM68K
endif

# Where include files are located, useful for cross-compiling.
includedir	= $(ROOTDIR)/lib/$(LIBCDIR)/include

# Where to install the program:
# I recommend `/usr' for Linux, `/usr/local' for the others.
prefix		= /usr/local
exec_prefix	= ${prefix}

bindir		= ${exec_prefix}/bin
mandir		= ${prefix}/man
man1dir		= $(mandir)/man1
man1ext = .1

SHELL		= /bin/sh

INCLUDES	= -I. -I$(OS)/$(ARCH) -I$(srcdir)/$(OS)/$(ARCH) -I$(OS) \
			-I$(srcdir)/$(OS)
SUBDIRS		= $(OSARCH)

ALL_SUBDIRS	= test linux linux/alpha linux/powerpc sunos4 svr4 freebsd/i386
OBJ		= strace.o version.o syscall.o util.o desc.o file.o ipc.o \
			io.o ioctl.o mem.o net.o process.o bjm.o \
			resource.o signal.o sock.o system.o term.o time.o \
			proc.o stream.o

all: strace

strace: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

install: all
	$(INSTALL_PROGRAM) strace $(bindir)/strace
	$(INSTALL_PROGRAM) $(srcdir)/strace-graph $(bindir)/strace-graph
	$(INSTALL_DATA) $(srcdir)/strace.1 $(man1dir)/strace$(man1ext)

romfs:
	$(ROMFSINST) /bin/strace

clean: clean-local
	for i in $(ALL_SUBDIRS); do \
		(test -f $$i/Makefile && cd ./$$i && $(MAKE) $@ || true); \
	done; exit 0

clean-local:
	rm -f *.o a.out core strace make.out

distclean: distclean-local
	for i in $(ALL_SUBDIRS); do \
		(test -f $$i/Makefile && cd ./$$i && $(MAKE) $@ || true); \
	done; exit 0

distclean-local: clean-local
	rm -f machine
	rm -f Makefile config.h config.status config.cache config.log

maintainer-clean: maintainter-clean-local
	for i in $(ALL_SUBDIRS); do \
		(test -f $$i/Makefile && cd ./$$i && $(MAKE) $@ || true); \
	done; exit 0

maintainer-clean-local: distclean-local

.c.o:
	$(CC) $(WARNFLAGS) $(DEFS) $(EXTRA_DEFS) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c $<

desc.o: desc.c defs.h config.h
file.o: file.c defs.h config.h
io.o: io.c defs.h config.h
ioctl.o: ioctl.c defs.h config.h
mem.o: mem.c defs.h config.h
net.o: net.c defs.h config.h
process.o: process.c defs.h config.h
resource.o: resource.c defs.h config.h
signal.o: signal.c defs.h config.h
socket.o: socket.c defs.h config.h
strace.o: strace.c defs.h config.h
syscall.o: syscall.c defs.h config.h
system.o: system.c defs.h config.h
time.o: time.c defs.h config.h
util.o: util.c defs.h config.h
