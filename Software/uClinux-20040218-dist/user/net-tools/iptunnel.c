/*
 * iptunnel.c	       "ip tunnel"
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 *
 * Changes:
 *
 * Rani Assaf <rani@magic.metawire.com> 980929:	resolve addresses
 * Rani Assaf <rani@magic.metawire.com> 980930:	do not allow key for ipip/sit
 * Bernd Eckenfels 990715: add linux/types.h (not clean but solves missing __u16
 * Arnaldo Carvalho de Melo 20010404: use setlocale
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1))
#include <net/if.h>
#include <net/if_arp.h>
#else
#include <linux/if.h>
#include <linux/if_arp.h>
#endif
#include <linux/types.h>
#include <linux/if_tunnel.h>

#include "config.h"
#include "intl.h"
#include "net-support.h"
#include "version.h"
#include "util.h"

#undef GRE_CSUM
#define GRE_CSUM	htons(0x8000)
#undef GRE_ROUTING
#define GRE_ROUTING	htons(0x4000)
#undef GRE_KEY
#define GRE_KEY		htons(0x2000)
#undef GRE_SEQ
#define GRE_SEQ		htons(0x1000)
#undef GRE_STRICT
#define GRE_STRICT	htons(0x0800)
#undef GRE_REC
#define GRE_REC		htons(0x0700)
#undef GRE_FLAGS
#define GRE_FLAGS	htons(0x00F8)
#undef GRE_VERSION
#define GRE_VERSION	htons(0x0007)

/* Old versions of glibc do not define this */
#if __GLIBC__ == 2 && __GLIBC_MINOR__ == 0
#define IPPROTO_GRE	47
#endif

#include "util-ank.h"

char *Release = RELEASE,
     *Version = "iptunnel 1.01",
     *Signature = "Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>";

static void version(void)
{
	printf("%s\n%s\n%s\n", Release, Version, Signature);
	exit(E_VERSION);
}

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, _("Usage: iptunnel { add | change | del | show } [ NAME ]\n"));
	fprintf(stderr, _("          [ mode { ipip | gre | sit } ] [ remote ADDR ] [ local ADDR ]\n"));
	fprintf(stderr, _("          [ [i|o]seq ] [ [i|o]key KEY ] [ [i|o]csum ]\n"));
	fprintf(stderr, _("          [ ttl TTL ] [ tos TOS ] [ nopmtudisc ] [ dev PHYS_DEV ]\n"));
	fprintf(stderr, _("       iptunnel -V | --version\n\n"));
	fprintf(stderr, _("Where: NAME := STRING\n"));
	fprintf(stderr, _("       ADDR := { IP_ADDRESS | any }\n"));
	fprintf(stderr, _("       TOS  := { NUMBER | inherit }\n"));
	fprintf(stderr, _("       TTL  := { 1..255 | inherit }\n"));
	fprintf(stderr, _("       KEY  := { DOTTED_QUAD | NUMBER }\n"));
	exit(-1);
}

static int do_ioctl_get_ifindex(char *dev)
{
	struct ifreq ifr;
	int fd;
	int err;

	strcpy(ifr.ifr_name, dev);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (err) {
		perror("ioctl");
		return 0;
	}
	close(fd);
	return ifr.ifr_ifindex;
}

static int do_ioctl_get_iftype(char *dev)
{
	struct ifreq ifr;
	int fd;
	int err;

	strcpy(ifr.ifr_name, dev);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFHWADDR, &ifr);
	if (err) {
		perror("ioctl");
		return -1;
	}
	close(fd);
	return ifr.ifr_addr.sa_family;
}


static char * do_ioctl_get_ifname(int idx)
{
	static struct ifreq ifr;
	int fd;
	int err;

	ifr.ifr_ifindex = idx;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFNAME, &ifr);
	if (err) {
		perror("ioctl");
		return NULL;
	}
	close(fd);
	return ifr.ifr_name;
}



static int do_get_ioctl(char *basedev, struct ip_tunnel_parm *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	strcpy(ifr.ifr_name, basedev);
	ifr.ifr_ifru.ifru_data = (void*)p;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGETTUNNEL, &ifr);
	if (err)
		perror("ioctl");
	close(fd);
	return err;
}

static int do_add_ioctl(int cmd, char *basedev, struct ip_tunnel_parm *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	strcpy(ifr.ifr_name, basedev);
	ifr.ifr_ifru.ifru_data = (void*)p;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, cmd, &ifr);
	if (err)
		perror("ioctl");
	close(fd);
	return err;
}

static int do_del_ioctl(char *basedev, struct ip_tunnel_parm *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	strcpy(ifr.ifr_name, basedev);
	ifr.ifr_ifru.ifru_data = (void*)p;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCDELTUNNEL, &ifr);
	if (err)
		perror("ioctl");
	close(fd);
	return err;
}

static int parse_args(int argc, char **argv, struct ip_tunnel_parm *p)
{
	char medium[IFNAMSIZ];

	memset(p, 0, sizeof(*p));
	memset(&medium, 0, sizeof(medium));

	p->iph.version = 4;
	p->iph.ihl = 5;
#ifndef IP_DF
#define IP_DF		0x4000		/* Flag: "Don't Fragment"	*/
#endif
	p->iph.frag_off = htons(IP_DF);

	while (argc > 0) {
		if (strcmp(*argv, "mode") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "ipip") == 0) {
				if (p->iph.protocol)
					usage();
				p->iph.protocol = IPPROTO_IPIP;
			} else if (strcmp(*argv, "gre") == 0) {
				if (p->iph.protocol)
					usage();
				p->iph.protocol = IPPROTO_GRE;
			} else if (strcmp(*argv, "sit") == 0) {
				if (p->iph.protocol)
					usage();
				p->iph.protocol = IPPROTO_IPV6;
			} else
				usage();
		} else if (strcmp(*argv, "key") == 0) {
			unsigned uval;
			NEXT_ARG();
			p->i_flags |= GRE_KEY;
			p->o_flags |= GRE_KEY;
			if (strchr(*argv, '.'))
				p->i_key = p->o_key = get_addr32(*argv);
			else {
				if (scan_number(*argv, &uval)<0)
					usage();
				p->i_key = p->o_key = htonl(uval);
			}
		} else if (strcmp(*argv, "ikey") == 0) {
			unsigned uval;
			NEXT_ARG();
			p->i_flags |= GRE_KEY;
			if (strchr(*argv, '.'))
				p->o_key = get_addr32(*argv);
			else {
				if (scan_number(*argv, &uval)<0)
					usage();
				p->i_key = htonl(uval);
			}
		} else if (strcmp(*argv, "okey") == 0) {
			unsigned uval;
			NEXT_ARG();
			p->o_flags |= GRE_KEY;
			if (strchr(*argv, '.'))
				p->o_key = get_addr32(*argv);
			else {
				if (scan_number(*argv, &uval)<0)
					usage();
				p->o_key = htonl(uval);
			}
		} else if (strcmp(*argv, "seq") == 0) {
			p->i_flags |= GRE_SEQ;
			p->o_flags |= GRE_SEQ;
		} else if (strcmp(*argv, "iseq") == 0) {
			p->i_flags |= GRE_SEQ;
		} else if (strcmp(*argv, "oseq") == 0) {
			p->o_flags |= GRE_SEQ;
		} else if (strcmp(*argv, "csum") == 0) {
			p->i_flags |= GRE_CSUM;
			p->o_flags |= GRE_CSUM;
		} else if (strcmp(*argv, "icsum") == 0) {
			p->i_flags |= GRE_CSUM;
		} else if (strcmp(*argv, "ocsum") == 0) {
			p->o_flags |= GRE_CSUM;
		} else if (strcmp(*argv, "nopmtudisc") == 0) {
			p->iph.frag_off = 0;
		} else if (strcmp(*argv, "remote") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "any"))
				p->iph.daddr = get_addr32(*argv);
		} else if (strcmp(*argv, "local") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "any"))
				p->iph.saddr = get_addr32(*argv);
		} else if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			safe_strncpy(medium, *argv, IFNAMSIZ-1);
		} else if (strcmp(*argv, "ttl") == 0) {
			unsigned uval;
			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0) {
				if (scan_number(*argv, &uval)<0)
					usage();
				if (uval > 255)
					usage();
				p->iph.ttl = uval;
			}
		} else if (strcmp(*argv, "tos") == 0) {
			unsigned uval;
			NEXT_ARG();
			if (strcmp(*argv, "inherit") != 0) {
				if (scan_number(*argv, &uval)<0)
					usage();
				if (uval > 255)
					usage();
				p->iph.tos = uval;
			} else
				p->iph.tos = 1;
		} else {
			if (p->name[0])
				usage();
			safe_strncpy(p->name, *argv, IFNAMSIZ);
		}
		argc--; argv++;
	}

	if (p->iph.protocol == 0) {
		if (memcmp(p->name, "gre", 3) == 0)
			p->iph.protocol = IPPROTO_GRE;
		else if (memcmp(p->name, "ipip", 4) == 0)
			p->iph.protocol = IPPROTO_IPIP;
		else if (memcmp(p->name, "sit", 3) == 0)
			p->iph.protocol = IPPROTO_IPV6;
	}

	if (p->iph.protocol == IPPROTO_IPIP || p->iph.protocol == IPPROTO_IPV6) {
		if ((p->i_flags & GRE_KEY) || (p->o_flags & GRE_KEY)) {
			fprintf(stderr, _("Keys are not allowed with ipip and sit.\n"));
			return -1;
		}
	}

	if (medium[0]) {
		p->link = do_ioctl_get_ifindex(medium);
		if (p->link == 0)
			return -1;
	}

	if (p->i_key == 0 && IN_MULTICAST(ntohl(p->iph.daddr))) {
		p->i_key = p->iph.daddr;
		p->i_flags |= GRE_KEY;
	}
	if (p->o_key == 0 && IN_MULTICAST(ntohl(p->iph.daddr))) {
		p->o_key = p->iph.daddr;
		p->o_flags |= GRE_KEY;
	}
	if (IN_MULTICAST(ntohl(p->iph.daddr)) && !p->iph.saddr) {
		fprintf(stderr, _("Broadcast tunnel requires a source address.\n"));
		return -1;
	}
	return 0;
}


static int do_add(int cmd, int argc, char **argv)
{
	struct ip_tunnel_parm p;

	if (parse_args(argc, argv, &p) < 0)
		return -1;

	if (p.iph.ttl && p.iph.frag_off == 0) {
		fprintf(stderr, _("ttl != 0 and noptmudisc are incompatible\n"));
		return -1;
	}

	switch (p.iph.protocol) {
	case IPPROTO_IPIP:
		return do_add_ioctl(cmd, "tunl0", &p);
	case IPPROTO_GRE:
		return do_add_ioctl(cmd, "gre0", &p);
	case IPPROTO_IPV6:
		return do_add_ioctl(cmd, "sit0", &p);
	default:	
		fprintf(stderr, _("cannot determine tunnel mode (ipip, gre or sit)\n"));
		return -1;
	}
	return -1;
}

int do_del(int argc, char **argv)
{
	struct ip_tunnel_parm p;

	if (parse_args(argc, argv, &p) < 0)
		return -1;

	switch (p.iph.protocol) {
	case IPPROTO_IPIP:	
		return do_del_ioctl(p.name[0] ? p.name : "tunl0", &p);
	case IPPROTO_GRE:	
		return do_del_ioctl(p.name[0] ? p.name : "gre0", &p);
	case IPPROTO_IPV6:	
		return do_del_ioctl(p.name[0] ? p.name : "sit0", &p);
	default:	
		return do_del_ioctl(p.name, &p);
	}
	return -1;
}

void print_tunnel(struct ip_tunnel_parm *p)
{
	char s1[256];
	char s2[256];
	char s3[64];
	char s4[64];

	format_host(AF_INET, &p->iph.daddr, s1, sizeof(s1));
	format_host(AF_INET, &p->iph.saddr, s2, sizeof(s2));
	inet_ntop(AF_INET, &p->i_key, s3, sizeof(s3));
	inet_ntop(AF_INET, &p->o_key, s4, sizeof(s4));

	printf(_("%s: %s/ip  remote %s  local %s "),
	       p->name,
	       p->iph.protocol == IPPROTO_IPIP ? "ip" :
	       (p->iph.protocol == IPPROTO_GRE ? "gre" :
		(p->iph.protocol == IPPROTO_IPV6 ? "ipv6" : _("unknown"))),
	       p->iph.daddr ? s1 : "any", p->iph.saddr ? s2 : "any");
	if (p->link) {
		char *n = do_ioctl_get_ifname(p->link);
		if (n)
			printf(" dev %s ", n);
	}
	if (p->iph.ttl)
		printf(" ttl %d ", p->iph.ttl);
	else
		printf(" ttl inherit ");
	if (p->iph.tos) {
		printf(" tos");
		if (p->iph.tos&1)
			printf(" inherit");
		if (p->iph.tos&~1)
			printf("%c%02x ", p->iph.tos&1 ? '/' : ' ', p->iph.tos&~1);
	}
	if (!(p->iph.frag_off&htons(IP_DF)))
		printf(" nopmtudisc");

	if ((p->i_flags&GRE_KEY) && (p->o_flags&GRE_KEY) && p->o_key == p->i_key)
		printf(" key %s", s3);
	else if ((p->i_flags|p->o_flags)&GRE_KEY) {
		if (p->i_flags&GRE_KEY)
			printf(" ikey %s ", s3);
		if (p->o_flags&GRE_KEY)
			printf(" okey %s ", s4);
	}
	printf("\n");

	if (p->i_flags&GRE_SEQ)
		printf(_("  Drop packets out of sequence.\n"));
	if (p->i_flags&GRE_CSUM)
		printf(_("  Checksum in received packet is required.\n"));
	if (p->o_flags&GRE_SEQ)
		printf(_("  Sequence packets on output.\n"));
	if (p->o_flags&GRE_CSUM)
		printf(_("  Checksum output packets.\n"));
}

static int do_tunnels_list(struct ip_tunnel_parm *p)
{
	char name[IFNAMSIZ];
	unsigned long  rx_bytes, rx_packets, rx_errs, rx_drops,
	rx_fifo, rx_frame,
	tx_bytes, tx_packets, tx_errs, tx_drops,
	tx_fifo, tx_colls, tx_carrier, rx_multi;
	int type;
	struct ip_tunnel_parm p1;

	char buf[512];
	FILE *fp = fopen("/proc/net/dev", "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}

	fgets(buf, sizeof(buf), fp);
	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *ptr;
		buf[sizeof(buf) - 1] = 0;
		if ((ptr = strchr(buf, ':')) == NULL ||
		    (*ptr++ = 0, sscanf(buf, "%s", name) != 1)) {
			fprintf(stderr, _("Wrong format of /proc/net/dev. Sorry.\n"));
			return -1;
		}
		if (sscanf(ptr, "%ld%ld%ld%ld%ld%ld%ld%*d%ld%ld%ld%ld%ld%ld%ld",
			   &rx_bytes, &rx_packets, &rx_errs, &rx_drops,
			   &rx_fifo, &rx_frame, &rx_multi,
			   &tx_bytes, &tx_packets, &tx_errs, &tx_drops,
			   &tx_fifo, &tx_colls, &tx_carrier) != 14)
			continue;
		if (p->name[0] && strcmp(p->name, name))
			continue;
		type = do_ioctl_get_iftype(name);
		if (type == -1) {
			fprintf(stderr, _("Failed to get type of [%s]\n"), name);
			continue;
		}
		if (type != ARPHRD_TUNNEL && type != ARPHRD_IPGRE && type != ARPHRD_SIT)
			continue;
		memset(&p1, 0, sizeof(p1));
		if (do_get_ioctl(name, &p1))
			continue;
		if ((p->link && p1.link != p->link) ||
		    (p->name[0] && strcmp(p1.name, p->name)) ||
		    (p->iph.daddr && p1.iph.daddr != p->iph.daddr) ||
		    (p->iph.saddr && p1.iph.saddr != p->iph.saddr) ||
		    (p->i_key && p1.i_key != p->i_key))
			continue;
		print_tunnel(&p1);
		if (show_stats) {
			printf(_("RX: Packets    Bytes        Errors CsumErrs OutOfSeq Mcasts\n"));
			printf("    %-10ld %-12ld %-6ld %-8ld %-8ld %-8ld\n",
			       rx_packets, rx_bytes, rx_errs, rx_frame, rx_fifo, rx_multi);
			printf(_("TX: Packets    Bytes        Errors DeadLoop NoRoute  NoBufs\n"));
			printf("    %-10ld %-12ld %-6ld %-8ld %-8ld %-6ld\n\n",
			       tx_packets, tx_bytes, tx_errs, tx_colls, tx_carrier, tx_drops);
		}
	}
	return 0;
}

static int do_show(int argc, char **argv)
{
	int err;
	struct ip_tunnel_parm p;

	if (parse_args(argc, argv, &p) < 0)
		return -1;

	switch (p.iph.protocol) {
	case IPPROTO_IPIP:	
		err = do_get_ioctl(p.name[0] ? p.name : "tunl0", &p);
		break;
	case IPPROTO_GRE:
		err = do_get_ioctl(p.name[0] ? p.name : "gre0", &p);
		break;
	case IPPROTO_IPV6:
		err = do_get_ioctl(p.name[0] ? p.name : "sit0", &p);
		break;
	default:
		do_tunnels_list(&p);
		return 0;
	}
	if (err)
		return -1;

	print_tunnel(&p);
	return 0;
}

int do_iptunnel(int argc, char **argv)
{
	if (argc > 0) {
		if (matches(*argv, "add") == 0)
			return do_add(SIOCADDTUNNEL, argc-1, argv+1);
		if (matches(*argv, "change") == 0)
			return do_add(SIOCCHGTUNNEL, argc-1, argv+1);
		if (matches(*argv, "del") == 0)
			return do_del(argc-1, argv+1);
		if (matches(*argv, "show") == 0 ||
		    matches(*argv, "lst") == 0 ||
		    matches(*argv, "list") == 0)
			return do_show(argc-1, argv+1);
	} else
		return do_show(0, NULL);

	usage();
}


int preferred_family = AF_UNSPEC;
int show_stats = 0;
int resolve_hosts = 0;

int main(int argc, char **argv)
{
	char *basename;

#if I18N
	setlocale (LC_ALL, "");
	bindtextdomain("net-tools", "/usr/share/locale");
	textdomain("net-tools");
#endif

	basename = strrchr(argv[0], '/');
	if (basename == NULL)
		basename = argv[0];
	else
		basename++;
	
	while (argc > 1) {
		if (argv[1][0] != '-')
			break;
		if (matches(argv[1], "-family") == 0) {
			argc--;
			argv++;
			if (argc <= 1)
				usage();
			if (strcmp(argv[1], "inet") == 0)
				preferred_family = AF_INET;
			else if (strcmp(argv[1], "inet6") == 0)
				preferred_family = AF_INET6;
			else
				usage();
		} else if (matches(argv[1], "-stats") == 0 ||
			   matches(argv[1], "-statistics") == 0) {
			++show_stats;
		} else if (matches(argv[1], "-resolve") == 0) {
			++resolve_hosts;
		} else if ((matches(argv[1], "-V") == 0) || (matches(argv[1], "--version") == 0)) {
			version();
		} else
			usage();
		argc--;	argv++;
	}

	return do_iptunnel(argc-1, argv+1);
}
