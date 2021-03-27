
.EXPORT_ALL_VARIABLES:

EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libdes
EXTRA_CFLAGS += -DEMBED
LDLIBS += ../libc/libc.a ../libm/libm.a



VPATH = $(ROOTDIR)/freeswan/libdes

first_target: libdes.a

include $(ROOTDIR)/freeswan/libdes/Makefile

romfs:

