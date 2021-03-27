
ROUTE = route
ROUTEOBJS = route.o

IFCONFIG = ifconfig
IFCONFIGOBJS = ifconfig.o

ARP = arp
ARPOBJS = arp.o

NETSTAT = netstat
NETSTATOBJS = netstat.o

MIITOOL = mii-tool
MIITOOLOBJS = mii-tool.o

CFLAGS += -D_GNU_SOURCE -D__USE_BSD -Iinclude -Ilib -I../pppd/pppd
CFLAGS += $(INCNET)
LIBROUTE = lib/libroute.a

all: libs $(ROUTE) $(IFCONFIG) $(ARP) $(MIITOOL)

$(ROUTE): $(ROUTEOBJS) $(LIBROUTE)
	$(CC) $(LDFLAGS) -o $@ $(ROUTEOBJS) $(LIBROUTE) $(LIBNET) $(LDLIBS)

$(IFCONFIG): $(IFCONFIGOBJS) $(LIBROUTE)
	$(CC) $(LDFLAGS) -o $@ $(IFCONFIGOBJS) $(LIBROUTE) $(LIBNET) $(LDLIBS)

$(ARP): $(ARPOBJS) $(LIBROUTE) 
	$(CC) $(LDFLAGS) -o $@ $(ARPOBJS) $(LIBROUTE) $(LIBNET) $(LDLIBS)

$(NETSTAT): $(NETSTATOBJS) $(LIBROUTE) 
	$(CC) $(LDFLAGS) -o $@ $(NETSTATOBJS) $(LIBROUTE) $(LIBNET) $(LDLIBS)

$(MIITOOL): $(MIITOOLOBJS) $(LIBROUTE) 
	$(CC) $(LDFLAGS) -o $@ $(MIITOOLOBJS) $(LIBROUTE) $(LIBNET) $(LDLIBS)

romfs:
	$(ROMFSINST) -e CONFIG_USER_ROUTE_ROUTE    /bin/route
	$(ROMFSINST) -e CONFIG_USER_ROUTE_IFCONFIG /bin/ifconfig
	$(ROMFSINST) -e CONFIG_USER_ROUTE_ARP      /bin/arp
	$(ROMFSINST) -e CONFIG_USER_ROUTE_NETSTAT  /bin/netstat
	$(ROMFSINST) -e CONFIG_USER_ROUTE_MIITOOL  /bin/mii-tool

clean:
	-rm -f $(ROUTE) $(IFCONFIG) $(ARP) $(NETSTAT) $(MIITOOL) *.elf *.gdb *.o
	-$(MAKE) -C lib clean

.PHONY: libs
libs:
	$(MAKE) -C lib


