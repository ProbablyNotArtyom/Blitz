# ROOTDIR = $(UCLINUX)
# UCLINUX_BUILD_USER = 1
# include $(ROOTDIR)/.config
# LINUXDIR = $(CONFIG_LINUXDIR)
# LIBCDIR = $(CONFIG_LIBCDIR)
# include $(ROOTDIR)/config.arch

CFLAGS += -Wall

EXEC = kendin-config
OBJS = kendin-config.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-elf2flt -lc

clean:
	rm -f $(EXEC) $(OBJS) *.gdb

romfs: $(EXEC)
	$(ROMFSINST) /bin/$(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
