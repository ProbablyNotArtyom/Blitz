/* libhttpd.h - defines for libhttpd
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

#ifndef _LIBHTTPD_H_
#define _LIBHTTPD_H_

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


/* A few convenient defines. */

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#define NEW(t,n) ((t*) malloc( sizeof(t) * (n) ))
#define RENEW(o,t,n) ((t*) realloc( (void*) o, sizeof(t) * (n) ))


/* The httpd structs. */

typedef struct {
    char* hostname;
    struct in_addr host_addr;
    int port;
    char* cgi_pattern;
    char* cwd;
    int listen_fd;
    FILE* logfp;
    int chrooted;
    } httpd_server;

typedef struct {
    int initialized;
    httpd_server* hs;
    struct in_addr client_addr;
    char read_buf[4096];
    int read_idx, checked_idx;
    int checked_state;
    int method;
    int status;
    off_t bytes;
    char* encodedurl;
    char* decodedurl;
    char* protocol;
    char* origfilename;
    char* expnfilename;
    char* encodings;
    char* pathinfo;
    char* query;
    char* altdir;
    char* referer;
    char* useragent;
    char* accept;
    char* accepte;
    char* cookie;
    char* contenttype;
    char* reqhost;
    char* hdrhost;
    char* authorization;
    char* remoteuser;
    char* response;
    int maxdecodedurl, maxorigfilename, maxexpnfilename, maxencodings,
        maxpathinfo, maxquery, maxaltdir, maxaccept, maxaccepte, maxreqhost,
	maxremoteuser, maxresponse;
    int responselen;
    time_t if_modified_since, range_if;
    off_t contentlength;
    char* type;		/* not malloc()ed */
    int mime_flag;
    int one_one;	/* HTTP/1.1 or better */
    int got_range;
    off_t init_byte_loc, end_byte_loc;
    int keep_alive;
    int should_linger;
    struct stat sb;
    int conn_fd;
    char* file_address;
    } httpd_conn;

/* Methods. */
#define METHOD_GET 1
#define METHOD_HEAD 2
#define METHOD_POST 3

/* States for checked_state. */
#define CHST_FIRSTWORD 0
#define CHST_FIRSTWS 1
#define CHST_SECONDWORD 2
#define CHST_SECONDWS 3
#define CHST_THIRDWORD 4
#define CHST_LINE 5
#define CHST_LF 6
#define CHST_CR 7
#define CHST_CRLF 8
#define CHST_CRLFCR 9
#define CHST_BOGUS 10


/* Initializes.  Does the socket(), bind(), and listen().   Returns an
** httpd_server* which includes a socket fd that you can select() on.
** Return (httpd_server*) 0 on error.
*/
extern httpd_server* httpd_initialize(
    char* hostname, u_int addr, int port, char* cgi_pattern, char* cwd,
    FILE* logfp, int chrooted );

/* Call to shut down. */
extern void httpd_terminate( httpd_server* hs );


/* When the socket fd is ready to read, call this.  It does the accept() and
** returns an httpd_conn* which includes the fd to read the request from and
** write the response to.  Returns an indication of whether the accept()
** failed, succeeded, or if there were no more connections to accept.
**
** In order to minimize malloc()s, the caller passes in the httpd_conn.
** The caller is also responsible for setting initialized to zero before the
** first call using each different httpd_conn.
*/
extern int httpd_get_conn( httpd_server* hs, httpd_conn* hc );
#define GC_FAIL 0
#define GC_OK 1
#define GC_NO_MORE 2

/* Checks whether the data in hc->read_buf constitutes a complete request
** yet.  The caller reads data into hc->read_buf[hc->read_idx] and advances
** hc->read_idx.  This routine checks what has been read so far, using
** hc->checked_idx and hc->checked_state to keep track, and returns an
** indication of whether there is no complete request yet, there is a
** complete request, or there won't be a valid request due to a syntax error.
*/
extern int httpd_got_request( httpd_conn* hc );
#define GR_NO_REQUEST 0
#define GR_GOT_REQUEST 1
#define GR_BAD_REQUEST 2

/* Parses the request in hc->read_buf.  Fills in lots of fields in hc,
** like the URL and the various headers.
**
** Returns -1 on error.
*/
extern int httpd_parse_request( httpd_conn* hc );

/* Starts sending data back to the client.  In some cases (directories,
** CGI programs), finishes sending by itself - in those cases, hc->file_fd
** is <0.  If there is more data to be sent, then hc->file_fd is a file
** descriptor for the file to send.
**
** Returns -1 on error.
*/
extern int httpd_start_request( httpd_conn* hc );

/* Actually sends any buffered response text. */
extern void httpd_write_response( httpd_conn* hc );

/* Call this to close down a connection and free the data.  A fine point,
** if you fork() with a connection open you should still call this in the
** parent process - the connection will stay open in the child.
** If you don't have a current timeval handy just pass in 0.
*/
extern void httpd_close_conn( httpd_conn* hc, struct timeval* nowP );

/* Call this to de-initialize a connection struct and *really* free the
** mallocced strings.
*/
extern void httpd_destroy_conn( httpd_conn* hc );


/* Send an error message back to the client. */
extern void httpd_send_err(
    httpd_conn* hc, int status, char* title, char* form, char* arg );

/* Some error messages. */
extern char* httpd_err400title;
extern char* httpd_err400form;
extern char* httpd_err408title;
extern char* httpd_err408form;
extern char* httpd_err503title;
extern char* httpd_err503form;

/* Generates a string representation of a method number. */
extern char* httpd_method_str( int method );

/* Sets the allowed number of file descriptors to the maximum and returns it. */
extern int httpd_get_nfiles( void );

#endif /* _LIBHTTPD_H_ */
