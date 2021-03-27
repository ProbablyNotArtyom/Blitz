/* timers.c - simple timer routines
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

#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>

#include "timers.h"

/* It might make sense to store the timers list sorted.  It would
** depend on things like how often tmr_timeout() gets called during the
** average life of a timer.
*/


static Timer* timers = (Timer*) 0;
static Timer* free_timers = (Timer*) 0;


Timer*
tmr_create(
    struct timeval* nowP, TimerProc* timer_proc, ClientData client_data,
    long msecs, int periodic )
    {
    Timer* t;

    if ( free_timers != (Timer*) 0 )
	{
	t = free_timers;
	free_timers = t->next;
	}
    else
	{
	t = (Timer*) malloc( sizeof(Timer) );
	if ( t == (Timer*) 0 )
	    return (Timer*) 0;
	}
    t->timer_proc = timer_proc;
    t->client_data = client_data;
    t->msecs = msecs;
    t->periodic = periodic;
    if ( nowP != (struct timeval*) 0 )
	t->time = *nowP;
    else
	(void) gettimeofday( &t->time, (struct timezone*) 0 );
    t->time.tv_sec += msecs / 1000L;
    t->time.tv_usec += ( msecs % 1000L ) * 1000L;
    if ( t->time.tv_usec >= 1000000L )
	{
	t->time.tv_sec += t->time.tv_usec / 1000000L;
	t->time.tv_usec %= 1000000L;
	}
    t->next = timers;
    timers = t;
    return t;
    }


struct timeval*
tmr_timeout( struct timeval* nowP )
    {
    int gotone;
    long msecs, m;
    Timer* t;
    static struct timeval timeout;

    gotone = 0;
    msecs = 0;		/* make lint happy */
    for ( t = timers; t != (Timer*) 0; t = t->next )
	{
	m = ( t->time.tv_sec - nowP->tv_sec ) * 1000L +
	    ( t->time.tv_usec - nowP->tv_usec ) / 1000L;
	if ( ! gotone )
	    {
	    msecs = m;
	    gotone = 1;
	    }
	else if ( m < msecs )
	    msecs = m;
	}
    if ( ! gotone )
	return (struct timeval*) 0;
    if ( msecs <= 0 )
	msecs = 0;
    timeout.tv_sec = msecs / 1000L;
    timeout.tv_usec = ( msecs % 1000L ) * 1000L;
    return &timeout;
    }


void
tmr_run( struct timeval* nowP )
    {
    Timer* t;
    Timer* next;

    for ( t = timers; t != (Timer*) 0; t = next )
	{
	next = t->next;
	if ( t->time.tv_sec < nowP->tv_sec ||
	     ( t->time.tv_sec == nowP->tv_sec &&
	       t->time.tv_usec < nowP->tv_usec ) )
	    {
	    (t->timer_proc)( t->client_data, nowP );
	    if ( t->periodic )
		{
		/* Reschedule. */
		t->time.tv_sec += t->msecs / 1000L;
		t->time.tv_usec += ( t->msecs % 1000L ) * 1000L;
		if ( t->time.tv_usec >= 1000000L )
		    {
		    t->time.tv_sec += t->time.tv_usec / 1000000L;
		    t->time.tv_usec %= 1000000L;
		    }
		}
	    else
		tmr_cancel( t );
	    }
	}
    }


void
tmr_reset( struct timeval* nowP, Timer* t )
    {
    t->time = *nowP;
    t->time.tv_sec += t->msecs / 1000L;
    t->time.tv_usec += ( t->msecs % 1000L ) * 1000L;
    if ( t->time.tv_usec >= 1000000L )
	{
	t->time.tv_sec += t->time.tv_usec / 1000000L;
	t->time.tv_usec %= 1000000L;
	}
    }


void
tmr_cancel( Timer* t )
    {
    Timer** tt;

    for ( tt = &timers; *tt != (Timer*) 0; tt = &(*tt)->next )
	{
	if ( *tt == t )
	    {
	    *tt = t->next;
	    t->next = free_timers;
	    free_timers = t;
	    return;
	    }
	}
    /* Didn't find it.  Shrug. */
    }


void
tmr_cleanup( void )
    {
    Timer* t;

    while ( free_timers != (Timer*) 0 )
	{
	t = free_timers;
	free_timers = t->next;
	free( (void*) t );
	}
    }


void
tmr_destroy( void )
    {
    while ( timers != (Timer*) 0 )
	tmr_cancel( timers );
    tmr_cleanup();
    }


void
tmr_stats( int* activeP, int* freeP )
    {
    Timer* t;

    for ( *activeP = 0, t = timers; t != (Timer*) 0; ++*activeP, t = t->next )
	;
    for ( *freeP = 0, t = free_timers; t != (Timer*) 0; ++*freeP, t = t->next )
	;
    }
