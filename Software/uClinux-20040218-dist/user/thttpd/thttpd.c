/* thttpd.c - tiny/turbo/throttling HTTP server
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif
#include <unistd.h>

#include "libhttpd.h"
#include "mmc.h"
#include "timers.h"
#include "match.h"

#ifndef FD_SET
#define NFDBITS		32
#define FD_SETSIZE	32
#define FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#endif /* !FD_SET */


static char* argv0;

typedef struct {
    char* pattern;
    long limit;
    long rate;
    time_t last_avg;
    off_t bytes_since_avg;
    int num_sending;
    } throttletab;
static throttletab* throttles;
static int numthrottles, maxthrottles;


typedef struct {
    int conn_state;
    httpd_conn hc;
    int tnums[MAXTHROTTLENUMS];		/* throttle indexes */
    int numtnums;
    long limit;
    time_t started_at;
    Timer* idle_read_timer;
    Timer* idle_send_timer;
    Timer* wakeup_timer;
    Timer* linger_timer;
    long wouldblock_delay;
    off_t bytes;
    off_t bytes_sent;
    off_t bytes_to_send;
    } connecttab;
static connecttab* connects;
static int numconnects, maxconnects;
static int recompute_fdsets;

/* The connection states. */
#define CNST_FREE 0
#define CNST_READING 1
#define CNST_SENDING 2
#define CNST_PAUSING 3
#define CNST_LINGERING 4


static httpd_server* hs = (httpd_server*) 0;
#ifdef STATS_TIME
int stats_connections, stats_simultaneous;
#endif /* STATS_TIME */


/* Forwards. */
static void usage( void );
static void read_throttlefile( char* throttlefile );
static void shut_down( void );
static int handle_newconnect( struct timeval* tvP );
static void handle_read( connecttab* c, struct timeval* tvP );
static void handle_send( connecttab* c, struct timeval* tvP );
static void handle_linger( connecttab* c, struct timeval* tvP );
static int check_throttles( connecttab* c );
static void clear_throttles( connecttab* c, struct timeval* tvP );
static void clear_connection( connecttab* c, struct timeval* tvP );
static void really_clear_connection( connecttab* c, struct timeval* tvP );
static void idle_read_connection( ClientData client_data, struct timeval* nowP );
static void idle_send_connection( ClientData client_data, struct timeval* nowP );
static void wakeup_connection( ClientData client_data, struct timeval* nowP );
static void linger_clear_connection( ClientData client_data, struct timeval* nowP );
static void occasional( ClientData client_data, struct timeval* nowP );
#ifdef STATS_TIME
static void show_stats( ClientData client_data, struct timeval* nowP );
#endif /* STATS_TIME */


static void
handle_kill( int sig )
    {
    shut_down();
    syslog( LOG_NOTICE, "exiting due to signal %d", sig );
    closelog();
    exit( 1 );
    }


int
main( int argc, char** argv )
    {
    char* cp;
    int argn;
    int debug;
    int port;
    char* dir;
    int do_chroot;
#ifndef EMBED
    struct passwd* pwd;
#endif
    uid_t uid;
    gid_t gid;
    char* cgi_pattern;
    char* throttlefile;
    char* hostname;
    char* logfile;
    char* user;
    char cwd[MAXPATHLEN];
    FILE* logfp;
    int nfiles;
    fd_set master_rfdset;
    fd_set master_wfdset;
    fd_set working_rfdset;
    fd_set working_wfdset;
    int num_ready;
    int cnum;
    connecttab* c;
    u_int addr;
    struct timeval tv;
    struct hostent* he;

    argv0 = argv[0];

    cp = strrchr( argv0, '/' );
    if ( cp != (char*) 0 )
	++cp;
    else
	cp = argv0;
    openlog( cp, LOG_NDELAY|LOG_PID, LOG_FACILITY );

    (void) signal( SIGINT, handle_kill );
    (void) signal( SIGTERM, handle_kill );
    (void) signal( SIGPIPE, SIG_IGN );		/* get EPIPE instead */

    debug = 0;
    port = DEFAULT_PORT;
#ifdef EMBED
    dir = DEFAULT_DIR;
#else
    dir = (char*) 0;
#endif
#ifdef ALWAYS_CHROOT
    do_chroot = 1;
#else /* ALWAYS_CHROOT */
    do_chroot = 0;
#endif /* ALWAYS_CHROOT */
#ifdef CGI_PATTERN
    cgi_pattern = CGI_PATTERN;
#else /* CGI_PATTERN */
    cgi_pattern = (char*) 0;
#endif /* CGI_PATTERN */
    throttlefile = (char*) 0;
    hostname = (char*) 0;
    logfile = (char*) 0;
    user = DEFAULT_USER;
    argn = 1;
    while ( argn < argc && argv[argn][0] == '-' )
	{
	if ( strcmp( argv[argn], "-D" ) == 0 )
	    debug = 1;
	else if ( strcmp( argv[argn], "-p" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    port = atoi( argv[argn] );
	    }
	else if ( strcmp( argv[argn], "-d" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    dir = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-r" ) == 0 )
	    do_chroot = 1;
	else if ( strcmp( argv[argn], "-nor" ) == 0 )
	    do_chroot = 0;
	else if ( strcmp( argv[argn], "-u" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    user = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-c" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    cgi_pattern = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-t" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    throttlefile = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-h" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    hostname = argv[argn];
	    }
	else if ( strcmp( argv[argn], "-l" ) == 0 && argn + 1 < argc )
	    {
	    ++argn;
	    logfile = argv[argn];
	    }
	else
	    usage();
	++argn;
	}
    if ( argn != argc )
	usage();

    /* Lookup hostname now in case we're going to chroot(). */
    if ( hostname == (char*) 0 )
	addr = htonl( INADDR_ANY );
    else
	{
	addr = inet_addr( hostname );
	if ( (int) addr == -1 )
	    {
	    he = gethostbyname( hostname );
	    if ( he == (struct hostent*) 0 )
		{
#ifdef HAVE_HSTRERROR
		syslog( LOG_CRIT, "gethostbyname %.80s - %s", hostname, hstrerror( h_errno ) );
		(void) fprintf( stderr, "gethostbyname %.80s - %s", hostname, hstrerror( h_errno ) );
#else /* HAVE_HSTRERROR */
		syslog( LOG_CRIT, "gethostbyname %.80s", hostname );
		(void) fprintf( stderr, "gethostbyname %.80s\n", hostname );
#endif /* HAVE_HSTRERROR */
		exit( 1 );
		}
	    if ( he->h_addrtype != AF_INET || he->h_length != sizeof(addr) )
		{
		syslog( LOG_CRIT, "%.80s - non-IP network address", hostname );
		(void) fprintf( stderr, "%.80s - non-IP network address\n", hostname );
		exit( 1 );
		}
	    (void) memcpy( &addr, he->h_addr, sizeof(addr) );
	    }
	}

    /* Check port number. */
    if ( port <= 0 )
	{
	syslog( LOG_CRIT, "illegal port number" );
	(void) fprintf( stderr, "illegal port number\n" );
	exit( 1 );
	}

    /* Throttle file. */
    numthrottles = 0;
    maxthrottles = 0;
    if ( throttlefile != (char*) 0 )
	read_throttlefile( throttlefile );

    /* Log file. */
    if ( logfile != (char*) 0 )
	{
	logfp = fopen( logfile, "a" );
	if ( logfp == (FILE*) 0 )
	    {
	    syslog( LOG_CRIT, "%.80s - %m", logfile );
	    perror( logfile );
	    exit( 1 );
	    }
	(void) fcntl( fileno( logfp ), F_SETFD, 1 );
	}
    else
	logfp = (FILE*) 0;

#ifdef EMBED
    uid = 0;
    gid = 0;
#else
    /* Figure out uid/gid from user. */
    pwd = getpwnam( user );
    if ( pwd == (struct passwd*) 0 )
	{
	syslog( LOG_CRIT, "unknown user - '%.80s'", user );
	(void) fprintf( stderr, "unknown user - '%s'\n", user );
	exit( 1 );
	}
    uid = pwd->pw_uid;
    gid = pwd->pw_gid;
#endif

    /* Switch directories if requested. */
    if ( dir != (char*) 0 )
	{
	if ( chdir( dir ) < 0 )
	    {
	    syslog( LOG_CRIT, "chdir - %m" );
	    perror( "chdir" );
	    exit( 1 );
	    }
	}
#ifdef USE_USER_DIR
    else if ( getuid() == 0 )
	{
	/* No explicit directory was specified, we're root, and the
	** USE_USER_DIR option is set - switch to the specified user's
	** home dir.
	*/
	if ( chdir( pwd->pw_dir ) < 0 )
	    {
	    syslog( LOG_CRIT, "chdir - %m" );
	    perror( "chdir" );
	    exit( 1 );
	    }
	}
#endif /* USE_USER_DIR */

    /* Get current directory. */
    (void) getcwd( cwd, sizeof(cwd) );
    if ( cwd[strlen( cwd ) - 1] != '/' )
	(void) strcat( cwd, "/" );

    /* Chroot if requested. */
    if ( do_chroot )
	{
	if ( chroot( cwd ) < 0 )
	    {
	    syslog( LOG_CRIT, "chroot - %m" );
	    perror( "chroot" );
	    exit( 1 );
	    }
	(void) strcpy( cwd, "/" );
	}

    /* Figure out how many file descriptors we can use. */
    nfiles = MIN( httpd_get_nfiles(), FD_SETSIZE );

    if ( ! debug )
	{
	/* We're not going to use stdin stdout or stderr from here on, so close
	** them to save file descriptors.
	*/
#ifndef EMBED
	(void) fclose( stdin );
	(void) fclose( stdout );
	(void) fclose( stderr );

	/* Daemonize - make ourselves a subprocess. */
	switch ( fork() )
	    {
	    case 0:
	    break;
	    case -1:
	    syslog( LOG_CRIT, "fork - %m" );
	    exit( 1 );
	    default:
	    exit( 0 );
	    }
#endif
	}

    /* Initialize the HTTP layer.  Got to do this before giving up root,
    ** so that we can bind to a privileged port.
    */
    hs = httpd_initialize(
	hostname, addr, port, cgi_pattern, cwd, logfp, do_chroot );
    if ( hs == (httpd_server*) 0 )
	exit( 1 );

    /* Set up the occasional timer. */
    (void) tmr_create(
	(struct timeval*) 0, occasional, (ClientData) 0,
	OCCASIONAL_TIME * 1000L, 1 );
#ifdef STATS_TIME
    /* Set up the stats timer. */
    (void) tmr_create(
	(struct timeval*) 0, show_stats, (ClientData) 0, STATS_TIME * 1000L,
	1 );
    stats_connections = stats_simultaneous = 0;
#endif /* STATS_TIME */

    /* If we're root, try to become someone else. */
    if ( getuid() == 0 )
	{
	if ( setgroups( 0, (const gid_t*) 0 ) < 0 )
	    {
	    syslog( LOG_CRIT, "setgroups - %m" );
	    exit( 1 );
	    }
	if ( setgid( gid ) < 0 )
	    {
	    syslog( LOG_CRIT, "setgid - %m" );
	    exit( 1 );
	    }
	if ( setuid( uid ) < 0 )
	    {
	    syslog( LOG_CRIT, "setuid - %m" );
	    exit( 1 );
	    }
	/* Check for unnecessary security exposure. */
	if ( ! do_chroot )
	    syslog(
		LOG_CRIT,
		"started as root without requesting chroot(), warning only" );
	}

    /* Initialize our connections table. */
    maxconnects = nfiles - SPARE_FDS;
    connects = NEW( connecttab, maxconnects );
    if ( connects == (connecttab*) 0 )
	{
	syslog( LOG_CRIT, "out of memory" );
	exit( 1 );
	}
    for ( cnum = 0; cnum < maxconnects; ++cnum )
	{
	connects[cnum].conn_state = CNST_FREE;
	connects[cnum].hc.initialized = 0;
	}
    numconnects = 0;
    recompute_fdsets = 1;

    /* Main loop. */
    (void) gettimeofday( &tv, (struct timezone*) 0 );
    for (;;)
	{
	if ( recompute_fdsets )
	    {
	    /* Set up the fdsets. */
	    FD_ZERO( &master_rfdset );
	    FD_ZERO( &master_wfdset );
	    FD_SET( hs->listen_fd, &master_rfdset );
	    if ( numconnects > 0 )
		for ( cnum = 0; cnum < maxconnects; ++cnum )
		    switch ( connects[cnum].conn_state )
			{
			case CNST_READING:
			case CNST_LINGERING:
			FD_SET( connects[cnum].hc.conn_fd, &master_rfdset );
			break;
			case CNST_SENDING:
			FD_SET( connects[cnum].hc.conn_fd, &master_wfdset );
			break;
			}
	    recompute_fdsets = 0;
	    }

	/* Do the select. */
	working_rfdset = master_rfdset;
	working_wfdset = master_wfdset;
	num_ready = select(
	    nfiles, &working_rfdset, &working_wfdset, (fd_set*) 0,
	    tmr_timeout( &tv ) );
	if ( num_ready < 0 )
	    {
	    if ( errno == EINTR )
		continue;	/* try again */
	    syslog( LOG_ERR, "select - %m" );
	    exit( 1 );
	    }
	(void) gettimeofday( &tv, (struct timezone*) 0 );
	if ( num_ready == 0 )
	    {
	    /* No fd's are ready - run the timers. */
	    tmr_run( &tv );
	    continue;
	    }

	/* Is it a new connection? */
	if ( FD_ISSET( hs->listen_fd, &working_rfdset ) )
	    {
	    --num_ready;
	    if ( handle_newconnect( &tv ) )
		/* Go around the loop and do another select, rather than
		** dropping through and processing existing connections.
		** New connections always get priority.
		*/
		continue;
	    /* Only if handle_newconnect() fails in a particular way do
	    ** we fall through.
	    */
	    }

	/* Find the connections that need servicing. */
	for ( cnum = 0; num_ready > 0 && cnum < maxconnects; ++cnum )
	    {
	    c = &connects[cnum];
	    if ( c->conn_state == CNST_READING &&
		 FD_ISSET( c->hc.conn_fd, &working_rfdset ) )
		{
		--num_ready;
		handle_read( c, &tv );
		}
	    else if ( c->conn_state == CNST_SENDING &&
		 FD_ISSET( c->hc.conn_fd, &working_wfdset ) )
		{
		--num_ready;
		handle_send( c, &tv );
		}
	    else if ( c->conn_state == CNST_LINGERING &&
		 FD_ISSET( c->hc.conn_fd, &working_rfdset ) )
		{
		--num_ready;
		handle_linger( c, &tv );
		}
	    }
	tmr_run( &tv );
	}
    }


static void
usage( void )
    {
    (void) fprintf( stderr,
"usage:  %s [-p port] [-d dir] [-r|-nor] [-u user] [-c cgipat] [-t throttles] [-h host] [-l logfile]\n",
	argv0 );
    exit( 1 );
    }


static void
read_throttlefile( char* throttlefile )
    {
    FILE* fp;
    char buf[5000];
    char* cp;
    int len;
    char pattern[5000];
    long limit;
    struct timeval tv;

    fp = fopen( throttlefile, "r" );
    if ( fp == (FILE*) 0 )
	{
	syslog( LOG_CRIT, "%.80s - %m", throttlefile );
	perror( throttlefile );
	exit( 1 );
	}

    (void) gettimeofday( &tv, (struct timezone*) 0 );

    while ( fgets( buf, sizeof(buf), fp ) != (char*) 0 )
	{
	/* Nuke comments. */
	cp = strchr( buf, '#' );
	if ( cp != (char*) 0 )
	    *cp = '\0';

	/* Nuke trailing whitespace. */
	len = strlen( buf );
	while ( len > 0 &&
		( buf[len-1] == ' ' || buf[len-1] == '\t' ||
		  buf[len-1] == '\n' || buf[len-1] == '\r' ) )
	    buf[--len] = '\0';

	/* Ignore empty lines. */
	if ( len == 0 )
	    continue;

	/* Parse line. */
	if ( sscanf( buf, " %[^ \t] %ld", pattern, &limit ) != 2 || limit <= 0 )
	    {
	    syslog( LOG_CRIT,
		"unparsable line in %.80s - %.80s", throttlefile, buf );
	    (void) fprintf( stderr,
		"unparsable line in %.80s - %.80s\n", throttlefile, buf );
	    continue;
	    }

	/* Nuke any leading slashes in pattern. */
	if ( pattern[0] == '/' )
	    (void) strcpy( pattern, &pattern[1] );
	while ( ( cp = strstr( pattern, "|/" ) ) != (char*) 0 )
	    (void) strcpy( cp + 1, cp + 2 );

	/* Check for room in throttles. */
	if ( numthrottles >= maxthrottles )
	    {
	    if ( maxthrottles == 0 )
		{
		maxthrottles = 100;	/* arbitrary */
		throttles = NEW( throttletab, maxthrottles );
		}
	    else
		{
		maxthrottles *= 2;
		throttles = RENEW( throttles, throttletab, maxthrottles );
		}
	    if ( throttles == (throttletab*) 0 )
		{
		syslog( LOG_CRIT, "out of memory" );
		(void) fprintf( stderr, "out of memory\n" );
		exit( 1 );
		}
	    }

	/* Add to table. */
	throttles[numthrottles].pattern = strdup( pattern );
	if ( throttles[numthrottles].pattern == (char*) 0 )
	    {
	    syslog( LOG_CRIT, "out of memory" );
	    (void) fprintf( stderr, "out of memory\n" );
	    exit( 1 );
	    }
	throttles[numthrottles].limit = limit;
	throttles[numthrottles].rate = 0;
	throttles[numthrottles].last_avg = tv.tv_sec;
	throttles[numthrottles].bytes_since_avg = 0;
	throttles[numthrottles].num_sending = 0;

	++numthrottles;
	}
    (void) fclose( fp );
    }


static void
shut_down( void )
    {
    int cnum;
    struct timeval tv;

#ifdef STATS_TIME
    show_stats( (ClientData) 0, (struct timeval*) 0 );
#endif /* STATS_TIME */
    (void) gettimeofday( &tv, (struct timezone*) 0 );
    for ( cnum = 0; cnum < maxconnects; ++cnum )
	{
	if ( connects[cnum].conn_state != CNST_FREE )
	    httpd_close_conn( &connects[cnum].hc, &tv );
	httpd_destroy_conn( &connects[cnum].hc );
	}
    if ( hs != (httpd_server*) 0 )
	{
	httpd_terminate( hs );
	hs = (httpd_server*) 0;
	}
    mmc_destroy();
    tmr_destroy();
    free( (void*) connects );
    }


static int
handle_newconnect( struct timeval* tvP )
    {
    int cnum;
    connecttab* c;
    ClientData client_data;

    /* This loops until the accept() fails, trying to start new
    ** connections as fast as possible so we don't overrun the
    ** listen queue.
    */
    for (;;)
	{
	/* Is there room in the connection table? */
	if ( numconnects >= maxconnects )
	    {
	    /* Out of connection slots.  Run the timers, then the
	    ** existing connections, and maybe we'll free up a slot
	    ** by the time we get back here.
	    **/
	    syslog( LOG_WARNING, "too many connections!" );
	    tmr_run( tvP );
	    return 0;
	    }
	/* Find a free connection entry. */
	for ( cnum = 0; cnum < maxconnects; ++cnum )
	    if ( connects[cnum].conn_state == CNST_FREE )
		break;
	c = &connects[cnum];

	/* Get the connection. */
	switch ( httpd_get_conn( hs, &connects[cnum].hc ) )
	    {
	    case GC_FAIL:
	    case GC_NO_MORE:
	    return 1;
	    }
	c->conn_state = CNST_READING;
	recompute_fdsets = 1;
	++numconnects;
	client_data.p = c;
	c->idle_read_timer = tmr_create(
	    tvP, idle_read_connection, client_data, IDLE_READ_TIMELIMIT * 1000L,
	    0 );
	c->idle_send_timer = (Timer*) 0;
	c->wakeup_timer = (Timer*) 0;
	c->linger_timer = (Timer*) 0;
	c->bytes_sent = 0;

	/* Set the connection file descriptor to no-delay mode. */
	if ( fcntl( c->hc.conn_fd, F_SETFL, O_NDELAY ) < 0 )
	    syslog( LOG_ERR, "fcntl O_NDELAY - %m" );

#ifdef STATS_TIME
	++stats_connections;
	if ( numconnects > stats_simultaneous )
	    stats_simultaneous = numconnects;
#endif /* STATS_TIME */
	}
    }


static void
handle_read( connecttab* c, struct timeval* tvP )
    {
    int sz;
    ClientData client_data;

    /* Is there room in our buffer to read more bytes? */
    if ( c->hc.read_idx >= sizeof(c->hc.read_buf) )
	{
	httpd_send_err( &c->hc, 400, httpd_err400title, httpd_err400form, "" );
	clear_connection( c, tvP );
	return;
	}

    /* Read some more bytes. */
    sz = read(
        c->hc.conn_fd, &(c->hc.read_buf[c->hc.read_idx]),
	sizeof(c->hc.read_buf) - c->hc.read_idx );
    if ( sz <= 0 )
	{
	httpd_send_err( &c->hc, 400, httpd_err400title, httpd_err400form, "" );
	clear_connection( c, tvP );
	return;
	}
    c->hc.read_idx += sz;
    
    /* Do we have a complete request yet? */
    switch ( httpd_got_request( &c->hc ) )
	{
	case GR_NO_REQUEST:
	return;
	case GR_BAD_REQUEST:
	httpd_send_err( &c->hc, 400, httpd_err400title, httpd_err400form, "" );
	clear_connection( c, tvP );
	return;
	}
    
    /* Yes.  Try parsing it. */
    if ( httpd_parse_request( &c->hc ) < 0 )
	{
	clear_connection( c, tvP );
	return;
	}

    /* Check the throttle table */
    if ( ! check_throttles( c ) )
	{
	httpd_send_err(
	    &c->hc, 503, httpd_err503title, httpd_err503form,
	    c->hc.encodedurl );
	clear_connection( c, tvP );
	return;
	}

    /* Start the connection going. */
    if ( httpd_start_request( &c->hc ) < 0 )
	{
	/* Something went wrong.  Close down the connection. */
	clear_connection( c, tvP );
	return;
	}

    /* Fill in bytes_to_send. */
    if ( c->hc.got_range )
	{
	c->bytes_sent = c->hc.init_byte_loc;
	c->bytes_to_send = c->hc.end_byte_loc + 1;
	}
    else
	c->bytes_to_send = c->hc.bytes;

    /* Check if it's already handled. */
    if ( c->hc.file_address == (char*) 0 )
	{
	/* No file address means someone else is handling it. */
	c->bytes_sent = c->hc.bytes;
	clear_connection( c, tvP );
	return;
	}
    if ( c->bytes_sent >= c->bytes_to_send )
	{
	/* There's nothing to send. */
	clear_connection( c, tvP );
	return;
	}

    /* Cool, we have a valid connection and a file to send to it. */
    c->conn_state = CNST_SENDING;
    recompute_fdsets = 1;
    c->started_at = tvP->tv_sec;
    c->wouldblock_delay = 0;
    client_data.p = c;
    tmr_cancel( c->idle_read_timer );
    c->idle_read_timer = (Timer*) 0;
    c->idle_send_timer = tmr_create(
	tvP, idle_send_connection, client_data, IDLE_SEND_TIMELIMIT * 1000L,
	0 );
    }


static void
handle_send( connecttab* c, struct timeval* tvP )
    {
    int sz, coast;
    ClientData client_data;
    time_t elapsed;

    /* Do we need to write the headers first? */
    if ( c->hc.responselen == 0 )
	{
	/* No, just write the file. */
	sz = write(
	    c->hc.conn_fd, &c->hc.file_address[c->bytes_sent],
	    MIN( c->bytes_to_send - c->bytes_sent, c->limit ) );
	}
    else
	{
	/* Yes.  We'll combine headers and file into a single writev(),
	** hoping that this generates a single packet.
	*/
	struct iovec iv[2];

	iv[0].iov_base = c->hc.response;
	iv[0].iov_len = c->hc.responselen;
	iv[1].iov_base = &c->hc.file_address[c->bytes_sent];
	iv[1].iov_len = MIN( c->bytes_to_send - c->bytes_sent, c->limit );
	sz = writev( c->hc.conn_fd, iv, 2 );
	}

    if ( sz == 0 ||
	 ( sz < 0 && ( errno == EWOULDBLOCK || errno == EAGAIN ) ) )
	{
	/* This shouldn't happen, but some kernels, e.g.
	** SunOS 4.1.x, are broken and select() says that
	** O_NDELAY sockets are always writable even when
	** they're actually not.
	**
	** Current workaround is to block sending on this
	** socket for a brief adaptively-tuned period.
	** Fortunately we already have all the necessary
	** blocking code, for use with throttling.
	*/
	c->wouldblock_delay += MIN_WOULDBLOCK_DELAY;
	c->conn_state = CNST_PAUSING;
	recompute_fdsets = 1;
	client_data.p = c;
	c->wakeup_timer = tmr_create(
	    tvP, wakeup_connection, client_data, c->wouldblock_delay, 0 );
	return;
	}
    if ( sz < 0 )
	{
	/* Something went wrong, close this connection.
	**
	** If it's just an EPIPE, don't bother logging, that
	** just means the client hung up on us.
	**
	** On some systems, write() occasionally gives an EINVAL.
	** Dunno why, something to do with the socket going
	** bad.  Anyway, we don't log those either.
	*/
	if ( errno != EPIPE && errno != EINVAL )
	    syslog( LOG_ERR, "write - %m" );
	clear_connection( c, tvP );
	return;
	}

    /* Ok, we wrote something. */
    tmr_reset( tvP, c->idle_send_timer );
    /* Was this a headers + file writev()? */
    if ( c->hc.responselen > 0 )
	{
	/* Yes; did we write only part of the headers? */
	if ( sz < c->hc.responselen )
	    {
	    /* Yes; move the unwritten part to the front of the buffer. */
	    int newlen = c->hc.responselen - sz;
	    (void) memcpy( c->hc.response, &(c->hc.response[sz]), newlen );
	    c->hc.responselen = newlen;
	    sz = 0;
	    }
	else
	    {
	    /* Nope, we wrote the full headers, so adjust accordingly. */
	    sz -= c->hc.responselen;
	    c->hc.responselen = 0;
	    }
	}
    /* And update how much of the file we wrote. */
    c->bytes_sent += sz;

    /* Are we done? */
    if ( c->bytes_sent >= c->bytes_to_send )
	{
	/* This conection is finished! */
	clear_connection( c, tvP );
	return;
	}

    /* Tune the (blockheaded) wouldblock delay. */
    if ( c->wouldblock_delay > MIN_WOULDBLOCK_DELAY )
	c->wouldblock_delay -= MIN_WOULDBLOCK_DELAY;

    /* Check if we're sending faster than the throttle. */
    elapsed = tvP->tv_sec - c->started_at;
    if ( elapsed == 0 || c->bytes_sent / elapsed > c->limit )
	{
	c->conn_state = CNST_PAUSING;
	recompute_fdsets = 1;
	/* When should we send the next c->limit bytes
	** to get back on schedule?  If less than a second
	** (integer math rounding), use 1/8 second.
	*/
	coast = ( c->bytes_sent + c->limit ) / c->limit - elapsed;
	client_data.p = c;
	c->wakeup_timer = tmr_create(
	    tvP, wakeup_connection, client_data,
	    coast ? ( coast * 1000L ) : 125L, 0 );
	}
    }


static void
handle_linger( connecttab* c, struct timeval* tvP )
    {
    char buf[1024];
    int r;

    /* In lingering-close mode we just read and ignore bytes.  An error
    ** or EOF ends things, otherwise we go until a timeout.
    */
    r = read( c->hc.conn_fd, buf, sizeof(buf) );
    if ( r <= 0 )
	really_clear_connection( c, tvP );
    }


static int
check_throttles( connecttab* c )
    {
    int tnum;

    c->numtnums = 0;
    c->limit = 1234567890L;
    for ( tnum = 0; tnum < numthrottles && c->numtnums < MAXTHROTTLENUMS;
	  ++tnum )
	if ( match( throttles[tnum].pattern, c->hc.expnfilename ) )
	    {
	    c->tnums[c->numtnums++] = tnum;
	    /* If we're way over the limit, don't even start. */
	    if ( throttles[tnum].rate > throttles[tnum].limit * 3 / 2 )
		return 0;
	    ++throttles[tnum].num_sending;
	    c->limit = MIN(
		c->limit, throttles[tnum].limit / throttles[tnum].num_sending );
	    }
    return 1;
    }


static void
clear_throttles( connecttab* c, struct timeval* tvP )
    {
    int i, tnum;
    time_t elapsed;

    for ( i = 0; i < c->numtnums; ++i )
	{
	tnum = c->tnums[i];
	--throttles[tnum].num_sending;
	throttles[tnum].bytes_since_avg += c->bytes_sent;
	elapsed = tvP->tv_sec - throttles[tnum].last_avg;
	if ( elapsed >= THROTTLE_TIME )
	    {
	    throttles[tnum].rate =
		( throttles[tnum].rate +
		  throttles[tnum].bytes_since_avg / elapsed ) / 2;
	    throttles[tnum].bytes_since_avg = 0;
	    }
	}
    }


static void
clear_connection( connecttab* c, struct timeval* tvP )
    {
    ClientData client_data;

    /* If we haven't actually sent the buffered response yet, do so now. */
    httpd_write_response( &c->hc );

    if ( c->idle_read_timer != (Timer*) 0 )
	{
	tmr_cancel( c->idle_read_timer );
	c->idle_read_timer = 0;
	}
    if ( c->idle_send_timer != (Timer*) 0 )
	{
	tmr_cancel( c->idle_send_timer );
	c->idle_send_timer = 0;
	}
    if ( c->wakeup_timer != (Timer*) 0 )
	{
	tmr_cancel( c->wakeup_timer );
	c->wakeup_timer = 0;
	}

    /* This is our version of Apache's lingering_close() routine, which is
    ** their version of the often-broken SO_LINGER socket option.  For why
    ** this is necessary, see http://www.apache.org/docs/misc/fin_wait_2.html
    ** What we do is delay the actual closing for a few seconds, while reading
    ** any bytes that come over the connection.  However, we don't want to do
    ** this unless it's necessary, because it ties up a connection slot and
    ** file descriptor which means our maximum connection-handling rate
    ** is lower.  So, elsewhere we set a flag when we detect the few
    ** circumstances that make a lingering close necessary.  If the flag
    ** isn't set we do the real close now.
    */
    if ( c->hc.should_linger )
	{
	c->conn_state = CNST_LINGERING;
	recompute_fdsets = 1;
	client_data.p = c;
	c->linger_timer = tmr_create(
	    tvP, linger_clear_connection, client_data, LINGER_TIME * 1000L, 0 );
	}
    else
	really_clear_connection( c, tvP );
    }


static void
really_clear_connection( connecttab* c, struct timeval* tvP )
    {
    httpd_close_conn( &c->hc, tvP );
    clear_throttles( c, tvP );
    if ( c->linger_timer != (Timer*) 0 )
	{
	tmr_cancel( c->linger_timer );
	c->linger_timer = 0;
	}
    c->conn_state = CNST_FREE;
    recompute_fdsets = 1;
    --numconnects;
    }


static void
idle_read_connection( ClientData client_data, struct timeval* nowP )
    {
    connecttab* c;

    c = (connecttab*) client_data.p;
    c->idle_read_timer = (Timer*) 0;
    if ( c->conn_state != CNST_FREE )
	{
	syslog( LOG_NOTICE,
	    "%.80s connection timed out reading",
	    inet_ntoa( c->hc.client_addr ) );
	httpd_send_err( &c->hc, 408, httpd_err408title, httpd_err408form, "" );
	clear_connection( c, nowP );
	}
    }


static void
idle_send_connection( ClientData client_data, struct timeval* nowP )
    {
    connecttab* c;

    c = (connecttab*) client_data.p;
    c->idle_send_timer = (Timer*) 0;
    if ( c->conn_state != CNST_FREE )
	{
	syslog( LOG_NOTICE,
	    "%.80s connection timed out sending",
	    inet_ntoa( c->hc.client_addr ) );
	clear_connection( c, nowP );
	}
    }


static void
wakeup_connection( ClientData client_data, struct timeval* nowP )
    {
    connecttab* c;

    c = (connecttab*) client_data.p;
    c->wakeup_timer = (Timer*) 0;
    if ( c->conn_state == CNST_PAUSING )
	{
	c->conn_state = CNST_SENDING;
	recompute_fdsets = 1;
	}
    }

static void
linger_clear_connection( ClientData client_data, struct timeval* nowP )
    {
    connecttab* c;

    c = (connecttab*) client_data.p;
    c->linger_timer = (Timer*) 0;
    really_clear_connection( c, nowP );
    }


static void
occasional( ClientData client_data, struct timeval* nowP )
    {
    mmc_cleanup( nowP );
    tmr_cleanup();
    }


#ifdef STATS_TIME
static void
show_stats( ClientData client_data, struct timeval* nowP )
    {
    int am, fm, at, ft;

    mmc_stats( &am, &fm );
    tmr_stats( &at, &ft );
    syslog( LOG_INFO,
	"%d seconds, %d connections, %d simultaneous, %d maps, %d timers",
	STATS_TIME, stats_connections, stats_simultaneous, am, at );
    stats_connections = stats_simultaneous = 0;
    }
#endif /* STATS_TIME */
