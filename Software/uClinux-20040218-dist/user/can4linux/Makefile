# Make some example applications using can4linux device driver
# (c) 2003 port GmbH oe@port.de
#

EXEC = ctest can_send
CTEST_OBJS = ctest.o
CANSEND_OBJS = can_send.o

CFLAGS += -I.
CFLAGS += -Wall

all: $(EXEC)

ctest: $(CTEST_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(CTEST_OBJS) $(LDLIBS$(LDLIBS-$(@)))

can_send: $(CANSEND_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(CANSEND_OBJS) $(LDLIBS$(LDLIBS-$(@)))



clean:
	rm -f $(EXEC) *.elf *.gdb *.o

romfs:
	$(ROMFSINST) /bin/can_send
	$(ROMFSINST) /bin/ctest
