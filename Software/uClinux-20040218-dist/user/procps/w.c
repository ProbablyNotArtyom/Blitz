/* w - show what logged in users are doing.  Almost entirely rewritten from
 * scratch by Charles Blake circa June 1996.  Some vestigal traces of the
 * original may exist.  That was done in 1993 by Larry Greenfield with some
 * fixes by Michael K. Johnson.
 */
#include "proc/version.h"
#include "proc/whattime.h"
#include "proc/readproc.h"
#include "proc/devname.h"
#include "proc/procps.h"
#include "proc/sysinfo.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <utmp.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>
/* #include <sys/param.h>*/	/* for HZ */

int ignoreuser = 0;		/* for '-u' */
proc_t **procs;			/* our snapshot of the process table */

typedef struct utmp utmp_t;

#ifdef W_SHOWFROM
#   define FROM_STRING "on"
#else
#   define FROM_STRING "off"
#endif

#define USERSZ (sizeof u->ut_user)


/* This routine is careful since some programs leave utmp strings
 * unprintable.  Always outputs at least 16 chars padded with spaces
 * on the right if necessary.
 */
static void print_host(char* host, int len) {
    char *last;
    int width = 0;

    /* FIXME: there should really be a way to configure this... */
    /* for now, we'll just limit it to the 16 that the libc5 version
     * of utmp uses.
     */
    if (len > 16) len = 16;
    last = host + len;
    for ( ; host < last ; host++){
        if (isprint(*host) && *host != ' ') {
	    fputc(*host, stdout);
	    ++width;
	} else {
	    break;
	}
    }
    if(!width){   /* blank fields screw up parsers */
      fputc('-', stdout);
      ++width;
    }
    /* if *any* unprintables(or blanks), replace rest of line with spaces */
    while (width < 16) {
	fputc(' ', stdout);
	++width;
    }
}

/***** compact 7 char format for time intervals (belongs in libproc?) */
static void print_time_ival7(time_t t, int centi_sec, FILE* fout) {
    if((long)t < (long)0){  /* system clock changed? */
      printf("   ?   ");
      return;
    }
    if (t >= 48*60*60)				/* > 2 days */
	fprintf(fout, " %2ludays", t/(24*60*60));
    else if (t >= 60*60)			/* > 1 hour */
	fprintf(fout, " %2lu:%02um", t/(60*60), (unsigned) ((t/60)%60));
    else if (t > 60)				/* > 1 minute */
	fprintf(fout, " %2lu:%02u ", t/60, (unsigned) t%60);
    else
	fprintf(fout, " %2lu.%02us", t, centi_sec);
}

/**** stat the device file to get an idle time */
static time_t idletime(utmp_t *u, char *tty) {
    struct stat terminfo;
    time_t idle_secs;
    double uptime_secs;
    if (stat(tty, &terminfo) != 0)
	return 0;
    idle_secs = time(NULL) - terminfo.st_atime;
    uptime (&uptime_secs, NULL);
    if ((time_t) uptime_secs < idle_secs)
	idle_secs = (time_t) uptime_secs;
    return idle_secs;
}

/***** 7 character formatted login time */
static void print_logintime(time_t logt, FILE* fout) {
    char *weekday[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
	 *month  [] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
			"Aug", "Sep", "Oct", "Nov", "Dec" };
    time_t curt;
    struct tm *logtm, *curtm;
    int hour;
    char *merid; /* meridian indicator */
    int today;

    curt = time(NULL);
    curtm = localtime(&curt);
    /* localtime returns a pointer to static memory */
    today = curtm->tm_yday;
    logtm = localtime(&logt);
    hour = logtm->tm_hour;
    merid = (hour < 12) ? "am" : "pm";
    if (hour >= 12) hour -= 12;
    if (hour == 0)  hour = 12;
    if (curt - logt > 12*60*60 && logtm->tm_yday != today) {
	if (curt - logt > 6*24*60*60)
	    fprintf(fout, " %2d%3s%02d", logtm->tm_mday, month[logtm->tm_mon],
		    logtm->tm_year % 100);
	else
	    fprintf(fout, " %3s%2d%s", weekday[logtm->tm_wday], hour, merid);
    } else {
	fprintf(fout, " %2d:%02d%s", hour, logtm->tm_min, merid);
    }
}


/* This function scans the process table accumulating total cpu times for
 * any processes "associated" with this login session.  It also searches
 * for the "best" process to report as "(w)hat" the user for that login
 * session is doing currently.  This the essential core of 'w'.
 */
static proc_t *getproc(utmp_t *u, char *tty, int *jcpu, int *found_utpid) {
    int line;
    proc_t **p, *best = NULL, *secondbest = NULL;

    line = tty_to_dev(tty);
    *jcpu = *found_utpid = 0;
    for (p = procs; *p; p++) {
	if ((**p).pid == u->ut_pid)
	    *found_utpid = 1;
	if ((**p).tty == line) {
	    (*jcpu) += (**p).utime + (**p).stime;

	    secondbest=*p;

	    if ((ignoreuser || !strncmp((**p).euser, u->ut_user, USERSZ)) &&
		((**p).pid == (**p).tpgid && (!best || (**p).pid > best->pid)))
		best = *p;

	}
    }
    if (!best) {return secondbest;}
    return best;
}


/***** showinfo */
static void showinfo(utmp_t *u, int formtype, int maxcmd, int from) {
    int jcpu, i, ut_pid_found;
    char uname[USERSZ + 1] = "",
	tty[5 + sizeof u->ut_line + 1] = "/dev/";
    proc_t *best;

    for (i=0; i < sizeof u->ut_line; i++)	/* clean up tty if garbled */
	if (isalnum(u->ut_line[i]) || (u->ut_line[i]=='/'))
	    tty[i+5] = u->ut_line[i];
	else
	    tty[i+5] = '\0';

    best = getproc(u, tty + 5, &jcpu, &ut_pid_found);

    /* just skip if stale utmp entry (i.e. login proc doesn't exist).  If there
     * is a desire a cmdline flag could be added to optionally show it with a
     * prefix of (stale) in front of cmd or something like that.
     */
    if (!ut_pid_found)
	return;

    strncpy(uname, u->ut_user, USERSZ);		/* force NUL term for printf */
    if (formtype) {
	printf("%-9.8s%-9.8s", uname, u->ut_line);
	if (from)
	    print_host(u->ut_host, sizeof u->ut_host);
	print_logintime(u->ut_time, stdout);
	if (*u->ut_line == ':')			/* idle unknown for xdm logins */
	    printf("   ?   ");
	else
	    print_time_ival7(idletime(u, tty), 0, stdout);
	print_time_ival7(jcpu/Hertz, (jcpu%Hertz)*(100./Hertz), stdout);
	if (best) {
	    int pcpu = best->utime + best->stime;
	    print_time_ival7(pcpu/Hertz, (pcpu%Hertz)*(100./Hertz), stdout);
	} else
	    printf("   ?   ");
    } else {
	printf("%-9.8s%-9.8s", u->ut_user, u->ut_line);
	if (from)
	    print_host(u->ut_host, sizeof u->ut_host);
	if (*u->ut_line == ':')			/* idle unknown for xdm logins */
	    printf("   ?   ");
	else
	    print_time_ival7(idletime(u, tty), 0, stdout);
    }
    fputs("  ", stdout);
    if (best) {
	if (best->cmdline)
	    print_strlist(stdout, best->cmdline, " ", maxcmd);
	else
	    printf("%*.*s", -maxcmd, maxcmd, best->cmd);
    } else {
	printf("-");
    }
    fputc('\n', stdout);
}

/***** main */
int main(int argc, char **argv) {
    char *user = NULL;
    utmp_t *u;
    struct winsize win;
    int header=1, longform=1, from=1, args, maxcmd=80, ch;

#ifndef W_SHOWFROM
    from = 0;
#endif

    for (args=0; (ch = getopt(argc, argv, "hlusfV")) != EOF; args++)
	switch (ch) {
	  case 'h': header = 0;		break;
	  case 'l': longform = 1;	break;
	  case 's': longform = 0;	break;
	  case 'f': from = !from;	break;
	  case 'V': display_version();	exit(0);
	  case 'u': ignoreuser = 1;	break;
	  default:
	    printf("usage: w -hlsufV [user]\n"
		   "    -h    skip header\n"
		   "    -l    long listing (default)\n"
		   "    -s    short listing\n"
		   "    -u    ignore uid of processes\n"
		   "    -f    toggle FROM field (default %s)\n"
		   "    -V    display version\n", FROM_STRING);
	    exit(1);
	}

    if ((argv[optind]))
	user = (argv[optind]);

    if (ioctl(1, TIOCGWINSZ, &win) != -1 && win.ws_col > 0)
	maxcmd = win.ws_col;
    if (maxcmd < 71) {
	fprintf(stderr, "%d column window is too narrow\n", maxcmd);
	exit(1);
    }
    maxcmd -= 29 + (from ? 16 : 0) + (longform ? 20 : 0);
    if (maxcmd < 3)
	fprintf(stderr, "warning: screen width %d suboptimal.\n", win.ws_col);

    procs = readproctab(PROC_FILLCMD | PROC_FILLUSR);

    if (header) {				/* print uptime and headers */
	print_uptime();
	printf("USER     TTY      ");
	if (from)
	    printf("FROM            ");
	if (longform)
	    printf("  LOGIN@   IDLE   JCPU   PCPU  WHAT\n");
	else
	    printf("   IDLE  WHAT\n");
    }

    utmpname(UTMP_FILE);
    setutent();
    while ((u=getutent())) {
 	if (u->ut_type == USER_PROCESS &&
 	    (user ? !strncmp(u->ut_user, user, USERSZ) : *u->ut_user))
 	    showinfo(u, longform, maxcmd, from);
    }
    endutent();

    return 0;
}
