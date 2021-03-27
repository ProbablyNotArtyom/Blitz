
.EXPORT_ALL_VARIABLES:

DIRS = Tremor

all romfs:
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done

clean:
	for i in $(DIRS) ; do make -C $$i clean ; done

