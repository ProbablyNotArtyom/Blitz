# This makefile builds 2 executables. One with IDENT support, (RFC931)
# and one without. Rename the one you want to klaxon and install it as
# usual.
# mine.c is included if you wish to statically link this program
# on Solaris2.X machines, just cc -c it and then link it with
# the executable

# - known to compile cleanly on linux 1.2.13 kernel of slackware 3.0

# Change the line below if you don't like the 3 second timeout
# For finding out IDENT information
TIMEOUT=-DRFC931_TIMEOUT=3

# Uncomment the below lines for RPC rexd support only on Solaris2.X
# CFLAGS = -DRPC
# LDFLAGS = -lrpcsvc

EXEC = klaxon
OBJS = klaxon.o

# Stuff to add for rfc931 support
ifeq ($(CONFIG_USER_KLAXON_RFC931),y)
OBJS += rfc931.o
CFLAGS += $(TIMEOUT)
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

clean:
	-rm -f $(EXEC) *.gdb *.o

romfs:
	$(ROMFSINST) /bin/$(EXEC)
