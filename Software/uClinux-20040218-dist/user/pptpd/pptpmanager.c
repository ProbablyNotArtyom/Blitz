/*
 * pptpmanager.c
 *
 * Manages the PoPToP sessions.
 *
 * $Id: pptpmanager.c,v 1.11 2002/08/27 03:28:24 philipc Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __linux__
#define _GNU_SOURCE 1		/* broken arpa/inet.h */
#endif

#include "our_syslog.h"

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#if HAVE_LIBWRAP
/* re-include, just in case HAVE_SYSLOG_H wasn't defined */
#include <syslog.h>
#include <tcpd.h>

int allow_severity = LOG_WARNING;
int deny_severity = LOG_WARNING;
#endif

#ifdef __UCLIBC__
#define socklen_t int
#endif

#include "configfile.h"
#include "defaults.h"
#include "pptpctrl.h"
#include "pptpdefs.h"
#include "pptpmanager.h"
#include "compat.h"

/* command line arg variables */
extern char *pppdoptstr;
extern char *speedstr;
extern char *bindaddr;
extern int pptp_debug;

#if !defined(PPPD_IP_ALLOC)
extern char localIP[MAX_CONNECTIONS][16];
extern char remoteIP[MAX_CONNECTIONS][16];

/* for now, this contains all relavant info about a call
 * we are assuming that the remote and local IP's never change
 * and are set at startup.
 */
struct callArray {
	pid_t pid;
	char pppRemote[16];
	char pppLocal[16];
};

/* option for timeout on starting ctrl connection */
int pptp_stimeout = STIMEOUT_DEFAULT;

/* global for signal handler */
static struct callArray clientArray[MAX_CONNECTIONS];

/* from IP parser */
extern int maxConnections;
#endif

/* local function prototypes */
static void connectCall(int clientSocket, int clientNumber);
static int createHostSocket(int *hostSocket);

static void pptp_reap_child(int sig)
{
	int chld, status;

	while ((chld = waitpid(-1, &status, WNOHANG)) > 0) {
#if !defined(PPPD_IP_ALLOC)
		int i;
		for (i = 0; i < maxConnections; i++)
			if (clientArray[i].pid == chld)
				break;
		if (i < maxConnections) {
			clientArray[i].pid = 0;
			if (pptp_debug)
				syslog(LOG_DEBUG, "MGR: Reaped child %d", chld);
		} else
			syslog(LOG_INFO, "MGR: Reaped unknown child %d", chld);
#else
		if (pptp_debug)
			syslog(LOG_DEBUG, "MGR: Reaped child %d", chld);
#endif
	}
	signal(SIGCHLD, pptp_reap_child);
}

int pptp_manager(int argc, char **argv)
{
	/* misc ints */
	int loop, ctrl_pid;
	socklen_t addrsize;

	/* for host stuff */
	int hostSocket;
	fd_set connSet;
	sigset_t sigchld;
	struct sigaction sa;

	sigemptyset(&sigchld);
	sigaddset(&sigchld, SIGCHLD);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = pptp_reap_child;
	sigaction(SIGCHLD, &sa, NULL);

	/* openlog() not required, done in pptpd.c */

	syslog(LOG_INFO, "MGR: Manager process started");

#if !defined(PPPD_IP_ALLOC)
	for (loop = 0; loop < maxConnections; loop++) {
		clientArray[loop].pid = 0;
	}
#endif

	/* Connect the host socket and activate it for listening */
	if (createHostSocket(&hostSocket) < 0) {
		syslog(LOG_ERR, "MGR: Couldn't create host socket");
		syslog_perror("createHostSocket");
		exit(-1);
	}

	FD_ZERO(&connSet);

	while (1) {
#if !defined(PPPD_IP_ALLOC)
		int firstOpen = -1;

		for (loop = 0; loop < maxConnections; loop++)
			if (clientArray[loop].pid == 0) {
				firstOpen = loop;
				break;
			}

		if (firstOpen == -1) {
			syslog(LOG_ERR, "MGR: No free connection slots or IPs - no more clients can connect!");
			FD_CLR(hostSocket, &connSet);
		} else {
			FD_SET(hostSocket, &connSet);
		}
#else
		FD_SET(hostSocket, &connSet);
#endif

		/* wait for connection (or for SIGCHLD [EINTR]).  don't just loop on EINTR
		 * since hostSocket might not be in connSet (see above)
		 */
		if (select(hostSocket + 1, &connSet, NULL, NULL, NULL) < 0 && errno != EINTR) {
			syslog(LOG_ERR, "MGR: Error with manager select()!");
			syslog_perror("select");
			exit(-1);
		}

		if (FD_ISSET(hostSocket, &connSet)) {	/* A call came! */
			int clientSocket;
			struct sockaddr_in client_addr;

			/* Accept call and launch PPTPCTRL */
			addrsize = sizeof(client_addr);
			clientSocket = accept(hostSocket, (struct sockaddr *) &client_addr, &addrsize);

#if HAVE_LIBWRAP
			if (clientSocket != -1) {
				struct request_info r;
				request_init(&r, RQ_DAEMON, "pptpd", RQ_FILE, clientSocket, NULL);
				fromhost(&r);
				if (!hosts_access(&r)) {
					/* send a permission denied message? this is a tcp wrapper
					 * type deny so probably best to just drop it immediately like
					 * this, as tcp wrappers usually do.
					 */
					close(clientSocket);
					/* this would never be file descriptor 0, so use it as a error
					 * value
					 */
					clientSocket = 0;
				}
			}
#endif
			if (clientSocket == -1) {
				/* accept failed, but life goes on... */
				syslog(LOG_ERR, "MGR: accept() failed");
				syslog_perror("accept");
			} else if (clientSocket != 0) {
				fd_set rfds;
				struct timeval tv;
				struct pptp_header ph;
				int len;

				/*
				 * DOS protection: get a peek at the first packet
				 * and do some checks on it before we continue.
				 * A 10 second timeout on the first packet seems reasonable
				 * to me,  if anything looks sus,  throw it away.
				 */

				FD_ZERO(&rfds);
				FD_SET(clientSocket, &rfds);
				tv.tv_sec = pptp_stimeout;
				tv.tv_usec = 0;
				if (select(clientSocket + 1, &rfds, NULL, NULL, &tv) <= 0) {
					syslog(LOG_ERR, "MGR: dropped slow initial connection");
					close(clientSocket);
					continue;
				}

				len = recv(clientSocket, &ph, sizeof(ph), MSG_PEEK);
				if (len == 0) {
					syslog(LOG_WARNING, "MGR: dropped empty initial connection");
					close(clientSocket);
					continue;
				}
				if (len != sizeof(ph)) {
					syslog(LOG_ERR, "MGR: dropped small initial connection");
					close(clientSocket);
					continue;
				}

				ph.length = htons(ph.length);
				ph.pptp_type = ntohs(ph.pptp_type);
				ph.magic = ntohl(ph.magic);
				ph.ctrl_type = ntohs(ph.ctrl_type);

				if (ph.length <= 0 || ph.length > PPTP_MAX_CTRL_PCKT_SIZE) {
					syslog(LOG_WARNING, "MGR: initial packet length %d outside "
							"(0 - %d)",  ph.length, PPTP_MAX_CTRL_PCKT_SIZE);
					goto dos_exit;
				}

				if (ph.magic != PPTP_MAGIC_COOKIE) {
					syslog(LOG_WARNING, "MGR: initial packet bad magic");
					goto dos_exit;
				}

				if (ph.pptp_type != PPTP_CTRL_MESSAGE) {
					syslog(LOG_WARNING, "MGR: initial packet has bad type");
					goto dos_exit;
				}

				if (ph.ctrl_type != START_CTRL_CONN_RQST) {
					syslog(LOG_WARNING, "MGR: initial packet has bad ctrl type "
							"0x%x", ph.ctrl_type);
		dos_exit:
					close(clientSocket);
					continue;
				}

				/* block SIGCHLD until parent registers
				 * child in the connection list, else we
				 * have a race condition with the handler.
				 */
				if (sigprocmask(SIG_BLOCK, &sigchld, NULL) !=0) {
					syslog_perror("sigprocmask");
				}
#ifndef HAVE_FORK
				switch (ctrl_pid = vfork()) {
#else
				switch (ctrl_pid = fork()) {
#endif
				case -1:	/* error */
					syslog(LOG_ERR, "MGR: fork() failed launching " PPTP_CTRL_BIN);
					close(clientSocket);
					break;

				case 0:	/* child */
					close(hostSocket);
					if (pptp_debug)
						syslog(LOG_DEBUG, "MGR: Launching " PPTP_CTRL_BIN " to handle client");
#if !defined(PPPD_IP_ALLOC)
					connectCall(clientSocket, firstOpen);
#else
					connectCall(clientSocket, 0);
#endif
					_exit(1);
					/* NORETURN */
				default:	/* parent */
					close(clientSocket);
#if !defined(PPPD_IP_ALLOC)
					clientArray[firstOpen].pid = ctrl_pid;
#endif
					if (sigprocmask(SIG_UNBLOCK, &sigchld, NULL) !=0) {
						syslog_perror("sigprocmask");
					}
					break;
				}
			}
		}		/* FD_ISSET(hostSocket, &connSet) */
	}			/* while (1) */
}				/* pptp_manager() */

/*
 * Author: Kevin Thayer
 * 
 * This creates a socket to listen on, sets the max # of pending connections and 
 * various other options.
 * 
 * Returns the fd of the host socket.
 * 
 * The function return values are:
 * 0 for sucessful
 * -1 for bad socket creation
 * -2 for bad socket options
 * -3 for bad bind
 * -4 for bad listen
 */
static int createHostSocket(int *hostSocket)
{
	int opt = 1;
	struct sockaddr_in address;
#ifdef HAVE_GETSERVBYNAME
	struct servent *serv;
#endif

	/* create the master socket and check it worked */
	if ((*hostSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		return -1;

	/* set master socket to allow daemon to be restarted with connections active  */
	if (setsockopt(*hostSocket, SOL_SOCKET, SO_REUSEADDR,
		       (char *) &opt, sizeof(opt)) < 0)
		return -2;

	/* set up socket */
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	if(bindaddr)
		address.sin_addr.s_addr = inet_addr(bindaddr);
	else
		address.sin_addr.s_addr = INADDR_ANY;
#ifdef HAVE_GETSERVBYNAME
	if ((serv = getservbyname("pptp", "tcp")) != NULL) {
		address.sin_port = serv->s_port;
	} else
#endif
		address.sin_port = htons(PPTP_PORT);

	/* bind the socket to the pptp port */
	if (bind(*hostSocket, (struct sockaddr *) &address, sizeof(address)) < 0)
		return -3;

	/* minimal backlog to avoid DoS */
	if (listen(*hostSocket, 3) < 0)
		return -4;

	return 0;
}

/*
 * Author: Kevin Thayer
 * 
 * this routine sets up the arguments for the call handler and calls it.
 */

static void connectCall(int clientSocket, int clientNumber)
{

	char *ctrl_argv[16];	/* arguments for launching 'pptpctrl' binary */

	int pptpctrl_argc = 0;	/* count the number of arguments sent to pptpctrl */

	/* lame strings to hold passed args. */
	char ctrl_debug[2];
	char pppdoptfile_argv[2];
	char speedgiven_argv[2];
	extern char **environ;

	/*
	 * Launch the CTRL manager binary; we send it some information such as
	 * speed and option file on the command line.
	 */

	ctrl_argv[pptpctrl_argc++] = PPTP_CTRL_BIN "                                ";

	/* Pass socket as stdin */
	if (clientSocket != 0) {
		dup2(clientSocket, 0);
		close(clientSocket);
	}

	/* get argv set up */
	ctrl_debug[0] = pptp_debug ? '1' : '0';
	ctrl_debug[1] = '\0';
	ctrl_argv[pptpctrl_argc++] = ctrl_debug;

	/* optionfile = TRUE or FALSE; so the CTRL manager knows whether to load a non-standard options file */
	pppdoptfile_argv[0] = pppdoptstr ? '1' : '0';
	pppdoptfile_argv[1] = '\0';
	ctrl_argv[pptpctrl_argc++] = pppdoptfile_argv;
	if (pppdoptstr) {
		/* send the option filename so the CTRL manager can launch pppd with this alternate file */
		ctrl_argv[pptpctrl_argc++] = pppdoptstr;
	}
	/* tell the ctrl manager whether we were given a speed */
	speedgiven_argv[0] = speedstr ? '1' : '0';
	speedgiven_argv[1] = '\0';
	ctrl_argv[pptpctrl_argc++] = speedgiven_argv;
	if (speedstr) {
		/* send the CTRL manager the speed of the connection so it can fire pppd at that speed */
		ctrl_argv[pptpctrl_argc++] = speedstr;
	}
#if PPPD_IP_ALLOC
	/* no local or remote address to specify */
	ctrl_argv[pptpctrl_argc++] = "0";
	ctrl_argv[pptpctrl_argc++] = "0";
#else
	/* specify local & remote addresses for this call */
	ctrl_argv[pptpctrl_argc++] = "1";
	ctrl_argv[pptpctrl_argc++] = localIP[clientNumber];
	ctrl_argv[pptpctrl_argc++] = "1";
	ctrl_argv[pptpctrl_argc++] = remoteIP[clientNumber];
#endif
	/* terminate argv array with a NULL */
	ctrl_argv[pptpctrl_argc] = NULL;
	pptpctrl_argc++;

	/* ok, args are setup: invoke the call handler */
	execve(PPTP_CTRL_BIN, ctrl_argv, environ);
	syslog(LOG_ERR, "MGR: Failed to exec " PPTP_CTRL_BIN "!");
	//syslog_perror("execvp " PPTP_CTRL_BIN);
	_exit(1);
}
