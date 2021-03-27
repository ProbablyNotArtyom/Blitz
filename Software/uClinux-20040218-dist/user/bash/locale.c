/* locale.c - Miscellaneous internationalization functions. */

/* Copyright (C) 1996 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#include "config.h"

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashintl.h"
#include "bashansi.h"
#include <stdio.h>
#include <ctype.h>

#include "shell.h"

/* The current locale when the program begins */
static char *default_locale;

/* The current domain for textdomain(3). */
static char *default_domain;
static char *default_dir;

/* tracks the value of LC_ALL; used to override values for other locale
   categories */
static char *lc_all;

/* Set the value of default_locale and make the current locale the
   system default locale.  This should be called very early in main(). */
void
set_default_locale ()
{
#if defined (HAVE_SETLOCALE)
  default_locale = setlocale (LC_ALL, "");
  if (default_locale)
    default_locale = savestring (default_locale);
#endif /* HAVE_SETLOCALE */
}

/* Set default values for LC_CTYPE, LC_COLLATE, and LC_MESSAGES if they
   are not specified in the environment, but LANG or LC_ALL is.  This
   should be called from main() after parsing the environment. */
void
set_default_locale_vars ()
{
  char *val;

#if defined (HAVE_SETLOCALE)
  val = get_string_value ("LC_CTYPE");
  if (val == 0 && lc_all && *lc_all)
    setlocale (LC_CTYPE, lc_all);

#  if defined (LC_COLLATE)
  val = get_string_value ("LC_COLLATE");
  if (val == 0 && lc_all && *lc_all)
    setlocale (LC_COLLATE, lc_all);
#  endif /* LC_COLLATE */

#  if defined (LC_MESSAGES)
  val = get_string_value ("LC_MESSAGES");
  if (val == 0 && lc_all && *lc_all)
    setlocale (LC_MESSAGES, lc_all);
#  endif /* LC_MESSAGES */

#  if defined (LC_NUMERIC)
  val = get_string_value ("LC_NUMERIC");
  if (val == 0 && lc_all && *lc_all)
    setlocale (LC_NUMERIC, lc_all);
#  endif /* LC_NUMERIC */

#endif /* HAVE_SETLOCALE */

  val = get_string_value ("TEXTDOMAIN");
  if (val && *val)
    {
      FREE (default_domain);
      default_domain = savestring (val);
      textdomain (default_domain);
    }

  val = get_string_value ("TEXTDOMAINDIR");
  if (val && *val)
    {
      FREE (default_dir);
      default_dir = savestring (val);
      bindtextdomain (default_domain, default_dir);
    }
}

/* Set one of the locale categories (specified by VAR) to VALUE.  Returns 1
  if successful, 0 otherwise. */
int
set_locale_var (var, value)
     char *var, *value;
{
  if (var[0] == 'T' && var[10] == 0)		/* TEXTDOMAIN */
    {
      FREE (default_domain);
      default_domain = value ? savestring (value) : (char *)NULL;
      textdomain (default_domain);
      return (1);
    }
  else if (var[0] == 'T')			/* TEXTDOMAINDIR */
    {
      FREE (default_dir);
      default_dir = value ? savestring (value) : (char *)NULL;
      bindtextdomain (default_domain, default_dir);
      return (1);
    }

  /* var[0] == 'L' && var[1] == 'C' && var[2] == '_' */

  else if (var[3] == 'A')			/* LC_ALL */
    {
      FREE (lc_all);
      if (value)
	lc_all = savestring (value);
      else if (default_locale)
	lc_all = savestring (default_locale);
      else
	{
	  lc_all = xmalloc (1);
	  lc_all[0] = '\0';
	}
#if defined (HAVE_SETLOCALE)
      return (setlocale (LC_ALL, lc_all) != 0);
#else
      return (1);
#endif
    }

#if defined (HAVE_SETLOCALE)
  else if (var[3] == 'C' && var[4] == 'T')	/* LC_CTYPE */
    {
      if (lc_all == 0 || *lc_all == '\0')
	return (setlocale (LC_CTYPE, value ? value : "") != 0);
    }
  else if (var[3] == 'C' && var[4] == 'O')	/* LC_COLLATE */
    {
#  if defined (LC_COLLATE)
      if (lc_all == 0 || *lc_all == '\0')
	return (setlocale (LC_COLLATE, value ? value : "") != 0);
#  endif /* LC_COLLATE */
    }
  else if (var[3] == 'M' && var[4] == 'E')	/* LC_MESSAGES */
    {
#  if defined (LC_MESSAGES)
      if (lc_all == 0 || *lc_all == '\0')
	return (setlocale (LC_MESSAGES, value ? value : "") != 0);
#  endif /* LC_MESSAGES */
    }
  else if (var[3] = 'N' && var[4] == 'U')	/* LC_NUMERIC */
    {
#  if defined (LC_NUMERIC)
      if (lc_all == 0 || *lc_all == '\0')
	return (setlocale (LC_NUMERIC, value ? value : "") != 0);
#  endif /* LC_NUMERIC */
    }
#endif /* HAVE_SETLOCALE */

  return (0);
}

#if 0
/* Called when LANG is assigned a value.  Sets LC_ALL if that has not
   already been set. */
#else
/* This no longer does anything; we rely on the C library for correct
   behavior. */
#endif
int
set_lang (var, value)
     char *var, *value;
{
#if 0
  return ((lc_all == 0) ? set_locale_var ("LC_ALL", value) : 0);
#else
  return 0;
#endif
}

/* Get the value of one of the locale variables (LC_MESSAGES, LC_CTYPE) */
char *
get_locale_var (var)
     char *var;
{
  char *locale;

  locale = lc_all;

  if (locale == 0)
    locale = get_string_value (var);
  if (locale == 0)
    locale = default_locale;

  return (locale);
}

/* Translate the contents of STRING, a $"..." quoted string, according
   to the current locale.  In the `C' or `POSIX' locale, or if gettext()
   is not available, the passed string is returned unchanged.  The
   length of the translated string is returned in LENP, if non-null. */
char *
localetrans (string, len, lenp)
     char *string;
     int len, *lenp;
{
  char *locale, *t;
#if defined (HAVE_GETTEXT)
  char *translated;
  int tlen;
#endif

  /* Don't try to translate null strings. */
  if (string == 0 || *string == 0)
    {
      if (lenp)
	*lenp = 0;
      return ((char *)NULL);
    }

  locale = get_locale_var ("LC_MESSAGES");

  /* If we don't have setlocale() or the current locale is `C' or `POSIX',
     just return the string.  If we don't have gettext(), there's no use
     doing anything else. */
#if defined (HAVE_GETTEXT)
  if (locale == 0 || locale[0] == '\0' ||
      (locale[0] == 'C' && locale[1] == '\0') || STREQ (locale, "POSIX"))
#endif
    {
      t = xmalloc (len + 1);
      strcpy (t, string);
      if (lenp)
	*lenp = len;
      return (t);
    }

#if defined (HAVE_GETTEXT)
  /* Now try to translate it. */
  translated = gettext (string);
  if (translated == string)	/* gettext returns its argument if untranslatable */
    {
      t = xmalloc (len + 1);
      strcpy (t, string);
      if (lenp)
	*lenp = len;
    }
  else
    {
      tlen = strlen (translated);
      t = xmalloc (tlen + 1);
      strcpy (t, translated);
      if (lenp)
	*lenp = tlen;
    }
  return (t);
#endif /* HAVE_GETTEXT */
}
