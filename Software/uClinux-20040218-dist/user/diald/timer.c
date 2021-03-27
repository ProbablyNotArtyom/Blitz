/*
 * timer.c - This emulates the kernel timer routines in user space.
 *           Ugly, but it works.
 *
 * Copyright (c) 1994, 1995, 1996 Eric Schenk.
 * All rights reserved. Please see the file LICENSE which should be
 * distributed with this software for terms of use.
 */

#include "diald.h"

#include <signal.h>
#include <sys/time.h>
#include <sys/times.h>
#if !defined(__UC_LIBC__)
#include <unistd.h>
#endif

static struct timer_lst head = {&head,&head,0,0,0,0};
static int in_alarm = 0;

static int block_level = 0;

/*
 * The following gets a time stamp based on the number of system
 * clock ticks since the system has been up.
 * Note that this measure of time is immune to changes in the
 * wall clock setting, and so we don't have to worry about the
 * wall clock getting mucked with.
 */

unsigned long timestamp()
{
   struct tms buf;
   return times(&buf)/CLK_TCK;
}

void init_timer(struct timer_lst * timer)
{
    timer->next = NULL;
    timer->prev = NULL;
}


/* These are needed in user space because we can't control when
 * context switches happen
 */

void block_timer()
{
    sigset_t mask;
    if (block_level) {
	block_level++;
	return;
    }
    block_level++;
    if (!in_alarm) {
        sigemptyset(&mask);
        sigaddset(&mask,SIGALRM);
        sigprocmask(SIG_BLOCK, &mask,NULL);
    }
}

void unblock_timer()
{
    sigset_t mask;
    block_level--;
    if (block_level) return;
    if (!in_alarm) {
        sigemptyset(&mask);
        sigaddset(&mask,SIGALRM);
        sigprocmask(SIG_UNBLOCK, &mask,NULL);
    }
}

/*
 * Basic idea: store time outs in order.
 * I'd be happier with a basic priority queue of some kind,
 * but this was the most direct way to code it, and it
 * matched the original kernel type definition I was using.
 */

void add_timer(struct timer_lst *timer)
{
    struct timer_lst *c;
    struct itimerval itime;
    unsigned long atime;
    block_timer();
    atime = timestamp();
    timer->expected = atime+timer->expires;
    c = head.next;
    /* march down the list looking for a home */
    while (c != &head) {
	if (timer->expected < c->expected) break;
	c = c->next;
    }

    timer->next = c;
    c->prev->next = timer;
    timer->prev = c->prev;
    c->prev = timer;
    if (head.next == timer) {
	/* Timer gets replaced by a new value */
	itime.it_interval.tv_sec = 0;
	itime.it_interval.tv_usec = 0;
	itime.it_value.tv_sec = head.next->expected-atime;
	if (itime.it_value.tv_sec <= 0) {
	    itime.it_value.tv_sec = 0;
	    itime.it_value.tv_usec = 1;	/* time out right away */
	} else
	    itime.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &itime, 0);
    }
    unblock_timer();
}

int del_timer(struct timer_lst *timer)
{
    unsigned long atime;
    block_timer();
    atime = timestamp();
    /* return 0 if timer was not active */
    if (!timer->next) { unblock_timer(); return 0; }
    timer->next->prev = timer->prev;
    timer->prev->next = timer->next;
    if (head.next == timer->next) {
	struct itimerval itime;
	itime.it_interval.tv_sec = 0;
	itime.it_interval.tv_usec = 0;
	if (head.next == &head) {
	    /* remove the timeout */
	    itime.it_value.tv_sec = 0;
	    itime.it_value.tv_usec = 0;
	} else {
	    /* fix the timeout */
	    itime.it_value.tv_sec = head.next->expected-atime;
	    if (itime.it_value.tv_sec <= 0) {
	    	itime.it_value.tv_sec = 0;
	        itime.it_value.tv_usec = 1;	/* time out right away */
	    } else
	        itime.it_value.tv_usec = 0;
	}
	setitimer(ITIMER_REAL, &itime, 0);
    }
    timer->next = timer->prev = NULL;
    unblock_timer();
    return 1;
}

void alrm_timer(int sig)
{
    struct itimerval itime;
    struct timer_lst *cn;
    struct timer_lst *c = head.next;
    unsigned long atime = timestamp();
    unsigned long now;

    /* process the functions at the head of the list, then reset the timer */
    if (c == &head) return;
    now = c->expected;
    if (now <= atime) {
	do {
	    c->next->prev = c->prev;
	    c->prev->next = c->next;
	    cn = c->next;
	    c->next = c->prev = NULL;
	    /* process the data, this may free the timer entry,
	     * so we can't refer to the contents of c again.
	     */
	    in_alarm = 1; /* just in case the processing adds a timer */
	    (*c->function)(c->data);
	    in_alarm = 0;
	    c = cn;
	} while (c != &head && c->expected == now);
    } else {
	syslog(LOG_INFO,"Alarm was %ld seconds early",c->expected-atime);
    }

    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_usec = 0;
    if (head.next == &head) {
	/* remove the timeout */
	itime.it_value.tv_sec = 0;
	itime.it_value.tv_usec = 0;
    } else {
	/* fix the timeout */
	itime.it_value.tv_sec = head.next->expected-atime;
	if (itime.it_value.tv_sec == 0)
	    itime.it_value.tv_usec = 1; /* time out right away */
	else
	    itime.it_value.tv_usec = 0;
    }
    setitimer(ITIMER_REAL, &itime, 0);
}

int next_alarm()
{
    struct itimerval itime;
    getitimer(ITIMER_REAL, &itime);
    return itime.it_value.tv_sec;
}

void sleep_wakeup(int *var)
{
    *var = 0;
}

/* We can't use the libc sleep together with itimers. */
#ifdef __UC_LIBC__
int sleep(unsigned int secs)
#else
unsigned int sleep(unsigned int secs)
#endif
{
    static int asleep;
    struct timer_lst sleept;
    init_timer(&sleept);
    sleept.data = (int)&asleep;
    sleept.function = (void *)(unsigned long)sleep_wakeup;
    sleept.expires = secs;
    add_timer(&sleept);
    asleep = 1;
    while (asleep)
	pause();
    return 0;
}

#ifdef EMBED
int realsleep(unsigned int sec)
{
        struct timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = 0;
        select(0,0,0,0, &tv);
        return tv.tv_sec;
}
#endif
