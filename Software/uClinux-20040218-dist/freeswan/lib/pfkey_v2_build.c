/*
 * RFC2367 PF_KEYv2 Key management API message parser
 * Copyright (C) 1999, 2000, 2001  Richard Guy Briggs.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * RCSID $Id: pfkey_v2_build.c,v 1.31 2002/01/29 22:25:35 rgb Exp $
 */

/*
 *		Template from klips/net/ipsec/ipsec/ipsec_parser.c.
 */

char pfkey_v2_build_c_version[] = "$Id: pfkey_v2_build.c,v 1.31 2002/01/29 22:25:35 rgb Exp $";

/*
 * Some ugly stuff to allow consistent debugging code for use in the
 * kernel and in user space
*/

#ifdef __KERNEL__

# include <linux/kernel.h>  /* for printk */

# include "ipsec_kversion.h" /* for malloc switch */
# ifdef MALLOC_SLAB
#  include <linux/slab.h> /* kmalloc() */
# else /* MALLOC_SLAB */
#  include <linux/malloc.h> /* kmalloc() */
# endif /* MALLOC_SLAB */
# include <linux/errno.h>  /* error codes */
# include <linux/types.h>  /* size_t */
# include <linux/interrupt.h> /* mark_bh */

# include <linux/netdevice.h>   /* struct device, and other headers */
# include <linux/etherdevice.h> /* eth_type_trans */
# include <linux/ip.h>          /* struct iphdr */ 
# if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
#  include <linux/ipv6.h>        /* struct ipv6hdr */
# endif /* if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE) */

# define MALLOC(size) kmalloc(size, GFP_ATOMIC)
# define FREE(obj) kfree(obj)
# include <freeswan.h>
#else /* __KERNEL__ */

# include <sys/types.h>
# include <linux/types.h>
# include <linux/errno.h>
# include <malloc.h>
# include <string.h> /* memset */

# include <freeswan.h>
# include "../pluto/constants.h" 
# include "../pluto/defs.h"  /* for PRINTF_LIKE */
# include "../pluto/log.h"  /* for debugging and DBG_log */

extern unsigned int debugging;  /* bits selecting what to report */
unsigned int pfkey_lib_debug = 0;

/* #define PLUTO */

# ifdef PLUTO
#  define DEBUGGING(args...)  { DBG_log("pfkey_lib_debug:" args);  }
# else
#  define DEBUGGING(args...)  if(pfkey_lib_debug) { printf("pfkey_lib_debug:" args); } else { ; }
# endif
# define MALLOC(size) malloc(size)
# define FREE(obj) free(obj)
#endif /* __KERNEL__ */

#include <pfkeyv2.h>
#include <pfkey.h>

#ifdef __KERNEL__
# include "radij.h"  /* rd_nodes */
# include "ipsec_encap.h"  /* sockaddr_encap */
# include "ipsec_netlink.h"  /* KLIPS_PRINT */
# define DEBUGGING(args...) \
         KLIPS_PRINT(debug_pfkey, "klips_debug:" args)
/*         ((debug_pfkey) ? printk(KERN_INFO "klips_debug:" format , ## args) : 0) */
#endif /* __KERNEL__ */


#define SENDERR(_x) do { error = -(_x); goto errlab; } while (0)

void
pfkey_extensions_init(struct sadb_ext *extensions[SADB_EXT_MAX + 1])
{
	int i;
	
	for (i = 0; i != SADB_EXT_MAX + 1; i++) {
		extensions[i] = NULL;
	}
}

void
pfkey_extensions_free(struct sadb_ext *extensions[SADB_EXT_MAX + 1])
{
	int i;
	
	if(!extensions) {
		return;
	}

	if(extensions[0]) {
		memset(extensions[0], 0, sizeof(struct sadb_msg));
		FREE(extensions[0]);
		extensions[0] = NULL;
	}
	
	for (i = 1; i != SADB_EXT_MAX + 1; i++) {
		if(extensions[i]) {
			memset(extensions[i], 0, extensions[i]->sadb_ext_len * IPSEC_PFKEYv2_ALIGN);
			FREE(extensions[i]);
			extensions[i] = NULL;
		}
	}
}

void
pfkey_msg_free(struct sadb_msg **pfkey_msg)
{
	if(*pfkey_msg) {
		memset(*pfkey_msg, 0, (*pfkey_msg)->sadb_msg_len * IPSEC_PFKEYv2_ALIGN);
		FREE(*pfkey_msg);
		*pfkey_msg = NULL;
	}
}

/* Default extension builders taken from the KLIPS code */

int
pfkey_msg_hdr_build(struct sadb_ext**	pfkey_ext,
		    uint8_t		msg_type,
		    uint8_t		satype,
		    uint8_t		msg_errno,
		    uint32_t		seq,
		    uint32_t		pid)
{
	int error = 0;
	struct sadb_msg *pfkey_msg = (struct sadb_msg *)*pfkey_ext;

	DEBUGGING(
		"pfkey_msg_hdr_build:\n");
	DEBUGGING(
		"pfkey_msg_hdr_build: "
		"on_entry &pfkey_ext=%p pfkey_ext=%p *pfkey_ext=%p.\n",
		&pfkey_ext,
		pfkey_ext,
		*pfkey_ext);
	/* sanity checks... */
	if(pfkey_msg) {
		DEBUGGING(
			"pfkey_msg_hdr_build: "
			"why is pfkey_msg already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if(!msg_type) {
		DEBUGGING(
			"pfkey_msg_hdr_build: "
			"msg type not set, must be non-zero..\n");
		SENDERR(EINVAL);
	}

	if(msg_type > SADB_MAX) {
		DEBUGGING(
			"pfkey_msg_hdr_build: "
			"msg type too large:%d.\n",
			msg_type);
		SENDERR(EINVAL);
	}

	if(satype > SADB_SATYPE_MAX) {
		DEBUGGING(
			"pfkey_msg_hdr_build: "
			"satype %d > max %d\n", 
			satype, SADB_SATYPE_MAX);
		SENDERR(EINVAL);
	}

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_msg = (struct sadb_msg*)
	     MALLOC(sizeof(struct sadb_msg)))) {
		DEBUGGING(
			"pfkey_msg_hdr_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_msg, 0, sizeof(struct sadb_msg));

	pfkey_msg->sadb_msg_len = sizeof(struct sadb_msg) / IPSEC_PFKEYv2_ALIGN;

	pfkey_msg->sadb_msg_type = msg_type;
	pfkey_msg->sadb_msg_satype = satype;

	pfkey_msg->sadb_msg_version = PF_KEY_V2;
	pfkey_msg->sadb_msg_errno = msg_errno;
	pfkey_msg->sadb_msg_reserved = 0;
	pfkey_msg->sadb_msg_seq = seq;
	pfkey_msg->sadb_msg_pid = pid;
	DEBUGGING(
		"pfkey_msg_hdr_build: "
		"on_exit &pfkey_ext=%p pfkey_ext=%p *pfkey_ext=%p.\n",
		&pfkey_ext,
		pfkey_ext,
		*pfkey_ext);
errlab:
	return error;
}	

int
pfkey_sa_build(struct sadb_ext **	pfkey_ext,
	       uint16_t			exttype,
	       uint32_t			spi, /* in network order */
	       uint8_t			replay_window,
	       uint8_t			sa_state,
	       uint8_t			auth,
	       uint8_t			encrypt,
	       uint32_t			flags)
{
	int error = 0;
	struct sadb_sa *pfkey_sa = (struct sadb_sa *)*pfkey_ext;

	DEBUGGING(
		    "pfkey_sa_build: "
		    "spi=%08x replay=%d sa_state=%d auth=%d encrypt=%d flags=%d\n",
		    ntohl(spi), /* in network order */
		    replay_window,
		    sa_state,
		    auth,
		    encrypt,
		    flags);
	/* sanity checks... */
	if(pfkey_sa) {
		DEBUGGING(
			"pfkey_sa_build: "
			"why is pfkey_sa already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if(exttype != SADB_EXT_SA &&
	   exttype != SADB_X_EXT_SA2) {
		DEBUGGING(
			"pfkey_sa_build: "
			"invalid exttype=%d.\n",
			exttype);
		SENDERR(EINVAL);
	}

	if(replay_window > 64) {
		DEBUGGING(
			"pfkey_sa_build: "
			"replay window size: %d -- must be 0 <= size <= 64\n",
			replay_window);
		SENDERR(EINVAL);
	}

	if(auth > SADB_AALG_MAX) {
		DEBUGGING(
			"pfkey_sa_build: "
			"auth=%d > SADB_AALG_MAX=%d.\n",
			auth,
			SADB_AALG_MAX);
		SENDERR(EINVAL);
	}

	if(encrypt > SADB_EALG_MAX) {
		DEBUGGING(
			"pfkey_sa_build: "
			"encrypt=%d > SADB_EALG_MAX=%d.\n",
			encrypt,
			SADB_EALG_MAX);
		SENDERR(EINVAL);
	}

	if(sa_state > SADB_SASTATE_MAX) {
		DEBUGGING(
			"pfkey_sa_build: "
			"sa_state=%d exceeds MAX=%d.\n",
			sa_state,
			SADB_SASTATE_MAX);
		SENDERR(EINVAL);
	}

	if(sa_state == SADB_SASTATE_DEAD) {
		DEBUGGING(
			"pfkey_sa_build: "
			"sa_state=%d is DEAD=%d is not allowed.\n",
			sa_state,
			SADB_SASTATE_DEAD);
		SENDERR(EINVAL);
	}
	
	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_sa = (struct sadb_sa*)
	     MALLOC(sizeof(struct sadb_sa)))) {
		DEBUGGING(
			"pfkey_sa_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_sa, 0, sizeof(struct sadb_sa));
	
	pfkey_sa->sadb_sa_len = sizeof(*pfkey_sa) / IPSEC_PFKEYv2_ALIGN;
	pfkey_sa->sadb_sa_exttype = exttype;
	pfkey_sa->sadb_sa_spi = spi;
	pfkey_sa->sadb_sa_replay = replay_window;
	pfkey_sa->sadb_sa_state = sa_state;
	pfkey_sa->sadb_sa_auth = auth;
	pfkey_sa->sadb_sa_encrypt = encrypt;
	pfkey_sa->sadb_sa_flags = flags;

errlab:
	return error;
}	

int
pfkey_lifetime_build(struct sadb_ext **	pfkey_ext,
		     uint16_t		exttype,
		     uint32_t		allocations,
		     uint64_t		bytes,
		     uint64_t		addtime,
		     uint64_t		usetime,
		     uint32_t		packets)
{
	int error = 0;
	struct sadb_lifetime *pfkey_lifetime = (struct sadb_lifetime *)*pfkey_ext;

	DEBUGGING(
		"pfkey_lifetime_build:\n");
	/* sanity checks... */
	if(pfkey_lifetime) {
		DEBUGGING(
			"pfkey_lifetime_build: "
			"why is pfkey_lifetime already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if(exttype != SADB_EXT_LIFETIME_CURRENT &&
	   exttype != SADB_EXT_LIFETIME_HARD &&
	   exttype != SADB_EXT_LIFETIME_SOFT) {
		DEBUGGING(
			"pfkey_lifetime_build: "
			"invalid exttype=%d.\n",
			exttype);
		SENDERR(EINVAL);
	}

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_lifetime = (struct sadb_lifetime*)
	     MALLOC(sizeof(struct sadb_lifetime)))) {
		DEBUGGING(
			"pfkey_lifetime_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_lifetime, 0, sizeof(struct sadb_lifetime));

	pfkey_lifetime->sadb_lifetime_len = sizeof(struct sadb_lifetime) / IPSEC_PFKEYv2_ALIGN;
	pfkey_lifetime->sadb_lifetime_exttype = exttype;
	pfkey_lifetime->sadb_lifetime_allocations = allocations;
	pfkey_lifetime->sadb_lifetime_bytes = bytes;
	pfkey_lifetime->sadb_lifetime_addtime = addtime;
	pfkey_lifetime->sadb_lifetime_usetime = usetime;
	pfkey_lifetime->sadb_x_lifetime_packets = packets;

errlab:
	return error;
}

int
pfkey_address_build(struct sadb_ext**	pfkey_ext,
		    uint16_t		exttype,
		    uint8_t		proto,
		    uint8_t		prefixlen,
		    struct sockaddr*	address)
{
	int error = 0;
	int saddr_len = 0;
	char ipaddr_txt[ADDRTOT_BUF + 6/*extra for port number*/];
	struct sadb_address *pfkey_address = (struct sadb_address *)*pfkey_ext;
	
	DEBUGGING(
		"pfkey_address_build: "
		"exttype=%d proto=%d prefixlen=%d\n",
		exttype,
		proto,
		prefixlen);
	/* sanity checks... */
	if(pfkey_address) {
		DEBUGGING(
			"pfkey_address_build: "
			"why is pfkey_address already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if (!address)  {
			DEBUGGING("pfkey_address_build: "
				  "address is NULL\n");
			SENDERR(EINVAL);
	}
	
	switch(exttype) {	
	case SADB_EXT_ADDRESS_SRC:
	case SADB_EXT_ADDRESS_DST:
	case SADB_EXT_ADDRESS_PROXY:
	case SADB_X_EXT_ADDRESS_DST2:
	case SADB_X_EXT_ADDRESS_SRC_FLOW:
	case SADB_X_EXT_ADDRESS_DST_FLOW:
	case SADB_X_EXT_ADDRESS_SRC_MASK:
	case SADB_X_EXT_ADDRESS_DST_MASK:
#if defined(CONFIG_IPSEC_NAT_TRAVERSAL) || defined(NAT_TRAVERSAL)
	case SADB_X_EXT_NAT_T_OA:
#endif
		break;
	default:
		DEBUGGING( 
			"pfkey_address_build: "
			"unrecognised ext_type=%d.\n", 
			exttype); 
		SENDERR(EINVAL); 
	}

	switch(address->sa_family) {
	case AF_INET:
		DEBUGGING(
			"pfkey_address_build: "
			"found address family AF_INET.\n");
		saddr_len = sizeof(struct sockaddr_in);
		sprintf(ipaddr_txt, "%d.%d.%d.%d:%d"
			, (((struct sockaddr_in*)address)->sin_addr.s_addr >>  0) & 0xFF
			, (((struct sockaddr_in*)address)->sin_addr.s_addr >>  8) & 0xFF
			, (((struct sockaddr_in*)address)->sin_addr.s_addr >> 16) & 0xFF
			, (((struct sockaddr_in*)address)->sin_addr.s_addr >> 24) & 0xFF
			, ntohs(((struct sockaddr_in*)address)->sin_port));
		break;
	case AF_INET6:
		DEBUGGING(
			"pfkey_address_build: "
			"found address family AF_INET6.\n");
		saddr_len = sizeof(struct sockaddr_in6);
		sprintf(ipaddr_txt, "%x:%x:%x:%x:%x:%x:%x:%x-%x"
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[0])
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[1])
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[2])
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[3])
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[4])
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[5])
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[6])
			, ntohs(((struct sockaddr_in6*)address)->sin6_addr.s6_addr16[7])
			, ntohs(((struct sockaddr_in6*)address)->sin6_port));
		break;
	default:
		DEBUGGING(
			"pfkey_address_build: "
			"address->sa_family=%d not supported.\n",
			address->sa_family);
		SENDERR(EPFNOSUPPORT);
	}

	DEBUGGING(
		"pfkey_address_build: "
		"found address=%s.\n",
		ipaddr_txt);
	if(prefixlen != 0) {
		DEBUGGING(
			"pfkey_address_build: "
			"address prefixes not supported yet.\n");
		SENDERR(EAFNOSUPPORT); /* not supported yet */
	}

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_address = (struct sadb_address*)
	     MALLOC(ALIGN_N(sizeof(struct sadb_address) + saddr_len, IPSEC_PFKEYv2_ALIGN) ))) {
		DEBUGGING(
			"pfkey_lifetime_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_address,
	       0,
	       ALIGN_N(sizeof(struct sadb_address) + saddr_len,
		     IPSEC_PFKEYv2_ALIGN));
	       
	pfkey_address->sadb_address_len = DIVUP(sizeof(struct sadb_address) + saddr_len,
						IPSEC_PFKEYv2_ALIGN);
	
	pfkey_address->sadb_address_exttype = exttype;
	pfkey_address->sadb_address_proto = proto;
	pfkey_address->sadb_address_prefixlen = prefixlen;
	pfkey_address->sadb_address_reserved = 0;

	memcpy((char*)pfkey_address + sizeof(struct sadb_address),
	       address,
	       saddr_len);

#if 0
	for(i = 0; i < sizeof(struct sockaddr_in) - offsetof(struct sockaddr_in, sin_zero); i++) {
		pfkey_address_s_ska.sin_zero[i] = 0;
	}
#endif
	DEBUGGING(
		"pfkey_address_build: "
		"successful.\n");

 errlab:
	return error;
}

int
pfkey_key_build(struct sadb_ext**	pfkey_ext,
		uint16_t		exttype,
		uint16_t		key_bits,
		char*			key)
{
	int error = 0;
	struct sadb_key *pfkey_key = (struct sadb_key *)*pfkey_ext;

	DEBUGGING(
		"pfkey_key_build:\n");
	/* sanity checks... */
	if(pfkey_key) {
		DEBUGGING(
			"pfkey_key_build: "
			"why is pfkey_key already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if(!key_bits) {
		DEBUGGING(
			"pfkey_key_build: "
			"key_bits is zero, it must be non-zero.\n");
		SENDERR(EINVAL);
	}

	if( !((exttype == SADB_EXT_KEY_AUTH) || (exttype == SADB_EXT_KEY_ENCRYPT))) {
		DEBUGGING(
			"pfkey_key_build: "
			"unsupported extension type=%d.\n",
			exttype);
		SENDERR(EINVAL);
	}

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_key = (struct sadb_key*)
	     MALLOC(sizeof(struct sadb_key) +
				    DIVUP(key_bits, 64) * IPSEC_PFKEYv2_ALIGN))) {
		DEBUGGING(
			"pfkey_key_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_key,
	       0,
	       sizeof(struct sadb_key) +
	       DIVUP(key_bits, 64) * IPSEC_PFKEYv2_ALIGN);
	
	pfkey_key->sadb_key_len = DIVUP(sizeof(struct sadb_key) * IPSEC_PFKEYv2_ALIGN +	key_bits,
					64);
	pfkey_key->sadb_key_exttype = exttype;
	pfkey_key->sadb_key_bits = key_bits;
	pfkey_key->sadb_key_reserved = 0;
	memcpy((char*)pfkey_key + sizeof(struct sadb_key),
	       key,
	       DIVUP(key_bits, 8));

errlab:
	return error;
}

int
pfkey_ident_build(struct sadb_ext**	pfkey_ext,
		  uint16_t		exttype,
		  uint16_t		ident_type,
		  uint64_t		ident_id,
		  uint8_t               ident_len,
		  char*			ident_string)
{
	int error = 0;
	struct sadb_ident *pfkey_ident = (struct sadb_ident *)*pfkey_ext;
	int data_len = ident_len * IPSEC_PFKEYv2_ALIGN - sizeof(struct sadb_ident);

	DEBUGGING(
		"pfkey_ident_build:\n");
	/* sanity checks... */
	if(pfkey_ident) {
		DEBUGGING(
			"pfkey_ident_build: "
			"why is pfkey_ident already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if( ! ((exttype == SADB_EXT_IDENTITY_SRC) ||
	       (exttype == SADB_EXT_IDENTITY_DST))) {
		DEBUGGING(
			"pfkey_ident_build: "
			"unsupported extension type=%d.\n",
			exttype);
		SENDERR(EINVAL);
	}

	if((ident_type == SADB_IDENTTYPE_RESERVED)) {
		DEBUGGING(
			"pfkey_ident_build: "
			"ident_type must be non-zero.\n");
		SENDERR(EINVAL);
	}

	if(ident_type > SADB_IDENTTYPE_MAX) {
		DEBUGGING(
			"pfkey_ident_build: "
			"identtype=%d out of range.\n",
			ident_type);
		SENDERR(EINVAL);
	}

	if(((ident_type == SADB_IDENTTYPE_PREFIX) ||
	    (ident_type == SADB_IDENTTYPE_FQDN)) &&
	   !ident_string) {
		DEBUGGING(
			"pfkey_ident_build: "
			"string required to allocate size of extension.\n");
		SENDERR(EINVAL);
	}
	
#if 0
	if((ident_type == SADB_IDENTTYPE_USERFQDN) ) {
	}
#endif
	    
	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_ident = (struct sadb_ident*)
	     MALLOC(ident_len * IPSEC_PFKEYv2_ALIGN))) {
		DEBUGGING(
			"pfkey_ident_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_ident, 0, ident_len * IPSEC_PFKEYv2_ALIGN);
	
	pfkey_ident->sadb_ident_len = ident_len;
	pfkey_ident->sadb_ident_exttype = exttype;
	pfkey_ident->sadb_ident_type = ident_type;
	pfkey_ident->sadb_ident_reserved = 0;
	pfkey_ident->sadb_ident_id = ident_id;
	memcpy((char*)pfkey_ident + sizeof(struct sadb_ident),
	       ident_string,
	       data_len);

errlab:
	return error;
}

int
pfkey_sens_build(struct sadb_ext**	pfkey_ext,
		 uint32_t		dpd,
		 uint8_t		sens_level,
		 uint8_t		sens_len,
		 uint64_t*		sens_bitmap,
		 uint8_t		integ_level,
		 uint8_t		integ_len,
		 uint64_t*		integ_bitmap)
{
	int error = 0;
	struct sadb_sens *pfkey_sens = (struct sadb_sens *)*pfkey_ext;
	int i;
	uint64_t* bitmap;

	DEBUGGING(
		"pfkey_sens_build:\n");
	/* sanity checks... */
	if(pfkey_sens) {
		DEBUGGING(
			"pfkey_sens_build: "
			"why is pfkey_sens already pointing to something?\n");
		SENDERR(EINVAL);
	}

	DEBUGGING(
		"pfkey_sens_build: "
		"Sorry, I can't build exttype=%d yet.\n",
		(*pfkey_ext)->sadb_ext_type);
	SENDERR(EINVAL); /* don't process these yet */

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_sens = (struct sadb_sens*)
	     MALLOC(sizeof(struct sadb_sens) +
		    (sens_len + integ_len) * sizeof(uint64_t)))) {
		DEBUGGING(
			"pfkey_sens_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_sens,
	       0,
	       sizeof(struct sadb_sens) +
	       (sens_len + integ_len) * sizeof(uint64_t));
	
	pfkey_sens->sadb_sens_len = (sizeof(struct sadb_sens) +
		    (sens_len + integ_len) * sizeof(uint64_t)) / IPSEC_PFKEYv2_ALIGN;
	pfkey_sens->sadb_sens_exttype = SADB_EXT_SENSITIVITY;
	pfkey_sens->sadb_sens_dpd = dpd;
	pfkey_sens->sadb_sens_sens_level = sens_level;
	pfkey_sens->sadb_sens_sens_len = sens_len;
	pfkey_sens->sadb_sens_integ_level = integ_level;
	pfkey_sens->sadb_sens_integ_len = integ_len;
	pfkey_sens->sadb_sens_reserved = 0;

	bitmap = (uint64_t*)((char*)pfkey_ext + sizeof(struct sadb_sens));
	for(i = 0; i < sens_len; i++) {
		*bitmap = sens_bitmap[i];
		bitmap++;
	}
	for(i = 0; i < integ_len; i++) {
		*bitmap = integ_bitmap[i];
		bitmap++;
	}

errlab:
	return error;
}

int
pfkey_prop_build(struct sadb_ext**	pfkey_ext,
		 uint8_t		replay,
		 unsigned int		comb_num,
		 struct sadb_comb*	comb)
{
	int error = 0;
	int i;
	struct sadb_prop *pfkey_prop = (struct sadb_prop *)*pfkey_ext;
	struct sadb_comb *combp;

	DEBUGGING(
		"pfkey_prop_build:\n");
	/* sanity checks... */
	if(pfkey_prop) {
		DEBUGGING(
			"pfkey_prop_build: "
			"why is pfkey_prop already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_prop = (struct sadb_prop*)
	     MALLOC(sizeof(struct sadb_prop) +
		    comb_num * sizeof(struct sadb_comb)))) {
		DEBUGGING(
			"pfkey_prop_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_prop,
	       0,
	       sizeof(struct sadb_prop) +
		    comb_num * sizeof(struct sadb_comb));
	
	pfkey_prop->sadb_prop_len = (sizeof(struct sadb_prop) +
		    comb_num * sizeof(struct sadb_comb)) / IPSEC_PFKEYv2_ALIGN;

	pfkey_prop->sadb_prop_exttype = SADB_EXT_PROPOSAL;
	pfkey_prop->sadb_prop_replay = replay;

	for(i=0; i<3; i++) {
		pfkey_prop->sadb_prop_reserved[i] = 0;
	}

	combp = (struct sadb_comb*)((char*)*pfkey_ext + sizeof(struct sadb_prop));
	for(i = 0; i < comb_num; i++) {
		memcpy (combp, &(comb[i]), sizeof(struct sadb_comb));
		combp++;
	}

#if 0
  uint8_t sadb_comb_auth;
  uint8_t sadb_comb_encrypt;
  uint16_t sadb_comb_flags;
  uint16_t sadb_comb_auth_minbits;
  uint16_t sadb_comb_auth_maxbits;
  uint16_t sadb_comb_encrypt_minbits;
  uint16_t sadb_comb_encrypt_maxbits;
  uint32_t sadb_comb_reserved;
  uint32_t sadb_comb_soft_allocations;
  uint32_t sadb_comb_hard_allocations;
  uint64_t sadb_comb_soft_bytes;
  uint64_t sadb_comb_hard_bytes;
  uint64_t sadb_comb_soft_addtime;
  uint64_t sadb_comb_hard_addtime;
  uint64_t sadb_comb_soft_usetime;
  uint64_t sadb_comb_hard_usetime;
  uint32_t sadb_comb_soft_packets;
  uint32_t sadb_comb_hard_packets;
#endif
errlab:
	return error;
}

int
pfkey_supported_build(struct sadb_ext**	pfkey_ext,
		      uint16_t		exttype,
		      unsigned int	alg_num,
		      struct sadb_alg*	alg)
{
	int error = 0;
	unsigned int i;
	struct sadb_supported *pfkey_supported = (struct sadb_supported *)*pfkey_ext;
	struct sadb_alg *pfkey_alg;

	/* sanity checks... */
	if(pfkey_supported) {
		DEBUGGING(
			"pfkey_supported_build: "
			"why is pfkey_supported already pointing to something?\n");
		SENDERR(EINVAL);
	}

	if( !((exttype == SADB_EXT_SUPPORTED_AUTH) || (exttype == SADB_EXT_SUPPORTED_ENCRYPT))) {
		DEBUGGING(
			"pfkey_supported_build: "
			"unsupported extension type=%d.\n",
			exttype);
		SENDERR(EINVAL);
	}

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_supported = (struct sadb_supported*)
	     MALLOC(sizeof(struct sadb_supported) +
					       alg_num *
					       sizeof(struct sadb_alg)))) {
		DEBUGGING(
			"pfkey_supported_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_supported,
	       0,
	       sizeof(struct sadb_supported) +
					       alg_num *
					       sizeof(struct sadb_alg));
	
	pfkey_supported->sadb_supported_len = (sizeof(struct sadb_supported) +
					       alg_num *
					       sizeof(struct sadb_alg)) /
						IPSEC_PFKEYv2_ALIGN;
	pfkey_supported->sadb_supported_exttype = exttype;
	pfkey_supported->sadb_supported_reserved = 0;

	pfkey_alg = (struct sadb_alg*)((char*)pfkey_supported + sizeof(struct sadb_supported));
	for(i = 0; i < alg_num; i++) {
		memcpy (pfkey_alg, &(alg[i]), sizeof(struct sadb_alg));
		pfkey_alg->sadb_alg_reserved = 0;
		pfkey_alg++;
	}
	
#if 0
	DEBUGGING(
		"pfkey_supported_build: "
		"Sorry, I can't build exttype=%d yet.\n",
		(*pfkey_ext)->sadb_ext_type);
	SENDERR(EINVAL); /* don't process these yet */

  uint8_t sadb_alg_id;
  uint8_t sadb_alg_ivlen;
  uint16_t sadb_alg_minbits;
  uint16_t sadb_alg_maxbits;
  uint16_t sadb_alg_reserved;
#endif
errlab:
	return error;
}

int
pfkey_spirange_build(struct sadb_ext**	pfkey_ext,
		     uint16_t		exttype,
		     uint32_t		min, /* in network order */
		     uint32_t		max) /* in network order */
{
	int error = 0;
	struct sadb_spirange *pfkey_spirange = (struct sadb_spirange *)*pfkey_ext;
	
	/* sanity checks... */
	if(pfkey_spirange) {
		DEBUGGING(
			"pfkey_spirange_build: "
			"why is pfkey_spirange already pointing to something?\n");
		SENDERR(EINVAL);
	}
	
        if(ntohl(max) < ntohl(min)) {
		DEBUGGING(
			"pfkey_spirange_build: "
			"minspi=%08x must be < maxspi=%08x.\n",
			ntohl(min),
			ntohl(max));
                SENDERR(EINVAL);
        }
	
	if(ntohl(min) <= 255) {
		DEBUGGING(
			"pfkey_spirange_build: "
			"minspi=%08x must be > 255.\n",
			ntohl(min));
		SENDERR(EEXIST);
	}
	
	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_spirange = (struct sadb_spirange*)
	     MALLOC(sizeof(struct sadb_spirange)))) {
		DEBUGGING(
			"pfkey_spirange_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_spirange,
	       0,
	       sizeof(struct sadb_spirange));
	
        pfkey_spirange->sadb_spirange_len = sizeof(struct sadb_spirange) / IPSEC_PFKEYv2_ALIGN;

	pfkey_spirange->sadb_spirange_exttype = SADB_EXT_SPIRANGE;
	pfkey_spirange->sadb_spirange_min = min;
	pfkey_spirange->sadb_spirange_max = max;
	pfkey_spirange->sadb_spirange_reserved = 0;
 errlab:
	return error;
}

int
pfkey_x_kmprivate_build(struct sadb_ext**	pfkey_ext)
{
	int error = 0;
	struct sadb_x_kmprivate *pfkey_x_kmprivate = (struct sadb_x_kmprivate *)*pfkey_ext;

	/* sanity checks... */
	if(pfkey_x_kmprivate) {
		DEBUGGING(
			"pfkey_x_kmprivate_build: "
			"why is pfkey_x_kmprivate already pointing to something?\n");
		SENDERR(EINVAL);
	}
	
	pfkey_x_kmprivate->sadb_x_kmprivate_reserved = 0;

	DEBUGGING(
		"pfkey_x_kmprivate_build: "
		"Sorry, I can't build exttype=%d yet.\n",
		(*pfkey_ext)->sadb_ext_type);
	SENDERR(EINVAL); /* don't process these yet */

	if(!(*pfkey_ext = (struct sadb_ext*)
	     pfkey_x_kmprivate = (struct sadb_x_kmprivate*)
	     MALLOC(sizeof(struct sadb_x_kmprivate)))) {
		DEBUGGING(
			"pfkey_x_kmprivate_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_x_kmprivate,
	       0,
	       sizeof(struct sadb_x_kmprivate));
	
        pfkey_x_kmprivate->sadb_x_kmprivate_len =
		sizeof(struct sadb_x_kmprivate) / IPSEC_PFKEYv2_ALIGN;

        pfkey_x_kmprivate->sadb_x_kmprivate_exttype = SADB_X_EXT_KMPRIVATE;
        pfkey_x_kmprivate->sadb_x_kmprivate_reserved = 0;
errlab:
	return error;
}

int
pfkey_x_satype_build(struct sadb_ext**	pfkey_ext,
		     uint8_t		satype)
{
	int error = 0;
	int i;
	struct sadb_x_satype *pfkey_x_satype = (struct sadb_x_satype *)*pfkey_ext;

	DEBUGGING(
		"pfkey_x_satype_build:\n");
	/* sanity checks... */
	if(pfkey_x_satype) {
		DEBUGGING(
			"pfkey_x_satype_build: "
			"why is pfkey_x_satype already pointing to something?\n");
		SENDERR(EINVAL);
	}
	
	if(!satype) {
		DEBUGGING(
			"pfkey_x_satype_build: "
			"SA type not set, must be non-zero.\n");
		SENDERR(EINVAL);
	}

	if(satype > SADB_SATYPE_MAX) {
		DEBUGGING(
			"pfkey_x_satype_build: "
			"satype %d > max %d\n", 
			satype, SADB_SATYPE_MAX);
		SENDERR(EINVAL);
	}

	if(!(*pfkey_ext = (struct sadb_ext*)pfkey_x_satype = (struct sadb_x_satype*)
	     MALLOC(sizeof(struct sadb_x_satype)))) {
		DEBUGGING(
			"pfkey_x_satype_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	memset(pfkey_x_satype,
	       0,
	       sizeof(struct sadb_x_satype));
	
        pfkey_x_satype->sadb_x_satype_len = sizeof(struct sadb_x_satype) / IPSEC_PFKEYv2_ALIGN;

	pfkey_x_satype->sadb_x_satype_exttype = SADB_X_EXT_SATYPE2;
	pfkey_x_satype->sadb_x_satype_satype = satype;
	for(i=0; i<3; i++) {
		pfkey_x_satype->sadb_x_satype_reserved[i] = 0;
	}

errlab:
	return error;
}

int
pfkey_x_debug_build(struct sadb_ext**	pfkey_ext,
		    uint32_t            tunnel,
		    uint32_t		netlink,
		    uint32_t		xform,
		    uint32_t		eroute,
		    uint32_t		spi,
		    uint32_t		radij,
		    uint32_t		esp,
		    uint32_t		ah,
		    uint32_t		rcv,
		    uint32_t            pfkey,
		    uint32_t            ipcomp,
		    uint32_t            verbose)
{
	int error = 0;
	int i;
	struct sadb_x_debug *pfkey_x_debug = (struct sadb_x_debug *)*pfkey_ext;

	DEBUGGING(
		"pfkey_x_debug_build:\n");
	/* sanity checks... */
	if(pfkey_x_debug) {
		DEBUGGING(
			"pfkey_x_debug_build: "
			"why is pfkey_x_debug already pointing to something?\n");
		SENDERR(EINVAL);
	}
	
	DEBUGGING(
		"pfkey_x_debug_build: "
		"tunnel=%x netlink=%x xform=%x eroute=%x spi=%x radij=%x esp=%x ah=%x rcv=%x pfkey=%x ipcomp=%x verbose=%x?\n",
		tunnel, netlink, xform, eroute, spi, radij, esp, ah, rcv, pfkey, ipcomp, verbose);

	if(!(*pfkey_ext = (struct sadb_ext*)pfkey_x_debug = (struct sadb_x_debug*)
	     MALLOC(sizeof(struct sadb_x_debug)))) {
		DEBUGGING(
			"pfkey_x_debug_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
#if 0
	memset(pfkey_x_debug,
	       0,
	       sizeof(struct sadb_x_debug));
#endif
	
        pfkey_x_debug->sadb_x_debug_len = sizeof(struct sadb_x_debug) / IPSEC_PFKEYv2_ALIGN;
	pfkey_x_debug->sadb_x_debug_exttype = SADB_X_EXT_DEBUG;

	pfkey_x_debug->sadb_x_debug_tunnel = tunnel;
	pfkey_x_debug->sadb_x_debug_netlink = netlink;
	pfkey_x_debug->sadb_x_debug_xform = xform;
	pfkey_x_debug->sadb_x_debug_eroute = eroute;
	pfkey_x_debug->sadb_x_debug_spi = spi;
	pfkey_x_debug->sadb_x_debug_radij = radij;
	pfkey_x_debug->sadb_x_debug_esp = esp;
	pfkey_x_debug->sadb_x_debug_ah = ah;
	pfkey_x_debug->sadb_x_debug_rcv = rcv;
	pfkey_x_debug->sadb_x_debug_pfkey = pfkey;
	pfkey_x_debug->sadb_x_debug_ipcomp = ipcomp;
	pfkey_x_debug->sadb_x_debug_verbose = verbose;

	for(i=0; i<4; i++) {
		pfkey_x_debug->sadb_x_debug_reserved[i] = 0;
	}

errlab:
	return error;
}

#if defined(CONFIG_IPSEC_NAT_TRAVERSAL) || defined(NAT_TRAVERSAL)
int
pfkey_x_nat_t_type_build(struct sadb_ext**	pfkey_ext,
		    uint8_t         type)
{
	int error = 0;
	int i;
	struct sadb_x_nat_t_type *pfkey_x_nat_t_type = (struct sadb_x_nat_t_type *)*pfkey_ext;

	DEBUGGING(
		"pfkey_x_nat_t_type_build:\n");
	/* sanity checks... */
	if(pfkey_x_nat_t_type) {
		DEBUGGING(
			"pfkey_x_nat_t_type_build: "
			"why is pfkey_x_nat_t_type already pointing to something?\n");
		SENDERR(EINVAL);
	}
	
	DEBUGGING(
		"pfkey_x_nat_t_type_build: "
		"type=%d\n", type);

	if(!(*pfkey_ext = (struct sadb_ext*)pfkey_x_nat_t_type = (struct sadb_x_nat_t_type*)
	     MALLOC(sizeof(struct sadb_x_nat_t_type)))) {
		DEBUGGING(
			"pfkey_x_nat_t_type_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	
	pfkey_x_nat_t_type->sadb_x_nat_t_type_len = sizeof(struct sadb_x_nat_t_type) / IPSEC_PFKEYv2_ALIGN;
	pfkey_x_nat_t_type->sadb_x_nat_t_type_exttype = SADB_X_EXT_NAT_T_TYPE;
	pfkey_x_nat_t_type->sadb_x_nat_t_type_type = type;
	for(i=0; i<3; i++) {
		pfkey_x_nat_t_type->sadb_x_nat_t_type_reserved[i] = 0;
	}

errlab:
	return error;
}
int
pfkey_x_nat_t_port_build(struct sadb_ext**	pfkey_ext,
		    uint16_t         exttype,
		    uint16_t         port)
{
	int error = 0;
	struct sadb_x_nat_t_port *pfkey_x_nat_t_port = (struct sadb_x_nat_t_port *)*pfkey_ext;

	DEBUGGING(
		"pfkey_x_nat_t_port_build:\n");
	/* sanity checks... */
	if(pfkey_x_nat_t_port) {
		DEBUGGING(
			"pfkey_x_nat_t_port_build: "
			"why is pfkey_x_nat_t_port already pointing to something?\n");
		SENDERR(EINVAL);
	}
	
	switch(exttype) {	
	case SADB_X_EXT_NAT_T_SPORT:
	case SADB_X_EXT_NAT_T_DPORT:
		break;
	default:
		DEBUGGING( 
			"pfkey_nat_t_port_build: "
			"unrecognised ext_type=%d.\n", 
			exttype); 
		SENDERR(EINVAL); 
	}

	DEBUGGING(
		"pfkey_x_nat_t_port_build: "
		"ext=%d, port=%d\n", exttype, port);

	if(!(*pfkey_ext = (struct sadb_ext*)pfkey_x_nat_t_port = (struct sadb_x_nat_t_port*)
	     MALLOC(sizeof(struct sadb_x_nat_t_port)))) {
		DEBUGGING(
			"pfkey_x_nat_t_port_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}
	
	pfkey_x_nat_t_port->sadb_x_nat_t_port_len = sizeof(struct sadb_x_nat_t_port) / IPSEC_PFKEYv2_ALIGN;
	pfkey_x_nat_t_port->sadb_x_nat_t_port_exttype = exttype;
	pfkey_x_nat_t_port->sadb_x_nat_t_port_port = port;
	pfkey_x_nat_t_port->sadb_x_nat_t_port_reserved = 0;

errlab:
	return error;
}
#endif

#if I_DONT_THINK_THIS_WILL_BE_USEFUL
int (*ext_default_builders[SADB_EXT_MAX +1])(struct sadb_msg*, struct sadb_ext*)
 =
{
	NULL, /* pfkey_msg_build, */
	pfkey_sa_build,
	pfkey_lifetime_build,
	pfkey_lifetime_build,
	pfkey_lifetime_build,
	pfkey_address_build,
	pfkey_address_build,
	pfkey_address_build,
	pfkey_key_build,
	pfkey_key_build,
	pfkey_ident_build,
	pfkey_ident_build,
	pfkey_sens_build,
	pfkey_prop_build,
	pfkey_supported_build,
	pfkey_supported_build,
	pfkey_spirange_build,
	pfkey_x_kmprivate_build,
	pfkey_x_satype_build,
	pfkey_sa_build,
	pfkey_address_build,
	pfkey_address_build,
	pfkey_address_build,
	pfkey_address_build,
	pfkey_address_build,
	pfkey_x_ext_debug_build
};
#endif

int
pfkey_msg_build(struct sadb_msg **pfkey_msg, struct sadb_ext *extensions[], int dir)
{
	int error = 0;
	int ext;
	int total_size;
	struct sadb_ext *pfkey_ext;
	int extensions_seen = 0;
	struct sadb_ext *extensions_check[SADB_EXT_MAX + 1];
	
	if(!extensions[0]) {
		DEBUGGING(
			"pfkey_msg_build: "
			"extensions[0] must be specified (struct sadb_msg).\n");
		SENDERR(EINVAL);
	}

	total_size = sizeof(struct sadb_msg) / IPSEC_PFKEYv2_ALIGN;
	for(ext = 1; ext <= SADB_EXT_MAX; ext++) {
		if(extensions[ext]) {
			total_size += (extensions[ext])->sadb_ext_len;
		}
        }                

	if(!(*pfkey_msg = (struct sadb_msg*)MALLOC(total_size * IPSEC_PFKEYv2_ALIGN))) {
		DEBUGGING(
			"pfkey_msg_build: "
			"memory allocation failed\n");
		SENDERR(ENOMEM);
	}

	DEBUGGING(
		"pfkey_msg_build: "
		"pfkey_msg=%p allocated %ld bytes, &(extensions[0])=%p\n",
		*pfkey_msg,
		total_size * IPSEC_PFKEYv2_ALIGN,
		&(extensions[0]));
	memcpy(*pfkey_msg,
	       extensions[0],
	       sizeof(struct sadb_msg));
	(*pfkey_msg)->sadb_msg_len = total_size;
	(*pfkey_msg)->sadb_msg_reserved = 0;
	extensions_seen =  1 ;

	pfkey_ext = (struct sadb_ext*)(((char*)(*pfkey_msg)) + sizeof(struct sadb_msg));

	for(ext = 1; ext <= SADB_EXT_MAX; ext++) {
		/* copy from extension[ext] to buffer */
		if(extensions[ext]) {    
			/* Is this type of extension permitted for this type of message? */
			if(!(extensions_bitmaps[dir][EXT_BITS_PERM][(*pfkey_msg)->sadb_msg_type] &
			     1<<ext)) {
				DEBUGGING(
					"pfkey_msg_build: "
					"ext type %d not permitted, exts_perm=%08x, 1<<type=%08x\n", 
					ext, 
					extensions_bitmaps[dir][EXT_BITS_PERM][(*pfkey_msg)->sadb_msg_type],
					1<<ext);
				SENDERR(EINVAL);
			}
			DEBUGGING(
				"pfkey_msg_build: "
				"copying %ld bytes from extensions[%d]=%p to=%p\n",
				(extensions[ext])->sadb_ext_len * IPSEC_PFKEYv2_ALIGN,
				ext,
				(extensions[ext]),
				pfkey_ext);
			memcpy(pfkey_ext,
			       extensions[ext],
			       (extensions[ext])->sadb_ext_len * IPSEC_PFKEYv2_ALIGN);
			((char*)pfkey_ext) += (extensions[ext])->sadb_ext_len * IPSEC_PFKEYv2_ALIGN;
			/* Mark that we have seen this extension and remember the header location */
			extensions_seen |= ( 1 << ext );
		}
	}

	/* check required extensions */
	DEBUGGING(
		"pfkey_msg_build: "
		"extensions permitted=%08x, seen=%08x, required=%08x.\n",
		extensions_bitmaps[dir][EXT_BITS_PERM][(*pfkey_msg)->sadb_msg_type],
		extensions_seen,
		extensions_bitmaps[dir][EXT_BITS_REQ][(*pfkey_msg)->sadb_msg_type]);
	
	if((extensions_seen &
	    extensions_bitmaps[dir][EXT_BITS_REQ][(*pfkey_msg)->sadb_msg_type]) !=
	   extensions_bitmaps[dir][EXT_BITS_REQ][(*pfkey_msg)->sadb_msg_type]) {
		DEBUGGING(
			"pfkey_msg_build: "
			"required extensions missing:%08x.\n",
			extensions_bitmaps[dir][EXT_BITS_REQ][(*pfkey_msg)->sadb_msg_type] -
			(extensions_seen &
			 extensions_bitmaps[dir][EXT_BITS_REQ][(*pfkey_msg)->sadb_msg_type]) );
		SENDERR(EINVAL);
	}
	
	if((error = pfkey_msg_parse(*pfkey_msg, NULL, extensions_check, dir))) {
		DEBUGGING(
			"pfkey_msg_build: "
			"Trouble parsing newly built pfkey message, error=%d.\n",
			error);
		SENDERR(-error);
	}

errlab:

	return error;
}

/*
 * $Log: pfkey_v2_build.c,v $
 * Revision 1.31  2002/01/29 22:25:35  rgb
 * Re-add ipsec_kversion.h to keep MALLOC happy.
 *
 * Revision 1.30  2002/01/29 01:59:09  mcr
 * 	removal of kversions.h - sources that needed it now use ipsec_param.h.
 * 	updating of IPv6 structures to match latest in6.h version.
 * 	removed dead code from freeswan.h that also duplicated kversions.h
 * 	code.
 *
 * Revision 1.29  2001/12/19 21:06:09  rgb
 * Added port numbers to pfkey_address_build() debugging.
 *
 * Revision 1.28  2001/11/06 19:47:47  rgb
 * Added packet parameter to lifetime and comb structures.
 *
 * Revision 1.27  2001/10/18 04:45:24  rgb
 * 2.4.9 kernel deprecates linux/malloc.h in favour of linux/slab.h,
 * lib/freeswan.h version macros moved to lib/kversions.h.
 * Other compiler directive cleanups.
 *
 * Revision 1.26  2001/09/08 21:13:34  rgb
 * Added pfkey ident extension support for ISAKMPd. (NetCelo)
 *
 * Revision 1.25  2001/06/14 19:35:16  rgb
 * Update copyright date.
 *
 * Revision 1.24  2001/03/20 03:49:45  rgb
 * Ditch superfluous debug_pfkey declaration.
 * Move misplaced freeswan.h inclusion for kernel case.
 *
 * Revision 1.23  2001/03/16 07:41:50  rgb
 * Put freeswan.h include before pluto includes.
 *
 * Revision 1.22  2001/02/27 22:24:56  rgb
 * Re-formatting debug output (line-splitting, joining, 1arg/line).
 * Check for satoa() return codes.
 *
 * Revision 1.21  2000/11/17 18:10:30  rgb
 * Fixed bugs mostly relating to spirange, to treat all spi variables as
 * network byte order since this is the way PF_KEYv2 stored spis.
 *
 * Revision 1.20  2000/10/12 00:02:39  rgb
 * Removed 'format, ##' nonsense from debug macros for RH7.0.
 *
 * Revision 1.19  2000/10/10 20:10:20  rgb
 * Added support for debug_ipcomp and debug_verbose to klipsdebug.
 *
 * Revision 1.18  2000/09/12 18:59:54  rgb
 * Added Gerhard's IPv6 support to pfkey parts of libfreeswan.
 *
 * Revision 1.17  2000/09/12 03:27:00  rgb
 * Moved DEBUGGING definition to compile kernel with debug off.
 *
 * Revision 1.16  2000/09/08 19:22:12  rgb
 * Fixed pfkey_prop_build() parameter to be only single indirection.
 * Fixed struct alg copy.
 *
 * Revision 1.15  2000/08/20 21:40:01  rgb
 * Added an address parameter sanity check to pfkey_address_build().
 *
 * Revision 1.14  2000/08/15 17:29:23  rgb
 * Fixes from SZI to untested pfkey_prop_build().
 *
 * Revision 1.13  2000/06/02 22:54:14  rgb
 * Added Gerhard Gessler's struct sockaddr_storage mods for IPv6 support.
 *
 * Revision 1.12  2000/05/10 19:24:01  rgb
 * Fleshed out sensitivity, proposal and supported extensions.
 *
 * Revision 1.11  2000/03/16 14:07:23  rgb
 * Renamed ALIGN macro to avoid fighting with others in kernel.
 *
 * Revision 1.10  2000/01/24 21:14:35  rgb
 * Added disabled pluto pfkey lib debug flag.
 *
 * Revision 1.9  2000/01/21 06:27:32  rgb
 * Added address cases for eroute flows.
 * Removed unused code.
 * Dropped unused argument to pfkey_x_satype_build().
 * Indented compiler directives for readability.
 * Added klipsdebug switching capability.
 * Fixed SADB_EXT_MAX bug not permitting last extension access.
 *
 * Revision 1.8  1999/12/29 21:17:41  rgb
 * Changed pfkey_msg_build() I/F to include a struct sadb_msg**
 * parameter for cleaner manipulation of extensions[] and to guard
 * against potential memory leaks.
 * Changed the I/F to pfkey_msg_free() for the same reason.
 *
 * Revision 1.7  1999/12/09 23:12:20  rgb
 * Removed unused cruft.
 * Added argument to pfkey_sa_build() to do eroutes.
 * Fixed exttype check in as yet unused pfkey_lifetime_build().
 *
 * Revision 1.6  1999/12/07 19:54:29  rgb
 * Removed static pluto debug flag.
 * Added functions for pfkey message and extensions initialisation
 * and cleanup.
 *
 * Revision 1.5  1999/12/01 22:20:06  rgb
 * Changed pfkey_sa_build to accept an SPI in network byte order.
 * Added <string.h> to quiet userspace compiler.
 * Moved pfkey_lib_debug variable into the library.
 * Removed SATYPE check from pfkey_msg_hdr_build so FLUSH will work.
 * Added extension assembly debugging.
 * Isolated assignment with brackets to be sure of scope.
 *
 * Revision 1.4  1999/11/27 11:57:35  rgb
 * Added ipv6 headers.
 * Remove over-zealous algorithm sanity checkers from pfkey_sa_build.
 * Debugging error messages added.
 * Fixed missing auth and encrypt assignment bug.
 * Add argument to pfkey_msg_parse() for direction.
 * Move parse-after-build check inside pfkey_msg_build().
 * Consolidated the 4 1-d extension bitmap arrays into one 4-d array.
 * Add CVS log entry to bottom of file.
 *
 */
