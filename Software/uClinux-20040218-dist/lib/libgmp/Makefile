
.EXPORT_ALL_VARIABLES:

# VPATH = $(ROOTDIR)/freeswan/gmp
# CFLAGS += -I../libc/include -DHAVE_CONFIG_H
CFLAGS += -DHAVE_CONFIG_H
srcdir = $(ROOTDIR)/lib/libgmp

SRCS = assert.c errno.c extract-dbl.c insert-dbl.c memory.c mp_bpl.c \
       mp_clz_tab.c mp_set_fns.c rand.c randclr.c randlc.c randlc2x.c \
       randraw.c randsd.c randsdui.c stack-alloc.c version.c
OBJS = assert.o errno.o extract-dbl.o insert-dbl.o memory.o mp_bpl.o \
       mp_clz_tab.o mp_set_fns.o rand.o randclr.o randlc.o randlc2x.o \
       randraw.o randsd.o randsdui.o stack-alloc.o version.o

FILES = gmp.h mp.h gmp-impl.h longlong.h urandom.h move-if-change \
 mkinstalldirs INSTALL COPYING.LIB ChangeLog Makefile.in \
 NEWS README SPEED TODO config.guess config.sub configure configure.in \
 gmp.info* gmp.texi texinfo.tex $(SRCS)

H = $(srcdir)/gmp.h $(srcdir)/gmp-impl.h mpn/gmp-mparam.h

INCLUDES = -I. -Impn -I$(srcdir)
CFLAGS += $(INCLUDES)

# FLAGS_TO_PASS = "CC=$(CC)" "CFLAGS=$(CFLAGS)" "XCFLAGS=$(XCFLAGS)"

all: libgmp.a

libgmp.a: mpn/libmpn.a mpz/libmpz.a mpf/libmpf.a mpq/libmpq.a $(OBJS)
	rm -rf tmpdir
	mkdir tmpdir
	for i in mpn mpz mpf mpq; \
	  do \
	    mkdir tmpdir/$$i; \
	    ( cd tmpdir/$$i; $(AR) x ../../$$i/lib$$i.a ); \
	  done
	cp $(OBJS) tmpdir
	cd tmpdir; $(AR) cr $@ *.o */*.o
	-$(RANLIB) tmpdir/$@
	mv tmpdir/$@ .
	rm -rf tmpdir

libmp.a: mpn/libmpn.a mpbsd/libmpbsd.a $(OBJS)
	rm -rf tmpdir
	mkdir tmpdir
	for i in mpn mpbsd; \
	  do \
	    mkdir tmpdir/$$i; \
	    ( cd tmpdir/$$i; $(AR) x ../../$$i/lib$$i.a ); \
	  done
	cp $(OBJS) tmpdir
	cd tmpdir; $(AR) cr $@ *.o */*.o
	-$(RANLIB) tmpdir/$@
	mv tmpdir/$@ .
	rm -rf tmpdir

.PHONY: force

force:

mpn/libmpn.a: force
	$(MAKE) -C mpn $(FLAGS_TO_PASS) libmpn.a

mpz/libmpz.a: force
	$(MAKE) -C mpz $(FLAGS_TO_PASS) libmpz.a

mpf/libmpf.a: force
	$(MAKE) -C mpf $(FLAGS_TO_PASS) libmpf.a

mpq/libmpq.a: force
	$(MAKE) -C mpq $(FLAGS_TO_PASS) libmpq.a

mpbsd/libmpbsd.a: force
	$(MAKE) -C mpbsd $(FLAGS_TO_PASS) libmpbsd.a

romfs:

clean: force
	-rm -f *.o libgmp.a libmp.a gmp.dvi gmp.ps tmp.* tmp-*
	-rm -f gmp.?? gmp.??s gmp.log gmp.toc gmp.*aux gmp*.html
	-$(MAKE) -C mpn $@
	-$(MAKE) -C mpz $@
	-$(MAKE) -C mpf $@
	-$(MAKE) -C mpq $@
	-$(MAKE) -C mpbsd $@

memory.o: $(srcdir)/memory.c $(H)
mp_bpl.o: $(srcdir)/mp_bpl.c
mp_clz_tab.o: $(srcdir)/mp_clz_tab.c
mp_set_fns.o: $(srcdir)/mp_set_fns.c $(H)
stack-alloc.o: $(srcdir)/stack-alloc.c $(srcdir)/stack-alloc.h
version.o: $(srcdir)/version.c


