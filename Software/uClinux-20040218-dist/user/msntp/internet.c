/*  Copyright (C) 1996 N.M. Maclaren
    Copyright (C) 1996 The University of Cambridge

This includes all of the code needed to handle Internet addressing.  It is way
outside current POSIX, unfortunately.  It should be easy to convert to a system
that uses another mechanism.  The signal handling is not necessary for its
function, but is an attempt to avoid the program hanging when the name server
is inaccessible. */



#include "header.h"
#include "internet.h"

#include <netdb.h>
#include <arpa/inet.h>

#define INTERNET
#include "kludges.h"
#undef INTERNET



/* There needs to be some disgusting grobble for handling timeouts, which is
identical to the grobble in socket.c. */

static jmp_buf jump_buffer;

static void jump_handler (int sig) {
    longjmp(jump_buffer,1);
}

static void clear_alarm (void) {
    int k;

    k = errno;
    alarm(0);
    errno = 0;
    if (signal(SIGALRM,SIG_DFL) == SIG_ERR)
        fatal(1,"unable to reset signal handler",NULL);
    errno = k;
}



void find_address (struct in_addr *address, struct in_addr *anywhere,
    struct in_addr *everywhere, int *port, char *hostname, int timespan) {

/* Locate the specified NTP server and return its Internet address and port 
number. */

    struct in_addr nowhere[1];
    struct hostent *host;
    struct servent *service;

/* Set up the reserved Internet addresses, attempting not to assume that
addresses are 32 bits. */

    local_to_address(nowhere,INADDR_LOOPBACK);
    local_to_address(anywhere,INADDR_ANY);
    local_to_address(everywhere,INADDR_BROADCAST);

/* Check the address, if any.  This assumes that the DNS is reliable, or is at
least checked by someone else.  But it doesn't assume that it is accessible, so
it needs to set up a timeout. */

    if (hostname == NULL)
        *address = *anywhere;
    else {
        if (setjmp(jump_buffer))
            fatal(0,"unable to set up access to NTP server %s",hostname);
        errno = 0;
        if (signal(SIGALRM,jump_handler) == SIG_ERR)
            fatal(1,"unable to set up signal handler",NULL);
        alarm((unsigned int)timespan);

/* Look up the Internet name or IP number. */

        host = gethostbyname(hostname);

/* Now clear the timer and check the result. */

        clear_alarm();
        if (host == NULL) fatal(1,"unable to locate IP address/number",NULL);
        if (host->h_length != sizeof(struct in_addr))
            fatal(0,"the address does not seem to be an Internet one",NULL);
        *address = *((struct in_addr **)host->h_addr_list)[0];
        if (memcmp(address,nowhere,sizeof(struct in_addr)) == 0 ||
                memcmp(address,anywhere,sizeof(struct in_addr)) == 0 ||
                memcmp(address,everywhere,sizeof(struct in_addr)) == 0)
            fatal(0,"reserved IP numbers cannot be used",NULL);
        if (verbose)
            fprintf(stderr,
                "%s: using NTP server %s (%s)\n",
                argv0,host->h_name,inet_ntoa(*address));
    }

/* Find out the port number (usually from /etc/services), and leave it in 
network format.  This is assumed not to be obtained from a network service!
Note that a port number is not assumed to be 16 bits. */

    if ((service = getservbyname("ntp","udp")) != NULL) {
        *port = service->s_port;
        if (verbose > 2)
            fprintf(stderr,"Using port %d for NTP\n",port_to_integer(*port));
    } else {
        *port = NTP_PORT;
        if (verbose)
            fprintf(stderr,
                "%s: assuming port %d for NTP - check /etc/services\n",
                argv0,port_to_integer(*port));
    }
}
