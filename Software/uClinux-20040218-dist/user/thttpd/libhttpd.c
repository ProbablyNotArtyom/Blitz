/* libhttpd.c - HTTP protocol library
**
** Copyright (C)1995,1998 by Jef Poskanzer <jef@acme.com>. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/


#include "config.h"
#include "version.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#ifdef HAVE_DIRENT_H
# include <dirent.h>
# define NAMLEN(dirent) strlen((dirent)->d_name)
#else
# define dirent direct
# define NAMLEN(dirent) (dirent)->d_namlen
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# ifdef HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# ifdef HAVE_NDIR_H
#  include <ndir.h>
# endif
#endif

extern char* crypt( const char* key, const char* setting );

#include "libhttpd.h"
#include "mmc.h"
#include "timers.h"
#include "match.h"
#include "tdate_parse.h"

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static const char* rfc1123fmt = "%a, %d %b %Y %H:%M:%S GMT";
static const char* cernfmt = "%d/%b/%Y:%H:%M:%S %Z";


/* Forwards. */
static void child_reaper( ClientData client_data, struct timeval* nowP );
static int do_reap( void );
static void check_options( void );
static void free_httpd_server( httpd_server* hs );
static void add_response( httpd_conn* hc, char* str );
static void send_mime( httpd_conn* hc, int status, char* title, char* encodings, char* extraheads, char* type, int length, time_t mod );
static void realloc_str( char** strP, int* maxsizeP, int size );
static void send_response( httpd_conn* hc, int status, char* title, char* extrahead, char* form, char* arg );
#ifdef AUTH_FILE
static void send_authenticate( httpd_conn* hc, char* realm );
static int b64_decode( const char* str, unsigned char* space, int size );
static int auth_check( httpd_conn* hc, char* dirname  );
#endif /* AUTH_FILE */
static void send_dirredirect( httpd_conn* hc );
static int is_hexit( char c );
static int hexit( char c );
static void strdecode( char* to, char* from );
static int tilde_map( httpd_conn* hc );
static char* expand_symlinks( char* path, char** restP, int chrooted );
static char* bufgets( httpd_conn* hc );
static void figure_mime( httpd_conn* hc );
static void cgi_kill2( ClientData client_data, struct timeval* nowP );
static void cgi_kill( ClientData client_data, struct timeval* nowP );
#ifdef GENERATE_INDEXES
static off_t ls( httpd_conn* hc );
#endif /* GENERATE_INDEXES */
static char* build_env( char* fmt, char* arg );
#ifdef SERVER_NAME_LIST
static char* hostname_map( char* hostname );
#endif /* SERVER_NAME_LIST */
static char** make_envp( httpd_conn* hc );
static char** make_argp( httpd_conn* hc );
static void cgi_interpose( httpd_conn* hc, int wfd );
static void cgi_child( httpd_conn* hc );
static off_t cgi( httpd_conn* hc );
static int really_start_request( httpd_conn* hc );


static int reap_time;

static void
child_reaper( ClientData client_data, struct timeval* nowP )
    {
    int child_count;
    static int prev_child_count = 0;

    child_count = do_reap();

    /* Reschedule reaping, with adaptively changed time. */
    if ( child_count > prev_child_count * 3 / 2 )
	reap_time = max( reap_time / 2, MIN_REAP_TIME );
    else if ( child_count < prev_child_count * 2 / 3 )
	reap_time = min( reap_time * 5 / 4, MAX_REAP_TIME );
    (void) tmr_create(
	nowP, child_reaper, (ClientData) 0, reap_time * 1000L, 0 );
    }

static int
do_reap( void )
    {
    int child_count;
    pid_t pid;
    int status;

    /* Reap defunct children until there aren't any more. */
    for ( child_count = 0; ; ++child_count )
	{
#ifdef HAVE_WAITPID
	pid = waitpid( (pid_t) -1, &status, WNOHANG );
#else /* HAVE_WAITPID */
	pid = wait3( &status, WNOHANG, (struct rusage*) 0 );
#endif /* HAVE_WAITPID */
	if ( (int) pid == 0 )		/* none left */
	    break;
	if ( (int) pid < 0 )
	    {
	    if ( errno == EINTR )	/* because of ptrace */
		continue;
	    /* ECHILD shouldn't happen with the WNOHANG option, but with
	    ** some kernels it does anyway.  Ignore it.
	    */
	    if ( errno != ECHILD )
		syslog( LOG_ERR, "waitpid - %m" );
	    break;
	    }
	}
    return child_count;
    }


static void
check_options( void )
    {
#if defined(TILDE_MAP_1) && defined(TILDE_MAP_2)
    syslog( LOG_CRIT, "both TILDE_MAP_1 and TILDE_MAP_2 are defined" );
    exit( 1 );
#endif /* both */
    }


static void
free_httpd_server( httpd_server* hs )
    {
    if ( hs->cwd != (char*) 0 )
	free( (void*) hs->cwd );
    if ( hs->cgi_pattern != (char*) 0 )
	free( (void*) hs->cgi_pattern );
    free( (void*) hs );
    }


httpd_server*
httpd_initialize(
    char* hostname, u_int addr, int port, char* cgi_pattern, char* cwd,
    FILE* logfp, int chrooted )
    {
    httpd_server* hs;
    int on;
    struct sockaddr_in sa;
    char* cp;

    check_options();

    /* Set up child-process reaper. */
    reap_time = min( MIN_REAP_TIME * 4, MAX_REAP_TIME );
    (void) tmr_create(
	(struct timeval*) 0, child_reaper, (ClientData) 0, reap_time * 1000L,
	0 );

    hs = NEW( httpd_server, 1 );
    if ( hs == (httpd_server*) 0 )
	{
	syslog( LOG_CRIT, "out of memory" );
	return (httpd_server*) 0;
	}
    if ( hostname == (char*) 0 )
	hs->hostname = (char*) 0;
    else
	hs->hostname = strdup( hostname );
    hs->port = port;
    if ( cgi_pattern == (char*) 0 )
	hs->cgi_pattern = (char*) 0;
    else
	{
	/* Nuke any leading slashes. */
	if ( cgi_pattern[0] == '/' )
	    ++cgi_pattern;
	hs->cgi_pattern = strdup( cgi_pattern );
	if ( hs->cgi_pattern == (char*) 0 )
	    {
	    syslog( LOG_CRIT, "out of memory" );
	    return (httpd_server*) 0;
	    }
	/* Nuke any leading slashes in the cgi pattern. */
	while ( ( cp = strstr( hs->cgi_pattern, "|/" ) ) != (char*) 0 )
	    (void) strcpy( cp + 1, cp + 2 );
	}
    hs->cwd = strdup( cwd );
    if ( hs->cwd == (char*) 0 )
	{
	syslog( LOG_CRIT, "out of memory" );
	return (httpd_server*) 0;
	}
    hs->logfp = logfp;
    hs->chrooted = chrooted;

    /* Create socket. */
    hs->listen_fd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( hs->listen_fd < 0 )
	{
	syslog( LOG_CRIT, "socket - %m" );
	free_httpd_server( hs );
	return (httpd_server*) 0;
	}
    (void) fcntl( hs->listen_fd, F_SETFD, 1 );

    /* Allow reuse of local addresses. */
    on = 1;
    if ( setsockopt(
	     hs->listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on,
	     sizeof(on) ) < 0 )
        syslog( LOG_CRIT, "setsockopt SO_REUSEADDR - %m" );

    /* Bind to it. */
    memset( (char*) &sa, 0, sizeof(sa) );
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = addr;
    sa.sin_port = htons( hs->port );
    hs->host_addr = sa.sin_addr;
    if ( bind( hs->listen_fd, (struct sockaddr*) &sa, sizeof(sa) ) < 0 )
	{
	syslog( LOG_CRIT, "bind %.80s - %m", inet_ntoa( sa.sin_addr ) );
	(void) close( hs->listen_fd );
	free_httpd_server( hs );
	return (httpd_server*) 0;
	}

    /* Set the listen file descriptor to no-delay mode. */
    if ( fcntl( hs->listen_fd, F_SETFL, O_NDELAY ) < 0 )
	{
	syslog( LOG_CRIT, "fcntl O_NDELAY - %m" );
	(void) close( hs->listen_fd );
	free_httpd_server( hs );
	return (httpd_server*) 0;
	}

    /* Start a listen going. */
    if ( listen( hs->listen_fd, LISTEN_BACKLOG ) < 0 )
	{
	syslog( LOG_CRIT, "listen - %m" );
	(void) close( hs->listen_fd );
	free_httpd_server( hs );
	return (httpd_server*) 0;
	}

    /* Done initializing. */
    if ( hs->hostname == (char*) 0 )
	syslog( LOG_INFO, "%s starting on port %d", SERVER_SOFTWARE, hs->port );
    else
	syslog(
	    LOG_INFO, "%s starting on %.80s, port %d", SERVER_SOFTWARE,
	    inet_ntoa( hs->host_addr ), hs->port );
    return hs;
    }


void
httpd_terminate( httpd_server* hs )
    {
    (void) close( hs->listen_fd );
    if ( hs->logfp != (FILE*) 0 )
	(void) fclose( hs->logfp );
    free_httpd_server( hs );
    }


static char* ok200title = "OK";
static char* ok206title = "Partial Content";

static char* err302title = "Found";
static char* err302form = "The actual URL is '%.80s'.\n";

static char* err304title = "Not Modified";

char* httpd_err400title = "Bad Request";
char* httpd_err400form =
    "Your request has bad syntax or is inherently impossible to satisfy.\n";

#ifdef AUTH_FILE
static char* err401title = "Unauthorized";
static char* err401form =
    "Authorization required for the URL '%.80s'.\n";
#endif /* AUTH_FILE */

static char* err403title = "Forbidden";
static char* err403form =
    "You do not have permission to get URL '%.80s' from this server.\n";

static char* err404title = "Not Found";
static char* err404form =
    "The requested URL '%.80s' was not found on this server.\n";

char* httpd_err408title = "Request Timeout";
char* httpd_err408form =
    "No request appeared within a reasonable time period.\n";

static char* err500title = "Internal Error";
static char* err500form =
    "There was an unusual problem serving the requested URL '%.80s'.\n";

static char* err501title = "Not Implemented";
static char* err501form =
    "The requested method '%.80s' is not implemented by this server.\n";

char* httpd_err503title = "Service Temporarily Overloaded";
char* httpd_err503form =
    "The requested URL '%.80s' is temporarily overloaded.  Please try again later.\n";


/* Append a string to the buffer waiting to be sent as response. */
static void
add_response( httpd_conn* hc, char* str )
    {
    int len;

    len = strlen( str );
    realloc_str( &hc->response, &hc->maxresponse, hc->responselen + len );
    (void) memcpy( &(hc->response[hc->responselen]), str, len );
    hc->responselen += len;
    }

/* Send the buffered response. */
void
httpd_write_response( httpd_conn* hc )
    {
    if ( hc->responselen > 0 )
	{
	(void) write( hc->conn_fd, hc->response, hc->responselen );
	hc->responselen = 0;
	}
    }


static void
send_mime( httpd_conn* hc, int status, char* title, char* encodings, char* extraheads, char* type, int length, time_t mod )
    {
    time_t now;
    char nowbuf[100];
    char modbuf[100];
    char buf[1000];
    int partial_content;

    hc->status = status;
    hc->bytes = length;
    if ( hc->mime_flag )
	{
	if ( status == 200 && hc->got_range &&
	     ( hc->end_byte_loc >= hc->init_byte_loc ) &&
	     ( ( hc->end_byte_loc != length - 1 ) ||
	       ( hc->init_byte_loc != 0 ) ) &&
	     ( hc->range_if == (time_t) -1 ||
	       hc->range_if == hc->sb.st_mtime ) )
	    {
	    partial_content = 1;
	    hc->status = status = 206;
	    title = ok206title;
	    }
	else
	    partial_content = 0;

	now = time( (time_t*) 0 );
	if ( mod == (time_t) 0 )
	    mod = now;
#ifdef EMBED
	modbuf[0] = nowbuf[0] = 0;
#else
	(void) strftime( nowbuf, sizeof(nowbuf), rfc1123fmt, gmtime( &now ) );
	(void) strftime( modbuf, sizeof(modbuf), rfc1123fmt, gmtime( &mod ) );
#endif
	(void) sprintf( buf,
	    "%.20s %d %s\r\nServer: %s\r\nContent-type: %s\r\nDate: %s\r\nLast-modified: %s\r\nAccept-Ranges: bytes\r\nConnection: close\r\n",
	    hc->protocol, status, title, SERVER_SOFTWARE, type, nowbuf,
	    modbuf );
	add_response( hc, buf );
	if ( encodings[0] != '\0' )
	    {
	    (void) sprintf( buf, "Content-encoding: %s\r\n", encodings );
	    add_response( hc, buf );
	    }
	if ( partial_content )
	    {
	    (void) sprintf(
		buf, "Content-range: bytes %ld-%ld/%d\r\nContent-length: %ld\r\n",
		(long) hc->init_byte_loc, (long) hc->end_byte_loc, length,
		(long) ( hc->end_byte_loc - hc->init_byte_loc + 1 ) );
	    add_response( hc, buf );
	    }
	else if ( length >= 0 )
	    {
	    (void) sprintf( buf, "Content-length: %d\r\n", length );
	    add_response( hc, buf );
	    }
	if ( extraheads[0] != '\0' )
	    add_response( hc, extraheads );
	add_response( hc, "\r\n" );
	}
    }


static void
realloc_str( char** strP, int* maxsizeP, int size )
    {
    if ( *maxsizeP == 0 )
	{
	*maxsizeP = MAX( 200, size );	/* arbitrary */
	*strP = NEW( char, *maxsizeP + 1 );
	}
    else if ( size > *maxsizeP )
	{
	*maxsizeP = MAX( *maxsizeP * 2, size * 5 / 4 );
	*strP = RENEW( *strP, char, *maxsizeP + 1 );
	}
    else
	return;
    if ( *strP == (char*) 0 )
	{
	syslog( LOG_ERR, "out of memory" );
	exit( 1 );
	}
    }


static void
send_response( httpd_conn* hc, int status, char* title, char* extrahead, char* form, char* arg )
    {
    char buf[1000];

    send_mime( hc, status, title, "", extrahead, "text/html", -1, 0 );
    (void) sprintf(
	buf, "<HTML><HEAD><TITLE>%d %s</TITLE></HEAD>\n<BODY><H2>%d %s</H2>\n",
	status, title, status, title );
    add_response( hc, buf );
    (void) sprintf( buf, form, arg );
    add_response( hc, buf );
    (void) sprintf( buf,
	"<HR>\n<ADDRESS><A HREF=\"%s\">%s</A></ADDRESS>\n</BODY></HTML>\n",
	SERVER_ADDRESS, SERVER_SOFTWARE );
    add_response( hc, buf );
    }


void
httpd_send_err( httpd_conn* hc, int status, char* title, char* form, char* arg )
    {
    send_response( hc, status, title, "", form, arg );
    }


#ifdef AUTH_FILE

static void
send_authenticate( httpd_conn* hc, char* realm )
    {
    static char* header;
    static int maxheader = 0;
    static char headstr[] = "WWW-Authenticate: Basic realm=\"";

    realloc_str( &header, &maxheader, sizeof(headstr) + strlen( realm ) + 1 );
    (void) sprintf( header, "%s%s\"", headstr, realm );
    send_response( hc, 401, err401title, header, err401form, hc->encodedurl );
    /* If the request was a POST then there might still be data to be read,
    ** so we need to do a lingering close.
    */
    if ( hc->method == METHOD_POST )
	hc->should_linger = 1;
    }

/* Base-64 decoding.  This represents binary data as printable ASCII
** characters.  Three 8-bit binary bytes are turned into four 6-bit
** values, like so:
**
**   [11111111]  [22222222]  [33333333]
**
**   [111111] [112222] [222233] [333333]
**
** Then the 6-bit values are represented using the characters "A-Za-z0-9+/".
*/

static int b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

/* Do base-64 decoding on a string.  Ignore any non-base64 bytes.
** Return the actual number of bytes generated.  The decoded size will
** be at most 3/4 the size of the encoded, and may be smaller if there
** are padding characters (blanks, newlines).
*/
static int
b64_decode( const char* str, unsigned char* space, int size )
    {
    const char* cp;
    int space_idx, phase;
    int d, prev_d;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
	{
	d = b64_decode_table[*cp];
	if ( d != -1 )
	    {
	    switch ( phase )
		{
		case 0:
		++phase;
		break;
		case 1:
		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 2:
		c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
		if ( space_idx < size )
		    space[space_idx++] = c;
		++phase;
		break;
		case 3:
		c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
		if ( space_idx < size )
		    space[space_idx++] = c;
		phase = 0;
		break;
		}
	    prev_d = d;
	    }
	}
    return space_idx;
    }

static int
auth_check( httpd_conn* hc, char* dirname  )
    {
    static char* authpath;
    static int maxauthpath = 0;
    struct stat sb;
    char authinfo[500];
    char* authpass;
    int l;
    FILE* fp;
    char line[500];
    char* cryp;
    static char* prevauthpath;
    static int maxprevauthpath = 0;
    static time_t prevmtime;
    static char* prevuser;
    static int maxprevuser = 0;
    static char* prevcryp;
    static int maxprevcryp = 0;

    /* Construct auth filename. */
    realloc_str(
	&authpath, &maxauthpath, strlen( dirname ) + 1 + sizeof(AUTH_FILE) );
    (void) sprintf( authpath, "%s/%s", dirname, AUTH_FILE );

    /* Does this directory have an auth file? */
    if ( stat( authpath, &sb ) < 0 )
	/* Nope, let the request go through. */
	return 0;

    /* Does this request contain authorization info? */
    if ( hc->authorization[0] == '\0' )
	{
	/* Nope, return a 401 Unauthorized. */
	send_authenticate( hc, dirname );
	return -1;
	}

    /* Basic authorization info? */
    if ( strncmp( hc->authorization, "Basic ", 6 ) != 0 )
	{
	send_authenticate( hc, dirname );
	return -1;
	}

    /* Decode it. */
    l = b64_decode( &(hc->authorization[6]), authinfo, sizeof(authinfo) );
    authinfo[l] = '\0';
    /* Split into user and password. */
    authpass = strchr( authinfo, ':' );
    if ( authpass == (char*) 0 )
	{
	/* No colon?  Bogus auth info. */
	send_authenticate( hc, dirname );
	return -1;
	}
    *authpass++ = '\0';

    /* See if we have a cached entry and can use it. */
    if ( maxprevauthpath != 0 &&
	 strcmp( authpath, prevauthpath ) == 0 &&
	 sb.st_mtime == prevmtime &&
	 strcmp( authinfo, prevuser ) == 0 )
	{
	/* Yes.  Check against the cached encrypted password. */
	if ( strcmp( crypt( authpass, prevcryp ), prevcryp ) == 0 )
	    {
	    /* Ok! */
	    realloc_str( &hc->remoteuser, &hc->maxremoteuser, strlen( line ) );
	    (void) strcpy( hc->remoteuser, line );
	    return 0;
	    }
	else
	    {
	    /* No. */
	    send_authenticate( hc, dirname );
	    return -1;
	    }
	}

    /* Open the password file. */
    fp = fopen( authpath, "r" );
    if ( fp == (FILE*) 0 )
	{
	/* The file exists but we can't open it?  Disallow access. */
	syslog( LOG_ERR, "fopen auth file %.80s - %m", authpath );
	httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	return -1;
	}

    /* Read it. */
    while ( fgets( line, sizeof(line), fp ) != (char*) 0 )
	{
	/* Nuke newline. */
	l = strlen( line );
	if ( line[l - 1] == '\n' )
	    line[l - 1] = '\0';
	/* Split into user and encrypted password. */
	cryp = strchr( line, ':' );
	if ( cryp == (char*) 0 )
	    continue;
	*cryp++ = '\0';
	/* Is this the right user? */
	if ( strcmp( line, authinfo ) == 0 )
	    {
	    /* Yes. */
	    (void) fclose( fp );
	    /* So is the password right? */
	    if ( strcmp( crypt( authpass, cryp ), cryp ) == 0 )
		{
		/* Ok! */
		realloc_str( &hc->remoteuser, &hc->maxremoteuser, strlen( line ) );
		(void) strcpy( hc->remoteuser, line );
		/* And cache this user's info for next time. */
		realloc_str(
		    &prevauthpath, &maxprevauthpath, strlen( authpath ) );
		(void) strcpy( prevauthpath, authpath );
		prevmtime = sb.st_mtime;
		realloc_str( &prevuser, &maxprevuser, strlen( authinfo ) );
		(void) strcpy( prevuser, authinfo );
		realloc_str( &prevcryp, &maxprevcryp, strlen( cryp ) );
		(void) strcpy( prevcryp, cryp );
		return 0;
		}
	    else
		{
		/* No. */
		send_authenticate( hc, dirname );
		return -1;
		}
	    }
	}

    /* Didn't find that user.  Access denied. */
    (void) fclose( fp );
    send_authenticate( hc, dirname );
    return -1;
    }
#endif /* AUTH_FILE */


static void
send_dirredirect( httpd_conn* hc )
    {
    static char* location;
    static char* header;
    static int maxlocation = 0, maxheader = 0;
    static char headstr[] = "Location: ";

    realloc_str( &location, &maxlocation, strlen( hc->encodedurl ) + 1 );
    (void) sprintf( location, "%s/", hc->encodedurl );
    realloc_str( &header, &maxheader, sizeof(headstr) + strlen( location ) );
    (void) sprintf( header, "%s%s", headstr, location );
    send_response( hc, 302, err302title, header, err302form, location );
    }


char*
httpd_method_str( int method )
    {
    switch ( method )
	{
	case METHOD_GET: return "GET";
	case METHOD_HEAD: return "HEAD";
	case METHOD_POST: return "POST";
	default: return (char*) 0;
	}
    }


static int
is_hexit( char c )
    {
    if ( strchr( "0123456789abcdefABCDEF", c ) != (char*) 0 )
	return 1;
    return 0;
    }


static int
hexit( char c )
    {
    if ( c >= '0' && c <= '9' )
	return c - '0';
    if ( c >= 'a' && c <= 'f' )
	return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
	return c - 'A' + 10;
    return 0;		/* shouldn't happen, we're guarded by is_hexit() */
    }


/* Copies and decodes a string.  It's ok for from and to to be the
** same string.
*/
static void
strdecode( char* to, char* from )
    {
    for ( ; *from != '\0'; ++to, ++from )
	{
	if ( from[0] == '%' && is_hexit( from[1] ) && is_hexit( from[2] ) )
	    {
	    *to = hexit( from[1] ) * 16 + hexit( from[2] );
	    from += 2;
	    }
	else
	    *to = *from;
	}
    *to = '\0';
    }


int
httpd_get_nfiles( void )
    {
#ifdef EMBED
    return(8);
#else
    static int inited = 0;
    static int n;

    if ( ! inited )
	{
#ifdef RLIMIT_NOFILE
	struct rlimit rl;
	if ( getrlimit( RLIMIT_NOFILE, &rl ) < 0 )
	    {
	    syslog( LOG_ERR, "getrlimit - %m" );
	    exit( 1 );
	    }
	if ( rl.rlim_max == RLIM_INFINITY )
	    rl.rlim_cur = 4096;		/* arbitrary */
	else
	    rl.rlim_cur = rl.rlim_max;
	if ( setrlimit( RLIMIT_NOFILE, &rl ) < 0 )
	    {
	    syslog( LOG_ERR, "setrlimit - %m" );
	    exit( 1 );
	    }
	n = rl.rlim_cur;
#else /* RLIMIT_NOFILE */
	n = getdtablesize();
#endif /* RLIMIT_NOFILE */
	inited = 1;
	}
    return n;
#endif
    }


/* Map a ~username/whatever URL into something else.  Two different ways. */
static int
tilde_map( httpd_conn* hc )
    {
#if defined(TILDE_MAP_1) || defined(TILDE_MAP_2)
    static char* temp;
    static int maxtemp = 0;
#endif
#ifdef TILDE_MAP_1
    /* Map ~username to <prefix>/username. */
    int len;
    static char* prefix = TILDE_MAP_1;

    len = strlen( hc->expnfilename ) - 1;
    realloc_str( &temp, &maxtemp, len );
    (void) strcpy( temp, &hc->expnfilename[1] );
    realloc_str(
	&hc->expnfilename, &hc->maxexpnfilename, strlen( prefix ) + 1 + len );
    (void) strcpy( hc->expnfilename, prefix );
    if ( prefix[0] != '\0' )
	(void) strcat( hc->expnfilename, "/" );
    (void) strcat( hc->expnfilename, temp );
#endif /* TILDE_MAP_1 */

#ifdef TILDE_MAP_2
    /* Map ~username to <user's homedir>/<postfix>. */
    static char* postfix = TILDE_MAP_2;
    char* cp;
    struct passwd* pw;

    /* Get the username. */
    realloc_str( &temp, &maxtemp, strlen( hc->expnfilename ) - 1 );
    (void) strcpy( temp, &hc->expnfilename[1] );
    cp = strchr( temp, '/' );
    if ( cp != (char*) 0 )
	*cp++ = '\0';
    else
	cp = "";
    
    /* Get the passwd entry. */
    pw = getpwnam( temp );
    if ( pw == (struct passwd*) 0 )
	return 0;
    
    /* Set up altdir. */
    realloc_str(
	&hc->altdir, &hc->maxaltdir,
	strlen( pw->pw_dir ) + 1 + strlen( postfix ) );
    (void) strcpy( hc->altdir, pw->pw_dir );
    if ( postfix[0] != '\0' )
	{
	(void) strcat( hc->altdir, "/" );
	(void) strcat( hc->altdir, postfix );
	}

    /* And the filename becomes altdir plus the post-~ part of the original. */
    realloc_str(
	&hc->expnfilename, &hc->maxexpnfilename,
	strlen( hc->altdir ) + 1 + strlen( cp ) );
    (void) sprintf( hc->expnfilename, "%s/%s", hc->altdir, cp );
#endif /* TILDE_MAP_2 */
    return 1;
    }


/* Expands all symlinks in the given filename, eliding ..'s and leading /'s.
** Returns the expanded path (pointer to static string), or (char*) 0 on
** errors.  Also returns, in the string pointed to by restP, any trailing
** parts of the path that don't exist.
**
** This is a fairly nice little routine.  It handles any size filenames
** without excessive mallocs.
*/
static char*
expand_symlinks( char* path, char** restP, int chrooted )
    {
    static char* checked;
    static char* rest;
#ifdef EMBED
    char link[2048];
#else
    char link[5000];
#endif
    static int maxchecked = 0, maxrest = 0;
    int checkedlen, restlen, linklen, prevcheckedlen, prevrestlen, nlinks, i;
    char* r;
    char* cp1;
    char* cp2;

    if ( chrooted )
	{
	/* If we are chrooted, we can actually skip the symlink-expansion,
	** since it's impossible to get out of the tree.  However, we still
	** need to do the pathinfo check, and the existing symlink expansion
	** code is a pretty reasonable way to do this.  So, what we do is
	** a single stat() of the whole filename - if it exists, then we
	** return it as is with nothing in restP.  If it doesn't exist, we
	** fall through to the existing code.
	**
	** One side-effect of this is that users can't symlink to central
	** approved CGIs any more.  The workaround is to use the central
	** URL for the CGI instead of a local symlinked one.
	*/
	struct stat sb;
	if ( stat( path, &sb ) != -1 )
	    {
	    realloc_str( &checked, &maxchecked, strlen( path ) );
	    (void) strcpy( checked, path );
	    realloc_str( &rest, &maxrest, 0 );
	    rest[0] = '\0';
	    *restP = rest;
	    return checked;
	    }
	}

    /* Start out with nothing in checked and the whole filename in rest. */
    realloc_str( &checked, &maxchecked, 1 );
    checked[0] = '\0';
    checkedlen = 0;
    restlen = strlen( path );
    realloc_str( &rest, &maxrest, restlen );
    (void) strcpy( rest, path );
    if ( rest[restlen - 1] == '/' )
	rest[--restlen] = '\0';		/* trim trailing slash */
    /* Remove any leading slashes. */
    while ( rest[0] == '/' )
	{
	(void) strcpy( rest, &(rest[1]) );
	--restlen;
	}
    r = rest;
    nlinks = 0;

    /* While there are still components to check... */
    while ( restlen > 0 )
	{
	/* Save current checkedlen in case we get a symlink.  Save current
	** restlen in case we get a non-existant component.
	*/
	prevcheckedlen = checkedlen;
	prevrestlen = restlen;

	/* Grab one component from r and transfer it to checked. */
	cp1 = strchr( r, '/' );
	if ( cp1 != (char*) 0 )
	    {
	    i = cp1 - r;
	    if ( i == 0 )
		{
		/* Special case for absolute paths. */
		realloc_str( &checked, &maxchecked, checkedlen + 1 );
		(void) strncpy( &checked[checkedlen], r, 1 );
		checkedlen += 1;
		}
	    else if ( strncmp( r, "..", MAX( i, 2 ) ) == 0 )
		{
		/* Ignore ..'s that go above the start of the path. */
		if ( checkedlen != 0 )
		    {
		    cp2 = strrchr( checked, '/' );
		    if ( cp2 == (char*) 0 )
			checkedlen = 0;
		    else if ( cp2 == checked )
			checkedlen = 1;
		    else
			checkedlen = cp2 - checked;
		    }
		}
	    else
		{
		realloc_str( &checked, &maxchecked, checkedlen + 1 + i );
		if ( checkedlen > 0 && checked[checkedlen-1] != '/' )
		    checked[checkedlen++] = '/';
		(void) strncpy( &checked[checkedlen], r, i );
		checkedlen += i;
		}
	    checked[checkedlen] = '\0';
	    r += i + 1;
	    restlen -= i + 1;
	    }
	else
	    {
	    /* No slashes remaining, r is all one component. */
	    if ( strcmp( r, ".." ) == 0 )
		{
		/* Ignore ..'s that go above the start of the path. */
		if ( checkedlen != 0 )
		    {
		    cp2 = strrchr( checked, '/' );
		    if ( cp2 == (char*) 0 )
			checkedlen = 0;
		    else if ( cp2 == checked )
			checkedlen = 1;
		    else
			checkedlen = cp2 - checked;
		    checked[checkedlen] = '\0';
		    }
		}
	    else
		{
		realloc_str( &checked, &maxchecked, checkedlen + 1 + restlen );
		if ( checkedlen > 0 && checked[checkedlen-1] != '/' )
		    checked[checkedlen++] = '/';
		(void) strcpy( &checked[checkedlen], r );
		checkedlen += restlen;
		}
	    r += restlen;
	    restlen = 0;
	    }

	/* Try reading the current filename as a symlink */
	linklen = readlink( checked, link, sizeof(link) );
	if ( linklen == -1 )
	    {
	    if ( errno == EINVAL )
		continue;		/* not a symlink */
	    if ( errno == EACCES || errno == ENOENT || errno == ENOTDIR || errno == EBADF )
		{
		/* That last component was bogus.  Restore and return. */
		*restP = r - ( prevrestlen - restlen );
		if ( prevcheckedlen == 0 )
		    (void) strcpy( checked, "." );
		else
		    checked[prevcheckedlen] = '\0';
		return checked;
		}
	    syslog( LOG_ERR, "readlink %s - (%d) %m", checked, errno );
	    return (char*) 0;
	    }
	++nlinks;
	if ( nlinks > MAX_LINKS )
	    {
	    syslog( LOG_ERR, "too many symlinks in %.80s", path );
	    return (char*) 0;
	    }
	link[linklen] = '\0';
	if ( link[linklen - 1] == '/' )
	    link[--linklen] = '\0';	/* trim trailing slash */

	/* Insert the link contents in front of the rest of the filename. */
	if ( restlen != 0 )
	    {
	    (void) strcpy( rest, r );
	    realloc_str( &rest, &maxrest, restlen + linklen + 1 );
	    for ( i = restlen; i >= 0; --i )
		rest[i + linklen + 1] = rest[i];
	    (void) strcpy( rest, link );
	    rest[linklen] = '/';
	    restlen += linklen + 1;
	    r = rest;
	    }
	else
	    {
	    /* There's nothing left in the filename, so the link contents
	    ** becomes the rest.
	    */
	    realloc_str( &rest, &maxrest, linklen );
	    (void) strcpy( rest, link );
	    restlen = linklen;
	    r = rest;
	    }

	/* Re-check this component. */
	checkedlen = prevcheckedlen;
	checked[checkedlen] = '\0';
	}

    /* Ok. */
    *restP = r;
    if ( checked[0] == '\0' )
	(void) strcpy( checked, "." );
    return checked;
    }


int
httpd_get_conn( httpd_server* hs, httpd_conn* hc )
    {
    struct sockaddr_in sin;
    int sz;

    if ( ! hc->initialized )
	{
	hc->maxdecodedurl =
	    hc->maxorigfilename = hc->maxexpnfilename = hc->maxencodings =
	    hc->maxpathinfo = hc->maxquery = hc->maxaltdir =
	    hc->maxaccept = hc->maxaccepte = hc->maxreqhost =
	    hc->maxremoteuser = hc->maxresponse = 0;
	realloc_str( &hc->decodedurl, &hc->maxdecodedurl, 1 );
	realloc_str( &hc->origfilename, &hc->maxorigfilename, 1 );
	realloc_str( &hc->expnfilename, &hc->maxexpnfilename, 0 );
	realloc_str( &hc->encodings, &hc->maxencodings, 0 );
	realloc_str( &hc->pathinfo, &hc->maxpathinfo, 0 );
	realloc_str( &hc->query, &hc->maxquery, 0 );
	realloc_str( &hc->altdir, &hc->maxaltdir, 0 );
	realloc_str( &hc->accept, &hc->maxaccept, 0 );
	realloc_str( &hc->accepte, &hc->maxaccepte, 0 );
	realloc_str( &hc->reqhost, &hc->maxreqhost, 0 );
	realloc_str( &hc->remoteuser, &hc->maxremoteuser, 0 );
	realloc_str( &hc->response, &hc->maxresponse, 0 );
	hc->initialized = 1;
	}

    /* Accept the new connection. */
    sz = sizeof(sin);
    hc->conn_fd = accept( hs->listen_fd, (struct sockaddr*) &sin, &sz );
    if ( hc->conn_fd < 0 )
	{
	if ( errno == EWOULDBLOCK )
	    return GC_NO_MORE;
	syslog( LOG_ERR, "accept - %m" );
	return GC_FAIL;
	}
    (void) fcntl( hc->conn_fd, F_SETFD, 1 );
    hc->hs = hs;
    hc->client_addr = sin.sin_addr;
    hc->read_idx = 0;
    hc->checked_idx = 0;
    hc->checked_state = CHST_FIRSTWORD;
    hc->protocol = "HTTP/1.0";
    hc->mime_flag = 1;
    hc->should_linger = 0;
    hc->file_address = (char*) 0;
    return GC_OK;
    }


/* Checks hc->read_buf to see whether a complete request has been read so far;
** either the first line has two words (an HTTP/0.9 request), or the first
** line has three words and there's a blank line present.
**
** hc->read_idx is how much has been read in; hc->checked_idx is how much we
** have checked so far; and hc->checked_state is the current state of the
** finite state machine.
*/
int
httpd_got_request( httpd_conn* hc )
    {
    char c;

    for ( ; hc->checked_idx < hc->read_idx; ++hc->checked_idx )
	{
	c = hc->read_buf[hc->checked_idx];
	switch ( hc->checked_state )
	    {
	    case CHST_FIRSTWORD:
	    switch ( c )
		{
		case ' ': case '\t':
		hc->checked_state = CHST_FIRSTWS;
		break;
		case '\n': case '\r':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		}
	    break;
	    case CHST_FIRSTWS:
	    switch ( c )
		{
		case ' ': case '\t':
		break;
		case '\n': case '\r':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		default:
		hc->checked_state = CHST_SECONDWORD;
		break;
		}
	    break;
	    case CHST_SECONDWORD:
	    switch ( c )
		{
		case ' ': case '\t':
		hc->checked_state = CHST_SECONDWS;
		break;
		case '\n': case '\r':
		/* The first line has only two words - an HTTP/0.9 request. */
		return GR_GOT_REQUEST;
		}
	    break;
	    case CHST_SECONDWS:
	    switch ( c )
		{
		case ' ': case '\t':
		break;
		case '\n': case '\r':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		default:
		hc->checked_state = CHST_THIRDWORD;
		break;
		}
	    break;
	    case CHST_THIRDWORD:
	    switch ( c )
		{
		case ' ': case '\t':
		hc->checked_state = CHST_BOGUS;
		return GR_BAD_REQUEST;
		case '\n':
		hc->checked_state = CHST_LF;
		break;
		case '\r':
		hc->checked_state = CHST_CR;
		break;
		}
	    break;
	    case CHST_LINE:
	    switch ( c )
		{
		case '\n':
		hc->checked_state = CHST_LF;
		break;
		case '\r':
		hc->checked_state = CHST_CR;
		break;
		}
	    break;
	    case CHST_LF:
	    switch ( c )
		{
		case '\n':
		/* Two newlines in a row - a blank line - end of request. */
		return GR_GOT_REQUEST;
		case '\r':
		hc->checked_state = CHST_CR;
		break;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_CR:
	    switch ( c )
		{
		case '\n':
		hc->checked_state = CHST_CRLF;
		break;
		case '\r':
		/* Two returns in a row - end of request. */
		return GR_GOT_REQUEST;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_CRLF:
	    switch ( c )
		{
		case '\n':
		/* Two newlines in a row - end of request. */
		return GR_GOT_REQUEST;
		case '\r':
		hc->checked_state = CHST_CRLFCR;
		break;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_CRLFCR:
	    switch ( c )
		{
		case '\n': case '\r':
		/* Two CRLFs or two CRs in a row - end of request. */
		return GR_GOT_REQUEST;
		default:
		hc->checked_state = CHST_LINE;
		break;
		}
	    break;
	    case CHST_BOGUS:
	    return GR_BAD_REQUEST;
	    }
	}
    return GR_NO_REQUEST;
    }


int
httpd_parse_request( httpd_conn* hc )
    {
    char* buf;
    char* method_str;
    char* url;
    char* protocol;
    char* reqhost;
    char* eol;
    char* cp;
    char* pi;

    hc->checked_idx = 0;
    hc->mime_flag = 0;
    hc->one_one = 0;
    method_str = bufgets( hc );
    url = strpbrk( method_str, " \t\n\r" );
    if ( url == (char*) 0 )
	{
	httpd_send_err( hc, 400, httpd_err400title, httpd_err400form, "" );
	return -1;
	}
    *url++ = '\0';
    url += strspn( url, " \t\n\r" );
    protocol = strpbrk( url, " \t\n\r" );
    if ( protocol == (char*) 0 )
	protocol = "HTTP/0.9";
    else
	{
	*protocol++ = '\0';
	protocol += strspn( protocol, " \t\n\r" );
	if ( *protocol != '\0' )
	    {
	    hc->mime_flag = 1;
	    eol = strpbrk( protocol, " \t\n\r" );
	    if ( eol != (char*) 0 )
		*eol = '\0';
	    if ( strcasecmp( protocol, "HTTP/1.0" ) != 0 )
		hc->one_one = 1;
	    }
	}
    /* Check for HTTP/1.1 absolute URL. */
    if ( strncasecmp( url, "http://", 7 ) == 0 )
	{
	if ( ! hc->one_one )
	    {
	    httpd_send_err( hc, 400, httpd_err400title, httpd_err400form, "" );
	    return -1;
	    }
	reqhost = url + 7;
	url = strchr( reqhost, '/' );
	if ( url == (char*) 0 )
	    {
	    httpd_send_err( hc, 400, httpd_err400title, httpd_err400form, "" );
	    return -1;
	    }
	*url = '\0';
	realloc_str( &hc->reqhost, &hc->maxreqhost, strlen( reqhost ) );
	(void) strcpy( hc->reqhost, reqhost );
	*url = '/';
	}
    else
	hc->reqhost[0] = '\0';

    if ( strcasecmp( method_str, httpd_method_str( METHOD_GET ) ) == 0 )
	hc->method = METHOD_GET;
    else if ( strcasecmp( method_str, httpd_method_str( METHOD_HEAD ) ) == 0 )
	hc->method = METHOD_HEAD;
    else if ( strcasecmp( method_str, httpd_method_str( METHOD_POST ) ) == 0 )
	hc->method = METHOD_POST;
    else
	{
	httpd_send_err( hc, 501, err501title, err501form, method_str );
	return -1;
	}

    hc->encodedurl = url;
    realloc_str(
	&hc->decodedurl, &hc->maxdecodedurl, strlen( hc->encodedurl ) );
    strdecode( hc->decodedurl, hc->encodedurl );

    hc->protocol = protocol;

    if ( hc->decodedurl[0] != '/' )
	{
	httpd_send_err( hc, 400, httpd_err400title, httpd_err400form, "" );
	return -1;
	}
    realloc_str(
	&hc->origfilename, &hc->maxorigfilename, strlen( hc->decodedurl ) );
    (void) strcpy( hc->origfilename, &hc->decodedurl[1] );
    /* Special case for top-level URL. */
    if ( hc->origfilename[0] == '\0' )
	(void) strcpy( hc->origfilename, "." );

    /* Extract query string from encoded URL. */
    hc->query[0] = '\0';
    cp = strchr( hc->encodedurl, '?' );
    if ( cp != (char*) 0 )
	{
	++cp;
	realloc_str( &hc->query, &hc->maxquery, strlen( cp ) );
	(void) strcpy( hc->query, cp );
	}
    /* And remove query from filename. */
    cp = strchr( hc->origfilename, '?' );
    if ( cp != (char*) 0 )
	*cp = '\0';

    /* Copy original filename to expanded filename. */
    realloc_str(
	&hc->expnfilename, &hc->maxexpnfilename, strlen( hc->origfilename ) );
    (void) strcpy( hc->expnfilename, hc->origfilename );

    /* Tilde mapping. */
    hc->altdir[0] = '\0';
    if ( hc->expnfilename[0] == '~' )
	{
	if ( ! tilde_map( hc ) )
	    {
	    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	    return -1;
	    }
	}

    /* Expand all symbolic links in the filename.  This also gives us
    ** any trailing non-existing components, for pathinfo.
    */
    cp = expand_symlinks( hc->expnfilename, &pi, hc->hs->chrooted );
    if ( cp == (char*) 0 )
	{
	httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	return -1;
	}
    realloc_str( &hc->expnfilename, &hc->maxexpnfilename, strlen( cp ) );
    (void) strcpy( hc->expnfilename, cp );
    realloc_str( &hc->pathinfo, &hc->maxpathinfo, strlen( pi ) );
    (void) strcpy( hc->pathinfo, pi );

    /* Remove pathinfo stuff from the original filename too. */
    if ( hc->pathinfo[0] != '\0' )
	{
	int i;
	i = strlen( hc->origfilename ) - strlen( hc->pathinfo );
	if ( i > 0 && strcmp( &hc->origfilename[i], hc->pathinfo ) == 0 )
	    hc->origfilename[i - 1] = '\0';
	}

    /* If the expanded filename is an absolute path, check that it's still
    ** within the current directory or the alternate directory.
    */
    if ( hc->expnfilename[0] == '/' )
	{
	if ( strncmp(
		 hc->expnfilename, hc->hs->cwd, strlen( hc->hs->cwd ) ) == 0 )
	    /* Elide the current directory. */
	    (void) strcpy(
		hc->expnfilename, &hc->expnfilename[strlen( hc->hs->cwd )] );
	else if ( hc->altdir[0] != '\0' &&
		  ( strncmp(
		       hc->expnfilename, hc->altdir,
		       strlen( hc->altdir ) ) != 0 ||
		    ( hc->expnfilename[strlen( hc->altdir )] != '\0' &&
		      hc->expnfilename[strlen( hc->altdir )] != '/' ) ) )
	    {
	    httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	    return -1;
	    }
	}

    hc->accept[0] = '\0';
    hc->accepte[0] = '\0';
    hc->remoteuser[0] = '\0';
    hc->response[0] = '\0';
    hc->responselen = 0;

    hc->referer = "";
    hc->useragent = "";
    hc->cookie = "";
    hc->contenttype = "";
    hc->hdrhost = "";
    hc->authorization = "";

    hc->if_modified_since = (time_t) -1;
    hc->range_if = (time_t) -1;
    hc->contentlength = -1;
    hc->got_range = 0;
    hc->init_byte_loc = 0;
    hc->end_byte_loc = -1;
    hc->keep_alive = 0;

    if ( hc->mime_flag )
	{
	/* Read the MIME headers. */
	while ( ( buf = bufgets( hc ) ) != (char*) 0 )
	    {
	    if ( buf[0] == '\0' )
		break;
	    if ( strncasecmp( buf, "Referer:", 8 ) == 0 )
		{
		cp = &buf[8];
		cp += strspn( cp, " \t" );
		hc->referer = cp;
		}
	    else if ( strncasecmp( buf, "User-Agent:", 11 ) == 0 )
		{
		cp = &buf[11];
		cp += strspn( cp, " \t" );
		hc->useragent = cp;
		}
	    else if ( strncasecmp( buf, "Host:", 5 ) == 0 )
		{
		cp = &buf[5];
		cp += strspn( cp, " \t" );
		hc->hdrhost = cp;
		}
	    else if ( strncasecmp( buf, "Accept:", 7 ) == 0 )
		{
		cp = &buf[7];
		cp += strspn( cp, " \t" );
		if ( hc->accept[0] != '\0' )
		    {
		    if ( strlen( hc->accept ) > 5000 )
			{
			syslog(
			    LOG_ERR, "%.80s way too much Accept: data",
			    inet_ntoa( hc->client_addr ) );
			continue;
			}
		    realloc_str(
			&hc->accept, &hc->maxaccept,
			strlen( hc->accept ) + 2 + strlen( cp ) );
		    (void) strcat( hc->accept, ", " );
		    }
		else
		    realloc_str( &hc->accept, &hc->maxaccept, strlen( cp ) );
		(void) strcat( hc->accept, cp );
		}
	    else if ( strncasecmp( buf, "Accept-Encoding:", 16 ) == 0 )
		{
		cp = &buf[16];
		cp += strspn( cp, " \t" );
		if ( hc->accepte[0] != '\0' )
		    {
		    if ( strlen( hc->accepte ) > 5000 )
			{
			syslog(
			    LOG_ERR, "%.80s way too much Accept-Encoding: data",
			    inet_ntoa( hc->client_addr ) );
			continue;
			}
		    realloc_str(
			&hc->accepte, &hc->maxaccepte,
			strlen( hc->accepte ) + 2 + strlen( cp ) );
		    (void) strcat( hc->accepte, ", " );
		    }
		else
		    realloc_str( &hc->accepte, &hc->maxaccepte, strlen( cp ) );
		(void) strcpy( hc->accepte, cp );
		}
	    else if ( strncasecmp( buf, "If-Modified-Since:", 18 ) == 0 )
		{
		cp = &buf[18];
		hc->if_modified_since = tdate_parse( cp );
		if ( hc->if_modified_since == (time_t) -1 )
		    syslog( LOG_DEBUG, "unparsable time: %.80s", cp );
		}
	    else if ( strncasecmp( buf, "Cookie:", 7 ) == 0 )
		{
		cp = &buf[7];
		cp += strspn( cp, " \t" );
		hc->cookie = cp;
		}
	    else if ( strncasecmp( buf, "Range:", 6 ) == 0 )
		{
		/* Only support %d- and %d-%d, not %d-%d,%d-%d or -%d. */
		if ( strchr( buf, ',' ) == (char*) 0 )
		    {
		    char* cp_dash;
		    cp = strpbrk( buf, "=" );
		    if ( cp != (char*) 0 )
			{
			cp_dash = strchr( cp + 1, '-' );
			if ( cp_dash != (char*) 0 && cp_dash != cp + 1 )
			    {
			    *cp_dash = '\0';
			    hc->got_range = 1;
			    hc->init_byte_loc = atol( cp + 1 );
			    if ( isdigit( (int) cp_dash[1] ) )
				hc->end_byte_loc = atol( cp_dash + 1 );
			    }
			}
		    }
		}
	    else if ( strncasecmp( buf, "Range-If:", 9 ) == 0 ||
	              strncasecmp( buf, "If-Range:", 9 ) == 0 )
		{
		cp = &buf[9];
		hc->range_if = tdate_parse( cp );
		if ( hc->range_if == (time_t) -1 )
		    syslog( LOG_DEBUG, "unparsable time: %.80s", cp );
		}
	    else if ( strncasecmp( buf, "Content-Type:", 13 ) == 0 )
		{
		cp = &buf[13];
		cp += strspn( cp, " \t" );
		hc->contenttype = cp;
		}
	    else if ( strncasecmp( buf, "Content-Length:", 15 ) == 0 )
		{
		cp = &buf[15];
		hc->contentlength = atol( cp );
		}
	    else if ( strncasecmp( buf, "Authorization:", 14 ) == 0 )
		{
		cp = &buf[14];
		cp += strspn( cp, " \t" );
		hc->authorization = cp;
		}
	    else if ( strncasecmp( buf, "Connection:", 11 ) == 0 )
		{
		cp = &buf[11];
		cp += strspn( cp, " \t" );
		if ( strcasecmp( cp, "keep-alive" ) == 0 )
		    hc->keep_alive = 1;
		}
#ifdef LOG_UNKNOWN_HEADERS
	    else if ( strncasecmp( buf, "Accept-Charset:", 15 ) == 0 ||
	              strncasecmp( buf, "Accept-Language:", 16 ) == 0 ||
	              strncasecmp( buf, "Agent:", 6 ) == 0 ||
	              strncasecmp( buf, "Cache-Control:", 14 ) == 0 ||
	              strncasecmp( buf, "Cache-Info:", 11 ) == 0 ||
	              strncasecmp( buf, "Charge-To:", 10 ) == 0 ||
	              strncasecmp( buf, "Client-ip:", 10 ) == 0 ||
	              strncasecmp( buf, "Date:", 5 ) == 0 ||
	              strncasecmp( buf, "Extension:", 10 ) == 0 ||
	              strncasecmp( buf, "Forwarded:", 10 ) == 0 ||
	              strncasecmp( buf, "From:", 5 ) == 0 ||
	              strncasecmp( buf, "HTTP-Version:", 13 ) == 0 ||
	              strncasecmp( buf, "Message-ID:", 11 ) == 0 ||
	              strncasecmp( buf, "MIME-Version:", 13 ) == 0 ||
	              strncasecmp( buf, "Negotiate:", 10 ) == 0 ||
	              strncasecmp( buf, "Pragma:", 7 ) == 0 ||
	              strncasecmp( buf, "Proxy-agent:", 12 ) == 0 ||
	              strncasecmp( buf, "Proxy-Connection:", 17 ) == 0 ||
	              strncasecmp( buf, "Security-Scheme:", 16 ) == 0 ||
	              strncasecmp( buf, "Session-ID:", 11 ) == 0 ||
	              strncasecmp( buf, "UA-color:", 9 ) == 0 ||
	              strncasecmp( buf, "UA-CPU:", 7 ) == 0 ||
	              strncasecmp( buf, "UA-Disp:", 8 ) == 0 ||
	              strncasecmp( buf, "UA-OS:", 6 ) == 0 ||
	              strncasecmp( buf, "UA-pixels:", 10 ) == 0 ||
	              strncasecmp( buf, "User:", 5 ) == 0 ||
	              strncasecmp( buf, "Via:", 4 ) == 0 ||
	              strncasecmp( buf, "X-", 2 ) == 0 )
		; /* ignore */
	    else
		syslog( LOG_DEBUG, "unknown request header: %.80s", buf );
#endif /* LOG_UNKNOWN_HEADERS */
	    }
	}
    
    if ( hc->one_one )
	{
	/* Check that HTTP/1.1 requests specify a host, as required. */
	if ( hc->reqhost[0] == '\0' && hc->hdrhost[0] == '\0' )
	    {
	    httpd_send_err( hc, 400, httpd_err400title, httpd_err400form, "" );
	    return -1;
	    }
	/* This is where we would check that the host specified is the
	** host we're serving.  However, since we still only support one
	** virtual host per thttpd process, it's not currently possible
	** to use thttpd to serve the new HTTP/1.1-style CNAME-only
	** virtual hosts.  Therefore, why bother checking for the correct
	** host here?  It would just be something for the admin to screw
	** up, with no benefit from getting it right.
	*/

	/* If the client wants to do keep-alives, it might also be doing
	** pipelining.  There's no way for us to tell.  Since we don't
	** implement keep-alives yet, if we close such a connection there
	** might be unread pipelined requests waiting.  So, we have to
	** do a lingering close.
	*/
	if ( hc->keep_alive )
	    hc->should_linger = 1;
	}

    return 0;
    }


static char*
bufgets( httpd_conn* hc )
    {
    int i;
    char c;

    for ( i = hc->checked_idx; hc->checked_idx < hc->read_idx; ++hc->checked_idx )
	{
	c = hc->read_buf[hc->checked_idx];
	if ( c == '\n' || c == '\r' )
	    {
	    hc->read_buf[hc->checked_idx] = '\0';
	    ++hc->checked_idx;
	    if ( c == '\r' && hc->checked_idx < hc->read_idx &&
		 hc->read_buf[hc->checked_idx] == '\n' )
		{
		hc->read_buf[hc->checked_idx] = '\0';
		++hc->checked_idx;
		}
	    return &(hc->read_buf[i]);
	    }
	}
    return (char*) 0;
    }


void
httpd_close_conn( httpd_conn* hc, struct timeval* nowP )
    {
    if ( hc->file_address != (char*) 0 )
	{
	mmc_unmap( hc->file_address, nowP );
	hc->file_address = (char*) 0;
	}
    if ( hc->conn_fd >= 0 )
	{
	(void) close( hc->conn_fd );
	hc->conn_fd = -1;
	}
    }

void
httpd_destroy_conn( httpd_conn* hc )
    {
    if ( hc->initialized )
	{
	free( (void*) hc->decodedurl );
	free( (void*) hc->origfilename );
	free( (void*) hc->expnfilename );
	free( (void*) hc->encodings );
	free( (void*) hc->pathinfo );
	free( (void*) hc->query );
	free( (void*) hc->altdir );
	free( (void*) hc->accept );
	free( (void*) hc->accepte );
	free( (void*) hc->reqhost );
	free( (void*) hc->remoteuser );
	free( (void*) hc->response );
	hc->initialized = 0;
	}
    }


/* Figures out MIME encodings and type based on the filename.  Multiple
** encodings are separated by semicolons.
*/
static void
figure_mime( httpd_conn* hc )
    {
    int i, j, k, l;
    int got_enc;
    struct table {
	char* ext;
	char* val;
	};
    static struct table enc_tab[] = {
#include "mime_encodings.h"
	};
    static struct table typ_tab[] = {
#include "mime_types.h"
	};

    /* Look at the extensions on hc->expnfilename from the back forwards. */
    hc->encodings[0] = '\0';
    i = strlen( hc->expnfilename );
    for (;;)
	{
	j = i;
	for (;;)
	    {
	    --i;
	    if ( i <= 0 )
		{
		/* No extensions left. */
		hc->type = "text/plain";
		return;
		}
	    if ( hc->expnfilename[i] == '.' )
		break;
	    }
	/* Found an extension. */
	got_enc = 0;
	for ( k = 0; k < sizeof(enc_tab)/sizeof(*enc_tab); ++k )
	    {
	    l = strlen( enc_tab[k].ext );
	    if ( l == j - i - 1 &&
		 strncasecmp( &hc->expnfilename[i+1], enc_tab[k].ext, l ) == 0 )
		{
		realloc_str(
		    &hc->encodings, &hc->maxencodings,
		    strlen( enc_tab[k].val ) + 1 );
		if ( hc->encodings[0] != '\0' )
		    (void) strcat( hc->encodings, ";" );
		(void) strcat( hc->encodings, enc_tab[k].val );
		got_enc = 1;
		}
	    }
	if ( ! got_enc )
	    {
	    /* No encoding extension found - time to try type extensions. */
	    for ( k = 0; k < sizeof(typ_tab)/sizeof(*typ_tab); ++k )
		{
		l = strlen( typ_tab[k].ext );
		if ( l == j - i - 1 &&
		     strncasecmp(
			 &hc->expnfilename[i+1], typ_tab[k].ext, l ) == 0 )
		    {
		    hc->type = typ_tab[k].val;
		    return;
		    }
		}
	    /* No recognized type extension found - return default. */
	    hc->type = "text/plain";
	    return;
	    }
	}
    }


#ifdef CGI_TIMELIMIT
static void
cgi_kill2( ClientData client_data, struct timeval* nowP )
    {
    pid_t pid;

    pid = (pid_t) client_data.i;
    if ( kill( pid, SIGKILL ) == 0 )
	syslog( LOG_ERR, "hard-killed CGI process %d", pid );
    }

static void
cgi_kill( ClientData client_data, struct timeval* nowP )
    {
    pid_t pid;

    /* Before trying to kill the CGI process, reap any zombie processes.
    ** That may get rid of the CGI process.
    */
    (void) do_reap();

    pid = (pid_t) client_data.i;
    if ( kill( pid, SIGINT ) == 0 )
	{
	syslog( LOG_ERR, "killed CGI process %d", pid );
	/* In case this isn't enough, schedule an uncatchable kill. */
	(void) tmr_create( nowP, cgi_kill2, client_data, 5 * 1000L, 0 );
	}
    }
#endif /* CGI_TIMELIMIT */


#ifdef GENERATE_INDEXES

/* qsort comparison routine - declared old-style on purpose, for portability. */
static int
name_compare( a, b )
    char** a;
    char** b;
    {
    return strcmp( *a, *b );
    }


static off_t
ls( httpd_conn* hc )
    {
    DIR* dirp;
    struct dirent* de;
    int namlen;
    static int maxnames = 0;
    int nnames;
    static char* names;
    static char** nameptrs;
    static char* name;
    static int maxname = 0;
    static char* rname;
    static int maxrname = 0;
    FILE* fp;
    int i, r;
    struct stat sb;
    struct stat lsb;
    char modestr[20];
    char* linkprefix;
    char link[MAXPATHLEN];
    int linklen;
    char* fileclass;
    time_t now;
    char* timestr;
    ClientData client_data;

    dirp = opendir( hc->expnfilename );
    if ( dirp == (DIR*) 0 )
	{
	syslog( LOG_ERR, "opendir %.80s - %m", hc->expnfilename );
	httpd_send_err( hc, 404, err404title, err404form, hc->encodedurl );
	return -1;
	}

    send_mime(
	hc, 200, ok200title, "", "", "text/html", -1, hc->sb.st_mtime );
    hc->bytes = 0;
    if ( hc->method == METHOD_HEAD )
	closedir( dirp );
    else if ( hc->method == METHOD_GET )
	{
	httpd_write_response( hc );
#ifdef EMBED
	r = vfork( );
#else
	r = fork( );
#endif
	if ( r < 0 )
	    {
	    syslog( LOG_ERR, "fork - %m" );
	    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	    return -1;
	    }
	if ( r == 0 )
	    {
	    /* Child process. */

#ifdef CGI_NICE
	    /* Set priority. */
	    (void) nice( CGI_NICE );
#endif /* CGI_NICE */

	    /* Open a stdio stream so that we can use fprintf, which is more
	    ** efficient than a bunch of separate write()s.  We don't have
	    ** to worry about double closes or file descriptor leaks cause
	    ** we're in a subprocess.
	    */
	    fp = fdopen( hc->conn_fd, "w" );
	    if ( fp == (FILE*) 0 )
		{
		syslog( LOG_ERR, "fdopen - %m" );
		httpd_send_err(
		    hc, 500, err500title, err500form, hc->encodedurl );
		closedir( dirp );
		return -1;
		}

	    (void) fprintf( fp, "\
<HTML><HEAD><TITLE>Index of %.80s</TITLE></HEAD>\n\
<BODY>\n\
<H2>Index of %.80s</H2>\n\
<PRE>\n\
mode  links  bytes  last-changed  name\n\
<HR>",
		hc->encodedurl, hc->encodedurl );

	    /* Read in names. */
	    nnames = 0;
	    while ( ( de = readdir( dirp ) ) != 0 )	/* dirent or direct */
		{
		if ( nnames >= maxnames )
		    {
		    if ( maxnames == 0 )
			{
			maxnames = 100;
			names = NEW( char, maxnames * MAXPATHLEN );
			nameptrs = NEW( char*, maxnames );
			}
		    else
			{
			maxnames *= 2;
			names = RENEW( names, char, maxnames * MAXPATHLEN );
			nameptrs = RENEW( nameptrs, char*, maxnames );
			}
		    if ( names == (char*) 0  || nameptrs == (char**) 0 )
			{
			syslog( LOG_ERR, "out of memory" );
			exit( 1 );
			}
		    for ( i = 0; i < maxnames; ++i )
			nameptrs[i] = &names[i * MAXPATHLEN];
		    }
		namlen = NAMLEN(de);
		(void) strncpy( nameptrs[nnames], de->d_name, namlen );
		nameptrs[nnames][namlen] = '\0';
		++nnames;
		}
	    closedir( dirp );

	    /* Sort the names. */
	    qsort( nameptrs, nnames, sizeof(*nameptrs), name_compare );

	    /* Generate output. */
	    for ( i = 0; i < nnames; ++i )
		{
		realloc_str(
		    &name, &maxname,
		    strlen( hc->expnfilename ) + 1 + strlen( nameptrs[i] ) );
		realloc_str(
		    &rname, &maxrname,
		    strlen( hc->origfilename ) + 1 + strlen( nameptrs[i] ) );
		if ( hc->expnfilename[0] == '\0' ||
		     strcmp( hc->expnfilename, "." ) == 0 )
		    {
		    (void) strcpy( name, nameptrs[i] );
		    (void) strcpy( rname, nameptrs[i] );
		    }
		else
		    {
		    (void) sprintf(
			name, "%s/%s", hc->expnfilename, nameptrs[i] );
		    (void) sprintf(
			rname, "%s%s", hc->origfilename, nameptrs[i] );
		    }
		if ( stat( name, &sb ) < 0 || lstat( name, &lsb ) < 0 )
		    continue;
		linkprefix = "";
		link[0] = '\0';
		/* Break down mode word.  First the file type. */
		switch ( lsb.st_mode & S_IFMT )
		    {
		    case S_IFIFO:  modestr[0] = 'p'; break;
		    case S_IFCHR:  modestr[0] = 'c'; break;
		    case S_IFDIR:  modestr[0] = 'd'; break;
		    case S_IFBLK:  modestr[0] = 'b'; break;
		    case S_IFREG:  modestr[0] = '-'; break;
		    case S_IFSOCK: modestr[0] = 's'; break;
		    case S_IFLNK:  modestr[0] = 'l';
		    linklen = readlink( name, link, sizeof(link) );
		    if ( linklen != -1 )
			{
			link[linklen] = '\0';
			linkprefix = " -> ";
			}
		    break;
		    default:       modestr[0] = '?'; break;
		    }
		/* Now the world permissions.  Owner and group permissions
		** are not of interest to web clients.
		*/
		modestr[1] = ( lsb.st_mode & S_IROTH ) ? 'r' : '-';
		modestr[2] = ( lsb.st_mode & S_IWOTH ) ? 'w' : '-';
		modestr[3] = ( lsb.st_mode & S_IXOTH ) ? 'x' : '-';
		modestr[4] = '\0';

		/* We also leave out the owner and group name, they are
		** also not of interest to web clients.  Plus if we're
		** running under chroot(), they would require a copy
		** of /etc/passwd and /etc/group, which we want to avoid.
		*/

		/* Get time string. */
		now = time( (time_t*) 0 );
		timestr = ctime( &lsb.st_mtime );
		timestr[ 0] = timestr[ 4];
		timestr[ 1] = timestr[ 5];
		timestr[ 2] = timestr[ 6];
		timestr[ 3] = ' ';
		timestr[ 4] = timestr[ 8];
		timestr[ 5] = timestr[ 9];
		timestr[ 6] = ' ';
		if ( now - lsb.st_mtime > 60*60*24*182 )	/* 1/2 year */
		    {
		    timestr[ 7] = ' ';
		    timestr[ 8] = timestr[20];
		    timestr[ 9] = timestr[21];
		    timestr[10] = timestr[22];
		    timestr[11] = timestr[23];
		    }
		else
		    {
		    timestr[ 7] = timestr[11];
		    timestr[ 8] = timestr[12];
		    timestr[ 9] = ':';
		    timestr[10] = timestr[14];
		    timestr[11] = timestr[15];
		    }
		timestr[12] = '\0';

		/* The ls -F file class. */
		switch ( sb.st_mode & S_IFMT )
		    {
		    case S_IFDIR:  fileclass = "/"; break;
		    case S_IFSOCK: fileclass = "="; break;
		    case S_IFLNK:  fileclass = "@"; break;
		    default:
		    fileclass = ( sb.st_mode & S_IXOTH ) ? "*" : "";
		    break;
		    }

		/* And print. */
		(void)  fprintf( fp,
		   "%s %3ld  %8ld  %s  <A HREF=\"/%.500s%s\">%.80s</A>%s%s%s\n",
		    modestr, (long) lsb.st_nlink, (long) lsb.st_size, timestr,
		    rname, S_ISDIR(sb.st_mode) ? "/" : "", nameptrs[i],
		    linkprefix, link, fileclass );
		}

	    (void) fprintf( fp, "</PRE></BODY></HTML>\n" );
	    (void) fclose( fp );
	    exit( 0 );
	    }

	/* Parent process. */
	closedir( dirp );
#ifdef CGI_TIMELIMIT
	/* Schedule a kill for the child process, in case it runs too long */
	client_data.i = r;
	(void) tmr_create(
	    (struct timeval*) 0, cgi_kill, client_data, CGI_TIMELIMIT * 1000L,
	    0 );
#endif /* CGI_TIMELIMIT */
	hc->status = 200;
	hc->bytes = CGI_BYTECOUNT;
	}
    else
	{
	httpd_send_err(
	    hc, 501, err501title, err501form, httpd_method_str( hc->method ) );
	return -1;
	}

    return 0;
    }

#endif /* GENERATE_INDEXES */


static char*
build_env( char* fmt, char* arg )
    {
    char* cp;
    int size;
    static char* buf;
    static int maxbuf = 0;

    size = strlen( fmt ) + strlen( arg );
    if ( size > maxbuf )
        realloc_str( &buf, &maxbuf, size );
    (void) sprintf( buf, fmt, arg );
    cp = strdup( buf );
    if ( cp == (char*) 0 )
        {
        syslog( LOG_ERR, "out of memory" );
        exit( 1 );
        }
    return cp;
    }


#ifdef SERVER_NAME_LIST
static char*
hostname_map( char* hostname )
    {
    int len, n;
    static char* list[] = { SERVER_NAME_LIST };

    len = strlen( hostname );
    for ( n = sizeof(list) / sizeof(*list) - 1; n >= 0; --n )
	if ( strncasecmp( hostname, list[n], len ) == 0 )
	    if ( list[n][len] == '/' )	/* check in case of a substring match */
		return &list[n][len + 1];
    return (char*) 0;
    }
#endif /* SERVER_NAME_LIST */


/* Set up environment variables. Be real careful here to avoid
** letting malicious clients overrun a buffer.  We don't have
** to worry about freeing stuff since we're a sub-process.
*/
static char**
make_envp( httpd_conn* hc )
    {
    static char* envp[50];
    int envn;
    char* cp;
    char buf[256];

    envn = 0;
    envp[envn++] = build_env( "PATH=%s", CGI_PATH );
    envp[envn++] = build_env( "SERVER_SOFTWARE=%s", SERVER_SOFTWARE );
    cp = hc->hs->hostname;
#ifdef SERVER_NAME_LIST
    if ( cp == (char*) 0 && gethostname( buf, sizeof(buf) ) >= 0 )
	cp = hostname_map( buf );
#endif /* SERVER_NAME_LIST */
    if ( cp == (char*) 0 )
	{
#ifdef SERVER_NAME
	cp = SERVER_NAME;
#else /* SERVER_NAME */
	if ( gethostname( buf, sizeof(buf) ) >= 0 )
	    cp = buf;
#endif /* SERVER_NAME */
	}
    if ( cp != (char*) 0 )
	envp[envn++] = build_env( "SERVER_NAME=%s", cp );
    envp[envn++] = "GATEWAY_INTERFACE=CGI/1.1";
    envp[envn++] = build_env("SERVER_PROTOCOL=%s", hc->protocol);
    (void) sprintf( buf, "%d", hc->hs->port );
    envp[envn++] = build_env( "SERVER_PORT=%s", buf );
    envp[envn++] = build_env(
	"REQUEST_METHOD=%s", httpd_method_str( hc->method ) );
    if ( hc->pathinfo[0] != '\0' )
	{
	char* cp2;
	envp[envn++] = build_env( "PATH_INFO=/%s", hc->pathinfo );
	cp2 = NEW( char, strlen( hc->hs->cwd ) + strlen( hc->pathinfo ) );
	if ( cp2 != (char*) 0 )
	    {
	    (void) sprintf( cp2, "%s%s", hc->hs->cwd, hc->pathinfo );
	    envp[envn++] = build_env( "PATH_TRANSLATED=%s", cp2 );
	    }
	}
    envp[envn++] = build_env( "SCRIPT_NAME=/%s", hc->origfilename );
    if ( hc->query[0] != '\0')
	envp[envn++] = build_env( "QUERY_STRING=%s", hc->query );
    envp[envn++] = build_env( "REMOTE_ADDR=%s", inet_ntoa( hc->client_addr ) );
    if ( hc->referer[0] != '\0' )
	envp[envn++] = build_env( "HTTP_REFERER=%s", hc->referer );
    if ( hc->useragent[0] != '\0' )
	envp[envn++] = build_env( "HTTP_USER_AGENT=%s", hc->useragent );
    if ( hc->accept[0] != '\0' )
	envp[envn++] = build_env( "HTTP_ACCEPT=%s", hc->accept );
    if ( hc->accepte[0] != '\0' )
	envp[envn++] = build_env( "HTTP_ACCEPT_ENCODING=%s", hc->accepte );
    if ( hc->cookie[0] != '\0' )
	envp[envn++] = build_env( "HTTP_COOKIE=%s", hc->cookie );
    if ( hc->contenttype[0] != '\0' )
	envp[envn++] = build_env( "CONTENT_TYPE=%s", hc->contenttype );
    if ( hc->contentlength != -1 )
	{
	(void) sprintf( buf, "%ld", (long) hc->contentlength );
	envp[envn++] = build_env( "CONTENT_LENGTH=%s", buf );
	}
    if ( hc->remoteuser[0] != '\0' )
	envp[envn++] = build_env( "REMOTE_USER=%s", hc->remoteuser );
    if ( getenv( "TZ" ) != (char*) 0 )
	envp[envn++] = build_env( "TZ=%s", getenv( "TZ" ) );

    envp[envn] = (char*) 0;
    return envp;
    }


/* Set up argument vector.  Again, we don't have to worry about freeing stuff
** since we're a sub-process.  This gets done after make_envp() because we
** scribble on hc->query.
*/
static char**
make_argp( httpd_conn* hc )
    {
    char** argp;
    int argn;
    char* cp1;
    char* cp2;

    /* By allocating an arg slot for every character in the query, plus
    ** one for the filename and one for the NULL, we are guaranteed to
    ** have enough.  We could actually use strlen/2.
    */
    argp = NEW( char*, strlen( hc->query ) + 2 );
    if ( argp == (char**) 0 )
	return (char**) 0;

    argp[0] = strrchr( hc->expnfilename, '/' );
    if ( argp[0] != (char*) 0 )
	++argp[0];
    else
	argp[0] = hc->expnfilename;

    argn = 1;
    /* According to the CGI spec at http://hoohoo.ncsa.uiuc.edu/cgi/cl.html,
    ** "The server should search the query information for a non-encoded =
    ** character to determine if the command line is to be used, if it finds
    ** one, the command line is not to be used."
    */
    if ( strchr( hc->query, '=' ) == (char*) 0 )
	{
	for ( cp1 = cp2 = hc->query; *cp2 != '\0'; ++cp2 )
	    {
	    if ( *cp2 == '+' )
		{
		*cp2 = '\0';
		strdecode( cp1, cp1 );
		argp[argn++] = cp1;
		cp1 = cp2 + 1;
		}
	    }
	if ( cp2 != cp1 )
	    {
	    strdecode( cp1, cp1 );
	    argp[argn++] = cp1;
	    }
	}

    argp[argn] = (char*) 0;
    return argp;
    }


/* This routine is used only for POST requests.  It reads the data
** from the request and sends it to the child process.  The only reason
** we need to do it this way instead of just letting the child read
** directly is that we have already read part of the data into our
** buffer.
*/
static void
cgi_interpose( httpd_conn* hc, int wfd )
    {
    int c, r;
    char buf[1024];

    c = hc->read_idx - hc->checked_idx;
    if ( c > 0 )
	{
	if ( write( wfd, &(hc->read_buf[hc->checked_idx]), c ) < 0 )
	    exit( -1 );
	}
    while ( c < hc->contentlength )
	{
	r = read( hc->conn_fd, buf, MIN( sizeof(buf), hc->contentlength - c ) );
	if ( r == 0 )
	    sleep( 1 );
	else if ( r < 0 )
	    {
	    if ( errno == EAGAIN )
		sleep( 1 );
	    else
		exit( -1 );
	    }
	else
	    {
	    if ( write( wfd, buf, r ) < 0 )
		exit( -1 );
	    c += r;
	    }
	}
    exit( 0 );
    }


/* CGI child process. */
static void
cgi_child( httpd_conn* hc )
    {
    int r;
    char** argp;
    char** envp;
    char http_head[] = "HTTP/1.0 200 OK\n";
    char* binary;
    char* directory;

    /* Set up stdin.  For POSTs we may have to set up a pipe from an
    ** interposer process, depending on if we've read some of the data
    ** into our buffer.
    */
    if ( hc->method == METHOD_POST && hc->read_idx > hc->checked_idx )
	{
	int p[2];

	if ( pipe( p ) < 0 )
	    {
	    syslog( LOG_ERR, "pipe - %m" );
	    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	    exit( 1 );
	    }
#ifdef EMBED
	r = vfork( );
#else
	r = fork( );
#endif
	if ( r < 0 )
	    {
	    syslog( LOG_ERR, "fork - %m" );
	    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	    exit( 1 );
	    }
	if ( r == 0 )
	    {
	    /* Interposer process. */
	    (void) close( p[0] );
	    cgi_interpose( hc, p[1] );
	    }
	(void) close( p[1] );
	(void) dup2( p[0], STDIN_FILENO );
	}
    else
	{
	/* Otherwise, the request socket is stdin. */
	(void) dup2( hc->conn_fd, STDIN_FILENO );
	}

    /* Unset close-on-exec flag for this socket.  This actually shouldn't
    ** be necessary, according to POSIX a dup()'d file descriptor does
    ** *not* inherit the close-on-exec flag, its flag is always clear.
    ** However, Linux messes this up and does copy the flag to the
    ** dup()'d descriptor, so we have to clear it.  This could be
    ** ifdeffed for Linux only.
    */
    (void) fcntl( hc->conn_fd, F_SETFD, 0 );

    /* The response socket always becomes stdout and stderr. */
    (void) dup2( hc->conn_fd, STDOUT_FILENO );
    (void) dup2( hc->conn_fd, STDERR_FILENO );

    /* Close the syslog descriptor so that the CGI program can't
    ** mess with it.  All other open descriptors should be either
    ** the listen socket, sockets from accept(), or the file-logging
    ** fd, and all of those are set to close-on-exec, so we don't
    ** have to close anything else.
    */
    closelog();

    /* Make the environment vector. */
    envp = make_envp( hc );

    /* Make the argument vector. */
    argp = make_argp( hc );

    /* Be minimally compatible with the parsed-headers vs. nph- crap;
    ** but never send header for HTTP/0.9
    */
    if ( strncmp( argp[0], "nph-", 4 ) != 0 && hc->mime_flag )
	(void) write( STDOUT_FILENO, http_head, sizeof(http_head) - 1 );

#ifdef CGI_NICE
    /* Set priority. */
    (void) nice( CGI_NICE );
#endif /* CGI_NICE */

    /* Split the program into directory and binary, so we can chdir()
    ** to the program's own directory.  This isn't in the CGI 1.1
    ** spec, but it's what most other HTTP servers do.
    */
    directory = strdup( hc->expnfilename );
    if ( directory == (char*) 0 )
	binary = hc->expnfilename;	/* ignore errors */
    else
	{
	binary = strrchr( directory, '/' );
	if ( binary == (char*) 0 )
	    binary = hc->expnfilename;
	else
	    {
	    *binary++ = '\0';
	    (void) chdir( directory );	/* ignore errors */
	    }
	}

    /* Run the program. */
    (void) execve( binary, argp, envp );

    /* Something went wrong. */
    syslog( LOG_ERR, "execve %s - %m", hc->expnfilename );
    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
    exit( 1 );
    }


static off_t
cgi( httpd_conn* hc )
    {
    int r;
    ClientData client_data;

    if ( hc->method == METHOD_GET || hc->method == METHOD_POST )
	{
	httpd_write_response( hc );
#ifdef EMBED
	r = vfork( );
#else
	r = fork( );
#endif
	if ( r < 0 )
	    {
	    syslog( LOG_ERR, "fork - %m" );
	    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	    return -1;
	    }
	if ( r == 0 )
	    cgi_child( hc );

	/* Parent process. */
#ifdef CGI_TIMELIMIT
	/* Schedule a kill for the child process, in case it runs too long */
	client_data.i = r;
	(void) tmr_create(
	    (struct timeval*) 0, cgi_kill, client_data, CGI_TIMELIMIT * 1000L,
	    0 );
#endif /* CGI_TIMELIMIT */
	hc->status = 200;
	hc->bytes = CGI_BYTECOUNT;
	}
    else
	{
	httpd_send_err(
	    hc, 501, err501title, err501form, httpd_method_str( hc->method ) );
	return -1;
	}

    return 0;
    }


static int
really_start_request( httpd_conn* hc )
    {
    static char* indexname;
    static int maxindexname = 0;
#ifdef AUTH_FILE
    static char* dirname;
    static int maxdirname = 0;
#endif /* AUTH_FILE */
    int expnlen, indxlen;
    char* cp;
    char* pi;

    expnlen = strlen( hc->expnfilename );

    if ( hc->method != METHOD_GET && hc->method != METHOD_HEAD &&
	 hc->method != METHOD_POST )
	{
	httpd_send_err(
	    hc, 501, err501title, err501form, httpd_method_str( hc->method ) );
	return -1;
	}

    /* Stat the file. */
    if ( stat( hc->expnfilename, &hc->sb ) < 0 )
	{
	httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	return -1;
	}

    /* Is it world-readable or world-executable?  We check explicitly instead
    ** of just trying to open it, so that no one ever gets surprised by
    ** a file that's not set world-readable and yet somehow is
    ** readable by the HTTP server and therefore the *whole* world.
    */
    if ( ! ( hc->sb.st_mode & ( S_IROTH | S_IXOTH ) ) )
	{
	httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	return -1;
	}

    /* Is it a directory? */
    if ( S_ISDIR(hc->sb.st_mode) )
	{
	/* If there's pathinfo, it's just a non-existent file. */
	if ( hc->pathinfo[0] != '\0' )
	    {
	    httpd_send_err( hc, 404, err404title, err404form, hc->encodedurl );
	    return -1;
	    }

	/* Special handling for directory URLs that don't end in a slash.
	** We send back an explicit redirect with the slash, because
	** otherwise many clients can't build relative URLs properly.
	*/
	if ( hc->decodedurl[strlen( hc->decodedurl ) - 1] != '/' )
	    {
	    send_dirredirect( hc );
	    return -1;
	    }

	/* Check for an index.html file. */
	realloc_str(
	    &indexname, &maxindexname, expnlen + 1 + sizeof(INDEX_NAME) );
	(void) strcpy( indexname, hc->expnfilename );
	indxlen = strlen( indexname );
	if ( indxlen == 0 || indexname[indxlen - 1] != '/' )
	    (void) strcat( indexname, "/" );
	(void) strcat( indexname, INDEX_NAME );
	if ( stat( indexname, &hc->sb ) < 0 )
	    {
	    /* Nope, no index.html, so it's an actual directory request. */
#ifdef GENERATE_INDEXES

	    /* Directories must be readable for indexing. */
	    if ( ! ( hc->sb.st_mode & S_IROTH ) )
		{
		httpd_send_err(
		    hc, 403, err403title, err403form, hc->encodedurl );
		return -1;
		}
#ifdef AUTH_FILE
	    /* Check authorization for this directory. */
	    if ( auth_check( hc, hc->expnfilename ) == -1 )
		return -1;
#endif /* AUTH_FILE */
	    return ls( hc );

#else /* GENERATE_INDEXES */

	    httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	    return -1;

#endif /* GENERATE_INDEXES */
	    }
	/* Expand symlinks again.  More pathinfo means something went wrong. */
	cp = expand_symlinks( indexname, &pi, hc->hs->chrooted );
	if ( cp == (char*) 0 || pi[0] != '\0' )
	    {
	    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	    return -1;
	    }
	expnlen = strlen( cp );
	realloc_str( &hc->expnfilename, &hc->maxexpnfilename, expnlen );
	(void) strcpy( hc->expnfilename, cp );

	/* Now, is the index.html version world-readable or world-executable? */
	if ( ! ( hc->sb.st_mode & ( S_IROTH | S_IXOTH ) ) )
	    {
	    httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	    return -1;
	    }
	}

#ifdef AUTH_FILE
    /* Check authorization for this directory. */
    realloc_str( &dirname, &maxdirname, expnlen );
    (void) strcpy( dirname, hc->expnfilename );
    cp = strrchr( dirname, '/' );
    if ( cp == (char*) 0 )
	(void) strcpy( dirname, "." );
    else
	*cp = '\0';
    if ( auth_check( hc, dirname ) == -1 )
	return -1;

    /* Check if the filename is the AUTH_FILE itself - that's verboten. */
    if ( expnlen == sizeof(AUTH_FILE) - 1 )
	{
	if ( strcmp( hc->expnfilename, AUTH_FILE ) == 0 )
	    {
	    httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	    return -1;
	    }
	}
    else
	{
	if ( strcmp( &(hc->expnfilename[expnlen - sizeof(AUTH_FILE) + 1]), AUTH_FILE ) == 0 &&
	     hc->expnfilename[expnlen - sizeof(AUTH_FILE)] == '/' )
	    {
	    httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	    return -1;
	    }
	}
#endif /* AUTH_FILE */

    /* Is it world-executable and in the CGI area? */
    if ( hc->hs->cgi_pattern != (char*) 0 &&
	 ( hc->sb.st_mode & S_IXOTH ) &&
	 match( hc->hs->cgi_pattern, hc->expnfilename ) )
	return cgi( hc );

    /* It's not CGI.  If it's executable or there's pathinfo, someone's
    ** trying to either serve or run a non-CGI file as CGI.   Either case
    ** is prohibited.
    */
    if ( ( hc->sb.st_mode & S_IXOTH ) || hc->pathinfo[0] != '\0' )
	{
	httpd_send_err( hc, 403, err403title, err403form, hc->encodedurl );
	return -1;
	}

    /* Fill in end_byte_loc if necessary. */
    if ( hc->got_range &&
	 ( hc->end_byte_loc == -1 || hc->end_byte_loc >= hc->sb.st_size ) )
	hc->end_byte_loc = hc->sb.st_size - 1;

    figure_mime( hc );

    if ( hc->method == METHOD_HEAD )
	{
	hc->bytes = 0;
	send_mime(
	    hc, 200, ok200title, hc->encodings, "", hc->type, hc->sb.st_size,
	    hc->sb.st_mtime );
	}
    else if ( hc->if_modified_since != (time_t) -1 &&
	 hc->if_modified_since == hc->sb.st_mtime )
	{
	hc->method = METHOD_HEAD;
	hc->bytes = 0;
	send_mime(
	    hc, 304, err304title, hc->encodings, "", hc->type, hc->sb.st_size,
	    hc->sb.st_mtime );
	}
    else
	{
	hc->file_address = mmc_map( hc->expnfilename, &(hc->sb) );
	if ( hc->file_address == (char*) 0 )
	    {
	    httpd_send_err( hc, 500, err500title, err500form, hc->encodedurl );
	    return -1;
	    }
	hc->bytes = hc->sb.st_size;
	send_mime(
	    hc, 200, ok200title, hc->encodings, "", hc->type, hc->sb.st_size,
	    hc->sb.st_mtime );
	}

    return 0;
    }


int
httpd_start_request( httpd_conn* hc )
    {
    int r;
    char bytes[40];
    char* ru;

    /* Really start the request, then do the log entry. */
    r = really_start_request( hc );

    /* This is straight CERN Combined Log Format - the only tweak
    ** being that if we're using syslog() we leave out the date, because
    ** syslogd puts it in.  The included syslogtocern script turns the
    ** results into true CERN format.
    */
    if ( (long) hc->bytes >= 0 )
	(void) sprintf( bytes, "%ld", (long) hc->bytes );
    else
	(void) strcpy( bytes, "-" );
    if ( hc->remoteuser[0] != '\0' )
	ru = hc->remoteuser;
    else
	ru = "-";
    if ( hc->hs->logfp != (FILE*) 0 )
	{
	time_t now;
	char date[100];

	now = time( (time_t*) 0 );
#ifdef EMBED
	date[0] = 0;
#else
	(void) strftime( date, sizeof(date), cernfmt, localtime( &now ) );
#endif
	(void) fprintf( hc->hs->logfp,
	    "%.80s - %.80s [%s] \"%.80s %.200s %.80s\" %d %s \"%.200s\" \"%.80s\"\n",
	    inet_ntoa( hc->client_addr ), ru, date,
	    httpd_method_str( hc->method ), hc->encodedurl, hc->protocol,
	    hc->status, bytes, hc->referer, hc->useragent );
	(void) fflush( hc->hs->logfp );
	}
    else
	syslog( LOG_INFO,
	    "%.80s - %.80s \"%.80s %.200s %.80s\" %d %s \"%.200s\" \"%.80s\"",
	    inet_ntoa( hc->client_addr ), ru,
	    httpd_method_str( hc->method ), hc->encodedurl, hc->protocol,
	    hc->status, bytes, hc->referer, hc->useragent );

    return r;
    }
