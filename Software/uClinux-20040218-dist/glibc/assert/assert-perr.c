/* Copyright (C) 1994,1995,1996,1997,1998,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <assert.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdep.h>


extern const char *__progname;

#ifdef USE_IN_LIBIO
# include <wchar.h>
# include <libio/iolibio.h>
# define fflush(s) _IO_fflush (s)
#endif

/* This function, when passed an error number, a filename, and a line
   number, prints a message on the standard error stream of the form:
   	a.c:10: foobar: Unexpected error: Computer bought the farm
   It then aborts program execution via a call to `abort'.  */

#ifdef FATAL_PREPARE_INCLUDE
# include FATAL_PREPARE_INCLUDE
#endif

void
__assert_perror_fail (int errnum,
		      const char *file, unsigned int line,
		      const char *function)
{
  char errbuf[1024];
  char *buf;

#ifdef FATAL_PREPARE
  FATAL_PREPARE;
#endif

  (void) __asprintf (&buf, _("%s%s%s:%u: %s%sUnexpected error: %s.\n"),
		     __progname, __progname[0] ? ": " : "",
		     file, line,
		     function ? function : "", function ? ": " : "",
		     __strerror_r (errnum, errbuf, sizeof errbuf));

  /* Print the message.  */
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", buf);
  else
#endif
    (void) fputs (buf, stderr);

  (void) fflush (stderr);

  /* We have to free the buffer since the appplication might catch the
     SIGABRT.  */
  free (buf);

  abort ();
}
