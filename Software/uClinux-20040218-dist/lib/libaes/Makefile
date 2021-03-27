
.EXPORT_ALL_VARIABLES:

EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libcrypto/libaes
EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libcrypto/include
EXTRA_CFLAGS += -DEMBED -DHIFN=1 -I$(ROOTDIR)/lib/$(CONFIG_LIBCDIR)/include -I$(ROOTDIR)/modules/crypto
LDLIBS += ../libc/libc.a

CONFIG_HW_ASSIST = y

VPATH = $(ROOTDIR)/freeswan/libcrypto/libaes

first_target: libaes.a

include $(ROOTDIR)/freeswan/libcrypto/libaes/Makefile

romfs:

