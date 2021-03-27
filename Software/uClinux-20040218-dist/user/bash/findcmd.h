/* findcmd.h - functions from findcmd.c. */

/* Copyright (C) 1997 Free Software Foundation, Inc.

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

#if !defined (_FINDCMD_H_)
#define _FINDCMD_H_

#include "stdc.h"

extern int file_status __P((char *));
extern int executable_file __P((char *));
extern int is_directory __P((char *));
extern int executable_or_directory __P((char *));
extern char *find_user_command __P((char *));
extern char *find_path_file __P((char *));
extern char *search_for_command __P((char *));
extern char *user_command_matches __P((char *, int, int));

#endif /* _FINDCMD_H_ */
