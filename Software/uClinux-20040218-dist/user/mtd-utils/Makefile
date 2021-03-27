# -*- sh -*-

# $Id: Makefile,v 1.20 2001/09/05 00:18:11 andersen Exp $

#
# These flags are used for build native versions of the tools for the
# host system so it can generate images etc
#
#MY_CFLAGS = -I$(ROOTDIR)/$(LINUXDIR)/include
MY_CFLAGS = -I/usr/include
BUILD_CFLAGS += $(MY_CFLAGS)
BUILD_CC = gcc
CFLAGS += $(INCZ) -D__USE_BSD

TARGETS = erase eraseall \
	      nftl_format nftldump doc_loadbios doc_loadipl \
	      ftl_format ftl_check \
	      nanddump nandtest nandwrite \
	      mkfs.jffs mkfs.jffs2 \
	      lock unlock einfo mtd_debug fcp 

BUILD_TARGETS = $(patsubst %,build/%,$(TARGETS))

SYMLINKS = 
JFFS2_OBJS = crc32.o mkfs.jffs2.o compr_zlib.o compr_rtime.o

ifeq ($(LINUXDIR),linux-2.2.x)
SYMLINKS += compr_rubin.h compr_rubin.c compr.c histo_mips.h pushpull.h pushpull.c
JFFS2_OBJS += compr_rubin.o compr.o pushpull.o
endif

BUILD_JFFS2_OBJS = $(patsubst %,build/%,$(JFFS2_OBJS))

all: build $(TARGETS) $(BUILD_TARGETS)

.PHONY:
build:
	[ -d build ] || mkdir build

clean:
	rm -f *.o *.gdb $(TARGETS) $(BUILD_TARGETS) $(SYMLINKS)
	rm -rf build

romfs:
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_ERASE        /bin/erase
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_ERASEALL     /bin/eraseall
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_FTL_CHECK    /bin/ftl_check
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_FTL_FORMAT   /bin/ftl_format
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_MKFSJFFS     /bin/mkfs.jffs
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_MKFSJFFS2    /bin/mkfs.jffs2
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NFTLDUMP     /bin/nftldump
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NFTL_FORMAT  /bin/nftl_format
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDDUMP     /bin/nanddump
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDTEST     /bin/nandtest
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_NANDWRITE    /bin/nandwrite
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_DOC_LOADBIOS /bin/doc_loadbios
	$(ROMFSINST) -e CONFIG_USER_MTDUTILS_DOC_LOADIPL  /bin/doc_loadipl

mkfs.jffs2.o crc32.o: crc32.h
build/mkfs.jffs2.o build/crc32.o: crc32.h
compr_rubin.o: pushpull.h
build/compr_rubin.o: pushpull.h
compr_rubin.o: histo_mips.h compr_rubin.h
build/compr_rubin.o: histo_mips.h compr_rubin.h

$(SYMLINKS):
	ln -sf $(ROOTDIR)/$(LINUXDIR)/fs/jffs2/$@ $@

mkfs.jffs2: $(JFFS2_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDPATH) -lz $(LDLIBS)

build/mkfs.jffs2: $(BUILD_JFFS2_OBJS)
	$(BUILD_CC) -o $@ $^ -lz

doc_loadipl: doc_loadipl.o rsencode.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LDPATH) $(LDLIBS)

build/doc_loadipl: build/doc_loadipl.o build/rsencode.o
	$(BUILD_CC) -o $@ $^

compr.o: compr.c
	$(CC) $(CFLAGS) -Dprintk=printf -DKERN_NOTICE= -DKERN_WARNING= -c -o $@ $<

build/compr.o: compr.c
	$(BUILD_CC) $(BUILD_CFLAGS) -Dprintk=printf -DKERN_NOTICE= -DKERN_WARNING= -c -o $@ $<

compr_zlib.o: compr_zlib.c
	$(CC) $(CFLAGS) $(INCZ) -c -o $@ $<

build/compr_zlib.o: compr_zlib.c
	$(BUILD_CC) $(BUILD_CFLAGS) -Dprintk=printf -DKERN_NOTICE= -DKERN_WARNING= $(INCZ) -c -o $@ $<

jffs2reader: jffs2reader.c
	$(CC) $(LDFLAGS) $(CFLAGS) jffs2reader.c -o jffs2reader $(LDPATH) -lz $(LDLIBS)

build/jffs2reader: jffs2reader.c
	$(BUILD_CC) $(BUILD_CFLAGS) -o $@ $< -lz

build/%.o: %.c
	$(BUILD_CC) $(BUILD_CFLAGS) -c -o $@ $<

build/%: %.c
	$(BUILD_CC) $(BUILD_CFLAGS) -o $@ $<

