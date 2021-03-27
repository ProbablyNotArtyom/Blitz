/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)daemon.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <fcntl.h>
#include <paths.h>
#include <unistd.h>
#include <sys/stat.h>

#include <device-nrs.h>

int
daemon(nochdir, noclose)
	int nochdir, noclose;
{
	int fd;

	switch (__fork()) {
	case -1:
		return (-1);
	case 0:
		break;
	default:
		_exit(0);
	}

	if (__setsid() == -1)
		return (-1);

	if (!nochdir)
		(void)__chdir("/");

	if (!noclose && (fd = __open(_PATH_DEVNULL, O_RDWR, 0)) != -1) {
		struct stat64 st;

		if (__builtin_expect (__fxstat64 (_STAT_VER, fd, &st), 0) == 0
		    && __builtin_expect (S_ISCHR (st.st_mode), 1) != 0
#if defined DEV_NULL_MAJOR && defined DEV_NULL_MINOR
		    && st.st_rdev == makedev (DEV_NULL_MAJOR, DEV_NULL_MINOR)
#endif
		    ) {
			(void)__dup2(fd, STDIN_FILENO);
			(void)__dup2(fd, STDOUT_FILENO);
			(void)__dup2(fd, STDERR_FILENO);
			if (fd > 2)
				(void)__close (fd);
		} else {
			(void)__close (fd);
			return -1;
		}
	}
	return (0);
}
