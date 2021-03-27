/*
 * Copyright 1998 by Albert Cahalan; all rights resered.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */
#include <fcntl.h>
#include <pwd.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


static int f_flag, i_flag, v_flag, w_flag, n_flag;

static int tty_count, uid_count, cmd_count, pid_count;
static int *ttys;
static int *uids;
static char **cmds;
static int *pids;

#define ENLIST(thing,addme) do{ \
if(!thing##s) thing##s = malloc(sizeof(*thing##s)*saved_argc); \
if(!thing##s) fprintf(stderr,"No memory.\n"),exit(2); \
thing##s[thing##_count++] = addme; \
}while(0)

static int my_pid;
static int saved_argc;
static char **saved_argv;

static int sig_or_pri;

static int program;
#define PROG_GARBAGE 0  /* keep this 0 */
#define PROG_KILL  1
#define PROG_SKILL 2
/* #define PROG_NICE  3 */ /* easy, but the old one isn't broken */
#define PROG_SNICE 4

/***********************************************************************/

/* Linux signals:
 *
 * SIGSYS is required by Unix98.
 * SIGEMT is part of SysV, BSD, and ancient UNIX tradition.
 *
 * They are provided by these Linux ports: alpha, mips, sparc, and sparc64.
 * You get SIGSTKFLT and SIGUNUSED instead on i386, m68k, ppc, and arm.
 * (this is a Linux & libc bug -- both must be fixed)
 *
 * Total garbage: SIGIO SIGINFO SIGIOT SIGLOST SIGCLD
 * Nearly garbage: SIGSTKFLT SIGUNUSED (nothing else to fill slots)
 */

#ifdef SIGSYS
#  undef SIGUNUSED
#  undef SIGSTKFLT
#endif

#ifndef SIGRTMIN
#  define SIGRTMIN 32
#endif

int sigvals[] = {
SIGABRT,
SIGALRM,
SIGBUS,
SIGCHLD,
SIGCONT,
#ifdef SIGEMT
SIGEMT,
#endif
SIGFPE,
SIGHUP,
SIGILL,
SIGINT,
SIGKILL,
SIGPIPE,
SIGPOLL,
SIGPROF,
#ifdef SIGPWR
SIGPWR,
#endif
SIGQUIT,
SIGSEGV,
#ifdef SIGSTKFLT
SIGSTKFLT,
#endif
SIGSTOP,
#ifdef SIGSYS
SIGSYS,
#endif
SIGTERM,
SIGTRAP,
SIGTSTP,
SIGTTIN,
SIGTTOU,
#ifdef SIGUNUSED
SIGUNUSED,
#endif
SIGURG,
SIGUSR1,
SIGUSR2,
SIGVTALRM,
SIGWINCH,
SIGXCPU,
SIGXFSZ,
};

char *signames[] = {
"ABRT",
"ALRM",
"BUS",
"CHLD",
"CONT",
#ifdef SIGEMT
"EMT",
#endif
"FPE",
"HUP",
"ILL",
"INT",
"KILL",
"PIPE",
"POLL",
"PROF",
"PWR",
"QUIT",
"SEGV",
#ifdef SIGSTKFLT
"STKFLT",
#endif
"STOP",
#ifdef SIGSYS
"SYS",
#endif
"TERM",
"TRAP",
"TSTP",
"TTIN",
"TTOU",
#ifdef SIGUNUSED
"UNUSED",
#endif
"URG",
"USR1",
"USR2",
"VTALRM",
"WINCH",
"XCPU",
"XFSZ"
};

const int number_of_signals = sizeof(sigvals)/sizeof(int);

static int compare_signal_names(const void *a, const void *b){
  return strcasecmp(*(char**)a,*(char**)b);
}

static int signal_name_to_number(char *name){
  const char **ptr;
  if(!strncasecmp(name,"SIG",3)) name += 3;
  ptr = bsearch(&name, signames, number_of_signals,
     sizeof(char *), compare_signal_names
  );
  if(!ptr){
    long val;
    char *endp;
    val = strtol(name,&endp,10);
    if(*endp) return -1; /* not valid */
    if(val>127) return -1; /* not valid */
    return val;
  }
  return sigvals[((unsigned long)ptr-(unsigned long)signames)/sizeof(char *)];
}

static const char *signal_name(int signo){
  static char buf[32];
  int n = number_of_signals;
  signo &= 0x7f; /* need to process exit values too */
  while(n--){
    if(sigvals[n]==signo) return signames[n];
  }
  if(signo) sprintf(buf, "RTMIN+%d", signo-SIGRTMIN);
  else      strcpy(buf,"0");  /* AIX would use "NULL" */
  return buf;
}

static void pretty_print_signals(void){
  int i = 0;
  while(++i <= number_of_signals){
    int n;
    n = printf("%2d %s", i, signal_name(i));
    if(i%7) printf("           \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" + n);
    else printf("\n");
  }
  if((i-1)%7) printf("\n");
}

static void unix_print_signals(void){
  int pos = 0;
  int i = 0;
  while(++i <= number_of_signals){
    if(i-1) printf("%c", (pos>73)?(pos=0,'\n'):(pos++,' ') );
    pos += printf("%s", signal_name(i));
  }
  printf("\n");
}

/********************************************************************/

/* This is junk of course, to be replaced soon */
const char *uid_to_user(int uid){
  static struct passwd *p;
  static char txt[64];
  p = getpwuid(uid);
  if(p) return p->pw_name;
  sprintf(txt, "%d", uid);
  return txt;
}

/* this must be replaced soon... */
const char *dev_to_tty(int tty){
  static char txt[64];
  sprintf(txt, "%d,%d", (tty>>8)&0xff, tty&0xff);
  return txt;
}


/********************************************************************/


/***** kill or nice a process */
static void hurt_proc(int tty, int user, int pid, char *cmd){
  int failed;
  int saved_errno;
  if(i_flag){
    char buf[8];
    fprintf(stderr, "%-8.8s %-8.8s %5d %-16.16s   ? ",
      dev_to_tty(tty),uid_to_user(user),pid,cmd
    );
    if(!fgets(buf,7,stdin)){
      printf("\n");
      exit(0);
    }
    if(*buf!='y' && *buf!='Y') return;
  }
  /* do the actual work */
  if(program==PROG_SKILL) failed=kill(pid,sig_or_pri);
  else                    failed=setpriority(PRIO_PROCESS,pid,sig_or_pri);
  saved_errno = errno;
  if(w_flag && failed){
    fprintf(stderr, "%-8.8s %-8.8s %5d %-16.16s   ",
      dev_to_tty(tty),uid_to_user(user),pid,cmd
    );
    errno = saved_errno;
    perror("");
    return;
  }
  if(i_flag) return;
  if(v_flag){
    printf("%-8.8s %-8.8s %5d %-16.16s\n",
      dev_to_tty(tty),uid_to_user(user),pid,cmd
    );
    return;
  }
  if(n_flag){
   printf("%d\n",pid);
   return;
  }
}


/***** check one process */
static void check_proc(int pid){
  char buf[128];
  struct stat statbuf;
  char *tmp;
  int tty;
  int fd;
  int i;
  if(pid==my_pid) return;
  sprintf(buf, "/proc/%d/stat", pid); /* pid (cmd) state ppid pgrp session tty */
  fd = open(buf,O_RDONLY);
  if(fd==-1){  /* process exited maybe */
    if(pids && w_flag) printf("WARNING: process %d could not be found.",pid);
    return;
  }
  fstat(fd, &statbuf);
  if(uids){  /* check the EUID */
    i=uid_count;
    while(i--) if(uids[i]==statbuf.st_uid) break;
    if(i==-1) goto closure;
  }
  read(fd,buf,128);
  buf[127] = '\0';
  tmp = strrchr(buf, ')');
  *tmp++ = '\0';
  i = 5; while(i--) while(*tmp++!=' '); /* scan to find tty */
  tty = atoi(tmp);
  if(ttys){
    i=tty_count;
    while(i--) if(ttys[i]==tty) break;
    if(i==-1) goto closure;
  }
  tmp = strchr(buf, '(') + 1;
  if(cmds){
    i=cmd_count;
    /* fast comparison trick -- useful? */
    while(i--) if(cmds[i][0]==*tmp && !strcmp(cmds[i],tmp)) break;
    if(i==-1) goto closure;
  }
  /* This is where we kill/nice something. */
/*  fprintf(stderr, "PID %d, UID %d, TTY %d,%d, COMM %s\n",
    pid, statbuf.st_uid, tty>>8, tty&0xf, tmp
  );
*/
  hurt_proc(tty, statbuf.st_uid, pid, tmp);
closure:
  close(fd); /* kill/nice _first_ to avoid PID reuse */
}


/***** debug function */
#if 0
static void show_lists(void){
  int i;

  fprintf(stderr, "%d TTY: ", tty_count);
  if(ttys){
    i=tty_count;
    while(i--){
      fprintf(stderr, "%d,%d%c", (ttys[i]>>8)&0xff, ttys[i]&0xff, i?' ':'\n');
    }
  }else fprintf(stderr, "\n");
  
  fprintf(stderr, "%d UID: ", uid_count);
  if(uids){
    i=uid_count;
    while(i--) fprintf(stderr, "%d%c", uids[i], i?' ':'\n');
  }else fprintf(stderr, "\n");
  
  fprintf(stderr, "%d PID: ", pid_count);
  if(pids){
    i=pid_count;
    while(i--) fprintf(stderr, "%d%c", pids[i], i?' ':'\n');
  }else fprintf(stderr, "\n");
  
  fprintf(stderr, "%d CMD: ", cmd_count);
  if(cmds){
    i=cmd_count;
    while(i--) fprintf(stderr, "%s%c", cmds[i], i?' ':'\n');
  }else fprintf(stderr, "\n");
}
#endif


/***** iterate over all PIDs */
static void iterate(void){
  int pid;
  DIR *d;
  struct dirent *de;
  if(pids){
    pid = pid_count;
    while(pid--) check_proc(pids[pid]);
    return;
  }
#if 0
  /* could setuid() and kill -1 to have the kernel wipe out a user */
  if(!ttys && !cmds && !pids && !i_flag){
  }
#endif
  if (!(d = opendir ("/proc"))) {
    perror ("/proc"); exit (1);
  }
  while (de = readdir (d))
    if (pid = atoi (de->d_name))
	check_proc (pid);
  closedir (d);
}

/***** kill help */
static void kill_usage(void){
  fprintf(stderr,
    "Usage:\n"
    "  kill pid ...              Send SIGTERM to every process listed.\n"
    "  kill signal pid ...       Send a signal to every process listed.\n"
    "  kill -s signal pid ...    Send a signal to every process listed.\n"
    "  kill -l                   List all signal names.\n"
    "  kill -L                   List all signal names in a nice table.\n"
    "  kill -l signal            Convert a signal number into a name.\n"
  );
  exit(1);
}

/***** kill */
static void kill_main(int argc, char *argv[]){
  char *sigptr;
  int signo = SIGTERM;
  int exitvalue = 0;
  if(argc<2) kill_usage();
  if(argv[1][0]!='-'){
    argv++;
    argc--;
    goto no_more_args;
  }
  /* The -l option prints out signal names. */
  if(argv[1][1]=='l' && argv[1][2]=='\0'){
    if(argc==2){
      unix_print_signals();
      exit(0);
    }
    if(argc==3 && argv[2][0]>'0' && argv[2][0]<='9'){
      long arg;
      char *endp;
      arg = strtol(argv[2],&endp,10);
      if(!*endp){
        printf("%s\n", signal_name(arg));
        exit(0);
      }
    }
    kill_usage();
  }
  /* The -L option prints out signal names in a nice table. */
  if(argv[1][1]=='L' && argv[1][2]=='\0'){
    if(argc==2){
      pretty_print_signals();
      exit(0);
    }
    kill_usage();
  }
  if(argv[1][1]=='-' && argv[1][2]=='\0'){
    argv+=2;
    argc-=2;
    goto no_more_args;
  }
  if(argv[1][1]=='-') kill_usage(); /* likely --help */
  if(argv[1][1]=='s' && argv[1][2]=='\0'){
    sigptr = argv[2];
    argv+=3;
    argc-=3;
  }else{
    sigptr = argv[1]+1;
    argv+=2;
    argc-=2;
  }
  signo = signal_name_to_number(sigptr);
  if(signo<0){
    fprintf(stderr, "ERROR: unknown signal name \"%s\".\n", sigptr);
    kill_usage();
  }
no_more_args:
  if(!argc) kill_usage();  /* nothing to kill? */
  while(argc--){
    long pid;
    char *endp;
    pid = strtol(argv[argc],&endp,10);
    if(!*endp){
      if(!kill((pid_t)pid,signo)) continue;
      exitvalue = 1;
      continue;
    }
    fprintf(stderr, "ERROR: garbage process ID \"%s\".\n", argv[argc]);
    kill_usage();
  }
  exit(exitvalue);
}

/***** skill/snice help */
static void skillsnice_usage(void){
  if(program==PROG_SKILL){
    fprintf(stderr,
      "Usage:   skill [signal to send] [options] process selection criteria\n"
      "Example: skill -KILL -v pts/*\n"
      "\n"
      "The default signal is TERM. Use -l or -L to list available signals.\n"
      "Particularly useful signals include HUP, INT, KILL, STOP, CONT, and 0.\n"
      "Alternate signals may be specified in three ways: -SIGKILL -KILL -9\n"
    );
  }else{
    fprintf(stderr,
      "Usage:   snice [new priority] [options] process selection criteria\n"
      "Example: snice netscape crack +7\n"
      "\n"
      "The default priority is +4. (snice +4 ...)\n"
      "Priority numbers range from +20 (slowest) to -20 (fastest).\n"
      "Negative priority numbers are restricted to administrative users.\n"
    );
  }
  fprintf(stderr,
    "\n"
    "General options:\n"
    "-f  fast mode            This is not currently useful.\n"
    "-i  interactive use      You will be asked to approve each action.\n"
    "-v  verbose output       Display information about selected processes.\n"
    "-w  warnings enabled     This is not currently useful.\n"
    "-n  no action            This only displays the process ID.\n"
    "\n"
    "Selection criteria can be: terminal, user, pid, command.\n"
    "The options below may be used to ensure correct interpretation.\n"
    "-t  The next argument is a terminal (tty or pty).\n"
    "-u  The next argument is a username.\n"
    "-p  The next argument is a process ID number.\n"
    "-c  The next argument is a command name.\n"
  );
  exit(1);
}

#if 0
static void _skillsnice_usage(int line){
  fprintf(stderr,"Something at line %d.\n", line);
  skillsnice_usage();
}
#define skillsnice_usage() _skillsnice_usage(__LINE__)
#endif

#define NEXTARG (argc?( argc--, ((argptr=*++argv)) ):NULL)

/***** common skill/snice argument parsing code */
static void skillsnice_parse(int argc, char *argv[]){
  int signo = -1;
  int prino = 0xdeafbeef;
  int force = 0;
  int num_found = 0;
  char *argptr;
  if(argc<2) skillsnice_usage();
  if(argc==2 && argv[1][0]=='-'){
    if(!strcmp(argv[1],"-L")){
      pretty_print_signals();
      exit(0);
    }
    if(!strcmp(argv[1],"-l")){
      unix_print_signals();
      exit(0);
    }
    skillsnice_usage();
  }
  NEXTARG;
  /* Time for serious parsing. What does "skill -int 123 456" mean? */
  while(argc){
    if(force && !num_found){  /* if forced, _must_ find something */
      fprintf(stderr,"ERROR: -%c used with bad data.\n", force);
      skillsnice_usage();
    }
    force = 0;
    if(program==PROG_SKILL && signo<0 && *argptr=='-'){
      signo = signal_name_to_number(argptr+1);
      if(signo>=0){      /* found a signal */
        if(!NEXTARG) break;
        continue;
      }
    }
    if(program==PROG_SNICE && prino==0xdeafbeef
    && (*argptr=='+' || *argptr=='-') && argptr[1]){
      long val;
      char *endp;
      val = strtol(argptr,&endp,10);
      if(!*endp && val<=999 && val>=-999){
        prino=val;
        if(!NEXTARG) break;
        continue;
      }
    }
    /* If '-' found, collect any flags. (but lone "-" is a tty) */
    if(*argptr=='-' && argptr[1]){
      argptr++;
      do{
        switch(( force = *argptr++ )){
        default:  skillsnice_usage();
        case 't':
        case 'u':
        case 'p':
        case 'c':
          if(!*argptr){ /* nothing left here, *argptr is '\0' */
            if(!NEXTARG){
              fprintf(stderr,"ERROR: -%c with nothing after it.\n", force);
              skillsnice_usage();
            }
          }
          goto selection_collection;
        case 'f': f_flag++; break;
        case 'i': i_flag++; break;
        case 'v': v_flag++; break;
        case 'w': w_flag++; break;
        case 'n': n_flag++; break;
        case 0:
          NEXTARG;
          /*
           * If no more arguments, all the "if(argc)..." tests will fail
           * and the big loop will exit.
           */
        } /* END OF SWITCH */
      }while(force);
    } /* END OF IF */
selection_collection:
    num_found = 0; /* we should find at least one thing */
    switch(force){ /* fall through each data type */
    default: skillsnice_usage();
    case 0: /* not forced */
    case 't':
      if(argc){
        struct stat sbuf;
        char path[32];
        if(!argptr) skillsnice_usage(); /* Huh? Maybe "skill -t ''". */
        snprintf(path,32,"/dev/%s",argptr);
        if(stat(path, &sbuf)>=0 && S_ISCHR(sbuf.st_mode)){
          num_found++;
          ENLIST(tty,sbuf.st_rdev);
          if(!NEXTARG) break;
        }else if(!(argptr[1])){  /* if only 1 character */
          switch(*argptr){
          default:
            if(stat(argptr,&sbuf)<0) break; /* the shell eats '?' */
          case '-':
          case '?':
            num_found++;
            ENLIST(tty,-1);
            if(!NEXTARG) break;
          }
        }
      }
      if(force) continue;
    case 'u':
      if(argc){
        struct passwd *passwd_data;
        passwd_data = getpwnam(argptr);
        if(passwd_data){
          num_found++;
          ENLIST(uid,passwd_data->pw_uid);
          if(!NEXTARG) break;
        }
      }
      if(force) continue;
    case 'p':
      if(argc && *argptr>='0' && *argptr<='9'){
        char *endp;
        int num;
        num = strtol(argptr, &endp, 0);
        if(*endp == '\0'){
          num_found++;
          ENLIST(pid,num);
          if(!NEXTARG) break;
        }
      }
      if(force) continue;
      if(num_found) continue; /* could still be an option */
    case 'c':
      if(argc){
        num_found++;
        ENLIST(cmd,argptr);
        if(!NEXTARG) break;
      }
    } /* END OF SWITCH */
  } /* END OF WHILE */
  /* No more arguments to process. Must sanity check. */
  if(!tty_count && !uid_count && !cmd_count && !pid_count){
    fprintf(stderr,"ERROR: no process selection criteria.\n");
    skillsnice_usage();
  }
  if((f_flag|i_flag|v_flag|w_flag|n_flag) & ~1){
    fprintf(stderr,"ERROR: general flags may not be repeated.\n");
    skillsnice_usage();
  }
  if(i_flag && (v_flag|f_flag|n_flag)){
    fprintf(stderr,"ERROR: -i makes no sense with -v, -f, and -n.\n");
    skillsnice_usage();
  }
  if(v_flag && (i_flag|f_flag)){
    fprintf(stderr,"ERROR: -v makes no sense with -i and -f.\n");
    skillsnice_usage();
  }
  if(n_flag && signo>=0){
    fprintf(stderr,"ERROR: -n makes no sense with a signal.\n");
    skillsnice_usage();
  }
  if(n_flag && prino!=0xdeafbeef){
    fprintf(stderr,"ERROR: -n makes no sense with a priority value.\n");
    skillsnice_usage();
  }
  /* OK, set up defaults */
  if(prino==0xdeadbeef) prino=4;
  if(signo<0) signo=SIGTERM;
  if(n_flag){
    program=PROG_SKILL;
    signo=0; /* harmless */
  }
  if(program==PROG_SKILL) sig_or_pri = signo;
  else sig_or_pri = prino;
}

/***** main body */
int main(int argc, char *argv[]){
  char *tmpstr;

  my_pid = getpid();
  saved_argv = argv;
  saved_argc = argc;
  if(!argc){
    fprintf(stderr,"ERROR: could not determine own name.\n");
    exit(1);
  }
  tmpstr=strrchr(*argv,'/');
  if(tmpstr) tmpstr++;
  if(!tmpstr) tmpstr=*argv;
  program = PROG_GARBAGE;
  if(*tmpstr=='s'){
    setpriority(PRIO_PROCESS,my_pid,-20);
    if(!strcmp(tmpstr,"snice")) program = PROG_SNICE;
    if(!strcmp(tmpstr,"skill")) program = PROG_SKILL;
  }else{
    if(!strcmp(tmpstr,"kill")) program = PROG_KILL;
  }
  switch(program){
  case PROG_SNICE:
  case PROG_SKILL:
    skillsnice_parse(argc, argv);
/*    show_lists(); */
    iterate(); /* this is it, go get them */
    break;
  case PROG_KILL:
    kill_main(argc, argv);
    break;
  default:
    fprintf(stderr,"ERROR: no \"%s\" support.\n",tmpstr);
  }
  return 0;
}


