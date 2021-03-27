# Generated automatically from Makefile.in by configure.
#####################################################
# Makefile for stunnel by Michal Trojnara 1998-2001 #
#                                                   #
# Modified by Brian Hatch <bri@stunnel.org>         #
#####################################################

# Hacked by hand for nettel-x86 [sra]

prefix=/usr/local
exec_prefix=${prefix}
sbindir=${exec_prefix}/sbin
libdir=${exec_prefix}/lib
man8dir=${prefix}/man/man8
piddir=/var/run/
ssldir=${ROOTDIR}/lib/openssl

PEM_DIR=/etc/config

VERSION=stunnel-3.14
RANDOM_OPT="-rand /dev/urandom"

CFLAGS += -g -O2 -Wall \
	-I${ROOTDIR}/lib/openssl/include \
	-DVERSION=\"3.14\" \
	-DHAVE_OPENSSL=1 \
	-Dssldir=\"${ROOTDIR}/lib/openssl\" \
	-DPEM_DIR=\"/etc/config\" \
	-DPEM_DIR=\"/etc/config\" \
	-DCERT_FILE=\"/etc/config/certs.pem\" \
	-DCERT_DIR=\"/etc/config/certs\" \
	-DRANDOM_FILE=\"/dev/urandom\" \
	-DSSLLIB_CS=0 \
	-DHOST=\"i686-pc-linux-gnu\" \
	-DHAVE_LIBDL=1 \
	-DHAVE_LIBNSL=1 \
	-DHAVE_LIBPTHREAD=0 \
	-DHAVE_LIBUTIL=1 \
	-DHAVE_LIBWRAP=0 \
	-DHAVE_DEV_PTMX=1 \
	-DHAVE_GETOPT_H=1 \
	-DHAVE_UNISTD_H=1 \
	-DHAVE_SYS_SELECT_H=1 \
	-DHAVE_TCPD_H=0 \
	-DHAVE_PTHREAD_H=0 \
	-DHAVE_PTY_H=1 \
	-DHAVE_STROPTS_H=1 \
	-DHAVE_GETOPT=1 \
	-DHAVE_SNPRINTF=1 \
	-DHAVE_VSNPRINTF=1 \
	-DHAVE_OPENPTY=1 \
	-DHAVE_DAEMON=1 \
	-DHAVE_WAITPID=1 \
	-DSIZEOF_UNSIGNED_CHAR=1 \
	-DSIZEOF_UNSIGNED_SHORT=2 \
	-DSIZEOF_UNSIGNED_INT=4 \
	-DSIZEOF_UNSIGNED_LONG=4 \
	-DSIZEOF_UNSIGNED_LONG_LONG=8 \
	-Dlibdir=\"$(libdir)\" \
	-DPIDDIR=\"$(piddir)\"

# Some of the shared libraries built for NETtel aren't installed on NETtel,
# presumably to save space.  For now, just staticly link these, since this
# is the only application using them.  Fix later if necessary.
#
#LDLIBS += -lutil -lpthread -lnsl -ldl -L${ROOTDIR}/lib/openssl -lssl -lcrypto
#LDLIBS += -lutil -lnsl -ldl -L${ROOTDIR}/lib/openssl -lssl -lcrypto
#
LDLIBS  += ${ROOTDIR}/lib/usr/lib/libutil.a -lnsl -ldl -L${ROOTDIR}/lib/openssl -lssl -lcrypto

OBJS=stunnel.o ssl.o protocol.o sthreads.o pty.o log.o

all: stunnel

clean:
	rm -f stunnel stunnel.so $(OBJS) core config.log

cert:
	rm -f stunnel.pem
	$(MAKE) stunnel.pem

stunnel: $(OBJS)
	${LD} ${LDFLAGS} -o $@.elf $^ ${LDLIBS}
	${CONVERT}

stunnel.so: Makefile env.c
	$(CC) -fPIC -shared $(LDFLAGS) -o stunnel.so env.c $(LIBS) || \
	touch stunnel.so

$(OBJS): Makefile common.h

stunnel.pem: stunnel.cnf
	$(ssldir)/bin/openssl req -new -x509 -days 365 -nodes \
		-config stunnel.cnf -out stunnel.pem -keyout stunnel.pem
	$(ssldir)/bin/openssl x509 -subject -dates -fingerprint -noout \
		-in stunnel.pem
