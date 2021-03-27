/* Copyright (C) 1991, 1995, 1996, 1998 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <stdio.h>

#ifdef USE_IN_LIBIO
# include <libio/iolibio.h>
# define __vsscanf(s, f, a) _IO_vsscanf (s, f, a)
#endif

/* Read formatted input from S, according to the format string FORMAT.  */
/* VARARGS2 */
int
sscanf (const char *s, const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = __vsscanf (s, format, arg);
  va_end (arg);

  return done;
}

#ifdef USE_IN_LIBIO
# undef _IO_sscanf
/* This is for libg++.  */
strong_alias (sscanf, _IO_sscanf)
#endif
