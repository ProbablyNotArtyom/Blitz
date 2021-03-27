CONFOPTIONS=								   \
	--disable-HOPFSERIAL --disable-HOPFPCI --disable-BANCOMM           \
	--disable-GPSVME --disable-SHM --disable-all-clocks                \
	--disable-ACTS --disable-ARBITER --disable-ARCRON-MSF              \
	--disable-ATOM --disable-AS2201 --disable-CHU --disable-AUDIO-CHU  \
	--disable-DATUM --disable-FG --disable-HEATH --disable-HPGPS       \
	--disable-IRIG --disable-JJY --disable-LEITCH                      \
	--disable-LOCAL-CLOCK --disable-MSFEES --disable-MX4200            \
	--disable-NMEA --disable-ONCORE --disable-PALISADE --disable-PST   \
	--disable-JUPITER --disable-PTBACTS --disable-TPRO --disable-TRAK  \
	--disable-CHRONOLOG --disable-DUMBCLOCK --disable-PCF              \
	--disable-SPECTRACOM --disable-TRUETIME --disable-ULINK            \
	--disable-WWV --disable-USNO --disable-parse-clocks                \
	--disable-COMPUTIME --disable-DCF7000 --disable-HOPF6021           \
	--disable-MEINBERG --disable-RAWDCF --disable-RCC8000              \
	--disable-SCHMID --disable-TRIMTAIP --disable-TRIMTSIP             \
	--disable-WHARTON --disable-VARITEXT --disable-kmem                \
	--without-openssl-libdir --without-openssl-incdir --without-crypto \
	--without-electricfence

.PHONY: all
all:
	if [ \! -f Makefile ]; then					   \
		chmod 755 ./configure					  ;\
		unset CC CFLAGS LDFLAGS && ./configure -C $(CONFOPTIONS)  ;\
	fi
	make -f Makefile

.PHONY: romfs
romfs:
	$(MAKE) -C ntpd romfs
	$(MAKE) -C ntpdc romfs
	$(MAKE) -C ntpdate romfs
	$(MAKE) -C ntpq romfs


%:
	if [ \! -f Makefile ]; then					   \
		chmod 755 ./configure					  ;\
		unset CC CFLAGS LDFLAGS && ./configure -C $(CONFOPTIONS)  ;\
	fi
	make -f Makefile $@
