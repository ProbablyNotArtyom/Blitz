/*
 * addresses to text
 * Copyright (C) 2000  Henry Spencer.
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/lgpl.txt>.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * RCSID $Id: addrtot.c,v 1.8 2000/09/12 04:56:39 henry Exp $
 */
#include "internal.h"
#include "freeswan.h"

#define	IP4BYTES	4	/* bytes in an IPv4 address */
#define	PERBYTE		4	/* three digits plus a dot or NUL */
#define	IP6BYTES	16	/* bytes in an IPv6 address */

/* forwards */
static size_t normal4(const unsigned char *s, size_t len, char *b, char **dp);
static size_t normal6(const unsigned char *s, size_t len, char *b, char **dp);
static size_t reverse4(const unsigned char *s, size_t len, char *b, char **dp);
static size_t reverse6(const unsigned char *s, size_t len, char *b, char **dp);
static size_t reverse62(const unsigned char *s, size_t len, char *b, char **dp);

/*
 - addrtot - convert binary address to text (dotted decimal or IPv6 string)
 */
size_t				/* space needed for full conversion */
addrtot(src, format, dst, dstlen)
const ip_address *src;
int format;			/* character */
char *dst;			/* need not be valid if dstlen is 0 */
size_t dstlen;
{
	const unsigned char *b;
	size_t n;
	char buf[1+ADDRTOT_BUF+1];	/* :address: */
	char *p;
	int t = addrtypeof(src);
#	define	TF(t, f)	(((t)<<8) | (f))

	n = addrbytesptr(src, &b);
	if (n == 0)
		return 0;

	switch (TF(t, format)) {
	case TF(AF_INET, 0):
		n = normal4(b, n, buf, &p);
		break;
	case TF(AF_INET6, 0):
		n = normal6(b, n, buf, &p);
		break;
	case TF(AF_INET, 'r'):
		n = reverse4(b, n, buf, &p);
		break;
	case TF(AF_INET6, 'r'):
		n = reverse6(b, n, buf, &p);
		break;
	case TF(AF_INET6, 'R'):
		n = reverse62(b, n, buf, &p);
		break;
	default:		/* including (AF_INET, 'R') */
		return 0;
		break;
	}

	if (dstlen > 0) {
		if (dstlen < n)
			p[dstlen - 1] = '\0';
		strcpy(dst, p);
	}
	return n;
}

/*
 - normal4 - normal IPv4 address-text conversion
 */
static size_t			/* size of text, including NUL */
normal4(srcp, srclen, buf, dstp)
const unsigned char *srcp;
size_t srclen;
char *buf;			/* guaranteed large enough */
char **dstp;			/* where to put result pointer */
{
	int i;
	char *p;

	if (srclen != IP4BYTES)	/* "can't happen" */
		return 0;
	p = buf;
	for (i = 0; i < IP4BYTES; i++) {
		p += ultot(srcp[i], 10, p, PERBYTE);
		if (i != IP4BYTES - 1)
			*(p-1) = '.';	/* overwrites the NUL */
	}
	*dstp = buf;
	return p - buf;
}

/*
 - normal6 - normal IPv6 address-text conversion
 */
static size_t			/* size of text, including NUL */
normal6(srcp, srclen, buf, dstp)
const unsigned char *srcp;
size_t srclen;
char *buf;			/* guaranteed large enough, plus 2 */
char **dstp;			/* where to put result pointer */
{
	int i;
	unsigned long piece;
	char *p;
	char *q;

	if (srclen != IP6BYTES)	/* "can't happen" */
		return 0;
	p = buf;
	*p++ = ':';
	for (i = 0; i < IP6BYTES/2; i++) {
		piece = (srcp[2*i] << 8) + srcp[2*i + 1];
		p += ultot(piece, 16, p, 5);	/* 5 = abcd + NUL */
		*(p-1) = ':';	/* overwrites the NUL */
	}
	*p = '\0';
	q = strstr(buf, ":0:0:");
	if (q != NULL) {	/* zero squishing is possible */
		p = q + 1;
		while (*p == '0' && *(p+1) == ':')
			p += 2;
		q++;
		*q++ = ':';	/* overwrite first 0 */
		while (*p != '\0')
			*q++ = *p++;
		*q = '\0';
		if (!(*(q-1) == ':' && *(q-2) == ':'))
			*--q = '\0';	/* strip final : unless :: */
		p = buf;
		if (!(*p == ':' && *(p+1) == ':'))
			p++;	/* skip initial : unless :: */
	} else {
		q = p;
		*--q = '\0';	/* strip final : */
		p = buf + 1;	/* skip initial : */
	}
	*dstp = p;
	return q - p + 1;
}

/*
 - reverse4 - IPv4 reverse-lookup conversion
 */
static size_t			/* size of text, including NUL */
reverse4(srcp, srclen, buf, dstp)
const unsigned char *srcp;
size_t srclen;
char *buf;			/* guaranteed large enough */
char **dstp;			/* where to put result pointer */
{
	int i;
	char *p;

	if (srclen != IP4BYTES)	/* "can't happen" */
		return 0;
	p = buf;
	for (i = IP4BYTES-1; i >= 0; i--) {
		p += ultot(srcp[i], 10, p, PERBYTE);
		*(p-1) = '.';	/* overwrites the NUL */
	}
	strcpy(p, "IN-ADDR.ARPA.");
	*dstp = buf;
	return strlen(buf) + 1;
}

/*
 - reverse62 - obsolete IPv6 reverse-lookup conversion (RFC 1886)
 * A trifle inefficient, really shouldn't use ultot...
 */
static size_t			/* size of text, including NUL */
reverse62(srcp, srclen, buf, dstp)
const unsigned char *srcp;
size_t srclen;
char *buf;			/* guaranteed large enough */
char **dstp;			/* where to put result pointer */
{
	int i;
	unsigned long piece;
	char *p;

	if (srclen != IP6BYTES)	/* "can't happen" */
		return 0;
	p = buf;
	for (i = IP6BYTES-1; i >= 0; i--) {
		piece = srcp[i];
		p += ultot(piece&0xf, 16, p, 2);
		*(p-1) = '.';
		p += ultot(piece>>4, 16, p, 2);
		*(p-1) = '.';
	}
	strcpy(p, "IP6.INT.");
	*dstp = buf;
	return strlen(buf) + 1;
}

/*
 - reverse6 - modern IPv6 reverse-lookup conversion (RFC 2874)
 */
static size_t			/* size of text, including NUL */
reverse6(srcp, srclen, buf, dstp)
const unsigned char *srcp;
size_t srclen;
char *buf;			/* guaranteed large enough */
char **dstp;			/* where to put result pointer */
{
	char *p;

	if (srclen != IP6BYTES)	/* "can't happen" */
		return 0;
	strcpy(buf, "\\[x");
	p = buf + strlen(buf);
	(void) datatot((const char *)srcp, srclen, 16, p, IP6BYTES*2 + 1);
	strcat(buf, "].IP6.ARPA.");	/* leave count implicit */
	*dstp = buf;
	return strlen(buf) + 1;
}
