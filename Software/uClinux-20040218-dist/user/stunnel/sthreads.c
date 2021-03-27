/*
 *   stunnel       Universal SSL tunnel
 *   Copyright (c) 1998-2001 Michal Trojnara <Michal.Trojnara@mirt.net>
 *                 All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "common.h"
#include <stdio.h>

#ifdef HAVE_OPENSSL
#include <openssl/crypto.h> /* for CRYPTO_* */
#else
#include <crypto.h> /* for CRYPTO_* */
#endif

#define MAX_CRIT_SECTIONS 3

#ifdef USE_PTHREAD

#include <pthread.h>
#include <unistd.h> /* for getpid() */
#include <signal.h> /* for sigemptyset(), sigaddset() */

pthread_mutex_t stunnel_cs[MAX_CRIT_SECTIONS];

pthread_mutex_t lock_cs[CRYPTO_NUM_LOCKS];
pthread_attr_t pth_attr;

void enter_critical_section(int i) {
    pthread_mutex_lock(stunnel_cs+i);
}

void leave_critical_section(int i) {
    pthread_mutex_unlock(stunnel_cs+i);
}

static void locking_callback(int mode, int type,
#ifdef HAVE_OPENSSL
    const /* Callback definition has been changed in openssl 0.9.3 */
#endif
    char *file, int line)
{
    if(mode&CRYPTO_LOCK)
        pthread_mutex_lock(lock_cs+type);
    else
        pthread_mutex_unlock(lock_cs+type);
}

void sthreads_init() {
    int i;

    /* Initialize stunnel critical sections */
    for(i=0; i<MAX_CRIT_SECTIONS; i++)
        pthread_mutex_init(stunnel_cs+i, NULL);

    /* Initialize OpenSSL locking callback */
    for(i=0; i<CRYPTO_NUM_LOCKS; i++)
        pthread_mutex_init(lock_cs+i, NULL);
    CRYPTO_set_id_callback(thread_id);
    CRYPTO_set_locking_callback(locking_callback);

    pthread_attr_init(&pth_attr);
    pthread_attr_setdetachstate(&pth_attr, PTHREAD_CREATE_DETACHED);
}

unsigned long process_id() {
    return (unsigned long)getpid();
}

unsigned long thread_id() {
    return (unsigned long)pthread_self();
}

int create_client(int ls, int s, void (*cli)(int)) {
     pthread_t thread;
     sigset_t mask, oldmask;

     sigemptyset(&mask);
     sigaddset(&mask, SIGCHLD);
     pthread_sigmask(SIG_BLOCK, &mask, &oldmask); /* block SIGCHLD */
     if(pthread_create(&thread, &pth_attr, (void *)cli, (void *)s)) {
         /* SIGCHLD will remain blocked here */
         closesocket(s);
         return -1;
     }
     pthread_sigmask(SIG_SETMASK, &oldmask, NULL); /* restore the mask */
     return 0;
}

#endif

#ifdef USE_WIN32

#include <windows.h>

CRITICAL_SECTION stunnel_cs[MAX_CRIT_SECTIONS];

void enter_critical_section(int i) {
    EnterCriticalSection(stunnel_cs+i);
}

void leave_critical_section(int i) {
    LeaveCriticalSection(stunnel_cs+i);
}

void sthreads_init() {
    int i;

    /* Initialize stunnel critical sections */
    for(i=0; i<MAX_CRIT_SECTIONS; i++)
        InitializeCriticalSection(stunnel_cs+i);
}

unsigned long process_id() {
    return GetCurrentProcessId();
}

unsigned long thread_id() {
    return GetCurrentThreadId();
}

int create_client(int ls, int s, void (*cli)(int)) {
    DWORD iID;

    CloseHandle(CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cli,
        (void *)s, 0, &iID));
    return 0;
}

#endif

#ifdef USE_FORK

#include <unistd.h> /* for getpid() */
#include <signal.h> /* for signal() */

void enter_critical_section(int i) {
    /* empty */
}

void leave_critical_section(int i) {
    /* empty */
}

void sthreads_init() {
    /* empty */
}

unsigned long process_id() {
    return (unsigned long)getpid();
}

unsigned long thread_id() {
    return 0L;
}

int create_client(int ls, int s, void (*cli)(int)) {
    switch(fork()) {
    case -1:    /* error */
        closesocket(s);
        return -1;
    case  0:    /* child */
        closesocket(ls);
        signal(SIGCHLD, SIG_IGN);
        cli(s);
        exit(0);
    default:    /* parent */
        closesocket(s);
    }
    return 0;
}

#endif
