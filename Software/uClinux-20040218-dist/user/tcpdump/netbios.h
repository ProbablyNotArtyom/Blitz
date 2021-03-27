/*
 * NETBIOS protocol formats
 *
 * @(#) $Header: /cvs/sw/new-wave/user/tcpdump/netbios.h,v 1.1 2000/08/03 05:59:19 gerg Exp $
 */

struct p8022Hdr {
    u_char	dsap;
    u_char	ssap;
    u_char	flags;
};

#define	p8022Size	3		/* min 802.2 header size */

#define UI		0x03		/* 802.2 flags */

