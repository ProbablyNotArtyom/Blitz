
SERVICES =    \"/etc/services\"
INETD_CONF =  \"/etc/inetd.conf\"

EXEC = inetd
OBJS = inetd.o config.o

CFLAGS += -DSERVICES=$(SERVICES) -DINETD_CONF=$(INETD_CONF)
CFLAGS += -Wall

# These changes allow for an additional, optional config file in /etc/config/inetd.conf
# which can be modified
CFLAGS += -DMODIFY_CONFIG -DALT_INETD_CONF=\"/etc/config/inetd.conf\"

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

clean:
	rm -f $(EXEC) *.elf *.gdb *.o

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	:
	: inetd.conf
	:
	$(ROMFSINST) -e CONFIG_USER_DISCARD_DISCARD \
		-a "discard dgram  udp wait   root /bin/discard" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_DISCARD_DISCARD \
		-a "discard stream tcp nowait root /bin/discard" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_MARADNS_MARADNS \
		-a "domain  stream tcp nowait root /bin/zoneserver" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_TFTPD_TFTPD \
	   	-a "tftp    dgram  udp wait   root /bin/tftpd" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_SSH_SSHD \
		-a "ssh     stream tcp nowait root /bin/sshd -i" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_TELNETD_TELNETD \
		-a "telnet  stream tcp nowait root /bin/telnetd" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_FTPD_FTPD \
		-a "ftp     stream tcp nowait root /bin/ftpd -l" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_GAMES_ADVENT4 \
		-a "advent4 stream tcp nowait root /bin/advent4" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_GAMES_DUNGEON \
		-a "dungeon stream tcp nowait root /bin/dungeon" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_DESTEST_DESPERF \
		-a "des     stream tcp nowait root /bin/desperf des" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_DESTEST_DESPERF \
		-a "des3    stream tcp nowait root /bin/desperf 3des" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_DESTEST_DESPERF \
		-a "aes    stream tcp nowait root /bin/desperf aes" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_SAMBA \
		-a "netbios-ssn stream tcp nowait root smbd -d1" /etc/inetd.conf
	:
	: Klaxon configuration
	:
	$(ROMFSINST) -e CONFIG_USER_KLAXON_CHARGEN \
			-a "chargen stream tcp nowait root /bin/klaxon tcp-chargen" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_CHARGEN \
			-a "chargen stream udp wait   root /bin/klaxon udp-chargen" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_DISCARD \
			-a "discard stream tcp nowait root /bin/klaxon tcp-discard" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_DISCARD \
			-a "discard stream udp wait   root /bin/klaxon udp-discard" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_ECHO \
			-a "echo    stream tcp nowait root /bin/klaxon tcp-echo" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_ECHO \
			-a "echo    stream udp wait   root /bin/klaxon udp-echo" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_FTP \
			-a "ftp     stream tcp nowait root /bin/klaxon ftp" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_FTP \
			-a "ftpdata stream tcp nowait root /bin/klaxon ftp-data" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_HTTP \
			-a "http    stream tcp nowait root /bin/klaxon http" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_SMTP \
			-a "smtp    stream tcp nowait root /bin/klaxon smtp" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_SNMP \
			-a "snmp    stream tcp nowait root /bin/klaxon tcp-snmp" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_SNMP \
			-a "snmp    stream udp wait   root /bin/klaxon udp-snmp" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_SSH \
			-a "ssh     stream tcp nowait root /bin/klaxon ssh" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_TELNET \
			-a "telnet  stream tcp nowait root /bin/klaxon telnet" /etc/inetd.conf
	$(ROMFSINST) -e CONFIG_USER_KLAXON_TFTP \
			-a "tftp    stream udp wait   root /bin/klaxon tftp" /etc/inetd.conf
