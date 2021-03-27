/* mmc.c - mmap cache
**
** Copyright (C)1998 by Jef Poskanzer <jef@acme.com>. All rights reserved.
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
#include <sys/stat.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif /* HAVE_MMAP */

#include "mmc.h"


/* The Map struct. */
typedef struct MapStruct {
    ino_t ino;
    dev_t dev;
    off_t size;
    time_t mtime; 
    int refcount;
    time_t reftime; 
    void* addr;
    int hash;
    int hash_idx;
    struct MapStruct* next;
    } Map;


/* Globals. */
static Map* maps = (Map*) 0;
static int map_count = 0;
static Map* free_maps = (Map*) 0;
static int free_count = 0;
static Map** hash_table = (Map**) 0;
static int hash_size;


/* Defines. */
#ifndef EXPIRE_AGE
#define EXPIRE_AGE 600
#endif
#ifndef DESIRED_FREE_COUNT
#define DESIRED_FREE_COUNT 100
#endif
#ifndef INITIAL_HASH_SIZE
#define INITIAL_HASH_SIZE 1009
#endif


/* Forwards. */
static void really_unmap( Map** mm );
static int check_hash_size( void );
static int is_prime( int n );
static int add_hash( Map* m );
static Map* find_hash( ino_t ino, dev_t dev, off_t size, time_t mtime );
static int hash( ino_t ino, dev_t dev, off_t size, time_t mtime );


void*
mmc_map( char* filename, struct stat* sbP )
    {
    struct stat sb;
    Map* m;
    int fd;

    /* Stat the file if necessary. */
    if ( sbP != (struct stat*) 0 )
	sb = *sbP;
    else
	{
	if ( stat( filename, &sb ) != 0 )
	    {
	    syslog( LOG_ERR, "stat - %m" );
	    return (void*) 0;
	    }
	}

    /* See if we have it mapped already, via the hash table. */
    if ( check_hash_size() < 0 )
	{
	syslog( LOG_ERR, "check_hash_size() failure" );
	return (void*) 0;
	}
    m = find_hash( sb.st_ino, sb.st_dev, sb.st_size, sb.st_mtime );
    if ( m != (Map*) 0 )
	{
	/* Yep. */
	++m->refcount;
	return m->addr;
	}

    /* Nope.  Open the file. */
    fd = open( filename, O_RDONLY );
    if ( fd < 0 )
	{
	syslog( LOG_ERR, "open - %m" );
	return (void*) 0;
	}

    /* Find a free Map entry or make a new one. */
    if ( free_maps != (Map*) 0 )
	{
	m = free_maps;
	free_maps = m->next;
	--free_count;
	}
    else
	{
	m = (Map*) malloc( sizeof(Map) );
	if ( m == (Map*) 0 )
	    {
	    (void) close( fd );
	    return (void*) 0;
	    }
	}

    /* Fill in the Map entry. */
    m->ino = sb.st_ino;
    m->dev = sb.st_dev;
    m->size = sb.st_size;
    m->mtime = sb.st_mtime;
    m->refcount = 1;

#ifdef HAVE_MMAP
    /* Map the file into memory. */
    m->addr = mmap( 0, m->size, PROT_READ, MAP_SHARED, fd, 0 );
    if ( m->addr == (void*) -1 )
	{
	syslog( LOG_ERR, "mmap - %m" );
	(void) close( fd );
	free( (void*) m );
	return (void*) 0;
	}
#else /* HAVE_MMAP */
    /* Read the file into memory. */
    m->addr = (void*) malloc( m->size );
    if ( m->addr == (void*) 0 )
	{
	syslog( LOG_ERR, "not enough memory" );
	(void) close( fd );
	free( (void*) m );
	return (void*) 0;
	}
    if ( read( fd, m->addr, m->size ) != m->size )
	{
	syslog( LOG_ERR, "read - %m" );
	(void) close( fd );
	free( (void*) m );
	return (void*) 0;
	}
#endif /* HAVE_MMAP */
    (void) close( fd );

    /* Put the Map into the hash table. */
    if ( add_hash( m ) < 0 )
	{
	syslog( LOG_ERR, "add_hash() failure" );
	free( (void*) m );
	return (void*) 0;
	}

    /* Put the Map on the active list. */
    m->next = maps;
    maps = m;
    ++map_count;

    /* And return the address. */
    return m->addr;
    }


void
mmc_unmap( void* addr, struct timeval* nowP )
    {
    Map* m;

    /* Find the Map entry for this address. */
    for ( m = maps; m != (Map*) 0; m = m->next )
	{
	if ( m->addr == addr )
	    {
	    --m->refcount;
	    if ( nowP != (struct timeval*) 0 )
		m->reftime = nowP->tv_sec;
	    else
		m->reftime = time( (time_t*) 0 );
	    return;
	    }
	}
    /* Didn't find it.  Shrug. */
    }


void
mmc_cleanup( struct timeval* nowP )
    {
    time_t now;
    Map** mm;
    Map* m;

    /* Get current time, if necessary. */
    if ( nowP != (struct timeval*) 0 )
	now = nowP->tv_sec;
    else
	now = time( (time_t*) 0 );

    /* Really unmap any unreferenced entries older than the limit. */
    for ( mm = &maps; *mm != (Map*) 0; )
	{
	m = *mm;
	if ( m->refcount == 0 && now - m->reftime >= EXPIRE_AGE )
	    really_unmap( mm );
	else
	    mm = &(*mm)->next;
	    
	}

    /* Really free excess blocks on the free list. */
    while ( free_count > DESIRED_FREE_COUNT )
	{
	m = free_maps;
	free_maps = m->next;
	--free_count;
	free( (void*) m );
	}
    }


static void
really_unmap( Map** mm )
    {
    Map* m;

    m = *mm;
#ifdef HAVE_MMAP
    if ( munmap( m->addr, m->size ) < 0 )
	syslog( LOG_ERR, "munmap - %m" );
#else /* HAVE_MMAP */
    free( (void*) m->addr );
#endif /* HAVE_MMAP */
    /* And move the Map to the free list. */
    *mm = m->next;
    --map_count;
    m->next = free_maps;
    free_maps = m;
    ++free_count;
    /* This will sometimes break hash chains, but that's harmless.
    ** Worst case we map another copy of the same file.
    */
    hash_table[m->hash_idx] = (Map*) 0;	
    }


void
mmc_destroy( void )
    {
    Map* m;

    while ( maps != (Map*) 0 )
	really_unmap( &maps );
    while ( free_maps != (Map*) 0 )
	{
	m = free_maps;
	free_maps = m->next;
	--free_count;
	free( (void*) m );
	}
    }


void
mmc_stats( int* activeP, int* freeP )
    {
    *activeP = map_count;
    *freeP = free_count;
    }


/* Make sure the hash table is big enough. */
static int
check_hash_size( void )
    {
    int i;
    Map* m;

    /* Are we just starting out? */
    if ( hash_table == (Map**) 0 )
	hash_size = INITIAL_HASH_SIZE;
    /* Is it at least three times bigger than the number of entries? */
    else if ( hash_size >= map_count * 3 )
	return 0;
    else
	{
	/* No, got to expand. */
	free( (void*) hash_table );
	/* Find a prime at least six times bigger. */
	for ( hash_size = map_count * 6; ! is_prime( hash_size ); ++hash_size )
	    ;
	}
    /* Make the new table. */
    hash_table = (Map**) malloc( hash_size * sizeof(Map*) );
    if ( hash_table == (Map**) 0 )
	return -1;
    /* Clear it. */
    for ( i = 0; i < hash_size; ++i )
	hash_table[i] = (Map*) 0;
    /* And rehash all entries. */
    for ( m = maps; m != (Map*) 0; m = m->next )
	if ( add_hash( m ) < 0 )
	    return -1;
    return 0;
    }


static int
is_prime( int n )
    {
    int m, nom;

    if ( n % 2 == 0 )
	return 0;
    for ( m = 3; ; m += 2 )
	{
	nom = n / m;
	if ( nom < m )
	    return 1;
	if ( nom * m == n )
	    return 0;
	}
    }


static int
add_hash( Map* m )
    {
    int h, he, i;

    h = hash( m->ino, m->dev, m->size, m->mtime );
    he = ( h + hash_size - 1 ) % hash_size;
    for ( i = h; ; i = ( i + 1 ) % hash_size )
	{
	if ( hash_table[i] == (Map*) 0 )
	    {
	    hash_table[i] = m;
	    m->hash = h;
	    m->hash_idx = i;
	    return 0;
	    }
	if ( i == he )
	    break;
	}
    return -1;
    }


static Map*
find_hash( ino_t ino, dev_t dev, off_t size, time_t mtime )
    {
    int h, he, i;
    Map* m;

    h = hash( ino, dev, size, mtime );
    he = ( h + hash_size - 1 ) % hash_size;
    for ( i = h; ; i = ( i + 1 ) % hash_size )
	{
	m = hash_table[i];
	if ( m == (Map*) 0 )
	    break;
	if ( m->hash == h && m->ino == ino && m->dev == dev &&
	     m->size == size && m->mtime == mtime )
	    return m;
	if ( i == he )
	    break;
	}
    return (Map*) 0;
    }


static int
hash( ino_t ino, dev_t dev, off_t size, time_t mtime )
    {
    return ( ino ^ dev ^ size ^ mtime ) % hash_size;
    }
