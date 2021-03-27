
EXEC = init
OBJS = init.o expand.o

ifdef BUILD_BIG
CFLAGS += -DBUILD_BIG
endif
ifdef BUILD_ELITE
CFLAGS += -DBUILD_ELITE
endif
ifdef BUILD_NETtel
CFLAGS += -DBUILD_NETtel
endif
ifdef BUILD_NETtel1500
CFLAGS += -DBUILD_NETtel
endif
ifdef BUILD_CADRE3
CFLAGS += -DBUILD_CADRE3
endif
ifdef CONSOLE_BAUD_RATE
CFLAGS += -DCONSOLE_BAUD_RATE=$(CONSOLE_BAUD_RATE)
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
