/*
 * Copyright (C) 1991,1992 Erik Schoenfelder (schoenfr@ibr.cs.tu-bs.de)
 *
 * This file is part of NASE A60.
 * 
 * NASE A60 is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NASE A60 is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with NASE A60; see the file COPYING.  If not, write to the Free
 * Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * type.c:					aug '90
 *
 * Erik Schoenfelder (schoenfr@ibr.cs.tu-bs.de)
 *
 * contains not many things; may be i'll move it to symtab.c 
 * or tree.c ...
 */

#include "type.h"


char *
type_tag_name[] = {
	"unknown",
	"procedure",
	"switch",
	"label",
	"string",
	"integer",
	"integer array",
	"integer procedure",
	"real",
	"real array",
	"real procedure",
	"boolean",
	"boolean array",
	"boolean procedure",
	"last_type_tag_name"
};

/* end of type.c */
