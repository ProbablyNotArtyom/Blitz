/*
 * top.h header file 1996/05/18, 
 *
 * function prototypes, global data definitions and string constants.
 */

proc_t** readproctab2(int flags, proc_t** tab, ...);
void parse_options(char *Options, int secure);
void get_options(void);
void error_end(int rno);
void end(int signo);
void stop(int signo);
void window_size(int signo);
int make_header(void);
int getnum(void);
char *getstr(void);
int getsig(void);
float getfloat(void);
int time_sort(proc_t **P, proc_t **Q);
int pcpu_sort(proc_t **P, proc_t **Q);
int mem_sort(proc_t **P, proc_t **Q);
int age_sort(proc_t **P, proc_t **Q);
void show_fields(void);
void change_order(void);
void change_fields(void);
void show_task_info(proc_t *task, int pmem);
void show_procs(void);
float get_elapsed_time(void);
unsigned show_meminfo(void);
void do_stats(proc_t** p, float elapsed_time, int pass);
void do_key(char c);


/* configurable field display support */

int pflags[30];
int sflags[10];
int Numfields;


	/* Name of the config file (in $HOME)  */
#ifndef RCFILE
#define RCFILE		".toprc"
#endif

#ifndef SYS_TOPRC
#define SYS_TOPRC	"/etc/toprc"
#endif

#define MAXLINES 2048
#define MAXNAMELEN 1024

/* this is what procps top does by default, so let's do this, if nothing is
 * specified
 */
#ifndef DEFAULT_SHOW
/*                       0         1         2         3 */
/*                       0123456789012345678901234567890 */
#define DEFAULT_SHOW    "AbcDgHIjklMnoTP|qrsuzyV{EFWX"
#endif
char Fields[256] = "";


/* This structure stores some critical information from one frame to
   the next. mostly used for sorting. Added cumulative and resident fields. */
struct save_hist {
    int ticks;
    int pid;
    int pcpu;
    int utime;
    int stime;
};

	/* The original terminal attributes. */
struct termios Savetty;
	/* The new terminal attributes. */
struct termios Rawtty;
	/* Cached termcap entries. */
char *cm, *cl, *top_clrtobot, *top_clrtoeol, *ho, *md, *me, *mr;
	/* Current window size.  Note that it is legal to set Display_procs
	   larger than can fit; if the window is later resized, all will be ok.
	   In other words: Display_procs is the specified max number of
	   processes to display (zero for infinite), and Maxlines is the actual
	   number. */
int Lines, Cols, Maxlines, Display_procs;
	/* Maximum length to display of the command line of a process. */
unsigned Maxcmd;

	/* The top of the main loop. */
jmp_buf redraw_jmp;

	/* Controls how long we sleep between screen updates.  Accurate to
	   microseconds. */
float Sleeptime = 5;
	/* for opening/closing the system map */
int psdbsucc = 0;
	/* Mode flags. */
int Irixmode = 1;
int Secure = 0;
int Cumulative = 0;
int Noidle = 0;
int CPU_states = 0;
char CurrUser[BUFSIZ];
int CL_pg_shift = (PAGE_SHIFT - 10);
int CL_wchan_nout = -1;
int show_stats = 1;    /* show status summary */
int show_memory = 1;   /* show memory summary */
int show_loadav = 1;   /* show load average and uptime */
int show_cmd = 1;      /* show command name instead of commandline */
int monpids_max = 20;
pid_t monpids[11] = { 0 };
int monpids_index = 0;
int Loops = -1;	       /* number of iterations. -1 loops forever */
int Batch = 0;         /* batch mode. Collect no input, dumb output */

/* sorting order: cpu%, mem, time (cumulative, if in cumulative mode) */
enum {
    S_PCPU, S_MEM, S_TIME, S_AGE, S_NONE
};
/* default sorting by CPU% */ 
int sort_type = S_PCPU;

/* flags for each possible field. At the moment up to 30 are supported */
enum {
    P_PID, P_PPID, P_EUID, P_EUSER,
    P_PCPU, P_PMEM, P_TTY, P_PRI,
    P_NICE, P_PAGEIN, P_TSIZ, P_DSIZ,
    P_SIZE, P_TRS, P_SWAP, P_SHARE,
    P_A, P_WP, P_DT, P_RSS,
    P_WCHAN, P_STAT, P_TIME, P_COMMAND,
    P_LCPU, P_FLAGS, P_END
};
/* corresponding headers */
char *headers[] =
{
    "  PID ", " PPID ", " UID ",
    "USER     ", "%CPU ", "%MEM ",
    "TTY      ", "PRI ", " NI ",
    "PAGEIN ", "TSIZE ", "DSIZE ",
    " SIZE ", " TRS ", "SWAP ",
    "SHARE ", "  A ", " WP ",
    "  D ", " RSS ", "WCHAN     ",
    "STAT ", "  TIME ", "COMMAND",
    "LC ",
    "   FLAGS "
};
/* corresponding field desciptions */
char *headers2[] =
{
    "Process Id", "Parent Process Id", "User Id",
    "User Name", "CPU Usage", "Memory Usage",
    "Controlling tty", "Priority", "Nice Value",
    "Page Fault Count", "Code Size (kb)", "Data+Stack Size (kb)",
    "Virtual Image Size (kb)", "Resident Text Size (kb)", "Swapped kb",
    "Shared Pages (kb)", "Accessed Page count", "Write Protected Pages",
    "Dirty Pages", "Resident Set Size (kb)", "Sleeping in Function",
    "Process Status", "CPU Time", "Command",
    "Last used CPU (expect this to change regularly)",
    "Task Flags (see linux/sched.h)"
};

	/* The header printed at the top of the process list.*/
char Header[MAXLINES];

	/* The response to the interactive 'h' command. */
#define HELP_SCREEN "\
Interactive commands are:\n\
\n\
space\tUpdate display\n\
^L\tRedraw the screen\n\
fF\tadd and remove fields\n\
oO\tChange order of displayed fields\n\
h or ?\tPrint this list\n\
S\tToggle cumulative mode\n\
i\tToggle display of idle proceses\n\
I\tToggle between Irix and Solaris views (SMP-only)\n\
c\tToggle display of command name/line\n\
l\tToggle display of load average\n\
m\tToggle display of memory information\n\
t\tToggle display of summary information\n\
k\tKill a task (with any signal)\n\
r\tRenice a task\n\
N\tSort by pid (Numerically)\n\
A\tSort by age\n\
P\tSort by CPU usage\n\
M\tSort by resident memory usage\n\
T\tSort by time / cumulative time\n\
u\tShow only a specific user\n\
n or #\tSet the number of process to show\n\
s\tSet the delay in seconds between updates\n\
W\tWrite configuration file ~/.toprc\n\
q\tQuit"
#define SECURE_HELP_SCREEN "\
Interactive commands available in secure mode are:\n\
\n\
space\tUpdate display\n\
^L\tRedraw the screen\n\
fF\tadd and remove fields\n\
h or ?\tPrint this list\n\
S\tToggle cumulative mode\n\
i\tToggle display of idle proceses\n\
c\tToggle display of command name/line\n\
l\tToggle display of load average\n\
m\tToggle display of memory information\n\
t\tToggle display of summary information\n\
n or #\tSet the number of process to show\n\
u\tShow only a specific user\n\
oO\tChange order of displayed fields\n\
W\tWrite configuration file ~/.toprc\n\
q\tQuit"

	/* Number of lines needed to display the header information. */
int header_lines;

/* ############## Some Macro definitions for screen handling ######### */
	/* String to use in error messages. */
#define PROGNAME "top"
	/* Clear the screen. */
#define clear_screen() \
	    printf("%s", cl)
	/* Show an error in the context of the spiffy full-screen display. */
#define SHOWMESSAGE(x) do { 			\
	    printf("%s%s%s%s", tgoto(cm, 0, header_lines-2), top_clrtoeol,md,mr);	\
	    printf x;					\
	    printf ("%s",me);                           \
	    fflush(stdout);				\
	    sleep(2);					\
	} while (0)
