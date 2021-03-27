/* vi: set sw=4 ts=4: */
/* agetty.c - another getty program for Linux. By W. Z. Venema 1989
   Ported to Linux by Peter Orbaek <poe@daimi.aau.dk>
   This program is freely distributable. The entire man-page used to
   be here. Now read the real man-page agetty.8 instead.

   -f option added by Eric Rasmussen <ear@usfirst.org> - 12/28/95
   
   1999-02-22 Arkadiusz Mi�kiewicz <misiek@misiek.eu.org>
   - added Native Language Support

   1999-05-05 Thorsten Kranzkowski <dl8bcu@gmx.net>
   - enable hardware flow control before displaying /etc/issue
   
*/
#include "tinylogin.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>
#include <utmp.h>
#include <getopt.h>
#include <termios.h>

#ifdef linux
#include <sys/param.h>
#define USE_SYSLOG
#endif

 /* If USE_SYSLOG is undefined all diagnostics go directly to /dev/console. */
#ifdef	USE_SYSLOG
#include <syslog.h>
#endif


 /*
  * Some heuristics to find out what environment we are in: if it is not
  * System V, assume it is SunOS 4.
  */

#ifdef LOGIN_PROCESS			/* defined in System V utmp.h */
#define	SYSV_STYLE				/* select System V style getty */
#endif

 /*
  * Things you may want to modify.
  * 
  * If ISSUE is not defined, agetty will never display the contents of the
  * /etc/issue file. You will not want to spit out large "issue" files at the
  * wrong baud rate. Relevant for System V only.
  * 
  * You may disagree with the default line-editing etc. characters defined
  * below. Note, however, that DEL cannot be used for interrupt generation
  * and for line editing at the same time.
  */

#ifdef	SYSV_STYLE
#define	ISSUE "/etc/issue"		/* displayed before the login prompt */
#include <sys/utsname.h>
#include <time.h>
#endif

#define LOGIN " login: "		/* login prompt */

/* Some shorthands for control characters. */

#define CTL(x)		(x ^ 0100)	/* Assumes ASCII dialect */
#define	CR		CTL('M')		/* carriage return */
#define	NL		CTL('J')		/* line feed */
#define	BS		CTL('H')		/* back space */
#define	DEL		CTL('?')		/* delete */

/* Defaults for line-editing etc. characters; you may want to change this. */

#define DEF_ERASE	DEL			/* default erase character */
#define DEF_INTR	CTL('C')	/* default interrupt character */
#define DEF_QUIT	CTL('\\')	/* default quit char */
#define DEF_KILL	CTL('U')	/* default kill char */
#define DEF_EOF		CTL('D')	/* default EOF char */
#define DEF_EOL		0
#define DEF_SWITCH	0			/* default switch char */

 /*
  * SunOS 4.1.1 termio is broken. We must use the termios stuff instead,
  * because the termio -> termios translation does not clear the termios
  * CIBAUD bits. Therefore, the tty driver would sometimes report that input
  * baud rate != output baud rate. I did not notice that problem with SunOS
  * 4.1. We will use termios where available, and termio otherwise.
  */

/* linux 0.12 termio is broken too, if we use it c_cc[VERASE] isn't set
   properly, but all is well if we use termios?! */

#ifdef	TCGETS
#undef	TCGETA
#undef	TCSETA
#undef	TCSETAW
#define	termio	termios
#define	TCGETA	TCGETS
#define	TCSETA	TCSETS
#define	TCSETAW	TCSETSW
#endif

 /*
  * This program tries to not use the standard-i/o library.  This keeps the
  * executable small on systems that do not have shared libraries (System V
  * Release <3).
  */
#ifndef BUFSIZ
#define	BUFSIZ		1024
#endif

 /*
  * When multiple baud rates are specified on the command line, the first one
  * we will try is the first one specified.
  */

#define	FIRST_SPEED	0

/* Storage for command-line options. */

#define	MAX_SPEED	10			/* max. nr. of baud rates */

struct options {
	int flags;					/* toggle switches, see below */
	int timeout;				/* time-out period */
	char *login;				/* login program */
	char *tty;					/* name of tty */
	char *initstring;			/* modem init string */
	char *issue;				/* alternative issue file */
	int numspeed;				/* number of baud rates to try */
	int speeds[MAX_SPEED];		/* baud rates to be tried */
};

#define	F_PARSE		(1<<0)		/* process modem status messages */
#define	F_ISSUE		(1<<1)		/* display /etc/issue */
#define	F_RTSCTS	(1<<2)		/* enable RTS/CTS flow control */
#define F_LOCAL		(1<<3)		/* force local */
#define F_INITSTRING    (1<<4)	/* initstring is set */
#define F_WAITCRLF	(1<<5)		/* wait for CR or LF */
#define F_CUSTISSUE	(1<<6)		/* give alternative issue file */
#define F_NOPROMPT	(1<<7)		/* don't ask for login name! */

/* Storage for things detected while the login name was read. */

struct chardata {
	int erase;					/* erase character */
	int kill;					/* kill character */
	int eol;					/* end-of-line character */
	int parity;					/* what parity did we see */
	int capslock;				/* upper case without lower case */
};

/* Initial values for the above. */

struct chardata init_chardata = {
	DEF_ERASE,					/* default erase character */
	DEF_KILL,					/* default kill character */
	13,							/* default eol char */
	0,							/* space parity */
	0,							/* no capslock */
};

struct Speedtab {
	long speed;
	int code;
};

static struct Speedtab speedtab[] = {
	{50, B50},
	{75, B75},
	{110, B110},
	{134, B134},
	{150, B150},
	{200, B200},
	{300, B300},
	{600, B600},
	{1200, B1200},
	{1800, B1800},
	{2400, B2400},
	{4800, B4800},
	{9600, B9600},
#ifdef	B19200
	{19200, B19200},
#endif
#ifdef	B38400
	{38400, B38400},
#endif
#ifdef	EXTA
	{19200, EXTA},
#endif
#ifdef	EXTB
	{38400, EXTB},
#endif
#ifdef B57600
	{57600, B57600},
#endif
#ifdef B115200
	{115200, B115200},
#endif
#ifdef B230400
	{230400, B230400},
#endif
	{0, 0},
};

void parse_args(int argc, char **argv, struct options *op);
void parse_speeds(struct options *op, char *arg);
void update_utmp(char *line);
void open_tty(char *tty, struct termio *tp, int local);
void termio_init(struct termio *tp, int speed, struct options *op);
void auto_baud(struct termio *tp);
void do_prompt(struct options *op, struct termio *tp);
void next_speed(struct termio *tp, struct options *op);
char *get_logname(struct options *op, struct chardata *cp,

				  struct termio *tp);
void termio_final(struct options *op, struct termio *tp,

				  struct chardata *cp);
int caps_lock(char *s);
int bcode(char *s);
static void error(const char *fmt, ...);

/* The following is used for understandable diagnostics. */

char *progname;

/* Fake hostname for ut_host specified on command line. */
char *fakehost = NULL;

/* ... */
#ifdef DEBUGGING
#define debug(s) fprintf(dbf,s); fflush(dbf)
#define DEBUGTERM "/dev/ttyp0"
FILE *dbf;
#else
#define debug(s)				/* nothing */
#endif

int getty_main(int argc, char **argv)
{
	char *logname = NULL;		/* login name, given to /bin/login */
	struct chardata chardata;	/* set by get_logname() */
	struct termio termio;		/* terminal mode bits */
	static struct options options = {
		F_ISSUE,				/* show /etc/issue (SYSV_STYLE) */
		0,						/* no timeout */
		_PATH_LOGIN,			/* default login program */
		"tty1",					/* default tty line */
		"",						/* modem init string */
		ISSUE,					/* default issue file */
		0,						/* no baud rates known yet */
	};

	/* The BSD-style init command passes us a useless process name. */

#ifdef	SYSV_STYLE
	progname = argv[0];
#else
	progname = "getty";
#endif

#ifdef DEBUGGING
	dbf = fopen(DEBUGTERM, "w");
	if (!dbf) {
		perror_msg_and_die("Cannot open " DEBUGTERM);
	}

	{
		int i;

		for (i = 1; i < argc; i++) {
			debug(argv[i]);
			debug("\n");
		}
	}
#endif

	/* Parse command-line arguments. */

	parse_args(argc, argv, &options);

#ifdef __linux__
	setsid();
#endif

	/* Update the utmp file. */

#ifdef	SYSV_STYLE
	update_utmp(options.tty);
#endif

	debug("calling open_tty\n");
	/* Open the tty as standard { input, output, error }. */
	open_tty(options.tty, &termio, options.flags & F_LOCAL);

#ifdef __linux__
	{
		int iv;

		iv = getpid();
		if (ioctl(0, TIOCSPGRP, &iv) < 0)
			perror_msg("ioctl() TIOCSPGRP call failed");
	}
#endif
	/* Initialize the termio settings (raw mode, eight-bit, blocking i/o). */
	debug("calling termio_init\n");
	termio_init(&termio, options.speeds[FIRST_SPEED], &options);

	/* write the modem init string and DON'T flush the buffers */
	if (options.flags & F_INITSTRING) {
		debug("writing init string\n");
		write(1, options.initstring, strlen(options.initstring));
	}

	if (!(options.flags & F_LOCAL)) {
		/* go to blocking write mode unless -L is specified */
		fcntl(1, F_SETFL, fcntl(1, F_GETFL, 0) & ~O_NONBLOCK);
	}

	/* Optionally detect the baud rate from the modem status message. */
	debug("before autobaud\n");
	if (options.flags & F_PARSE)
		auto_baud(&termio);

	/* Set the optional timer. */
	if (options.timeout)
		(void) alarm((unsigned) options.timeout);

	/* optionally wait for CR or LF before writing /etc/issue */
	if (options.flags & F_WAITCRLF) {
		char ch;

		debug("waiting for cr-lf\n");
		while (read(0, &ch, 1) == 1) {
			ch &= 0x7f;			/* strip "parity bit" */
#ifdef DEBUGGING
			fprintf(dbf, "read %c\n", ch);
#endif
			if (ch == '\n' || ch == '\r')
				break;
		}
	}

	chardata = init_chardata;
	if (!(options.flags & F_NOPROMPT)) {
		/* Read the login name. */
		debug("reading login name\n");
		/* while ((logname = get_logname(&options, &chardata, &termio)) == 0) */
		while ((logname = get_logname(&options, &chardata, &termio)) ==
			   NULL) next_speed(&termio, &options);
	}

	/* Disable timer. */

	if (options.timeout)
		(void) alarm(0);

	/* Finalize the termio settings. */

	termio_final(&options, &termio, &chardata);

	/* Now the newline character should be properly written. */

	(void) write(1, "\n", 1);

	/* Let the login program take care of password validation. */

	(void) execl(options.login, options.login, "--", logname, (char *) 0);
	error("%s: can't exec %s: %m", options.tty, options.login);
	return (0);					/* quiet GCC */
}

/* parse-args - parse command-line arguments */

void parse_args(argc, argv, op)
int argc;
char **argv;
struct options *op;
{
	extern char *optarg;		/* getopt */
	extern int optind;			/* getopt */
	int c;

	while (isascii(c = getopt(argc, argv, "I:LH:f:hil:mt:wn"))) {
		switch (c) {
		case 'I':
			if (!(op->initstring = malloc(strlen(optarg)))) {
				error("can't malloc initstring");
				break;
			}
			{
				char ch, *p, *q;
				int i;

				/* copy optarg into op->initstring decoding \ddd
				   octal codes into chars */
				q = op->initstring;
				p = optarg;
				while (*p) {
					if (*p == '\\') {	/* know \\ means \ */
						p++;
						if (*p == '\\') {
							ch = '\\';
							p++;
						} else {	/* handle \000 - \177 */
							ch = 0;
							for (i = 1; i <= 3; i++) {
								if (*p >= '0' && *p <= '7') {
									ch <<= 3;
									ch += *p - '0';
									p++;
								} else
									break;
							}
						}
						*q++ = ch;
					} else {
						*q++ = *p++;
					}
				}
				*q = '\0';
			}
			op->flags |= F_INITSTRING;
			break;

		case 'L':				/* force local */
			op->flags |= F_LOCAL;
			break;
		case 'H':				/* fake login host */
			fakehost = optarg;
			break;
		case 'f':				/* custom issue file */
			op->flags |= F_CUSTISSUE;
			op->issue = optarg;
			break;
		case 'h':				/* enable h/w flow control */
			op->flags |= F_RTSCTS;
			break;
		case 'i':				/* do not show /etc/issue */
			op->flags &= ~F_ISSUE;
			break;
		case 'l':
			op->login = optarg;	/* non-default login program */
			break;
		case 'm':				/* parse modem status message */
			op->flags |= F_PARSE;
			break;
		case 'n':
			op->flags |= F_NOPROMPT;
			break;
		case 't':				/* time out */
			if ((op->timeout = atoi(optarg)) <= 0)
				error("bad timeout value: %s", optarg);
			break;
		case 'w':
			op->flags |= F_WAITCRLF;
			break;
		default:
			usage(getty_usage);
		}
	}
	debug("after getopt loop\n");
	if (argc < optind + 2)		/* check parameter count */
		usage(getty_usage);

	/* we loosen up a bit and accept both "baudrate tty" and "tty baudrate" */
	if ('0' <= argv[optind][0] && argv[optind][0] <= '9') {
		/* a number first, assume it's a speed (BSD style) */
		parse_speeds(op, argv[optind++]);	/* baud rate(s) */
		op->tty = argv[optind];	/* tty name */
	} else {
		op->tty = argv[optind++];	/* tty name */
		parse_speeds(op, argv[optind]);	/* baud rate(s) */
	}

	optind++;
	if (argc > optind && argv[optind])
		setenv("TERM", argv[optind], 1);

	debug("exiting parseargs\n");
}

/* parse_speeds - parse alternate baud rates */

void parse_speeds(op, arg)
struct options *op;
char *arg;
{
	char *strtok();
	char *cp;

	debug("entered parse_speeds\n");
	for (cp = strtok(arg, ","); cp != 0; cp = strtok((char *) 0, ",")) {
		if ((op->speeds[op->numspeed++] = bcode(cp)) <= 0)
			error("bad speed: %s", cp);
		if (op->numspeed > MAX_SPEED)
			error("too many alternate speeds");
	}
	debug("exiting parsespeeds\n");
}

#ifdef	SYSV_STYLE

/* update_utmp - update our utmp entry */
void update_utmp(line)
char *line;
{
	struct utmp ut;
	time_t t;
	int mypid = getpid();
	long time();
	long lseek();
	struct utmp *utp;

	/*
	 * The utmp file holds miscellaneous information about things started by
	 * /sbin/init and other system-related events. Our purpose is to update
	 * the utmp entry for the current process, in particular the process type
	 * and the tty line we are listening to. Return successfully only if the
	 * utmp file can be opened for update, and if we are able to find our
	 * entry in the utmp file.
	 */

#ifdef __linux__
	utmpname(_PATH_UTMP);
	setutent();
	while ((utp = getutent())
		   && !(utp->ut_type == INIT_PROCESS && utp->ut_pid == mypid))	/* nothing */
		;

	if (utp) {
		memcpy(&ut, utp, sizeof(ut));
	} else {
		/* some inits don't initialize utmp... */
		memset(&ut, 0, sizeof(ut));
		strncpy(ut.ut_id, line + 3, sizeof(ut.ut_id));
	}
	/*endutent(); */

	strncpy(ut.ut_user, "LOGIN", sizeof(ut.ut_user));
	strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	if (fakehost)
		strncpy(ut.ut_host, fakehost, sizeof(ut.ut_host));
	time(&t);
	ut.ut_time = t;
	ut.ut_type = LOGIN_PROCESS;
	ut.ut_pid = mypid;

	pututline(&ut);
	endutent();

	updwtmp(_PATH_WTMP, &ut);
#else							/* not __linux__ */
	{
		int ut_fd;

		if ((ut_fd = open(UTMP_FILE, 2)) < 0) {
			error("%s: open for update: %m"), UTMP_FILE;
		} else {
			long ut_size = sizeof(ut);	/* avoid nonsense */

			while (read(ut_fd, (char *) &ut, sizeof(ut)) == sizeof(ut)) {
				if (ut.ut_type == INIT_PROCESS && ut.ut_pid == mypid) {
					ut.ut_type = LOGIN_PROCESS;
					ut.ut_time = time((long *) 0);
					(void) strncpy(ut.ut_name, "LOGIN", sizeof(ut.ut_name));
					(void) strncpy(ut.ut_line, line, sizeof(ut.ut_line));
					if (fakehost)
						(void) strncpy(ut.ut_host, fakehost, sizeof(ut.ut_host));
					(void) lseek(ut_fd, -ut_size, 1);
					(void) write(ut_fd, (char *) &ut, sizeof(ut));
					(void) close(ut_fd);
					return;
				}
			}
			error("%s: no utmp entry", line);
		}
	}
#endif							/* __linux__ */
}

#endif

/* open_tty - set up tty as standard { input, output, error } */
void open_tty(tty, tp, local)
char *tty;
struct termio *tp;
int local;
{
	/* Get rid of the present standard { output, error} if any. */

	(void) close(1);
	(void) close(2);
	errno = 0;					/* ignore above errors */

	/* Set up new standard input, unless we are given an already opened port. */

	if (strcmp(tty, "-")) {
		struct stat st;

		/* Sanity checks... */

		if (chdir("/dev"))
			error("/dev: chdir() failed: %m");
		if (stat(tty, &st) < 0)
			error("/dev/%s: %m", tty);
		if ((st.st_mode & S_IFMT) != S_IFCHR)
			error("/dev/%s: not a character device", tty);

		/* Open the tty as standard input. */

		(void) close(0);
		errno = 0;				/* ignore close(2) errors */

		debug("open(2)\n");
		if (open(tty, O_RDWR | O_NONBLOCK, 0) != 0)
			error("/dev/%s: cannot open as standard input: %m", tty);

	} else {

		/*
		 * Standard input should already be connected to an open port. Make
		 * sure it is open for read/write.
		 */

		if ((fcntl(0, F_GETFL, 0) & O_RDWR) != O_RDWR)
			error("%s: not open for read/write", tty);
	}

	/* Set up standard output and standard error file descriptors. */
	debug("duping\n");
	if (dup(0) != 1 || dup(0) != 2)	/* set up stdout and stderr */
		error("%s: dup problem: %m", tty);	/* we have a problem */

	/*
	 * The following ioctl will fail if stdin is not a tty, but also when
	 * there is noise on the modem control lines. In the latter case, the
	 * common course of action is (1) fix your cables (2) give the modem more
	 * time to properly reset after hanging up. SunOS users can achieve (2)
	 * by patching the SunOS kernel variable "zsadtrlow" to a larger value;
	 * 5 seconds seems to be a good value.
	 */

	if (ioctl(0, TCGETA, tp) < 0)
		error("%s: ioctl: %m", tty);

	/*
	 * It seems to be a terminal. Set proper protections and ownership. Mode
	 * 0622 is suitable for SYSV <4 because /bin/login does not change
	 * protections. SunOS 4 login will change the protections to 0620 (write
	 * access for group tty) after the login has succeeded.
	 */

#ifdef DEBIAN
	{
		/* tty to root.dialout 660 */
		struct group *gr;
		int id;

		id = (gr = getgrnam("dialout")) ? gr->gr_gid : 0;
		chown(tty, 0, id);
		chmod(tty, 0660);

		/* vcs,vcsa to root.sys 600 */
		if (!strncmp(tty, "tty", 3) && isdigit(tty[3])) {
			char *vcs, *vcsa;

			if (!(vcs = malloc(strlen(tty))))
				error("Can't malloc for vcs");
			if (!(vcsa = malloc(strlen(tty) + 1)))
				error("Can't malloc for vcsa");
			strcpy(vcs, "vcs");
			strcpy(vcs + 3, tty + 3);
			strcpy(vcsa, "vcsa");
			strcpy(vcsa + 4, tty + 3);

			id = (gr = getgrnam("sys")) ? gr->gr_gid : 0;
			chown(vcs, 0, id);
			chmod(vcs, 0600);
			chown(vcsa, 0, id);
			chmod(vcs, 0600);

			free(vcs);
			free(vcsa);
		}
	}
#else
	(void) chown(tty, 0, 0);	/* root, sys */
	(void) chmod(tty, 0622);	/* crw--w--w- */
	errno = 0;					/* ignore above errors */
#endif
}

/* termio_init - initialize termio settings */

char gbuf[1024];
char area[1024];

void termio_init(tp, speed, op)
struct termio *tp;
int speed;
struct options *op;
{

	/*
	 * Initial termio settings: 8-bit characters, raw-mode, blocking i/o.
	 * Special characters are set after we have read the login name; all
	 * reads will be done in raw mode anyway. Errors will be dealt with
	 * lateron.
	 */
#ifdef __linux__
	/* flush input and output queues, important for modems! */
	(void) ioctl(0, TCFLSH, TCIOFLUSH);
#endif

	tp->c_cflag = CS8 | HUPCL | CREAD | speed;
	if (op->flags & F_LOCAL) {
		tp->c_cflag |= CLOCAL;
	}

	tp->c_iflag = tp->c_lflag = tp->c_oflag = tp->c_line = 0;
	tp->c_cc[VMIN] = 1;
	tp->c_cc[VTIME] = 0;

	/* Optionally enable hardware flow control */

#ifdef	CRTSCTS
	if (op->flags & F_RTSCTS)
		tp->c_cflag |= CRTSCTS;
#endif

	(void) ioctl(0, TCSETA, tp);

	/* go to blocking input even in local mode */
	fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~O_NONBLOCK);

	debug("term_io 2\n");
}

/* auto_baud - extract baud rate from modem status message */
void auto_baud(tp)
struct termio *tp;
{
	int speed;
	int vmin;
	unsigned iflag;
	char buf[BUFSIZ];
	char *bp;
	int nread;

	/*
	 * This works only if the modem produces its status code AFTER raising
	 * the DCD line, and if the computer is fast enough to set the proper
	 * baud rate before the message has gone by. We expect a message of the
	 * following format:
	 * 
	 * <junk><number><junk>
	 * 
	 * The number is interpreted as the baud rate of the incoming call. If the
	 * modem does not tell us the baud rate within one second, we will keep
	 * using the current baud rate. It is advisable to enable BREAK
	 * processing (comma-separated list of baud rates) if the processing of
	 * modem status messages is enabled.
	 */

	/*
	 * Use 7-bit characters, don't block if input queue is empty. Errors will
	 * be dealt with lateron.
	 */

	iflag = tp->c_iflag;
	tp->c_iflag |= ISTRIP;		/* enable 8th-bit stripping */
	vmin = tp->c_cc[VMIN];
	tp->c_cc[VMIN] = 0;			/* don't block if queue empty */
	(void) ioctl(0, TCSETA, tp);

	/*
	 * Wait for a while, then read everything the modem has said so far and
	 * try to extract the speed of the dial-in call.
	 */

	(void) sleep(1);
	if ((nread = read(0, buf, sizeof(buf) - 1)) > 0) {
		buf[nread] = '\0';
		for (bp = buf; bp < buf + nread; bp++) {
			if (isascii(*bp) && isdigit(*bp)) {
				if ((speed = bcode(bp))) {
					tp->c_cflag &= ~CBAUD;
					tp->c_cflag |= speed;
				}
				break;
			}
		}
	}
	/* Restore terminal settings. Errors will be dealt with lateron. */

	tp->c_iflag = iflag;
	tp->c_cc[VMIN] = vmin;
	(void) ioctl(0, TCSETA, tp);
}

/* do_prompt - show login prompt, optionally preceded by /etc/issue contents */
void do_prompt(op, tp)
struct options *op;
struct termio *tp;
{
#ifdef	ISSUE
	FILE *fd;
	int oflag;
	int c;
	struct utsname uts;

	(void) uname(&uts);
#endif

	(void) write(1, "\r\n", 2);	/* start a new line */
#ifdef	ISSUE					/* optional: show /etc/issue */
	if ((op->flags & F_ISSUE) && (fd = fopen(op->issue, "r"))) {
		oflag = tp->c_oflag;	/* save current setting */
		tp->c_oflag |= (ONLCR | OPOST);	/* map NL in output to CR-NL */
		(void) ioctl(0, TCSETAW, tp);


		while ((c = getc(fd)) != EOF) {
			if (c == '\\') {
				c = getc(fd);

				switch (c) {
				case 's':
					(void) printf("%s", uts.sysname);
					break;

				case 'n':
					(void) printf("%s", uts.nodename);
					break;

				case 'r':
					(void) printf("%s", uts.release);
					break;

				case 'v':
					(void) printf("%s", uts.version);
					break;

				case 'm':
					(void) printf("%s", uts.machine);
					break;

				case 'o':
				{
					char domainname[256];

					getdomainname(domainname, sizeof(domainname));
					domainname[sizeof(domainname) - 1] = '\0';
					printf("%s", domainname);
				}
					break;

				case 'd':
				case 't':
				{
					char *weekday[] = { "Sun", "Mon", "Tue", "Wed", "Thu",
						"Fri", "Sat"
					};
					char *month[] = { "Jan", "Feb", "Mar", "Apr", "May",
						"Jun", "Jul", "Aug", "Sep", "Oct",
						"Nov", "Dec"
					};
					time_t now;
					struct tm *tm;

					(void) time(&now);
					tm = localtime(&now);

					if (c == 'd')
						(void) printf("%s %s %d  %d",
									  weekday[tm->tm_wday],
									  month[tm->tm_mon], tm->tm_mday,
									  tm->tm_year <
									  70 ? tm->tm_year +
									  2000 : tm->tm_year + 1900);
					else
						(void) printf("%02d:%02d:%02d", tm->tm_hour,
									  tm->tm_min, tm->tm_sec);

					break;
				}

				case 'l':
					(void) printf("%s", op->tty);
					break;

				case 'b':
				{
					int i;

					for (i = 0; speedtab[i].speed; i++) {
						if (speedtab[i].code == (tp->c_cflag & CBAUD)) {
							printf("%ld", speedtab[i].speed);
							break;
						}
					}
					break;
				}
				case 'u':
				case 'U':
				{
					int users = 0;
					struct utmp *ut;

					setutent();
					while ((ut = getutent()))
						if (ut->ut_type == USER_PROCESS)
							users++;
					endutent();
					printf("%d ", users);
					if (c == 'U')
						printf((users == 1) ? "user" : "users");
					break;
				}
				default:
					(void) putchar(c);
				}
			} else
				(void) putchar(c);
		}
		fflush(stdout);

		tp->c_oflag = oflag;	/* restore settings */
		(void) ioctl(0, TCSETAW, tp);	/* wait till output is gone */
		(void) fclose(fd);
	}
#endif
#ifdef __linux__
	{
		char hn[MAXHOSTNAMELEN + 1];

		(void) gethostname(hn, MAXHOSTNAMELEN);
		write(1, hn, strlen(hn));
	}
#endif
	(void) write(1, LOGIN, sizeof(LOGIN) - 1);	/* always show login prompt */
}

/* next_speed - select next baud rate */
void next_speed(tp, op)
struct termio *tp;
struct options *op;
{
	static int baud_index = FIRST_SPEED;	/* current speed index */

	baud_index = (baud_index + 1) % op->numspeed;
	tp->c_cflag &= ~CBAUD;
	tp->c_cflag |= op->speeds[baud_index];
	(void) ioctl(0, TCSETA, tp);
}

/* get_logname - get user name, establish parity, speed, erase, kill, eol */
/* return NULL on failure, logname on success */
char *get_logname(op, cp, tp)
struct options *op;
struct chardata *cp;
struct termio *tp;
{
	static char logname[BUFSIZ];
	char *bp;
	char c;						/* input character, full eight bits */
	char ascval;				/* low 7 bits of input character */
	int bits;					/* # of "1" bits per character */
	int mask;					/* mask with 1 bit up */
	static char *erase[] = {	/* backspace-space-backspace */
		"\010\040\010",			/* space parity */
		"\010\040\010",			/* odd parity */
		"\210\240\210",			/* even parity */
		"\210\240\210",			/* no parity */
	};

	/* Initialize kill, erase, parity etc. (also after switching speeds). */

	*cp = init_chardata;

	/* Flush pending input (esp. after parsing or switching the baud rate). */

	(void) sleep(1);
	(void) ioctl(0, TCFLSH, TCIFLUSH);

	/* Prompt for and read a login name. */

	for (*logname = 0; *logname == 0; /* void */ ) {

		/* Write issue file and prompt, with "parity" bit == 0. */

		do_prompt(op, tp);

		/* Read name, watch for break, parity, erase, kill, end-of-line. */

		for (bp = logname, cp->eol = 0; cp->eol == 0; /* void */ ) {

			/* Do not report trivial EINTR/EIO errors. */

			if (read(0, &c, 1) < 1) {
				if (errno == EINTR || errno == EIO)
					exit(0);
				error("%s: read: %m", op->tty);
			}
			/* Do BREAK handling elsewhere. */

			if ((c == 0) && op->numspeed > 1)
				/* return (0); */
				return NULL;

			/* Do parity bit handling. */

			if (c != (ascval = (c & 0177))) {	/* "parity" bit on ? */
				for (bits = 1, mask = 1; mask & 0177; mask <<= 1)
					if (mask & ascval)
						bits++;	/* count "1" bits */
				cp->parity |= ((bits & 1) ? 1 : 2);
			}
			/* Do erase, kill and end-of-line processing. */

			switch (ascval) {
			case CR:
			case NL:
				*bp = 0;		/* terminate logname */
				cp->eol = ascval;	/* set end-of-line char */
				break;
			case BS:
			case DEL:
			case '#':
				cp->erase = ascval;	/* set erase character */
				if (bp > logname) {
					(void) write(1, erase[cp->parity], 3);
					bp--;
				}
				break;
			case CTL('U'):
			case '@':
				cp->kill = ascval;	/* set kill character */
				while (bp > logname) {
					(void) write(1, erase[cp->parity], 3);
					bp--;
				}
				break;
			case CTL('D'):
				exit(0);
			default:
				if (!isascii(ascval) || !isprint(ascval)) {
					/* ignore garbage characters */ ;
				} else if (bp - logname >= sizeof(logname) - 1) {
					error("%s: input overrun", op->tty);
				} else {
					(void) write(1, &c, 1);	/* echo the character */
					*bp++ = ascval;	/* and store it */
				}
				break;
			}
		}
	}
	/* Handle names with upper case and no lower case. */

	if ((cp->capslock = caps_lock(logname))) {
		for (bp = logname; *bp; bp++)
			if (isupper(*bp))
				*bp = tolower(*bp);	/* map name to lower case */
	}
	return (logname);
}

/* termio_final - set the final tty mode bits */
void termio_final(op, tp, cp)
struct options *op;
struct termio *tp;
struct chardata *cp;
{
	/* General terminal-independent stuff. */

	tp->c_iflag |= IXON | IXOFF;	/* 2-way flow control */
	tp->c_lflag |= ICANON | ISIG | ECHO | ECHOE | ECHOK | ECHOKE;
	/* no longer| ECHOCTL | ECHOPRT */
	tp->c_oflag |= OPOST;
	/* tp->c_cflag = 0; */
	tp->c_cc[VINTR] = DEF_INTR;	/* default interrupt */
	tp->c_cc[VQUIT] = DEF_QUIT;	/* default quit */
	tp->c_cc[VEOF] = DEF_EOF;	/* default EOF character */
	tp->c_cc[VEOL] = DEF_EOL;
#ifdef __linux__
	tp->c_cc[VSWTC] = DEF_SWITCH;	/* default switch character */
#else
	tp->c_cc[VSWTCH] = DEF_SWITCH;	/* default switch character */
#endif

	/* Account for special characters seen in input. */

	if (cp->eol == CR) {
		tp->c_iflag |= ICRNL;	/* map CR in input to NL */
		tp->c_oflag |= ONLCR;	/* map NL in output to CR-NL */
	}
	tp->c_cc[VERASE] = cp->erase;	/* set erase character */
	tp->c_cc[VKILL] = cp->kill;	/* set kill character */

	/* Account for the presence or absence of parity bits in input. */

	switch (cp->parity) {
	case 0:					/* space (always 0) parity */
		break;
	case 1:					/* odd parity */
		tp->c_cflag |= PARODD;
		/* FALLTHROUGH */
	case 2:					/* even parity */
		tp->c_cflag |= PARENB;
		tp->c_iflag |= INPCK | ISTRIP;
		/* FALLTHROUGH */
	case (1 | 2):				/* no parity bit */
		tp->c_cflag &= ~CSIZE;
		tp->c_cflag |= CS7;
		break;
	}
	/* Account for upper case without lower case. */

	if (cp->capslock) {
		tp->c_iflag |= IUCLC;
		tp->c_lflag |= XCASE;
		tp->c_oflag |= OLCUC;
	}
	/* Optionally enable hardware flow control */

#ifdef	CRTSCTS
	if (op->flags & F_RTSCTS)
		tp->c_cflag |= CRTSCTS;
#endif

	/* Finally, make the new settings effective */

	if (ioctl(0, TCSETA, tp) < 0)
		error("%s: ioctl: TCSETA: %m", op->tty);
}

/* caps_lock - string contains upper case without lower case */
/* returns 1 if true, 0 if false */
int caps_lock(s)
char *s;
{
	int capslock;

	for (capslock = 0; *s; s++) {
		if (islower(*s))
			return (0);
		if (capslock == 0)
			capslock = isupper(*s);
	}
	return (capslock);
}

/* bcode - convert speed string to speed code; return 0 on failure */
int bcode(s)
char *s;
{
	struct Speedtab *sp;
	long speed = atol(s);

	for (sp = speedtab; sp->speed; sp++)
		if (sp->speed == speed)
			return (sp->code);
	return (0);
}

/* error - report errors to console or syslog; only understands %s and %m */

#define	str2cpy(b,s1,s2)	strcat(strcpy(b,s1),s2)

/*
 * output error messages
 */
static void error(const char *fmt, ...)
{
	va_list va_alist;
	char buf[256], *bp;

#ifndef	USE_SYSLOG
	int fd;
#endif

#ifdef USE_SYSLOG
	buf[0] = '\0';
	bp = buf;
#else
	strncpy(buf, progname, 256);
	strncat(buf, ": ", 256);
	buf[255] = 0;
	bp = buf + strlen(buf);
#endif

	va_start(va_alist, fmt);
	vsnprintf(bp, 256 - strlen(buf), fmt, va_alist);
	buf[255] = 0;
	va_end(va_alist);

#ifdef	USE_SYSLOG
	openlog(progname, LOG_PID, LOG_AUTH);
	syslog(LOG_ERR, buf);
	closelog();
#else
	strncat(bp, "\r\n", 256 - strlen(buf));
	buf[255] = 0;
	if ((fd = open("/dev/console", 1)) >= 0) {
		write(fd, buf, strlen(buf));
		close(fd);
	}
#endif
	(void) sleep((unsigned) 10);	/* be kind to init(8) */
	exit(1);
}
