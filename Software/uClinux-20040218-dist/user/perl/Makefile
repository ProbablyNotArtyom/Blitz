# Makefile.SH
# This file is derived from Makefile.SH.  Any changes made here will
# be lost the next time you run Configure.
#  Makefile is used to generate makefile.  The only difference
#  is that makefile has the dependencies filled in at the end.
#
#
# I now supply perly.c with the kits, so don't remake perly.c without byacc
BYACC = byacc
#CC = gcc
#LD = cc
#CFLAGS = 
#LDFLAGS =

#LDFLAGS =  -L/usr/local/lib
#CLDFLAGS =  -L/usr/local/lib

mallocsrc = 
mallocobj = 
LNS = /bin/ln -s
# NOTE: some systems don't grok "cp -f". XXX Configure test needed?
CPS = cp
RMS = rm -f
ranlib = :

# The following are mentioned only to make metaconfig include the
# appropriate questions in Configure.  If you want to change these,
# edit config.sh instead, or specify --man1dir=/wherever on
# installman commandline.
bin = /usr/bin
scriptdir = /usr/bin
shrpdir = /usr/lib/perl5/5.6.1/linux/CORE
privlib = /usr/lib/perl5/5.6.1
sitelib = /usr/lib/perl5/site_perl
man1dir = /usr/man/man1
man1ext = 1
man3dir = /usr/man/man3
man3ext = 3

# The following are used to build and install shared libraries for
# dynamic loading.
LDDLFLAGS = -shared -L/usr/local/lib
SHRPLDFLAGS = $(LDDLFLAGS)
CCDLFLAGS = -rdynamic
DLSUFFIX = .so
PLDLFLAGS = 
LIBPERL = libperl.a
LLIBPERL= $(LIBPERL)
SHRPENV = 

# The following is used to include the current directory in
# the dynamic loader path you are building a shared libperl.
LDLIBPTH = 

dynamic_ext =   lib/auto/attrs/attrs.so lib/auto/B/B.so lib/auto/ByteLoader/ByteLoader.so lib/auto/Data/Dumper/Dumper.so lib/auto/Devel/DProf/DProf.so lib/auto/Devel/Peek/Peek.so lib/auto/Fcntl/Fcntl.so lib/auto/File/Glob/Glob.so lib/auto/IO/IO.so lib/auto/IPC/SysV/SysV.so lib/auto/Opcode/Opcode.so lib/auto/POSIX/POSIX.so lib/auto/re/re.so lib/auto/SDBM_File/SDBM_File.so lib/auto/Socket/Socket.so lib/auto/Sys/Hostname/Hostname.so lib/auto/Sys/Syslog/Syslog.so
static_ext =  
nonxs_ext =   ext/Errno/pm_to_blib
ext = $(dynamic_ext) $(static_ext) $(nonxs_ext)
DYNALOADER = lib/auto/DynaLoader/DynaLoader$(LIB_EXT)

libs = -lnsl -ldl -lm -lc -lcrypt -lutil 

public = perl  utilities translators

shellflags = 

# This is set to  MAKE=make if your make command doesn't
# do it for you.
#

# These variables may need to be manually set for non-Unix systems.
AR = /usr/bin/ar
EXE_EXT = 
LIB_EXT = .a
OBJ_EXT = .o
PATH_SEP = 

FIRSTMAKEFILE = makefile

# Any special object files needed by this architecture, e.g. os2/os2.obj
ARCHOBJS = 

.SUFFIXES: .c $(OBJ_EXT)

# grrr
SHELL = /bin/sh

# how to tr(anslate) newlines
TRNL = '\n'


CCCMD = `sh $(shellflags) cflags $(LIBPERL) $@`

private = preplibrary lib/ExtUtils/Miniperl.pm lib/Config.pm

# Files to be built with variable substitution before miniperl
# is available.
sh = Makefile.SH cflags.SH config_h.SH makeaperl.SH makedepend.SH \
	makedir.SH myconfig.SH writemain.SH pod/Makefile.SH

shextract = Makefile cflags config.h makeaperl makedepend \
	makedir myconfig writemain pod/Makefile

# Files to be built with variable substitution after miniperl is
# available.  Dependencies handled manually below (for now).

pl = pod/pod2html.PL pod/pod2latex.PL pod/pod2man.PL pod/pod2text.PL \
	pod/pod2usage.PL pod/podchecker.PL pod/podselect.PL \
	pod/buildtoc.PL

plextract = pod/pod2html pod/pod2latex pod/pod2man pod/pod2text \
	pod/pod2usage pod/podchecker pod/podselect \
	pod/buildtoc

addedbyconf = UU $(shextract) $(plextract) pstruct

h1 = EXTERN.h INTERN.h XSUB.h av.h config.h cop.h cv.h dosish.h
h2 = embed.h form.h gv.h handy.h hv.h keywords.h mg.h op.h
h3 = opcode.h patchlevel.h perl.h perlapi.h perly.h pp.h proto.h regcomp.h
h4 = regexp.h scope.h sv.h unixish.h util.h iperlsys.h thread.h
h5 = utf8.h warnings.h
h = $(h1) $(h2) $(h3) $(h4) $(h5)

c1 = $(mallocsrc) av.c scope.c op.c doop.c doio.c dump.c hv.c mg.c
c2 = perl.c perly.c pp.c pp_hot.c pp_ctl.c pp_sys.c regcomp.c regexec.c utf8.c
c3 = gv.c sv.c taint.c toke.c util.c deb.c run.c universal.c xsutils.c
c4 = globals.c perlio.c perlapi.c

c = $(c1) $(c2) $(c3) $(c4) miniperlmain.c perlmain.c

obj1 = $(mallocobj) gv$(OBJ_EXT) toke$(OBJ_EXT) perly$(OBJ_EXT) op$(OBJ_EXT) regcomp$(OBJ_EXT) dump$(OBJ_EXT) util$(OBJ_EXT) mg$(OBJ_EXT)
obj2 = hv$(OBJ_EXT) av$(OBJ_EXT) run$(OBJ_EXT) pp_hot$(OBJ_EXT) sv$(OBJ_EXT) pp$(OBJ_EXT) scope$(OBJ_EXT) pp_ctl$(OBJ_EXT) pp_sys$(OBJ_EXT)
obj3 = doop$(OBJ_EXT) doio$(OBJ_EXT) regexec$(OBJ_EXT) utf8$(OBJ_EXT) taint$(OBJ_EXT) deb$(OBJ_EXT) universal$(OBJ_EXT) xsutils$(OBJ_EXT) globals$(OBJ_EXT) perlio$(OBJ_EXT) perlapi$(OBJ_EXT)

obj = $(obj1) $(obj2) $(obj3) $(ARCHOBJS)

# Once perl has been Configure'd and built ok you build different
# perl variants (Debugging, Embedded, Multiplicity etc) by saying:
#	make clean; make LIBPERL=libperl<type>.a
# where <type> is some combination of 'd' and(or) 'e' or 'm'.
# See cflags to understand how this works.
#
# This mechanism is getting clunky and might not even work any more.
# EMBEDDING is on by default, and MULTIPLICITY doesn't work.
#

lintflags = -hbvxac

.c$(OBJ_EXT):
	$(CCCMD) $(PLDLFLAGS) $*.c

all: config.sh $(FIRSTMAKEFILE) miniperl extra.pods $(private) $(public) $(dynamic_ext) $(nonxs_ext)
	@echo " ";
	@echo "	Everything is up to date. 'make test' to run test suite."

config.sh: config.sh.template
	@echo "Updating configuration in config.sh"
	cflags=`echo $(CFLAGS) | sed "s/\\//\\\\\\\\\\//g"` ; \
	ldflags=`echo $(LDFLAGS) | sed "s/\\//\\\\\\\\\\//g"` ; \
	ldlibs=`echo $(LDLIBS) | sed "s/\\//\\\\\\\\\\//g"` ; \
	inclibc=`echo $(INCLIBC) | sed "s/\\//\\\\\\\\\\//g"` ; \
	cc=`echo $(CC) | sed "s/\\//\\\\\\\\\\//g"` ; \
	ar=`echo $(AR) | sed "s/\\//\\\\\\\\\\//g"` ; \
	sed -e "s/^ccflags=.*$$/ccflags='-fno-strict-aliasing $$cflags'/g" \
		-e "s/^cppflags=.*$$/cppflags='-fno-strict-aliasing $$cflags'/g" \
		-e "s/^cc=.*$$/cc='$$cc'/g" \
		-e "s/^cpprun=.*$$/cpprun='$$cc -E'/g" \
		-e "s/^cppstdin=.*$$/cppstdin='$$cc -E'/g" \
		-e "s/^ld=.*$$/ld='$$cc'/g" \
		-e "s/^full_ar=.*$$/full_ar='$$ar'/g" \
		-e "s/^lddlflags=.*$$/lddlflags='-shared $$ldflags'/g" \
		-e "s/^ldflags=.*$$/ldflags='$$ldflags'/g" \
		-e "s/^libs=.*$$/libs='$$ldlibs'/g" \
		-e "s/^libsfound=.*$$/libsfound=''/g" \
		-e "s/^libspath=.*$$/libspath=''/g" \
		-e "s/^libsdirs=.*$$/libsdirs=''/g" \
		-e "s/^libsfiles=.*$$/libsfiles=''/g" \
		-e "s/^locincpth=.*$$/locincpth=''/g" \
		-e "s/^loclibpth=.*$$/loclibpth=''/g" \
		-e "s/^perllibs=.*$$/loclibpth='$$ldlibs'/g" \
		-e "s/^strings=.*$$/strings='$$inclibc\/string.h'/g" \
		-e "s/^timeincl=.*$$/timeincl='$$inclibc\/sys\/time.h $$inclibc\/time.h'/g" \
		< config.sh.template > config.sh

romfs:
	strip perl
	$(ROMFSINST) /usr/bin/perl
	mkdir -p $(ROMFSDIR)$(privlib)
	mkdir -p $(ROMFSDIR)$(sitelib)

compile: all
	echo "testing compilation" > testcompile;
	cd utils;  $(MAKE) compile;
	cd x2p; $(MAKE) compile; 
	cd pod; $(MAKE) compile;

translators:	miniperl lib/Config.pm FORCE
	@echo " "; echo "	Making x2p stuff"; cd x2p; $(LDLIBPTH) $(MAKE) all

utilities:	miniperl lib/Config.pm $(plextract) FORCE
	@echo " "; echo "	Making utilities"; cd utils; $(LDLIBPTH) $(MAKE) all


# This is now done by installman only if you actually want the man pages.
#	@echo " "; echo "	Making docs"; cd pod; $(MAKE) all;

# Phony target to force checking subdirectories.
# Apparently some makes require an action for the FORCE target.
FORCE:
	@sh -c true
opmini$(OBJ_EXT): op.c config.h
	$(RMS) opmini.c
	$(CPS) op.c opmini.c
	$(CCCMD) $(PLDLFLAGS) -DPERL_EXTERNAL_GLOB opmini.c
	$(RMS) opmini.c

miniperlmain$(OBJ_EXT): miniperlmain.c patchlevel.h
	$(CCCMD) $(PLDLFLAGS) $*.c

perlmain.c: miniperlmain.c config.sh $(FIRSTMAKEFILE)
	sh writemain $(DYNALOADER) $(static_ext) > writemain.tmp
	sh mv-if-diff writemain.tmp perlmain.c

perlmain$(OBJ_EXT): perlmain.c
	$(CCCMD) $(PLDLFLAGS) $*.c

# The file ext.libs is a list of libraries that must be linked in
# for static extensions, e.g. -lm -lgdbm, etc.  The individual
# static extension Makefile's add to it.
ext.libs: $(static_ext)
	-@test -f ext.libs || touch ext.libs

$(LIBPERL): $& perl$(OBJ_EXT) $(obj) $(LIBPERLEXPORT)
	rm -f $(LIBPERL)
	$(AR) rcu $(LIBPERL) perl$(OBJ_EXT) $(obj)
	@$(ranlib) $(LIBPERL)

# How to build executables.

# The $& notation tells Sequent machines that it can do a parallel make,
# and is harmless otherwise.
# The miniperl -w -MExporter line is a basic cheap test to catch errors
# before make goes on to run preplibrary and then MakeMaker on extensions.
# This is very handy because later errors are often caused by miniperl
# build problems but that's not obvious to the novice.
# The Module used here must not depend on Config or any extensions.

miniperl: $& miniperlmain$(OBJ_EXT) $(LIBPERL) opmini$(OBJ_EXT)
	$(LDLIBPTH) $(CC) $(CLDFLAGS) -o miniperl \
	    miniperlmain$(OBJ_EXT) opmini$(OBJ_EXT) $(LLIBPERL) $(libs)
	$(LDLIBPTH) ./miniperl -w -Ilib -MExporter -e '<?>' || $(MAKE) minitest

perl: $& perlmain$(OBJ_EXT) $(LIBPERL) $(DYNALOADER) $(static_ext) ext.libs $(PERLEXPORT)
	$(SHRPENV) $(LDLIBPTH) $(CC) -o perl $(CLDFLAGS) $(CCDLFLAGS) perlmain$(OBJ_EXT) $(DYNALOADER) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)

pureperl: $& perlmain$(OBJ_EXT) $(LIBPERL) $(DYNALOADER) $(static_ext) ext.libs $(PERLEXPORT)
	$(SHRPENV) $(LDLIBPTH) purify $(CC) -o pureperl $(CLDFLAGS) $(CCDLFLAGS) perlmain$(OBJ_EXT) $(DYNALOADER) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)

purecovperl: $& perlmain$(OBJ_EXT) $(LIBPERL) $(DYNALOADER) $(static_ext) ext.libs $(PERLEXPORT)
	$(SHRPENV) $(LDLIBPTH) purecov $(CC) -o purecovperl $(CLDFLAGS) $(CCDLFLAGS) perlmain$(OBJ_EXT) $(DYNALOADER) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)

quantperl: $& perlmain$(OBJ_EXT) $(LIBPERL) $(DYNALOADER) $(static_ext) ext.libs $(PERLEXPORT)
	$(SHRPENV) $(LDLIBPTH) quantify $(CC) -o quantperl $(CLDFLAGS) $(CCDLFLAGS) perlmain$(OBJ_EXT) $(DYNALOADER) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)

# This version, if specified in Configure, does ONLY those scripts which need
# set-id emulation.  Suidperl must be setuid root.  It contains the "taint"
# checks as well as the special code to validate that the script in question
# has been invoked correctly.

suidperl: $& sperl$(OBJ_EXT) perlmain$(OBJ_EXT) $(LIBPERL) $(DYNALOADER) $(static_ext) ext.libs $(PERLEXPORT)
	$(SHRPENV) $(LDLIBPTH) $(CC) -o suidperl $(CLDFLAGS) $(CCDLFLAGS) perlmain$(OBJ_EXT) sperl$(OBJ_EXT) $(DYNALOADER) $(static_ext) $(LLIBPERL) `cat ext.libs` $(libs)


sperl$(OBJ_EXT): perl.c $(h)
	$(RMS) sperl.c
	$(LNS) perl.c sperl.c
	$(CCCMD) -DIAMSUID sperl.c
	$(RMS) sperl.c

# We have to call our ./makedir because Ultrix 4.3 make can't handle the line
#	test -d lib/auto || mkdir lib/auto
#
preplibrary: miniperl lib/Config.pm
	@sh ./makedir lib/auto
	@echo "	AutoSplitting perl library"
	$(LDLIBPTH) ./miniperl -Ilib -e 'use AutoSplit; \
		autosplit_lib_modules(@ARGV)' lib/*.pm lib/*/*.pm

# Take care to avoid modifying lib/Config.pm without reason
# (If trying to create a new port and having problems with the configpm script, 
# try 'make minitest' and/or commenting out the tests at the end of configpm.)
lib/Config.pm: config.sh miniperl configpm lib/re.pm
	$(LDLIBPTH) ./miniperl configpm configpm.tmp
	sh mv-if-diff configpm.tmp $@

lib/ExtUtils/Miniperl.pm: miniperlmain.c miniperl minimod.pl lib/Config.pm
	$(LDLIBPTH) ./miniperl minimod.pl > minimod.tmp
	sh mv-if-diff minimod.tmp $@

lib/re.pm: ext/re/re.pm
	rm -f $@
	cat ext/re/re.pm > $@

$(plextract):	miniperl lib/Config.pm
	$(LDLIBPTH) ./miniperl -Ilib $@.PL

extra.pods: miniperl
	-@test -f extra.pods && rm -f `cat extra.pods`
	-@rm -f extra.pods
	-@for x in `grep -l '^=[a-z]' README.* | grep -v README.vms` ; do \
	    nx=`echo $$x | sed -e "s/README\.//"`; \
	    cd pod ; $(LNS) ../$$x "perl"$$nx".pod" ; cd .. ; \
	    echo "pod/perl"$$nx".pod" >> extra.pods ; \
	done
	-@test -f vms/perlvms.pod && cd pod && $(LNS) ../vms/perlvms.pod perlvms.pod && cd .. && echo "pod/perlvms.pod" >> extra.pods

install-strip:
	$(MAKE) STRIPFLAGS=-s install

install:
	$(MAKE) install.perl install.man STRIPFLAGS=$(STRIPFLAGS)

install-verbose:
	$(MAKE) install.perl install.man STRIPFLAGS=$(STRIPFLAGS) INSTALLFLAGS=-V

install-silent:
	$(MAKE) install.perl install.man STRIPFLAGS=$(STRIPFLAGS) INSTALLFLAGS=-S

no-install:
	$(MAKE) install.perl install.man STRIPFLAGS=$(STRIPFLAGS) INSTALLFLAGS=-n

install.perl:	all installperl
	if [ -n "$(COMPILE)" ]; \
	then \
		cd utils; $(MAKE) compile; \
		cd ../x2p; $(MAKE) compile; \
		cd ../pod; $(MAKE) compile; \
	else :; \
	fi
	$(LDLIBPTH) ./perl installperl $(INSTALLFLAGS) $(STRIPFLAGS)

install.man:	all installman
	$(LDLIBPTH) ./perl installman $(INSTALLFLAGS)

# XXX Experimental. Hardwired values, but useful for testing.
# Eventually Configure could ask for some of these values.
install.html: all installhtml
	-@test -f README.vms && cd vms && $(LNS) ../README.vms README_vms.pod && cd ..
	$(LDLIBPTH) ./perl installhtml                   \
      --podroot=. --podpath=. --recurse  \
      --htmldir=$(privlib)/html     \
      --htmlroot=$(privlib)/html    \
      --splithead=pod/perlipc     \
      --splititem=pod/perlfunc    \
      --libpods=perlfunc:perlguts:perlvar:perlrun:perlop \
      --verbose


# I now supply perly.c with the kits, so the following section is
# used only if you force byacc to run by saying
# 	make run_byacc
# Since we patch up the byacc output, the perly.fixer script needs
# to run with precisely the same version of byacc as I use.  You
# normally shouldn't remake perly.[ch].

run_byacc:	FORCE
	$(BYACC) -d perly.y
	-chmod 664 perly.c perly.h
	sh $(shellflags) ./perly.fixer y.tab.c perly.c
	sed -e 's/fprintf *( *stderr *,/PerlIO_printf(Perl_debug_log,/g' \
	    -e 's/y\.tab/perly/g' perly.c >perly.tmp && mv perly.tmp perly.c
	sed -e '/^extern YYSTYPE yy/D' y.tab.h >yh.tmp && mv yh.tmp y.tab.h
	cmp -s y.tab.h perly.h && rm -f y.tab.h || mv y.tab.h perly.h
	chmod 664 vms/perly_c.vms vms/perly_h.vms
	perl vms/vms_yfix.pl perly.c perly.h vms/perly_c.vms vms/perly_h.vms

# We don't want to regenerate perly.c and perly.h, but they might
# appear out-of-date after a patch is applied or a new distribution is
# made.
perly.c: perly.y
	-@sh -c true

perly.h: perly.y
	-@sh -c true

# No compat3.sym here since and including the 5.004_50.
# No interp.sym since 5.005_03.
SYM  = global.sym globvar.sym perlio.sym pp.sym

SYMH = perlvars.h intrpvar.h thrdvar.h

CHMOD_W = chmod +w

# The following files are generated automatically
#	keywords.pl:	keywords.h
#	opcode.pl:	opcode.h opnames.h pp_proto.h pp.sym
# [* embed.pl needs pp.sym generated by opcode.pl! *]
#	embed.pl:	proto.h embed.h embedvar.h global.sym objXSUB.h
#			perlapi.h perlapi.c pod/perlintern.pod
#			pod/perlapi.pod
#	bytecode.pl:	ext/ByteLoader/byterun.h ext/ByteLoader/byterun.c
#			ext/B/B/Asmdata.pm
#	regcomp.pl:	regnodes.h
#	warnings.pl:	warnings.h lib/warnings.pm
# The correct versions should be already supplied with the perl kit,
# in case you don't have perl available.
# To force them to be regenerated, type
#	make regen_headers

AUTOGEN_FILES = keywords.h opcode.h opnames.h pp_proto.h pp.sym proto.h \
		embed.h embedvar.h global.sym \
		pod/perlintern.pod pod/perlapi.pod \
		objXSUB.h perlapi.h perlapi.c ext/ByteLoader/byterun.h \
		ext/ByteLoader/byterun.c ext/B/B/Asmdata.pm regnodes.h \
		warnings.h lib/warnings.pm \
		vms/perly_c.vms vms/perly_h.vms

regen_headers:	FORCE
	-$(CHMOD_W) $(AUTOGEN_FILES)
	-perl keywords.pl
	-perl opcode.pl
	-perl embed.pl
	-perl bytecode.pl
	-perl regcomp.pl
	-perl warnings.pl

regen_pods:	FORCE
	-cd pod; $(LDLIBPTH) make regen_pods

regen_vms:	FORCE
	-perl vms/vms_yfix.pl perly.c perly.h vms/perly_c.vms vms/perly_h.vms

regen_all: regen_headers regen_pods regen_vms

# Extensions:
# Names added to $(dynamic_ext) or $(static_ext) or $(nonxs_ext) will
# automatically get built.  There should ordinarily be no need to change
# any of this part of makefile.
#
# The dummy dependency is a place holder in case $(dynamic_ext) or
# $(static_ext) is empty.
#
# DynaLoader may be needed for extensions that use Makefile.PL.

$(DYNALOADER):	miniperl preplibrary FORCE
	@$(LDLIBPTH) sh ext/util/make_ext static $@ MAKE=$(MAKE) LIBPERL_A=$(LIBPERL)

d_dummy $(dynamic_ext):	miniperl preplibrary $(DYNALOADER) FORCE
	@$(LDLIBPTH) sh ext/util/make_ext dynamic $@ MAKE=$(MAKE) LIBPERL_A=$(LIBPERL)

s_dummy $(static_ext):	miniperl preplibrary $(DYNALOADER) FORCE
	@$(LDLIBPTH) sh ext/util/make_ext static $@ MAKE=$(MAKE) LIBPERL_A=$(LIBPERL)

n_dummy $(nonxs_ext):	miniperl preplibrary $(DYNALOADER) FORCE
	@$(LDLIBPTH) sh ext/util/make_ext nonxs $@ MAKE=$(MAKE) LIBPERL_A=$(LIBPERL)

clean:		config.sh _tidy _mopup
	rm -f config.sh lib/Config.pm
	find . -name Makefile.old | xargs rm -f
	find . -name makefile | xargs rm -f
	find . -name "*.o" | xargs rm -f
	find . -name "*.so" | xargs rm -f
	find . -name "*.a" | xargs rm -f

realclean:	_realcleaner _mopup

_clobber:
	rm -f config.sh cppstdin Policy.sh

clobber:	_realcleaner _mopup _clobber

distclean:	clobber

# Like distclean but also removes emacs backups and *.orig.
veryclean:	_verycleaner _mopup _clobber
	-@rm -f Obsolete Wanted

# Do not 'make _mopup' directly.
_mopup:
	rm -f *$(OBJ_EXT) *$(LIB_EXT) all perlmain.c
	-@test -f extra.pods && rm -f `cat extra.pods`
	-@test -f vms/README_vms.pod && rm -f vms/README_vms.pod
	-rm -f perl.exp ext.libs extra.pods opmini.o
	-rm -f perl.export perl.dll perl.libexp perl.map perl.def
	-rm -f perl.loadmap miniperl.loadmap perl.prelmap miniperl.prelmap
	rm -f perl suidperl miniperl $(LIBPERL) libperl.* microperl

# Do not 'make _tidy' directly.
_tidy:
	-cd pod; $(LDLIBPTH) $(MAKE) clean
	-cd utils; $(LDLIBPTH) $(MAKE) clean
	-cd x2p; $(LDLIBPTH) $(MAKE) clean
	-@for x in $(DYNALOADER) $(dynamic_ext) $(static_ext) $(nonxs_ext) ; do \
	$(LDLIBPTH) sh ext/util/make_ext clean $$x MAKE=$(MAKE) ; \
	done
	rm -f testcompile compilelog

_cleaner1:
	-cd os2; rm -f Makefile
	-cd pod; $(LDLIBPTH) $(MAKE) $(CLEAN)
	-cd utils; $(LDLIBPTH) $(MAKE) $(CLEAN)
	-cd x2p; $(LDLIBPTH) $(MAKE) $(CLEAN)
	-@for x in $(DYNALOADER) $(dynamic_ext) $(static_ext) $(nonxs_ext) ; do \
	$(LDLIBPTH) sh ext/util/make_ext $(CLEAN) $$x MAKE=$(MAKE) ; \
	done

_cleaner2:
	rm -f core core.*perl.*.? *perl.core t/core t/core.perl.*.? t/*perl.core t/misctmp* t/forktmp* t/tmp* t/c t/perl t/rantests .?*.c so_locations $(LIBPERL_NONSHR) $(MINIPERL_NONSHR)
	rm -rf $(addedbyconf)
	rm -f $(FIRSTMAKEFILE) $(FIRSTMAKEFILE).old
	rm -f $(private)
	rm -rf lib/auto
	rm -f lib/.exists lib/*/.exists
	rm -f h2ph.man pstruct
	rm -rf .config
	rm -f testcompile compilelog
	-rmdir lib/B lib/Data lib/IO/Socket lib/IO lib/Sys lib/Thread

_realcleaner: 
	@$(LDLIBPTH) $(MAKE) _cleaner1 CLEAN=realclean
	@$(LDLIBPTH) $(MAKE) _cleaner2

_verycleaner: 
	@$(LDLIBPTH) $(MAKE) _cleaner1 CLEAN=veryclean
	@$(LDLIBPTH) $(MAKE) _cleaner2
	-rm -f *~ *.orig */*~ */*.orig */*/*~ */*/*.orig

# The following lint has practically everything turned on.  Unfortunately,
# you have to wade through a lot of mumbo jumbo that can't be suppressed.
# If the source file has a /*NOSTRICT*/ somewhere, ignore the lint message
# for that spot.

lint: $(c)
	lint $(lintflags) $(defs) perly.c $(c) > perl.fuzz

# Need to unset during recursion to go out of loop.
# The README below ensures that the dependency list is never empty and
# that when MAKEDEPEND is empty $(FIRSTMAKEFILE) doesn't need rebuilding.

MAKEDEPEND = Makefile makedepend

$(FIRSTMAKEFILE):	README $(MAKEDEPEND)
	$(MAKE) depend MAKEDEPEND=

config.h: config_h.SH config.sh
	$(SHELL) config_h.SH

# When done, touch perlmain.c so that it doesn't get remade each time.
depend: makedepend
	sh ./makedepend MAKE=$(MAKE)
	- test -s perlmain.c && touch perlmain.c
	cd x2p; $(MAKE) depend

# Cannot postpone this until $firstmakefile is ready ;-)
makedepend: makedepend.SH config.sh
	sh ./makedepend.SH

# Cannot delegate rebuilding of t/perl to make to allow interlaced
# test and minitest
test-prep: miniperl perl preplibrary utilities $(dynamic_ext) $(nonxs_ext) $(TEST_PERL_DLL)
	cd t && (rm -f perl$(EXE_EXT); $(LNS) ../perl$(EXE_EXT) perl$(EXE_EXT))

# Second branch is for testing without a tty or controling terminal.
# See t/op/stat.t
test check: test-prep
	if (true </dev/tty) >/dev/null 2>&1; then \
	  cd t && $(LDLIBPTH) ./perl TEST </dev/tty; \
	else \
	  cd t && PERL_SKIP_TTY_TEST=1 $(LDLIBPTH) ./perl TEST; \
	fi
	@echo "Ran tests" > t/rantests

utest ucheck: test-prep
	if (true </dev/tty) >/dev/null 2>&1; then \
	  cd t && $(LDLIBPTH) ./perl UTEST </dev/tty; \
	else \
	  cd t && PERL_SKIP_TTY_TEST=1 $(LDLIBPTH) ./perl UTEST; \
	fi

# For testing without a tty or controling terminal. See t/op/stat.t
test-notty: test-prep
	cd t && PERL_SKIP_TTY_TEST=1 $(LDLIBPTH) ./perl TEST

# Can't depend on lib/Config.pm because that might be where miniperl
# is crashing.
minitest: miniperl lib/re.pm
	@echo "You may see some irrelevant test failures if you have been unable"
	@echo "to build lib/Config.pm."
	- cd t && (rm -f perl$(EXE_EXT); $(LNS) ../miniperl$(EXE_EXT) perl$(EXE_EXT)) \
		&& $(LDLIBPTH) ./perl TEST base/*.t comp/*.t cmd/*.t run/*.t io/*.t op/*.t pragma/*.t </dev/tty

# Handy way to run perlbug -ok without having to install and run the
# installed perlbug. We don't re-run the tests here - we trust the user.
# Please *don't* use this unless all tests pass.
# If you want to report test failures, use "make nok" instead.
ok:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -ok -s '(UNINSTALLED)'

okfile:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -ok -s '(UNINSTALLED)' -F perl.ok

oknack:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -ok -s '(UNINSTALLED)' -A

okfilenack:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -ok -s '(UNINSTALLED)' -F perl.ok -A

nok:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -nok -s '(UNINSTALLED)'

nokfile:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -nok -s '(UNINSTALLED)' -F perl.nok

noknack:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -nok -s '(UNINSTALLED)' -A

nokfilenack:	utilities
	$(LDLIBPTH) ./perl -Ilib utils/perlbug -nok -s '(UNINSTALLED)' -F perl.nok -A

clist:	$(c)
	echo $(c) | tr ' ' $(TRNL) >.clist

hlist:  $(h)
	echo $(h) | tr ' ' $(TRNL) >.hlist

shlist: $(sh)
	echo $(sh) | tr ' ' $(TRNL) >.shlist

pllist: $(pl)
	echo $(pl) | tr ' ' $(TRNL) >.pllist

#Makefile: Makefile.SH ./config.sh
#	$(SHELL) Makefile.SH
Makefile:

distcheck: FORCE
	perl '-MExtUtils::Manifest=&fullcheck' -e 'fullcheck()'

elc:	emacs/cperl-mode.elc

emacs/cperl-mode.elc: emacs/cperl-mode.el
	-cd emacs; emacs -batch -q -no-site-file -f batch-byte-compile cperl-mode.el

etags:	TAGS

TAGS: emacs/cperl-mode.elc
	sh emacs/ptags

ctags:	tags

# Let's hope make will not go into an infinite loop on case-unsensitive systems
# This may also fail if . is in the head of the path, since perl will
# require -Ilib
tags:	TAGS
	perl emacs/e2ctags.pl TAGS > tags

# AUTOMATICALLY GENERATED MAKE DEPENDENCIES--PUT NOTHING BELOW THIS LINE
# If this runs make out of memory, delete /usr/include lines.
