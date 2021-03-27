#
# Basic and crude Makefile...
#

# Targets to build
PROGS= iwconfig iwlist iwpriv iwspy iwgetid

# Installation directory. By default, go in local.
# Distributions should probably use /usr/sbin, but they probably know better...
INSTALL_DIR= /usr/local/sbin

# This is our header selection. Try to hide the mess and the misery :-(
# Please choose *only one* of the define...

# Kernel headers 2.4.X + Glibc 2.2 - Mandrake 8.0 */
#HEADERS= -DGLIBC22_HEADERS

# Kernel headers 2.0.X + Glibc 2.0 - Debian 2.0, RH 5
# Kernel headers 2.2.X + Glibc 2.1 - Debian 2.2, RH 6.1
# Kernel headers 2.4.X + Glibc 2.1 - Debian 2.2 upgraded, RH 7.0
# HEADERS= -DGLIBC_HEADERS

# Kernel headers 2.2.X + Glibc 2.0 - Debian 2.1
#HEADERS= -DKLUDGE_HEADERS

# Kernel headers 2.0.X + libc5 - old systems
#HEADERS= -DLIBC5_HEADERS

# Why bother with messy kernel headers !
HEADERS= -DNORMAL_HEADERS

# Use private copy of Wireless Extension definition instead of the
# system wide one in /usr/include/linux. Use with care.
# Can be used to create multiple versions of the tools on the same system
# for multiple kernels or get around broken distributions.
#WE_HEADER= -DPRIVATE_WE_HEADER
WE_HEADER=

# ------------ End of config --------------

# CC = gcc
RM = rm -f

RM_CMD = $(RM) *.BAK *.bak *.o ,* *~ *.a

CFLAGS += $(HEADERS) $(WE_HEADER)

LIBS=$(LIBM)

all:: $(PROGS)

# .c.o:
#	$(CC) $(CFLAGS) -c $<

iwconfig: iwconfig.o iwcommon.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LDLIBS)

iwlist: iwlist.o iwcommon.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LDLIBS)

iwpriv: iwpriv.o iwcommon.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LDLIBS)

iwspy: iwspy.o iwcommon.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(LDLIBS)

iwgetid: iwgetid.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

romfs:
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_TOOLS_IWCONFIG /bin/iwconfig
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_TOOLS_IWLIST   /bin/iwlist
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_TOOLS_IWPRIV   /bin/iwpriv
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_TOOLS_IWSPY    /bin/iwspy
	$(ROMFSINST) -e CONFIG_USER_WIRELESS_TOOLS_IWGETID  /bin/iwgetid

# So crude but so effective ;-)
install::
	cp $(PROGS) $(INSTALL_DIR)

clean::
	$(RM_CMD) 

realclean::
	$(RM_CMD) 
	$(RM) $(PROGS)

depend::
	makedepend -s "# DO NOT DELETE" -- $(INCLUDES) -- $(SRCS)
# DO NOT DELETE
