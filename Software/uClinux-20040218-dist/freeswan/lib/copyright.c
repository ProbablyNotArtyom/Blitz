/*
 * return IPsec copyright notice
 * Copyright (C) 2001, 2002  Henry Spencer.
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
 * RCSID $Id: copyright.c,v 1.2 2002/02/25 20:33:11 henry Exp $
 */
#include "internal.h"
#include "freeswan.h"

static const char *co[] = {
 "Copyright (C) 1999, 2000, 2001, 2002  Henry Spencer, Richard Guy Briggs,",
 "    D. Hugh Redelmeier, Sandy Harris, Claudia Schmeing,",
 "    Michael Richardson, Angelos D. Keromytis, John Ioannidis.",
 "",
 "This program is free software; you can redistribute it and/or modify it",
 "under the terms of the GNU General Public License as published by the",
 "Free Software Foundation; either version 2 of the License, or (at your",
 "option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.",
 "",
 "This program is distributed in the hope that it will be useful, but",
 "WITHOUT ANY WARRANTY; without even the implied warranty of",
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General",
 "Public License (file COPYING in the distribution) for more details.",
 NULL
};

/*
 - ipsec_copyright_notice - return copyright notice, as a vector of strings
 */
const char **
ipsec_copyright_notice()
{
	return co;
}
