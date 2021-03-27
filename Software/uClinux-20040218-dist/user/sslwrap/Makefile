
EXEC = sslwrap
OBJS = s_server.o s_socket.o s_cb.o

CFLAGS += $(INCSSL) -DFLAT_INC
LIBS += $(LIBSSL) $(LIBCRYPTO)

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
