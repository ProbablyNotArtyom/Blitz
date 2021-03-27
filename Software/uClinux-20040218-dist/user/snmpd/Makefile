.EXPORT_ALL_VARIABLES:

CXXFLAGS += $(INCSG)
CFLAGS += $(INCSG)

DIRS = snmplib modules snmpd

all romfs test:
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

clean:
	for i in $(DIRS) ; do make -C $$i clean ; done

