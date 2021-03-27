
#EXECS = flashloader himemloader reset
EXECS = flashloader reset

#OBJS = flashloader.o himemloader.o reset.o
OBJS = flashloader.o reset.o

CFLAGS += -I./include

all: $(EXECS)

$(EXECS): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $@.o $(LDLIBS)

romfs:
	$(ROMFSINST) -e CONFIG_USER_BOOTTOOLS_FLASHLOADER  /bin/flashloader
	$(ROMFSINST) -e CONFIG_USER_BOOTTOOLS_HIMEMLOADER  /bin/himemloader

clean:
	rm -f $(EXECS) *.elf *.gdb *.[oa] *~ core 
