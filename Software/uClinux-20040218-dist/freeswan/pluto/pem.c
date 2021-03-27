/* Loading of PEM encoded files with optional encryption
 * Copyright (C) 2001-2002 Andreas Steffen, Zuercher Hochschule Winterthur
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
 * RCSID $Id: pem.c,v 1.4 2002/07/08 05:17:42 danield Exp $
 */

/* decrypt a PEM encoded data block using DES-EDE3-CBD
 * see RFC 1423 PEM: Algorithms, Modes and Identifiers
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>

#include <freeswan.h>
#define HEADER_DES_LOCL_H   /* stupid trick to force prototype decl in <des.h> */
#include <des.h>

#include "constants.h"
#include "defs.h"
#include "log.h"
#include "md5.h"
#include "pem.h"

/*
 * check the presence of a pattern in a character string
 */
static bool
present(const char* pattern, chunk_t* ch)
{
    u_int pattern_len = strlen(pattern);

    if (ch->len >= pattern_len && strncmp(ch->ptr, pattern, pattern_len) == 0)
    {
	ch->ptr += pattern_len;
	ch->len -= pattern_len;
	return TRUE;
    }
    return FALSE;
}

/*
 * compare string with chunk
 */
static bool
match(const char *pattern, const chunk_t *ch)
{
    return ch->len == strlen(pattern) &&
	   strncmp(pattern, ch->ptr, ch->len) == 0;
}

/*
 * find a boundary of the form -----tag name-----
 */
static bool
find_boundary(const char* tag, chunk_t *line)
{
    chunk_t name = empty_chunk;

    if (!present("-----", line))
	return FALSE;
    if (!present(tag, line))
	return FALSE;
    if (*line->ptr != ' ')
	return FALSE;
    line->ptr++;  line->len--;

    /* extract name */
    name.ptr = line->ptr;
    while (line->len > 0)
    {
	if (present("-----", line))
	{
	    DBG(DBG_PARSING,
		DBG_log("  -----%s %.*s-----",
			tag, (int)name.len, name.ptr);
	    )
	    return TRUE;
	}
	line->ptr++;  line->len--;  name.len++;
    }
    return FALSE;
}

/*
 * eat whitespace
 */
static void
eat_whitespace(chunk_t *src)
{
    while (src->len > 0 && (*src->ptr == ' ' || *src->ptr == '\t'))
    {
	src->ptr++;  src->len--;
    }
}

/*
 * extracts a token ending with a given termination symbol
 */
static bool
extract_token(chunk_t *token, char termination, chunk_t *src)
{
    u_char *eot = memchr(src->ptr, termination, src->len);

    /* initialize empty token */
    *token = empty_chunk;

    if (eot == NULL) /* termination symbol not found */
	return FALSE;

    /* extract token */
    token->ptr = src->ptr;
    token->len = (u_int)(eot - src->ptr);

    /* advance src pointer after termination symbol */
    src->ptr = eot + 1;
    src->len -= (token->len + 1);

   return TRUE;
}

/*
 * extracts a name: value pair from the PEM header
 */
static bool
extract_parameter(chunk_t *name, chunk_t *value, chunk_t *line)
{
    DBG(DBG_PARSING,
	DBG_log("  %.*s", (int)line->len, line->ptr);
    )

    /* extract name */
    if (!extract_token(name,':', line))
	return FALSE;

    eat_whitespace(line);

    /* extract value */
    *value = *line;
    return TRUE;
}

/*
 *  fetches a new line terminated by \n or \r\n
 */
static bool
fetchline(chunk_t *src, chunk_t *line)
{
    if (src->len == 0) /* end of src reached */
	return FALSE;

    if (extract_token(line, '\n', src))
    {
	if (line->len > 0 && *(line->ptr + line->len -1) == '\r')
	    line->len--;  /* remove optional \r */
    }
    else /*last line ends without newline */
    {
	*line = *src;
	src->ptr += src->len;
	src->len = 0;
    }
    return TRUE;
}

/*
 * decrypts a DES-EDE-CBC encrypted data block
 */
static err_t
pem_decrypt_3des(chunk_t *blob, chunk_t *iv, const char *passphrase)
{
    MD5_CTX context;
    u_char digest[MD5_DIGEST_SIZE];
    u_char key[24];
    des_cblock *deskey = (des_cblock *)key;
    des_key_schedule ks[3];
    u_char padding, *last_padding_pos, *first_padding_pos;

   if (iv->len != DES_CBC_BLOCK_SIZE)
	return "size of DES-EDE3-CBC IV is not 8 bytes";

    /* Convert passphrase to 3des key */
    MD5Init(&context);
    MD5Update(&context, passphrase, strlen(passphrase));
    MD5Update(&context, iv->ptr, iv->len);
    MD5Final(digest, &context);

    memcpy(key, digest, MD5_DIGEST_SIZE);

    MD5Init(&context);
    MD5Update(&context, digest, MD5_DIGEST_SIZE);
    MD5Update(&context, passphrase, strlen(passphrase));
    MD5Update(&context, iv->ptr, iv->len);
    MD5Final(digest, &context);

    memcpy(key + MD5_DIGEST_SIZE, digest, 24 - MD5_DIGEST_SIZE);

    (void) des_set_key(&deskey[0], ks[0]);
    (void) des_set_key(&deskey[1], ks[1]);
    (void) des_set_key(&deskey[2], ks[2]);

    /* decrypt data block */
    des_ede3_cbc_encrypt((des_cblock *)blob->ptr, (des_cblock *)blob->ptr,
	blob->len, ks[0], ks[1], ks[2], (des_cblock *)iv->ptr, FALSE);

    /* determine amount of padding */
    last_padding_pos = blob->ptr + blob->len - 1;
    padding = *last_padding_pos;
    first_padding_pos = (padding > blob->len)?
			 blob->ptr : last_padding_pos - padding;

    /* check the padding pattern */
    while (--last_padding_pos > first_padding_pos)
    {
	if (*last_padding_pos != padding)
	    return "wrong padding after decryption - please check your passphrase";
    }

    /* remove padding */
    blob->len -= padding;
    return NULL;
}

 /*  Converts a PEM encoded file into its binary form
 *
 *  RFC 1421 Privacy Enhancement for Electronic Mail, February 1993
 *  RFC 934 Message Encapsulation, January 1985
 */
err_t
pemtobin(chunk_t *blob, const char* passphrase)
{
    typedef enum {
	PEM_PRE    = 0,
	PEM_MSG    = 1,
	PEM_HEADER = 2,
	PEM_BODY   = 3,
	PEM_POST   = 4,
	PEM_ABORT  = 5
    } state_t;

    bool encrypted = FALSE;

    state_t state  = PEM_PRE;

    chunk_t src    = *blob;
    chunk_t dst    = *blob;
    chunk_t line   = empty_chunk;
    chunk_t iv     = empty_chunk;

    u_char iv_buf[MAX_DIGEST_LEN];

    /* zero size of converted blob */
    dst.len = 0;

    /* zero size of IV */
    iv.ptr = iv_buf;
    iv.len = 0;

    while (fetchline(&src, &line))
    {
	if (state == PEM_PRE)
	{
	    if (find_boundary("BEGIN", &line))
		state = PEM_MSG;
	    continue;
	}
	else
	{
	    if (find_boundary("END", &line))
	    {
		state = PEM_POST;
		break;
	    }
	    if (state == PEM_MSG)
	    {
		state = (memchr(line.ptr, ':', line.len) == NULL)?
			    PEM_BODY : PEM_HEADER;
	    }
	    if (state == PEM_HEADER)
	    {
		chunk_t name  = empty_chunk;
		chunk_t value = empty_chunk;

		/* an empty line separates HEADER and BODY */
		if (line.len == 0)
		{
		    state = PEM_BODY;
		    continue;
		}

	        /* we are looking for a name: value pair */
		if (!extract_parameter(&name, &value, &line))
		    continue;

		if (match("Proc-Type", &name) && *value.ptr == '4')
			encrypted = TRUE;
		else if (match("DEK-Info", &name))
		{
		    char *ugh = NULL;
		    int len = 0;
		    chunk_t dek;

		    if (!extract_token(&dek, ',', &value))
			dek = value;

		    /* we support DES-EDE3-CBC encrypted files, only */
		    if (!match("DES-EDE3-CBC", &dek))
			return "we support DES-EDE3-CBC encrypted files, only";

		    eat_whitespace(&value);
		    ugh = ttodata(value.ptr, value.len, 16,
		    		  iv.ptr, MAX_DIGEST_LEN, &len);
		    if (ugh)
			return "error in IV";

		    iv.len = len;
		}
	    }
	    else /* state is PEM_BODY */
	    {
		char *ugh = NULL;
		int len = 0;
		chunk_t data;

		/* remove any trailing whitespace */
		if (!extract_token(&data ,' ', &line))
		    data = line;

		ugh = ttodata(data.ptr, data.len, 64,
			      dst.ptr, blob->len - dst.len, &len);
		if (ugh)
		{
		    DBG(DBG_PARSING,
			DBG_log("  %s", ugh);
		    )
		    state = PEM_ABORT;
		    break;
		}
		else
		{
		    dst.ptr += len;
		    dst.len += len;
		}
	    }
	}
    }
    /* set length to size of binary blob */
    blob->len = dst.len;

    if (state != PEM_POST)
#if 0
	return "file coded in unknown format, discarded";
#endif
	return " ";
	
    if (encrypted)
    {
	DBG(DBG_CRYPT,
	    DBG_log("  decrypting file using 'DES-EDE3-CBC'");
	)
	if (strlen(passphrase) == 0)
	    return "no passphrase available";

	return pem_decrypt_3des(blob, &iv, passphrase);
    }
    else
	return NULL;
}
