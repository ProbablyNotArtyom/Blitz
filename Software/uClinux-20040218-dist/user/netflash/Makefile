
EXEC = netflash
OBJS = tftpmain.o tftp.o tftpsubs.o http.o
COBJS = netflash.o

ifdef CONFIG_USER_NETFLASH_WITH_FTP
CFLAGS += -DFTP
OBJS += cmds.o cmdtab.o domacro.o ftp.o glob.o ftpmain.o ruserpass.o
endif

ifdef CONFIG_USER_NETFLASH_HMACMD5
EXEC += hmacmd5
OBJS += md5.o hmacmd5.o
endif

CFLAGS+= -DCONFIG_VERSION=\"$(VERSIONPKG)\"
COBJS += versioning.o

ifdef CONFIG_USER_NETFLASH_DECOMPRESS
CFLAGS   += $(INCZ)
ADD_LIBZ  = -lz
endif

ifdef CONFIG_USER_NETFLASH_SETSRC
CFLAGS += -DSETSRC
endif

ifdef CONFIG_USER_NETFLASH_CRYPTO
CFLAGS += $(INCAES) $(INCSSL)
ADD_LIBZ += $(LIBAES) $(LIBCRYPTO)
endif

CFLAGS += -Dnetflashmain=main


all: $(EXEC)

netflash: $(OBJS) $(COBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(COBJS) $(ADD_LIBZ) $(LDLIBS$(LDLIBS_$@))

netflash.o: netflash.c $(ROOTDIR)/config.arch
	echo $(COBJS)
	$(CC) -c -o $@ netflash.c $(CFLAGS)

versioning.o: versioning.c $(ROOTDIR)/config.arch
	$(CC) -c -o $@ versioning.c $(CFLAGS)

hmacmd5: hmacmd5main.c hmacmd5.c md5.c
	gcc -o $@ $^

romfs:
	$(ROMFSINST) /bin/netflash

clean:
	-rm -f netflash hmacmd5 *.elf *.gdb *.o

