/****************************************************************************
 *
 * linux/kernel/sysctl_pivio.c
 *
 * Copyright (C) 2000, Crossport Systems (www.crossport.com)
 *
 ***************************************************************************/
#include <linux/config.h>

#ifdef CROSSPORT_PROC_GLOBALS

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/sysctl.h>
#include <linux/swapctl.h>
#include <linux/proc_fs.h>
#include <linux/malloc.h>
#include <linux/stat.h>
#include <linux/ctype.h>
#include <asm/bitops.h>
#include <asm/segment.h>

#include <linux/utsname.h>
#include <linux/swapctl.h>
#include <asm/nettel.h>

/****************************************************************************
 * prototypes
 ***************************************************************************/
int proc_do_pwantype(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp);
int proc_do_ip(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp);
int proc_do_ushort(ctl_table *table, int write, struct file *filp,
		  void *buffer, size_t *lenp);
int proc_do_ulonghex(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp);
int proc_do_snmp(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp);
int proc_do_pingsites(ctl_table *table, int write, struct file *filp,
                      void *buffer, size_t *lenp);
int proc_do_ipfw(ctl_table *table, int write, struct file *filp,
                 void *buffer, size_t *lenp);
int proc_do_vpns(ctl_table *table, int write, struct file *filp,
                 void *buffer, size_t *lenp);

/****************************************************************************
 * actual globals
 ***************************************************************************/

char            pivio_password[XP_PROC_PW_LEN+1];
char            pivio_username[XP_PROC_UN_LEN+1];

unsigned short  pivio_configed = 0;
unsigned short  pivio_online = 0;
unsigned short  pivio_wan_lease = 0;
unsigned short  pivio_lan_lease = 0;

/* firewall settings */
unsigned short  pivio_quiet_mode = 0;
unsigned short  pivio_stealth_mode = 0;
unsigned short  pivio_ipsec_nat = 0;
unsigned short  pivio_wan_nat = 0;
unsigned short  pivio_force_ping = 0;
unsigned short  pivio_pluto_started = 0;

unsigned short  pivio_lan_static = 0;
tp_wan_type     wan_type = p_wan_dhcp;

/* ip's for wan and lan */
unsigned long   pivio_ipwan = 0;
unsigned long   pivio_nmwan = 0;
unsigned long   pivio_gwwan = 0;
unsigned long   pivio_dns1wan = 0;
unsigned long   pivio_dns2wan = 0;
unsigned long   pivio_dns3wan = 0;
unsigned long   pivio_iplan = 0;
unsigned long   pivio_nmlan = 0;
unsigned long   pivio_dhcp_start = 0;
unsigned long   pivio_dhcp_end = 0;
unsigned long   pivio_lease_online = 0;
unsigned long   pivio_lease_offline = 0;
char            pivio_dhcp_lease_period[4] = {0, 0, 0, 0};

/* pids */
pid_t           pivio_xpd_pid = 0;
pid_t           pivio_dhcpd_pid = 0;
pid_t           pivio_dhcpc_pid = 0;
pid_t           pivio_dns_pid = 0;
pid_t           pivio_xpupd_pid = 0;
pid_t           pivio_pppoe_pid = 0;
pid_t           pivio_snmpd_pid = 0;

/* leds */
unsigned short  pivio_led_online = 0;
unsigned short  pivio_led_lan1 = 0;
unsigned short  pivio_led_lan2 = 0;
unsigned short  pivio_led_wan1 = 0;
unsigned short  pivio_led_wan2 = 0;
unsigned short  pivio_led_update_avail = 0;
unsigned short  pivio_led_vpn = 0;
unsigned short  pivio_led_user = 0;

/* update stuff */
unsigned long   pivio_config_ip = 0;
unsigned long   pivio_update_ip = 0;
unsigned long   pivio_ping_sites[XP_PROC_NUM_PING_SITES] =
                {
                    0L, 0L, 0L, 0L, 0L
                };
unsigned short  pivio_update_ip_locked = 0;
unsigned short  pivio_update_avail = 0;
unsigned short  pivio_update_acknowledged = 0;

unsigned short  pivio_xpd_major = 0;
unsigned short  pivio_xpd_minor = 0;
unsigned short  pivio_xpd_pl = 0;
unsigned short  pivio_xpd_update_pending = 0;
char            pivio_xpd_update_file[XP_PROC_STRLEN+1];

unsigned short  pivio_kernel_major = 0;
unsigned short  pivio_kernel_minor = 0;
unsigned short  pivio_kernel_pl = 0;
unsigned short  pivio_kernel_update_pending = 0;
char            pivio_kernel_update_file[XP_PROC_STRLEN+1];

unsigned short  pivio_bigburn_pending = 0;
char            pivio_bigburn_file[XP_PROC_STRLEN+1];
unsigned long   pivio_addr_bootargs = 0;
unsigned long	pivio_addr_mac = 0;
unsigned long	pivio_addr_config = 0;
unsigned long	pivio_len_bootargs = 0;
unsigned long	pivio_len_mac = 0;
unsigned long	pivio_len_config = 0;

unsigned short  pivio_bootloader_version = 0;
unsigned short  pivio_bootloader_update_pending = 0;
char            pivio_bootloader_update_file[XP_PROC_STRLEN+1];

char            pivio_pppoe_username[XP_PROC_STRLEN+1];
char            pivio_pppoe_password[XP_PROC_PW_LEN+1];

unsigned short  pivio_snmp_eth0_axs = p_snmp_read_write;
unsigned short  pivio_snmp_eth1_axs = p_snmp_read_only;
unsigned short  pivio_snmp_ipsec0_axs = p_snmp_read_write;
unsigned short  pivio_snmp_enabled = 0;
tp_snmp_communities     pivio_snmp[XP_PROC_NUM_SNMP] =
                        {
                            {0L, 0L, {0}, p_snmp_none},
                            {0L, 0L, {0}, p_snmp_none},
                            {0L, 0L, {0}, p_snmp_none},
                            {0L, 0L, {0}, p_snmp_none},
                            {0L, 0L, {0}, p_snmp_none}
                        };

tp_ipfw         pivio_port_forwards[XP_PROC_NUM_IPFWS];

tp_vpn          pivio_vpns[XP_PROC_NUM_VPNS];

tp_netbios      pivio_nbSettings;

#define IP_LEN 16

ctl_table pivio_wan_table[] = {
	{PIVIO_WAN_TYPE, "wan_type", &wan_type, sizeof(tp_wan_type),
         	0644, NULL, &proc_do_pwantype},
	{PIVIO_WAN_IP, "ip", &pivio_ipwan, sizeof(unsigned long),
		0644, NULL, &proc_do_ip},
	{PIVIO_WAN_NM, "nm", &pivio_nmwan, sizeof(unsigned long),
		0644, NULL, &proc_do_ip},
	{PIVIO_WAN_GW, "gw", &pivio_gwwan, sizeof(unsigned long),
		0644, NULL, &proc_do_ip},
	{PIVIO_WAN_DNS1, "dns1", &pivio_dns1wan, sizeof(unsigned long),
		0644, NULL, &proc_do_ip},
	{PIVIO_WAN_DNS1, "dns2", &pivio_dns2wan, sizeof(unsigned long),
		0644, NULL, &proc_do_ip},
	{PIVIO_WAN_DNS1, "dns3", &pivio_dns3wan, sizeof(unsigned long),
		0644, NULL, &proc_do_ip},
        {PIVIO_WAN_PPPOE_USER,	"pppoe_username",	&pivio_pppoe_username,
		XP_PROC_STRLEN+1, 0644, NULL, &proc_dostring},
        {PIVIO_WAN_PPPOE_PW,	"pppoe_password",	&pivio_pppoe_password,
		XP_PROC_PW_LEN+1, 0644, NULL, &proc_dostring},
        {0}
};

ctl_table pivio_lan_table[] = {
	{PIVIO_LAN_STATIC, "lan_static", &pivio_lan_static, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LAN_IP, "ip", &pivio_iplan,
		sizeof(unsigned long), 0644, NULL, &proc_do_ip},
	{PIVIO_LAN_NM, "nm", &pivio_nmlan,
		sizeof(unsigned long), 0644, NULL, &proc_do_ip},
	{PIVIO_LAN_DHCP_START, "dhcp_start", &pivio_dhcp_start,
		sizeof(unsigned long), 0644, NULL, &proc_do_ip},
	{PIVIO_LAN_DHCP_END, "dhcp_end", &pivio_dhcp_end,
		sizeof(unsigned long), 0644, NULL, &proc_do_ip},
	{PIVIO_LAN_LEASE_ONLINE, "lease_online", &pivio_lease_online,
		sizeof(unsigned long), 0644, NULL, &proc_dointvec},
	{PIVIO_LAN_LEASE_OFFLINE, "lease_offline", &pivio_lease_offline,
		sizeof(unsigned long), 0644, NULL, &proc_dointvec},
        {0}
};

ctl_table pivio_pids_table[] = {
	{PIVIO_XPD_PID, "xpd", &pivio_xpd_pid, sizeof(pid_t),
		0644, NULL, &proc_dointvec},
	{PIVIO_DHCPD_PID, "dhcpd", &pivio_dhcpd_pid, sizeof(pid_t),
		0644, NULL, &proc_dointvec},
	{PIVIO_DHCPC_PID, "dhcpc", &pivio_dhcpc_pid, sizeof(pid_t),
		0644, NULL, &proc_dointvec},
	{PIVIO_DNS_PID, "dnsp", &pivio_dns_pid, sizeof(pid_t),
		0644, NULL, &proc_dointvec},
	{PIVIO_XPUPD_PID, "xpupd", &pivio_xpupd_pid, sizeof(pid_t),
		0644, NULL, &proc_dointvec},
	{PIVIO_PPPOE_PID, "pppoe", &pivio_pppoe_pid, sizeof(pid_t),
		0644, NULL, &proc_dointvec},
	{PIVIO_SNMPD_PID, "snmpd", &pivio_snmpd_pid, sizeof(pid_t),
		0644, NULL, &proc_dointvec},
	{0}
};

ctl_table pivio_leds_table[] = {
	{PIVIO_LED_ONLINE, "online", &pivio_led_online,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LED_LAN1, "lan_status", &pivio_led_lan1,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LED_LAN2, "lan_act", &pivio_led_lan2,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LED_WAN1, "wan_status", &pivio_led_wan1,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LED_WAN2, "wan_act", &pivio_led_wan2,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LED_UPDATE, "update", &pivio_led_update_avail,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LED_VPN, "vpn", &pivio_led_vpn,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_LED_USER, "user", &pivio_led_user,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{0}
};

ctl_table pivio_firewall_table[] = {
	{PIVIO_FWOPT_QUIET_MODE, "quiet_mode", &pivio_quiet_mode,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_FWOPT_STEALTH_MODE, "stealth_mode", &pivio_stealth_mode,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_FWOPT_IPSEC_NAT, "ipsec_nat", &pivio_ipsec_nat,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_FWOPT_WAN_NAT, "wan_nat", &pivio_wan_nat,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_FORCE_PING, "force_ping", &pivio_force_ping,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_PLUTO_STARTED, "pluto_started", &pivio_pluto_started,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_IPFWS, "port_forwards", &pivio_port_forwards,
		sizeof(pivio_port_forwards), 0444, NULL, &proc_do_ipfw},
	{PIVIO_VPNS, "vpns", &pivio_vpns, 
		sizeof(pivio_vpns), 0444, NULL, &proc_do_vpns},
	{0}
};

ctl_table pivio_updates_table[] = {
	{PIVIO_XPD_MAJOR, "xpd_major", &pivio_xpd_major,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_XPD_MINOR, "xpd_minor", &pivio_xpd_minor,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_XPD_PL, "xpd_pl", &pivio_xpd_pl,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_XPD_PENDING, "xpd_update_pending", &pivio_xpd_update_pending,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_XPD_FILE, "xpd_update_file", &pivio_xpd_update_file,
		XP_PROC_STRLEN+1, 0644, NULL, &proc_dostring},
	{PIVIO_KERN_MAJOR, "kernel_major", &pivio_kernel_major,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_KERN_MINOR, "kernel_minor", &pivio_kernel_minor,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_KERN_PL, "kernel_pl", &pivio_kernel_pl,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_KERN_PENDING, "kernel_update_pending", 
		&pivio_kernel_update_pending,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_KERN_FILE, "kernel_update_file", &pivio_kernel_update_file,
		XP_PROC_STRLEN+1, 0644, NULL, &proc_dostring},
	{PIVIO_BOOTLOADER_VER, "bootloader_version", &pivio_bootloader_version,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_BOOTLOADER_PENDING, "bootloader_update_pending",
		&pivio_bootloader_update_pending,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_BOOTLOADER_FILE, "bootloader_update_file",
		&pivio_bootloader_update_file,
		XP_PROC_STRLEN+1, 0644, NULL, &proc_dostring},
	{PIVIO_UPDATE_IP, "update_server", &pivio_update_ip,
		sizeof(unsigned long), 0644, NULL, &proc_do_ip},
	{PIVIO_UPDATE_IP_FIXED, "update_server_locked", &pivio_update_ip_locked,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_UPDATE_AVAIL, "update_avail", &pivio_update_avail,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_UPDATE_ACKNOWLEDGED, "update_acked", &pivio_update_acknowledged,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_CFG_IP, "config_ip", &pivio_config_ip,
		sizeof(unsigned long), 0644, NULL, &proc_do_ip},
	{PIVIO_PING_SITES, "ping_sites", &pivio_ping_sites,
		sizeof(pivio_ping_sites), 0444, NULL, &proc_do_pingsites},
	{PIVIO_BIGBURN_PENDING, "bigburn_update_pending", 
		&pivio_bigburn_pending,
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_BIGBURN_FILE, "bigburn_update_file", &pivio_bigburn_file,
		XP_PROC_STRLEN+1, 0644, NULL, &proc_dostring},
	{PIVIO_ADDR_BOOTARGS, "bigburn_addr_bootargs", &pivio_addr_bootargs,
		sizeof(unsigned long), 0644, NULL, &proc_do_ulonghex},
	{PIVIO_ADDR_MAC, "bigburn_addr_mac", &pivio_addr_mac,
		sizeof(unsigned long), 0644, NULL, &proc_do_ulonghex},
	{PIVIO_ADDR_CONFIG, "bigburn_addr_config", &pivio_addr_config,
		sizeof(unsigned long), 0644, NULL, &proc_do_ulonghex},
	{PIVIO_LEN_BOOTARGS, "bigburn_len_bootargs", &pivio_len_bootargs,
		sizeof(unsigned long), 0644, NULL, &proc_do_ulonghex},
	{PIVIO_LEN_MAC, "bigburn_len_mac", &pivio_len_mac,
		sizeof(unsigned long), 0644, NULL, &proc_do_ulonghex},
	{PIVIO_LEN_CONFIG, "bigburn_len_config", &pivio_len_config,
		sizeof(unsigned long), 0644, NULL, &proc_do_ulonghex},
	{0}
};

ctl_table pivio_snmp_table[] = {
	{PIVIO_SNMP_ENABLED, "enabled", &pivio_snmp_enabled, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_SNMP_COMMUNITIES, "communities", &pivio_snmp, 
		sizeof(pivio_snmp), 0444, NULL, &proc_do_snmp},
	{PIVIO_SNMP_ETH0_AXS, "eth0_axs", &pivio_snmp_eth0_axs, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_SNMP_ETH1_AXS, "eth1_axs", &pivio_snmp_eth1_axs, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{PIVIO_SNMP_IPSEC0_AXS, "ipsec0_axs", &pivio_snmp_ipsec0_axs, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
	{0}
};

ctl_table pivio_table[] = {
#if 0
        {PIVIO_VERSION, "version", NULL, 0, 0444, NULL, NULL, NULL},
        {PIVIO_MODEL,   "model", NULL, 0, 0444, NULL, NULL, NULL},
#endif
        {PIVIO_PIDS,    "pids", NULL, 0, 0555, pivio_pids_table},
        {PIVIO_WAN,     "wan", NULL, 0, 0555, pivio_wan_table},
        {PIVIO_LAN,     "lan", NULL, 0, 0555, pivio_lan_table},
        {PIVIO_FIREWALL,"firewall", NULL, 0, 0555, pivio_firewall_table},
        {PIVIO_SNMP,    "snmp", NULL, 0, 0555, pivio_snmp_table},
        {PIVIO_LEDS,    "leds", NULL, 0, 0555, pivio_leds_table},
        {PIVIO_UPDATES, "updates", NULL, 0, 0555, pivio_updates_table},
        {PIVIO_CONFIGED,        "configed",     &pivio_configed, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_ONLINE,          "online",       &pivio_online, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_WAN_LEASE,       "wan_lease",    &pivio_wan_lease, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_LAN_LEASE,       "lan_lease",    &pivio_lan_lease, 
		sizeof(unsigned short), 0644, NULL, &proc_do_ushort},
        {PIVIO_PASSWORD,	"password",	&pivio_password,
		XP_PROC_PW_LEN+1, 0644, NULL, &proc_dostring},
        {PIVIO_USERNAME,	"username",	&pivio_username,
		XP_PROC_UN_LEN+1, 0644, NULL, &proc_dostring},
        {0}
};

int write_ip(the_ip,ip_buffer)
long the_ip;
char *ip_buffer;
{
 unsigned char a,b,c,d;

   if (ip_buffer == NULL) {
      return 0;
   }

   if (the_ip == 0) {
      ip_buffer[0] = 0;
      return 0;
   }

   a = (the_ip & 0xff000000) >> 24;
   b = (the_ip & 0x00ff0000) >> 16;
   c = (the_ip & 0x0000ff00) >> 8;
   d =  the_ip & 0x000000ff;

   sprintf(ip_buffer,"%u.%u.%u.%u",a,b,c,d);

   return strlen(ip_buffer);
}

int proc_do_pwantype(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp)
{
 tp_wan_type *wan_val;
 int len;
 char *p, c;
#define WAN_LEN 7
 char wan_buffer[WAN_LEN+1]; 

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   wan_val = (tp_wan_type *) table->data;

   if (write) {
      len = 0;
      p = buffer;
      if (*lenp > WAN_LEN) {
         /* longest is 'Static' */
         return -EINVAL;
      }
      if (*lenp < 5) {
         /* shortest is 'DHCP' */
         return -EINVAL;
      }

      while (len < *lenp && (c = get_user(p++)) != 0 && c != '\n') {
         /* convert to lower case */
         if (c < 'a') {
            c = c + ('a'-'A');
         }
         wan_buffer[len++] = c;
      }
      wan_buffer[len] = 0;

      switch(len) {
       case 4:
         if ((wan_buffer[0] == 'd') && (wan_buffer[1] == 'h') && 
             (wan_buffer[2] == 'c') && (wan_buffer[3] == 'p')) {
            *wan_val = p_wan_dhcp;
         } else {
            return -EINVAL;
         }
         break;
       case 5:
         if ((wan_buffer[0] == 'p') && (wan_buffer[1] == 'p') && 
             (wan_buffer[2] == 'p') && (wan_buffer[3] == 'o') &&
             (wan_buffer[4] == 'e')) {
            *wan_val = p_wan_pppoe;
         } else {
            return -EINVAL;
         }
         break;
       case 6:
         if ((wan_buffer[0] == 's') && (wan_buffer[1] == 't') && 
             (wan_buffer[2] == 'a') && (wan_buffer[3] == 't') &&
             (wan_buffer[4] == 'i') && (wan_buffer[5] == 'c')) {
            *wan_val = p_wan_static;
         } else {
            return -EINVAL;
         }
         break;
       default:
         return -EINVAL;
      }
      filp->f_pos += *lenp;
   } else {
      if (*lenp < WAN_LEN) {
         /* not enough space */
         return -EINVAL;
      }
     
      switch(*wan_val) {
       case p_wan_dhcp:
         memcpy_tofs(buffer,"DHCP\n",5);
         *lenp = 5;
         break;
       case p_wan_pppoe:
         memcpy_tofs(buffer,"PPPoE\n",6);
         *lenp = 6;
         break;
       case p_wan_static:
         memcpy_tofs(buffer,"Static\n",7);
         *lenp = 7;
         break;
       default:
         *lenp = 0;
      }
      filp->f_pos += *lenp;
   }
   return 0;
}
/****************************************************************************
 * proc_do_ip()
 *
 *   This is a /proc handler for unsigned longs that will be entered
 *   or displayed as IP addresses. Internally they are stored as an unsigned
 *   long, but will be displayed as x.x.x.x and entered this way also.
 ***************************************************************************/
int proc_do_ip(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp)
{
 long *ip;
 int len;
 char *p, c;
 char ip_buffer[IP_LEN+1]; 

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   ip = (long *) table->data;

   if (write) {
    char octet_buffer[3];
    int obi,recursion_index;
    int recursion_count = 0;
    long dbuff = 0;

      len = 0;
      p = buffer;
      if (*lenp > IP_LEN) {
         /* longest is nnn.nnn.nnn.nnn */
         return -EINVAL;
      }
      if (*lenp < 7) {
         /* shortest is n.n.n.n */
         return -EINVAL;
      }

      while (len < *lenp && (c = get_user(p++)) != 0 && c != '\n') {
         ip_buffer[len++] = c;
      }
      ip_buffer[len] = 0;

      /* so now we need to process what is hopefully x.x.x.x into a */
      /* long                                                       */
      recursion_index = 0;
      for (recursion_count = 0;recursion_count < 4; recursion_count++) {
       unsigned char ip_octet;
       unsigned long tmp_octet;

         obi = 0;
         while (obi < 3 && recursion_index <= len 
                && ip_buffer[recursion_index] != '.') {
            octet_buffer[obi++] = ip_buffer[recursion_index++];
         }
         switch(ip_buffer[recursion_index]) {
          case '.': /* inter-octet separator */
            recursion_index++;
            break;
          case 0: /* end of input */
          case '\n': /* end of input */
          case '\r': /* end of input */
            break;
          default:
            return -EINVAL;
         }

         octet_buffer[obi] = 0;
         tmp_octet = simple_strtoul(octet_buffer,&octet_buffer,0);
         if (tmp_octet > 255) {
            return -EINVAL;
         }
         ip_octet = tmp_octet;

         dbuff += (unsigned short) ip_octet << (3-recursion_count)*8;
      }

      *ip = dbuff;
      filp->f_pos += *lenp;
   } else {
    unsigned long my_ip;
    unsigned char a,b,c,d;

      my_ip = *ip;
      a = (my_ip & 0xff000000) >> 24;
      b = (my_ip & 0x00ff0000) >> 16;
      c = (my_ip & 0x0000ff00) >> 8;
      d = my_ip & 0x000000ff;
      sprintf(ip_buffer,"%u.%u.%u.%u",a,b,c,d);
      len = strlen(ip_buffer);
      if (len > *lenp) {
         len = *lenp;
      }
      if (len) {
         memcpy_tofs(buffer,ip_buffer,len);
      }
      if (len < *lenp) {
         put_user('\n',((char *)buffer) + len);
         len++;
      }
      *lenp = len;
      filp->f_pos += len;
   }
   return 0;
}
/****************************************************************************
 * proc_do_short()
 *   Proc handler to handle unsigned short. 
 ***************************************************************************/
int proc_do_ushort(ctl_table *table, int write, struct file *filp,
		  void *buffer, size_t *lenp)
{
 unsigned short *s_buf;
 int len;
 char *p, c;
#define SHORT_BUFF_LEN 5
 char short_buffer[SHORT_BUFF_LEN+1]; 
 int result;

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   s_buf = (unsigned short *) table->data;

   if (write) {

      len = 0;
      p = buffer;
      if (*lenp > SHORT_BUFF_LEN) {
         return -EINVAL;
      }

      while (len < *lenp && (c = get_user(p++)) != 0 && c != '\n') {
         short_buffer[len++] = c;
      }
      short_buffer[len] = 0;

      result = simple_strtoul(short_buffer,&short_buffer,0);
      if (result > 255) {
         return -EINVAL;
      }

      *s_buf = result;
      filp->f_pos += *lenp;
   } else {

      result = *s_buf;
      sprintf(short_buffer,"%u",result);
      len = strlen(short_buffer);
      if (len > *lenp) {
         len = *lenp;
      }
      if (len) {
         memcpy_tofs(buffer,short_buffer,len);
      }
      if (len < *lenp) {
         put_user('\n',((char *)buffer) + len);
         len++;
      }
      *lenp = len;
      filp->f_pos += len;
   }
   return 0;
}

/****************************************************************************
 * proc_do_ulonghex()
 *   Proc handler to handle unsigned longs such as 0xf0001000
 ***************************************************************************/
int proc_do_ulonghex(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp)
{
 unsigned long *s_buf;
 int len;
 char *p, c;
#define LONG_BUFF_LEN 10
 char long_buffer[SHORT_BUFF_LEN+1];
 unsigned long result;

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   s_buf = (unsigned long *) table->data;

   if (write) {

      len = 0;
      p = buffer;
      if (*lenp > LONG_BUFF_LEN) {
         return -EINVAL;
      }

      while (len < *lenp && (c = get_user(p++)) != 0 && c != '\n') {
         long_buffer[len++] = c;
      }
      long_buffer[len] = 0;

      *s_buf = simple_strtoul(long_buffer,0,0);

      filp->f_pos += *lenp;
   } else {

      result = *s_buf;
      sprintf(long_buffer,"0x%08x",result);
      len = strlen(long_buffer);
      if (len > *lenp) {
         len = *lenp;
      }
      if (len) {
         memcpy_tofs(buffer,long_buffer,len);
      }
      if (len < *lenp) {
         put_user('\n',((char *)buffer) + len);
         len++;
      }
      *lenp = len;
      filp->f_pos += len;
   }
   return 0;
}

/****************************************************************************
 * proc_do_snmp()
 ***************************************************************************/
int proc_do_snmp(ctl_table *table, int write, struct file *filp,
                  void *buffer, size_t *lenp)
{
 tp_snmp_communities *p;
 int len = 0;

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   if (write) {
      /* even though we're using the sysctl structs, we don't really want */
      /* to support write for the snmp tables, since people should be     */
      /* using set_pivio_table() rather than proc.. we might support this */
      /* later maybe.. probably better to move it over to a normal proc   */
      /* entry if we're not going to support a proc based write to it     */
      return -EINVAL;
   } else {
    int i;
    char ip_buffer[IP_LEN+1];

      p = (tp_snmp_communities *) table->data;
      for (i=0;i<XP_PROC_NUM_SNMP && (p!=NULL);i++) {
       int tlen;

         tlen = write_ip(p->ip,ip_buffer);

         if (len+tlen+1 > *lenp) {
            /* this is too big to fit in the return buffer */
            /* abort..                                     */
            printk("output exceeds proc buffer length\n");
            break;
         }

         if (tlen > 0) {
            memcpy_tofs(buffer+len,ip_buffer,tlen);
            len += tlen;
         } else {
            if (len+8+1 > *lenp) {
               break;
            }

            memcpy_tofs(buffer+len,"0.0.0.0",8);
            len += 8;
         }

         memcpy_tofs(buffer+len," ",1);
         len += 1;
 
         if ((p->community != NULL) && (p->community[0] != 0)) {
            tlen += strlen(p->community);

            if (len+tlen > *lenp) {
               break;
            }

            memcpy_tofs(buffer+len,p->community,tlen);
            len += tlen;
         } else {
            if (len+3 > *lenp) {
               break;
            }

            memcpy_tofs(buffer+len,"n/a",3);
            len += 3;
         }

         if (len+2 > *lenp) {
            break;
         }

         memcpy_tofs(buffer+len," ",1);
         len += 1;

         memcpy_tofs(buffer+len,"\n",1);
         len += 1;

         /* ack, buffer size problems. printk for now */
         printk("%lX / %lX / %s / %X\n",p->ip,p->nm,p->community,p->access);
         p++;
      }
      *lenp = len;
      filp->f_pos += len;
   }
   return 0;
}
/****************************************************************************
 * proc_do_pingsites()
 ***************************************************************************/
int proc_do_pingsites(ctl_table *table, int write, struct file *filp,
                      void *buffer, size_t *lenp)
{
 long *p;
 int len = 0;

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   if (write) {
      /* don't support writes to this */
      return -EINVAL;
   } else {
    int i;
    char ip_buffer[IP_LEN+1];

      p = (long *) table->data;
      for (i=0;i<XP_PROC_NUM_PING_SITES && (p!=NULL);i++) {
       int tlen;

         tlen = write_ip(*p,ip_buffer);

         if ((len+tlen+1) > *lenp) {
            /* this is too big to fit in the return buffer */
            /* abort..                                     */
            break;
         }

         if (tlen > 0) {
            memcpy_tofs(buffer+len,ip_buffer,tlen);
            len += tlen;
         } else {
            /* eek, last buffer overrun check had a 0 for tlen. check again */
            if ((len+8+1) > *lenp) {
               break;
            }

            memcpy_tofs(buffer+len,"0.0.0.0",8);
            len += 8;
         }

         memcpy_tofs(buffer+len,"\n",1);
         len += 1;

         p++;
      }
      *lenp = len;
      filp->f_pos += len;
   }
   return 0;
}
/****************************************************************************
 * proc_do_ipfw()
 ***************************************************************************/
int proc_do_ipfw(ctl_table *table, int write, struct file *filp,
                 void *buffer, size_t *lenp)
{
 tp_ipfw *p;
 int len = 0;

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   if (write) {
      /* don't support writes to this */
      return -EINVAL;
   } else {
    int i;
    char ip_buffer[IP_LEN+1];
    int iplen;

      p = (tp_ipfw *) table->data;

      for (i=0;i<XP_PROC_NUM_IPFWS && (p!=NULL);i++) {
       int tlen;

         iplen = write_ip(p->ip,ip_buffer);

         /* check for overwrite of iplen + " -> 32767\n"   */
         if ((len+iplen+5+5) > *lenp) {
            /* this is too big to fit in the return buffer */
            /* abort..                                     */
            break;
         }

         tlen = sprintf(buffer+len,"%u",p->port);
         len += tlen;

         memcpy_tofs(buffer+len," -> ",4);
         len += 4;

         if (iplen > 0) {
            memcpy_tofs(buffer+len,ip_buffer,iplen);
            len += iplen;
         } else {
            if ((len+8+1) > *lenp) {
               break;
            }

            memcpy_tofs(buffer+len,"0.0.0.0",8);
            len += 8;
         }

         memcpy_tofs(buffer+len,"\n",1);
         len += 1;

         p++;
      }

      *lenp = len;
      filp->f_pos += len;
   }

   return 0;
}
/****************************************************************************
 * proc_do_vpns()
 ***************************************************************************/
int proc_do_vpns(ctl_table *table, int write, struct file *filp,
                 void *buffer, size_t *lenp)
{
 tp_vpn *p;

 int len = 0;

   if (!table->data || !table->maxlen || !*lenp || (filp->f_pos && !write)) {
      *lenp = 0;
      return 0;
   }

   if (write) {
      /* don't support writes to this */
      return -EINVAL;
   } else {
    int i;

      p = (tp_vpn *) table->data;
 
      for (i=0;i<XP_PROC_NUM_VPNS && (p!=NULL);i++) {
       int tlen;
 
         tlen = strlen(p->name);
         if ((len+tlen+6+1) > *lenp) {
            break;
         }

         memcpy_tofs(buffer+len,p->name,tlen);
         len += tlen;
         
         memcpy_tofs(buffer+len," ",1);
         len += 1;

         switch(p->type) {
          case p_ipsec_3des:
            memcpy_tofs(buffer+len,"3DES ",5);
            len += 5;
            break;
          case p_ipsec_des_x:
            memcpy_tofs(buffer+len,"DES-X ",6);
            len += 6;
            break;
          case p_ipsec_des:
            memcpy_tofs(buffer+len,"DES ",4);
            len += 4;
            break;
          case p_ipsec_aes:
            memcpy_tofs(buffer+len,"AES ",4);
            len += 4;
            break;
          case p_vpn_pptp:
            memcpy_tofs(buffer+len,"PPTP ",5);
            len += 5;
            break;
          case p_vpn_ssh:
            memcpy_tofs(buffer+len,"SSH ",4);
            len += 4;
            break;
          case p_vpn_vpnd:
            memcpy_tofs(buffer+len,"VPND ",5);
            len += 5;
            break;
          default:
            memcpy_tofs(buffer+len,"? ",2);
            len += 2;
            break;
         }

         tlen = strlen(p->cfg_file);
         if ((len+tlen+1+9+1) > *lenp) {
            break;
         }
   
         memcpy_tofs(buffer+len,p->cfg_file,tlen);
         len += tlen;

         memcpy_tofs(buffer+len," ",1);
         len += 1;

         switch(p->started) {
          case p_started:
            memcpy_tofs(buffer+len,"started ",8);
            len += 8;
            break;
          case p_stopped:
            memcpy_tofs(buffer+len,"stopped ",8);
            len += 8;
            break;
          case p_starting:
            memcpy_tofs(buffer+len,"starting ",9);
            len += 9;
            break;
          case p_stopping:
            memcpy_tofs(buffer+len,"stopping ",9);
            len += 9;
            break;
          default:
            memcpy_tofs(buffer+len,"? ",2);
            len += 2;
            break;
         }

         memcpy_tofs(buffer+len,"\n",1);
         len += 1;

         p++;
      }

      *lenp = len;
   }
   return 0;
}
/****************************************************************************
 * sys_setpivioptr()
 *
 *   The setpivioptr() system call
 ***************************************************************************/
asmlinkage int sys_setpivioptr(int item, void *valptr)
{
   switch(item) {
    case PIVIO_CONFIGED:
      pivio_configed      = *((int *)valptr);
      break;
    case PIVIO_ONLINE:
      pivio_online        = *((int *)valptr);
      break;
    case PIVIO_WAN_LEASE:
      pivio_wan_lease     = *((int *)valptr);
      break;
    case PIVIO_LAN_LEASE:
      pivio_lan_lease     = *((int *)valptr);
      break;
    case PIVIO_WAN_TYPE:
      wan_type            = *((int *)valptr);
      break;
    case PIVIO_WAN_IP:
      pivio_ipwan         = *((long*) valptr);
      break;
    case PIVIO_WAN_NM:
      pivio_nmwan         = *((long *) valptr);
      break;
    case PIVIO_WAN_GW:
      pivio_gwwan         = *((long *) valptr);
      break;
    case PIVIO_LAN_STATIC:
      pivio_lan_static    = *((int *)valptr);
      break;
    case PIVIO_LAN_IP:
      pivio_iplan         = *((long *) valptr);
      break;
    case PIVIO_LAN_NM:
      pivio_nmlan         = *((long *) valptr);
      break;
    case PIVIO_LAN_DHCP_START:
      pivio_dhcp_start    = *((long *) valptr);
      break;
    case PIVIO_LAN_DHCP_END:
      pivio_dhcp_end      = *((long *) valptr);
      break;
    case PIVIO_XPD_PID:
      pivio_xpd_pid       = *((pid_t *) valptr);
      break;
    case PIVIO_DHCPD_PID:
      pivio_dhcpd_pid     = *((pid_t *) valptr);
      break;
    case PIVIO_DHCPC_PID:
      pivio_dhcpc_pid     = *((pid_t *) valptr);
      break;
    case PIVIO_DNS_PID:
      pivio_dns_pid       = *((pid_t *) valptr);
      break;
    case PIVIO_XPUPD_PID:
      pivio_xpupd_pid     = *((pid_t *) valptr);
      break;
    case PIVIO_PPPOE_PID:
      pivio_pppoe_pid     = *((pid_t *) valptr);
      break;
    case PIVIO_SNMPD_PID:
      pivio_snmpd_pid     = *((pid_t *) valptr);
      break;
    case PIVIO_PASSWORD:
      strncpy(pivio_password,valptr,XP_PROC_PW_LEN);
      break;
    case PIVIO_USERNAME:
      strncpy(pivio_username,valptr,XP_PROC_UN_LEN);
      break;
    case PIVIO_LED_ONLINE:
      pivio_led_online    = *((int*)valptr);
      break;
    case PIVIO_LED_LAN1:
      pivio_led_lan1      = *((int *)valptr);
      break;
    case PIVIO_LED_LAN2:
      pivio_led_lan2      = *((int *)valptr);
      break;
    case PIVIO_LED_WAN1:
      pivio_led_wan1      = *((int *)valptr);
      break;
    case PIVIO_LED_WAN2:
      pivio_led_wan2      = *((int *)valptr);
      break;
    case PIVIO_LED_UPDATE:
      pivio_led_update_avail = *((int *)valptr);
      break;
    case PIVIO_LED_VPN:
      pivio_led_vpn       = *((int *)valptr);
      break;
    case PIVIO_LED_USER:
      pivio_led_user      = *((int *)valptr);
      break;
    case PIVIO_FWOPT_QUIET_MODE:
      pivio_quiet_mode    = *((int *)valptr);
      break;
    case PIVIO_FWOPT_STEALTH_MODE:
      pivio_stealth_mode  = *((int *)valptr);
      break;
    case PIVIO_FWOPT_IPSEC_NAT:
      pivio_ipsec_nat     = *((int *)valptr);
      break;
    case PIVIO_FWOPT_WAN_NAT:
      pivio_wan_nat       = *((int *)valptr);
      break;
    case PIVIO_FORCE_PING:
      pivio_force_ping    = *((int *)valptr);
      break;
    case PIVIO_PLUTO_STARTED:
      pivio_pluto_started = *((int *)valptr);
      break;
    case PIVIO_XPD_MAJOR:
      pivio_xpd_major = *((int *)valptr);
      break;
    case PIVIO_XPD_MINOR:
      pivio_xpd_minor = *((int *)valptr);
      break;
    case PIVIO_XPD_PL:
      pivio_xpd_pl =*((int *)valptr);
      break;
    case PIVIO_XPD_PENDING:
      pivio_xpd_update_pending = *((int *)valptr);
      break;
    case PIVIO_XPD_FILE:
      strncpy(pivio_xpd_update_file,valptr,XP_PROC_STRLEN);
      break;
    case PIVIO_KERN_MAJOR:
      pivio_kernel_major = *((int *)valptr);
      break;
    case PIVIO_KERN_MINOR:
      pivio_kernel_minor = *((int *)valptr);
      break;
    case PIVIO_KERN_PL:
      pivio_kernel_pl = *((int *)valptr);
      break;
    case PIVIO_KERN_PENDING:
      pivio_kernel_update_pending = *((int *)valptr);
      break;
    case PIVIO_KERN_FILE:
      strncpy(pivio_kernel_update_file,valptr,XP_PROC_STRLEN);
      break;
    case PIVIO_BOOTLOADER_VER:
      pivio_bootloader_version = *((int *)valptr);
      break;
    case PIVIO_BOOTLOADER_PENDING:
      pivio_bootloader_update_pending = *((int *)valptr);
      break;
    case PIVIO_BOOTLOADER_FILE:
      strncpy(pivio_bootloader_update_file,valptr,XP_PROC_STRLEN);
      break;
    case PIVIO_WAN_PPPOE_USER:
      strncpy(pivio_pppoe_username,valptr,XP_PROC_STRLEN);
      break;
    case PIVIO_WAN_PPPOE_PW:
      strncpy(pivio_pppoe_password,valptr,XP_PROC_PW_LEN);
      break;
    case PIVIO_CFG_IP:
      pivio_config_ip           = *((long *) valptr);
      break;
    case PIVIO_UPDATE_IP:
      pivio_update_ip           = *((long *) valptr);
      break;
    case PIVIO_UPDATE_IP_FIXED:
      pivio_update_ip_locked    = *((unsigned short *) valptr);
      break;
    case PIVIO_UPDATE_AVAIL:
      pivio_update_avail        = *((unsigned short *) valptr);
      break;
    case PIVIO_UPDATE_ACKNOWLEDGED:
      pivio_update_acknowledged = *((unsigned short *) valptr);
      break;
    case PIVIO_WAN_DNS1:
      pivio_dns1wan = *((long *) valptr);
      break;
    case PIVIO_WAN_DNS2:
      pivio_dns2wan = *((long *) valptr);
      break;
    case PIVIO_WAN_DNS3:
      pivio_dns3wan = *((long *) valptr);
      break;
    case PIVIO_LAN_LEASE_ONLINE:
      pivio_lease_online = *((long *) valptr);
      break;
    case PIVIO_LAN_LEASE_OFFLINE:
      pivio_lease_offline = *((long *) valptr);
      break;
    case PIVIO_SNMP_ENABLED: 
      pivio_snmp_enabled = *((int *) valptr);
      break;
    case PIVIO_BIGBURN_PENDING:
      pivio_bigburn_pending = *((int *)valptr);
      break;
    case PIVIO_BIGBURN_FILE:
      strncpy(pivio_bigburn_file, valptr, XP_PROC_STRLEN);
      break;
    case PIVIO_ADDR_BOOTARGS:
      pivio_addr_bootargs = *((long *) valptr);
      break;
    case PIVIO_ADDR_MAC:
      pivio_addr_mac = *((long *) valptr);
      break;
    case PIVIO_ADDR_CONFIG:
      pivio_addr_config = *((long *) valptr);
      break;
    case PIVIO_LEN_BOOTARGS:
      pivio_len_bootargs = *((long *) valptr);
      break;
    case PIVIO_LEN_MAC:
      pivio_len_mac = *((long *) valptr);
      break;
    case PIVIO_LEN_CONFIG:
      pivio_len_config = *((long *) valptr);
      break;
    case PIVIO_NB_SETTINGS:
      pivio_nbSettings = *((tp_netbios *) valptr);
      break;
    case PIVIO_DHCP_LEASE_PERIOD:
      memcpy(pivio_dhcp_lease_period, valptr, sizeof(pivio_dhcp_lease_period));
      break;
    case PIVIO_SNMP_ETH0_AXS:
      pivio_snmp_eth0_axs = *((int *)valptr);
      break;
    case PIVIO_SNMP_ETH1_AXS:
      pivio_snmp_eth1_axs = *((int *)valptr);
      break;
    case PIVIO_SNMP_IPSEC0_AXS:
      pivio_snmp_ipsec0_axs = *((int *)valptr);
      break;
    default:
      return -EINVAL;
   }
   return 0;
}

/****************************************************************************
 * sys_set_pivio()
 *
 *   The setpivio() system call
 ***************************************************************************/
asmlinkage int sys_setpivio(unsigned int item, unsigned long val)
{
   switch(item) {
    case PIVIO_CONFIGED:
      pivio_configed      = val;
      break;
    case PIVIO_ONLINE:
      pivio_online        = val;
      break;
    case PIVIO_WAN_LEASE:
      pivio_wan_lease     = val;
      break;
    case PIVIO_LAN_LEASE:
      pivio_lan_lease     = val;
      break;
    case PIVIO_WAN_TYPE:
      wan_type            = val;
      break;
    case PIVIO_WAN_IP:
      pivio_ipwan         = val;
      break;
    case PIVIO_WAN_NM:
      pivio_nmwan         = val;
      break;
    case PIVIO_WAN_GW:
      pivio_gwwan         = val;
      break;
    case PIVIO_LAN_STATIC:
      pivio_lan_static    = val;
      break;
    case PIVIO_LAN_IP:
      pivio_iplan         = val;
      break;
    case PIVIO_LAN_NM:
      pivio_nmlan         = val;
      break;
    case PIVIO_LAN_DHCP_START:
      pivio_dhcp_start    = val;
      break;
    case PIVIO_LAN_DHCP_END:
      pivio_dhcp_end      = val;
      break;
    case PIVIO_XPD_PID:
      pivio_xpd_pid       = val;
      break;
    case PIVIO_DHCPD_PID:
      pivio_dhcpd_pid     = val;
      break;
    case PIVIO_DHCPC_PID:
      pivio_dhcpc_pid     = val;
      break;
    case PIVIO_DNS_PID:
      pivio_dns_pid       = val;
      break;
    case PIVIO_XPUPD_PID:
      pivio_xpupd_pid     = val;
      break;
    case PIVIO_PPPOE_PID:
      pivio_pppoe_pid     = val;
      break;
    case PIVIO_SNMPD_PID:
      pivio_snmpd_pid     = val;
      break;
    case PIVIO_LED_ONLINE:
      pivio_led_online    = val;
      break;
    case PIVIO_LED_LAN1:
      pivio_led_lan1      = val;
      break;
    case PIVIO_LED_LAN2:
      pivio_led_lan2      = val; 
      break;
    case PIVIO_LED_WAN1:
      pivio_led_wan1      = val; 
      break;
    case PIVIO_LED_WAN2:
      pivio_led_wan2      = val; 
      break;
    case PIVIO_LED_UPDATE:
      pivio_led_update_avail = val; 
      break;
    case PIVIO_LED_VPN:
      pivio_led_vpn       = val; 
      break;
    case PIVIO_LED_USER:
      pivio_led_user      = val; 
      break;
    case PIVIO_FWOPT_QUIET_MODE:
      pivio_quiet_mode    = val; 
      break;
    case PIVIO_FWOPT_STEALTH_MODE:
      pivio_stealth_mode  = val; 
      break;
    case PIVIO_FWOPT_IPSEC_NAT:
      pivio_ipsec_nat     = val; 
      break;
    case PIVIO_FWOPT_WAN_NAT:
      pivio_wan_nat       = val; 
      break;
    case PIVIO_FORCE_PING:
      pivio_force_ping    = val;
      break;
    case PIVIO_PLUTO_STARTED:
      pivio_pluto_started = val;
      break;
    case PIVIO_XPD_MAJOR:
      pivio_xpd_major = val;
      break;
    case PIVIO_XPD_MINOR:
      pivio_xpd_minor = val;
      break;
    case PIVIO_XPD_PL:
      pivio_xpd_pl =  val;
      break;
    case PIVIO_XPD_PENDING:
      pivio_xpd_update_pending = val;
      break;
    case PIVIO_KERN_MAJOR:
      pivio_kernel_major = val;
      break;
    case PIVIO_KERN_MINOR:
      pivio_kernel_minor = val;
      break;
    case PIVIO_KERN_PL:
      pivio_kernel_pl = val;
      break;
    case PIVIO_KERN_PENDING:
      pivio_kernel_update_pending = val;
      break;
    case PIVIO_BOOTLOADER_VER:
      pivio_bootloader_version = val;
      break;
    case PIVIO_BOOTLOADER_PENDING:
      pivio_bootloader_update_pending = val;
      break;
    case PIVIO_CFG_IP:
      pivio_config_ip           = val;
      break;
    case PIVIO_UPDATE_IP:
      pivio_update_ip           = val;
      break;
    case PIVIO_UPDATE_IP_FIXED:
      pivio_update_ip_locked    = val;
      break;
    case PIVIO_UPDATE_AVAIL:
      pivio_update_avail        = val;
      break;
    case PIVIO_UPDATE_ACKNOWLEDGED:
      pivio_update_acknowledged = val;
      break;
    case PIVIO_WAN_DNS1:
      pivio_dns1wan = val;
      break;
    case PIVIO_WAN_DNS2:
      pivio_dns2wan = val;
      break;
    case PIVIO_WAN_DNS3:
      pivio_dns3wan = val;
      break;
    case PIVIO_LAN_LEASE_ONLINE:
      pivio_lease_online = val;
      break;
    case PIVIO_LAN_LEASE_OFFLINE:
      pivio_lease_offline = val;
      break;
    case PIVIO_SNMP_ENABLED: 
      pivio_snmp_enabled = val;
      break;
    case PIVIO_BIGBURN_PENDING:
      pivio_bigburn_pending = val;
      break;
    case PIVIO_ADDR_BOOTARGS:
      pivio_addr_bootargs = val;
      break;
    case PIVIO_ADDR_MAC:
      pivio_addr_mac = val;
      break;
    case PIVIO_ADDR_CONFIG:
      pivio_addr_config = val;
      break;
    case PIVIO_LEN_BOOTARGS:
      pivio_len_bootargs = val;
      break;
    case PIVIO_LEN_MAC:
      pivio_len_mac = val;
      break;
    case PIVIO_LEN_CONFIG:
      pivio_len_config = val;
      break;
    case PIVIO_SNMP_ETH0_AXS:
      pivio_snmp_eth0_axs = val;
      break;
    case PIVIO_SNMP_ETH1_AXS:
      pivio_snmp_eth1_axs = val;
      break;
    case PIVIO_SNMP_IPSEC0_AXS:
      pivio_snmp_ipsec0_axs = val;
      break;
    default:
      return -EINVAL;
   }
   return 0;
}
/****************************************************************************
 * sys_getpivio()
 *
 *   The getpivio() system call
 ***************************************************************************/
asmlinkage int sys_getpivio(unsigned int item, void *ptr)
{
   if (ptr == NULL) {
      return -EINVAL;
   }

   switch(item) {
    case PIVIO_CONFIGED:
      *(int *) ptr = (short) pivio_configed;
      break;
    case PIVIO_ONLINE:
      *(int *) ptr = (short) pivio_online;
      break;
    case PIVIO_WAN_LEASE:
      *(int *) ptr = (short) pivio_wan_lease;
      break;
    case PIVIO_LAN_LEASE:
      *(int *) ptr = (short) pivio_lan_lease;
      break;
    case PIVIO_WAN_TYPE:
      *(int *) ptr = (int) wan_type;
      break;
    case PIVIO_WAN_IP:
      *(long *) ptr = pivio_ipwan;
      break;
    case PIVIO_WAN_NM:
      *(long *) ptr = pivio_nmwan;
      break;
    case PIVIO_WAN_GW:
      *(long *) ptr = pivio_gwwan;
      break;
    case PIVIO_LAN_STATIC:
      *(int *) ptr = (short) pivio_lan_static;
      break;
    case PIVIO_LAN_IP:
      *(long *) ptr = pivio_iplan;
      break;
    case PIVIO_LAN_NM:
      *(long *) ptr = pivio_nmlan;
      break;
    case PIVIO_LAN_DHCP_START:
      *(long *) ptr = pivio_dhcp_start;
      break;
    case PIVIO_LAN_DHCP_END:
      *(long *) ptr = pivio_dhcp_end;
      break;
    case PIVIO_XPD_PID:
      *(pid_t*) ptr = pivio_xpd_pid;
      break;
    case PIVIO_DHCPD_PID:
      *(pid_t*) ptr = pivio_dhcpd_pid;
      break;
    case PIVIO_DHCPC_PID:
      *(pid_t*) ptr = pivio_dhcpc_pid;
      break;
    case PIVIO_DNS_PID:
      *(pid_t*) ptr = pivio_dns_pid;
      break;
    case PIVIO_XPUPD_PID:
      *(pid_t*) ptr = pivio_xpupd_pid;
      break;
    case PIVIO_PPPOE_PID:
      *(pid_t*) ptr = pivio_pppoe_pid;
      break;
    case PIVIO_SNMPD_PID:
      *(pid_t*) ptr = pivio_snmpd_pid;
      break;
    case PIVIO_PASSWORD:
      strcpy(ptr,pivio_password);
      break;
    case PIVIO_USERNAME:
      strcpy(ptr,pivio_username);
      break;
    case PIVIO_LED_ONLINE:
      *(int *) ptr = (short) pivio_led_online;
      break;
    case PIVIO_LED_LAN1:
      *(int *) ptr = (short) pivio_led_lan1;
      break;
    case PIVIO_LED_LAN2:
      *(int *) ptr = (short) pivio_led_lan2;
      break;
    case PIVIO_LED_WAN1:
      *(int *) ptr = (short) pivio_led_wan1;
      break;
    case PIVIO_LED_WAN2:
      *(int *) ptr = (short) pivio_led_wan2;
      break;
    case PIVIO_LED_UPDATE:
      *(int *) ptr = (short) pivio_led_update_avail;
      break;
    case PIVIO_LED_VPN:
      *(int *) ptr = (short) pivio_led_vpn;
      break;
    case PIVIO_LED_USER:
      *(int *) ptr = (short) pivio_led_user;
      break;
    case PIVIO_FWOPT_QUIET_MODE:
      *(int *) ptr = (short) pivio_quiet_mode;
      break;
    case PIVIO_FWOPT_STEALTH_MODE:
      *(int *) ptr = (short) pivio_stealth_mode;
      break;
    case PIVIO_FWOPT_IPSEC_NAT:
      *(int *) ptr = (short) pivio_ipsec_nat;
      break;
    case PIVIO_FWOPT_WAN_NAT:
      *(int *) ptr = (short) pivio_wan_nat;
      break;
    case PIVIO_FORCE_PING:
      *(int *) ptr = (short) pivio_force_ping;
      break;
    case PIVIO_PLUTO_STARTED:
      *(int *) ptr = (short) pivio_pluto_started;
      break;
    case PIVIO_XPD_MAJOR:
      *(int *) ptr = (short) pivio_xpd_major;
      break;
    case PIVIO_XPD_MINOR:
      *(int *) ptr = (short) pivio_xpd_minor;
      break;
    case PIVIO_XPD_PL:
      *(int *) ptr = (short) pivio_xpd_pl;
      break;
    case PIVIO_XPD_PENDING:
      *(int *) ptr = (short) pivio_xpd_update_pending;
      break;
    case PIVIO_XPD_FILE:
      strcpy(ptr,pivio_xpd_update_file);
      break;
    case PIVIO_KERN_MAJOR:
      *(int *) ptr = (short) pivio_kernel_major;
      break;
    case PIVIO_KERN_MINOR:
      *(int *) ptr = (short) pivio_kernel_minor;
      break;
    case PIVIO_KERN_PL:
      *(int *) ptr = (short) pivio_kernel_pl;
      break;
    case PIVIO_KERN_PENDING:
      *(int *) ptr = (short) pivio_kernel_update_pending;
      break;
    case PIVIO_KERN_FILE:
      strcpy(ptr,pivio_kernel_update_file);
      break;
    case PIVIO_BOOTLOADER_VER:
      *(int *) ptr = (short) pivio_bootloader_version;
      break;
    case PIVIO_BOOTLOADER_PENDING:
      *(int *) ptr = (short) pivio_bootloader_update_pending;
      break;
    case PIVIO_BOOTLOADER_FILE:
      strcpy(ptr,pivio_bootloader_update_file);
      break;
    case PIVIO_WAN_PPPOE_USER:
      strcpy(ptr,pivio_pppoe_username);
      break;
    case PIVIO_WAN_PPPOE_PW:
      strcpy(ptr,pivio_pppoe_password);
      break;
    case PIVIO_CFG_IP:
      *(long *) ptr = pivio_config_ip;
      break;
    case PIVIO_UPDATE_IP:
      *(long *) ptr = pivio_update_ip;
      break;
    case PIVIO_UPDATE_IP_FIXED:
      *(int *) ptr = (short) pivio_update_ip_locked;
      break;
    case PIVIO_UPDATE_AVAIL:
      *(int *) ptr = (short) pivio_update_avail;
      break;
    case PIVIO_UPDATE_ACKNOWLEDGED:
      *(int *) ptr = (short) pivio_update_acknowledged;
      break;
    case PIVIO_WAN_DNS1:
      *(unsigned long *) ptr = pivio_dns1wan;
      break;
    case PIVIO_WAN_DNS2:
      *(unsigned long *) ptr = pivio_dns2wan;
      break;
    case PIVIO_WAN_DNS3:
      *(unsigned long *) ptr = pivio_dns3wan;
      break;
    case PIVIO_LAN_LEASE_ONLINE:
      *(unsigned long *) ptr = pivio_lease_online;
      break;
    case PIVIO_LAN_LEASE_OFFLINE:
      *(unsigned long *) ptr = pivio_lease_offline;
      break;
    case PIVIO_SNMP_ENABLED: 
      *(int *) ptr = pivio_snmp_enabled;
      break;
    case PIVIO_BIGBURN_PENDING:
      *(int *) ptr = (short) pivio_bigburn_pending;
      break;
    case PIVIO_BIGBURN_FILE:
      strcpy(ptr,pivio_bigburn_file);
      break;
    case PIVIO_ADDR_BOOTARGS:
      *(long *) ptr = pivio_addr_bootargs;
      break;
    case PIVIO_ADDR_MAC:
      *(long *) ptr = pivio_addr_mac;
      break;
    case PIVIO_ADDR_CONFIG:
      *(long *) ptr = pivio_addr_config;
      break;
    case PIVIO_LEN_BOOTARGS:
      *(long *) ptr = pivio_len_bootargs;
      break;
    case PIVIO_LEN_MAC:
      *(long *) ptr = pivio_len_mac;
      break;
    case PIVIO_LEN_CONFIG:
      *(long *) ptr = pivio_len_config;
      break;
    case PIVIO_NB_SETTINGS: 
      *(tp_netbios *) ptr = pivio_nbSettings;
      break;
    case PIVIO_DHCP_LEASE_PERIOD:
      memcpy(ptr, pivio_dhcp_lease_period, sizeof(pivio_dhcp_lease_period));
      break;
    case PIVIO_SNMP_ETH0_AXS:
      *(int *) ptr = (short) pivio_snmp_eth0_axs;
      break;
    case PIVIO_SNMP_ETH1_AXS:
      *(int *) ptr = (short) pivio_snmp_eth1_axs;
      break;
    case PIVIO_SNMP_IPSEC0_AXS:
      *(int *) ptr = (short) pivio_snmp_ipsec0_axs;
      break;
    default:
      return -EINVAL;
   }
   return 0;
}

/****************************************************************************
 * sys_getpiviosize()
 *
 *   The getpiviosize() system call
 ***************************************************************************/
asmlinkage int sys_getpiviosize(unsigned int item)
{
   return 0;
}

/****************************************************************************
 * sys_getpiviocount()
 *
 *   The getpiviocount() system call
 ***************************************************************************/
asmlinkage int sys_getpiviocount(unsigned int item)
{
    switch (item)
    {
        case PIVIO_SNMP_COMMUNITIES:
            return XP_PROC_NUM_SNMP;

        case PIVIO_PING_SITES:
            return XP_PROC_NUM_PING_SITES;

        case PIVIO_VPNS:
            return XP_PROC_NUM_VPNS;

        case PIVIO_IPFWS:
            return XP_PROC_NUM_IPFWS;

        default:
            return -EINVAL;
    }

   return 0;
}



/****************************************************************************
 * sys_getpivioelemsize()
 *
 *   The getpivioelemsize() system call
 ***************************************************************************/
asmlinkage int sys_getpivioelemsize(unsigned int item,int ndx)
{
   return 0;
}

/****************************************************************************
 * sys_getpivioelem()
 *
 *   The getpivioelem() system call
 ***************************************************************************/
asmlinkage int sys_getpivioelem(unsigned int item,int ndx, void *ptr)
{
    int     count;

    if (ptr == NULL)
        return -EINVAL;

    if ((count = sys_getpiviocount(item)) < 1)
        return -EINVAL;

    if (ndx < 0 || ndx >= count)
        return -ERANGE;

    switch (item)
    {
        case PIVIO_SNMP_COMMUNITIES:
            *(tp_snmp_communities *)ptr = pivio_snmp[ndx];
            break;

        case PIVIO_PING_SITES:
            *(unsigned long *)ptr = pivio_ping_sites[ndx];
            break;

        case PIVIO_VPNS:
            *(tp_vpn *)ptr = pivio_vpns[ndx];
            break;

        case PIVIO_IPFWS:
            *(tp_ipfw *)ptr = pivio_port_forwards[ndx];
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

/****************************************************************************
 * sys_setpivioelem()
 *
 *   The setpivioelem() system call
 ***************************************************************************/
asmlinkage int sys_setpivioelem(unsigned int item,int ndx, unsigned long val)
{
    int     count;

    if ((count = sys_getpiviocount(item)) < 1)
        return -EINVAL;

    if (ndx < 0 || ndx >= count)
        return -ERANGE;

    switch (item)
    {
        case PIVIO_PING_SITES:
            pivio_ping_sites[ndx] = val;
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

/****************************************************************************
 * sys_setpivioelemptr()
 *
 *   The setpivioelemptr() system call
 ***************************************************************************/
asmlinkage int sys_setpivioelemptr(unsigned int item,int ndx, void *ptr)
{
    int     count;

    if (ptr == NULL)
        return -EINVAL;

    if ((count = sys_getpiviocount(item)) < 1)
        return -EINVAL;

    if (ndx < 0 || ndx >= count)
        return -ERANGE;

    switch (item)
    {
        case PIVIO_SNMP_COMMUNITIES:
            pivio_snmp[ndx] = *(tp_snmp_communities *)ptr;
            break;

        case PIVIO_PING_SITES:
            pivio_ping_sites[ndx] = *(unsigned long *)ptr;
            break;

        case PIVIO_VPNS:
            pivio_vpns[ndx]= *(tp_vpn *)ptr;
            break;

        case PIVIO_IPFWS:
            pivio_port_forwards[ndx]= *(tp_ipfw *)ptr;
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

/****************************************************************************
 * sys_getpiviostrsize()
 *
 *   The getpiviostrsize() system call
 ***************************************************************************/
asmlinkage int sys_getpiviostrsize(unsigned int item)
{
   return 0;
}

#endif /* CROSSPORT_PROC_GLOBALS */
