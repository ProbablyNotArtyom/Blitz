/*	$OpenBSD: auth2-gss.c,v 1.3 2003/09/01 20:44:54 markus Exp $	*/

/*
 * Copyright (c) 2001-2003 Simon Wilkinson. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AS IS'' AND ANY EXPRESS OR
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

#ifdef GSSAPI

#include "auth.h"
#include "ssh2.h"
#include "xmalloc.h"
#include "log.h"
#include "dispatch.h"
#include "servconf.h"
#include "compat.h"
#include "packet.h"
#include "monitor_wrap.h"

#include "ssh-gss.h"

extern ServerOptions options;

static void input_gssapi_token(int type, u_int32_t plen, void *ctxt);
static void input_gssapi_exchange_complete(int type, u_int32_t plen, void *ctxt);
static void input_gssapi_errtok(int, u_int32_t, void *);

/*
 * We only support those mechanisms that we know about (ie ones that we know
 * how to check local user kuserok and the like
 */
static int
userauth_gssapi(Authctxt *authctxt)
{
	gss_OID_desc oid = {0, NULL};
	Gssctxt *ctxt = NULL;
	int mechs;
	gss_OID_set supported;
	int present;
	OM_uint32 ms;
	u_int len;
	char *doid = NULL;

	if (!authctxt->valid || authctxt->user == NULL)
		return (0);

	mechs = packet_get_int();
	if (mechs == 0) {
		debug("Mechanism negotiation is not supported");
		return (0);
	}

	ssh_gssapi_supported_oids(&supported);
	do {
		mechs--;

		if (doid)
			xfree(doid);

		doid = packet_get_string(&len);

		if (doid[0] != SSH_GSS_OIDTYPE || doid[1] != len-2) {
			logit("Mechanism OID received using the old encoding form");
			oid.elements = doid;
			oid.length = len;
		} else {
			oid.elements = doid + 2;
			oid.length   = len - 2;
		}
		gss_test_oid_set_member(&ms, &oid, supported, &present);
	} while (mechs > 0 && !present);

	gss_release_oid_set(&ms, &supported);

	if (!present) {
		xfree(doid);
		return (0);
	}

	if (GSS_ERROR(PRIVSEP(ssh_gssapi_server_ctx(&ctxt, &oid)))) {
		xfree(doid);
		return (0);
	}

	authctxt->methoddata=(void *)ctxt;

	packet_start(SSH2_MSG_USERAUTH_GSSAPI_RESPONSE);

	/* Return OID in same format as we received it*/
	packet_put_string(doid, len);

	packet_send();
	xfree(doid);

	dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_TOKEN, &input_gssapi_token);
	dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_ERRTOK, &input_gssapi_errtok);
	authctxt->postponed = 1;

	return (0);
}

static void
input_gssapi_token(int type, u_int32_t plen, void *ctxt)
{
	Authctxt *authctxt = ctxt;
	Gssctxt *gssctxt;
	gss_buffer_desc send_tok = GSS_C_EMPTY_BUFFER;
	gss_buffer_desc recv_tok;
	OM_uint32 maj_status, min_status;
	u_int len;

	if (authctxt == NULL || (authctxt->methoddata == NULL && !use_privsep))
		fatal("No authentication or GSSAPI context");

	gssctxt = authctxt->methoddata;
	recv_tok.value = packet_get_string(&len);
	recv_tok.length = len; /* u_int vs. size_t */

	packet_check_eom();

	maj_status = PRIVSEP(ssh_gssapi_accept_ctx(gssctxt, &recv_tok,
	    &send_tok, NULL));

	xfree(recv_tok.value);

	if (GSS_ERROR(maj_status)) {
		if (send_tok.length != 0) {
			packet_start(SSH2_MSG_USERAUTH_GSSAPI_ERRTOK);
			packet_put_string(send_tok.value, send_tok.length);
			packet_send();
		}
		authctxt->postponed = 0;
		dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_TOKEN, NULL);
		userauth_finish(authctxt, 0, "gssapi");
	} else {
		if (send_tok.length != 0) {
			packet_start(SSH2_MSG_USERAUTH_GSSAPI_TOKEN);
			packet_put_string(send_tok.value, send_tok.length);
			packet_send();
		}
		if (maj_status == GSS_S_COMPLETE) {
			dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_TOKEN, NULL);
			dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE,
				     &input_gssapi_exchange_complete);
		}
	}

	gss_release_buffer(&min_status, &send_tok);
}

static void
input_gssapi_errtok(int type, u_int32_t plen, void *ctxt)
{
	Authctxt *authctxt = ctxt;
	Gssctxt *gssctxt;
	gss_buffer_desc send_tok = GSS_C_EMPTY_BUFFER;
	gss_buffer_desc recv_tok;
	OM_uint32 maj_status;
	u_int len;

	if (authctxt == NULL || (authctxt->methoddata == NULL && !use_privsep))
		fatal("No authentication or GSSAPI context");

	gssctxt = authctxt->methoddata;
	recv_tok.value = packet_get_string(&len);
	recv_tok.length = len;

	packet_check_eom();

	/* Push the error token into GSSAPI to see what it says */
	maj_status = PRIVSEP(ssh_gssapi_accept_ctx(gssctxt, &recv_tok,
	    &send_tok, NULL));

	xfree(recv_tok.value);

	/* We can't return anything to the client, even if we wanted to */
	dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_TOKEN, NULL);
	dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_ERRTOK, NULL);

	/* The client will have already moved on to the next auth */

	gss_release_buffer(&maj_status, &send_tok);
}

/*
 * This is called when the client thinks we've completed authentication.
 * It should only be enabled in the dispatch handler by the function above,
 * which only enables it once the GSSAPI exchange is complete.
 */

static void
input_gssapi_exchange_complete(int type, u_int32_t plen, void *ctxt)
{
	Authctxt *authctxt = ctxt;
	Gssctxt *gssctxt;
	int authenticated;

	if (authctxt == NULL || (authctxt->methoddata == NULL && !use_privsep))
		fatal("No authentication or GSSAPI context");

	gssctxt = authctxt->methoddata;

	/*
	 * We don't need to check the status, because the stored credentials
	 * which userok uses are only populated once the context init step
	 * has returned complete.
	 */

	packet_check_eom();

	authenticated = PRIVSEP(ssh_gssapi_userok(authctxt->user));

	authctxt->postponed = 0;
	dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_TOKEN, NULL);
	dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_ERRTOK, NULL);
	dispatch_set(SSH2_MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE, NULL);
	userauth_finish(authctxt, authenticated, "gssapi");
}

Authmethod method_gssapi = {
	"gssapi",
	userauth_gssapi,
	&options.gss_authentication
};

#endif /* GSSAPI */
