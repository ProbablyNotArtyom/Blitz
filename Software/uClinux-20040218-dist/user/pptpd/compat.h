/*
 * compat.h
 *
 * Compatibility functions for different OSes (prototypes)
 *
 * $Id: compat.h,v 1.1.1.1 1999/11/22 03:48:02 christ Exp $
 */

#ifndef _PPTPD_COMPAT_H
#define _PPTPD_COMPAT_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>

#ifndef HAVE_STRLCPY
/* void since to be fast and portable, we use strncpy, but this
 * means we don't know how many bytes were copied
 */
extern void strlcpy(char *dst, const char *src, size_t size);
#endif	/* !HAVE_STRLCPY */

#ifndef HAVE_MEMMOVE
extern void *memmove(void *dst, const void *src, size_t size);
#endif	/* !HAVE_MEMMOVE */

#ifndef HAVE_OPENPTY
/* Originally from code by C. S. Ananian */

/* These are the Linux values - and fairly sane defaults.
 * Since we search from the start and just skip errors, they'll do.
 * Note that Unix98 has an openpty() call so we don't need to worry
 * about the new pty names here.
 */
#define PTYDEV		"/dev/ptyxx"
#define TTYDEV		"/dev/ttyxx"
#define PTYMAX		11
#define TTYMAX		11
#define PTYCHAR1	"pqrstuvwxyzabcde"
#define PTYCHAR2	"0123456789abcdef"

/* Dummy the last 2 args, so we don't have to find the right include
 * files on every OS to define the needed structures.
 */
extern int openpty(int *, int *, char *, void *, void *);
#endif	/* !HAVE_OPENPTY */

#ifndef HAVE_STRERROR
extern char *strerror(int);
#endif

#endif	/* !_PPTPD_COMPAT_H */
