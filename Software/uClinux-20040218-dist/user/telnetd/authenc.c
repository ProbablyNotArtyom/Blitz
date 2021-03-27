/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef EMBED
/*
 * From: @(#)authenc.c	5.1 (Berkeley) 3/1/91
 */
char authenc_rcsid[] =
  "$Id: authenc.c,v 1.2 2003/07/21 03:11:25 davidm Exp $";
#endif

#if	defined(ENCRYPT) || defined(AUTHENTICATE)
#include "telnetd.h"
#include <libtelnet/misc.h>

int
net_write(str, len)
    unsigned char *str;
    int len;
{
    if (nfrontp + len < netobuf + BUFSIZ) {
	bcopy((void *)str, (void *)nfrontp, len);
	nfrontp += len;
	return(len);
    }
    return(0);
}

void
net_encrypt()
{
#if	defined(ENCRYPT)
    char *s = (nclearto > nbackp) ? nclearto : nbackp;
    if (s < nfrontp && encrypt_output) {
	(*encrypt_output)((unsigned char *)s, nfrontp - s);
    }
    nclearto = nfrontp;
#endif
}

int
telnet_spin()
{
    ttloop();
    return(0);
}

char *
telnet_getenv(val)
    char *val;
{
    extern char *getenv();
    return(getenv(val));
}

char *
telnet_gets(prompt, result, length, echo)
    char *prompt;
    char *result;
    int length;
    int echo;
{
    return((char *)0);
}
#endif
