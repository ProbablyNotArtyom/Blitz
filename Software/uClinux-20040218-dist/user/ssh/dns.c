/*	$OpenBSD: dns.c,v 1.6 2003/06/11 10:18:47 jakob Exp $	*/

/*
 * Copyright (c) 2003 Wesley Griffin. All rights reserved.
 * Copyright (c) 2003 Jakob Schlyter. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "includes.h"

#ifdef DNS
#include <openssl/bn.h>
#ifdef LWRES
#include <lwres/netdb.h>
#include <dns/result.h>
#else /* LWRES */
#include <netdb.h>
#endif /* LWRES */

#include "xmalloc.h"
#include "key.h"
#include "dns.h"
#include "log.h"
#include "uuencode.h"

extern char *__progname;
RCSID("$OpenBSD: dns.c,v 1.6 2003/06/11 10:18:47 jakob Exp $");

#ifndef LWRES
static const char *errset_text[] = {
	"success",		/* 0 ERRSET_SUCCESS */
	"out of memory",	/* 1 ERRSET_NOMEMORY */
	"general failure",	/* 2 ERRSET_FAIL */
	"invalid parameter",	/* 3 ERRSET_INVAL */
	"name does not exist",	/* 4 ERRSET_NONAME */
	"data does not exist",	/* 5 ERRSET_NODATA */
};

static const char *
dns_result_totext(unsigned int error)
{
	switch (error) {
	case ERRSET_SUCCESS:
		return errset_text[ERRSET_SUCCESS];
	case ERRSET_NOMEMORY:
		return errset_text[ERRSET_NOMEMORY];
	case ERRSET_FAIL:
		return errset_text[ERRSET_FAIL];
	case ERRSET_INVAL:
		return errset_text[ERRSET_INVAL];
	case ERRSET_NONAME:
		return errset_text[ERRSET_NONAME];
	case ERRSET_NODATA:
		return errset_text[ERRSET_NODATA];
	default:
		return "unknown error";
	}
}
#endif /* LWRES */


/*
 * Read SSHFP parameters from key buffer.
 */
static int
dns_read_key(u_int8_t *algorithm, u_int8_t *digest_type,
    u_char **digest, u_int *digest_len, Key *key)
{
	int success = 0;

	switch (key->type) {
	case KEY_RSA:
		*algorithm = SSHFP_KEY_RSA;
		break;
	case KEY_DSA:
		*algorithm = SSHFP_KEY_DSA;
		break;
	default:
		*algorithm = SSHFP_KEY_RESERVED;
	}

	if (*algorithm) {
		*digest_type = SSHFP_HASH_SHA1;
		*digest = key_fingerprint_raw(key, SSH_FP_SHA1, digest_len);
		success = 1;
	} else {
		*digest_type = SSHFP_HASH_RESERVED;
		*digest = NULL;
		*digest_len = 0;
		success = 0;
	}

	return success;
}

/*
 * Read SSHFP parameters from rdata buffer.
 */
static int
dns_read_rdata(u_int8_t *algorithm, u_int8_t *digest_type,
    u_char **digest, u_int *digest_len, u_char *rdata, int rdata_len)
{
	int success = 0;

	*algorithm = SSHFP_KEY_RESERVED;
	*digest_type = SSHFP_HASH_RESERVED;

	if (rdata_len >= 2) {
		*algorithm = rdata[0];
		*digest_type = rdata[1];
		*digest_len = rdata_len - 2;

		if (*digest_len > 0) {
			*digest = (u_char *) xmalloc(*digest_len);
			memcpy(*digest, rdata + 2, *digest_len);
		} else {
			*digest = NULL;
		}

		success = 1;
	}

	return success;
}


/*
 * Verify the given hostname, address and host key using DNS.
 * Returns 0 if key verifies or -1 if key does NOT verify
 */
int
verify_host_key_dns(const char *hostname, struct sockaddr *address,
    Key *hostkey)
{
	int counter;
	int result;
	struct rrsetinfo *fingerprints = NULL;
	int failures = 0;

	u_int8_t hostkey_algorithm;
	u_int8_t hostkey_digest_type;
	u_char *hostkey_digest;
	u_int hostkey_digest_len;

	u_int8_t dnskey_algorithm;
	u_int8_t dnskey_digest_type;
	u_char *dnskey_digest;
	u_int dnskey_digest_len;


	debug3("verify_hostkey_dns");
	if (hostkey == NULL)
		fatal("No key to look up!");

	result = getrrsetbyname(hostname, DNS_RDATACLASS_IN,
	    DNS_RDATATYPE_SSHFP, 0, &fingerprints);
	if (result) {
		verbose("DNS lookup error: %s", dns_result_totext(result));
		return DNS_VERIFY_ERROR;
	}

#ifdef DNSSEC
	/* Only accept validated answers */
	if (!fingerprints->rri_flags & RRSET_VALIDATED) {
		error("Ignored unvalidated fingerprint from DNS.");
		freerrset(fingerprints);
		return DNS_VERIFY_ERROR;
	}
#endif

	debug("found %d fingerprints in DNS", fingerprints->rri_nrdatas);

	/* Initialize host key parameters */
	if (!dns_read_key(&hostkey_algorithm, &hostkey_digest_type,
	    &hostkey_digest, &hostkey_digest_len, hostkey)) {
		error("Error calculating host key fingerprint.");
		freerrset(fingerprints);
		return DNS_VERIFY_ERROR;
	}

	for (counter = 0 ; counter < fingerprints->rri_nrdatas ; counter++)  {
		/*
		 * Extract the key from the answer. Ignore any badly
		 * formatted fingerprints.
		 */
		if (!dns_read_rdata(&dnskey_algorithm, &dnskey_digest_type,
		    &dnskey_digest, &dnskey_digest_len,
		    fingerprints->rri_rdatas[counter].rdi_data,
		    fingerprints->rri_rdatas[counter].rdi_length)) {
			verbose("Error parsing fingerprint from DNS.");
			continue;
		}

		/* Check if the current key is the same as the given key */
		if (hostkey_algorithm == dnskey_algorithm &&
		    hostkey_digest_type == dnskey_digest_type) {

			if (hostkey_digest_len == dnskey_digest_len &&
			    memcmp(hostkey_digest, dnskey_digest,
			    hostkey_digest_len) == 0) {

				/* Matching algoritm and digest. */
				freerrset(fingerprints);
				debug("matching host key fingerprint found in DNS");
				return DNS_VERIFY_OK;
			} else {
				/* Correct algorithm but bad digest */
				debug("verify_hostkey_dns: failed");
				failures++;
			}
		}
	}

	freerrset(fingerprints);

	if (failures) {
		error("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		error("@    WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!     @");
		error("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
		error("IT IS POSSIBLE THAT SOMEONE IS DOING SOMETHING NASTY!");
		error("Someone could be eavesdropping on you right now (man-in-the-middle attack)!");
		error("It is also possible that the %s host key has just been changed.",
		    key_type(hostkey));
		error("Please contact your system administrator.");
		return DNS_VERIFY_FAILED;
	}

	debug("fingerprints found in DNS, but none of them matched");

	return DNS_VERIFY_ERROR;
}


/*
 * Export the fingerprint of a key as a DNS resource record
 */
int
export_dns_rr(const char *hostname, Key *key, FILE *f, int generic)
{
	u_int8_t rdata_pubkey_algorithm = 0;
	u_int8_t rdata_digest_type = SSHFP_HASH_SHA1;
	u_char *rdata_digest;
	u_int rdata_digest_len;

	int i;
	int success = 0;

	if (dns_read_key(&rdata_pubkey_algorithm, &rdata_digest_type,
			 &rdata_digest, &rdata_digest_len, key)) {

		if (generic)
			fprintf(f, "%s IN TYPE%d \\# %d %02x %02x ", hostname,
			    DNS_RDATATYPE_SSHFP, 2 + rdata_digest_len,
			    rdata_pubkey_algorithm, rdata_digest_type);
		else
			fprintf(f, "%s IN SSHFP %d %d ", hostname,
			    rdata_pubkey_algorithm, rdata_digest_type);

		for (i = 0; i < rdata_digest_len; i++)
			fprintf(f, "%02x", rdata_digest[i]);
		fprintf(f, "\n");
		success = 1;
	} else {
		error("dns_export_rr: unsupported algorithm");
	}

	return success;
}

#endif /* DNS */
