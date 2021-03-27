
EXEC = brcfg
OBJS = brcfg.o

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -s -o $@ $(OBJS) $(LDLIBS)

clean:
	rm -f $(EXEC) *.o *.gdb *.elf
