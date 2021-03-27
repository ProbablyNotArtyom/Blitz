# Edit this path to match with your system (it should point to the root
# directory of the Linux kernel source)
#KERNEL_PATH=/usr/src/linux
KERNEL_PATH=$(ROOTDIR)/$(LINUXDIR)

# Systems using external pcmcia-cs package (instead of kernel-tree pcmcia
# modules):
#
# hostap_cs.o can be compiled either by copying driver/* into pcmcia-cs root
# directory and compiling pcmcia-cs package or by configuring PCMCIA_PATH
# and using this Makefile with 'make pccard; make install_pccard'.
# Leave this blank for kernel-tree PCMCIA compilations (CONFIG_PCMCIA):
PCMCIA_PATH=

# This path will be prepended to every installed file
DESTDIR=

#CC=gcc
#CFLAGS=-O2 -D__KERNEL__ -DMODULE -Wall -g -c $(EXTRA_CFLAGS)
CFLAGS += -D__KERNEL__ -DMODULE -g -c $(EXTRA_CFLAGS) -DHOSTAP_CRYPT_MODULE

include $(KERNEL_PATH)/.config

INCLUDES=-I$(KERNEL_PATH)/include
ifdef PCMCIA_PATH
ifndef CONFIG_PCMCIA
INCLUDES=-I$(PCMCIA_PATH)/include -I$(KERNEL_PATH)/include
endif
ifeq (,$(wildcard $(PCMCIA_PATH)/include/pcmcia/version.h))
$(error PCMCIA_PATH doesn't contain valid pcmcia-cs source)
endif
ifdef CONFIG_PCMCIA
$(error Kernel supports PCMCIA - PCMCIA_PATH shouldn't be defined)
endif
export PCMCIA_PATH
endif

ifndef PCMCIA_PATH
ifndef CONFIG_PCMCIA
$(warning WARNING: No kernel PCMCIA support found and PCMCIA_PATH is not defined)
endif
endif

ifndef CONFIG_NET_RADIO
# Most driver configuration commands and hostapd require Linux wireless
# extensions
$(warning WARNING: Linux wireless extensions, CONFIG_NET_RADIO, not enabled in the kernel)
endif

VERFILE := $(KERNEL_PATH)/include/linux/version.h
KERNELRELEASE := $(shell if [ -r $(VERFILE) ]; \
	then (cat $(VERFILE); echo UTS_RELEASE) | $(CC) $(INCLUDES) $(CFLAGS) -E - | tail -1 | xargs echo; \
	else uname -r; fi)
KERNELVER := $(shell echo "$(KERNELRELEASE)" | \
	sed "s/\([0-9]*\.[0-9]*\.[0-9]*\).*/\1/")
MODPATH := $(DESTDIR)/lib/modules/$(KERNELRELEASE)
MODULE_CS := hostap_cs.o
MODULE_PLX := hostap_plx.o
MODULE_PCI := hostap_pci.o
MODPATH_CS := $(MODPATH)/pcmcia
MODPATH_PLX := $(MODPATH)/net
MODPATH_PCI := $(MODPATH)/net
CURRENTKV := $(shell echo `uname -r`)

MSRC=driver/modules

# if Rules.make exists in the kernel tree, we assume 2.4 style modules
# if it doesn't assume 2.6 style
OLDMAKE = $(wildcard $(KERNEL_PATH)/Rules.make)


all: pci pccard

help:
	@echo "Select the version to be build:"
	@echo "make pccard      PC Card (PCMCIA) driver (Prism2 or Prism2.5)"
	@echo "make plx         PLX-based PCI card with PC Card"
	@echo "make pci         Prism 2.5 PCI Card"
	@echo "make crypt       Make hostap_crypt_*.o"
	@echo "make hostap      Make hostap.o"

ifdef CONFIG_ALPHA
CFLAGS += -mno-fp-regs -ffixed-8
endif

ifdef CONFIG_MODVERSIONS
CFLAGS += -DMODVERSIONS -include $(KERNEL_PATH)/include/linux/modversions.h
endif

ifdef CONFIG_SMP
CFLAGS += -D__SMP__ -DSMP
endif

ifndef CONFIG_FRAME_POINTER
CFLAGS += -fomit-frame-pointer
endif


# Old compilation using this Makefile and not the build system in the kernel
# tree. This is used when compiling for Linux 2.2.x (or Linux 2.4.x with
# 'make pccard'). Both Linux 2.6.x and 2.4.x can use kernel tree build system
# with 'make'.

ifdef CONFIG_MODVERSIONS

VERFILES=$(MSRC)/hostap.ver ## $(MSRC)/hostap_crypt.ver
$(MSRC)/hostap_crypt_wep.o: $(VERFILES)
$(MSRC)/hostap.o: $(VERFILES)
$(MSRC)/hostap_cs.o: $(VERFILES)
$(MSRC)/hostap_plx.o: $(VERFILES)
$(MSRC)/hostap_pci.o: $(VERFILES)
INCLUDES_MODVER=-include $(MSRC)/hostap.ver ##-include $(MSRC)/hostap_crypt.ver

GENKSYMS=/sbin/genksyms
ifdef CONFIG_SMP
        GENKSYMS += -p smp_
endif

##$(MSRC)/hostap_crypt.ver: $(MSRC)/hostap_crypt.c
##	$(CC) $(INCLUDES) $(CFLAGS) -E -D__GENKSYMS__ $^ | \
##		$(GENKSYMS) -k $(KERNELVER) > $@
$(MSRC)/hostap.ver: $(MSRC)/hostap.c
	$(CC) $(INCLUDES) $(CFLAGS) -E -D__GENKSYMS__ $^ | \
		$(GENKSYMS) -k $(KERNELVER) > $@

endif


.c.o:
	$(CC) $(INCLUDES) $(INCLUDES_MODVER) $(CFLAGS) -o $@ $<

# end of old compilation


2.4:
	@if ! echo "$(KERNELRELEASE)" | grep -q "^2\.4"; then \
		echo "*** Can't build for 2.4 with a non-2.4 source!"; \
		exit 1; \
	fi
	$(MAKE) -C $(KERNEL_PATH) SUBDIRS=$(PWD)/driver/modules modules
	$(MAKE) install_all_info

2.6:
	@if ! echo "$(KERNELRELEASE)" | grep -q "^2\.6" && \
	    ! echo "$(KERNELRELEASE)" | grep -q "^2\.5"; then \
		echo "*** Can't build for 2.6 with a non-2.6 source!"; \
		exit 1; \
	fi
	$(MAKE) -C $(KERNEL_PATH) SUBDIRS=$(PWD)/driver/modules \
		MODVERDIR=$(PWD)/driver/modules modules
	$(MAKE) install_all_info

pccard:	$(MSRC)/$(MODULE_CS) hostap crypt
plx:	$(MSRC)/$(MODULE_PLX) hostap crypt
pci:	$(MSRC)/$(MODULE_PCI) hostap crypt
crypt:  $(MSRC)/hostap_crypt.o $(MSRC)/hostap_crypt_wep.o
hostap: $(MSRC)/hostap.o

SRC=$(MSRC)/hostap_hw.c $(MSRC)/hostap_wlan.h $(MSRC)/hostap_config.h \
	$(MSRC)/hostap_download.c \
	$(MSRC)/hostap_ap.h \
	$(MSRC)/hostap_crypt.h $(MSRC)/hostap.h $(MSRC)/hostap_compat.h

## $(MSRC)/hostap_crypt.o: $(MSRC)/hostap_crypt.c $(MSRC)/hostap_crypt.h

$(MSRC)/hostap_crypt_wep.o: $(MSRC)/hostap_crypt_wep.c $(MSRC)/hostap_crypt.h

$(MSRC)/hostap.o: $(MSRC)/hostap.c $(MSRC)/hostap.h \
	$(MSRC)/hostap_ap.h $(MSRC)/hostap_ap.c $(MSRC)/hostap_info.c \
	$(MSRC)/hostap_ioctl.c $(MSRC)/hostap_proc.c \
	$(MSRC)/hostap_wlan.h $(MSRC)/hostap_config.h \
	$(MSRC)/hostap_compat.h

$(MSRC)/$(MODULE_CS): $(MSRC)/hostap_cs.c $(SRC)

$(MSRC)/$(MODULE_PLX): $(MSRC)/hostap_plx.c $(SRC)

$(MSRC)/$(MODULE_PCI): $(MSRC)/hostap_pci.c $(SRC)


install_info:
	@echo
	@echo "Run 'make install_pccard' as root to install $(MODULE_CS)"
	@echo "Run 'make install_plx' as root to install $(MODULE_PLX)"
	@echo "Run 'make install_pci' as root to install $(MODULE_PCI)"
	@echo

install_all_info:
	@echo "*** Become superuser and 'make install' to install the modules. ***" 


ifeq (,$(OLDMAKE))
install: install_2.6
else
install: install_2.4
endif

install_check:
	@if [ ! -d "$(MODPATH)" ]; then \
		echo "Module installation directory not valid: '$(MODPATH)'"; \
		echo -n "Most probable cause for this is modified kernel "; \
		echo "headers."; \
		echo -n "You will need to install module file "; \
		echo "($(MSRC)/hostap*.{,k}o) manually."; \
		exit 1; \
	fi

install_2.4: uninstall install_pccard install_pci install_plx

install_2.6: install_conf
	$(MAKE) install_check
	@echo "*** Copying modules to: $(MODPATH)/kernel/drivers/net/wireless"
	mkdir -p $(MODPATH)/kernel/drivers/net/wireless
	cp $(MSRC)/*.ko $(MODPATH)/kernel/drivers/net/wireless
	@if [ -r $(MODPATH)/kernel/drivers/net/wireless/hostap_crypt.ko ]; then \
		echo "You may need to remove old $(MODPATH)/kernel/drivers/net/wireless/hostap_crypt.ko"; \
	fi
	@if [ -f "/boot/System.map-$(KERNELRELEASE)" ] || [ "$(CURRENTKV)" = "$(KERNELRELEASE)" ]; then \
		echo "*** Running depmod..."; \
		/sbin/depmod -ae; \
	else \
		echo "*** I won't run depmod because I can't find an appropriate System.map file or you are not running the same kernel as the one you are building against. Please run it manually when the conditions are correct..."; \
	fi

install_hostap: $(MSRC)/hostap.o
	@echo "Installing hostap.o to $(MODPATH_PCI)"
	mkdir -p $(MODPATH_PCI)
	cp -f $(MSRC)/hostap.o $(MODPATH_PCI)

install_crypt: $(MSRC)/hostap_crypt_wep.o ## $(MSRC)/hostap_crypt.o
	@echo "Installing hostap_crypt_*.o to $(MODPATH_PCI)"
	mkdir -p $(MODPATH_PCI)
	cp -f $(MSRC)/hostap_crypt_*.o $(MODPATH_PCI)
	@if [ -r $(MODPATH_PCI)/hostap_crypt.o ]; then \
		echo "You may need to remove old $(MODPATH_PCI)/hostap_crypt.o"; \
	fi

install_pccard: install_check install_crypt install_hostap install_conf
	@echo "Installing $(MODULE_CS) to $(MODPATH_CS)"
	mkdir -p $(MODPATH_CS)
	cp -f $(MSRC)/$(MODULE_CS) $(MODPATH_CS)
ifndef DESTDIR
	/sbin/depmod -ae
endif

install_conf:
	@if [ ! -r $(DESTDIR)/etc/pcmcia/hostap_cs.conf ]; then \
		echo "Installing $(DESTDIR)/etc/pcmcia/hostap_cs.conf"; \
		cp -f driver/etc/hostap_cs.conf $(DESTDIR)/etc/pcmcia/hostap_cs.conf; \
	fi
	@if [ -r $(DESTDIR)/etc/pcmcia/prism2.conf ]; then \
		echo "You may need to remove old $(DESTDIR)/etc/pcmcia/prism2.conf"; \
	fi

install_plx: install_check install_crypt install_hostap
	@echo "Installing $(MODULE_PLX) to $(MODPATH_PLX)"
	mkdir -p $(MODPATH_PLX)
	cp -f $(MSRC)/$(MODULE_PLX) $(MODPATH_PLX)
	/sbin/depmod -ae

install_pci: install_check install_crypt install_hostap
	@echo "Installing $(MODULE_PCI) to $(MODPATH_PCI)"
	mkdir -p $(MODPATH_PCI)
	cp -f $(MSRC)/$(MODULE_PCI) $(MODPATH_PCI)
	/sbin/depmod -ae

romfs:
	mkdir -p $(ROMFSDIR)/$(MODPATH_PCI)
	mkdir -p $(ROMFSDIR)/$(MODPATH_CS)
	$(ROMFSINST) $(MSRC)/hostap.o $(MODPATH_PCI)/hostap.o
	$(ROMFSINST) $(MSRC)/hostap_crypt.o $(MODPATH_PCI)/hostap_crypt.o
	$(ROMFSINST) $(MSRC)/hostap_crypt_wep.o $(MODPATH_PCI)/hostap_crypt_wep.o
	$(ROMFSINST) -e CONFIG_USER_HOSTAP_PCI \
			$(MSRC)/$(MODULE_PCI) $(MODPATH_PCI)/$(MODULE_PCI)
	$(ROMFSINST) -e CONFIG_USER_HOSTAP_CS \
			$(MSRC)/$(MODULE_CS) $(MODPATH_CS)/hostap_cs.o
	mkdir -p $(ROMFSDIR)/etc/pcmcia
	$(ROMFSINST) -e CONFIG_USER_HOSTAP_CS \
			driver/etc/hostap_cs.conf /etc/pcmcia/hostap_cs.conf

uninstall:
	find $(MODPATH) -name "hostap*" | xargs rm -f

clean:
	rm -f $(MSRC)/{$(MODULE_CS),$(MODULE_PLX),$(MODULE_PCI)}
	rm -f $(MSRC)/{hostap.o,hostap_crypt_wep.o}
	rm -f $(MSRC)/*.ko
	rm -f $(MSRC)/*.ver
	rm -f $(MSRC)/*.mod.[co]
	rm -f $(MSRC)/*.mod
	rm -f $(MSRC)/.hostap*.cmd
