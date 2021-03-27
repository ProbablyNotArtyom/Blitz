/* vi: set sw=4 ts=4: */
/*
 * Mini syslogd implementation for busybox
 *
 * Copyright (C) 1999,2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 1999,2000,2001 by Erik Andersen <andersee@debian.org>
 *
 * Copyright (C) 2000 by Karl M. Hegbloom <karlheg@debian.org>
 *
 * "circular buffer" Copyright (C) 2001 by Gennady Feldman <gfeldman@cachier.com>
 *
 * Maintainer: Gennady Feldman <gena01@cachier.com> as of Mar 12, 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <paths.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/param.h>
#include <config/autoconf.h>

#include "busybox.h"

/* SYSLOG_NAMES defined to pull some extra junk from syslog.h */
#define SYSLOG_NAMES
#include <sys/syslog.h>
#include <sys/uio.h>

/* Path for the file where all log messages are written */
#define __LOG_FILE "/var/log/messages"

/* Path to the unix socket */
static char lfile[MAXPATHLEN];

static char *logFilePath = __LOG_FILE;
#ifdef EMBED
static int logFileMaxSize = 16384;
#endif

/* interval between marks in seconds */
#ifndef CONFIG_USER_SMTP_SMTPCLIENT
#ifdef EMBED
static int MarkInterval = 0;
#else
static int MarkInterval = 20 * 60;
#endif
#endif

/* localhost's name */
static char LocalHostName[64];

#ifdef BB_FEATURE_REMOTE_LOG
#include <netinet/in.h>

struct RemoteLogInfo {
	int fd;					/* udp socket for logging to remote host */
	char *hostname;			/* where do we log? */
	int port;				/* what port to log to? */
	int filterlevel;		/* (7 - LOG_xxx) only logs messages at priority LOG_xxx and higher.
	                         * 0 = log all. 8 = log none
							 */
	int enable;
	struct sockaddr_in saremote;   /* sockaddr representing hostname, port */
};

#ifdef CONFIG_USER_MGMT_MGMT
#define NUM_REMOTE_HOSTS 2
#else
#define NUM_REMOTE_HOSTS 1
#endif

static struct RemoteLogInfo remote_log_info[NUM_REMOTE_HOSTS] = {
  { -1, 0, 514, 7 - LOG_DEBUG, 0 },
#if NUM_REMOTE_HOSTS > 1
  { -1, 0, 514, 7 - LOG_DEBUG, 0 },
#endif
};

static int remote_logging_initialised = FALSE;
static int remote_logging = FALSE;
static int local_logging = FALSE;
static void init_RemoteLog(void);
#endif
static int ext_date = FALSE;
#ifdef CONFIG_USER_SMTP_SMTPCLIENT
static unsigned char msys_on, msys_lev = 4;
static int msys_freq, msys_time = 60;
static const char *msys_tgt, *msys_smtpd;
static FILE *msys_p;
static volatile int msys_c;
static volatile time_t msys_target;
#endif

#define MAXLINE         1024            /* maximum line length */
#define MIN_ERRMSG_INTERVAL 300

/* circular buffer variables/structures */
#ifdef BB_FEATURE_IPC_SYSLOG
#if __GNU_LIBRARY__ < 5
#error Sorry.  Looks like you are using libc5.
#error libc5 shm support isnt good enough.
#error Please disable BB_FEATURE_IPC_SYSLOG
#endif

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

/* our shared key */
static const long KEY_ID = 0x414e4547; /*"GENA"*/

// Semaphore operation structures
static struct shbuf_ds {
	int size;               // size of data written
	int head;               // start of message list
	int tail;               // end of message list
	char data[1];           // data/messages
} *buf = NULL;                  // shared memory pointer

static struct sembuf SMwup[1] = {{1, -1, IPC_NOWAIT}}; // set SMwup
static struct sembuf SMwdn[3] = {{0, 0}, {1, 0}, {1, +1}}; // set SMwdn

static int      shmid = -1;     // ipc shared memory id
static int      s_semid = -1;   // ipc semaphore id
int     data_size = 16000; // data size
int     shm_size = 16000 + sizeof(*buf); // our buffer size
static int circular_logging = FALSE;

/*
 * sem_up - up()'s a semaphore.
 */
static inline void sem_up(int semid)
{
	if ( semop(semid, SMwup, 1) == -1 )
		perror_msg_and_die("semop[SMwup]");
}

/*
 * sem_down - down()'s a semaphore
 */
static inline void sem_down(int semid)
{
	if ( semop(semid, SMwdn, 3) == -1 )
		perror_msg_and_die("semop[SMwdn]");
}


void ipcsyslog_cleanup(void){
	printf("Exiting Syslogd!\n");
	if (shmid != -1)
		shmdt(buf);

	if (shmid != -1)
		shmctl(shmid, IPC_RMID, NULL);
	if (s_semid != -1)
		semctl(s_semid, 0, IPC_RMID, 0);
}

void ipcsyslog_init(void){
	if (buf == NULL){
	    if ((shmid = shmget(KEY_ID, shm_size, IPC_CREAT | 1023)) == -1)
			perror_msg_and_die("shmget");


	    if ((buf = shmat(shmid, NULL, 0)) == NULL)
			perror_msg_and_die("shmat");


	    buf->size=data_size;
	    buf->head=buf->tail=0;

	    // we'll trust the OS to set initial semval to 0 (let's hope)
	    if ((s_semid = semget(KEY_ID, 2, IPC_CREAT | IPC_EXCL | 1023)) == -1){
		if (errno == EEXIST){
		   if ((s_semid = semget(KEY_ID, 2, 0)) == -1)
		    perror_msg_and_die("semget");
		}else
			perror_msg_and_die("semget");
	    }
	}else{
		printf("Buffer already allocated just grab the semaphore?");
	}
}

/* write message to buffer */
void circ_message(const char *msg){
	int l=strlen(msg)+1; /* count the whole message w/ '\0' included */

	sem_down(s_semid);

	/*
	 * Circular Buffer Algorithm:
	 * --------------------------
	 *
	 * Start-off w/ empty buffer of specific size SHM_SIZ
	 * Start filling it up w/ messages. I use '\0' as separator to break up messages.
	 * This is also very handy since we can do printf on message.
	 *
	 * Once the buffer is full we need to get rid of the first message in buffer and
	 * insert the new message. (Note: if the message being added is >1 message then
	 * we will need to "remove" >1 old message from the buffer). The way this is done
	 * is the following:
	 *      When we reach the end of the buffer we set a mark and start from the beginning.
	 *      Now what about the beginning and end of the buffer? Well we have the "head"
	 *      index/pointer which is the starting point for the messages and we have "tail"
	 *      index/pointer which is the ending point for the messages. When we "display" the
	 *      messages we start from the beginning and continue until we reach "tail". If we
	 *      reach end of buffer, then we just start from the beginning (offset 0). "head" and
	 *      "tail" are actually offsets from the beginning of the buffer.
	 *
	 * Note: This algorithm uses Linux IPC mechanism w/ shared memory and semaphores to provide
	 *       a threasafe way of handling shared memory operations.
	 */
	if ( (buf->tail + l) < buf->size ){
		/* before we append the message we need to check the HEAD so that we won't
		   overwrite any of the message that we still need and adjust HEAD to point
		   to the next message! */
		if ( buf->tail < buf->head){
			if ( (buf->tail + l) >= buf->head ){
			  /* we need to move the HEAD to point to the next message
			   * Theoretically we have enough room to add the whole message to the
			   * buffer, because of the first outer IF statement, so we don't have
			   * to worry about overflows here!
			   */
			   int k= buf->tail + l - buf->head; /* we need to know how many bytes
								we are overwriting to make
								enough room */
			   char *c=memchr(buf->data+buf->head + k,'\0',buf->size - (buf->head + k));
			   if (c != NULL) {/* do a sanity check just in case! */
				buf->head = c - buf->data + 1; /* we need to convert pointer to
								  offset + skip the '\0' since
								  we need to point to the beginning
								  of the next message */
				/* Note: HEAD is only used to "retrieve" messages, it's not used
					when writing messages into our buffer */
			   }else{ /* show an error message to know we messed up? */
				printf("Weird! Can't find the terminator token??? \n");
				buf->head=0;
			   }
			}
		} /* in other cases no overflows have been done yet, so we don't care! */

		/* we should be ok to append the message now */
		strncpy(buf->data + buf->tail,msg,l); /* append our message */
		buf->tail+=l; /* count full message w/ '\0' terminating char */
	}else{
		/* we need to break up the message and "circle" it around */
		char *c;
		int k=buf->tail + l - buf->size; /* count # of bytes we don't fit */

		/* We need to move HEAD! This is always the case since we are going
		 * to "circle" the message.
		 */
		c=memchr(buf->data + k ,'\0', buf->size - k);

		if (c != NULL) /* if we don't have '\0'??? weird!!! */{
			/* move head pointer*/
			buf->head=c-buf->data+1;

			/* now write the first part of the message */
			strncpy(buf->data + buf->tail, msg, l - k - 1);

			/* ALWAYS terminate end of buffer w/ '\0' */
			buf->data[buf->size-1]='\0';

			/* now write out the rest of the string to the beginning of the buffer */
			strcpy(buf->data, &msg[l-k-1]);

			/* we need to place the TAIL at the end of the message */
			buf->tail = k + 1;
		}else{
			printf("Weird! Can't find the terminator token from the beginning??? \n");
			buf->head = buf->tail = 0; /* reset buffer, since it's probably corrupted */
		}

	}
	sem_up(s_semid);
}
#endif  /* BB_FEATURE_IPC_SYSLOG */

/* Note: There is also a function called "message()" in init.c */
/* Print a message to the log file. */
static void message (char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
static void message (char *fmt, ...)
{
	int fd;
	struct flock fl;
	va_list arguments;

	fl.l_whence = SEEK_SET;
	fl.l_start  = 0;
	fl.l_len    = 1;

#ifdef BB_FEATURE_IPC_SYSLOG
	if ((circular_logging == TRUE) && (buf != NULL)){
			char b[1024];
			va_start (arguments, fmt);
			vsnprintf (b, sizeof(b)-1, fmt, arguments);
			va_end (arguments);
			circ_message(b);

	}else
#endif
	if ((fd = device_open (logFilePath,
						   O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND |
						   O_NONBLOCK)) >= 0) {
		fl.l_type = F_WRLCK;
		fcntl (fd, F_SETLKW, &fl);
		va_start (arguments, fmt);
		vdprintf (fd, fmt, arguments);
		va_end (arguments);
		fl.l_type = F_UNLCK;
		fcntl (fd, F_SETLKW, &fl);
#ifdef EMBED
		{
			struct stat st;
			char buf[128];

			if (fstat(fd, &st) != -1 && st.st_size >= logFileMaxSize) {
				snprintf(buf, sizeof(buf), "%s.old", logFilePath);
				rename(logFilePath, buf);
			}
		}
#endif
		close (fd);
	} else {
		/* Always send console messages to /dev/console so people will see them. */
		if ((fd = device_open (_PATH_CONSOLE,
							   O_WRONLY | O_NOCTTY | O_NONBLOCK)) >= 0) {
			va_start (arguments, fmt);
			vdprintf (fd, fmt, arguments);
			va_end (arguments);
			close (fd);
		} else {
			fprintf (stderr, "Bummer, can't print: ");
			va_start (arguments, fmt);
			vfprintf (stderr, fmt, arguments);
			fflush (stderr);
			va_end (arguments);
		}
	}
}

static void logOneMessage(int pri, const char *msg, const char *timestamp, struct timeval *tv)
{
	struct tm *tm;
	static char res[20] = "";
	CODE *c_pri, *c_fac;
	char iso_time[22];
	char *content;

	if (pri != 0) {
		for (c_fac = facilitynames;
				c_fac->c_name && !(c_fac->c_val == LOG_FAC(pri) << 3); c_fac++);
		for (c_pri = prioritynames;
				c_pri->c_name && !(c_pri->c_val == LOG_PRI(pri)); c_pri++);
		if (c_fac->c_name == NULL || c_pri->c_name == NULL)
			snprintf(res, sizeof(res), "<%d>", pri);
		else
			snprintf(res, sizeof(res), "%s.%s", c_fac->c_name, c_pri->c_name);
	}

	if (ext_date) {
		tm = localtime(&tv->tv_sec);
		snprintf(iso_time, 22,  "(%.4d%.2d%.2dT%.2d%.2d%.2d%.3lu) ", 
				tm->tm_year + 1900, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, tv->tv_usec / 1000);
		content = strchr(msg, ' ');
		if (content && (*(content - 1) == ':')) {
			*content = '\0';
			content++;
		} else {
			content = "";
			iso_time[0] = '\0';
		}
	} else {
		content = "";
		iso_time[0] = '\0';
	}


#ifdef CONFIG_USER_SMTP_SMTPCLIENT
	if (msys_on && LOG_PRI(pri) <= msys_lev) {
		unsigned int oldalrm = 0;
		if (msys_time > 0) oldalrm = alarm(0);
		if (msys_p == NULL) {
			char buf[999];
			char myname[200];
			if (gethostname(myname, sizeof(myname)-1) < 0)
				strcpy(myname, "unknownhost");
			else
				myname[sizeof(myname)-1] = '\0';
			snprintf(buf, sizeof(buf),
				"/bin/mail -R -s syslog_%s -S %s -H %s %s",
					LocalHostName,
					msys_smtpd,
					myname, msys_tgt);
			msys_p = popen(buf, "w");
			if (msys_time > 0) {
				alarm(msys_time);
				msys_target = time(NULL) + msys_time;
			}
		} else
			fprintf(msys_p, "\n========\n\n");
		fprintf(msys_p, "%s<%d>%s %s\n\t%s\n\t%s", iso_time, pri,
				timestamp, LocalHostName, msg, content);
		if ((msys_freq > 0 && ++msys_c >= msys_freq) ||
				(msys_freq == 0 && msys_time == 0) ||
				(msys_time > 0 && msys_target <= time(NULL))) {
			pclose(msys_p);
			msys_p = NULL;
			msys_c = 0;
			if (msys_time > 0) {
				alarm(0);
				msys_target = 0;
			}
		} else if (oldalrm > 0)
			alarm(oldalrm);
	}
#endif

#ifdef BB_FEATURE_REMOTE_LOG
		/* send message to remote logger */
	if (!remote_logging_initialised) {
		  init_RemoteLog();
		  remote_logging_initialised = TRUE;
	}
	if (remote_logging) {
		char buffer[1024];
		int i;
		snprintf(buffer, 1024, "<%d>%s %s %s %s%s", pri, timestamp, LocalHostName, msg, iso_time, content);

		for (i = 0; i < NUM_REMOTE_HOSTS; i++) {
			if (!remote_log_info[i].enable || remote_log_info[i].fd == -1) {
				continue;
			}
			if (7 - LOG_PRI(pri) < remote_log_info[i].filterlevel) {
				continue;
			}
		writev_retry:
			if ( -1 == sendto(remote_log_info[i].fd, buffer, strlen(buffer), 0,
				(struct sockaddr *)&remote_log_info[i].saremote, sizeof(remote_log_info[i].saremote))){
				time_t now;
				static time_t last_message_time = 0;

				if (errno == EINTR) goto writev_retry;

				/* Throttle these messages so that we don't get one after every message if the network is down */
				now = time(0);
				if (now - last_message_time > MIN_ERRMSG_INTERVAL) {
					message("syslogd: cannot write to remote file handle on " 
							"%s:%d - %d\n",remote_log_info[i].hostname,remote_log_info[i].port,errno);
					last_message_time = now;
				}
			}
		}
	}
	if (local_logging == TRUE)
#endif
		/* now spew out the message to wherever it is supposed to go */
#ifdef EMBED
		message("<%d>%s %s %s%s\n", pri, timestamp, msg, iso_time, content);
#else
		message("%s %s %s %s\n", timestamp, LocalHostName, res, msg);
#endif
}

static void logMessage (int pri, char *msg)
{
	struct timeval tv;
	char  *timestamp;

	/* Count messages repeats */
	static int repeats = 0;
	static int old_pri = 0;
	static char old_msg[64];

	gettimeofday(&tv,NULL);

	if (strlen(msg) < 16 || msg[3] != ' ' || msg[6] != ' ' ||
			msg[9] != ':' || msg[12] != ':' || msg[15] != ' ') {
		timestamp = ctime(&(tv.tv_sec)) + 4;
		timestamp[15] = '\0';
	} else {
		timestamp = msg;
		timestamp[15] = '\0';
		msg += 16;
	}

		/* todo: supress duplicates */

	/* Now, is this a duplicate? */
	if (pri == old_pri && strncmp(msg, old_msg, sizeof(old_msg)) == 0) {
		/* Yes, so remember it but don't log it */
		repeats++;
		return;
	}
	else {
		/* No */
		if (repeats) {
			/* Not a repeat, but we previously had repeats, so output a message */
			snprintf(old_msg, sizeof(old_msg), "last message repeated %d time(s)", repeats);
			logOneMessage(old_pri, old_msg, timestamp, &tv);
			repeats = 0;
		}

		/* Remember the previous message */
		old_pri = pri;
		strncpy(old_msg, msg, sizeof(old_msg));
	}

	/* Log this message */
	logOneMessage(pri, msg, timestamp, &tv);
}

static void quit_signal(int sig)
{
	logMessage(LOG_SYSLOG | LOG_INFO, "System log daemon exiting.");
	unlink(lfile);
#ifdef BB_FEATURE_IPC_SYSLOG
	ipcsyslog_cleanup();
#endif

	exit(TRUE);
}

#ifdef CONFIG_USER_SMTP_SMTPCLIENT
static void domailtime(int sig) {
	if (msys_p != NULL) {
		pclose(msys_p);
		msys_p = NULL;
		msys_c = 0;
	}
}
#else
static void domark(int sig)
{
	if (MarkInterval > 0) {
		logMessage(LOG_SYSLOG | LOG_INFO, "-- MARK --");
		alarm(MarkInterval);
	}
}
#endif

/* This must be a #define, since when DODEBUG and BUFFERS_GO_IN_BSS are
 * enabled, we otherwise get a "storage size isn't constant error. */
static int serveConnection (char* tmpbuf, int n_read)
{
	int    pri_set = 0;
	char  *p = tmpbuf;

	/* SOCK_DGRAM messages do not have the terminating NUL,  add it */
	if (n_read > 0)
		tmpbuf[n_read] = '\0';

	while (p < tmpbuf + n_read) {

		int           pri = (LOG_USER | LOG_NOTICE);
		char          line[ MAXLINE + 1 ];
		char         *q = line;

		while (q < &line[ sizeof (line) - 1 ]) {
			if (!pri_set && *p == '<') {
			/* Parse the magic priority number. */
				pri = 0;
				p++;
				while (isdigit(*p)) {
					pri = 10 * pri + (*p - '0');
					p++;
				}
				if (pri & ~(LOG_FACMASK | LOG_PRIMASK)){
					pri = (LOG_USER | LOG_NOTICE);
				}
				pri_set = 1;
			} else if (*p == '\0') {
				pri_set = 0;
				*q = *p++;
				break;
			} else if (*p == '\n') {
				*q++ = ' ';
			} else if (iscntrl(*p) && (*p < 0177)) {
				*q++ = '^';
				*q++ = *p ^ 0100;
			} else {
				*q++ = *p;
			}
			p++;
		}
		*q = '\0';
		p++;
		/* Now log it */
		if (q > line)
			logMessage (pri, line);
	}
	return n_read;
}


#ifdef BB_FEATURE_REMOTE_LOG
static void init_RemoteLog (void)
{
	int i;
	struct hostent *hostinfo;

	remote_logging = FALSE;

	for (i = 0; i < NUM_REMOTE_HOSTS; i++) {
		if (!remote_log_info[i].enable || remote_log_info[i].hostname == 0 ||
			remote_log_info[i].port == 0 || remote_log_info[i].filterlevel > 7) {
			continue;
		}

		memset(&remote_log_info[i].saremote, 0, sizeof(remote_log_info[i].saremote));

		remote_log_info[i].fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (remote_log_info[i].fd < 0) {
			error_msg_and_die("syslogd: cannot create socket");
		}

		hostinfo = xgethostbyname(remote_log_info[i].hostname);
		remote_log_info[i].saremote.sin_family = AF_INET;
		remote_log_info[i].saremote.sin_addr = *(struct in_addr *) *hostinfo->h_addr_list;
		remote_log_info[i].saremote.sin_port = htons(remote_log_info[i].port);

		/* NOTE: We don't connect() the socket here since it can cause
		         problems if the outgoing interface is masqueraded */
		remote_logging = TRUE;
	}
}
#endif


#ifdef CONFIG_USER_FLATFSD_FLATFSD
/*
 *	load config files from the /etc/config area
 *
 *  syslog { <remote-server> <port> }
 *  syslog_maxsize <size-in-bytes>
 *  mgmt 0|1
 *  mgmt_server <remote-server>
 *  mgmt_syslogport <remote-port>
 *  mgmt_filter <syslog-filter-level>
 */

static void load_config_file_settings()
{
	FILE *fp;
	char line[80];
	char *whitespace = " \t";
	char *p;

	/* Read options from /etc/config/config. */
	if ((fp = fopen("/etc/config/config", "r")) != NULL) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if ((p = strchr(line, '\n')))
				*p = '\0';
			if ((p = strchr(line, '\r')))
				*p = '\0';
			p = strtok(line, whitespace);
			if (p) {
				if (strcmp(p, "syslog_maxsize") == 0) {
					p = strtok(NULL, whitespace);
					if (p && atoi(p) > 0)
						logFileMaxSize = atoi(p);
				}
#ifdef BB_FEATURE_REMOTE_LOG
				else if (!remote_logging && strcmp(p, "syslog") == 0) {
					if (remote_log_info[0].hostname)
						free(remote_log_info[0].hostname);
					remote_log_info[0].hostname = strtok(NULL, whitespace);
					if (remote_log_info[0].hostname) {
						remote_logging = TRUE;
						local_logging = TRUE; /* force both */
						p = strtok(NULL, whitespace);
						remote_log_info[0].port = 514;
						if (p && atoi(p) > 0) {
							remote_log_info[0].port = atoi(p);
						}
						remote_log_info[0].hostname =
								strdup(remote_log_info[0].hostname);
						remote_log_info[0].enable = 1;
						remote_log_info[0].filterlevel = 7 - LOG_DEBUG;
					}
				}
#endif
				else if (strcmp(p, "syslog_ext_date") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						ext_date = atoi(p);
					}
				}
#ifdef CONFIG_USER_MGMT_MGMT
				else if (strcmp(p, "mgmt") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						remote_log_info[1].enable = atoi(p);
					}
				}
				else if (strcmp(p, "mgmt_server") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						remote_log_info[1].hostname = strdup(p);
					}
				}
				else if (strcmp(p, "mgmt_syslogport") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						remote_log_info[1].port = atoi(p);
					}
				}
				else if (strcmp(p, "mgmt_filter") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						remote_log_info[1].filterlevel = atoi(p) - 1;
					}
				}
#endif
#ifdef CONFIG_USER_SMTP_SMTPCLIENT
				else if (strcmp(p, "mail_syslog") == 0) {
					p = strtok(NULL, whitespace);
					if (p) msys_on = atoi(p);
				} else if (strcmp(p, "mail_syslog_freq") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						msys_freq = atoi(p);
						if (msys_freq < 0) msys_freq = 0;
					}
				} else if (strcmp(p, "mail_syslog_level") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						int x = atoi(p);
						if (x < 0) msys_lev = 0;
						else if (x > 7) msys_lev = 7;
						else msys_lev = x;
					}
				} else if (strcmp(p, "mail_server") == 0) {
					p = strtok(NULL, whitespace);
					if (p) msys_smtpd = strdup(p);
				} else if (strcmp(p, "mail_address") == 0) {
					p = strtok(NULL, "");
					if (p) msys_tgt = strdup(p);
				} else if (strcmp(p, "mail_syslog_time") == 0) {
					p = strtok(NULL, whitespace);
					if (p) {
						msys_time = atoi(p);
						if (msys_time < 0) msys_time = 0;
					}
				}
#endif
			}
		}
		fclose(fp);
	}
}

#endif /* CONFIG_USER_FLATFSD_FLATFSD */


static void doSyslogd (void) __attribute__ ((noreturn));
static void doSyslogd (void)
{
	struct sockaddr_un sunx;
	socklen_t addrLength;

	int sock_fd;
	fd_set fds;

	/* Set up signal handlers. */
	signal (SIGINT,  quit_signal);
	signal (SIGTERM, quit_signal);
	signal (SIGQUIT, quit_signal);
	signal (SIGHUP,  SIG_IGN);
	signal (SIGCHLD,  SIG_IGN);
#ifdef SIGCLD
	signal (SIGCLD,  SIG_IGN);
#endif
#ifdef CONFIG_USER_SMTP_SMTPCLIENT
	signal(SIGALRM, domailtime);
#else
	signal (SIGALRM, domark);
	alarm (MarkInterval);
#endif

	/* Create the syslog file so realpath() can work. */
	if (realpath (_PATH_LOG, lfile) != NULL)
		unlink (lfile);

	memset (&sunx, 0, sizeof (sunx));
	sunx.sun_family = AF_UNIX;
	strncpy (sunx.sun_path, lfile, sizeof (sunx.sun_path));
	if ((sock_fd = socket (AF_UNIX, SOCK_DGRAM, 0)) < 0)
		perror_msg_and_die ("Couldn't get file descriptor for socket " _PATH_LOG);

	addrLength = sizeof (sunx.sun_family) + strlen (sunx.sun_path);
	if (bind(sock_fd, (struct sockaddr *) &sunx, addrLength) < 0)
		perror_msg_and_die ("Could not connect to socket " _PATH_LOG);

	if (chmod (lfile, 0666) < 0)
		perror_msg_and_die ("Could not set permission on " _PATH_LOG);


#ifdef BB_FEATURE_IPC_SYSLOG
	if (circular_logging == TRUE ){
	   ipcsyslog_init();
	}
#endif

	logMessage (LOG_SYSLOG | LOG_INFO, "syslogd started: " BB_BANNER);

	for (;;) {

		FD_ZERO (&fds);
		FD_SET (sock_fd, &fds);

		if (select (sock_fd+1, &fds, NULL, NULL, NULL) < 0) {
			if (errno == EINTR) {
				/* alarm may have happened. */
				continue;
			}
			perror_msg_and_die ("select error");
		}

		if (FD_ISSET (sock_fd, &fds)) {
		       int   i;
		       RESERVE_BB_BUFFER(tmpbuf, BUFSIZ + 1);

		       memset(tmpbuf, '\0', BUFSIZ+1);
		       if ( (i = recv(sock_fd, tmpbuf, BUFSIZ, 0)) > 0) {
			       serveConnection(tmpbuf, i);
		       } else {
			       perror_msg_and_die ("UNIX socket error");
		       }
		       RELEASE_BB_BUFFER (tmpbuf);
		}/* FD_ISSET() */
	} /* for main loop */
}

extern int syslogd_main(int argc, char **argv)
{
	int opt;
#if ! defined(__uClinux__)
	int doFork = TRUE;
#endif
	char *p;

	/* do normal option parsing */
	while ((opt = getopt(argc, argv, "m:nO:R:LC")) > 0) {
		switch (opt) {
#ifndef CONFIG_USER_SMTP_SMTPCLIENT
			case 'm':
				MarkInterval = atoi(optarg) * 60;
				break;
#endif
			case 'n':
#if ! defined(__uClinux__)
				doFork = FALSE;
#endif
				break;
			case 'O':
				logFilePath = xstrdup(optarg);
				break;
#ifdef BB_FEATURE_REMOTE_LOG
			case 'R':
				remote_log_info[0].hostname = strdup(optarg);
				if ( (p = strchr(remote_log_info[0].hostname, ':'))){
					remote_log_info[0].port = atoi(p+1);
					*p = '\0';
				}
				remote_log_info[0].enable = 1;
				remote_logging = TRUE;
				break;
			case 'L':
				local_logging = TRUE;
				break;
#endif
#ifdef BB_FEATURE_IPC_SYSLOG
			case 'C':
				circular_logging = TRUE;
				break;
#endif
			default:
				show_usage();
		}
	}

#ifdef CONFIG_USER_FLATFSD_FLATFSD
	load_config_file_settings();
#endif

#ifdef BB_FEATURE_REMOTE_LOG
	/* If they have not specified remote logging, then log locally */
	if (!remote_logging)
		local_logging = TRUE;
#endif

#ifdef DEBUG_REMOTE
	{
		int i;
		printf("remote_logging=%d\n", remote_logging);
		printf("local_logging=%d\n", local_logging);
		printf("remote_logging_initialised=%d\n", remote_logging_initialised);
		for (i = 0; i < NUM_REMOTE_HOSTS; i++) {
			printf("remote[%d]: hostname=%s port=%d filterlevel=%d enable=%d fd=%d\n",
				i,
				remote_log_info[i].hostname, 
				remote_log_info[i].port, 
				remote_log_info[i].filterlevel, 
				remote_log_info[i].enable, 
				remote_log_info[i].fd);
		}
	}
#endif

	/* Store away localhost's name before the fork */
	gethostname(LocalHostName, sizeof(LocalHostName));
	if ((p = strchr(LocalHostName, '.'))) {
		*p++ = '\0';
	}

	umask(0);

#if ! defined(__uClinux__)
	if (doFork == TRUE) {
		if (daemon(0, 1) < 0)
			perror_msg_and_die("daemon");
	}
#endif
	doSyslogd();

	return EXIT_SUCCESS;
}

/*
Local Variables
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/
