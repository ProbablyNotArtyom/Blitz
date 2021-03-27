#CFLAGS=-Os -fomit-frame-pointer
VERSION=$(shell head -1 CHANGES|sed 's/://')
CFLAGS += -Wall
CFLAGS += -DFNORD=\"fnord/$(VERSION)\"
CFLAGS += -DCGI
FLTFLAGS += -s 15000
ifdef CONFIG_USER_TINYLOGIN_SHADOWPASSWDS
CFLAGS += -DSHADOW_AUTH
endif

ifdef CONFIG_USER_FLATFSD_FLATFSD
CFLAGS += -DCONFIG_USER_FLATFSD_FLATFSD=1
endif

ifdef CONFIG_USER_OLD_PASSWORDS
CFLAGS += -DOLD_CONFIG_PASSWORDS
endif

# For debugging...
#LDLIBS=$(SLIBC)
#CFLAGS += -g -DDEBUG

OBJS=httpd.o

# These are for auth
CFLAGS += -DUSE_AUTH
OBJS += auth.o base64.o

all: httpd

httpd: $(OBJS) libowfat.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBCRYPT) $(LDLIBS)

libowfat.a: httpd.o buffer_1.o buffer_puts.o buffer_flush.o buffer_put.o \
buffer_putulong.o buffer_2.o buffer_putspace.o buffer_stubborn.o \
buffer_putflush.o str_copy.o fmt_ulong.o byte_diff.o byte_copy.o \
str_len.o str_diff.o str_chr.o str_diffn.o str_start.o scan_ulong.o
	ar cru $@ $^
	-ranlib $@

.PHONY: rename clean install server
server:
	tcpserver -v -RHl localhost 0 8000 ./httpd

clean:
	-rm -f *.[oa] httpd *.gdb *.elf

romfs:
	$(ROMFSINST) /bin/httpd
	echo "80 stream tcp nowait root /bin/httpd /home/httpd" >>$(ROMFSDIR)/etc/default/inetd.conf
	echo "80 stream tcp nowait root /bin/httpd /home/httpd" >>$(ROMFSDIR)/etc/default-static/inetd.conf
ifdef CONFIG_USER_SSLWRAP_SSLWRAP
	echo "443 stream tcp nowait root /bin/sslwrap -cert /etc/config/ssl_cert.pem -key /etc/config/ssl_key.pem -exec /bin/httpd /home/httpd" >>$(ROMFSDIR)/etc/default/inetd.conf
	echo "443 stream tcp nowait root /bin/sslwrap -cert /etc/config/ssl_cert.pem -key /etc/config/ssl_key.pem -exec /bin/httpd /home/httpd" >>$(ROMFSDIR)/etc/default-static/inetd.conf
endif
