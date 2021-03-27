# Generated automatically from Makefile.in by configure.
#  Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997
# 	The Regents of the University of California.  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that: (1) source code distributions
#  retain the above copyright notice and this paragraph in its entirety, (2)
#  distributions including binary code include the above copyright notice and
#  this paragraph in its entirety in the documentation or other materials
#  provided with the distribution, and (3) all advertising materials mentioning
#  features or use of this software display the following acknowledgement:
#  ``This product includes software developed by the University of California,
#  Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
#  the University nor the names of its contributors may be used to endorse
#  or promote products derived from this software without specific prior
#  written permission.
#  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
#  WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
#
#
# Various configurable paths (remember to edit Makefile.in, not Makefile)
#

# Top level hierarchy
prefix = /usr/local
exec_prefix = ${prefix}
# Pathname of directory to install the binary
BINDEST = ${exec_prefix}/sbin
# Pathname of directory to install the man page
MANDEST = ${prefix}/man

# VPATH
srcdir = .

#
# You shouldn't need to edit anything below here.
#

#CC = gcc
PROG = tcpdump
#CCOPT = -O2
INCLS += -I.  -Ilinux-include $(INCPCAP)
DEFS =  -DHAVE_FCNTL_H=1 -DHAVE_MALLOC_H=1 -DHAVE_MEMORY_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_NET_SLIP_H=1 -DHAVE_VFPRINTF=1 -DHAVE_STRCASECMP=1 -DHAVE_SETLINEBUF=1 -DRETSIGTYPE=void -DRETSIGVAL= -D__USE_BSD=1

# larger stack size required for tcpdump
FLTFLAGS += -s 8192

# Standard CFLAGS
CFLAGS += $(CCOPT) $(DEFS)

# Standard LDFLAGS
#LDFLAGS = 

# Standard LIBS
LIBS = $(LIBPCAP)

INSTALL = /usr/bin/install -c

# Explicitly define compilation rule since SunOS 4's make doesn't like gcc.
# Also, gcc does not remove the .o before forking 'as', which can be a
# problem if you don't own the file but can write to the directory.
.c.o:
	@rm -f $@
	$(CC) $(INCLS) $(CFLAGS) -c $(srcdir)/$*.c

CSRC =	tcpdump.c \
	print-arp.c print-atalk.c print-atm.c print-bootp.c \
	print-decnet.c print-domain.c print-dvmrp.c print-egp.c \
	print-ether.c print-fddi.c print-gre.c print-icmp.c \
	print-igrp.c print-ip.c print-ipx.c print-isoclns.c print-krb.c \
	print-llc.c print-nfs.c print-ntp.c print-null.c print-ospf.c \
	print-pim.c print-ppp.c print-raw.c print-rip.c print-sl.c \
	print-snmp.c print-sunrpc.c print-tcp.c print-tftp.c print-udp.c \
	print-wb.c addrtoname.c bpf_dump.c gmt2local.c machdep.c \
	parsenfsfh.c util.c savestr.c setsignal.c
LOCALSRC =
GENSRC = version.c

SRC =	$(CSRC) $(GENSRC) $(LOCALSRC)

# We would like to say "OBJ = $(SRC:.c=.o)" but Ultrix's make cannot
# hack the extra indirection
OBJ =	$(CSRC:.c=.o) $(GENSRC:.c=.o) $(LOCALSRC:.c=.o) 
HDR =   addrtoname.h appletalk.h bootp.h decnet.h \
	ethertype.h extract.h fddi.h gmt2local.h interface.h igrp.h \
	ipx.h llc.h machdep.h mib.h nfsfh.h nfsv2.h ntp.h ospf.h \
	savestr.c setsignal.h

TAGHDR = \
	/usr/include/arpa/tftp.h \
	/usr/include/net/if_arp.h \
	/usr/include/net/slip.h \
	/usr/include/netinet/if_ether.h \
	/usr/include/netinet/in.h \
	/usr/include/netinet/ip_icmp.h \
	/usr/include/netinet/tcp.h \
	/usr/include/netinet/udp.h \
	/usr/include/protocols/routed.h

TAGFILES = $(SRC) $(HDR) $(TAGHDR)

CLEANFILES = $(PROG) $(OBJ) $(GENSRC) *.elf *.gdb

all: $(PROG)

$(PROG): $(OBJ) 
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS) $(LDLIBS)

version.o: version.c
version.c: $(srcdir)/VERSION
	@rm -f $@
	sed -e 's/.*/char version[] = "&";/' $(srcdir)/VERSION > $@

install: force
	$(INSTALL) -m 550 -o bin -g wheel $(PROG) \
	    $(DESTDIR)$(BINDEST)/$(PROG)

install-man: force
	$(INSTALL) -m 444 -o bin -g bin $(srcdir)/$(PROG).1 \
	    $(DESTDIR)$(MANDEST)/man1/$(PROG).1

lint: $(GENSRC) force
	lint -hbxn $(SRC) | \
	    grep -v 'struct/union .* never defined' | \
	    grep -v 'possible pointer alignment problem'

romfs:
	$(ROMFSINST) /bin/$(PROG)

clean:
	rm -f $(CLEANFILES)

distclean:
	rm -f $(CLEANFILES) Makefile config.cache config.log config.status \
	    gnuc.h os-proto.h

tags: $(TAGFILES)
	ctags -wtd $(TAGFILES)

tar: force
	@cwd=`pwd` ; dir=`basename $$cwd` ; name=$(PROG)-`cat VERSION` ; \
	    list="" ; tar="tar chFFf" ; \
	    for i in `cat FILES` ; do list="$$list $$name/$$i" ; done; \
	    echo \
	    "rm -f ../$$name; ln -s $$dir ../$$name" ; \
	     rm -f ../$$name; ln -s $$dir ../$$name ; \
	    echo \
	    "(cd .. ; $$tar - [lots of files]) | compress > /tmp/$$name.tar.Z" ; \
	     (cd .. ; $$tar - $$list) | compress > /tmp/$$name.tar.Z ; \
	    echo \
	    "rm -f ../$$name" ; \
	     rm -f ../$$name

force: /tmp
depend: $(GENSRC) force
	./mkdep -c $(CC) $(DEFS) $(INCLS) $(SRC)
