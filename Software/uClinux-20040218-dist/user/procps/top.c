/*
 * top.c              - show top CPU processes
 *
 * Copyright (c) 1992 Branko Lankester
 * Copyright (c) 1992 Roger Binns
 * Copyright (c) 1997 Michael K. Johnson
 *
 * Snarfed and HEAVILY modified in december 1992 for procps
 * by Michael K. Johnson, johnsonm@sunsite.unc.edu.
 *
 * Modified Michael K. Johnson's ps to make it a top program.
 * Also borrowed elements of Roger Binns kmem based top program.
 * Changes made by Robert J. Nation (nation@rocket.sanders.lockheed.com)
 * 1/93
 *
 * Modified by Michael K. Johnson to be more efficient in cpu use
 * 2/21/93
 *
 * Changed top line to use uptime for the load average.  Also
 * added SIGTSTP handling.  J. Cowley, 19 Mar 1993.
 *
 * Modified quite a bit by Michael Shields (mjshield@nyx.cs.du.edu)
 * 1994/04/02.  Secure mode added.  "d" option added.  Argument parsing
 * improved.  Switched order of tick display to user, system, nice, idle,
 * because it makes more sense that way.  Style regularized (to K&R,
 * more or less).  Cleaned up much throughout.  Added cumulative mode.
 * Help screen improved.
 *
 * Fixed kill buglet brought to my attention by Rob Hooft.
 * Problem was mixing of stdio and read()/write().  Added
 * getnum() to solve problem.
 * 12/30/93 Michael K. Johnson
 *
 * Added toggling output of idle processes via 'i' key.
 * 3/29/94 Gregory K. Nickonov
 *
 * Fixed buglet where rawmode wasn't getting restored.
 * Added defaults for signal to send and nice value to use.
 * 5/4/94 Jon Tombs.
 *
 * Modified 1994/04/25 Michael Shields <mjshield@nyx.cs.du.edu>
 * Merged previous changes to 0.8 into 0.95.
 * Allowed the use of symbolic names (e.g., "HUP") for signal input.
 * Rewrote getnum() into getstr(), getint(), getsig(), etc.
 * 
 * Modified 1995  Helmut Geyer <Helmut.Geyer@iwr.uni-heidelberg.de> 
 * added kmem top functionality (configurable fields)
 * configurable order of process display
 * Added options for dis/enabling uptime, statistics, and memory info.
 * fixed minor bugs for ELF systems (e.g. SIZE, RSS fields)
 *
 * Modified 1996/05/18 Helmut Geyer <Helmut.Geyer@iwr.uni-heidelberg.de>
 * Use of new interface and general cleanup. The code should be far more
 * readable than before.
 *
 * Modified 1996/06/25 Zygo Blaxell <zblaxell@ultratech.net>
 * Added field scaling code for programs that run more than two hours or
 * take up more than 100 megs.  We have lots of both on our production line.
 *
 * Modified 1998/02/21 Kirk Bauer <kirk@kaybee.org>
 * Added the 'u' option to display only a selected user... plus it will
 * take into account that not all 20 top processes are actually shown,
 * so it can fit more onto the screen.  I think this may help the
 * 'don't show idle' mode, but I'm not sure.
 *
 * Modified 1997/07/27 & 1999/01/27 Tim Janik <timj@gtk.org>
 * added `-p' option to display specific process ids.
 * process sorting is by default disabled in this case.
 * added `N' and `A' keys to sort the tasks Numerically by pid or
 * sort them by Age (newest first).
 *
 * Modified 1999/10/22 Tim Janik <timj@gtk.org>
 * miscellaneous minor fixes, including "usage: ..." output for
 * unrecognized options.
 *
 * Modified 2000/02/07 Jakub Jelinek <jakub@redhat.com>
 * Only load System.map when we are going to display WCHAN.
 * Show possible error messages from that load using SHOWMESSAGE.
 *
 * Modified 2000/07/10 Michael K. Johnson <johnsonm@redhat.com>
 * Integrated a patch to display SMP information.
 */

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <libintl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <pwd.h>
#ifdef TINY_TCAP
#include "tinytcap.h"
#else
#include <termcap.h>
#endif
#include <termios.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <sys/param.h>

#include "proc/sysinfo.h"
#include "proc/procps.h"
#include "proc/whattime.h"
#include "proc/signals.h"
#include "proc/version.h"
#include "proc/readproc.h"
#include "proc/status.h"
#include "proc/devname.h"
/* these should be in the readproc.h header or in the procps.h header */
typedef int (*cmp_t)(void*,void*);
extern void reset_sort_options (void);
extern int parse_sort_opt(char* opt);
extern void register_sort_function (int dir, cmp_t func);

#define PUTP(x) (tputs(x,1,putchar))
#define BAD_INPUT -30

#include "top.h"  /* new header for top specific things */

static int *cpu_mapping;
static int nr_cpu;

/*#######################################################################
 *####  Startup routines: parse_options, get_options,      ##############
 *####                    setup_terminal and main          ##############
 *#######################################################################
 */

      /*
       * parse the options string as read from the config file(s).
       * if top is in secure mode, disallow changing of the delay time between
       * screen updates.
       */
void parse_options(char *Options, int secure)
{
    int i;
    for (i = 0; i < strlen(Options); i++) {
	switch (Options[i]) {
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    if (!secure)
		Sleeptime = (float) Options[i] - '0';
	    break;
	  case 'S':
	    Cumulative = 1;
	    headers[22][1] = 'C';
	    break;
	  case 's':
	    Secure = 1;
	    break;
	  case 'i':
	    Noidle = 1;
	    break;
	  case 'm':
	    show_memory = 0;
	    header_lines -= 2;
	    break;
	  case 'M':
	    sort_type = S_MEM;
	    reset_sort_options();
	    register_sort_function( -1, (cmp_t)mem_sort);
	    break;
	  case 'l':
	    show_loadav = 0;
	    header_lines -= 1;
	    break;
	  case 'P':
	    sort_type = S_PCPU;
	    reset_sort_options();
	    register_sort_function( -1, (cmp_t)pcpu_sort);
	    break;
	  case 'N':
	    sort_type = S_NONE;
	    reset_sort_options();
	    break;
	  case 'A':
	    sort_type = S_AGE;
	    reset_sort_options();
	    register_sort_function( -1, (cmp_t)age_sort);
	    break;
	  case 't':
	    show_stats = 0;
	    header_lines -= 2;
	    break;
	  case 'T':
	    sort_type = S_TIME;
	    reset_sort_options();
	    register_sort_function( -1, (cmp_t)time_sort);
	    break;
	  case 'c':
	    show_cmd = 0;
	    break;
	  case '\n':
	    break;
	  case 'I':
	    Irixmode = 0;
	    break;
	  default:
	    fprintf(stderr, "Wrong configuration option %c\n", i);
	    exit(1);
	    break;
	}
    }
}

/* 
 * Read the configuration file(s). There are two files, once SYS_TOPRC 
 * which should only contain the secure switch and a sleeptime
 * value iff ordinary users are to use top in secure mode only.
 * 
 * The other file is $HOME/RCFILE. 
 * The configuration file should contain two lines (any of which may be
 *  empty). The first line specifies the fields that are to be displayed
 * in the order you want them to. Uppercase letters specify fields 
 * displayed by default, lowercase letters specify fields not shown by
 * default. The order of the letters in this line corresponds to the 
 * order of the displayed fileds.
 *
 * all Options but 'q' can be read from this config file
 * The delay time option syntax differs from the commandline syntax:
 *   only integer values between 2 and 9 seconds are recognized
 *   (this is for standard configuration, so I think this should do).
 *
 * usually this file is not edited by hand, but written from top using
 * the 'W' command. 
 */

void get_options(void)
{
    FILE *fp;
    char *pt;
    char rcfile[MAXNAMELEN];
    char Options[256] = "";
    int i;

    nr_cpu = sysconf (_SC_NPROCESSORS_ONLN);
    cpu_mapping = (int *) xmalloc (sizeof (int) * nr_cpu);
    /* read cpuname */
    for (i=0; i< nr_cpu; i++) cpu_mapping[i]=i;
    header_lines = 6 + nr_cpu;
    strcpy(rcfile, SYS_TOPRC);
    fp = fopen(rcfile, "r");
    if (fp != NULL) {
	fgets(Options, 254, fp);
	fclose(fp);
    }
    parse_options(Options, 0);
    strcpy(Options, "");
    if (getenv("HOME")) {
	strcpy(rcfile, getenv("HOME"));
	strcat(rcfile, "/");
    }
    strcat(rcfile, RCFILE);
    fp = fopen(rcfile, "r");
    if (fp == NULL) {
	strcpy(Fields, DEFAULT_SHOW);
    } else {
	if (fgets(Fields, 254, fp) != NULL) {
	    pt = strchr(Fields, '\n');
	    if (pt) *pt = 0;
	}
	fgets(Options, 254, fp);
	fclose(fp);
    }
    parse_options(Options, getuid()? Secure : 0);
}

/*
     * Set up the terminal attributes.
     */
void setup_terminal(void)
{
    char *termtype;
    struct termios newtty;
    if (!Batch)
    	termtype = getenv("TERM");
    else 
	termtype = "dumb";
    if (!termtype) {
	/* In theory, $TERM should never not be set, but in practice,
	   some gettys don't.  Fortunately, vt100 is nearly always
	   correct (or pretty close). */
	termtype = "VT100";
	/* fprintf(stderr, PROGNAME ": $TERM not set\n"); */
	/* exit(1); */
    }

    /*
     * Get termcap entries and window size.
     */
    if(tgetent(NULL, termtype) != 1) {
       fprintf(stderr, PROGNAME ": Unknown terminal \"%s\" in $TERM\n",
               termtype);
       exit(1);
    }

    cm = tgetstr("cm", 0);
    top_clrtobot = tgetstr("cd", 0);
    cl = tgetstr("cl", 0);
    top_clrtoeol = tgetstr("ce", 0);
    ho = tgetstr("ho", 0);
    md = tgetstr("md", 0);
    mr = tgetstr("mr", 0);
    me = tgetstr("me", 0);


    if (Batch) return; /* the rest doesn't apply to batch mode */
    if (tcgetattr(0, &Savetty) == -1) {
        perror(PROGNAME ": tcgetattr() failed");
	error_end(errno);
    }
    newtty = Savetty;
    newtty.c_lflag &= ~ICANON;
    newtty.c_lflag &= ~ECHO;
    newtty.c_cc[VMIN] = 1;
    newtty.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSAFLUSH, &newtty) == -1) {
	printf("cannot put tty into raw mode\n");
	error_end(1);
    }
    tcgetattr(0, &Rawtty);
}

int main(int argc, char **argv)
{
    /* For select(2). */
    struct timeval tv;
    fd_set in;
    /* For parsing arguments. */
    char *cp;
    /* The key read in. */
    char c;

    struct sigaction sact;

    setlocale(LC_ALL, "");
    get_options();
    
    /* set to PCPU sorting */
    register_sort_function( -1, (cmp_t)pcpu_sort);
    
#ifdef HZ
    Hertz = HZ;
#endif

    /*
     * Parse arguments.
     */
    argv++;
    while (*argv) {
	cp = *argv++;
	while (*cp) {
	    switch (*cp) {
	      case 'd':
	        if (cp[1]) {
		    if (sscanf(++cp, "%f", &Sleeptime) != 1) {
			fprintf(stderr, PROGNAME ": Bad delay time `%s'\n", cp);
			exit(1);
		    }
		    goto breakargv;
		} else if (*argv) { /* last char in an argv, use next as arg */
		    if (sscanf(cp = *argv++, "%f", &Sleeptime) != 1) {
			fprintf(stderr, PROGNAME ": Bad delay time `%s'\n", cp);
			exit(1);
		    }
		    goto breakargv;
		} else {
		    fprintf(stderr, "-d requires an argument\n");
		    exit(1);
		}
		break;
	      case 'n':
		if (cp[1]) {
	   	    if (sscanf(++cp, "%d", &Loops) != 1) {
			fprintf(stderr, PROGNAME ": Bad value `%s'\n", cp);
			exit(1);
		    }
		    goto breakargv;
		} else if (*argv) { /* last char in an argv, use next as arg */
		    if (sscanf(cp = *argv++, "%d", &Loops) != 1) {
			fprintf(stderr, PROGNAME ": Bad value `%s'\n", cp);
			 exit(1);
	 	     }
		     goto breakargv;
		}
		break;
					
	      case 'q':
		if (!getuid())
		    /* set priority to -10 in order to stay above kswapd */
		    if (setpriority(PRIO_PROCESS, getpid(), -10)) {
			/* We check this just for paranoia.  It's not
			   fatal, and shouldn't happen. */
			perror(PROGNAME ": setpriority() failed");
		    }
		Sleeptime = 0;
		break;
	      case 'p':
		if (monpids_index >= monpids_max) {
		    fprintf(stderr, PROGNAME ": More than %u process ids specified\n",
			    monpids_max);
		    exit(1);
		}
		if (cp[1]) {
		    if (sscanf(++cp, "%d", &monpids[monpids_index]) != 1 ||
			monpids[monpids_index] < 0 || monpids[monpids_index] > 65535) {
			fprintf(stderr, PROGNAME ": Bad process id `%s'\n", cp);
			exit(1);
		    }
		} else if (*argv) { /* last char in an argv, use next as arg */
		    if (sscanf(cp = *argv++, "%d", &monpids[monpids_index]) != 1 ||
			monpids[monpids_index] < 0 || monpids[monpids_index] > 65535) {
			fprintf(stderr, PROGNAME ": Bad process id `%s'\n", cp);
			exit(1);
		    }
		} else {
		    fprintf(stderr, "-p requires an argument\n");
		    exit(1);
		}
		if (!monpids[monpids_index])
		    monpids[monpids_index] = getpid();
		/* default to no sorting when monitoring process ids */
		if (!monpids_index++) {
		    sort_type = S_NONE;
		    reset_sort_options();
		}
		cp = "_";
		break;
	      case 'b':
		Batch = 1;
	        break;
	      case 'c':
	        show_cmd = !show_cmd;
		break;
	      case 'S':
		Cumulative = 1;
		break;
	      case 'i':
		Noidle = 1;
		break;
	      case 's':
		  Secure = 1;
		  break;
	      case 'C':
		  CPU_states = 1;
		  break;
	      case '-':
		break;		/* Just ignore it */
 	      case 'v':
 	      case 'V':
 		fprintf(stdout, "top (%s)\n", procps_version);
 		exit(0);
	      case 'h':
 		fprintf(stdout, "usage: " PROGNAME " -hvbcisqS -d delay -p pid -n iterations\n");
		exit(0);
	      default:
		fprintf(stderr, PROGNAME ": Unknown argument `%c'\n", *cp);
 		fprintf(stdout, "usage: " PROGNAME " -hvbcisqS -d delay -p pid -n iterations\n");
 	        exit(1);
	    }
	    cp++;
	}
    breakargv:;
    }
    
    if (nr_cpu > 1 && CPU_states)
      header_lines++;

    setup_terminal();
    window_size(0);
    /*
     * Set up signal handlers.
     */
    sact.sa_handler = end;
    sact.sa_flags = 0;
    sigemptyset(&sact.sa_mask);
    sigaction(SIGHUP, &sact, NULL);
    sigaction(SIGINT, &sact, NULL);
    sigaction(SIGQUIT, &sact, NULL);
    sact.sa_handler = stop;
    sact.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sact, NULL);
    sact.sa_handler = window_size;
    sigaction(SIGWINCH, &sact, NULL);
    sigaction(SIGCONT, &sact, NULL);

    /* loop, collecting process info and sleeping */
    while (1) {
	if (Loops > 0)
		Loops--;
	/* display the tasks */
	show_procs();
	/* sleep & wait for keyboard input */
	if (Loops == 0)
	    end(0);
        if (!Batch)
        {
		tv.tv_sec = Sleeptime;
		tv.tv_usec = (Sleeptime - (int) Sleeptime) * 1000000;
		FD_ZERO(&in);
		FD_SET(0, &in);
		if (select(1, &in, 0, 0, &tv) > 0 && read(0, &c, 1) == 1)
	    		do_key(c);
        } else {
	   sleep(Sleeptime);
	}
    }
}

/*#######################################################################
 *#### Signal handled routines: error_end, end, stop, window_size     ###
 *#### Small utilities: make_header, getstr, getint, getfloat, getsig ###
 *#######################################################################
 */


	/*
	 *  end when exiting with an error.
	 */
void error_end(int rno)
{
    if (psdbsucc)
        close_psdb();
    if (!Batch)
	tcsetattr(0, TCSAFLUSH, &Savetty);
    PUTP(tgoto(cm, 0, Lines - 1));
    fputs("\r\n", stdout);
    exit(rno);
}
/*
	 * Normal end of execution.
	 */
void end(int signo)
{
    if (psdbsucc)
	close_psdb();
    if (!Batch)
	tcsetattr(0, TCSAFLUSH, &Savetty);
    PUTP(tgoto(cm, 0, Lines - 1));
    fputs("\r\n", stdout);
    exit(0);
}

/*
	 * SIGTSTP catcher.
	 */
void stop(int signo)
{
    /* Reset terminal. */
    if (!Batch)
	tcsetattr(0, TCSAFLUSH, &Savetty);
    PUTP(tgoto(cm, 0, Lines - 3));
    fflush(stdout);
    raise(SIGSTOP);
    /* Later... */
    if (!Batch)
	tcsetattr (0, TCSAFLUSH, &Rawtty);
}

/*
       * Reads the window size and clear the window.  This is called on setup,
       * and also catches SIGWINCHs, and adjusts Maxlines.  Basically, this is
       * the central place for window size stuff.
       */
void window_size(int signo)
{
    struct winsize ws;

    if((ioctl(1, TIOCGWINSZ, &ws) != -1) && (ws.ws_col>73) && (ws.ws_row>7)){
	Cols = ws.ws_col;
	Lines = ws.ws_row;
    }else{
	Cols = tgetnum("co");
	Lines = tgetnum("li");
    }
    if (!Batch)
        clear_screen();
    /*
     * calculate header size, length of cmdline field ...
     */
     Numfields = make_header();
}

/*
	* this prints a possible message from open_psdb_message
	*/
static void top_message(const char *format, ...) {
    va_list arg;
    int n;
    char buffer[512];

    va_start (arg, format);
    n = vsnprintf (buffer, 512, format, arg);
    va_end (arg);
    if (n > -1 && n < 512)
	SHOWMESSAGE(("%s", buffer));
}

/*
       * this adjusts the lines needed for the header to the current value
       */
int make_header(void)
{
    int i, j;

    j = 0;
    for (i = 0; i < strlen(Fields); i++) {
	if (Fields[i] < 'a') {
	    pflags[j++] = Fields[i] - 'A';
	    if (Fields[i] == 'U' && CL_wchan_nout == -1) {
		CL_wchan_nout = 0;
		/* for correct handling of WCHAN fields, we have to do distinguish 
		 * between kernel versions */
		/* get kernel symbol table, if needed */
		if (open_psdb_message(NULL, top_message)) {
		    CL_wchan_nout = 1;
		} else {
		    psdbsucc = 1;
		}
	    }
	}
    }
    strcpy(Header, "");
    for (i = 0; i < j; i++)
	strcat(Header, headers[pflags[i]]);
    /* readjust window size ... */
    Maxcmd = Cols - strlen(Header) + 7;
    Maxlines = Display_procs ? Display_procs : Lines - header_lines;
    if (Maxlines > Lines - header_lines)
	Maxlines = Lines - header_lines;
    return (j);
}



/*
 * Get a string from the user; the base of getint(), et al.  This really
 * ought to handle long input lines and errors better.  NB: The pointer
 * returned is a statically allocated buffer, so don't expect it to
 * persist between calls.
 */
char *getstr(void)
{
    static char line[BUFSIZ];	/* BUFSIZ from <stdio.h>; arbitrary */
    int i = 0;

    /* Must make sure that buffered IO doesn't kill us. */
    fflush(stdout);
    fflush(stdin);		/* Not POSIX but ok */

    do {
	read(STDIN_FILENO, &line[i], 1);
    } while (line[i++] != '\n' && i < sizeof(line));
    line[--i] = 0;

    return (line);
}


/*
 * Get an integer from the user.  Display an error message and return BAD_INPUT
 * if it's invalid; else return the number.
 */
int getint(void)
{
    char *line;
    int i;
    int r;

    line = getstr();

    for (i = 0; line[i]; i++) {
	if (!isdigit(line[i]) && line[i] != '-') {
	    SHOWMESSAGE(("That's not a number!"));
	    return (BAD_INPUT);
	}
    }

    /* An empty line is a legal error (hah!). */
    if (!line[0])
	return (BAD_INPUT);

    sscanf(line, "%d", &r);
    return (r);
}


/*
	 * Get a float from the user.  Just like getint().
	 */
float getfloat(void)
{
    char *line;
    int i;
    float r;

    line = getstr();

    for (i = 0; line[i]; i++) {
	if (!isdigit(line[i]) && line[i] != '.' && line[i] != '-') {
	    SHOWMESSAGE(("That's not a float!"));
	    return (BAD_INPUT);
	}
    }

    /* An empty line is a legal error (hah!). */
    if (!line[0])
	return (BAD_INPUT);

    sscanf(line, "%f", &r);
    return (r);
}


/*
	 * Get a signal number or name from the user.  Return the number, or -1
	 * on error.
	 */
int getsig(void)
{
    char *line;

    /* This is easy. */
    line = getstr();
    return (get_signal2(line));
}

/*#######################################################################
 *####  Routine for sorting on used time, resident memory and %CPU  #####
 *####  It would be easy to include full sorting capability as in   #####
 *####  ps, but I think there is no real use for something that     #####
 *####  complicated. Using register_sort_function or parse_sort_opt #####
 *####  you just have to do the natural thing and it will work.     #####
 *#######################################################################
 */

int time_sort (proc_t **P, proc_t **Q)
{
    if (Cumulative) {
	if( ((*P)->cutime + (*P)->cstime + (*P)->utime + (*P)->stime) < 
	    ((*Q)->cutime + (*Q)->cstime + (*Q)->utime + (*Q)->stime) )
	    return -1;
	if( ((*P)->cutime + (*P)->cstime + (*P)->utime + (*P)->stime) >
	    ((*Q)->cutime + (*Q)->cstime + (*Q)->utime + (*Q)->stime) )
	    return 1;
    } else {
	if( ((*P)->utime + (*P)->stime) < ((*Q)->utime + (*Q)->stime))
	    return -1;
	if( ((*P)->utime + (*P)->stime) > ((*Q)->utime + (*Q)->stime))
	    return 1;
    }
    return 0;
}

int pcpu_sort (proc_t **P, proc_t **Q)
{
    if( (*P)->pcpu < (*Q)->pcpu )      return -1;
    if( (*P)->pcpu > (*Q)->pcpu )      return 1;
    return 0;
}

int mem_sort (proc_t **P, proc_t **Q)
{
    if( (*P)->resident < (*Q)->resident )      return -1;
    if( (*P)->resident > (*Q)->resident )      return 1;  
    return 0;
}

int age_sort (proc_t **P, proc_t **Q)
{
    if( (*P)->start_time < (*Q)->start_time )      return -1;
    if( (*P)->start_time > (*Q)->start_time )      return 1;
    return 0;
}

/*#######################################################################
 *####  Routines handling the field selection/ordering screens:  ########
 *####    show_fields, change_order, change_fields               ########
 *#######################################################################
 */

        /*
	 * Display the specification line of all fields. Upper case indicates
	 * a displayed field, display order is according to the order of the 
	 * letters. A short description of each field is shown as well.
	 * The description of a displayed field is marked by a leading 
	 * asterisk (*).
	 */
void show_fields(void)
{
    int i, row, col;
    char *p;

    clear_screen();
    PUTP(tgoto(cm, 3, 0));
    printf("Current Field Order: %s\n", Fields);
    for (i = 0; i < sizeof headers / sizeof headers[0]; ++i) {
	row = i % (Lines - 3) + 3;
	col = i / (Lines - 3) * 40;
	PUTP(tgoto(cm, col, row));
	for (p = headers[i]; *p == ' '; ++p);
	printf("%c %c: %-10s = %s", (strchr(Fields, i + 'A') != NULL) ? '*' : ' ', i + 'A',
	       p, headers2[i]);
    }
}

/*
	 * change order of displayed fields
	 */
void change_order(void)
{
    char c, ch, *p;
    int i;

    show_fields();
    for (;;) {
	PUTP(tgoto(cm, 0, 0));
	PUTP(top_clrtoeol);
	PUTP(tgoto(cm, 3, 0));
	PUTP(mr);
	printf("Current Field Order: %s", Fields);
	PUTP(me);
	putchar('\n');
	PUTP(tgoto(cm, 0, 1));
	printf("Upper case characters move a field to the left, lower case to the right");
	fflush(stdout);
	if (!Batch) { /* should always be true, but... */
	    tcsetattr(0, TCSAFLUSH, &Rawtty);
	    read(0, &c, 1);
	    tcsetattr(0, TCSAFLUSH, &Savetty);
	}
	i = toupper(c) - 'A';
	if ((p = strchr(Fields, i + 'A')) != NULL) {
	    if (isupper(c))
		p--;
	    if ((p[1] != '\0') && (p >= Fields)) {
		ch = p[0];
		p[0] = p[1];
		p[1] = ch;
	    }
	} else if ((p = strchr(Fields, i + 'a')) != NULL) {
	    if (isupper(c))
		p--;
	    if ((p[1] != '\0') && (p >= Fields)) {
		ch = p[0];
		p[0] = p[1];
		p[1] = ch;
	    }
	} else {
	    break;
	}
    }
    Numfields = make_header();
}
/*
	 * toggle displayed fields
	 */
void change_fields(void)
{
    int i, changed = 0;
    int row, col;
    char c, *p;
    char tmp[2] = " ";

    show_fields();
    for (;;) {
	PUTP(tgoto(cm, 0, 0));
	PUTP(top_clrtoeol);
	PUTP(tgoto(cm, 3, 0));
	PUTP(mr);
	printf("Current Field Order: %s", Fields);
	PUTP(me);
	putchar('\n');
	PUTP(tgoto(cm, 0, 1));
	printf("Toggle fields with a-x, any other key to return: ");
	fflush(stdout);
	if (!Batch) { /* should always be true, but... */
	    tcsetattr(0, TCSAFLUSH, &Rawtty);
	    read(0, &c, 1);
	    tcsetattr(0, TCSAFLUSH, &Savetty);
	}
	i = toupper(c) - 'A';
	if (i >= 0 && i < sizeof headers / sizeof headers[0]) {
	    row = i % (Lines - 3) + 3;
	    col = i / (Lines - 3) * 40;
	    PUTP(tgoto(cm, col, row));
	    if ((p = strchr(Fields, i + 'A')) != NULL) {	/* deselect Field */
		*p = i + 'a';
		putchar(' ');
	    } else if ((p = strchr(Fields, i + 'a')) != NULL) {		/* select previously */
		*p = i + 'A';	/* deselected field */
		putchar('*');
	    } else {		/* select new field */
		tmp[0] = i + 'A';
		strcat(Fields, tmp);
		putchar('*');
	    }
	    changed = 1;
	    fflush(stdout);
	} else
	    break;
    }
    if (changed)
	Numfields = make_header();
}

/* Do the scaling stuff
   scale_time(time,width) - interprets time in seconds, formats it to fit width

   Both return pointer to static char*.
*/

char *scale_time(int time,int width) 
{
	static char buf[100];

	/* Try successively higher units until it fits */

	sprintf(buf,"%d:%02d",time/60,time%60);	/* minutes:seconds */
	if (strlen(buf)<=width) 
		return buf;

	time/=60;	/* minutes */
	sprintf(buf,"%dm",time);
	if (strlen(buf)<=width) 
		return buf;

	time/=60;	/* hours */
	sprintf(buf,"%dh",time);
	if (strlen(buf)<=width) 
		return buf;

	time/=24;	/* days */
	sprintf(buf,"%dd",time);
	if (strlen(buf)<=width) 
		return buf;
	
	time/=7;	/* weeks */
	sprintf(buf,"%dw",time);
	return buf;	/* this is our last try; 
				if it still doesn't fit, too bad. */

	/* :-) I suppose if someone has a SMP version of Linux with a few
           thousand processors, they could accumulate 18 years of CPU time... */
           
}

/*   scale_k(k,width,unit)  - interprets k as a count, formats to fit width.
                            if unit is 0, k is a byte count; 1 is a kilobyte
			    count; 2 for megabytes; 3 for gigabytes.
*/

char *scale_k(int k,int width,int unit) 
{
		/* kilobytes, megabytes, gigabytes, too-big-for-int-bytes */
	static double scale[]={1024,1024*1024,1024*1024*1024,0};
		/* kilo, mega, giga, tera */
	static char unitletters[]={'K','M','G','T',0};
	static char buf[100];
	char *up;
	double *dp;

	/* Try successively higher units until it fits */

	sprintf(buf,"%d",k);
	if (strlen(buf)<=width) 
		return buf;

	for (up=unitletters+unit,dp=scale ; *dp ; ++dp,++up) {
		sprintf(buf,"%.1f%c",k / *dp,*up);
		if (strlen(buf)<=width) 
			return buf;
		sprintf(buf,"%d%c",(int)(k / *dp),*up);
		if (strlen(buf)<=width) 
			return buf;
	}

	/* Give up; give them what we got on our shortest attempt */
	return buf;
}

/*
 *#######################################################################
 *####  Routines handling the main top screen:                   ########
 *####    show_task_info, show_procs, show_memory, do_stats      ########
 *#######################################################################
 */
	/*
	 * Displays infos for a single task
	 */
void show_task_info(proc_t *task, int pmem)
{
    int i,j;
    unsigned int t;
    char *cmdptr;
    char tmp[2048], tmp2[2048] = "", tmp3[2048] = "", *p;

    for (i = 0; i < Numfields; i++) {
	tmp[0] = 0;
	switch (pflags[i]) {
	  case P_PID:
	    sprintf(tmp, "%5d ", task->pid);
	    break;
	  case P_PPID:
	    sprintf(tmp, "%5d ", task->ppid);
	    break;
	  case P_EUID:
	    sprintf(tmp, "%4d ", task->euid);
	    break;
	  case P_EUSER:
	    sprintf(tmp, "%-8.8s ", task->euser);
	    break;
	  case P_PCPU:
	    sprintf(tmp, "%4.1f ", (float)task->pcpu / 10);
	    break;
	  case P_LCPU:
	    sprintf(tmp, "%2d ", task->lproc);
	    break;
	  case P_PMEM:
	    sprintf(tmp, "%4.1f ", (float)pmem / 10);
	    break;
	  case P_TTY: {
	      char outbuf[9];
	      dev_to_tty(outbuf, 8, task->tty, task->pid, ABBREV_DEV);
	      sprintf(tmp, "%-8.8s ", outbuf);
	    }
	    break;
	  case P_PRI:
	    sprintf(tmp, "%3.3s ", scale_k(task->priority, 3, 0));
	    break;
	  case P_NICE:
	    sprintf(tmp, "%3.3s ", scale_k(task->nice, 3, 0));
	    break;
	  case P_PAGEIN:
	    sprintf(tmp, "%6.6s ", scale_k(task->maj_flt, 6, 0));
	    break;
	  case P_TSIZ:
	    sprintf(tmp, "%5.5s ",
		scale_k(((task->end_code - task->start_code) / 1024), 5, 1));
	    break;
	  case P_DSIZ:
	    sprintf(tmp, "%5.5s ",
		scale_k(((task->vsize - task->end_code) / 1024), 5, 1));
	    break;
	  case P_SIZE:
	    sprintf(tmp, "%5.5s ", scale_k((task->size << CL_pg_shift), 5, 1));
	    break;
	  case P_TRS:
	    sprintf(tmp, "%4.4s ", scale_k((task->trs << CL_pg_shift), 4, 1));
	    break;
	  case P_SWAP:
	    sprintf(tmp, "%4.4s ",
		scale_k(((task->size - task->resident) << CL_pg_shift), 4, 1));
	    break;
	  case P_SHARE:
	    sprintf(tmp, "%5.5s ", scale_k((task->share << CL_pg_shift), 5, 1));
	    break;
	  case P_A:
	    sprintf(tmp, "%3.3s ", "NYI");
	    break;
	  case P_WP:
	    sprintf(tmp, "%3.3s ", "NYI");
	    break;
	  case P_DT:
	    sprintf(tmp, "%3.3s ", scale_k(task->dt, 3, 0));
	    break;
	  case P_RSS:	/* resident not rss, it seems to be more correct. */
	    sprintf(tmp, "%4.4s ",
		scale_k((task->resident << CL_pg_shift), 4, 1));
	    break;
	  case P_WCHAN:
	    if (!CL_wchan_nout)
		sprintf(tmp, "%-9.9s ", wchan(task->wchan));
	    else
		sprintf(tmp, "%-9lx", task->wchan);
	    break;
	  case P_STAT:
	    sprintf(tmp, "%-4.4s ", status(task));
	    break;
	  case P_TIME:
	    t = (task->utime + task->stime) / Hertz;
	    if (Cumulative)
		t += (task->cutime + task->cstime) / Hertz;
	    sprintf(tmp, "%6.6s ", scale_time(t,6));
	    break;
	  case P_COMMAND:
	    if (!show_cmd && task->cmdline && *(task->cmdline)) {
	        j=0;
	        while(((task->cmdline)[j] != NULL) && (strlen(tmp3)<1020)){
/* #if 0 */ /* This is useless? FIXME */
		    if (j > 0)
			strcat(tmp3, " ");
/* #endif */
		    strncat(tmp3, (task->cmdline)[j], 1000);
		    j++; 
	        }
	        cmdptr = tmp3;
	    } else {
		cmdptr = task->cmd;
	    }
	    if (strlen(cmdptr) > Maxcmd)
		cmdptr[Maxcmd - 1] = 0;
	    sprintf(tmp, "%s", cmdptr);
	    tmp3[0]=0;
	    break;
	  case P_FLAGS:
	    sprintf(tmp, "%8lx ", task->flags);
	    break;
	}
	strcat(tmp2, tmp);
    }
    if (strlen(tmp2) > Cols - 1)
	tmp2[Cols - 1] = 0;

    /* take care of cases like:
       perl -e 'foo
          bar foo bar
          foo
          # end of perl script'
    */
    for (p=tmp2;*p;++p)
        if (!isgraph(*p))
            *p=' ';

    printf("\n%s", tmp2);
    PUTP(top_clrtoeol);
}

/*
 * This is the real program!  Read process info and display it.
 * One could differentiate options of readproctable2, perhaps it
 * would be useful to support the PROC_UID and PROC_TTY
 * as command line options.
 */
void show_procs(void)
{
    static proc_t **p_table=NULL;
    static int proc_flags;
    int count;
    int ActualLines;
    float elapsed_time;
    unsigned int main_mem;
    static int first=0;

    if (first==0) {
	proc_flags=PROC_FILLMEM|PROC_FILLCMD|PROC_FILLUSR;
	if (monpids_index)
	    proc_flags |= PROC_PID;
	p_table=readproctab2(proc_flags, p_table, monpids);
	elapsed_time = get_elapsed_time();
	do_stats(p_table, elapsed_time, 0);
	sleep(1);
	first=1;
    }
    if (first && Batch)
	    fputs("\n\n",stdout);
    /* Display the load averages. */
    PUTP(ho);
    PUTP(md);
    if (show_loadav) {
	printf("%s", sprint_uptime());
	PUTP(top_clrtoeol);
	putchar('\n');
    }
    p_table=readproctab2(proc_flags, p_table, monpids);
    /* Immediately find out the elapsed time for the frame. */
    elapsed_time = get_elapsed_time();
    /* Display the system stats, calculate percent CPU time
     * and sort the list. */
    do_stats(p_table, elapsed_time,1);
    /* Display the memory and swap space usage. */
    main_mem = show_meminfo();
    if (strlen(Header) + 2 > Cols)
	Header[Cols - 2] = 0;
    PUTP(mr);
    fputs(Header, stdout);
    PUTP(top_clrtoeol);
    PUTP(me);

    /*
     * Finally!  Loop through to find the top task, and display it.
     * Lather, rinse, repeat.
     */
    count = 0;
    ActualLines = 0;
    while ((ActualLines < Maxlines) && (p_table[count]->pid!=-1)) {
	int pmem;
	char stat;

	stat = p_table[count]->state;

	if ( (!Noidle || (stat != 'S' && stat != 'Z')) &&
	     ( (CurrUser[0] == '\0') ||
	      (!strcmp((char *)CurrUser,p_table[count]->euser) ) ) ) {

	    /*
	     * Show task info.
	     */
	    pmem = p_table[count]->resident * 1000 / (main_mem / 4);
	    show_task_info(p_table[count], pmem);
	    if (!Batch)
	    	ActualLines++;
	}
	count++;
    }
    PUTP(top_clrtobot);
    PUTP(tgoto(cm, 0, header_lines - 2));
    fflush(stdout);
}


/*
 * Finds the current time (in microseconds) and calculates the time
 * elapsed since the last update. This is essential for computing
 * percent CPU usage.
 */
float get_elapsed_time(void)
{
    struct timeval time;
    static struct timeval oldtime;
    struct timezone timez;
    float elapsed_time;

    gettimeofday(&time, &timez);
    elapsed_time = (time.tv_sec - oldtime.tv_sec)
	+ (float) (time.tv_usec - oldtime.tv_usec) / 1000000.0;
    oldtime.tv_sec = time.tv_sec;
    oldtime.tv_usec = time.tv_usec;
    return (elapsed_time);
}


/*
 * Reads the memory info and displays it.  Returns the total memory
 * available, for use in percent memory usage calculations.
 */
unsigned show_meminfo(void)
{
    unsigned long long **mem = get_meminfo();	/* read+parse /proc/meminfo */
    
    if (!mem ||	mem[meminfo_main][meminfo_total] == 0) {	/* cannot normalize mem usage */
	fprintf(stderr, "Cannot get size of memory from /proc/meminfo\n");
	error_end(1);
    }
    if (show_memory) {
	printf("Mem:  %7LdK av, %7LdK used, %7LdK free, %7LdK shrd, %7LdK buff",
	       mem[meminfo_main][meminfo_total] >> 10,
	       mem[meminfo_main][meminfo_used] >> 10,
	       mem[meminfo_main][meminfo_free] >> 10,
	       mem[meminfo_main][meminfo_shared] >> 10,
	       mem[meminfo_main][meminfo_buffers] >> 10);
	PUTP(top_clrtoeol);
	putchar('\n');
	printf("Swap: %7LdK av, %7LdK used, %7LdK free                 %7LdK cached",
	       mem[meminfo_swap][meminfo_total] >> 10,
	       mem[meminfo_swap][meminfo_used] >> 10,
	       mem[meminfo_swap][meminfo_free] >> 10,
	       mem[meminfo_total][meminfo_cached] >> 10);
	PUTP(top_clrtoeol);
	putchar('\n');
    }
    PUTP(me);
    PUTP(top_clrtoeol);
    putchar('\n');
    return mem[meminfo_main][meminfo_total] >> 10;
}

/*
 * Calculates the number of tasks in each state (running, sleeping, etc.).
 * Calculates the CPU time in each state (system, user, nice, etc).
 * Calculates percent cpu usage for each task.
 */
void do_stats(proc_t** p, float elapsed_time, int pass)
{
    proc_t *this;
    int index, total_time, cpumap, i, n = 0;
    int sleeping = 0, stopped = 0, zombie = 0, running = 0;
    unsigned long system_ticks = 0, user_ticks = 0, nice_ticks = 0, idle_ticks;
    static int prev_count = 0;
    int stime, utime;

    /* start with one 4K page as a reasonable allocate size */
    static int save_history_size = sizeof(struct save_hist) * 204;
    static struct save_hist *save_history;
    struct save_hist *New_save_hist;
    static int *s_ticks_o = NULL, *u_ticks_o = NULL,
               *n_ticks_o = NULL, *i_ticks_o = NULL;
    int s_ticks, u_ticks, n_ticks, i_ticks, t_ticks;
    char str[128];
    FILE *file;


    if (!save_history)
	save_history = xcalloc(NULL, save_history_size);
    New_save_hist = xcalloc(NULL, save_history_size);

    if(s_ticks_o == NULL) {
      s_ticks_o = (int *)malloc(nr_cpu * sizeof(int));
      u_ticks_o = (int *)malloc(nr_cpu * sizeof(int));
      n_ticks_o = (int *)malloc(nr_cpu * sizeof(int));
      i_ticks_o = (int *)malloc(nr_cpu * sizeof(int));
    }
    idle_ticks = 1000 * nr_cpu;

    /*
     * Make a pass through the data to get stats.
     */
    index = 0;
    while (p[n]->pid != -1) {
	this = p[n];
	switch (this->state) {
	  case 'S':
	  case 'D':
	    sleeping++;
	    break;
	  case 'T':
	    stopped++;
	    break;
	  case 'Z':
	    zombie++;
	    break;
	  case 'R':
	    running++;
	    break;
	  default:
	    /* Don't know how to handle this one. */
	    break;
        }

	/*
	 * Calculate time in this process.  Time is sum of user time
	 * (utime) plus system time (stime).
	 */
	total_time = this->utime + this->stime;
	if (index * sizeof(struct save_hist) >= save_history_size) {
	    save_history_size *= 2;
	    save_history = xrealloc(save_history, save_history_size);
	    New_save_hist = xrealloc(New_save_hist, save_history_size);
	}
	New_save_hist[index].ticks = total_time;
	New_save_hist[index].pid = this->pid;
	stime = this->stime;
	utime = this->utime;
	New_save_hist[index].stime = stime;
	New_save_hist[index].utime = utime;

	/* find matching entry from previous pass */
	for (i = 0; i < prev_count; i++) {
	    if (save_history[i].pid == this->pid) {
		total_time -= save_history[i].ticks;
		stime -= save_history[i].stime;
		utime -= save_history[i].utime;

		i = prev_count;
	    }
	}

	/*
	 * Calculate percent cpu time for this task.
	 */
	this->pcpu = (total_time * 10 * 100/Hertz) / elapsed_time;
	if (this->pcpu > 999)
	    this->pcpu = 999;

	/*
	 * Calculate time in idle, system, user and niced tasks.
	 */
	idle_ticks -= this->pcpu;
	system_ticks += stime;
	user_ticks += utime;
	if (this->nice > 0)
	    nice_ticks += this->pcpu;

	/*
	 * If in Sun-mode, adjust cpu percentage not only for
	 * the cpu the process is running on, but for all cpu together.
	 */
	if(!Irixmode) {
	  this->pcpu /= nr_cpu;
	  }
	
	index++;
	n++;
    }

    if (idle_ticks < 0)
	idle_ticks = 0;
    system_ticks = (system_ticks * 10 * 100/Hertz) / elapsed_time;
    user_ticks = (user_ticks * 10 * 100/Hertz) / elapsed_time;

    /*
     * Display stats.
     */
    if (pass > 0 && show_stats) {
	printf("%d processes: %d sleeping, %d running, %d zombie, "
	       "%d stopped",
	       n, sleeping, running, zombie, stopped);
	PUTP(top_clrtoeol);
	putchar('\n');
	if (nr_cpu == 1 || CPU_states) {
	  /* BEGIN EXPERIMENTAL CODE */
	  /* Throw out the calculation above... TODO: remove calculation. */
	  four_cpu_numbers(&user_ticks,&nice_ticks,&system_ticks,&idle_ticks);
	  do{
	    unsigned long sum;
	    sum = user_ticks+nice_ticks+system_ticks+idle_ticks;
	    user_ticks   = (user_ticks   * 1000) / sum;
	    system_ticks = (system_ticks * 1000) / sum;
	    nice_ticks   = (nice_ticks   * 1000) / sum;
	    idle_ticks   = (idle_ticks   * 1000) / sum;
	  }while(0);
	  /* END EXPERIMENTAL CODE */
	  if (Irixmode) {
	    user_ticks *= nr_cpu;
	    system_ticks *= nr_cpu;
	    nice_ticks *= nr_cpu;
	    idle_ticks *= nr_cpu;
	  }
	  printf("CPU states:"
		 " %2ld.%ld%% user, %2ld.%ld%% system,"
		 " %2ld.%ld%% nice, %2ld.%ld%% idle",
		 user_ticks / 10UL, user_ticks % 10UL,
		 system_ticks / 10UL, system_ticks % 10UL,
		 nice_ticks / 10UL, nice_ticks % 10UL,
		 idle_ticks / 10UL, idle_ticks % 10UL);
	  PUTP(top_clrtoeol);
	  putchar('\n');
	}
	/*
         * Calculate stats for all cpus
         */
	if(nr_cpu > 1) {
	  file = fopen("/proc/stat", "r");
	  if(file == NULL) {
	    fprintf(stderr, "fopen failed on /proc/stat\n");
	  }
	  else {
	    /* Skip the total CPU states. */
	    if(fgets(str, 128, file) == NULL) {
	      fprintf(stderr, "fgets failed on /proc/stat\n");
	    }
	    else {
	      for(i = 0; i < nr_cpu; i++) {
		if(fscanf(file, "cpu%*d %d %d %d %d\n",
			  &u_ticks, &n_ticks, &s_ticks, &i_ticks) != 4) {
		  fprintf(stderr, "fscanf failed on /proc/stat for cpu %d\n", i);
		  break;
		}
		else  {
		  t_ticks = (u_ticks + s_ticks + i_ticks + n_ticks)
		  	    - (u_ticks_o[i] + s_ticks_o[i]
			       + i_ticks_o[i] + n_ticks_o[i]);
		  if (Irixmode) cpumap=i;
		  else cpumap=cpu_mapping[i];
		  printf ("CPU%d states: %2d.%-d%% user, %2d.%-d%% system,"
			  " %2d.%-d%% nice, %2d.%-d%% idle",
			  cpumap,
			  (u_ticks - u_ticks_o[i] + n_ticks - n_ticks_o[i]) * 100 / t_ticks,
			  (u_ticks - u_ticks_o[i]) * 100 % t_ticks / 100,
			  (s_ticks - s_ticks_o[i]) * 100 / t_ticks,
			  (s_ticks - s_ticks_o[i]) * 100 % t_ticks / 100,
			  (n_ticks - n_ticks_o[i]) * 100 / t_ticks,
			  (n_ticks - n_ticks_o[i]) * 100 % t_ticks / 100,
			  (i_ticks - i_ticks_o[i]) * 100 / t_ticks,
			  (i_ticks - i_ticks_o[i]) * 100 % t_ticks / 100);
		  s_ticks_o[i] = s_ticks;
		  u_ticks_o[i] = u_ticks;
		  n_ticks_o[i] = n_ticks;
		  i_ticks_o[i] = i_ticks;
		  PUTP (top_clrtoeol);
		  putchar ('\n');
		}
	      }
	    }
	    fclose(file);
	  }
	}
    } else {
      /*
       * During the first pass, get the information for the
       * different CPUs.
       */
      if(nr_cpu > 1) {
       file = fopen("/proc/stat", "r");
       if(file == NULL) {
         puts("open");
       }
       if(fgets(str, 128, file) == NULL) {
         fprintf(stderr, "fgets failed on /proc/stat\n");
       }
       for(i = 0; i < nr_cpu; i++) {
         if(fscanf(file, "cpu%*d %d %d %d %d\n",
                   &u_ticks_o[i], &n_ticks_o[i], &s_ticks_o[i],
                   &i_ticks_o[i]) != 4) {
           fprintf(stderr, "fscanf failed on /proc/stat for cpu %d\n", i);
         }
       }
       fclose(file);
      }
    }
    /*
     * Save this frame's information.
     */
    for (i = 0; i < n; i++) {
	/* copy the relevant info for the next pass */
 	save_history[i].pid = New_save_hist[i].pid;
	save_history[i].ticks = New_save_hist[i].ticks;
	save_history[i].stime = New_save_hist[i].stime;
	save_history[i].utime = New_save_hist[i].utime;
    }
    free(New_save_hist);

    prev_count = n;
    qsort(p, n, sizeof(proc_t*), (void*)mult_lvl_cmp);
}


/*
 * Process keyboard input during the main loop
 */
void do_key(char c)
{
    int numinput, i;
    char rcfile[MAXNAMELEN];
    FILE *fp;

    /*
     * First the commands which don't require a terminal mode switch.
     */
    if (c == 'q')
	end(0);
    else if (c == ' ')
        return;
    else if (c == 12) {
	clear_screen();
	return;
    } else if (c == 'I') {
	Irixmode=(Irixmode) ? 0 : 1;
	return;
    }

    /*
     * Switch the terminal to normal mode.  (Will the original
     * attributes always be normal?  Does it matter?  I suppose the
     * shell will be set up the way the user wants it.)
     */
    if (!Batch) tcsetattr(0, TCSANOW, &Savetty);

    /*
     * Handle the rest of the commands.
     */
    switch (c) {
      case '?':
      case 'h':
	PUTP(cl); PUTP(ho); putchar('\n'); PUTP(mr);
	printf("Proc-Top Revision 1.2");
	PUTP(me); putchar('\n');
	printf("Secure mode ");
	PUTP(md);
	fputs(Secure ? "on" : "off", stdout);
	PUTP(me);
	fputs("; cumulative mode ", stdout);
	PUTP(md);
	fputs(Cumulative ? "on" : "off", stdout);
	PUTP(me);
	fputs("; noidle mode ", stdout);
	PUTP(md);
	fputs(Noidle ? "on" : "off", stdout);
	PUTP(me);
	fputs("\n\n", stdout);
	printf("%s\n\nPress any key to continue", Secure ? SECURE_HELP_SCREEN : HELP_SCREEN);
	if (!Batch) tcsetattr(0, TCSANOW, &Rawtty);
	(void) getchar();
	break;
      case 'i':
	Noidle = !Noidle;
	SHOWMESSAGE(("No-idle mode %s", Noidle ? "on" : "off"));
	break;
      case 'u':
	SHOWMESSAGE(("Which User (Blank for All): "));
	strcpy(CurrUser,getstr());
	break;
      case 'k':
	if (Secure)
	    SHOWMESSAGE(("\aCan't kill in secure mode"));
	else {
	    int pid, signal;
	    PUTP(md);
	    SHOWMESSAGE(("PID to kill: "));
	    pid = getint();
	    if (pid == BAD_INPUT)
		break;
	    PUTP(top_clrtoeol);
	    SHOWMESSAGE(("Kill PID %d with signal [15]: ", pid));
	    PUTP(me);
	    signal = getsig();
	    if (signal == -1)
		signal = SIGTERM;
	    if (kill(pid, signal))
		SHOWMESSAGE(("\aKill of PID %d with %d failed: %s",
			     pid, signal, strerror(errno)));
	}
	break;
      case 'l':
	SHOWMESSAGE(("Display load average %s", !show_loadav ? "on" : "off"));
	if (show_loadav) {
	    show_loadav = 0;
	    header_lines--;
	} else {
	    show_loadav = 1;
	    header_lines++;
	}
	Numfields = make_header();
	break;
      case 'm':
	SHOWMESSAGE(("Display memory information %s", !show_memory ? "on" : "off"));
	if (show_memory) {
	    show_memory = 0;
	    header_lines -= 2;
	} else {
	    show_memory = 1;
	    header_lines += 2;
	}
	Numfields = make_header();
	break;
      case 'M':
        SHOWMESSAGE(("Sort by memory usage"));
	sort_type = S_MEM;
	reset_sort_options();
	register_sort_function(-1, (cmp_t)mem_sort);
	break;
      case 'n':
      case '#':
	printf("Processes to display (0 for unlimited): ");
	numinput = getint();
	if (numinput != BAD_INPUT) {
	    Display_procs = numinput;
	    window_size(0);
	}
	break;
      case 'r':
	if (Secure)
	    SHOWMESSAGE(("\aCan't renice in secure mode"));
	else {
	    int pid, val;

	    printf("PID to renice: ");
	    pid = getint();
	    if (pid == BAD_INPUT)
		break;
	    PUTP(tgoto(cm, 0, header_lines - 2));
	    PUTP(top_clrtoeol);
	    printf("Renice PID %d to value: ", pid);
	    val = getint();
	    if (val == BAD_INPUT)
		val = 10;
	    if (setpriority(PRIO_PROCESS, pid, val))
		SHOWMESSAGE(("\aRenice of PID %d to %d failed: %s",
			     pid, val, strerror(errno)));
	}
	break;
      case 'P':
        SHOWMESSAGE(("Sort by CPU usage"));
	sort_type = S_PCPU;
	reset_sort_options();
	register_sort_function(-1, (cmp_t)pcpu_sort);
	break;
      case 'A':
	SHOWMESSAGE(("Sort by age"));
	sort_type = S_AGE;
	reset_sort_options();
	register_sort_function(-1, (cmp_t)age_sort);
	break;
      case 'N':
	SHOWMESSAGE(("Sort numerically by pid"));
	sort_type = S_NONE;
	reset_sort_options();
	break;
    case 'c':
        show_cmd = !show_cmd;
	SHOWMESSAGE(("Show %s", show_cmd ? "command names" : "command line"));
	break;
      case 'S':
	Cumulative = !Cumulative;
	SHOWMESSAGE(("Cumulative mode %s", Cumulative ? "on" : "off"));
	if (Cumulative)
	    headers[22][1] = 'C';
	else
	    headers[22][1] = ' ';
	Numfields = make_header();
	break;
      case 's':
	if (Secure)
	    SHOWMESSAGE(("\aCan't change delay in secure mode"));
	else {
	    double tmp;
	    printf("Delay between updates: ");
	    tmp = getfloat();
	    if (!(tmp < 0))
		Sleeptime = tmp;
	}
	break;
      case 't':
	SHOWMESSAGE(("Display summary information %s", !show_stats ? "on" : "off"));
	if (show_stats) {
	    show_stats = 0;
	    header_lines -= 2;
	} else {
	    show_stats = 1;
	    header_lines += 2;
	}
	Numfields = make_header();
	break;
      case 'T':
	SHOWMESSAGE(("Sort by %stime", Cumulative ? "cumulative " : ""));
	sort_type = S_TIME;
	reset_sort_options();
	register_sort_function( -1, (cmp_t)time_sort);	
	break;
      case 'f':
      case 'F':
	change_fields();
	break;
      case 'o':
      case 'O':
	change_order();
	break;
      case 'W':
	if (getenv("HOME")) {
	    strcpy(rcfile, getenv("HOME"));
	    strcat(rcfile, "/");
	    strcat(rcfile, RCFILE);
	    fp = fopen(rcfile, "w");
	    if (fp != NULL) {
		fprintf(fp, "%s\n", Fields);
		i = (int) Sleeptime;
		if (i < 2)
		    i = 2;
		if (i > 9)
		    i = 9;
		fprintf(fp, "%d", i);
		if (Secure)
		    fprintf(fp, "%c", 's');
		if (Cumulative)
		    fprintf(fp, "%c", 'S');
		if (!show_cmd)
		    fprintf(fp, "%c", 'c');
		if (Noidle)
		    fprintf(fp, "%c", 'i');
		if (!show_memory)
		    fprintf(fp, "%c", 'm');
		if (!show_loadav)
		    fprintf(fp, "%c", 'l');
		if (!show_stats)
		    fprintf(fp, "%c", 't');
		if (!Irixmode)
		    fprintf(fp, "%c", 'I');
		fprintf(fp, "\n");
		fclose(fp);
		SHOWMESSAGE(("Wrote configuration to %s", rcfile));
	    } else {
		SHOWMESSAGE(("Couldn't open %s", rcfile));
	    }
	} else {
	    SHOWMESSAGE(("Couldn't get $HOME -- not saving"));
	}
	break;
      default:
	SHOWMESSAGE(("\aUnknown command `%c' -- hit `h' for help", c));
    }

    /*
     * Return to raw mode.
     */
    if (!Batch) tcsetattr(0, TCSANOW, &Rawtty);
    return;
}


/*#####################################################################
 *#######   A readproctable function that uses already allocated  #####
 *#######   table entries.                                        #####
 *#####################################################################
 */
#define Do(x) (flags & PROC_ ## x)

proc_t** readproctab2(int flags, proc_t** tab, ...) {
    PROCTAB* PT = NULL;
    static proc_t *buff;
    int n = 0;
    static int len = 0;
    va_list ap;

    va_start(ap, tab);		/* pass through args to openproc */
    if (Do(UID)) {
	/* temporary variables to ensure that va_arg() instances
	 * are called in the right order
	 */
	uid_t* u;
	int i;

	u = va_arg(ap, uid_t*);
	i = va_arg(ap, int);
	PT = openproc(flags, u, i);
    }
    else if (Do(PID)) {
	PT = openproc(flags, va_arg(ap, void*)); /* assume ptr sizes same */
	/* work around a bug in openproc() */
	PT->procfs = NULL;
	/* share some process time, since we skipped opendir("/proc") */
	usleep (50*1000);
    }
    else if (Do(TTY) || Do(STAT))
	PT = openproc(flags, va_arg(ap, void*)); /* assume ptr sizes same */
    else
	PT = openproc(flags);
    va_end(ap);
    buff = (proc_t *) 1;
    while (n<len && buff) {     /* read table: (i) already allocated chunks */
	if (tab[n]->cmdline) {
	    free((void*)*tab[n]->cmdline);
	    tab[n]->cmdline = NULL;
	}
        buff = readproc(PT, tab[n]);
	if (buff) n++;
    }
    if (buff) {
	do {               /* (ii) not yet allocated chunks */
	    tab = xrealloc(tab, (n+1)*sizeof(proc_t*));/* realloc as we go, using */
	    buff = readproc(PT, NULL);		  /* final null to terminate */
	    if(buff) tab[n]=buff;
	    len++;
	    n++;
	} while (buff);			  /* stop when NULL reached */
	tab[n-1] = xcalloc(NULL, sizeof (proc_t));
	tab[n-1]->pid=-1;		 /* Mark end of Table */
    } else {
	if (n == len) {
	    tab = xrealloc(tab, (n+1)*sizeof(proc_t*));
	    tab[n] = xcalloc(NULL, sizeof (proc_t));
	    len++;
	}
	tab[n]->pid=-1;    /* Use this instead of NULL when not at the end of */
    }                   /* the allocated space */
    closeproc(PT);
    return tab;
}
