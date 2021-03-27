/* vi: set sw=4 ts=4: */
/*
 * telnet implementation for busybox
 *
 * Author: Tomi Ollila <too@iki.fi>
 * Copyright (C) 1994-2000 by Tomi Ollila
 *
 * Created: Thu Apr  7 13:29:41 1994 too
 * Last modified: Fri Jun  9 14:34:24 2000 too
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
 * HISTORY
 * Revision 3.1  1994/04/17  11:31:54  too
 * initial revision
 * Modified 2000/06/13 for inclusion in BusyBox by Erik Andersen <andersee@debian.org> 
 * Modified 2001/05/07 to add ability to pass TTYPE to remote host by Jim McQuillan
 * <jam@ltsp.org>
 *
 */

#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <arpa/telnet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "busybox.h"

#ifdef BB_FEATURE_AUTOWIDTH
#   include <sys/ioctl.h>
#endif

#if 0
static const int DOTRACE = 1;
#endif

#ifdef DOTRACE
#include <arpa/inet.h> /* for inet_ntoa()... */
#define TRACE(x, y) do { if (x) printf y; } while (0)
#else
#define TRACE(x, y) 
#endif

#if 0
#define USE_POLL
#include <sys/poll.h>
#else
#include <sys/time.h>
#endif

#define DATABUFSIZE  128
#define IACBUFSIZE   128

static const int CHM_TRY = 0;
static const int CHM_ON = 1;
static const int CHM_OFF = 2;

static const int UF_ECHO = 0x01;
static const int UF_SGA = 0x02;

enum {
	TS_0 = 1,
	TS_IAC = 2,
	TS_OPT = 3,
	TS_SUB1 = 4,
	TS_SUB2 = 5,
};

#define WriteCS(fd, str) write(fd, str, sizeof str -1)

typedef unsigned char byte;

/* use globals to reduce size ??? */ /* test this hypothesis later */
static struct Globalvars {
	int		netfd; /* console fd:s are 0 and 1 (and 2) */
    /* same buffer used both for network and console read/write */
	char    buf[DATABUFSIZE]; /* allocating so static size is smaller */
	byte	telstate; /* telnet negotiation state from network input */
	byte	telwish;  /* DO, DONT, WILL, WONT */
	byte    charmode;
	byte    telflags;
	byte	gotsig;
	/* buffer to handle telnet negotiations */
	char    iacbuf[IACBUFSIZE];
	short	iaclen; /* could even use byte */
	struct termios termios_def;	
	struct termios termios_raw;	
} G;

#define xUSE_GLOBALVAR_PTR /* xUSE... -> don't use :D (makes smaller code) */

#ifdef USE_GLOBALVAR_PTR
struct Globalvars * Gptr;
#define G (*Gptr)
#else
static struct Globalvars G;
#endif

static inline void iacflush()
{
	write(G.netfd, G.iacbuf, G.iaclen);
	G.iaclen = 0;
}

/* Function prototypes */
static int getport(char * p);
static struct in_addr getserver(char * p);
static int create_socket();
static void setup_sockaddr_in(struct sockaddr_in * addr, int port);
static int remote_connect(struct in_addr addr, int port);
static void rawmode();
static void cookmode();
static void do_linemode();
static void will_charmode();
static void telopt(byte c);
static int subneg(byte c);
#if 0
static int local_bind(int port);
#endif

/* Some globals */
static int one = 1;

#ifdef BB_FEATURE_TELNET_TTYPE
static char *ttype;
#endif

#ifdef BB_FEATURE_AUTOWIDTH
static int win_width, win_height;
#endif

static void doexit(int ev)
{
	cookmode();
	exit(ev);
}	

static void conescape()
{
	char b;

	if (G.gotsig)	/* came from line  mode... go raw */
		rawmode();

	WriteCS(1, "\r\nConsole escape. Commands are:\r\n\n"
			" l	go to line mode\r\n"
			" c	go to character mode\r\n"
			" z	suspend telnet\r\n"
			" e	exit telnet\r\n");

	if (read(0, &b, 1) <= 0)
		doexit(1);

	switch (b)
	{
	case 'l':
		if (!G.gotsig)
		{
			do_linemode();
			goto rrturn;
		}
		break;
	case 'c':
		if (G.gotsig)
		{
			will_charmode();
			goto rrturn;
		}
		break;
	case 'z':
		cookmode();
		kill(0, SIGTSTP);
		rawmode();
		break;
	case 'e':
		doexit(0);
	}

	WriteCS(1, "continuing...\r\n");

	if (G.gotsig)
		cookmode();
	
 rrturn:
	G.gotsig = 0;
	
}
static void handlenetoutput(int len)
{
	/*	here we could do smart tricks how to handle 0xFF:s in output
	 *	stream  like writing twice every sequence of FF:s (thus doing
	 *	many write()s. But I think interactive telnet application does
	 *	not need to be 100% 8-bit clean, so changing every 0xff:s to
	 *	0x7f:s
	 *
	 *	2002-mar-21, Przemyslaw Czerpak (druzus@polbox.com)
	 *	I don't agree.
	 *	first - I cannot use programs like sz/rz
	 *	second - the 0x0D is sent as one character and if the next
	 *	         char is 0x0A then it's eaten by a server side.
	 *	third - whay doy you have to make 'many write()s'?
	 *	        I don't understand.
	 *	So I implemented it. It's realy useful for me. I hope that
	 *	others people will find it interesting to.
	 */

	int i, j;
	byte * p = G.buf;
	byte outbuf[4*DATABUFSIZE];

	for (i = len, j = 0; i > 0; i--, p++)
	{
		if (*p == 0x1d)
		{
			conescape();
			return;
		}
		outbuf[j++] = *p;
		if (*p == 0xff)
		    outbuf[j++] = 0xff;
		else if (*p == 0x0d)
		    outbuf[j++] = 0x00;
	}
	if (j > 0 )
	    write(G.netfd, outbuf, j);
}


static void handlenetinput(int len)
{
	int i;
	int cstart = 0;

	for (i = 0; i < len; i++)
	{
		byte c = G.buf[i];

		if (G.telstate == 0) /* most of the time state == 0 */
		{
			if (c == IAC)
			{
				cstart = i;
				G.telstate = TS_IAC;
			}
		}
		else
			switch (G.telstate)
			 {
			 case TS_0:
				 if (c == IAC)
					 G.telstate = TS_IAC;
				 else
					 G.buf[cstart++] = c;
				 break;

			 case TS_IAC:
				 if (c == IAC) /* IAC IAC -> 0xFF */
				 {
					 G.buf[cstart++] = c;
					 G.telstate = TS_0;
					 break;
				 }
				 /* else */
				 switch (c)
				 {
				 case SB:
					 G.telstate = TS_SUB1;
					 break;
				 case DO:
				 case DONT:
				 case WILL:
				 case WONT:
					 G.telwish =  c;
					 G.telstate = TS_OPT;
					 break;
				 default:
					 G.telstate = TS_0;	/* DATA MARK must be added later */
				 }
				 break;
			 case TS_OPT: /* WILL, WONT, DO, DONT */
				 telopt(c);
				 G.telstate = TS_0;
				 break;
			 case TS_SUB1: /* Subnegotiation */
			 case TS_SUB2: /* Subnegotiation */
				 if (subneg(c) == TRUE)
					 G.telstate = TS_0;
				 break;
			 }
	}
	if (G.telstate)
	{
		if (G.iaclen)			iacflush();
		if (G.telstate == TS_0)	G.telstate = 0;

		len = cstart;
	}

	if (len)
		write(1, G.buf, len);
}


/* ******************************* */

static inline void putiac(int c)
{
	G.iacbuf[G.iaclen++] = c;
}


static void putiac2(byte wwdd, byte c)
{
	if (G.iaclen + 3 > IACBUFSIZE)
		iacflush();

	putiac(IAC);
	putiac(wwdd);
	putiac(c);
}

#if 0
static void putiac1(byte c)
{
	if (G.iaclen + 2 > IACBUFSIZE)
		iacflush();

	putiac(IAC);
	putiac(c);
}
#endif

#ifdef BB_FEATURE_TELNET_TTYPE
static void putiac_subopt(byte c, char *str)
{
	int	len = strlen(str) + 6;   // ( 2 + 1 + 1 + strlen + 2 )

	if (G.iaclen + len > IACBUFSIZE)
		iacflush();

	putiac(IAC);
	putiac(SB);
	putiac(c);
	putiac(0);

	while(*str)
		putiac(*str++);

	putiac(IAC);
	putiac(SE);
}
#endif

#ifdef BB_FEATURE_AUTOWIDTH
static void putiac_naws(byte c, int x, int y)
{
	if (G.iaclen + 9 > IACBUFSIZE)
		iacflush();

	putiac(IAC);
	putiac(SB);
	putiac(c);

	putiac((x >> 8) & 0xff);
	putiac(x & 0xff);
	putiac((y >> 8) & 0xff);
	putiac(y & 0xff);

	putiac(IAC);
	putiac(SE);
}
#endif

/* void putiacstring (subneg strings) */

/* ******************************* */

static char const escapecharis[] = "\r\nEscape character is ";

static void setConMode()
{
	if (G.telflags & UF_ECHO)
	{
		if (G.charmode == CHM_TRY) {
			G.charmode = CHM_ON;
			printf("\r\nEntering character mode%s'^]'.\r\n", escapecharis);
			rawmode();
		}
	}
	else
	{
		if (G.charmode != CHM_OFF) {
			G.charmode = CHM_OFF;
			printf("\r\nEntering line mode%s'^C'.\r\n", escapecharis);
			cookmode();
		}
	}
}

/* ******************************* */

static void will_charmode()
{
	G.charmode = CHM_TRY;
	G.telflags |= (UF_ECHO | UF_SGA);
	setConMode();
  
	putiac2(DO, TELOPT_ECHO);
	putiac2(DO, TELOPT_SGA);
	iacflush();
}

static void do_linemode()
{
	G.charmode = CHM_TRY;
	G.telflags &= ~(UF_ECHO | UF_SGA);
	setConMode();

	putiac2(DONT, TELOPT_ECHO);
	putiac2(DONT, TELOPT_SGA);
	iacflush();
}

/* ******************************* */

static inline void to_notsup(char c)
{
	if      (G.telwish == WILL)	putiac2(DONT, c);
	else if (G.telwish == DO)	putiac2(WONT, c);
}

static inline void to_echo(void)
{
	/* if server requests ECHO, don't agree */
	if      (G.telwish == DO) {	putiac2(WONT, TELOPT_ECHO);	return; }
	else if (G.telwish == DONT)	return;
  
	if (G.telflags & UF_ECHO)
	{
		if (G.telwish == WILL)
			return;
	}
	else
		if (G.telwish == WONT)
			return;

	if (G.charmode != CHM_OFF)
		G.telflags ^= UF_ECHO;

	if (G.telflags & UF_ECHO)
		putiac2(DO, TELOPT_ECHO);
	else
		putiac2(DONT, TELOPT_ECHO);

	setConMode();
	WriteCS(1, "\r\n");  /* sudden modec */
}

static inline void to_sga(void)
{
	/* daemon always sends will/wont, client do/dont */

	if (G.telflags & UF_SGA)
	{
		if (G.telwish == WILL)
			return;
	}
	else
		if (G.telwish == WONT)
			return;
  
	if ((G.telflags ^= UF_SGA) & UF_SGA) /* toggle */
		putiac2(DO, TELOPT_SGA);
	else
		putiac2(DONT, TELOPT_SGA);

	return;
}

#ifdef BB_FEATURE_TELNET_TTYPE
static inline void to_ttype(void)
{
	/* Tell server we will (or won't) do TTYPE */

	if(ttype)
		putiac2(WILL, TELOPT_TTYPE);
	else
		putiac2(WONT, TELOPT_TTYPE);

	return;
}
#endif

#ifdef BB_FEATURE_AUTOWIDTH
static inline void to_naws(void)
{
	/* Tell server we will do NAWS */
	putiac2(WILL, TELOPT_NAWS);
	return;
}
#endif

static void telopt(byte c)
{
	switch (c)
	{
		case TELOPT_ECHO:		to_echo();	break;
		case TELOPT_SGA:		to_sga();	break;
#ifdef BB_FEATURE_TELNET_TTYPE
		case TELOPT_TTYPE:		to_ttype();	break;
#endif
#ifdef BB_FEATURE_AUTOWIDTH
		case TELOPT_NAWS:		to_naws();
								putiac_naws(c, win_width, win_height);
								break;
#endif
		default:				to_notsup(c);
								break;
	}
}


/* ******************************* */

/* subnegotiation -- ignore all (except TTYPE,NAWS) */

static int subneg(byte c)
{
	switch (G.telstate)
	{
	case TS_SUB1:
		if (c == IAC)
			G.telstate = TS_SUB2;
#ifdef BB_FEATURE_TELNET_TTYPE
		else
		if (c == TELOPT_TTYPE)
			putiac_subopt(TELOPT_TTYPE,ttype);
#endif
		break;
	case TS_SUB2:
		if (c == SE)
			return TRUE;
		G.telstate = TS_SUB1;
		/* break; */
	}
	return FALSE;
}

/* ******************************* */

static void fgotsig(int sig)
{
	G.gotsig = sig;
}


static void rawmode()
{
	tcsetattr(0, TCSADRAIN, &G.termios_raw);
}	

static void cookmode()
{
	tcsetattr(0, TCSADRAIN, &G.termios_def);
}

extern int telnet_main(int argc, char** argv)
{
	struct in_addr host;
	int port;
	int len;
#ifdef USE_POLL
	struct pollfd ufds[2];
#else	
	fd_set readfds;
	int maxfd;
#endif	

#ifdef BB_FEATURE_AUTOWIDTH
    struct winsize winp;
    if( ioctl(0, TIOCGWINSZ, &winp) == 0 ) {
	win_width  = winp.ws_col;
	win_height = winp.ws_row;
    }
#endif

#ifdef BB_FEATURE_TELNET_TTYPE
    ttype = getenv("TERM");
#endif

	memset(&G, 0, sizeof G);

	if (tcgetattr(0, &G.termios_def) < 0)
		exit(1);
	
	G.termios_raw = G.termios_def;
	cfmakeraw(&G.termios_raw);
	
	if (argc < 2)	show_usage();
	port = (argc > 2)? getport(argv[2]): 23;
	
	host = getserver(argv[1]);

	G.netfd = remote_connect(host, port);

	signal(SIGINT, fgotsig);

#ifdef USE_POLL
	ufds[0].fd = 0; ufds[1].fd = G.netfd;
	ufds[0].events = ufds[1].events = POLLIN;
#else	
	FD_ZERO(&readfds);
	FD_SET(0, &readfds);
	FD_SET(G.netfd, &readfds);
	maxfd = G.netfd + 1;
#endif
	
	while (1)
	{
#ifndef USE_POLL
		fd_set rfds = readfds;
		
		switch (select(maxfd, &rfds, NULL, NULL, NULL))
#else
		switch (poll(ufds, 2, -1))
#endif			
		{
		case 0:
			/* timeout */
		case -1:
			/* error, ignore and/or log something, bay go to loop */
			if (G.gotsig)
				conescape();
			else
				sleep(1);
			break;
		default:

#ifdef USE_POLL
			if (ufds[0].revents) /* well, should check POLLIN, but ... */
#else				
			if (FD_ISSET(0, &rfds))
#endif				
			{
				len = read(0, G.buf, DATABUFSIZE);

				if (len <= 0)
					doexit(0);

				TRACE(0, ("Read con: %d\n", len));
				
				handlenetoutput(len);
			}

#ifdef USE_POLL
			if (ufds[1].revents) /* well, should check POLLIN, but ... */
#else				
			if (FD_ISSET(G.netfd, &rfds))
#endif				
			{
				len = read(G.netfd, G.buf, DATABUFSIZE);

				if (len <= 0)
				{
					WriteCS(1, "Connection closed by foreign host.\r\n");
					doexit(1);
				}
				TRACE(0, ("Read netfd (%d): %d\n", G.netfd, len));

				handlenetinput(len);
			}
		}
	}
}

static int getport(char * p)
{
	unsigned int port = atoi(p);

	if ((unsigned)(port - 1 ) > 65534)
	{
		error_msg_and_die("%s: bad port number", p);
	}
	return port;
}

static struct in_addr getserver(char * host)
{
	struct in_addr addr;

	struct hostent * he;
	he = xgethostbyname(host);
	memcpy(&addr, he->h_addr, sizeof addr);

	TRACE(1, ("addr: %s\n", inet_ntoa(addr)));

	return addr;
}

static int create_socket()
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

static void setup_sockaddr_in(struct sockaddr_in * addr, int port)
{
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
}
  
static int remote_connect(struct in_addr addr, int port)
{
	struct sockaddr_in s_addr;
	int s = create_socket();

	setup_sockaddr_in(&s_addr, port);
	s_addr.sin_addr = addr;

	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof one);

	if (connect(s, (struct sockaddr *)&s_addr, sizeof s_addr) < 0)
	{
		perror_msg_and_die("Unable to connect to remote host");
	}
	return s;
}

/*
Local Variables:
c-file-style: "linux"
c-basic-offset: 4
tab-width: 4
End:
*/

