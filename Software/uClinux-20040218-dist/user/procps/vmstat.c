#define VERSION Version: 0.99, last modified 15 January 94: ALPHA 
#define PROGNAME "vmstat"
/* Copyright 1994 by Henry Ware <al172@yfn.ysu.edu>. Copyleft same year. */
/* This attempts to display virtual memory statistics.
   vmstat does not need to be run as root.
*/
/* TODO etc.
 * Count the system calls.  How?  I don't even know where to start.
 * add more items- aim at Posix 1003.2 (1003.7?) compliance, if relevant (I.  
   don't think it is, but let me know.)
 * sometimes d(inter)<d(ticks)!!  This is not possible: inter is a sum of 
   positive numbers including ticks.  But it happens.  This doesn't seem to
   affect things much, however.
 * It would be interesting to see when the buffers avert a block io:
   Like SysV4's "sar -b"... it might fit better here, with Linux's variable 
   buffer size.
 * Ideally, blocks in & out would be counted in 1k increments, rather than
   by block: this only makes a difference for CDs and is a problematic fix.
*/
/* PROCPS
   The procps suite is maintained by Michael K. Johnson <johnsonm@redhat.com>
*/
   
#include "proc/sysinfo.h"
#include "proc/version.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/dir.h>
#include <dirent.h>

#define NDEBUG !DEBUG

#define BUFFSIZE 4096
#define FALSE 0
#define TRUE 1

int main(int, char **);
void usage(void);
void crash(char *);
int winhi(void);
void showheader(void);
void getstat(unsigned *, unsigned *, unsigned *, unsigned long *,
	     unsigned *, unsigned *, unsigned *, unsigned *,
	     unsigned *, unsigned *, unsigned *);
void getmeminfo(unsigned *, unsigned *, unsigned *, unsigned *);
void getrunners(unsigned *, unsigned *, unsigned *);
static char buff[BUFFSIZE]; /* used in the procedures */

/***************************************************************
                             Main 
***************************************************************/

int main(int argc, char *argv[]) {

  const char format[]="%2u %2u %2u %6u %6u %6u %6u %3u %3u %5u %5u %4u %5u %3u %3u %3u\n";
  unsigned int height=22; /* window height, reset later if needed. */
  unsigned long int args[2]={0,0};
  unsigned int moreheaders=TRUE;
  unsigned int tog=0; /* toggle switch for cleaner code */
  unsigned int i,hz;
  unsigned int running,blocked,swapped;
  unsigned int cpu_use[2], cpu_nic[2], cpu_sys[2];
  unsigned int duse,dsys,didl,div,divo2;
  unsigned int memfree,membuff,swapused, memcache;
  unsigned long cpu_idl[2];
  unsigned int pgpgin[2], pgpgout[2], pswpin[2], pswpout[2];
  unsigned int inter[2],ticks[2],ctxt[2];
  unsigned int per=0, pero2; 
  unsigned long num=0;
  unsigned int kb_per_page = sysconf(_SC_PAGESIZE) / 1024;
  
  setlinebuf(stdout);
  argc=0; /* redefined as number of integer arguments */
  per=1;
  num=0;
  for (argv++;*argv;argv++) {
    if ('-' ==(**argv)) {
      switch (*(++(*argv))) {
	case 'V':
	display_version();
	exit(0);
      case 'n':
	/* print only one header */
	moreheaders=FALSE;
      break;
      default:
	/* no other aguments defined yet. */
	usage();
      }
    } else {
      argc++;
      switch (argc) {
      case 1:
        if ((per = atoi(*argv)) == 0)
         usage();
       num = ULONG_MAX;
       break;
      case 2:
        num = atol(*argv);
       break;
      default:
       usage();
      } /* switch */
    }
  }

  if (moreheaders) {
      int tmp=winhi()-3;
      height=((tmp>0)?tmp:22);
  }    

  pero2=(per/2);
  showheader();
  getrunners(&running,&blocked,&swapped);
  getmeminfo(&memfree,&membuff,&swapused,&memcache);
  getstat(cpu_use,cpu_nic,cpu_sys,cpu_idl,
	  pgpgin,pgpgout,pswpin,pswpout,
	  inter,ticks,ctxt);
  duse= *(cpu_use)+ *(cpu_nic); 
  dsys= *(cpu_sys);
  didl= (*(cpu_idl))%UINT_MAX;
  div= (duse+dsys+didl);
  hz=sysconf(_SC_CLK_TCK); /* get ticks/s from system */
  divo2= div/2;
  printf(format,
	 running,blocked,swapped,
	 swapused,memfree,membuff,memcache,
	 (*(pswpin)*kb_per_page*hz+divo2)/div,
	 (*(pswpout)*kb_per_page*hz+divo2)/div,
	 (*(pgpgin)*hz+divo2)/div,
	 (*(pgpgout)*hz+divo2)/div,
	 (*(inter)*hz+divo2)/div,
	 (*(ctxt)*hz+divo2)/div,
	 (100*duse+divo2)/div,(100*dsys+divo2)/div,(100*didl+divo2)/div);

  for(i=1;i<num;i++) { /* \\\\\\\\\\\\\\\\\\\\ main loop ////////////////// */
    sleep(per);
    if (moreheaders && ((i%height)==0)) showheader();
    tog= !tog;
    getrunners(&running,&blocked,&swapped);
    getmeminfo(&memfree,&membuff,&swapused,&memcache);
    getstat(cpu_use+tog,cpu_nic+tog,cpu_sys+tog,cpu_idl+tog,
	  pgpgin+tog,pgpgout+tog,pswpin+tog,pswpout+tog,
	  inter+tog,ticks+tog,ctxt+tog);
    duse= *(cpu_use+tog)-*(cpu_use+!tog)+*(cpu_nic+tog)-*(cpu_nic+!tog);
    dsys= *(cpu_sys+tog)-*(cpu_sys+!tog);
    didl= (*(cpu_idl+tog)-*(cpu_idl+!tog))%UINT_MAX;
    div= (duse+dsys+didl);
    divo2= div/2;
    printf(format,
	   running,blocked,swapped,
	   swapused,memfree,membuff,memcache,
	   (((*(pswpin+tog)-*(pswpin+(!tog)))*kb_per_page+pero2)/per),
	   ((*(pswpout+tog)-*(pswpout+(!tog)))*kb_per_page+pero2)/per,
	   (*(pgpgin+tog)-*(pgpgin+(!tog))+pero2)/per,
	   (*(pgpgout+tog)-*(pgpgout+(!tog))+pero2)/per,
	   (*(inter+tog)-*(inter+(!tog))+pero2)/per,
	   (*(ctxt+tog)-*(ctxt+(!tog))+pero2)/per,
	   (100*duse+divo2)/div,(100*dsys+divo2)/div,(100*didl+divo2)/div);
  }
  exit(EXIT_SUCCESS);
}

/**************************** others ***********************************/

void usage(void) {
  fprintf(stderr,"usage: %s [-V] [-n] [delay [count]]\n",PROGNAME);
  fprintf(stderr,"              -V prints version.\n");
  fprintf(stderr,"              -n causes the headers not to be reprinted regularly.\n");
  fprintf(stderr,"              delay is the delay between updates in seconds. \n");
  fprintf(stderr,"              count is the number of updates.\n");
  exit(EXIT_FAILURE);
}

void crash(char *filename) {
    perror(filename);
    exit(EXIT_FAILURE);
}

int winhi(void) {
    struct winsize win;
    int rows = 24;
 
    if (ioctl(1, TIOCGWINSZ, &win) != -1 && win.ws_row > 0)
      rows = win.ws_row;
 
    return rows;
}


void showheader(void){
  printf("%8s%28s%8s%12s%11s%12s\n",
	 "procs","memory","swap","io","system","cpu");
  printf("%2s %2s %2s %6s %6s %6s %6s %3s %3s %5s %5s %4s %5s %3s %3s %3s\n",
	 "r","b","w","swpd","free","buff","cache","si","so","bi","bo",
	 "in","cs","us","sy","id");
}

void getstat(unsigned *cuse, unsigned *cice, unsigned *csys, unsigned long *cide,
	     unsigned *pin, unsigned *pout, unsigned *sin, unsigned *sout,
	     unsigned *itot, unsigned *i1, unsigned *ct) {
  static int stat;

  if ((stat=open("/proc/stat", O_RDONLY, 0)) != -1) {
    char* b;
    buff[BUFFSIZE-1] = 0;  /* ensure null termination in buffer */
    read(stat,buff,BUFFSIZE-1);
    close(stat);
    *itot = 0; 
    *i1 = 1;   /* ensure assert below will fail if the sscanf bombs */
    b = strstr(buff, "cpu ");
    sscanf(b, "cpu  %u %u %u %lu", cuse, cice, csys, cide);
    b = strstr(buff, "page ");
    sscanf(b, "page %u %u", pin, pout);
    b = strstr(buff, "swap ");
    sscanf(b, "swap %u %u", sin, sout);
    b = strstr(buff, "intr ");
    sscanf(b, "intr %u %u", itot, i1);
    b = strstr(buff, "ctxt ");
    sscanf(b, "ctxt %u", ct);
  }
  else {
    crash("/proc/stat");
  }
}

void getmeminfo(unsigned *memfree, unsigned *membuff, unsigned *swapused, unsigned *memcache) {
  unsigned long long** mem;
  if (!(mem = meminfo())) crash("/proc/meminfo");
  *memfree  = mem[meminfo_main][meminfo_free]    >> 10;	/* bytes to k */
  *membuff  = mem[meminfo_main][meminfo_buffers] >> 10;
  *swapused = mem[meminfo_swap][meminfo_used]    >> 10;
  *memcache = mem[meminfo_main][meminfo_cached]  >> 10;
}

void getrunners(unsigned int *running, unsigned int *blocked, 
		unsigned int *swapped) {
  static struct direct *ent;
  static char filename[80];
  static int fd;
  static unsigned size;
  static char c;
  DIR *proc;

  *running=0;
  *blocked=0;
  *swapped=0;

  if ((proc=opendir("/proc"))==NULL) crash("/proc");

  while((ent=readdir(proc))) {
    if (isdigit(ent->d_name[0])) {  /*just to weed out the obvious junk*/
      sprintf(filename, "/proc/%s/stat", ent->d_name);
      if ((fd = open(filename, O_RDONLY, 0)) != -1) { /*this weeds the rest*/
	read(fd,buff,BUFFSIZE-1);
	sscanf(buff, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %*u %*d %*u %u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u\n",&c,&size);
	close(fd);

	if (c=='R') {
	  if (size>0) (*running)++;
	  else (*swapped)++;
        }
	else if (c=='D') {
	  if (size>0) (*blocked)++;
	  else (*swapped)++;
        }
      }
    }
  }
  closedir(proc);

#if 1
  /* is this next line a good idea?  It removes this thing which
     uses (hopefully) little time, from the count of running processes */
  (*running)--;
#endif
}
