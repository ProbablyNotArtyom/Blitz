/* randomness machinery
 * Copyright (C) 1998, 1999  D. Hugh Redelmeier.
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
 * RCSID $Id: rnd.h,v 1.6 1999/12/13 00:40:56 dhr Exp $
 */

extern u_char    secret_of_the_day[SHA1_DIGEST_SIZE];

extern void get_rnd_bytes(u_char *buffer, int length);
extern void init_rnd_pool(void);
extern void init_secret(void);
