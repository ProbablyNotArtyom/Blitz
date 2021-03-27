#
# Makefile for libcrypt_old
#

CFLAGS += -Wall

all: libcrypt_old.a

libcrypt_old.a: crypt.o crypt_old.o
	$(AR) crv $@ crypt.o crypt_old.o
	$(RANLIB) $@

.PHONY: romfs
romfs:

.PHONY: clean
clean:
	-rm -f *.o libcrypt_old.a
