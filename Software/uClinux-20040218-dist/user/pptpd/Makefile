
PACKAGE = pptpd

EXEC1 = pptpd
OBJS1 = pptpd.o configfile.o pptpmanager.o compat.o $(LIBRESOLV)

EXEC2 = pptpctrl
OBJS2 = pptpctrl.o ctrlpacket.o inststr.o compat.o pptpgre.o ppphdlc.o

CFLAGS += -include config.embed.h -DPACKAGE='"$(PACKAGE)"' -DVERSION='"$(pptpd.VERSION).$(pptpd.PATCHLEVEL).$(pptpd.SUBLEVEL)"'
FLTFLAGS += -s 8192

ifdef BUILD_NETtel
CFLAGS += -DCONFIG_NETtel
endif
ifdef BUILD_NETtel1500
CFLAGS += -DCONFIG_NETtel
endif


all: $(EXEC1) $(EXEC2)

$(EXEC1):	$(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LDLIBS)

$(EXEC2):	$(OBJS2)
	$(CC) $(LDFLAGS) -o $@ $(OBJS2) $(LDLIBS)

romfs:
	$(ROMFSINST) -e CONFIG_USER_PPTPD_PPTPD /bin/pptpd
	$(ROMFSINST) -e CONFIG_USER_PPTPD_PPTPCTRL /bin/pptpctrl

clean:
	-rm -f $(EXEC1) $(EXEC2) *.elf *.gdb *.o

## DO NOT ADD BELOW THIS POINT, DEPS ARE AUTOMATICALLY ADDED
compat.o: compat.c compat.h our_syslog.h

configfile.o: configfile.c defaults.h configfile.h

ctrlpacket.o: ctrlpacket.c our_syslog.h pptpdefs.h pptpctrl.h \
 ctrlpacket.h compat.h

inststr.o: inststr.c inststr.h compat.h

ppphdlc.o: ppphdlc.c ppphdlc.h

pptpctrl.o: pptpctrl.c our_syslog.h inststr.h compat.h pptpctrl.h \
 pptpgre.h pptpdefs.h ctrlpacket.h defaults.h

pptpd.o: pptpd.c our_syslog.h configfile.h defaults.h compat.h pptpmanager.h

pptpgre.o: pptpgre.c our_syslog.h ppphdlc.h pptpgre.h pptpdefs.h \
 defaults.h compat.h

pptpmanager.o: pptpmanager.c our_syslog.h configfile.h defaults.h \
 pptpctrl.h pptpdefs.h pptpmanager.h compat.h

