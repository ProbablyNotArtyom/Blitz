/* init.c: boot, set up networking and stuff, and start a shell
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>

#include <fcntl.h>
#include <linux/sockios.h>
#include <linux/socket.h>
#include <linux/fs.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/icmp.h>
#include <linux/route.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <termios.h>
#include <signal.h>
#include <sys/time.h>

#include <serial.h>

#include <sys/utsname.h>

/* Keep track of console device, if any... */
char	*console_device = NULL;
int	console_baud = CONSOLE_BAUD_RATE;

void close_on_exec(int f)
{
	if (fcntl(f, F_SETFD, 1))
		perror("close on exec: ");
}

void close_all_fds(int keep_top)
{
	int i;
	for(i=keep_top+1;i<NR_OPEN;i++)
		close(i);
}

void make_ascii_tty(struct termios * tty)
{
        /*
         *      Set or adjust tty modes.
         */
        tty->c_iflag &= ~(INLCR|IGNCR|IUCLC);
        tty->c_iflag |= ICRNL;
        tty->c_oflag &= ~(OCRNL|OLCUC|ONOCR|ONLRET|OFILL);
        tty->c_oflag |= OPOST|ONLCR;
        tty->c_cflag |= CLOCAL;
        tty->c_lflag  = ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE;

	cfsetospeedn(tty, console_baud);

        /*
         *      Set the most important characters
         */
        tty->c_cc[VINTR]  = 3;
        tty->c_cc[VQUIT]  = 28;
        tty->c_cc[VERASE] = 8/*127*/;
        tty->c_cc[VKILL]  = 24;
        tty->c_cc[VEOF]   = 4;
        tty->c_cc[VTIME]  = 0;
        tty->c_cc[VMIN]   = 1;
        tty->c_cc[VSTART] = 17;
        tty->c_cc[VSTOP]  = 19;
        tty->c_cc[VSUSP]  = 26;
}

#define MAX_TASKS 4
#define MAX_SERVICES 4

struct {
	int enabled;
	char name[32];
	int console;
	int pid;
	int reconfig;
	
	int changed;
} task[MAX_TASKS];

struct {
	int enabled;
	char name[32];
	int port;
	int tcp;
	int reconfig;
	
	int master_socket;
	
	int limit;
	int current;
	int pid[16];

	int changed;
} service[MAX_SERVICES];


int reap_child(int pid) {
  int i;
  char buf[64];
  int reaped = 0;
  for(i=0;i<MAX_TASKS;i++) {
    if (task[i].pid == pid) {
      task[i].pid = 0;
      reaped++;
    }
  }
  for(i=0;i<MAX_SERVICES;i++) {
    int j;
    for(j=0;j<16;j++)
      if (service[i].pid[j] == pid) {
        service[i].pid[j] = 0;
        service[i].current--;
      }
  }
  return reaped;
}

int start_children(void) {
  int i;
  for(i=0;i<MAX_TASKS;i++) {
    if (!task[i].pid && strlen(task[i].name) && task[i].enabled) {
      /*printf("init: Reforking task '%s'\n", task[i].name);*/
      if (!(task[i].pid=vfork())) {
      	if (task[i].console) {
      		int j;
      		struct termios tty;
      		close(0);
      		close(1);
      		close(2);

	        /*printf("task %d: setpgrp = %d (%s)\n", getpid(), i, strerror(errno));*/
	        j = setsid();

		
		if (!console_device || open(console_device, O_RDWR|O_NONBLOCK) == -1)
      			if (open("/dev/tty1", O_RDWR|O_NONBLOCK) == -1)
      				open("/dev/ttyS0", O_RDWR|O_NONBLOCK);
      		fcntl(0, F_SETFL, 0);
      		dup(0); 
      		dup(0);

		tcgetattr(0, &tty);
       
	        make_ascii_tty(&tty);

	        tcsetattr(0, TCSANOW, &tty);

	        /*printf("task %d: setsid = %d (%s)\n", getpid(), j, strerror(errno));*/

	        j = ioctl(0, TIOCSCTTY, (char*)0);
	        /*printf("task %d: IOCSCTTY = %d (%s)\n", getpid(), j, strerror(errno));*/
	           
        	close(0);
	        close(1);
	        close(2);
      		open("/dev/tty", O_RDWR|O_NONBLOCK);
      		fcntl(0, F_SETFL, 0);
      		dup(0);
      		dup(0);
           
        } else {
        	close(0);
        	close(1);
        	close(2);

		if (!console_device || open(console_device, O_RDWR|O_NONBLOCK) == -1)
      			if (open("/dev/tty1", O_RDWR|O_NONBLOCK) == -1)
      				open("/dev/ttyS0", O_RDWR|O_NONBLOCK);
      		fcntl(0, F_SETFL, 0);
      		dup(0); 
      		dup(0);

        }

        close_all_fds(2);

        execlp(task[i].name, task[i].name, 0);
        /*printf("Failure starting task %s, disabling.\n", task[i].name);
        fflush(stdout);*/
        task[i].enabled = 0;
        task[i].pid = 0;
        _exit(0);
      }
    }
  }
}

int generate_select_fds(fd_set * readmask, fd_set * writemask)
{
  int i;
  int max=0;
  FD_ZERO(readmask);
  FD_ZERO(writemask);
#define FD_MSET(x,y) FD_SET((x),(y)); if ((x)>max) max = (x);

  for(i=0;i<MAX_SERVICES;i++) {
  
    if (!strlen(service[i].name) || (service[i].current >= service[i].limit))
      continue;
      
    FD_MSET(service[i].master_socket, readmask);

  }
  
  return max+1;
}

int handle_incoming_fds(fd_set * readmask, fd_set * writemask)
{
  int i;
  

  for(i=0;i<MAX_SERVICES;i++) {
    int fd;
    if (service[i].master_socket && FD_ISSET(service[i].master_socket, readmask)) {
      int j;
      for(j=0;j<16;j++)
        if (service[i].pid[j] == 0)
          break;

      if (service[i].tcp) {
        struct sockaddr_in remote;
        int remotelen = sizeof(remote);
        fd = accept(service[i].master_socket, (struct sockaddr*)&remote, &remotelen);
        if (fd < 0) {
          printf("accept failed\n");
          break;
        }
      } else {
        fd = service[i].master_socket;
      }

      if (!(service[i].pid[j] = vfork())) {
        if (fd != 0)
          dup2(fd, 0);
        if (fd != 1)
          dup2(fd, 1);
        if (fd != 2)
          dup2(fd, 2);
        if (fd > 2)
          close(fd);
        close_all_fds(2);
        service[i].current++;
        execlp(service[i].name, service[i].name, NULL);

        service[i].current--;
        close(service[i].master_socket);
        service[i].enabled = 0;
        service[i].master_socket = 0;
        _exit(0);
      }
    
      if (service[i].tcp) {
        close(fd);
      }
    }
  }

}


int start_sockets(void) {
  int s;
  int i;
  struct server_sockaddr;
  
  for(i=0;i<MAX_SERVICES;i++) {
    struct sockaddr_in server_sockaddr;

    if (service[i].master_socket || !strlen(service[i].name))
      continue;
      
    if (service[i].tcp) {
      int true;
      
      if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Unable to create socket\n");
      }
      
      close_on_exec(s);

      server_sockaddr.sin_family = AF_INET;
      server_sockaddr.sin_port = htons(service[i].port);
      server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
     
      true = 1;

      if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (void *)&true, 
         sizeof(true))) == -1) {
        perror("setsockopt: ");
      }

      if(bind(s, (struct sockaddr *)&server_sockaddr, 
        sizeof(server_sockaddr)) == -1)  {
        perror("Unable to bind server socket: ");
        close(s);
      }

      if(listen(s, 1) == -1) {
        printf("Unable to listen to socket\n");
      }
    
    } else {

      if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("Unable to create socket\n");
      }
      
      close_on_exec(s);

      server_sockaddr.sin_family = AF_INET;
      server_sockaddr.sin_port = htons(service[i].port);
      server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

      if(bind(s, (struct sockaddr *)&server_sockaddr, 
        sizeof(server_sockaddr)) == -1)  {
        printf("Unable to bind socket\n");
        close(s);
      }

    }

    service[i].master_socket = s;

  }
  
  
}

int stop_sockets(void) {
  int s;
  int i;
  struct server_sockaddr;
  
  for(i=0;i<MAX_SERVICES;i++) {
    if(service[i].master_socket)
      close(service[i].master_socket);
    service[i].master_socket = 0;
  }
  
}

void reap_children(void)
{
  int child;
  int status;
  /*printf("Checking for dead children\n");*/
  while ((child = waitpid(-1, &status, WNOHANG)) > 0) {
    /*printf("init: process %d died with status %d", child, WEXITSTATUS(status));
    if (WIFSIGNALED(status))
      printf(", due to signal %d", child, WTERMSIG(status));
    printf("\n");*/
    reap_child(child);
  }
}

volatile int got_hup;
void hup_handler(int signo)
{
	got_hup = 1;
	
}
volatile int got_cont;
void cont_handler()
{
	got_cont = 1;
}

void stop_handler()
{
	/*printf("Init stopped\n");*/
	got_cont = 0;
	stop_sockets(); /* To reduce memory usage, and prevent callers from getting gummed up */
	while(!got_cont) {
		pause();
		reap_children();
	}
	/*printf("Init restarted\n");*/
	got_cont = 0;
}

void child_handler(int signo)
{
	/* Don't reap, just interrupt the syscall */
}

char * pstrdup(char * c) {
	char * _c = c;
	if (_c)
		_c = strdup(_c);
	return _c;
}

void kill_changed_things(void)
{
	int i;

	for(i=0;i<MAX_SERVICES;i++) {
		int j;
		
		if (!service[i].changed && !service[i].reconfig)
			continue;
		
		if (service[i].master_socket) {
			close(service[i].master_socket);
			service[i].master_socket = 0;
		}
		
		for (j=0;j<16;j++) {
			if (service[i].pid[j] != 0) {
				kill(service[i].pid[j], SIGTERM);
				kill(service[i].pid[j], SIGHUP);
			}
		}
		
		service[i].changed = 0;
	}

	for(i=0;i<MAX_TASKS;i++) {
		if (!task[i].changed && !task[i].reconfig)
			continue;
		
		if (task[i].pid) {
			kill(task[i].pid, SIGTERM);
			kill(task[i].pid, SIGHUP);
			/* SIGCHLD will clean out the pid */
		}
		
		task[i].changed = 0;
	}
}

void run(void)
{
  fd_set rfds, wfds;
  struct timeval tv;
  int max;
  for(;;) {
    reap_children();
    start_sockets();
    start_children();
    max = generate_select_fds(&rfds, &wfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (select(max, &rfds, &wfds, 0, &tv) > 0) {
      handle_incoming_fds(&rfds, &wfds);
    }
    if (got_hup) {
    	got_hup = 0;
    	/* Reread configuration, somehow */
    	kill_changed_things();
    	continue;
    }
  }
}

void banner()
{
	struct utsname n;
	
	memset(&n, 0, sizeof(n));
	uname(&n);
	printf("\n\n");
#if 0
	printf("=============================================================================\n");
	printf("\n\t\t%s\n\n", TOOLCHAIN_BANNER);
	printf("%s release %s, build %s\n", n.sysname, n.release, n.version);
	printf("%s release %s, build %s\n\n", TOOLCHAIN_NAME, TOOLCHAIN_RELEASE, TOOLCHAIN_VERSION);
#endif
	printf("=============================================================================\n");
	printf("\n\n");
}

main(int argc, char *argv[], char *env[])
{
	char *sp;
	int i, s;
	struct termios tty;
	int servicenr, tasknr;

  	got_cont = 0;
  	got_hup = 0;

#ifdef EMBED
  __signal(SIGPIPE, SIG_IGN, 0);
  __signal(SIGSTOP, stop_handler, 0);
  __signal(SIGTSTP, stop_handler, 0);
  __signal(SIGCONT, cont_handler, 0);
  __signal(SIGCHLD, child_handler, SA_INTERRUPT);
  __signal(SIGHUP, hup_handler, 0);
#endif  

	/* Set up console in a useful manner */
	if ((console_device = getenv("CONSOLE"))) {
		unsetenv("CONSOLE");
		if ((sp = strchr(console_device, ','))) {
			*sp++ = 0;
			i = atoi(sp);
			if ((i > 0) && (i <= 460800))
				console_baud = i;
		}
	}

	tcgetattr(0, &tty);
	make_ascii_tty(&tty);
	tcsetattr(0, TCSANOW, &tty);
	banner();

  	putenv("PATH=/bin");

#ifdef DS1302
	//;'pa990606 +
	//re-load the system time from the RTC Chip
	if (!(i=vfork()))
	{
		execlp("clock", "clock", "--sys", NULL);
		_exit(0);
	}
#endif

        printf("Mounting proc on /proc\n");
	i = mount("proc", "/proc", "proc", 0xC0ED0000, 0);

#if defined(BUILD_BIG) || defined(BUILD_NETtel) || defined(BUILD_ELITE) || defined(BUILD_CADRE3)
        printf("Expanding initial ramdisk image into /dev/ram0\n");
	expand("/etc/ramfs.img", "/dev/ram0");
	
        printf("Mounting /dev/ram0 on /var\n");
	i = mount("/dev/ram0", "/var", "ext2", /*0xC0ED0000*/ MS_MGC_VAL | MS_SYNCHRONOUS, 0);

	fflush(stdout);

        printf("Making /var directories\n");
	mkdir("/var/tmp", 0777);
	mkdir("/var/log", 0777);
#endif
#if defined(BUILD_NETtel)
        printf("Expanding configuration ramdisk image into /dev/ram1\n");
	expand("/etc/ramfs.img", "/dev/ram1");
        printf("Mounting /dev/ram1 on /etc/config\n");
	i = mount("/dev/ram1", "/etc/config", "ext2",
		(MS_MGC_VAL | MS_SYNCHRONOUS), 0);
	fflush(stdout);
	if (!(i=vfork())) {
		execlp("flatfsd", "flatfsd", "-r", NULL);
		_exit(0);
	}
	waitpid(i, &s, 0);
#endif

#if defined(BUILD_NETtel)
	if (!(i=vfork())) {
		execlp("sh", "sh", "/etc/config/start", NULL);
		_exit(0);
	}
	waitpid(i, &s, 0);
#else
	/* Attach loopback in separate process. (There's no particular
	   reason this needs to be done in a separate process, beyond
	   keeping init as trim as possible.)
	*/

#ifndef(BUILD_SMALL)
        printf("Attaching loopback device\n");
	if (!(i=vfork())) {
		execlp("loattach", "loattach", NULL);
		_exit(0);
	}
#endif
#endif
#if defined(BUILD_BIG) || defined(BUILD_CADRE3)
        printf("Starting DHCP/bootp client\n");
	if (!(i=vfork())) {
		execlp("dhcpcd", "dhcpcd", NULL);
		_exit(0);
	}
#endif

#if 0
	/* Attach SLIP line (hack, this should be configured) */
	if (!(i=vfork())) {
		execlp("slattach", "slattach", 
				"-p", "/dev/ttyS0", 
				"-s", "57600,N,8,1,L",
				"-m", "255.255.255.0",
				"-t", "60",
				"192.168.165.99",
				"192.168.165.84",
				 NULL);
		_exit(0);
	}
#endif

	tasknr = 0;

#if defined(BUILD_NETtel)
	if (console_device && strcmp(console_device, "/dev/null")) {
		strcpy(task[tasknr].name, "sh");
		task[tasknr].console = 1;
		task[tasknr].enabled = 1;
		tasknr++;
	}
	strcpy(task[tasknr].name, "flatfsd");
	task[tasknr].enabled = 1;
	tasknr++;
	strcpy(task[tasknr].name, "boa");
	task[tasknr].enabled = 1;
	tasknr++;
#else
	strcpy(task[tasknr].name, "sh");
	task[tasknr].console = 1;
	task[tasknr].enabled = 1;
	tasknr++;
#endif
#if defined(BUILD_BIG) || defined(BUILD_ELITE) || defined(BUILD_CADRE3)
	strcpy(task[tasknr].name, "httpd");
	task[tasknr].enabled = 1;
	tasknr++;
#endif

	/* install our handy-dandy inetd-style services */
	servicenr = 0;
#if 0
	strcpy(service[servicenr].name, "rootloader");
	service[servicenr].port = 194;
	service[servicenr].tcp = 1;
	service[servicenr].enabled = 1;
	service[servicenr].limit = 1;
	servicenr++;
#endif

#if defined(BUILD_BIG) || defined(BUILD_NETtel) || defined(BUILD_ELITE) || defined(BUILD_CADRE3)
	strcpy(service[servicenr].name, "telnetd");
	service[servicenr].port = 23;
	service[servicenr].tcp = 1;
	service[servicenr].enabled = 1;
	service[servicenr].limit = 16;
	servicenr++;
#endif

#if defined(BUILD_BIG)
	strcpy(service[servicenr].name, "tftpd");
	service[servicenr].port = 69;
	service[servicenr].tcp = 0;
	service[servicenr].enabled = 1;
	service[servicenr].limit = 16;
	servicenr++;
#endif

#if defined(BUILD_BIG) || defined(BUILD_NETtel) || defined(BUILD_ELITE) || defined(BUILD_CADRE3)
	strcpy(service[servicenr].name, "discard");
	service[servicenr].port = 9;
	service[servicenr].tcp = 0;
	service[servicenr].enabled = 1;
	service[servicenr].limit = 16;
	servicenr++;

	strcpy(service[servicenr].name, "discard");
	service[servicenr].port = 9;
	service[servicenr].tcp = 1;
	service[servicenr].enabled = 1;
	service[servicenr].limit = 16;
	servicenr++;
#endif

        printf("Starting task manager\n");

#if 0
	printf("Execing console shell\n");
	if (!(i=vfork())) {
        	execlp("/bin/sh", "sh", 0);
		_exit(0);
	}
#endif

	run();

	/* not reached */
}
