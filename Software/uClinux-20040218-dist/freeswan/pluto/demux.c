/* demultiplex incoming IKE messages
 * Copyright (C) 1997 Angelos D. Keromytis.
 * Copyright (C) 1998-2002  D. Hugh Redelmeier.
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
 * RCSID $Id$
 */

/* Ordering Constraints on Payloads
 *
 * rfc2409: The Internet Key Exchange (IKE)
 *
 * 5 Exchanges:
 *   "The SA payload MUST precede all other payloads in a phase 1 exchange."
 *
 *   "Except where otherwise noted, there are no requirements for ISAKMP
 *    payloads in any message to be in any particular order."
 *
 * 5.3 Phase 1 Authenticated With a Revised Mode of Public Key Encryption:
 *
 *   "If the HASH payload is sent it MUST be the first payload of the
 *    second message exchange and MUST be followed by the encrypted
 *    nonce. If the HASH payload is not sent, the first payload of the
 *    second message exchange MUST be the encrypted nonce."
 *
 *   "Save the requirements on the location of the optional HASH payload
 *    and the mandatory nonce payload there are no further payload
 *    requirements. All payloads-- in whatever order-- following the
 *    encrypted nonce MUST be encrypted with Ke_i or Ke_r depending on the
 *    direction."
 *
 * 5.5 Phase 2 - Quick Mode
 *
 *   "In Quick Mode, a HASH payload MUST immediately follow the ISAKMP
 *    header and a SA payload MUST immediately follow the HASH."
 *   [NOTE: there may be more than one SA payload, so this is not
 *    totally reasonable.  Probably all SAs should be so constrained.]
 *
 *   "If ISAKMP is acting as a client negotiator on behalf of another
 *    party, the identities of the parties MUST be passed as IDci and
 *    then IDcr."
 *
 *   "With the exception of the HASH, SA, and the optional ID payloads,
 *    there are no payload ordering restrictions on Quick Mode."
 */

/* Unfolding of Identity -- a central mystery
 *
 * This concerns Phase 1 identities, those of the IKE hosts.
 * These are the only ones that are authenticated.  Phase 2
 * identities are for IPsec SAs.
 *
 * There are three case of interest:
 *
 * (1) We initiate, based on a whack command specifying a Connection.
 *     We know the identity of the peer from the Connection.
 *
 * (2) (to be implemented) we initiate based on a flow from our client
 *     to some IP address.
 *     We immediately know one of the peer's client IP addresses from
 *     the flow.  We must use this to figure out the peer's IP address
 *     and Id.  To be solved.
 *
 * (3) We respond to an IKE negotiation.
 *     We immediately know the peer's IP address.
 *     We get an ID Payload in Main I2.
 *
 *     Unfortunately, this is too late for a number of things:
 *     - the ISAKMP SA proposals have already been made (Main I1)
 *       AND one accepted (Main R1)
 *     - the SA includes a specification of the type of ID
 *       authentication so this is negotiated without being told the ID.
 *     - with Preshared Key authentication, Main I2 is encrypted
 *       using the key, so it cannot be decoded to reveal the ID
 *       without knowing (or guessing) which key to use.
 *
 *     There are three reasonable choices here for the responder:
 *     + assume that the initiator is making wise offers since it
 *       knows the IDs involved.  We can balk later (but not gracefully)
 *       when we find the actual initiator ID
 *     + attempt to infer identity by IP address.  Again, we can balk
 *       when the true identity is revealed.  Actually, it is enough
 *       to infer properties of the identity (eg. SA properties and
 *       PSK, if needed).
 *     + make all properties universal so discrimination based on
 *       identity isn't required.  For example, always accept the same
 *       kinds of encryption.  Accept Public Key Id authentication
 *       since the Initiator presumably has our public key and thinks
 *       we must have / can find his.  This approach is weakest
 *       for preshared key since the actual key must be known to
 *       decrypt the Initiator's ID Payload.
 *     These choices can be blended.  For example, a class of Identities
 *     can be inferred, sufficient to select a preshared key but not
 *     sufficient to infer a unique identity.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>	/* only used for belt-and-suspenders select call */
#include <sys/poll.h>	/* only used for forensic poll call */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>

#if defined(IP_RECVERR) && defined(MSG_ERRQUEUE)
#  include <asm/types.h>	/* for __u8, __u32 */
#  include <linux/errqueue.h>
#  include <sys/uio.h>	/* struct iovec */
#endif

#include <freeswan.h>

#include "constants.h"
#include "defs.h"
#include "cookie.h"
#include "id.h"
#include "x509.h"
#include "connections.h"	/* needs id.h */
#include "state.h"
#include "packet.h"
#include "md5.h"
#include "sha1.h"
#include "crypto.h" /* requires sha1.h and md5.h */
#include "ike_alg.h"
#include "log.h"
#include "demux.h"	/* needs packet.h */
#include "ipsec_doi.h"	/* needs demux.h and state.h */
#include "timer.h"
#include "whack.h"	/* requires connections.h */
#include "server.h"

#ifdef NAT_TRAVERSAL
#include "nat_traversal.h"
#endif
#include "vendor.h"

/* This file does basic header checking and demux of
 * incoming packets.
 */

/* forward declarations */
static bool read_packet(struct msg_digest *md);
static void process_packet(struct msg_digest **mdp);

/* Reply messages are built in this buffer.
 * Only one state transition function can be using it at a time
 * so suspended STFs must save and restore it.
 * It could be an auto variable of complete_state_transition except for the fact
 * that when a suspended STF resumes, its reply message buffer
 * must be at the same location -- there are pointers into it.
 */
u_int8_t reply_buffer[MAX_OUTPUT_UDP_SIZE];

/* state_microcode is a tuple of information parameterizing certain
 * centralized processing of a packet.  For example, it roughly
 * specifies what payloads are expected in this message.
 * The microcode is selected primarily based on the state.
 * In Phase 1, the payload structure often depends on the
 * authentication technique, so that too plays a part in selecting
 * the state_microcode to use.
 */

struct state_microcode {
    enum state_kind state, next_state;
    lset_t flags;
    lset_t req_payloads;	/* required payloads (allows just one) */
    lset_t opt_payloads;	/* optional payloads (any mumber) */
    /* if not ISAKMP_NEXT_NONE, process_packet will emit HDR with this as np */
    u_int8_t first_out_payload;
    enum event_type timeout_event;
    state_transition_fn *processor;
};

/* State Microcode Flags, in several groups */

/* Oakley Auth values: to which auth values does this entry apply?
 * Most entries will use SMF_ALL_AUTH because they apply to all.
 * Note: SMF_ALL_AUTH matches 0 for those circumstances when no auth
 * has been set.
 */
#define SMF_ALL_AUTH	LRANGE(0, OAKLEY_AUTH_ROOF-1)
#define SMF_PSK_AUTH	LELEM(OAKLEY_PRESHARED_KEY)
#define SMF_DS_AUTH	(LELEM(OAKLEY_DSS_SIG) | LELEM(OAKLEY_RSA_SIG))
#define SMF_PKE_AUTH	(LELEM(OAKLEY_RSA_ENC) | LELEM(OAKLEY_ELGAMAL_ENC))
#define SMF_RPKE_AUTH	(LELEM(OAKLEY_RSA_ENC_REV) | LELEM(OAKLEY_ELGAMAL_ENC_REV))

/* misc flags */

#define SMF_INITIATOR	LELEM(OAKLEY_AUTH_ROOF + 0)
#define SMF_FIRST_ENCRYPTED_INPUT	LELEM(OAKLEY_AUTH_ROOF + 1)
#define SMF_INPUT_ENCRYPTED	LELEM(OAKLEY_AUTH_ROOF + 2)
#define SMF_OUTPUT_ENCRYPTED	LELEM(OAKLEY_AUTH_ROOF + 3)
#define SMF_RETRANSMIT_ON_DUPLICATE	LELEM(OAKLEY_AUTH_ROOF + 4)

#define SMF_ENCRYPTED (SMF_INPUT_ENCRYPTED | SMF_OUTPUT_ENCRYPTED)

/* this state generates a reply message */
#define SMF_REPLY   LELEM(OAKLEY_AUTH_ROOF + 5)

/* this state completes P1, so any pending P2 negotiations should start */
#define SMF_RELEASE_PENDING_P2	LELEM(OAKLEY_AUTH_ROOF + 6)

/* end of flags */


static state_transition_fn	/* forward declaration */
    unexpected,
    informational;

/* state_microcode_table is a table of all state_microcode tuples.
 * It must be in order of state (the first element).
 * After initialization, ike_microcode_index[s] points to the
 * first entry in state_microcode_table for state s.
 * Remember that each state name in Main or Quick Mode describes
 * what has happened in the past, not what this message is.
 */

static const struct state_microcode
    *ike_microcode_index[STATE_IKE_ROOF - STATE_IKE_FLOOR];

static const struct state_microcode state_microcode_table[] = {
#define PT(n) ISAKMP_NEXT_##n
#define P(n) LELEM(PT(n))

    /***** Phase 1 Main Mode *****/

    /* No state for main_outI1: --> HDR, SA */

    /* STATE_MAIN_R0: I1 --> R1
     * HDR, SA --> HDR, SA
     */
    { STATE_MAIN_R0, STATE_MAIN_R1
    , SMF_ALL_AUTH | SMF_REPLY
    , P(SA), P(VID) | P(CR), PT(NONE)
    , EVENT_RETRANSMIT, main_inI1_outR1},

    /* STATE_MAIN_I1: R1 --> I2
     * HDR, SA --> auth dependent
     * SMF_PSK_AUTH, SMF_DS_AUTH: --> HDR, KE, Ni
     * SMF_PKE_AUTH:
     *	--> HDR, KE, [ HASH(1), ] <IDi1_b>PubKey_r, <Ni_b>PubKey_r
     * SMF_RPKE_AUTH:
     *	--> HDR, [ HASH(1), ] <Ni_b>Pubkey_r, <KE_b>Ke_i, <IDi1_b>Ke_i [,<<Cert-I_b>Ke_i]
     * Note: since we don't know auth at start, we cannot differentiate
     * microcode entries based on it.
     */
    { STATE_MAIN_I1, STATE_MAIN_I2
    , SMF_ALL_AUTH | SMF_INITIATOR | SMF_REPLY
    , P(SA), P(VID) | P(CR), PT(NONE) /* don't know yet */
    , EVENT_RETRANSMIT, main_inR1_outI2 },

    /* STATE_MAIN_R1: I2 --> R2
     * SMF_PSK_AUTH, SMF_DS_AUTH: HDR, KE, Ni --> HDR, KE, Nr
     * SMF_PKE_AUTH: HDR, KE, [ HASH(1), ] <IDi1_b>PubKey_r, <Ni_b>PubKey_r
     *	    --> HDR, KE, <IDr1_b>PubKey_i, <Nr_b>PubKey_i
     * SMF_RPKE_AUTH:
     *	    HDR, [ HASH(1), ] <Ni_b>Pubkey_r, <KE_b>Ke_i, <IDi1_b>Ke_i [,<<Cert-I_b>Ke_i]
     *	    --> HDR, <Nr_b>PubKey_i, <KE_b>Ke_r, <IDr1_b>Ke_r
     */
    { STATE_MAIN_R1, STATE_MAIN_R2
    , SMF_PSK_AUTH | SMF_DS_AUTH | SMF_REPLY
#ifdef NAT_TRAVERSAL
    , P(KE) | P(NONCE), P(VID) | P(CR) | P(NATD_RFC), PT(KE)
#else
    , P(KE) | P(NONCE), P(VID) | P(CR), PT(KE)
#endif
    , EVENT_RETRANSMIT, main_inI2_outR2 },

    { STATE_MAIN_R1, STATE_UNDEFINED
    , SMF_PKE_AUTH | SMF_REPLY
    , P(KE) | P(ID) | P(NONCE), P(VID) | P(HASH), PT(KE)
    , EVENT_RETRANSMIT, unexpected /* ??? not yet implemented */ },

    { STATE_MAIN_R1, STATE_UNDEFINED
    , SMF_RPKE_AUTH | SMF_REPLY
    , P(NONCE) | P(KE) | P(ID), P(VID) | P(HASH) | P(CERT), PT(NONCE)
    , EVENT_RETRANSMIT, unexpected /* ??? not yet implemented */ },

    /* for states from here on, output message must be encrypted */

    /* STATE_MAIN_I2: R2 --> I3
     * SMF_PSK_AUTH: HDR, KE, Nr --> HDR*, IDi1, HASH_I
     * SMF_DS_AUTH: HDR, KE, Nr --> HDR*, IDi1, [ CERT, ] SIG_I
     * SMF_PKE_AUTH: HDR, KE, <IDr1_b>PubKey_i, <Nr_b>PubKey_i
     *	    --> HDR*, HASH_I
     * SMF_RPKE_AUTH: HDR, <Nr_b>PubKey_i, <KE_b>Ke_r, <IDr1_b>Ke_r
     *	    --> HDR*, HASH_I
     */
    { STATE_MAIN_I2, STATE_MAIN_I3
    , SMF_PSK_AUTH | SMF_DS_AUTH | SMF_INITIATOR | SMF_OUTPUT_ENCRYPTED | SMF_REPLY
#ifdef NAT_TRAVERSAL
    , P(KE) | P(NONCE), P(VID) | P(CR) | P(NATD_RFC), PT(ID)
#else
    , P(KE) | P(NONCE), P(VID) | P(CR), PT(ID)
#endif
    , EVENT_RETRANSMIT, main_inR2_outI3 },

    { STATE_MAIN_I2, STATE_UNDEFINED
    , SMF_PKE_AUTH | SMF_INITIATOR | SMF_OUTPUT_ENCRYPTED | SMF_REPLY
    , P(KE) | P(ID) | P(NONCE), P(VID), PT(HASH)
    , EVENT_RETRANSMIT, unexpected /* ??? not yet implemented */ },

    { STATE_MAIN_I2, STATE_UNDEFINED
    , SMF_ALL_AUTH | SMF_INITIATOR | SMF_OUTPUT_ENCRYPTED | SMF_REPLY
    , P(NONCE) | P(KE) | P(ID), P(VID), PT(HASH)
    , EVENT_RETRANSMIT, unexpected /* ??? not yet implemented */ },

    /* for states from here on, input message must be encrypted */

    /* STATE_MAIN_R2: I3 --> R3
     * SMF_PSK_AUTH: HDR*, IDi1, HASH_I --> HDR*, IDr1, HASH_R
     * SMF_DS_AUTH: HDR*, IDi1, [ CERT, ] SIG_I --> HDR*, IDr1, [ CERT, ] SIG_R
     * SMF_PKE_AUTH, SMF_RPKE_AUTH: HDR*, HASH_I --> HDR*, HASH_R
     */
    { STATE_MAIN_R2, STATE_MAIN_R3
    , SMF_PSK_AUTH | SMF_FIRST_ENCRYPTED_INPUT | SMF_ENCRYPTED
      | SMF_REPLY | SMF_RELEASE_PENDING_P2
    , P(ID) | P(HASH), P(VID), PT(NONE)
    , EVENT_SA_REPLACE, main_inI3_outR3 },

    { STATE_MAIN_R2, STATE_MAIN_R3
    , SMF_DS_AUTH | SMF_FIRST_ENCRYPTED_INPUT | SMF_ENCRYPTED
      | SMF_REPLY | SMF_RELEASE_PENDING_P2
    , P(ID) | P(SIG), P(VID) | P(CERT) | P(CR), PT(NONE)
    , EVENT_SA_REPLACE, main_inI3_outR3 },

    { STATE_MAIN_R2, STATE_UNDEFINED
    , SMF_PKE_AUTH | SMF_RPKE_AUTH | SMF_FIRST_ENCRYPTED_INPUT | SMF_ENCRYPTED
      | SMF_REPLY | SMF_RELEASE_PENDING_P2
    , P(HASH), P(VID), PT(NONE)
    , EVENT_SA_REPLACE, unexpected /* ??? not yet implemented */ },

    /* STATE_MAIN_I3: R3 --> done
     * SMF_PSK_AUTH: HDR*, IDr1, HASH_R --> done
     * SMF_DS_AUTH: HDR*, IDr1, [ CERT, ] SIG_R --> done
     * SMF_PKE_AUTH, SMF_RPKE_AUTH: HDR*, HASH_R --> done
     * May initiate quick mode by calling quick_outI1
     */
    { STATE_MAIN_I3, STATE_MAIN_I4
    , SMF_PSK_AUTH | SMF_INITIATOR
      | SMF_FIRST_ENCRYPTED_INPUT | SMF_ENCRYPTED | SMF_RELEASE_PENDING_P2
    , P(ID) | P(HASH), P(VID), PT(NONE)
    , EVENT_SA_REPLACE, main_inR3 },

    { STATE_MAIN_I3, STATE_MAIN_I4
    , SMF_DS_AUTH | SMF_INITIATOR
      | SMF_FIRST_ENCRYPTED_INPUT | SMF_ENCRYPTED | SMF_RELEASE_PENDING_P2
    , P(ID) | P(SIG), P(VID) | P(CERT), PT(NONE)
    , EVENT_SA_REPLACE, main_inR3 },

    { STATE_MAIN_I3, STATE_UNDEFINED
    , SMF_PKE_AUTH | SMF_RPKE_AUTH | SMF_INITIATOR
      | SMF_FIRST_ENCRYPTED_INPUT | SMF_ENCRYPTED | SMF_RELEASE_PENDING_P2
    , P(HASH), P(VID), PT(NONE)
    , EVENT_SA_REPLACE, unexpected /* ??? not yet implemented */ },

    /* STATE_MAIN_R3: can only get here due to packet loss */
    { STATE_MAIN_R3, STATE_UNDEFINED
    , SMF_ALL_AUTH | SMF_ENCRYPTED | SMF_RETRANSMIT_ON_DUPLICATE
    , LEMPTY, LEMPTY
    , PT(NONE), EVENT_NULL, unexpected },

    /* STATE_MAIN_I4: can only get here due to packet loss */
    { STATE_MAIN_I4, STATE_UNDEFINED
    , SMF_ALL_AUTH | SMF_INITIATOR | SMF_ENCRYPTED
    , LEMPTY, LEMPTY
    , PT(NONE), EVENT_NULL, unexpected },


    /***** Phase 1 Aggressive Mode *****/

    /* No state for aggr_outI1: -->HDR, SA, KE, Ni, IDii */

    /* STATE_AGGR_R0: HDR, SA, KE, Ni, IDii -->
     * HDR, SA, KE, Nr, IDir, HASH_R
     */
    { STATE_AGGR_R0, STATE_AGGR_R1, SMF_PSK_AUTH | SMF_REPLY,
      P(SA) | P(KE) | P(NONCE) | P(ID), P(VID), PT(NONE),
      EVENT_RETRANSMIT, aggr_inI1_outR1 },

    /* STATE_AGGR_I1: HDR, SA, KE, Nr, IDir, HASH_R --> HDR*, HASH_I */
    { STATE_AGGR_I1, STATE_AGGR_I2, 
      SMF_PSK_AUTH | SMF_INITIATOR | SMF_OUTPUT_ENCRYPTED | SMF_REPLY | SMF_RELEASE_PENDING_P2,
#ifdef NAT_TRAVERSAL
      P(SA) | P(KE) | P(NONCE) | P(ID) | P(HASH), P(VID) | P(NATD_RFC), PT(NONE),
#else
      P(SA) | P(KE) | P(NONCE) | P(ID) | P(HASH), P(VID), PT(NONE),
#endif

      EVENT_SA_REPLACE, aggr_inR1_outI2 },

    /* STATE_AGGR_R1: HDR*, HASH_I --> done */
    { STATE_AGGR_R1, STATE_AGGR_R2, 
      SMF_PSK_AUTH | SMF_FIRST_ENCRYPTED_INPUT | SMF_ENCRYPTED | SMF_RELEASE_PENDING_P2,
#ifdef NAT_TRAVERSAL
      P(HASH), P(VID)| P(NATD_RFC), PT(NONE),
#else
      P(HASH), P(VID), PT(NONE),
#endif
      EVENT_SA_REPLACE, aggr_inI2 },

    /* STATE_AGGR_I2: can only get here due to packet loss */
    { STATE_AGGR_I2, STATE_UNDEFINED, 
      SMF_PSK_AUTH | SMF_INITIATOR | SMF_RETRANSMIT_ON_DUPLICATE | SMF_RELEASE_PENDING_P2,
      LEMPTY, LEMPTY, PT(NONE), EVENT_NULL, unexpected },

    /* STATE_AGGR_R2: can only get here due to packet loss */
    { STATE_AGGR_R2, STATE_UNDEFINED, SMF_PSK_AUTH,
      LEMPTY, LEMPTY, PT(NONE), EVENT_NULL, unexpected },



    /***** Phase 2 Quick Mode *****/

    /* No state for quick_outI1:
     * --> HDR*, HASH(1), SA, Nr [, KE ] [, IDci, IDcr ]
     */

    /* STATE_QUICK_R0:
     * HDR*, HASH(1), SA, Ni [, KE ] [, IDci, IDcr ] -->
     * HDR*, HASH(2), SA, Nr [, KE ] [, IDci, IDcr ]
     * Installs inbound IPsec SAs.
     * Because it may suspend for asynchronous DNS, first_out_payload
     * is set to NONE to suppress early emission of HDR*.
     * ??? it is legal to have multiple SAs, but we don't support it yet.
     */
    { STATE_QUICK_R0, STATE_QUICK_R1
    , SMF_ALL_AUTH | SMF_ENCRYPTED | SMF_REPLY
#ifdef NAT_TRAVERSAL
    , P(HASH) | P(SA) | P(NONCE), /* P(SA) | */ P(KE) | P(ID) | P(NATOA_RFC), PT(NONE)
#else
    , P(HASH) | P(SA) | P(NONCE), /* P(SA) | */ P(KE) | P(ID), PT(NONE)
#endif
    , EVENT_RETRANSMIT, quick_inI1_outR1 },

    /* STATE_QUICK_I1:
     * HDR*, HASH(2), SA, Nr [, KE ] [, IDci, IDcr ] -->
     * HDR*, HASH(3)
     * Installs inbound and outbound IPsec SAs, routing, etc.
     * ??? it is legal to have multiple SAs, but we don't support it yet.
     */
    { STATE_QUICK_I1, STATE_QUICK_I2
    , SMF_ALL_AUTH | SMF_INITIATOR | SMF_ENCRYPTED | SMF_REPLY
#ifdef NAT_TRAVERSAL
    , P(HASH) | P(SA) | P(NONCE), /* P(SA) | */ P(KE) | P(ID) | P(NATOA_RFC), PT(HASH)
#else
    , P(HASH) | P(SA) | P(NONCE), /* P(SA) | */ P(KE) | P(ID), PT(HASH)
#endif
    , EVENT_SA_REPLACE, quick_inR1_outI2 },

    /* STATE_QUICK_R1: HDR*, HASH(3) --> done
     * Installs outbound IPsec SAs, routing, etc.
     */
    { STATE_QUICK_R1, STATE_QUICK_R2
    , SMF_ALL_AUTH | SMF_ENCRYPTED
    , P(HASH), LEMPTY, PT(NONE)
    , EVENT_SA_REPLACE, quick_inI2 },

    /* STATE_QUICK_I2: can only happen due to lost packet */
    { STATE_QUICK_I2, STATE_UNDEFINED
    , SMF_ALL_AUTH | SMF_INITIATOR | SMF_ENCRYPTED | SMF_RETRANSMIT_ON_DUPLICATE
    , LEMPTY, LEMPTY, PT(NONE)
    , EVENT_NULL, unexpected },

    /* STATE_QUICK_R2: can only happen due to lost packet */
    { STATE_QUICK_R2, STATE_UNDEFINED
    , SMF_ALL_AUTH | SMF_ENCRYPTED
    , LEMPTY, LEMPTY, PT(NONE)
    , EVENT_NULL, unexpected },


    /***** informational messages *****/

    /* STATE_INFO: */
    { STATE_INFO, STATE_UNDEFINED
    , SMF_ALL_AUTH
    , LEMPTY, LEMPTY, PT(NONE)
    , EVENT_NULL, informational },

    /* STATE_INFO_PROTECTED: */
    { STATE_INFO_PROTECTED, STATE_UNDEFINED
    , SMF_ALL_AUTH | SMF_ENCRYPTED
    , P(HASH), LEMPTY, PT(NONE)
    , EVENT_NULL, informational },

#undef P
#undef PT
};

void
init_demux(void)
{
    /* fill ike_microcode_index:
     * make ike_microcode_index[s] point to first entry in
     * state_microcode_table for state s (backward scan makes this easier).
     * Check that table is in order -- catch coding errors.
     * For what it's worth, this routine is idempotent.
     */
    const struct state_microcode *t;

    for (t = &state_microcode_table[elemsof(state_microcode_table) - 1];;)
    {
	passert(STATE_IKE_FLOOR <= t->state && t->state < STATE_IKE_ROOF);
	ike_microcode_index[t->state - STATE_IKE_FLOOR] = t;
	if (t == state_microcode_table)
	    break;
	t--;
	passert(t[0].state <= t[1].state);
    }
}

/* Process any message on the MSG_ERRQUEUE
 *
 * This information is generated because of the IP_RECVERR socket option.
 * The API is sparsely documented, and may be LINUX-only, and only on
 * fairly recent versions at that (hence the conditional compilation).
 *
 * - ip(7) describes IP_RECVERR
 * - recvmsg(2) describes MSG_ERRQUEUE
 * - readv(2) describes iovec
 * - cmsg(3) describes how to process auxilliary messages
 *
 * ??? we should link this message with one we've sent
 * so that the diagnostic can refer to that negotiation.
 *
 * ??? how long can the messge be?
 *
 * ??? poll(2) has a very incomplete description of the POLL* events.
 * We assume that POLLIN, POLLOUT, and POLLERR are all we need to deal with
 * and that POLLERR will be on iff there is a MSG_ERRQUEUE message.
 *
 * We have to code around a couple of surprises:
 *
 * - Select can say that a socket is ready to read from, and
 *   yet a read will hang.  It turns out that a message available on the
 *   MSG_ERRQUEUE will cause select to say something is pending, but
 *   a normal read will hang.  poll(2) can tell when a MSG_ERRQUEUE
 *   message is pending.
 *
 *   This is dealt with by calling check_msg_errqueue after select
 *   has indicated that there is something to read, but before the
 *   read is performed.  check_msg_errqueue will return TRUE if there
 *   is something left to read.
 *
 * - A write to a socket may fail because there is a pending MSG_ERRQUEUE
 *   message, without there being anything wrong with the write.  This
 *   makes for confusing diagnostics.
 *
 *   To avoid this, we call check_msg_errqueue before a write.  True,
 *   there is a race condition (a MSG_ERRQUEUE message might arrive
 *   between the check and the write), but we should eliminate many
 *   of the problematic events.  To narrow the window, the poll(2)
 *   will await until an event happens (in the case or a write,
 *   POLLOUT; this should be benign for POLLIN).
 */

#if defined(IP_RECVERR) && defined(MSG_ERRQUEUE)
static bool
check_msg_errqueue(const struct iface *ifp, short interest)
{
    struct pollfd pfd;

    pfd.fd = ifp->fd;
    pfd.events = interest | POLLPRI | POLLOUT;

    while (pfd.revents = 0
    , poll(&pfd, 1, -1) > 0 && (pfd.revents & POLLERR))
    {
	u_int8_t buffer[3000];	/* hope that this is big enough */
	union
	{
	    struct sockaddr sa;
	    struct sockaddr_in sa_in4;
	    struct sockaddr_in6 sa_in6;
	} from;

	int from_len = sizeof(from);

	int packet_len;

	struct msghdr emh;
	struct iovec eiov;
	union {
	    /* force alignment (not documented as necessary) */
	    struct cmsghdr ecms;

	    /* how much space is enough? */
	    unsigned char space[256];
	} ecms_buf;

	struct cmsghdr *cm;
	char fromstr[sizeof(" for message to  port 65536") + INET6_ADDRSTRLEN];
	struct state *sender = NULL;

	zero(&from.sa);
	from_len = sizeof(from);

	emh.msg_name = &from.sa;	/* ??? filled in? */
	emh.msg_namelen = sizeof(from);
	emh.msg_iov = &eiov;
	emh.msg_iovlen = 1;
	emh.msg_control = &ecms_buf;
	emh.msg_controllen = sizeof(ecms_buf);
	emh.msg_flags = 0;

	eiov.iov_base = buffer;	/* see readv(2) */
	eiov.iov_len = sizeof(buffer);

	packet_len = recvmsg(ifp->fd, &emh, MSG_ERRQUEUE);

	if (packet_len == -1)
	{
	    log_errno((e, "recvmsg(,, MSG_ERRQUEUE) on %s failed in comm_handle"
		, ifp->rname));
	    break;
	}
	else if (packet_len == sizeof(buffer))
	{
	    log("MSG_ERRQUEUE message longer than %lu bytes; truncated"
		, (unsigned long) sizeof(buffer));
	}
	else
	{
	    sender = find_sender((size_t) packet_len, buffer);
	}

	DBG_cond_dump(DBG_ALL, "rejected packet:\n", buffer, packet_len);
	DBG_cond_dump(DBG_ALL, "control:\n", emh.msg_control, emh.msg_controllen);
	/* ??? Andi Kleen <ak@suse.de> and misc documentation
	 * suggests that name will have the original destination
	 * of the packet.  We seem to see msg_namelen == 0.
	 * Andi says that this is a kernel bug and has fixed it.
	 * Perhaps in 2.2.18/2.4.0.
	 */
	passert(emh.msg_name == &from.sa);
	DBG_cond_dump(DBG_ALL, "name:\n", emh.msg_name
	    , emh.msg_namelen);

	fromstr[0] = '\0';	/* usual case :-( */
	switch (from.sa.sa_family)
	{
	char as[INET6_ADDRSTRLEN];

	case AF_INET:
	    if (emh.msg_namelen == sizeof(struct sockaddr_in))
		snprintf(fromstr, sizeof(fromstr)
		, " for message to %s port %u"
		    , inet_ntop(from.sa.sa_family
		    , &from.sa_in4.sin_addr, as, sizeof(as))
		    , ntohs(from.sa_in4.sin_port));
	    break;
	case AF_INET6:
	    if (emh.msg_namelen == sizeof(struct sockaddr_in6))
		snprintf(fromstr, sizeof(fromstr)
		    , " for message to %s port %u"
		    , inet_ntop(from.sa.sa_family
		    , &from.sa_in6.sin6_addr, as, sizeof(as))
		    , ntohs(from.sa_in6.sin6_port));
	    break;
	}

	for (cm = CMSG_FIRSTHDR(&emh)
	; cm != NULL
	; cm = CMSG_NXTHDR(&emh,cm))
	{
	    if (cm->cmsg_level == SOL_IP
	    && cm->cmsg_type == IP_RECVERR)
	    {
		/* ip(7) and recvmsg(2) specify:
		 * ee_origin is SO_EE_ORIGIN_ICMP for ICMP
		 *  or SO_EE_ORIGIN_LOCAL for locally generated errors.
		 * ee_type and ee_code are from the ICMP header.
		 * ee_info is the discovered MTU for EMSGSIZE errors
		 * ee_data is not used.
		 *
		 * ??? recvmsg(2) says "SOCK_EE_OFFENDER" but
		 * means "SO_EE_OFFENDER".  The OFFENDER is really
		 * the router that complained.  As such, the port
		 * is meaningless.
		 */

		/* ??? cmsg(3) claims that CMSG_DATA returns
		 * void *, but RFC 2292 and /usr/include/bits/socket.h
		 * say unsigned char *.  The manual is being fixed.
		 */
		struct sock_extended_err *ee = (void *)CMSG_DATA(cm);
		const char *offstr = "unspecified";
		char offstrspace[INET6_ADDRSTRLEN];
		char orname[50];

		if (cm->cmsg_len > CMSG_LEN(sizeof(struct sock_extended_err)))
		{
		    const struct sockaddr *offender = SO_EE_OFFENDER(ee);

		    switch (offender->sa_family)
		    {
		    case AF_INET:
			offstr = inet_ntop(offender->sa_family
			    , &((const struct sockaddr_in *)offender)->sin_addr
			    , offstrspace, sizeof(offstrspace));
			break;
		    case AF_INET6:
			offstr = inet_ntop(offender->sa_family
			    , &((const struct sockaddr_in6 *)offender)->sin6_addr
			    , offstrspace, sizeof(offstrspace));
			break;
		    default:
			offstr = "unknown";
			break;
		    }
		}

		switch (ee->ee_origin)
		{
		case SO_EE_ORIGIN_NONE:
		    snprintf(orname, sizeof(orname), "none");
		    break;
		case SO_EE_ORIGIN_LOCAL:
		    snprintf(orname, sizeof(orname), "local");
		    break;
		case SO_EE_ORIGIN_ICMP:
		    snprintf(orname, sizeof(orname)
			, "ICMP type %d code %d (not authenticated)"
			, ee->ee_type, ee->ee_code
			);
		    break;
		case SO_EE_ORIGIN_ICMP6:
		    snprintf(orname, sizeof(orname)
			, "ICMP6 type %d code %d (not authenticated)"
			, ee->ee_type, ee->ee_code
			);
		    break;
		default:
		    snprintf(orname, sizeof(orname), "invalid origin %lu"
			, (unsigned long) ee->ee_origin);
		    break;
		}

		{
		    struct state *old_state = cur_state;

		    cur_state = sender;

		    /* note dirty trick to suppress ~ at start of format
		     * if we know what state to blame.
		     */
#ifdef NAT_TRAVERSAL
		    if ((packet_len == 1) && (buffer[0] = 0xff)
#ifdef DEBUG
			&& ((cur_debugging & DBG_NATT) == 0)
#endif
			) {
			/* don't log NAT-T keepalive related errors unless NATT debug is
			 * enabled
			 */
		    }
		    else
#endif
		    log((sender != NULL) + "~"
			"ERROR: asynchronous network error report on %s"
			"%s"
			", complainant %s"
			": %s"
			" [errno %lu, origin %s"
			/* ", pad %d, info %ld" */
			/* ", data %ld" */
			"]"
			, ifp->rname
			, fromstr
			, offstr
			, strerror(ee->ee_errno)
			, (unsigned long) ee->ee_errno
			, orname
			/* , ee->ee_pad, (unsigned long)ee->ee_info */
			/* , (unsigned long)ee->ee_data */
			);
		    if(!strcmp(strerror(ee->ee_errno), "Connection refused "))
		    	log("The remote host may not have IPSec enabled");
		    if(!strcmp(strerror(ee->ee_errno), "No route to host "))
		    	log("The remote host may not be online");
		    cur_state = old_state;
		}
	    }
	    else
	    {
		log("unknown cmsg: level %d, type %d, len %d"
		    , cm->cmsg_level, cm->cmsg_type, cm->cmsg_len);
	    }
	}
    }
    return (pfd.revents & interest) != 0;
}
#endif /* defined(IP_RECVERR) && defined(MSG_ERRQUEUE) */

bool
#ifdef NAT_TRAVERSAL
_send_packet(struct state *st, const char *where, bool verbose)
#else
send_packet(struct state *st, const char *where)
#endif
{
    struct connection *c = st->st_connection;
#ifdef NAT_TRAVERSAL
    u_int8_t ike_pkt[MAX_OUTPUT_UDP_SIZE];
    u_int8_t *ptr;
    unsigned long len;

    if ((c->interface->ike_float == TRUE) && (st->st_tpacket.len != 1)) {
	if ((unsigned long) st->st_tpacket.len >
	    (MAX_OUTPUT_UDP_SIZE-sizeof(u_int32_t))) {
	    DBG_log("send_packet(): really too big");
	    return FALSE;
	}
	ptr = ike_pkt;
	/** Add Non-ESP marker **/
	memset(ike_pkt, 0, sizeof(u_int32_t));
	memcpy(ike_pkt + sizeof(u_int32_t), st->st_tpacket.ptr,
	    (unsigned long)st->st_tpacket.len);
	len = (unsigned long) st->st_tpacket.len + sizeof(u_int32_t);
    }
    else {
	ptr = st->st_tpacket.ptr;
	len = (unsigned long) st->st_tpacket.len;
    }
#endif

    DBG(DBG_RAW,
	{
	    DBG_log("sending %lu bytes for %s through %s to %s:%u:"
		, (unsigned long) st->st_tpacket.len
		, where
		, c->interface->rname
		, ip_str(&c->that.host_addr)
		, (unsigned)c->that.host_port);
	    DBG_dump_chunk(NULL, st->st_tpacket);
	});

    /* XXX: Not very clean.  We manipulate the port of the ip_address to
     * have a port in the sockaddr*
     */

    setportof(htons(c->that.host_port), &c->that.host_addr);

#if defined(IP_RECVERR) && defined(MSG_ERRQUEUE)
    (void) check_msg_errqueue(c->interface, POLLOUT);
#endif /* defined(IP_RECVERR) && defined(MSG_ERRQUEUE) */

#ifdef NAT_TRAVERSAL
    if (sendto(c->interface->fd
    , ptr, len, 0
    , sockaddrof(&c->that.host_addr)
    , sockaddrlenof(&c->that.host_addr)) != (ssize_t)len)
#else
    if (sendto(c->interface->fd
    , st->st_tpacket.ptr, st->st_tpacket.len, 0
    , sockaddrof(&c->that.host_addr)
    , sockaddrlenof(&c->that.host_addr)) != (ssize_t)st->st_tpacket.len)
#endif
    {
#ifdef NAT_TRAVERSAL
	/* do not log NAT-T Keep Alive packets */
	if (!verbose)
	    return FALSE;
#endif
	log_errno((e, "sendto on %s to %s:%u failed in %s"
	    , c->interface->rname
	    , ip_str(&c->that.host_addr)
	    , (unsigned)c->that.host_port
	    , where));
	return FALSE;
    }
    else
    {
	return TRUE;
    }
}

static stf_status
unexpected(struct msg_digest *md)
{
    loglog(RC_LOG_SERIOUS, "unexpected message received in state %s"
	, enum_name(&state_names, md->st->st_state));
    return STF_IGNORE;
}

static stf_status
informational(struct msg_digest *md)
{
    struct payload_digest *const n_pld = md->chain[ISAKMP_NEXT_N];

    /* log contents of any notification payload */
    if (n_pld != NULL)
    {
	pb_stream *const n_pbs = &n_pld->pbs;
	struct isakmp_notification *const n = &n_pld->payload.notification;
	int disp_len;
	char disp_buf[200];

	switch (n->isan_type)
	{
	case R_U_THERE:
	    return dpd_inI_outR(md->st, n, n_pbs);

	case R_U_THERE_ACK:
	    return dpd_inR(md->st, n, n_pbs);

	default:
	    if (pbs_left(n_pbs) >= sizeof(disp_buf)-1)
		disp_len = sizeof(disp_buf)-1;
	    else
		disp_len = pbs_left(n_pbs);
	    memcpy(disp_buf, n_pbs->cur, disp_len);
	    disp_buf[disp_len] = '\0';

	    /* should indicate from where... FIXME */
	    log("Notification: Pid=%d SPIsz=%d Type=%d Val=%s\n" 
		, n->isan_protoid, n->isan_spisize, n->isan_type
		, disp_buf);
	    break;
	}
    }

    loglog(RC_LOG_SERIOUS, "received and ignored informational message");
    return STF_IGNORE;
}

/* message digest allocation and deallocation */

static struct msg_digest *md_pool = NULL;

/* free_md_pool is only used to avoid leak reports */
void
free_md_pool(void)
{
    for (;;)
    {
	struct msg_digest *md = md_pool;

	if (md == NULL)
	    break;
	md_pool = md->next;
	pfree(md);
    }
}

static struct msg_digest *
alloc_md(void)
{
    struct msg_digest *md = md_pool;

    /* convenient initializer:
     * - all pointers NULL
     * - .note = NOTHING_WRONG
     * - .encrypted = FALSE
     */
    static const struct msg_digest blank_md;

    if (md == NULL)
	md = alloc_thing(struct msg_digest, "msg_digest");
    else
	md_pool = md->next;

    *md = blank_md;
    md->digest_roof = md->digest;

    /* note: although there may be multiple msg_digests at once
     * (due to suspended state transitions), there is a single
     * global reply_buffer.  It will need to be saved and restored.
     */
    init_pbs(&md->reply, reply_buffer, sizeof(reply_buffer), "reply packet");

    return md;
}

void
release_md(struct msg_digest *md)
{
    freeanychunk(md->raw_packet);
    pfreeany(md->packet_pbs.start);
    md->packet_pbs.start = NULL;
    md->next = md_pool;
    md_pool = md;
}

/* wrapper for read_packet and process_packet
 *
 * The main purpose of this wrapper is to factor out teardown code
 * from the many return points in process_packet.  This amounts to
 * releasing the msg_digest and resetting global variables.
 *
 * When processing of a packet is suspended (STF_SUSPEND),
 * process_packet sets md to NULL to prevent the msg_digest being freed.
 * Someone else must ensure that msg_digest is freed eventually.
 *
 * read_packet is broken out to minimize the lifetime of the
 * enormous input packet buffer, an auto.
 */
void
comm_handle(const struct iface *ifp)
{
    static struct msg_digest *md;

#if defined(IP_RECVERR) && defined(MSG_ERRQUEUE)
    /* Even though select(2) says that there is a message,
     * it might only be a MSG_ERRQUEUE message.  At least
     * sometimes that leads to a hanging recvfrom.  To avoid
     * what appears to be a kernel bug, check_msg_errqueue
     * uses poll(2) and tells us if there is anything for us
     * to read.
     *
     * This is early enough that teardown isn't required:
     * just return on failure.
     */
    if (!check_msg_errqueue(ifp, POLLIN))
	return;	/* no normal message to read */
#endif /* defined(IP_RECVERR) && defined(MSG_ERRQUEUE) */

    md = alloc_md();
    md->iface = ifp;

    if (read_packet(md))
	process_packet(&md);

    if (md != NULL)
	release_md(md);

    cur_state = NULL;
    reset_cur_connection();
    cur_from = NULL;
}

/* read the message.
 * Since we don't know its size, we read it into
 * an overly large buffer and then copy it to a
 * new, properly sized buffer.
 */
static bool
read_packet(struct msg_digest *md)
{
    const struct iface *ifp = md->iface;
    int packet_len;
    u_int8_t bigbuffer[MAX_INPUT_UDP_SIZE];
#ifdef NAT_TRAVERSAL
    u_int8_t *_buffer = bigbuffer;
#endif
    union
    {
	struct sockaddr sa;
	struct sockaddr_in sa_in4;
	struct sockaddr_in6 sa_in6;
    } from;
    int from_len = sizeof(from);
    err_t from_ugh = NULL;
    static const char undisclosed[] = "unknown source";

    happy(anyaddr(addrtypeof(&ifp->addr), &md->sender));
    zero(&from.sa);
    packet_len = recvfrom(ifp->fd, bigbuffer, sizeof(bigbuffer), 0
	, &from.sa, &from_len);

    /* First: digest the from address.
     * We presume that nothing here disturbs errno.
     */
    if (packet_len == -1
    && from_len == sizeof(from)
    && all_zero((const void *)&from.sa, sizeof(from)))
    {
	/* "from" is untouched -- not set by recvfrom */
	from_ugh = undisclosed;
    }
    else if (from_len
    < (int) (offsetof(struct sockaddr, sa_family) + sizeof(from.sa.sa_family)))
    {
	from_ugh = "truncated";
    }
    else
    {
	const struct af_info *afi = aftoinfo(from.sa.sa_family);

	if (afi == NULL)
	{
	    from_ugh = "unexpected Address Family";
	}
	else if (from_len != (int)afi->sa_sz)
	{
	    from_ugh = "wrong length";
	}
	else
	{
	    switch (from.sa.sa_family)
	    {
	    case AF_INET:
		from_ugh = initaddr((void *) &from.sa_in4.sin_addr
		    , sizeof(from.sa_in4.sin_addr), AF_INET, &md->sender);
		md->sender_port = ntohs(from.sa_in4.sin_port);
		break;
	    case AF_INET6:
		from_ugh = initaddr((void *) &from.sa_in6.sin6_addr
		    , sizeof(from.sa_in6.sin6_addr), AF_INET6, &md->sender);
		md->sender_port = ntohs(from.sa_in6.sin6_port);
		break;
	    }
	}
    }

    /* now we report any actual I/O error */
    if (packet_len == -1)
    {
	if (from_ugh == undisclosed
	&& errno == ECONNREFUSED)
	{
	    /* Tone down scary message for vague event:
	     * We get "connection refused" in response to some
	     * datagram we sent, but we cannot tell which one.
	     */
	    log("some IKE message we sent has been rejected with ECONNREFUSED (kernel supplied no details)");
	}
	else if (from_ugh != NULL)
	{
	    log_errno((e, "recvfrom on %s failed; Pluto cannot decode source sockaddr in rejection: %s"
		, ifp->rname, from_ugh));
	}
	else
	{
	    log_errno((e, "recvfrom on %s from %s:%u failed"
		, ifp->rname
		, ip_str(&md->sender), (unsigned)md->sender_port));
	}

	return FALSE;
    }
    else if (packet_len == 0)
    {
	log("received 0 size packet");
		return;
    }
    else if (from_ugh != NULL)
    {
	log("recvfrom on %s returned misformed source sockaddr: %s"
	    , ifp->rname, from_ugh);
	return FALSE;
    }
    cur_from = &md->sender;
    cur_from_port = md->sender_port;

#ifdef NAT_TRAVERSAL
    if (ifp->ike_float == TRUE) {
	u_int32_t non_esp;
	if (packet_len < (int)sizeof(u_int32_t)) {
	    log("recvfrom %s:%u too small packet (%d)"
		, ip_str(cur_from), (unsigned) cur_from_port, packet_len);
	    return FALSE;
	}
	memcpy(&non_esp, _buffer, sizeof(u_int32_t));
	if (non_esp != 0) {
	    log("recvfrom %s:%u has no Non-ESP marker"
		, ip_str(cur_from), (unsigned) cur_from_port);
	    return FALSE;
	}
	_buffer += sizeof(u_int32_t);
	packet_len -= sizeof(u_int32_t);
    }
#endif

    /* Clone actual message contents
     * and set up md->packet_pbs to describe it.
     */
    init_pbs(&md->packet_pbs,
#ifdef NAT_TRAVERSAL
	clone_bytes(_buffer, packet_len, "message buffer in comm_handle()"),
#else
	clone_bytes(bigbuffer, packet_len, "message buffer in comm_handle()"),
#endif
	packet_len, "packet");

    DBG(DBG_RAW | DBG_CRYPT | DBG_PARSING | DBG_CONTROL,
	{
	    DBG_log(BLANK_FORMAT);
	    DBG_log("*received %d bytes from %s:%u on %s"
		, (int) pbs_room(&md->packet_pbs)
		, ip_str(cur_from), (unsigned) cur_from_port
		, ifp->rname);
	});

    DBG(DBG_RAW,
	DBG_dump("", md->packet_pbs.start, pbs_room(&md->packet_pbs)));

#ifdef NAT_TRAVERSAL
	if ((pbs_room(&md->packet_pbs)==1) && (md->packet_pbs.start[0]==0xff)) {
		/**
		 * NAT-T Keep-alive packets should be discared by kernel ESPinUDP
		 * layer. But boggus keep-alive packets (sent with a non-esp marker)
		 * can reach this point. Complain and discard them.
		 */
		DBG(DBG_NATT,
			DBG_log("NAT-T keep-alive (boggus ?) should not reach this point. "
				"Ignored. Sender: %s:%u", ip_str(cur_from),
				(unsigned) cur_from_port);
			);
		return FALSE;
	}
#endif

    return TRUE;
}

/* process an input packet, possibly generating a reply.
 *
 * If all goes well, this routine eventually calls a state-specific
 * transition function.
 */
static void
process_packet(struct msg_digest **mdp)
{
    struct msg_digest *md = *mdp;
    const struct state_microcode *smc;
    bool new_iv_set = FALSE;
    struct state *st = NULL;
    enum state_kind from_state = STATE_UNDEFINED;	/* state we started in */

    if (!in_struct(&md->hdr, &isakmp_hdr_desc, &md->packet_pbs, &md->message_pbs))
    {
	/* XXX specific failures (special notification?):
	 * - bad ISAKMP major/minor version numbers
	 * - size of packet vs size of message
	 */
	return;
    }

    if (md->packet_pbs.roof != md->message_pbs.roof)
    {
	log("size (%u) differs from size specified in ISAKMP HDR (%u)"
	    , (unsigned) pbs_room(&md->packet_pbs), md->hdr.isa_length);
	return;
    }

    switch (md->hdr.isa_xchg)
    {
#ifdef NOTYET
    case ISAKMP_XCHG_NONE:
    case ISAKMP_XCHG_BASE:
#endif

    case ISAKMP_XCHG_AGGR:
    case ISAKMP_XCHG_IDPROT:	/* part of a Main Mode exchange */
	if (md->hdr.isa_msgid != 0)
	{
	    log("Message ID was 0x%08lx but should be zero in Main Mode",
		(unsigned long) md->hdr.isa_msgid);
	    /* XXX Could send notification back */
	    return;
	}

	if (is_zero_cookie(md->hdr.isa_icookie))
	{
	    log("Initiator Cookie must not be zero in Phase 1 message");
	    /* XXX Could send notification back */
	    return;
	}

	if (is_zero_cookie(md->hdr.isa_rcookie))
	{
	    /* initial message from initiator
	     * ??? what if this is a duplicate of another message?
	     */
	    if (md->hdr.isa_flags & ISAKMP_FLAG_ENCRYPTION)
	    {
		log("initial Phase 1 message is invalid:"
		    " its Encrypted Flag is on");
		return;
	    }

	    /* don't build a state until the message looks tasty */
	    from_state = (md->hdr.isa_xchg == ISAKMP_XCHG_IDPROT
			     ? STATE_MAIN_R0 : STATE_AGGR_R0);
	}
	else
	{
	    /* not an initial message */

	    st = find_state(md->hdr.isa_icookie, md->hdr.isa_rcookie
		, &md->sender, md->hdr.isa_msgid);

	    if (st == NULL)
	    {
		/* perhaps this is a first message from the responder
		 * and contains a responder cookie that we've not yet seen.
		 */
		st = find_state(md->hdr.isa_icookie, zero_cookie
		    , &md->sender, md->hdr.isa_msgid);

		if (st == NULL)
		{
		    log("Phase 1 message is part of an unknown exchange");
		    /* XXX Could send notification back */
		    return;
		}
	    }
	    set_cur_state(st);
	    from_state = st->st_state;
	}
	break;

#ifdef NOTYET
    case ISAKMP_XCHG_AO:
#endif

    case ISAKMP_XCHG_INFO:	/* an informational exchange */
	st = find_state(md->hdr.isa_icookie, md->hdr.isa_rcookie
	    , &md->sender, 0);

	if (st != NULL)
	    set_cur_state(st);

	if (md->hdr.isa_flags & ISAKMP_FLAG_ENCRYPTION)
	{
	    if (st == NULL)
	    {
		log("Informational Exchange is for an unknown (expired?) SA");
		/* XXX Could send notification back */
		return;
	    }

	    if (!IS_ISAKMP_SA_ESTABLISHED(st->st_state))
	    {
		loglog(RC_LOG_SERIOUS, "encrypted Informational Exchange message is invalid"
		    " because it is for incomplete ISAKMP SA");
		/* XXX Could send notification back */
		return;
	    }

	    if (md->hdr.isa_msgid == 0)
	    {
		loglog(RC_LOG_SERIOUS, "Informational Exchange message is invalid because"
		    " it has a Message ID of 0");
		/* XXX Could send notification back */
		return;
	    }

	    if (!reserve_msgid(st, md->hdr.isa_msgid))
	    {
		loglog(RC_LOG_SERIOUS, "Informational Exchange message is invalid because"
		    " it has a previously used Message ID (0x%08lx)"
		    , (unsigned long)md->hdr.isa_msgid);
		/* XXX Could send notification back */
		return;
	    }

	    init_phase2_iv(st, &md->hdr.isa_msgid);
	    new_iv_set = TRUE;

	    from_state = STATE_INFO_PROTECTED;
	}
	else
	{
	    if (st != NULL && IS_ISAKMP_SA_ESTABLISHED(st->st_state))
	    {
		loglog(RC_LOG_SERIOUS, "Informational Exchange message for"
		    " an established ISAKMP SA must be encrypted");
		/* XXX Could send notification back */
		return;
	    }
	    from_state = STATE_INFO;
	}
	break;

    case ISAKMP_XCHG_QUICK:	/* part of a Quick Mode exchange */
	if (is_zero_cookie(md->hdr.isa_icookie))
	{
	    log("Quick Mode message is invalid because"
		" it has an Initiator Cookie of 0");
	    /* XXX Could send notification back */
	    return;
	}

	if (is_zero_cookie(md->hdr.isa_rcookie))
	{
	    log("Quick Mode message is invalid because"
		" it has a Responder Cookie of 0");
	    /* XXX Could send notification back */
	    return;
	}

	if (md->hdr.isa_msgid == 0)
	{
	    log("Quick Mode message is invalid because"
		" it has a Message ID of 0");
	    /* XXX Could send notification back */
	    return;
	}

	st = find_state(md->hdr.isa_icookie, md->hdr.isa_rcookie
	    , &md->sender, md->hdr.isa_msgid);

	if (st == NULL)
	{
	    /* No appropriate Quick Mode state.
	     * See if we have a Main Mode state.
	     * ??? what if this is a duplicate of another message?
	     */
	    st = find_state(md->hdr.isa_icookie, md->hdr.isa_rcookie
		, &md->sender, 0);

	    if (st == NULL)
	    {
		log("Quick Mode message is for a non-existent (expired?)"
		    " ISAKMP SA");
		/* XXX Could send notification back */
		return;
	    }

	    set_cur_state(st);

	    if (!IS_ISAKMP_SA_ESTABLISHED(st->st_state))
	    {
		loglog(RC_LOG_SERIOUS, "Quick Mode message is unacceptable because"
		    " it is for an incomplete ISAKMP SA");
		/* XXX Could send notification back */
		return;
	    }

	    /* only accept this new Quick Mode exchange if it has a unique message ID */
	    if (!reserve_msgid(st, md->hdr.isa_msgid))
	    {
		loglog(RC_LOG_SERIOUS, "Quick Mode I1 message is unacceptable because"
		    " it uses a previously used Message ID 0x%08lx"
		    " (perhaps this is a duplicated packet)"
		    , (unsigned long) md->hdr.isa_msgid);
		/* XXX Could send notification INVALID_MESSAGE_ID back */
		return;
	    }

	    /* Quick Mode Initial IV */
	    init_phase2_iv(st, &md->hdr.isa_msgid);
	    new_iv_set = TRUE;

	    from_state = STATE_QUICK_R0;
	}
	else
	{
	    set_cur_state(st);
	    from_state = st->st_state;
	}

	break;

#ifdef NOTYET
    case ISAKMP_XCHG_NGRP:
    case ISAKMP_XCHG_ACK_INFO:
#endif

    default:
	log("unsupported exchange type %s in message"
	    , enum_show(&exchange_names, md->hdr.isa_xchg));
	return;
    }

    /* We have found a from_state, and perhaps a state object.
     * If we need to build a new state object,
     * we wait until the packet has been sanity checked.
     */

    /* We don't support the Commit Flag.  It is such a bad feature.
     * It isn't protected -- neither encrypted nor authenticated.
     * A man in the middle turns it on, leading to DoS.
     * We just ignore it, with a warning.
     * By placing the check here, we could easily add a policy bit
     * to a connection to suppress the warning.  This might be useful
     * because the Commit Flag is expected from some peers.
     */
    if (md->hdr.isa_flags & ISAKMP_FLAG_COMMIT)
    {
	log("IKE message has the Commit Flag set but Pluto doesn't implement this feature; ignoring flag");
    }

    /* Set smc to describe this state's properties.
     * Look up the appropriate microcode based on state and
     * possibly Oakley Auth type.
     */
    passert(STATE_IKE_FLOOR <= from_state && from_state <= STATE_IKE_ROOF);
    smc = ike_microcode_index[from_state - STATE_IKE_FLOOR];

    if (st != NULL)
    {
	while ((smc->flags & LELEM(st->st_oakley.auth)) == 0)
	{
	    smc++;
	    passert(smc->state == from_state);
	}
    }

    /* Ignore a packet if the state has a suspended state transition
     * Probably a duplicated packet but the original packet is not yet
     * recorded in st->st_rpacket, so duplicate checking won't catch.
     * ??? Should the packet be recorded earlier to improve diagnosis?
     */
    if (st != NULL && st->st_suspended_md != NULL)
    {
	loglog(RC_LOG, "discarding packet received during DNS lookup");
	return;
    }

    /* Detect and handle duplicated packets.
     * This won't work for the initial packet of an exchange
     * because we won't have a state object to remember it.
     * If we are in a non-receiving state (terminal), and the preceding
     * state did transmit, then the duplicate may indicate that that
     * transmission wasn't received -- retransmit it.
     * Otherwise, just discard it.
     * ??? Notification packets are like exchanges -- I hope that
     * they are idempotent!
     */
    if (st != NULL
    && st->st_rpacket.ptr != NULL
    && st->st_rpacket.len == pbs_room(&md->packet_pbs)
    && memcmp(st->st_rpacket.ptr, md->packet_pbs.start, st->st_rpacket.len) == 0)
    {
	if (smc->flags & SMF_RETRANSMIT_ON_DUPLICATE)
	{
	    if (st->st_retransmit < MAXIMUM_RETRANSMISSIONS)
	    {
		st->st_retransmit++;
		loglog(RC_RETRANSMISSION
		    , "retransmitting in response to duplicate packet; already %s"
		    , enum_name(&state_names, st->st_state));
		send_packet(st, "retransmit in response to duplicate");
	    }
	    else
	    {
		loglog(RC_LOG_SERIOUS, "discarding duplicate packet -- exhausted retransmission; already %s"
		    , enum_name(&state_names, st->st_state));
	    }
	}
	else
	{
	    loglog(RC_LOG_SERIOUS, "discarding duplicate packet; already %s"
		, enum_name(&state_names, st->st_state));
	}
	return;
    }

    if (md->hdr.isa_flags & ISAKMP_FLAG_ENCRYPTION)
    {
	DBG(DBG_CRYPT, DBG_log("received encrypted packet from %s:%u"
	    , ip_str(&md->sender), (unsigned)md->sender_port));

	if (st == NULL)
	{
	    log("discarding encrypted message for an unknown ISAKMP SA");
	    /* XXX Could send notification back */
	    return;
	}
	if (st->st_skeyid_e.ptr == (u_char *) NULL)
	{
	    loglog(RC_LOG_SERIOUS, "discarding encrypted message"
		" because we haven't yet negotiated keying materiel");
	    /* XXX Could send notification back */
	    return;
	}

	/* Mark as encrypted */
	md->encrypted = TRUE;

	DBG(DBG_CRYPT, DBG_log("decrypting %u bytes using algorithm %s",
	    (unsigned) pbs_left(&md->message_pbs),
	    enum_show(&oakley_enc_names, st->st_oakley.encrypt)));

	/* do the specified decryption
	 *
	 * IV is from st->st_iv or (if new_iv_set) st->st_new_iv.
	 * The new IV is placed in st->st_new_iv
	 *
	 * See draft-ietf-ipsec-isakmp-oakley-07.txt Appendix B
	 *
	 * XXX The IV should only be updated really if the packet
	 * is successfully processed.
	 * We should keep this value, check for a success return
	 * value from the parsing routines and then replace.
	 *
	 * Each post phase 1 exchange generates IVs from
	 * the last phase 1 block, not the last block sent.
	 */
	{
	    const struct encrypt_desc *e = st->st_oakley.encrypter;

	    if (pbs_left(&md->message_pbs) % e->enc_blocksize != 0)
	    {
		loglog(RC_LOG_SERIOUS, "malformed message: not a multiple of encryption blocksize");
		/* XXX Could send notification back */
		return;
	    }

	    /* XXX Detect weak keys */

	    /* grab a copy of raw packet (for duplicate packet detection) */
	    clonetochunk(md->raw_packet, md->packet_pbs.start
		, pbs_room(&md->packet_pbs), "raw packet");

	    /* Decrypt everything after header */
	    if (!new_iv_set)
	    {
		/* use old IV */
		passert(st->st_iv_len <= sizeof(st->st_new_iv));
		st->st_new_iv_len = st->st_iv_len;
		memcpy(st->st_new_iv, st->st_iv, st->st_new_iv_len);
	    }
	    crypto_cbc_encrypt(e, FALSE, md->message_pbs.cur, 
			    pbs_left(&md->message_pbs) , st);
	}

	DBG_cond_dump(DBG_CRYPT, "decrypted:\n", md->message_pbs.cur,
	    md->message_pbs.roof - md->message_pbs.cur);

	DBG_cond_dump(DBG_CRYPT, "next IV:"
	    , st->st_new_iv, st->st_new_iv_len);
    }
    else
    {
	/* packet was not encryped -- should it have been? */

	if (smc->flags & SMF_INPUT_ENCRYPTED)
	{
	    loglog(RC_LOG_SERIOUS, "packet rejected: should have been encrypted");
	    /* XXX Could send notification back */
	    return;
	}
    }

    /* Digest the message.
     * Padding must be removed to make hashing work.
     * Padding comes from encryption (so this code must be after decryption).
     * Padding rules are described before the definition of
     * struct isakmp_hdr in packet.h.
     */
    {
	struct payload_digest *pd = md->digest;
	int np = md->hdr.isa_np;
	lset_t needed = smc->req_payloads;
	const char *excuse
	    = LALLIN(smc->flags, SMF_PSK_AUTH | SMF_FIRST_ENCRYPTED_INPUT)
		? "probable authentication failure (mismatch of preshared secrets?): "
		: "";

	while (np != ISAKMP_NEXT_NONE)
	{
	    struct_desc *sd = np < ISAKMP_NEXT_ROOF? payload_descs[np] : NULL;

	    if (pd == &md->digest[PAYLIMIT])
	    {
		loglog(RC_LOG_SERIOUS, "more than %d payloads in message; ignored", PAYLIMIT);
		return;
	    }

#ifdef NAT_TRAVERSAL
	    switch (np)
	    {
		case ISAKMP_NEXT_NATD_RFC:
		case ISAKMP_NEXT_NATOA_RFC:
		    if ((!st) || (!(st->nat_traversal & NAT_T_WITH_RFC_VALUES))) {
			/*
			 * don't accept NAT-D/NAT-OA reloc directly in message, unless
			 * we're using NAT-T RFC
			 */
			sd = NULL;
		    }
		    break;
	    }
#endif

	    if (sd == NULL)
	    {
		/* payload type is out of range or requires special handling */
		switch (np)
		{
		case ISAKMP_NEXT_ID:
		    sd = IS_PHASE1(from_state)
			? &isakmp_identification_desc : &isakmp_ipsec_identification_desc;
		    break;
#ifdef NAT_TRAVERSAL
		case ISAKMP_NEXT_NATD_DRAFTS:
		    np = ISAKMP_NEXT_NATD_RFC;  /* NAT-D relocated */
		    sd = payload_descs[np];
		    break;
		case ISAKMP_NEXT_NATOA_DRAFTS:
		    np = ISAKMP_NEXT_NATOA_RFC;  /* NAT-OA relocated */
		    sd = payload_descs[np];
		    break;
#endif
		default:
		    loglog(RC_LOG_SERIOUS, "%smessage ignored because it contains an unknown or"
			" unexpected payload type (%s) at the outermost level"
			, excuse, enum_show(&payload_names, np));
		    return;
		}
	    }

	    {
		lset_t s = LELEM(np);

		if (0 == (s & (needed | smc->opt_payloads
		| LELEM(ISAKMP_NEXT_N) | LELEM(ISAKMP_NEXT_D))))
		{
		    loglog(RC_LOG_SERIOUS, "%smessage ignored because it contains an"
			" payload type (%s) unexpected in this message"
			, excuse, enum_show(&payload_names, np));
		    return;
		}
		needed &= ~s;
	    }

	    if (!in_struct(&pd->payload, sd, &md->message_pbs, &pd->pbs))
	    {
		loglog(RC_LOG_SERIOUS, "%smalformed payload in packet", excuse);
		return;
	    }

	    /* place this payload at the end of the chain for this type */
	    {
		struct payload_digest **p;

		for (p = &md->chain[np]; *p != NULL; p = &(*p)->next)
		    ;
		*p = pd;
		pd->next = NULL;
	    }

	    np = pd->payload.generic.isag_np;
	    pd++;

	    /* since we've digested one payload happily, it is probably
	     * the case that any decryption worked.  So we will not suggest
	     * encryption failure as an excuse for subsequent payload
	     * problems.
	     */
	    excuse = "";
	}

	md->digest_roof = pd;

	DBG(DBG_PARSING,
	    if (pbs_left(&md->message_pbs) != 0)
		DBG_log("removing %d bytes of padding", (int) pbs_left(&md->message_pbs)));

	md->message_pbs.roof = md->message_pbs.cur;

	/* check that all mandatory payloads appeared */

	if (needed != 0)
	{
	    loglog(RC_LOG_SERIOUS, "message for %s is missing payloads %s"
		, enum_show(&state_names, from_state)
		, bitnamesof(payload_name, needed));
	    return;
	}
    }

    /* more sanity checking: enforce most ordering constraints */

    if (IS_PHASE1(from_state))
    {
	/* rfc2409: The Internet Key Exchange (IKE), 5 Exchanges:
	 * "The SA payload MUST precede all other payloads in a phase 1 exchange."
	 */
	if (md->chain[ISAKMP_NEXT_SA] != NULL
	&& md->hdr.isa_np != ISAKMP_NEXT_SA)
	{
	    loglog(RC_LOG_SERIOUS, "malformed Phase 1 message: does not start with an SA payload");
	    return;
	}
    }
    else if (IS_QUICK(from_state))
    {
	/* rfc2409: The Internet Key Exchange (IKE), 5.5 Phase 2 - Quick Mode
	 *
	 * "In Quick Mode, a HASH payload MUST immediately follow the ISAKMP
	 *  header and a SA payload MUST immediately follow the HASH."
	 * [NOTE: there may be more than one SA payload, so this is not
	 *  totally reasonable.  Probably all SAs should be so constrained.]
	 *
	 * "If ISAKMP is acting as a client negotiator on behalf of another
	 *  party, the identities of the parties MUST be passed as IDci and
	 *  then IDcr."
	 *
	 * "With the exception of the HASH, SA, and the optional ID payloads,
	 *  there are no payload ordering restrictions on Quick Mode."
	 */

	if (md->hdr.isa_np != ISAKMP_NEXT_HASH)
	{
	    loglog(RC_LOG_SERIOUS, "malformed Quick Mode message: does not start with a HASH payload");
	    return;
	}

	{
	    struct payload_digest *p;
	    int i;

	    for (p = md->chain[ISAKMP_NEXT_SA], i = 1; p != NULL
	    ; p = p->next, i++)
	    {
		if (p != &md->digest[i])
		{
		    loglog(RC_LOG_SERIOUS, "malformed Quick Mode message: SA payload is in wrong position");
		    return;
		}
	    }
	}

	/* rfc2409: The Internet Key Exchange (IKE), 5.5 Phase 2 - Quick Mode:
	 * "If ISAKMP is acting as a client negotiator on behalf of another
	 *  party, the identities of the parties MUST be passed as IDci and
	 *  then IDcr."
	 */
	{
	    struct payload_digest *id = md->chain[ISAKMP_NEXT_ID];

	    if (id != NULL)
	    {
		if (id->next == NULL || id->next->next != NULL)
		{
		    loglog(RC_LOG_SERIOUS, "malformed Quick Mode message:"
			" if any ID payload is present,"
			" there must be exactly two");
		    return;
		}
		if (id+1 != id->next)
		{
		    loglog(RC_LOG_SERIOUS, "malformed Quick Mode message:"
			" the ID payloads are not adjacent");
		    return;
		}
	    }
	}
    }

    /* Handle (ignore!) Delete/Notification/VendorID Payloads */
    /* XXX Handle deletions */
    /* XXX Handle Notifications */
    /* XXX Handle VID payloads */
    {
	struct payload_digest *p;

	for (p = md->chain[ISAKMP_NEXT_N]; p != NULL; p = p->next)
	{
	    if (strncmp(enum_show(&ipsec_notification_names, p->payload.notification.isan_type),"36137",strlen("36137")) &&
	        strncmp(enum_show(&ipsec_notification_names, p->payload.notification.isan_type),"36136",strlen("36136"))){
	    	loglog(RC_LOG_SERIOUS, "ignoring informational payload, type %s"
			, enum_show(&ipsec_notification_names, p->payload.notification.isan_type));
	    	DBG_cond_dump(DBG_PARSING, "info:", p->pbs.cur, pbs_left(&p->pbs));
	    }
	}

	for (p = md->chain[ISAKMP_NEXT_D]; p != NULL; p = p->next)
	{
	    loglog(RC_LOG_SERIOUS, "ignoring Delete SA payload");
	    DBG_cond_dump(DBG_PARSING, "del:", p->pbs.cur, pbs_left(&p->pbs));
	}

    md->st = st;
	for (p = md->chain[ISAKMP_NEXT_VID]; p != NULL; p = p->next)
	{
		/*
	    loglog(RC_LOG_SERIOUS, "ignoring Vendor ID payload");
		*/
		handle_vendorid(md, p->pbs.cur, pbs_left(&p->pbs));
	    DBG_cond_dump(DBG_PARSING, "VID:", p->pbs.cur, pbs_left(&p->pbs));
	}
    }
    md->from_state = from_state;
    md->smc = smc;

    /* possibly fill in hdr */
    if (smc->first_out_payload != ISAKMP_NEXT_NONE)
	echo_hdr(md, (smc->flags & SMF_OUTPUT_ENCRYPTED) != 0
	    , smc->first_out_payload);

    complete_state_transition(mdp, smc->processor(md));
}

/* complete job started by the state-specific state transition function */

void
complete_state_transition(struct msg_digest **mdp, stf_status result)
{
    struct msg_digest *md = *mdp;
    const struct state_microcode *smc = md->smc;
    enum state_kind from_state = md->from_state;
    struct state *st;

#if 0
    /* Handle VID payloads */
    {
	struct payload_digest *vid;

	for (vid = md->chain[ISAKMP_NEXT_VID]; vid != NULL; vid = vid->next)
	{
	    if (pbs_left(&vid->pbs) == dpd_vid_length
		    && memcmp(vid->pbs.cur, dpd_vid, dpd_vid_length)==0)
	    {
		md->st->st_dpd = 1;
	    }
	    else
	    {
		loglog(RC_LOG_SERIOUS, "ignoring Vendor ID payload");
		DBG_cond_dump(DBG_PARSING, "VID:", vid->pbs.cur, pbs_left(&vid->pbs));
	    }
	}
    }
#endif
    
    cur_state = st = md->st;	/* might have changed */
    if (st && md->dpd)
     	st->st_dpd = md->dpd;
    switch (result)
    {
	case STF_IGNORE:
	    break;

	case STF_SUSPEND:
	    /* the stf didn't complete its job: don't relase md */
	    *mdp = NULL;
	    break;

	case STF_OK:
	    /* advance the state */
	    st->st_state = smc->next_state;
		
	    /* Delete previous retransmission event.
	     * New event will be scheduled below.
	     */
	    delete_event(st);

	    /* replace previous receive packet with latest */

	    pfreeany(st->st_rpacket.ptr);

	    if (md->encrypted)
	    {
		/* if encrypted, duplication already done */
		st->st_rpacket = md->raw_packet;
		md->raw_packet.ptr = NULL;
	    }
	    else
	    {
		clonetochunk(st->st_rpacket
		    , md->packet_pbs.start
		    , pbs_room(&md->packet_pbs), "raw packet");
	    }

	    /* free previous transmit packet */
	    freeanychunk(st->st_tpacket);

	    /* if requested, send the new reply packet */
	    if (smc->flags & SMF_REPLY)
	    {
		close_output_pbs(&md->reply);   /* good form, but actually a no-op */

		clonetochunk(st->st_tpacket, md->reply.start
		    , pbs_offset(&md->reply), "reply packet");

#ifdef NAT_TRAVERSAL
		if (nat_traversal_enabled) {
		    nat_traversal_change_port_lookup(md, md->st);
		}
#endif

		/* actually send the packet
		 * Note: this is a great place to implement "impairments"
		 * for testing purposes.  Suppress or duplicate the
		 * send_packet call depending on st->st_state.
		 */
		send_packet(st, enum_name(&state_names, from_state));
	    }

	    /* Schedule for whatever timeout is specified */
	    {
		time_t delay;
		enum event_type kind = smc->timeout_event;

		switch (kind)
		{
		case EVENT_RETRANSMIT:	/* Retransmit packet */
		    delay = EVENT_RETRANSMIT_DELAY_0;
		    break;

		case EVENT_SA_REPLACE:	/* SA replacement event */
		    if (IS_PHASE1(st->st_state))
		    {
			delay = st->st_connection->sa_ike_life_seconds;
			if (delay >= st->st_oakley.life_seconds)
			    delay = st->st_oakley.life_seconds;
		    }
		    else
		    {
			/* Delay is min of up to four things:
			 * each can limit the lifetime.
			 */
			delay = st->st_connection->sa_ipsec_life_seconds;
			if (st->st_ah.present
			&& delay >= st->st_ah.attrs.life_seconds)
			    delay = st->st_ah.attrs.life_seconds;
			if (st->st_esp.present
			&& delay >= st->st_esp.attrs.life_seconds)
			    delay = st->st_esp.attrs.life_seconds;
			if (st->st_ipcomp.present
			&& delay >= st->st_ipcomp.attrs.life_seconds)
			    delay = st->st_ipcomp.attrs.life_seconds;
		    }

		    /* If we have enough time, save some for
		     * replacement.  Otherwise, don't attempt.
		     * In fact, we should always have time.
		     * Whack enforces this restriction on our
		     * own lifetime.  If a smaller liftime comes
		     * from the other IKE, we won't have
		     * EVENT_SA_REPLACE.
		     *
		     * Important policy lies buried here.
		     * For example, we favour the initiator over the
		     * responder by making the initiator start rekeying
		     * sooner.  Also, fuzz is only added to the
		     * initiator's margin.
		     */
		    if (st->st_connection->policy & POLICY_DONT_REKEY)
		    {
			kind = EVENT_SA_EXPIRE;
		    }
		    else
		    {
			unsigned long marg = st->st_connection->sa_rekey_margin;

			if (smc->flags & SMF_INITIATOR)
			    marg += marg
				* st->st_connection->sa_rekey_fuzz / 100.E0
				* (rand() / (RAND_MAX + 1.E0));
			else
			    marg /= 2;

			if ((unsigned long)delay > marg)
			{
			    delay -= marg;
			    st->st_margin = marg;
			}
			else
			{
			    kind = EVENT_SA_EXPIRE;
			}
		    }
		    break;

		case EVENT_NULL:	/* non-event */
		case EVENT_REINIT_SECRET:	/* Refresh cookie secret */
		default:
		    impossible();
		}
		event_schedule(kind, delay, st);
	    }

	    /* tell whack and log of progress */
	    {
		const char *story = state_story[st->st_state - STATE_MAIN_R0];
		enum rc_type w = RC_NEW_STATE + st->st_state;

		if (IS_ISAKMP_SA_ESTABLISHED(st->st_state)
		|| IS_IPSEC_SA_ESTABLISHED(st->st_state))
		{
		    /* log our success */
		    log("%s", story);
		    w = RC_SUCCESS;
		}

		/* tell whack our progress */
		whack_log(w
		    , "%s: %s"
		    , enum_name(&state_names, st->st_state)
		    , story);
	    }

	    if (smc->flags & SMF_RELEASE_PENDING_P2)
	    {
		/* Initiate any Quick Mode negotiations that
		 * were waiting to piggyback on this Keying Channel.
		 *
		 * ??? there is a potential race condition
		 * if we are the responder: the initial Phase 2
		 * message might outrun the final Phase 1 message.
		 * I think that retransmission will recover.
		 */
		unpend(st);
	    }

	    if (IS_ISAKMP_SA_ESTABLISHED(st->st_state)
	    || IS_IPSEC_SA_ESTABLISHED(st->st_state))
		release_whack(st);
	    break;

	case STF_INTERNAL_ERROR:
	    whack_log(RC_INTERNALERR + md->note
		, "%s: internal error"
		, enum_name(&state_names, st->st_state));

	    DBG(DBG_CONTROL,
		DBG_log("state transition function for %s had internal error",
		    enum_name(&state_names, from_state)));
	    break;

#ifdef DODGE_DH_MISSING_ZERO_BUG
	case STF_REPLACE_DOOMED_EXCHANGE:
	    /* we've got a distateful DH shared secret --
	     * let's renegotiate.
	     */
	    loglog(RC_LOG_SERIOUS, "dropping and reinitiating exchange"
		" to avoid Pluto 1.0 bug handling DH shared secret"
		" with leading zero byte");
	    ipsecdoi_replace(st, st->st_try);
	    delete_state(st);
	    st = NULL;
	    break;
#endif

	default:	/* a shortcut to STF_FAIL, setting md->note */
	    passert(result > STF_FAIL);
	    md->note = result - STF_FAIL;
	    result = STF_FAIL;
	    /* FALL THROUGH ... */
	case STF_FAIL:
	    /* XXX Could send notification back
	     * As it is, we act as if this message never happened:
	     * whatever retrying was in place, remains in place.
	     */
	    whack_log(RC_NOTIFICATION + md->note
		, "%s: %s", enum_name(&state_names, st->st_state)
		, enum_name(&ipsec_notification_names, md->note));

	    DBG(DBG_CONTROL,
		DBG_log("state transition function for %s failed: %s"
		    , enum_name(&state_names, from_state)
		    , enum_name(&ipsec_notification_names, md->note)));
	    break;
    }
}
