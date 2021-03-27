#
#	Makefile -- Master makefile for all libraries.
#

.EXPORT_ALL_VARIABLES:

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_LIB=1
include $(LINUX_CONFIG)
include $(CONFIG_CONFIG)
include $(ARCH_CONFIG)

#
# always build these dir_
#
dir_y  = $(LIBCDIR) libnet libcrypt_old $(ROOTDIR)/prop
dir_n  =
dir_   =

ifeq ($(LIBCDIR),libc)
dir_y += libm
endif

#
# and build these if someone needs them
#
dir_$(CONFIG_LIB_LIBAES)       += libaes
dir_$(CONFIG_LIB_LIBAES_FORCE) += libaes
dir_$(CONFIG_LIB_LIBDES)       += libdes
dir_$(CONFIG_LIB_LIBDES_FORCE) += libdes
dir_$(CONFIG_LIB_LIBSSL)       += libssl
dir_$(CONFIG_LIB_LIBSSL_FORCE) += libssl
dir_$(CONFIG_LIB_LIBGMP)       += libgmp
dir_$(CONFIG_LIB_LIBGMP_FORCE) += libgmp
dir_$(CONFIG_LIB_LIBG)         += libg
dir_$(CONFIG_LIB_LIBG_FORCE)   += libg
dir_$(CONFIG_LIB_LIBPAM)       += libpam
dir_$(CONFIG_LIB_LIBPAM_FORCE) += libpam
dir_$(CONFIG_LIB_LIBPCAP)      += libpcap
dir_$(CONFIG_LIB_LIBPCAP_FORCE)+= libpcap
dir_$(CONFIG_LIB_ZLIB)         += zlib
dir_$(CONFIG_LIB_ZLIB_FORCE)   += zlib
dir_$(CONFIG_LIB_LIBATM)       += libatm
dir_$(CONFIG_LIB_LIBATM_FORCE) += libatm
dir_$(CONFIG_LIB_LIBPNG)       += libpng
dir_$(CONFIG_LIB_LIBPNG_FORCE) += libpng
dir_$(CONFIG_LIB_LIBJPEG)      += libjpeg
dir_$(CONFIG_LIB_LIBJPEG_FORCE)+= libjpeg

dir_$(CONFIG_LIB_LIBCCMALLOC)  += libccmalloc

# Figure out some libldap stuff if required
ifeq ($(CONFIG_LIB_LIBLDAP),y)
BUILD_OPENLDAP:=y
endif
ifeq ($(CONFIG_LIB_LIBLDAP_FORCE),y)
BUILD_OPENLDAP:=y
endif
ifeq ($(BUILD_OPENLDAP),y)
# determine our build target from the cross compiler options
LDAP_TARGET:=$(strip $(subst -, ,$(CROSS)))
LDAP_TARGET:=$(word 1,$(LDAP_TARGET))-$(word 2,$(LDAP_TARGET))
LDAP_OPTIONS:= --without-threads --without-local --disable-ipv6 \
		--enable-x-compile --disable-slapd --disable-slurpd
endif


all:
ifeq ($(findstring glibc,$(LIBCDIR)),glibc)
	[ -d $(LIBCDIR) ] || mkdir $(LIBCDIR)
	if [ ! -f $(LIBCDIR)/config.cache ]; then \
		cd $(LIBCDIR); \
		export CFLAGS="$(VENDOR_CFLAGS) -DEMBED -O2"; \
		unset LDFLAGS; \
		../configure  --prefix= \
			$(GLIBC_TARGET_OPTS) --build=i386-redhat-linux \
			--with-headers=$(ROOTDIR)/$(LINUXDIR)/include \
			--datadir=/usr/share \
			--without-cvs --without-gd \
			--disable-profile --disable-force-install \
			--enable-add-ons ; \
	fi
endif
	for i in $(dir_y) ; do [ ! -d $$i ] || make -C $$i || exit $$? ; done
ifeq ($(BUILD_OPENLDAP),y)
	if [ ! -f libldap/config.status ]; then			\
		cd libldap;					\
		export CFLAGS="$(subst -Wall,,$(CFLAGS))"; 	\
		export LDFLAGS='-lc';					\
		./configure --prefix= --build=i386-redhat-linux		\
			--with-headers=$(ROOTDIR)/$(LINUXDIR)/include	\
			--target=$(LDAP_TARGET) $(LDAP_OPTIONS)		\
		; make depend					\
	; fi
	make -C libldap || exit $$?
endif

romfs:
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || make -C $$i romfs || exit $$? ; \
	done
	-$(STRIP) $(ROMFSDIR)/lib/*

clean:
ifeq ($(findstring glibc,$(LIBCDIR)),glibc)
	-rm -rf $(LIBCDIR)
endif
	-for i in $(dir_y) $(dir_n) $(dir_); do \
		[ ! -d $$i ] || make -C $$i clean ; \
	done
ifeq ($(BUILD_OPENLDAP),y)
	make -C libldap distclean
endif

