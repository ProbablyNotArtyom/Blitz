######################################################

EXEC = flatfsd
OBJS = flatfsd.o newfs.o reboot.o
# FLTFLAGS += -s 2048

# force error checking on compile
#CFLAGS += -Wall -Werror
CFLAGS += -Wall

# CFLAGS += -DDEBUG

######################################################
#
# Erk! settings to change the way flatfsd works
#
# Explicit setting overrides autodetect
USING_FLASH_FILESYSTEM_$(CONFIG_USER_FLATFSD_USE_FLASH_FS) = y
USING_DISKLIKE_DEVICE_$(CONFIG_USER_FLATFSD_DISKLIKE) = y
ifdef CONFIG_USER_FLATFSD_AUTO
	ifdef CONFIG_NFTL
		ifndef CONFIG_MTD_CFI_INTELEXT
			USING_DISKLIKE_DEVICE_$(CONFIG_SH_SECUREEDGE5410) = y
		endif
	endif
	ifeq ($(USING_DISKLIKE_DEVICE_y),)
		USING_MTD_DEVICE_$(CONFIG_MTD) = y
		USING_BLKMEM_DEVICE_$(CONFIG_BLK_DEV_BLKMEM) = y
	endif
endif

######################################################

ifdef USING_DISKLIKE_DEVICE_y
CFLAGS += -DUSING_DISKLIKE_DEVICE=1
OBJS += flatfs.o flatfs_disk.o
else
ifdef USING_MTD_DEVICE_y
CFLAGS += -DUSING_MTD_DEVICE=1
OBJS += flatfs.o flatfs_mtd.o
else
ifdef USING_BLKMEM_DEVICE_y
CFLAGS += -DUSING_BLKMEM_DEVICE=1
OBJS += flatfs.o flatfs_blkmem.o
else
ifdef USING_FLASH_FILESYSTEM_y
CFLAGS += -DUSING_FLASH_FILESYSTEM=1
else
all:: ERROR-no-flatfsd-storage-type-defined
endif
endif
endif
endif

ifdef CONFIG_PROP_LOGD_LOGD
CFLAGS += -DLOGGING
endif


######################################################

all:: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

######################################################
