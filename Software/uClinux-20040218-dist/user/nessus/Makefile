.EXPORT_ALL_VARIABLES:
-include $(CONFIG_CONFIG)

# Define the directories we are going to build.
# There are four base targets present here:
#	nessus-libraries libnasl nessus-core nessus-plugins
BUILD_DIRS :=
ifdef CONFIG_USER_NESSUS_NASL
BUILD_DIRS += libnasl
endif

# Now work out some derived stuff
ROMFS_DIRS = $(addsuffix -romfs,$(BUILD_DIRS))
MY_TARGET:=$(strip $(subst -, ,$(CROSS)))
MY_TARGET:=$(word 1,$(MY_TARGET))-$(word 2,$(MY_TARGET))
CFLAGS += $(INCSSL) $(INCPCAP)

.PHONY: all romfs clean
.PHONY: nessus-libraries libnasl nessus-core nessus-plugins
.PHONY: nessus-libraries-romfs libnasl-romfs nessus-core-romfs nessus-plugins-romfs

all: $(BUILD_DIRS)
romfs: $(ROMFS_DIRS)
clean:
	rm -rf lib
	for i in $(BUILD_DIRS) ; do make -C $$i distclean ; done


# The base libraries are needed for everything
nessus-libraries:
	if [ ! -f nessus-libraries/config.status ]; then		\
		cd nessus-libraries ; 					\
		chmod 755 configure;					\
		export LDFLAGS='-lc';					\
		./configure --disable-fast-install --disable-shared	\
			--cache-file config.cache			\
			--prefix= --build=i386-redhat-linux		\
			--with-headers=$(ROOTDIR)/$(LINUXDIR)/include	\
			--target=$(MY_TARGET)				\
	; fi
	make -C nessus-libraries || exit $$?
	[ -d lib ] || mkdir lib
	cp `find nessus-libraries -type f -name '*.a'` lib/

nessus-libraries-romfs:


# Includes the NASL scripting language
libnasl: nessus-libraries
	if [ ! -f libnasl/config.status ]; then				\
		cd libnasl ; 						\
		chmod 755 configure;					\
		export LDFLAGS='-lc';					\
		export PATH="../nessus-libraries:$$PATH";		\
		./configure --disable-fast-install --disable-shared	\
			--cache-file config.cache			\
			--prefix= --build=i386-redhat-linux		\
			--with-headers=$(ROOTDIR)/$(LINUXDIR)/include	\
			--target=$(MY_TARGET)				\
	; fi
	make -C libnasl || exit $$?
	#cp `find nessus-libraries -type f -name '*.a'` lib/

libnasl-romfs: nessus-libraries-romfs
	$(ROMFSINST) libnasl/nasl/nasl /bin/nasl


# Not ported. Includes nessus and nessusd
nessus-core: libnasl
nessus-core-romfs: libnasl-romfs


# Not ported. The scripts etc
nessus-plugins: nessus-core
nessus-plugins-romfs: nessus-core-romfs

