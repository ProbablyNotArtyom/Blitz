/* User interface for extracting locale-dependent parameters.
   Copyright (C) 1995,1996,1997,1999,2000,2001 Free Software Foundation, Inc.
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

#include <langinfo.h>
#include <locale.h>
#include <errno.h>
#include <stddef.h>
#include "localeinfo.h"


/* Return a string with the data for locale-dependent parameter ITEM.  */

#ifdef USE_IN_EXTENDED_LOCALE_MODEL
char *
__nl_langinfo_l (item, l)
     nl_item item;
     __locale_t l;
#else
char *
nl_langinfo (item)
     nl_item item;
#endif
{
  int category = _NL_ITEM_CATEGORY (item);
  unsigned int index = _NL_ITEM_INDEX (item);
  const struct locale_data *data;

  if (category < 0 || category == LC_ALL || category >= __LC_LAST)
    /* Bogus category: bogus item.  */
    return (char *) "";

#ifdef USE_IN_EXTENDED_LOCALE_MODEL
  data = l->__locales[category];
#else
  data = *_nl_current[category];
#endif

  if (index >= data->nstrings)
    /* Bogus index for this category: bogus item.  */
    return (char *) "";

  /* Return the string for the specified item.  */
  return (char *) data->values[index].string;
}
