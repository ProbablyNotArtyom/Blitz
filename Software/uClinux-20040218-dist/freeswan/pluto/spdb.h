/* Security Policy Data Base (such as it is)
 * Copyright (C) 1998, 1999  D. Hugh Redelmeier.
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
 * RCSID $Id: spdb.h,v 1.11 2000/09/15 09:09:28 dhr Exp $
 */

/* database of SA properties */

/* Attribute type and value pair.
 * Note: only "basic" values are represented so far.
 */
struct db_attr {
    u_int16_t type;	/* ISAKMP_ATTR_AF_TV is implied; 0 for end */
    u_int16_t val;
};

/* transform */
struct db_trans {
    u_int8_t transid;	/* Transform-Id */
    struct db_attr *attrs;	/* array */
    int attr_cnt;	/* number of elements */
};

/* proposal */
struct db_prop {
    u_int8_t protoid;	/* Protocol-Id */
    struct db_trans *trans;	/* array (disjunction) */
    int trans_cnt;	/* number of elements */
    /* SPI size and value isn't part of DB */
};

/* conjunction of proposals */
struct db_prop_conj {
    struct db_prop *props;	/* array */
    int prop_cnt;	/* number of elements */
};

/* security association */
struct db_sa {
    struct db_prop_conj *prop_conjs;	/* array */
    int prop_conj_cnt;	/* number of elements */
    /* Hardwired for now;
     * DOI: ISAKMP_DOI_IPSEC
     * Situation: SIT_IDENTITY_ONLY
     */
};

/* The oakley sadb is subscripted by a bitset with members
 * from POLICY_PSK and POLICY_RSASIG.
 */
extern struct db_sa oakley_sadb[1 << 2];

/* The oakley sadb for aggressive mode.
 */
extern struct db_sa otpsk768des3sha_sadb_am;
extern struct db_sa otpsk1024des3sha_sadb_am;
extern struct db_sa otpsk1536des3sha_sadb_am;
extern struct db_sa otpsk768des3md5_sadb_am;
extern struct db_sa otpsk1024des3md5_sadb_am;
extern struct db_sa otpsk1536des3md5_sadb_am;
extern struct db_sa otpsk768dessha_sadb_am;
extern struct db_sa otpsk1024dessha_sadb_am;
extern struct db_sa otpsk1536dessha_sadb_am;
extern struct db_sa otpsk768desmd5_sadb_am;
extern struct db_sa otpsk1024desmd5_sadb_am;
extern struct db_sa otpsk1536desmd5_sadb_am;


/* The ipsec sadb is subscripted by a bitset with members
 * from POLICY_ENCRYPT, POLICY_AUTHENTICATE, POLICY_COMPRESS
 */
extern struct db_sa ipsec_sadb[1 << 3];

extern bool out_sa(
    pb_stream *outs,
    struct db_sa *sadb,
    struct state *st,
    bool oakley_mode,
    u_int8_t np);
#if FREESWAN_OLD
extern complaint_t accept_oakley_auth_method(
    struct state *st,   /* current state object */
    u_int32_t amethod,  /* room for larger values */
    bool credcheck);    /* whether we can check credentials now */

extern int init_st_oakley(struct state *st, lset_t policy);
#endif
extern notification_t parse_isakmp_sa_body(
    pb_stream *sa_pbs,	/* body of input SA Payload */
    const struct isakmp_sa *sa,	/* header of input SA Payload */
    pb_stream *r_sa_pbs,	/* if non-NULL, where to emit winning SA */
    bool selection,	/* if this SA is a selection, only one tranform can appear */
    struct state *st);	/* current state object */

extern notification_t parse_ipsec_sa_body(
    pb_stream *sa_pbs,	/* body of input SA Payload */
    const struct isakmp_sa *sa,	/* header of input SA Payload */
    pb_stream *r_sa_pbs,	/* if non-NULL, where to emit winning SA */
    bool selection,	/* if this SA is a selection, only one tranform can appear */
    struct state *st);	/* current state object */
