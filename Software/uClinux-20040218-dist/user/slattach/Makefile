
EXEC = slattach
OBJS = slattach.o pinger.o route.o

CFLAGS += -I../pppd.small/pppd $(INCNET)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBNET) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.gdb *.elf *.o
