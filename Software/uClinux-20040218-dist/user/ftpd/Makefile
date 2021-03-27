
EXEC = ftpd
OBJS = auth.o conf.o ftpcmd.o ftpd.o popen.o server_mode.o localhost.o xgetcwd.o logwtmp.o xmalloc.o

CFLAGS += -DHAVE_CONFIG_H=1 -I.
CFLAGS += -DPATH_FTPWELCOME=\"/etc/ftpwelcome\"
CFLAGS += -DPATH_FTPLOGINMESG=\"/etc/motd\"
CFLAGS += -DPATH_FTPUSERS=\"/etc/ftpusers\"
CFLAGS += -DPATH_BSHELL=\"/bin/sh\"
CFLAGS += -DPATH_FTPCHROOT=\"/usr/sbin/chroot\"
CFLAGS += -DPATH_FTPDPID=\"/var/run/ftpd.pid\"
CFLAGS += -DPATH_DEVNULL=\"/dev/null\"

ifeq ($(findstring glibc,$(LIBCDIR)),glibc)
CFLAGS += -DPATH_NOLOGIN=\"/etc/nologin\"
CFLAGS += -DPATH_WTMP=\"/etc/wtmp\"
CFLAGS += -DHAVE_ERR
endif

CFLAGS += -DKEEP_OPEN=1

EXTRALIBS = $(LIBCRYPT)


all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(EXTRALIBS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.elf2flt *.gdb *.o

