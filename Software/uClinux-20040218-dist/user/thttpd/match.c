/* match.c - simple shell-style filename matcher
**
** Only does ? and *, and multiple patterns separated by |.  Returns 1 or 0.
**
** Copyright (C) 1995 by Jef Poskanzer <jef@acme.com>.  All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/


#include <string.h>

#include "match.h"


int
match( char* pattern, char* string )
    {
    char* s;

    for ( ; ; ++pattern )
	{
	for ( s = string; ; ++pattern, ++s )
	    {
	    if ( *pattern == '?' && *s != '\0' )
		continue;
	    if ( *pattern == '*' )
		{
		int i;
		++pattern;
		for ( i = strlen( s ); i >= 0; --i )
		    if ( match( pattern, &(s[i]) ) )	/* not quite right */
			return 1;
		break;
		}
	    if ( *pattern == '|' || *pattern == '\0' )
		return 1;
	    if ( *pattern != *s )
		break;
	    }
	pattern = strchr( pattern, '|' );
	if ( pattern == (char*) 0 )
	    return 0;
	}
    }
