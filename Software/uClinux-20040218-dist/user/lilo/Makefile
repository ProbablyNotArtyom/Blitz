EXEC = lilo
OBJS = lilo.o map.o geometry.o boot.o device.o common.o bsect.o cfg.o temp.o \
  partition.o identify.o probe.o shs2.o
BOOT =

CONFIG = -DLCF_IGNORECASE

ifdef CONFIG_M586
CONFIG_X86=y
endif
ifdef CONFIG_X86
BOOT = boot.b
CONFIG += -DHAS_BOOT_H -DLCF_M386 -DLCF_READONLY -DLCF_VARSETUP
AFLAGS = `cat mylilo.h`
CFLAGS += $(AFLAGS)
LILOX=-DCODE_START_1=136 -DCODE_START_2=14
endif

ifndef CONFIG_X86
CONFIG += -DNOSIZELIMIT=1
endif


CFLAGS += $(CONFIG)

.SUFFIXES: .img .b .com


all: $(EXEC) $(BOOT)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)
ifdef CONFIG_X86
	$(ROMFSINST) /boot/boot.b
endif

clean:
	-rm -f $(EXEC) *.b *.img *.s *.lis *.elf *.gdb *.o

ifdef CONFIG_X86
.s.o:
	$(AS86) -0 -a -w -l $*.lis -o $*.o $*.s

.o.img:
	$(LD86) -0 -s -o $*.img $*.o

.img.b:
	dd if=$*.img of=$*.b bs=32 skip=1

mbr.s: mbr.S read.S lilo.h mylilo.h
	$(CPP) -traditional $(CONFIG) $(AFLAGS) -o mbr.s  mbr.S

mbr.b: mbr.img
	dd if=$*.img of=$*.b bs=32 skip=49

mylilo.h: temp2.b
	echo -DMAX_SECONDARY_FILESIZE=`wc -c <temp2.b | sed "s/ //g"` >mylilo.h

boot.b: first.b second.b
	(dd if=first.b bs=512 conv=sync; dd if=second.b) >boot.b

common.s: common.h
	$(CPP) -C -traditional -DLILO_ASM -o common.s common.h

temp2.o: temp2.s common.s
second.o: second.s common.s
first.o: first.s common.s

first.s: first.S read.S lilo.h version.h lilo mylilo.h
	$(CPP) $(CONFIG) $(AFLAGS) `./lilo -X` first.S -o first.s

second.s: second.S read.S lilo.h version.h lilo mylilo.h
	$(CPP) $(CONFIG) $(AFLAGS) `./lilo -X` second.S -o second.s

temp2.s: second.S read.S lilo.h version.h
	$(CPP) $(CONFIG) $(LILOX) second.S -o temp2.s
endif
