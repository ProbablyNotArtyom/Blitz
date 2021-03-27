#! /usr/bin/make

all: $(SHARED_LIBS) $(EXTRAS)

experimental: $(EXTRAS_EXP)

# Have to handle extensions which no longer exist.
.PHONY: clean
clean: $(EXTRA_CLEANS)
	-rm -f $(EXTRAS) $(EXTRAS_EXP)
	-rm -f $(SHARED_LIBS) $(SHARED_LIBS:%.so=%_sh.o)
	-rm -f $(STATIC_LIBS) $(STATIC6_LIBS)
	-rm -f extensions/initext.c extensions/initext6.c
	-find . -name '*.[ao]' -o -name '*.so' | xargs rm -f
	-find . -name '*.gdb' -print | xargs rm -f
	-rm -f $(DEPFILES) $(EXTRA_DEPENDS) .makefirst

install: all $(EXTRA_INSTALLS)
	@if [ -f /usr/local/bin/iptables -a "$(BINDIR)" = "/usr/local/sbin" ];\
	then echo 'Erasing iptables from old location (now /usr/local/sbin).';\
	rm -f /usr/local/bin/iptables;\
	fi

install-experimental: $(EXTRA_INSTALLS_EXP)

TAGS:
	@rm -f $@
	find . -name '*.[ch]' | xargs etags -a

dep: $(DEPFILES) $(EXTRA_DEPENDS)
	@echo Dependencies will be generated on next make.
	rm -f $(DEPFILES) $(EXTRA_DEPENDS) .makefirst

$(SHARED_LIBS:%.so=%.d): %.d: %.c
	@-$(CC) -M -MG $(CFLAGS) $< | \
	    sed -e 's@^.*\.o:@$*.d $*_sh.o:@' > $@

$(SHARED_LIBS): %.so : %_sh.o
	$(LD) -shared -o $@ $<

%_sh.o : %.c
	$(CC) $(SH_CFLAGS) -o $@ -c $<

.makefirst:
	@echo Making dependencies: please wait...
	@touch .makefirst

# This is useful for when dependencies completely screwed
%.h::
	echo Something wrong... deleting dependencies.
	rm -f $(EXTRAS) $(EXTRAS_EXP)
	rm -f $(SHARED_LIBS) $(SHARED_LIBS:%.so=%_sh.o)
	rm -f $(STATIC_LIBS) $(STATIC6_LIBS)
	rm -f extensions/initext.c extensions/initext6.c
	find . -name '*.[aod]' -o -name '*.so' | xargs rm -f
	find . -name '*.gdb' -print | xargs rm -f
	-rm -f $(DEPFILES) $(EXTRA_DEPENDS) .makefirst
	exit 1

-include $(DEPFILES) $(EXTRA_DEPENDS)
-include .makefirst
