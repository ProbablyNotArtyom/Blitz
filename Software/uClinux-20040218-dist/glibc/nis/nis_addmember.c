/* Copyright (c) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#include <string.h>
#include <rpcsvc/nis.h>

nis_error
nis_addmember (const_nis_name member, const_nis_name group)
{
  if (group != NULL && group[0] != '\0')
    {
      size_t grouplen = strlen (group);
      char buf[grouplen + 14 + NIS_MAXNAMELEN];
      char leafbuf[grouplen + 2];
      char domainbuf[grouplen + 2];
      nis_result *res, *res2;
      nis_error status;
      char *cp, *cp2;

      cp = stpcpy (buf, nis_leaf_of_r (group, leafbuf, sizeof (leafbuf) - 1));
      cp = stpcpy (cp, ".groups_dir");
      cp2 = nis_domain_of_r (group, domainbuf, sizeof (domainbuf) - 1);
      if (cp2 != NULL && cp2[0] != '\0')
        {
	  *cp++ = '.';
          stpcpy (cp, cp2);
        }
      res = nis_lookup (buf, FOLLOW_LINKS|EXPAND_NAME);
      if (NIS_RES_STATUS (res) != NIS_SUCCESS)
	{
	  status = NIS_RES_STATUS (res);
	  nis_freeresult (res);
	  return status;
	}
      if ((NIS_RES_NUMOBJ (res)  != 1) ||
          (__type_of (NIS_RES_OBJECT (res)) != NIS_GROUP_OBJ))
	{
	  nis_freeresult (res);
	  return NIS_INVALIDOBJ;
	}

      NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_val
	= realloc (NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_val,
		   (NIS_RES_OBJECT(res)->GR_data.gr_members.gr_members_len + 1)
		   * sizeof (char *));
      if (NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_val == NULL)
	goto nomem_out;
      NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_val[NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_len] = strdup (member);
      if (NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_val[NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_len] == NULL)
	{
	  free (NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_val);
	nomem_out:
	  nis_freeresult (res);
	  return NIS_NOMEMORY;
	}
      ++NIS_RES_OBJECT (res)->GR_data.gr_members.gr_members_len;

      cp = stpcpy (buf, NIS_RES_OBJECT(res)->zo_name);
      *cp++ = '.';
      strncpy (cp, NIS_RES_OBJECT (res)->zo_domain, NIS_MAXNAMELEN);
      res2 = nis_modify (buf, NIS_RES_OBJECT (res));
      status = NIS_RES_STATUS (res2);
      nis_freeresult (res);
      nis_freeresult (res2);

      return status;
    }
  else
    return NIS_FAIL;
}
