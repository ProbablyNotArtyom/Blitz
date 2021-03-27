/* misc. universal things
 * Copyright (C) 1997 Angelos D. Keromytis.
 * Copyright (C) 1998-2001  D. Hugh Redelmeier.
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
 * RCSID $Id: defs.h,v 1.31 2002/03/15 22:30:14 dhr Exp $
 */

/* GCC magic! */
#ifdef GCC_LINT
# define PRINTF_LIKE(n) __attribute__ ((format(printf, n, n+1)))
# define NEVER_RETURNS __attribute__ ((noreturn))
# define UNUSED __attribute__ ((unused))
# define BLANK_FORMAT " "	/* GCC_LINT whines about empty formats */
#else
# define PRINTF_LIKE(n)	/* ignore */
# define NEVER_RETURNS /* ignore */
# define UNUSED /* ignore */
# define BLANK_FORMAT ""
#endif

#ifdef KLIPS
# define USED_BY_KLIPS	/* ignore */
#else
# define USED_BY_KLIPS	UNUSED
#endif

#ifdef DEBUG
# define USED_BY_DEBUG	/* ignore */
#else
# define USED_BY_DEBUG	UNUSED
#endif

/* type of serial number of a state object
 * Needed in connections.h and state.h; here to simplify dependencies.
 */
typedef unsigned long so_serial_t;
#define SOS_NOBODY	0	/* null serial number */
#define SOS_FIRST	1	/* first normal serial number */

/* memory allocation */

extern void *alloc_bytes(size_t size, const char *name);
#define alloc_thing(thing, name) (alloc_bytes(sizeof(thing), (name)))

extern void *clone_bytes(const void *orig, size_t size, const char *name);
#define clone_thing(orig, name) clone_bytes((const void *)&(orig), sizeof(orig), (name))
#define clone_str(str, name) \
    ((str) == NULL? NULL : clone_bytes((str), strlen((str))+1, (name)))

#ifdef LEAK_DETECTIVE
  extern void pfree(void *ptr);
  extern void report_leaks(void);
#else
# define pfree(ptr) free(ptr)	/* ordinary stdc free */
#endif
#define pfreeany(p) { if ((p) != NULL) pfree(p); }
#define replace(p, q) { pfreeany(p); (p) = (q); }


/* chunk is a simple pointer-and-size abstraction */

struct chunk {
    u_char *ptr;
    size_t len;
    };
typedef struct chunk chunk_t;

#define setchunk(ch, addr, size) { (ch).ptr = (addr); (ch).len = (size); }
/* NOTE: freeanychunk, unlike pfreeany, NULLs .ptr */
#define freeanychunk(ch) { pfreeany((ch).ptr); (ch).ptr = NULL; }
#define clonetochunk(ch, addr, size, name) \
    { (ch).ptr = clone_bytes((addr), (ch).len = (size), name); }
#define clonereplacechunk(ch, addr, size, name) \
    { pfreeany((ch).ptr); clonetochunk(ch, addr, size, name); }
#define chunkcpy(dst, chunk) \
    { memcpy(dst, chunk.ptr, chunk.len); dst += chunk.len;}

extern const chunk_t empty_chunk;

/* display a date either in local or UTC time */
extern char* timetoa(const time_t *time, bool utc);

/* warns a predefined interval before expiry */
extern const char* check_expiry(time_t expiration_date,
    int warning_interval, bool strict);

/* no time defined in time_t */
#define UNDEFINED_TIME	0

/* size of timetoa string buffer */
#define TIMETOA_BUF	30

#define NO_IP	0	/* our s_addr value signifying no IPv4 address */

#define is_NO_IP(a) ((a).u.v4.sin_addr.s_addr == NO_IP)

/* cleanly exit Pluto */

extern void exit_pluto(int /*status*/) NEVER_RETURNS;


/* zero all bytes */
#define zero(x) memset((x), '\0', sizeof(*(x)))

/* are all bytes 0? */
extern bool all_zero(const unsigned char *m, size_t len);


/* some MP utilities */

#include <gmp.h>

extern void n_to_mpz(MP_INT *mp, const u_char *nbytes, size_t nlen);
extern chunk_t mpz_to_n(const MP_INT *mp, size_t bytes);

/* var := mod(base ** exp, mod), ensuring var is mpz_inited */
#define mpz_init_powm(flag, var, base, exp, mod) { \
    if (!(flag)) \
	mpz_init(&(var)); \
    (flag) = TRUE; \
    mpz_powm(&(var), &(base), &(exp), (mod)); \
    }


/* pad_up(n, m) is the amount to add to n to make it a multiple of m */
#define pad_up(n, m) (((m) - 1) - (((n) + (m) - 1) % (m)))
