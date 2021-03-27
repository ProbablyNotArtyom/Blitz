# Generated automatically from Makefile.in by configure.
FLTFLAGS += -s 65536
ifeq ($(CONFIG_PRODUCT),AmazonSME550)
CFLAGS += -DCONFIG_AMAZON
endif


prefix=
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
sbindir=${exec_prefix}/sbin
libexecdir=${exec_prefix}/libexec
mandir=${prefix}/man
mansubdir=man
sysconfdir=${prefix}/etc/config
piddir=/var/run
srcdir=.
top_srcdir=.

DESTDIR=


SSH_PROGRAM=${exec_prefix}/bin/ssh
ASKPASS_LOCATION=${exec_prefix}/libexec/ssh
ASKPASS_PROGRAM=$(ASKPASS_LOCATION)/ssh-askpass

# CC=gcc
# LD=gcc
PATHS= -D_SSH_PROGRAM=\"/bin/ssh\" -DSSH_ASKPASS_DEFAULT=\"/bin/ssh-askpass\"
CFLAGS += -g -O2 -Wall $(INCSSL) $(PATHS) -I. -DHAVE_CONFIG_H
LIBS=$(LIBCRYPTO) -lz
# AR=ar
# RANLIB=ranlib
INSTALL=/usr/bin/install -c
PERL=/usr/bin/perl
ENT=
LDFLAGS += -L. -Lopenbsd-compat

INSTALL_SSH_PRNG_CMDS=

TARGETS= sshd ssh ssh-keygen gen-keys
# ssh ssh-add ssh-keygen ssh-agent scp $(EXTRA_TARGETS)

LIBSSH_OBJS=atomicio.o authfd.o authfile.o bufaux.o buffer.o canohost.o channels.o cipher.o cipher-ctr.o cipher-3des1.o cipher-bf1.o compat.o compress.o crc32.o deattack.o dh.o dispatch.o fatal.o mac.o msg.o hostfile.o key.o kex.o kexdhs.o kexdh.o kexdhc.o kexgex.o log.o match.o misc.o mpaux.o nchan.o packet.o entropy.o readpass.o rsa.o scard.o scard-opensc.o ssh-dss.o ssh-rsa.o tildexpand.o ttymodes.o uidswap.o uuencode.o xmalloc.o monitor_wrap.o monitor_fdpass.o kexgexs.o kexgexc.o

SSHOBJS= ssh.o sshconnect.o sshconnect1.o sshconnect2.o sshtty.o readconf.o clientloop.o

SSHDOBJS= sshd.o auth.o auth1.o auth2.o auth2-hostbased.o auth2-kbdint.o auth2-none.o auth2-passwd.o auth2-pubkey.o auth-chall.o auth2-chall.o auth-rhosts.o auth-options.o auth-krb5.o auth-pam.o auth-passwd.o auth-rsa.o auth-rh-rsa.o auth-sia.o sshpty.o sshlogin.o loginrec.o servconf.o serverloop.o md5crypt.o session.o groupaccess.o auth-skey.o auth-bsdauth.o monitor_mm.o monitor.o


TROFFMAN	= scp.1 ssh-add.1 ssh-agent.1 ssh-keygen.1 ssh.1 sshd.8
CATMAN		= scp.0 ssh-add.0 ssh-agent.0 ssh-keygen.0 ssh.0 sshd.0
MANPAGES	= $(TROFFMAN)

CONFIGFILES=sshd_config ssh_config

PATHSUBS	= -D/etc/ssh_config=$(sysconfdir)/ssh_config -D/etc/known_hosts=$(sysconfdir)/ssh_known_hosts -D/etc/sshd_config=$(sysconfdir)/sshd_config -D/etc/shosts.equiv=$(sysconfdir)/shosts.equiv -D/etc/ssh_host_key=$(sysconfdir)/ssh_host_key -D/var/run/sshd.pid=$(piddir)/sshd.pid

FIXPATHSCMD	= $(PERL) $(srcdir)/fixpaths $(PATHSUBS)

all: subdirs $(TARGETS) $(CONFIGFILES)

manpages: $(MANPAGES)

$(LIBSSH_OBJS): config.h

$(LIBOPENBSD_COMPAT_OBJS): config.h

LIBCOMPAT=openbsd-compat/libopenbsd-compat.a
.PHONY: subdirs
subdirs:
	(cd openbsd-compat && $(MAKE))

libssh.a: $(LIBSSH_OBJS)
	$(AR) r $@ $(LIBSSH_OBJS)
	$(RANLIB) $@

ssh: $(LIBCOMPAT) libssh.a $(SSHOBJS)
	$(CC) $(LDFLAGS) -o $@ $(SSHOBJS) -lssh -lopenbsd-compat $(LIBS) $(LDLIBS)

sshdch: $(LIBCOMPAT) libssh.a $(SSHDCHOBJS)
	$(CC) $(LDFLAGS) -o $@ $(SSHDCHOBJS) -lssh -lopenbsd-compat $(LIBS) $(LDLIBS)

sshd: libssh.a	$(LIBCOMPAT) $(SSHDOBJS)
	$(CC) $(LDFLAGS) -o $@ $(SSHDOBJS) -lssh -lopenbsd-compat $(LIBS) $(LDLIBS)

scp: $(LIBCOMPAT) libssh.a scp.o
	$(CC) $(LDFLAGS) -o $@ scp.o -lssh -lopenbsd-compat $(LIBS) $(LDLIBS)

ssh-add: $(LIBCOMPAT) libssh.a ssh-add.o
	$(CC) $(LDFLAGS) -o $@ ssh-add.o -lssh -lopenbsd-compat $(LIBS) $(LDLIBS)

ssh-agent: $(LIBCOMPAT) libssh.a ssh-agent.o
	$(CC) $(LDFLAGS) -o $@ ssh-agent.o -lssh -lopenbsd-compat $(LIBS) $(LDLIBS)

ssh-keygen: $(LIBCOMPAT) libssh.a ssh-keygen.o moduli.o
	$(CC) $(LDFLAGS) -o $@ ssh-keygen.o moduli.o -lssh -lopenbsd-compat $(LIBS) $(LDLIBS)

# test driver for the loginrec code - not built by default
logintest: logintest.o $(LIBCOMPAT) libssh.a loginrec.o
	$(CC) $(LDFLAGS) -o $@ logintest.o loginrec.o -lopenbsd-compat -lssh $(LIBS) $(LDLIBS)

# The wrapper program that conditionally generates host keys
# We shrink the stack size post hoc since this one hardly needs
# any space.
ifeq ($(CONFIG_USER_SSH_SSHD),y)
GK_FLAGS=-DINCLUDE_SSHD
endif
ifeq ($(CONFIG_USER_SSH_SSH),y)
GK_FLAGS=-DINCLUDE_SSH
endif
gen-keys.o: gen-keys.c
	$(CC) $(CFLAGS) $(GK_FLAGS) -c gen-keys.c

gen-keys: gen-keys.o
	FLTFLAGS="-s 1000" \
	$(CC) $(LDFLAGS) -o $@ gen-keys.o $(LDLIBS)

romfs:
	$(ROMFSINST) -e CONFIG_USER_SSH_SSH /bin/ssh
	$(ROMFSINST) -e CONFIG_USER_SSH_SSH /etc/default/ssh_config
ifeq ($(CONFIG_USER_SSH_SSH),y)
	if test \! -e $(ROMFSDIR)/.ssh; then mkdir $(ROMFSDIR)/.ssh; fi
	chmod 700 $(ROMFSDIR)/.ssh
endif
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD /bin/sshd
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD /etc/default/sshd_config
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHKEYGEN /bin/ssh-keygen
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHKEYGEN /bin/gen-keys

	##
	# Add the appropriate stuff to the start files.
	#
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHKEYGEN \
		-a "gen-keys &" /etc/default/start
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
		-A "root:" -a "root:abTxyM1SdxNko:0:0:Root:/:/bin/sh" \
		/etc/default/passwd
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
		-A "root:" -a "root:x:0:root" /etc/default/group
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
		-A "sshd:" -a "sshd:x:100:65534::/var/run/sshd:/bin/false" \
		/etc/default/passwd
	$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
		-A "nobody:" -a "nobody:x:65534:" /etc/default/group

	##
	# Optionally add the startup stuff to the /etc/default-static dir.
	#
	if [ -d $(ROMFSDIR)/etc/default-static ] ; then \
		$(ROMFSINST) -e CONFIG_USER_SSH_SSHKEYGEN \
			-a "gen-keys &" /etc/default-static/start ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
			-a "root:abTxyM1SdxNko:0:0:Root:/:/bin/sh" -A "root:"\
			/etc/default-static/passwd ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)$(CONFIG_USER_SSH_SSH)" \
			-A "root:" -a "root:x:0:root" /etc/default-static/group ; \
		$(ROMFSINST) -e CONFIG_USER_SSH_SSH /etc/default-static/ssh_config ; \
		$(ROMFSINST) -e CONFIG_USER_SSH_SSHD /etc/default-static/sshd_config ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
			-A "sshd:" -a "sshd:x:100:65534::/var/run/sshd:/bin/false" \
			/etc/default-static/passwd ; \
		$(ROMFSINST) -o "$(CONFIG_USER_SSH_SSHD)" \
			-A "nobody:" -a "nobody:x:65534:" /etc/default-static/group ;\
	fi

clean:
	-(cd openbsd-compat && $(MAKE) clean)
	rm -f *.o *.a *.gdb *.elf $(TARGETS) logintest config.cache config.log 
	rm -f *.out core

distclean: clean
	rm -f Makefile config.h config.status ssh_prng_cmds *~

mrproper: distclean

veryclean: distclean
	rm -f configure config.h.in *.0

catman-do:
	@for f in $(TROFFMAN) ; do \
		echo "$$f -> $${f%%.[18]}.0" ; \
		nroff -mandoc $$f | cat -v | sed -e 's/.\^H//g' \
			>$${f%%.[18]}.0 ; \
	done

distprep: catman-do
	autoreconf

install: manpages $(TARGETS) install-files host-key

install-files:
	./mkinstalldirs $(DESTDIR)$(bindir)
	./mkinstalldirs $(DESTDIR)$(sbindir)
	./mkinstalldirs $(DESTDIR)$(mandir)
	./mkinstalldirs $(DESTDIR)$(mandir)/$(mansubdir)1
	./mkinstalldirs $(DESTDIR)$(mandir)/$(mansubdir)8
	$(INSTALL) -m 4755 -s ssh $(DESTDIR)$(bindir)/ssh
	$(INSTALL) -s scp $(DESTDIR)$(bindir)/scp
	$(INSTALL) -s ssh-add $(DESTDIR)$(bindir)/ssh-add
	$(INSTALL) -s ssh-agent $(DESTDIR)$(bindir)/ssh-agent
	$(INSTALL) -s ssh-keygen $(DESTDIR)$(bindir)/ssh-keygen
	$(INSTALL) -s sshd $(DESTDIR)$(sbindir)/sshd
	$(INSTALL) -m 644 ssh.[01].out $(DESTDIR)$(mandir)/$(mansubdir)1/ssh.1
	$(INSTALL) -m 644 scp.[01].out $(DESTDIR)$(mandir)/$(mansubdir)1/scp.1
	$(INSTALL) -m 644 ssh-add.[01].out $(DESTDIR)$(mandir)/$(mansubdir)1/ssh-add.1
	$(INSTALL) -m 644 ssh-agent.[01].out $(DESTDIR)$(mandir)/$(mansubdir)1/ssh-agent.1
	$(INSTALL) -m 644 ssh-keygen.[01].out $(DESTDIR)$(mandir)/$(mansubdir)1/ssh-keygen.1
	$(INSTALL) -m 644 sshd.[08].out $(DESTDIR)$(mandir)/$(mansubdir)8/sshd.8
	-rm -f $(DESTDIR)$(bindir)/slogin
	ln -s ssh $(DESTDIR)$(bindir)/slogin
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)1/slogin.1
	ln -s ssh.1 $(DESTDIR)$(mandir)/$(mansubdir)1/slogin.1
	if [ ! -f $(DESTDIR)$(sysconfdir)/ssh_config -a ! -f $(DESTDIR)$(sysconfdir)/sshd_config ]; then \
		./mkinstalldirs $(DESTDIR)$(sysconfdir); \
		$(INSTALL) -m 644 ssh_config.out $(DESTDIR)$(sysconfdir)/ssh_config; \
		$(INSTALL) -m 644 sshd_config.out $(DESTDIR)$(sysconfdir)/sshd_config; \
	fi
	if [ -f ssh_prng_cmds -a ! -z "$(INSTALL_SSH_PRNG_CMDS)" ]; then \
		$(PERL) fixprogs ssh_prng_cmds $(ENT); \
		$(INSTALL) -m 644 ssh_prng_cmds.out $(DESTDIR)$(sysconfdir)/ssh_prng_cmds; \
	fi

host-key: ssh-keygen
	if [ -z "$(DESTDIR)" ] ; then \
		if [ -f "$(DESTDIR)$(sysconfdir)/ssh_host_key" ] ; then \
			echo "$(DESTDIR)$(sysconfdir)/ssh_host_key already exists, skipping." ; \
		else \
			./ssh-keygen -b 1024 -f $(DESTDIR)$(sysconfdir)/ssh_host_key -N "" ; \
		fi ; \
		if [ -f $(DESTDIR)$(sysconfdir)/ssh_host_dsa_key ] ; then \
			echo "$(DESTDIR)$(sysconfdir)/ssh_host_dsa_key already exists, skipping." ; \
		else \
			./ssh-keygen -d -f $(DESTDIR)$(sysconfdir)/ssh_host_dsa_key -N "" ; \
		fi ; \
	fi ;

host-key-force: ssh-keygen
	./ssh-keygen -b 1024 -f $(DESTDIR)$(sysconfdir)/ssh_host_key -N ""
	./ssh-keygen -d -f $(DESTDIR)$(sysconfdir)/ssh_host_dsa_key -N ""

uninstallall:	uninstall
	-rm -f $(DESTDIR)$(sysconfdir)/ssh_config
	-rm -f $(DESTDIR)$(sysconfdir)/sshd_config
	-rm -f $(DESTDIR)$(sysconfdir)/ssh_prng_cmds
	-rmdir $(DESTDIR)$(sysconfdir)
	-rmdir $(DESTDIR)$(bindir)
	-rmdir $(DESTDIR)$(sbindir)
	-rmdir $(DESTDIR)$(mandir)/$(mansubdir)1
	-rmdir $(DESTDIR)$(mandir)/$(mansubdir)8
	-rmdir $(DESTDIR)$(mandir)
	-rmdir $(DESTDIR)$(libexecdir)

uninstall: 
	-rm -f $(DESTDIR)$(bindir)/ssh
	-rm -f $(DESTDIR)$(bindir)/scp
	-rm -f $(DESTDIR)$(bindir)/ssh-add
	-rm -f $(DESTDIR)$(bindir)/ssh-agent
	-rm -f $(DESTDIR)$(bindir)/ssh-keygen
	-rm -f $(DESTDIR)$(sbindir)/sshd
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)1/ssh.1
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)1/scp.1
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)1/ssh-add.1
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)1/ssh-agent.1
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)1/ssh-keygen.1
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)8/sshd.8
	-rm -f $(DESTDIR)$(bindir)/slogin
	-rm -f $(DESTDIR)$(mandir)/$(mansubdir)1/slogin.1
	-rm -f $(DESTDIR)${ASKPASS_PROGRAM}
	-rmdir $(DESTDIR)$(libexecdir)/ssh ;
