/*
 *   stunnel       Universal SSL tunnel
 *   Copyright (c) 1998-2001 Michal Trojnara <Michal.Trojnara@mirt.net>
 *                 All Rights Reserved
 *
 *   Based on a Public Domain code by Tatu Ylonen <ylo@cs.hut.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "common.h"

#include <stdio.h>       /* sprintf, snprintf */
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>      /* getpid, fork, execvp, exit */
#endif

#ifdef HAVE_UTIL_H
#include <util.h>
#endif /* HAVE_UTIL_H */

/* Pty allocated with _getpty gets broken if we do I_PUSH:es to it. */
#if defined(HAVE__GETPTY) || defined(HAVE_OPENPTY)
#undef HAVE_DEV_PTMX
#endif

#ifdef HAVE_PTY_H
#include <pty.h>
#endif

#if defined(HAVE_DEV_PTMX) && defined(HAVE_STROPTS_H)
#include <stropts.h>
#endif

#ifndef O_NOCTTY
#define O_NOCTTY 0
#endif

/*
 * Allocates and opens a pty.  Returns -1 if no pty could be allocated, or
 * zero if a pty was successfully allocated.  On success, open file
 * descriptors for the pty and tty sides and the name of the tty side are
 * returned (the buffer must be able to hold at least 64 characters).
 */

int pty_allocate(int *ptyfd, int *ttyfd, char *namebuf, int namebuflen) {
#if defined(HAVE_OPENPTY) || defined(BSD4_4)
    /* openpty(3) exists in OSF/1 and some other os'es */
    char buf[64];
    int i;

    i = openpty(ptyfd, ttyfd, buf, NULL, NULL);
    if (i < 0) {
        sockerror("openpty");
        return -1;
    }
    safecopy(namebuf, buf); /* possible truncation */
    return 0;
#else /* HAVE_OPENPTY */
#ifdef HAVE__GETPTY
    /*
     * _getpty(3) exists in SGI Irix 4.x, 5.x & 6.x -- it generates more
     * pty's automagically when needed
     */
    char *slave;

    slave = _getpty(ptyfd, O_RDWR, 0622, 0);
    if (slave == NULL) {
        sockerror("_getpty");
        return -1;
    }
    safecopy(namebuf, slave);
    /* Open the slave side. */
    *ttyfd = open(namebuf, O_RDWR | O_NOCTTY);
    if (*ttyfd < 0) {
        sockerror(namebuf);
        close(*ptyfd);
        return -1;
    }
    return 0;
#else /* HAVE__GETPTY */
#if defined(HAVE_DEV_PTMX)
    /*
     * This code is used e.g. on Solaris 2.x.  (Note that Solaris 2.3
     * also has bsd-style ptys, but they simply do not work.)
     */
    int ptm;
    char *pts;

    ptm = open("/dev/ptmx", O_RDWR | O_NOCTTY);
    if (ptm < 0) {
        sockerror("/dev/ptmx");
        return -1;
    }
    if (grantpt(ptm) < 0) {
        sockerror("grantpt");
        /* return -1; */
        /* Can you tell me why it doesn't work? */
    }
    if (unlockpt(ptm) < 0) {
        sockerror("unlockpt");
        return -1;
    }
    pts = ptsname(ptm);
    if (pts == NULL)
        log(LOG_ERR, "Slave pty side name could not be obtained.");
    safecopy(namebuf, pts);
    *ptyfd = ptm;

    /* Open the slave side. */
    *ttyfd = open(namebuf, O_RDWR | O_NOCTTY);
    if (*ttyfd < 0) {
        sockerror(namebuf);
        close(*ptyfd);
        return -1;
    }
    /* Push the appropriate streams modules, as described in Solaris pts(7). */
    if (ioctl(*ttyfd, I_PUSH, "ptem") < 0)
        sockerror("ioctl I_PUSH ptem");
    if (ioctl(*ttyfd, I_PUSH, "ldterm") < 0)
        sockerror("ioctl I_PUSH ldterm");
    if (ioctl(*ttyfd, I_PUSH, "ttcompat") < 0)
        sockerror("ioctl I_PUSH ttcompat");
    return 0;
#else /* HAVE_DEV_PTMX */
#ifdef HAVE_DEV_PTS_AND_PTC
    /* AIX-style pty code. */
    const char *name;

    *ptyfd = open("/dev/ptc", O_RDWR | O_NOCTTY);
    if (*ptyfd < 0) {
        sockerror("open(/dev/ptc)");
        return -1;
    }
    name = ttyname(*ptyfd);
    if (!name) {
        log(LOG_ERR, "Open of /dev/ptc returns device for which ttyname fails.");
        return -1;
    }
    safecopy(namebuf, name);
    *ttyfd = open(name, O_RDWR | O_NOCTTY);
    if (*ttyfd < 0) {
        sockerror(name);
        close(*ptyfd);
        return -1;
    }
    return 0;
#else /* HAVE_DEV_PTS_AND_PTC */
    /* BSD-style pty code. */
    char buf[64];
    int i;
    const char *ptymajors = "pqrstuvwxyzabcdefghijklmnoABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char *ptyminors = "0123456789abcdef";
    int num_minors = strlen(ptyminors);
    int num_ptys = strlen(ptymajors) * num_minors;

    for (i = 0; i < num_ptys; i++) {
#ifdef HAVE_SNPRINTF
        snprintf(buf, sizeof buf,
#else
        sprintf(buf,
#endif
             "/dev/pty%c%c", ptymajors[i / num_minors],
             ptyminors[i % num_minors]);
        *ptyfd = open(buf, O_RDWR | O_NOCTTY);
        if (*ptyfd < 0)
            continue;
#ifdef HAVE_SNPRINTF
        snprintf(namebuf, namebuflen,
#else
        sprintf(namebuf,
#endif
            "/dev/tty%c%c",
            ptymajors[i / num_minors], ptyminors[i % num_minors]);

        /* Open the slave side. */
        *ttyfd = open(namebuf, O_RDWR | O_NOCTTY);
        if (*ttyfd < 0) {
            sockerror(namebuf);
            close(*ptyfd);
            return -1;
        }
        return 0;
    }
    return -1;
#endif /* HAVE_DEV_PTS_AND_PTC */
#endif /* HAVE_DEV_PTMX */
#endif /* HAVE__GETPTY */
#endif /* HAVE_OPENPTY */
}

/* Releases the tty.  Its ownership is returned to root, and permissions to 0666. */

void pty_release(char *ttyname) {
    if (chown(ttyname, (uid_t) 0, (gid_t) 0) < 0)
        log(LOG_DEBUG, "chown %.100s 0 0 failed: %.100s", ttyname, strerror(errno));
    if (chmod(ttyname, (mode_t) 0666) < 0)
        log(LOG_DEBUG, "chmod %.100s 0666 failed: %.100s", ttyname, strerror(errno));
}

/* Makes the tty the processes controlling tty and sets it to sane modes. */

void pty_make_controlling_tty(int *ttyfd, char *ttyname) {
    int fd;

    /* First disconnect from the old controlling tty. */
#ifdef TIOCNOTTY
    fd = open("/dev/tty", O_RDWR | O_NOCTTY);
    if (fd >= 0) {
        (void) ioctl(fd, TIOCNOTTY, NULL);
        close(fd);
    }
#endif /* TIOCNOTTY */
    if (setsid() < 0)
        sockerror("setsid");

    /*
     * Verify that we are successfully disconnected from the controlling
     * tty.
     */
    fd = open("/dev/tty", O_RDWR | O_NOCTTY);
    if (fd >= 0) {
        log(LOG_ERR, "Failed to disconnect from controlling tty.");
        close(fd);
    }
    /* Make it our controlling tty. */
#ifdef TIOCSCTTY
    log(LOG_DEBUG, "Setting controlling tty using TIOCSCTTY.");
    /*
     * We ignore errors from this, because HPSUX defines TIOCSCTTY, but
     * returns EINVAL with these arguments, and there is absolutely no
     * documentation.
     */
    ioctl(*ttyfd, TIOCSCTTY, NULL);
#endif /* TIOCSCTTY */
    fd = open(ttyname, O_RDWR);
    if (fd < 0)
        sockerror(ttyname);
    else
        close(fd);

    /* Verify that we now have a controlling tty. */
    fd = open("/dev/tty", O_WRONLY);
    if (fd < 0)
        sockerror("open /dev/tty failed - could not set controlling tty");
    else {
        close(fd);
    }
}

/* End of stunnel.c */

