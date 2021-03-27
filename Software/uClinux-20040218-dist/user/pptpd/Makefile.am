# We have our own, simpler dependencies stuff in 'reconf'
AUTOMAKE_OPTIONS = no-dependencies foreign

@SET_MAKE@

##LIBFILES = -lxxx
INCLUDES = -I.
## Change this if you don't have gcc
## -pedantic removed for now (OpenBSD header files).
## -Werror removed for now (getopt stuff on OSF/1 throws a
##  warning with -Wmissing-prototypes).
## -Wmissing-prototypes removed (eg, Linux 2.2.6 headers
##  aren't up to it).
CFLAGS = -O2 -fno-builtin -Wall -ansi -DSBINDIR='"$(sbindir)"'
#CFLAGS = -O2 -fno-builtin -Wall -ansi -pedantic -Wmissing-prototypes -Werror -DSBINDIR='"$(sbindir)"'

man_MANS = pptpctrl.8 pptpd.8 pptpd.conf.5
	
EXTRA_DIST = README.inetd README.slirp ChangeLog-0.8 \
	version reconf Makefile.uClinux config.embed.h \
	CVS html samples $(man_MANS)

dist-hook:
	for file in `find $(distdir) -name Root -print` ; do \
		cp -pf $(srcdir)/Anon-CVS-Root $$file ; \
	done

sbin_PROGRAMS = pptpd pptpctrl

# Header files are only used to determine what to put in a distribution, not
# for dependencies, so just attribute them all to pptpd.

pptpd_SOURCES = \
	pptpd.c configfile.c pptpmanager.c compat.c getopt.c getopt1.c \
	compat.h configfile.h ctrlpacket.h defaults.h inststr.h our_getopt.h \
	our_syslog.h ppphdlc.h pptpctrl.h pptpdefs.h pptpgre.h pptpmanager.h

pptpctrl_SOURCES = \
	pptpctrl.c ctrlpacket.c inststr.c compat.c pptpgre.c ppphdlc.c


pptpd_LDADD = $(XTRALIBS_MGR)

pptpctrl_LDADD = $(XTRALIBS_CTRL)

##CLEANFILES = 

## DO NOT ADD BELOW THIS POINT, DEPS ARE AUTOMATICALLY ADDED
compat.o: compat.c config.h compat.h our_syslog.h

configfile.o: configfile.c config.h defaults.h configfile.h

ctrlpacket.o: ctrlpacket.c config.h our_syslog.h pptpdefs.h pptpctrl.h \
 ctrlpacket.h compat.h

getopt.o: getopt.c config.h our_getopt.h

getopt1.o: getopt1.c config.h our_getopt.h

inststr.o: inststr.c config.h inststr.h compat.h

ppphdlc.o: ppphdlc.c config.h ppphdlc.h

pptpctrl.o: pptpctrl.c config.h our_syslog.h inststr.h compat.h \
 pptpctrl.h pptpgre.h pptpdefs.h ctrlpacket.h defaults.h

pptpd.o: pptpd.c config.h our_syslog.h configfile.h defaults.h \
 compat.h our_getopt.h pptpmanager.h

pptpgre.o: pptpgre.c config.h our_syslog.h ppphdlc.h pptpgre.h \
 pptpdefs.h defaults.h compat.h

pptpmanager.o: pptpmanager.c config.h our_syslog.h configfile.h \
 defaults.h pptpctrl.h pptpdefs.h pptpmanager.h compat.h

