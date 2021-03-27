
include MCONFIG

RESIZE_DIR  = resize
DEBUGFS_DIR = debugfs

LIB_SUBDIRS  = lib/et lib/ss lib/e2p lib/ext2fs lib/uuid
PROG_SUBDIRS = e2fsck $(DEBUGFS_DIR) misc $(RESIZE_DIR) tests/progs 
SUBDIRS      = util $(LIB_SUBDIRS) $(PROG_SUBDIRS) tests

CLEANDIRS    = $(LIB_SUBDIRS) $(PROG_SUBDIRS) util

SUBS = lib/ext2fs/ext2_types.h

all: $(SUBS) 
	for i in $(LIB_SUBDIRS); do \
		$(MAKE) -C $$i all || exit 1; \
	done
	for i in $(PROG_SUBDIRS); do \
		$(MAKE) -C $$i all || exit 1; \
	done

clean:
	rm -f $(SUBS) 
	for i in $(CLEANDIRS); do \
		$(MAKE) -C $$i clean || exit 1; \
	done

romfs:
	for i in $(PROG_SUBDIRS); do \
		$(MAKE) -C $$i romfs || exit 1; \
	done

lib/ext2fs/ext2_types.h: $(DEP_SUBSTITUTE) $(srcdir)/lib/ext2fs/ext2_types.h.in
	$(SUBSTITUTE) $(srcdir)/lib/ext2fs/ext2_types.h.in > lib/ext2fs/ext2_types.h

$(DEP_SUBSTITUTE):
	$(MAKE) -C util subst
