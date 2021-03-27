silly_first_rule_thingy: Makefile
	make -f Makefile

CONFIGURE_TARGET := $(strip $(subst -, ,$(CROSS)))
CONFIGURE_TARGET := $(word 1,$(CONFIGURE_TARGET))-$(word 2,$(CONFIGURE_TARGET))
CONFIGURE_SETTINGS := -C --prefix= --build=i386-redhat-linux		\
		--enable-dependency-tracking				\
		--target=$(CONFIGURE_TARGET) --without-pthread		\
		--without-readline --disable-shared			\
		--with-headers=$(ROOTDIR)/$(LINUXDIR)/include		\
		--with-mysql=$(ROOTDIR)/lib/libmysqlclient		\
		--with-named-z-libs==$(ROOTDIR)/lib/zlib

#
# we always have glibc2 style,  so force it so that uClibc works as well
#
MY_EXTRA_CFLAGS := -DHAVE_GETHOSTBYNAME_R_GLIBC2_STYLE=1

Makefile:
	unset LDFLAGS CC CFLAGS && \
			CFLAGS="$(MY_EXTRA_CFLAGS)" ./configure $(CONFIGURE_SETTINGS)

romfs:
	-echo "nothing for roms"

%: Makefile
	make -f Makefile $@

