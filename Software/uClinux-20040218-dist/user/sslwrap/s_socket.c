/* apps/s_socket.c */
/* Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <syslog.h>
#define USE_SOCKETS
#define NON_MAIN
#include "apps.h"
#undef USE_SOCKETS
#undef NON_MAIN
#include "s_apps.h"
#include <openssl/ssl.h>

#ifndef NOPROTO
static struct hostent *GetHostByName(char *name);
int sock_init(void );
#else
static struct hostent *GetHostByName();
int sock_init();
#endif

#ifdef WIN16
#define SOCKET_PROTOCOL	0 /* more microsoft stupidity */
#else
#define SOCKET_PROTOCOL	IPPROTO_TCP
#endif

#ifdef WINDOWS
static struct WSAData wsa_state;
static int wsa_init_done=0;

#ifdef WIN16
static HWND topWnd=0;
static FARPROC lpTopWndProc=NULL;
static FARPROC lpTopHookProc=NULL;
extern HINSTANCE _hInstance;  /* nice global CRT provides */

static LONG FAR PASCAL topHookProc(hwnd,message,wParam,lParam)
HWND hwnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
	{
	if (hwnd == topWnd)
		{
		switch(message)
			{
		case WM_DESTROY:
		case WM_CLOSE:
			SetWindowLong(topWnd,GWL_WNDPROC,(LONG)lpTopWndProc);
			sock_cleanup();
			break;
			}
		}
	return CallWindowProc(lpTopWndProc,hwnd,message,wParam,lParam);
	}

static BOOL CALLBACK enumproc(HWND hwnd,LPARAM lParam)
	{
	topWnd=hwnd;
	return(FALSE);
	}

#endif /* WIN32 */
#endif /* WINDOWS */

void sock_cleanup()
	{
#ifdef WINDOWS
	if (wsa_init_done)
		{
		wsa_init_done=0;
		WSACancelBlockingCall();
		WSACleanup();
		}
#endif
	}

int sock_init()
	{
#ifdef WINDOWS
	if (!wsa_init_done)
		{
		int err;
	  
#ifdef SIGINT
		signal(SIGINT,(void (*)(int))sock_cleanup);
#endif
		wsa_init_done=1;
		memset(&wsa_state,0,sizeof(wsa_state));
		if (WSAStartup(0x0101,&wsa_state)!=0)
			{
			err=WSAGetLastError();
			BIO_printf(bio_err,"unable to start WINSOCK, error code=%d\n",err);
			return(0);
			}

#ifdef WIN16
		EnumTaskWindows(GetCurrentTask(),enumproc,0L);
		lpTopWndProc=(FARPROC)GetWindowLong(topWnd,GWL_WNDPROC);
		lpTopHookProc=MakeProcInstance((FARPROC)topHookProc,_hInstance);

		SetWindowLong(topWnd,GWL_WNDPROC,(LONG)lpTopHookProc);
#endif /* WIN16 */
		}
#endif /* WINDOWS */
	return(1);
	}

int init_client(sock, host, port)
int *sock;
char *host;
int port;
	{
	unsigned char ip[4];
	short p=0;

	if (!host_ip(host,&(ip[0])))
		{
		return(0);
		}
	if (p != 0) port=p;
	return(init_client_ip(sock,ip,port));
	}

int init_client_ip(sock, ip, port)
int *sock;
unsigned char ip[4];
int port;
	{
	unsigned long addr;
	struct sockaddr_in them;
	int s,i;

	if (!sock_init()) return(0);

	memset((char *)&them,0,sizeof(them));
	them.sin_family=AF_INET;
	them.sin_port=htons((unsigned short)port);
	addr=(unsigned long)
		((unsigned long)ip[0]<<24L)|
		((unsigned long)ip[1]<<16L)|
		((unsigned long)ip[2]<< 8L)|
		((unsigned long)ip[3]);
	them.sin_addr.s_addr=htonl(addr);

	s=socket(AF_INET,SOCK_STREAM,SOCKET_PROTOCOL);
	if (s == INVALID_SOCKET) { perror("socket"); return(0); }

	i=0;
	i=setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(char *)&i,sizeof(i));
	if (i < 0) { perror("keepalive"); return(0); }

	if (connect(s,(struct sockaddr *)&them,sizeof(them)) == -1)
		{ close(s); perror("connect"); return(0); }
	*sock=s;
	return(1);
	}

int nbio_sock_error(sock)
int sock;
	{
	int j,i,size;

	size=sizeof(int);
	i=getsockopt(sock,SOL_SOCKET,SO_ERROR,(char *)&j,&size);
	if (i < 0)
		return(1);
	else
		return(j);
	}

int nbio_init_client_ip(sock, ip, port)
int *sock;
unsigned char ip[4];
int port;
	{
	unsigned long addr;
	struct sockaddr_in them;
	int s,i;

	if (!sock_init()) return(0);

	memset((char *)&them,0,sizeof(them));
	them.sin_family=AF_INET;
	them.sin_port=htons((unsigned short)port);
	addr=	(unsigned long)
		((unsigned long)ip[0]<<24L)|
		((unsigned long)ip[1]<<16L)|
		((unsigned long)ip[2]<< 8L)|
		((unsigned long)ip[3]);
	them.sin_addr.s_addr=htonl(addr);

	if (*sock <= 0)
		{
		unsigned long l=1;

		s=socket(AF_INET,SOCK_STREAM,SOCKET_PROTOCOL);
		if (s == INVALID_SOCKET) { perror("socket"); return(0); }

		i=0;
		i=setsockopt(s,SOL_SOCKET,SO_KEEPALIVE,(char *)&i,sizeof(i));
		if (i < 0) { perror("keepalive"); return(0); }
		*sock=s;

#ifdef FIONBIO
		socket_ioctl(s,FIONBIO,&l);
#endif
		}
	else
		s= *sock;

	i=connect(s,(struct sockaddr *)&them,sizeof(them));
	if (i == INVALID_SOCKET)
		{
		if (BIO_sock_should_retry(i))
			return(-1);
		else
			return(0);
		}
	else
		return(1);
	}

int do_server(port, ret, cb)
int port;
int *ret;
int (*cb)();
	{
	int sock;
	char *name;
	int accept_socket;
	int i;

	if (!init_server(&accept_socket,port)) return(0);

	if (ret != NULL)
		{
		*ret=accept_socket;
		/* return(1);*/
		}
	for (;;)
		{
		if (do_accept(accept_socket,&sock,&name) == 0)
			{
			SHUTDOWN(accept_socket);
			return(0);
			}
		i=(*cb)(name,sock, sock);
		if (name != NULL) free(name);
		SHUTDOWN(sock);
		if (i < 0)
			{
			SHUTDOWN(accept_socket);
			return(i);
			}
		}
	}

int init_server(sock, port)
int *sock;
int port;
	{
	int ret=0;
	struct sockaddr_in server;
	int s= -1,i;

	if (!sock_init()) return(0);

	memset((char *)&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons((unsigned short)port);
	server.sin_addr.s_addr=INADDR_ANY;
	s=socket(AF_INET,SOCK_STREAM,SOCKET_PROTOCOL);

	if (s == INVALID_SOCKET) goto err;
	if (bind(s,(struct sockaddr *)&server,sizeof(server)) == -1)
		{
#ifndef WINDOWS
		perror("bind");
#endif
		goto err;
		}
	if (listen(s,5) == -1) goto err;
	i=0;
	*sock=s;
	ret=1;
err:
	if ((ret == 0) && (s != -1))
		{
		SHUTDOWN(s);
		}
	return(ret);
	}

int do_accept(acc_sock, sock, host)
int acc_sock;
int *sock;
char **host;
	{
	int ret,i;
	struct hostent *h1,*h2;
	static struct sockaddr_in from;
	int len;
/*	struct linger ling; */

	if (!sock_init()) return(0);

#ifndef WINDOWS
redoit:
#endif

	memset((char *)&from,0,sizeof(from));
	len=sizeof(from);
	ret=accept(acc_sock,(struct sockaddr *)&from,&len);
	if (ret == INVALID_SOCKET)
		{
#ifdef WINDOWS
		i=WSAGetLastError();
		BIO_printf(bio_err,"accept error %d\n",i);
#else
		if (errno == EINTR)
			{
			/*check_timeout(); */
			goto redoit;
			}
		fprintf(stderr,"errno=%d ",errno);
		perror("accept");
#endif
		return(0);
		}

/*
	ling.l_onoff=1;
	ling.l_linger=0;
	i=setsockopt(ret,SOL_SOCKET,SO_LINGER,(char *)&ling,sizeof(ling));
	if (i < 0) { perror("linger"); return(0); }
	i=0;
	i=setsockopt(ret,SOL_SOCKET,SO_KEEPALIVE,(char *)&i,sizeof(i));
	if (i < 0) { perror("keepalive"); return(0); }
*/

	if (host == NULL) goto end;
	/* I should use WSAAsyncGetHostByName() under windows */
	h1=gethostbyaddr((char *)&from.sin_addr.s_addr,
		sizeof(from.sin_addr.s_addr),AF_INET);
	if (h1 == NULL)
		{
		BIO_printf(bio_err,"bad gethostbyaddr\n");
		*host=NULL;
		/* return(0); */
		}
	else
		{
		if ((*host=(char *)malloc(strlen(h1->h_name)+1)) == NULL)
			{
			perror("Malloc");
			return(0);
			}
		strcpy(*host,h1->h_name);

		h2=GetHostByName(*host);
		if (h2 == NULL)
			{
			BIO_printf(bio_err,"gethostbyname failure\n");
			return(0);
			}
		i=0;
		if (h2->h_addrtype != AF_INET)
			{
			BIO_printf(bio_err,"gethostbyname addr is not AF_INET\n");
			return(0);
			}
		}
end:
	*sock=ret;
	return(1);
	}

int socket_ioctl(fd,type,arg)
int fd;
long type;
unsigned long *arg;
	{
	int i,err;
#ifdef WINDOWS
	i=ioctlsocket(fd,type,arg);
#else
	i=ioctl(fd,type,arg);
#endif
	if (i < 0)
		{
#ifdef WINDOWS
		err=WSAGetLastError();
#else
		err=errno;
#endif
		BIO_printf(bio_err,"ioctl on socket failed:error %d\n",err);
		}
	return(i);
	}

int sock_err()
	{
#ifdef WINDOWS
	return(WSAGetLastError());
#else
	return(errno);
#endif
	}

int extract_host_port(str,host_ptr,ip,port_ptr)
char *str;
char **host_ptr;
unsigned char *ip;
short *port_ptr;
	{
	char *h,*p;

	h=str;
	p=strchr(str,':');
	if (p == NULL)
		{
		BIO_printf(bio_err,"no port defined\n");
		return(0);
		}
	*(p++)='\0';

	if ((ip != NULL) && !host_ip(str,ip))
		goto err;
	if (host_ptr != NULL) *host_ptr=h;

	if (!extract_port(p,port_ptr))
		goto err;
	return(1);
err:
	return(0);
	}

int host_ip(str,ip)
char *str;
unsigned char ip[4];
	{
	unsigned int in[4]; 
	int i;

	if (sscanf(str,"%d.%d.%d.%d",&(in[0]),&(in[1]),&(in[2]),&(in[3])) == 4)
		{
		for (i=0; i<4; i++)
			if (in[i] > 255)
				{
				BIO_printf(bio_err,"invalid IP address\n");
				goto err;
				}
		ip[0]=in[0];
		ip[1]=in[1];
		ip[2]=in[2];
		ip[3]=in[3];
		}
	else
		{ /* do a gethostbyname */
		struct hostent *he;

		if (!sock_init()) return(0);

		he=GetHostByName(str);
		if (he == NULL)
			{
			BIO_printf(bio_err,"gethostbyname failure\n");
			goto err;
			}
		/* cast to short because of win16 winsock definition */
		if ((short)he->h_addrtype != AF_INET)
			{
			BIO_printf(bio_err,"gethostbyname addr is not AF_INET\n");
			return(0);
			}
		ip[0]=he->h_addr_list[0][0];
		ip[1]=he->h_addr_list[0][1];
		ip[2]=he->h_addr_list[0][2];
		ip[3]=he->h_addr_list[0][3];
		}
	return(1);
err:
	return(0);
	}

int extract_port(str,port_ptr)
char *str;
short *port_ptr;
	{
	int i;
	struct servent *s;

	i=atoi(str);
	if (i != 0)
		*port_ptr=(unsigned short)i;
	else
		{
		s=getservbyname(str,"tcp");
		if (s == NULL)
			{
			BIO_printf(bio_err,"getservbyname failure for %s\n",str);
			return(0);
			}
		*port_ptr=ntohs((unsigned short)s->s_port);
		}
	return(1);
	}

#define GHBN_NUM	4
static struct ghbn_cache_st
	{
	char name[128];
	struct hostent ent;
	unsigned long order;
	} ghbn_cache[GHBN_NUM];

static unsigned long ghbn_hits=0L;
static unsigned long ghbn_miss=0L;

static struct hostent *GetHostByName(name)
char *name;
	{
	struct hostent *ret;
	int i,lowi=0;
	unsigned long low= (unsigned long)-1;

	for (i=0; i<GHBN_NUM; i++)
		{
		if (low > ghbn_cache[i].order)
			{
			low=ghbn_cache[i].order;
			lowi=i;
			}
		if (ghbn_cache[i].order > 0)
			{
			if (strncmp(name,ghbn_cache[i].name,128) == 0)
				break;
			}
		}
	if (i == GHBN_NUM) /* no hit*/
		{
		ghbn_miss++;
		ret=gethostbyname(name);
		if (ret == NULL) return(NULL);
		/* else add to cache */
		strncpy(ghbn_cache[lowi].name,name,128);
		memcpy((char *)&(ghbn_cache[lowi].ent),ret,sizeof(struct hostent));
		ghbn_cache[lowi].order=ghbn_miss+ghbn_hits;
		return(ret);
		}
	else
		{
		ghbn_hits++;
		ret= &(ghbn_cache[i].ent);
		ghbn_cache[i].order=ghbn_miss+ghbn_hits;
		return(ret);
		}
	}

#ifndef MSDOS
int spawn(argc, argv, in, out)
int argc;
char **argv;
int *in;
int *out;
	{
	int pid;
#define CHILD_READ	p1[0]
#define CHILD_WRITE	p2[1]
#define PARENT_READ	p2[0]
#define PARENT_WRITE	p1[1]
	int p1[2],p2[2];

#if 0
	int i;
	for (i = 0; i < argc; i++) {
		syslog(LOG_INFO, "argv[%d]=[%s]", i, argv[i]);
	}
#endif
	/* REVISIT: Removing this syslog causes some wierd race condition
	 *          which stops things working!
	 */
	syslog(LOG_INFO, "argc=%d", argc);

	if ((pipe(p1) < 0) || (pipe(p2) < 0)) return(-1);

#ifdef __uClinux__
	if ((pid=vfork()) == 0)
#else
	if ((pid=fork()) == 0)
#endif
		{ /* child */
		if (dup2(CHILD_WRITE,fileno(stdout)) < 0)
			perror("dup2");
		if (dup2(CHILD_WRITE,fileno(stderr)) < 0)
			perror("dup2");
		if (dup2(CHILD_READ,fileno(stdin)) < 0)
			perror("dup2");
		close(CHILD_READ); 
		close(CHILD_WRITE);

		close(PARENT_READ);
		close(PARENT_WRITE);
		execvp(argv[0],argv);
		perror("child");
		exit(1);
		}

	/* parent */
	*in= PARENT_READ;
	*out=PARENT_WRITE;
	close(CHILD_READ);
	close(CHILD_WRITE);
	return(pid);
	}
#endif /* MSDOS */


#ifdef undef
	/* Turn on synchronous sockets so that we can do a WaitForMultipleObjects
	 * on sockets */
	{
	SOCKET s;
	int optionValue = SO_SYNCHRONOUS_NONALERT;
	int err;

	err = setsockopt( 
	    INVALID_SOCKET, 
	    SOL_SOCKET, 
	    SO_OPENTYPE, 
	    (char *)&optionValue, 
	    sizeof(optionValue));
	if (err != NO_ERROR) {
	/* failed for some reason... */
		BIO_printf(bio_err, "failed to setsockopt(SO_OPENTYPE, SO_SYNCHRONOUS_ALERT) - %d\n",
			WSAGetLastError());
		}
	}
#endif
