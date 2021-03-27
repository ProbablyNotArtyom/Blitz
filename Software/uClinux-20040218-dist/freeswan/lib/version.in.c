/*
 * return IPsec version information
 * Copyright (C) 2001  Henry Spencer.
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
 * RCSID $Id: version.in.c,v 1.1 2001/11/22 03:29:41 henry Exp $
 */
#include "internal.h"
#include "freeswan.h"

#define	V	"xxx"		/* substituted in by Makefile */
static const char freeswan_number[] = V;
static const char freeswan_string[] = "Linux FreeS/WAN " V;

/*
 - ipsec_version_code - return IPsec version number/code, as string
 */
const char *
ipsec_version_code()
{
	return freeswan_number;
}

/*
 - ipsec_version_string - return full version string
 */
const char *
ipsec_version_string()
{
	return freeswan_string;
}
