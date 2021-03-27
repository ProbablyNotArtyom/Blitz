/* eraseall.c -- erase the whole of a MTD device

   Copyright (C) 2000 Arcom Control System Ltd  

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA  

   $Id: eraseall.c,v 1.7 2001/04/25 07:11:19 ollie Exp $
*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

#include <linux/mtd/mtd.h>

#define PROGRAM "eraseall"
#define VERSION "0.1.0"

static const char *exe_name;
static const char *mtd_device;
static int quiet; /* true -- don't output progress */

static void process_options( int argc, char *argv[] );
static void display_help();
static void display_version();

int main( int argc,char *argv[] )
{
    mtd_info_t meminfo;
    int fd;
    erase_info_t erase;
   
    process_options( argc, argv );
   
    if( (fd = open( mtd_device, O_RDWR )) < 0 ) {
	fprintf( stderr, "%s: %s: %s\n", exe_name, mtd_device,
		 strerror( errno ) );
	exit( 1 );
    }
   
   
   if( ioctl( fd, MEMGETINFO, &meminfo ) != 0 ) {
       fprintf( stderr, "%s: %s: unable to get MTD device info\n", exe_name, 
		mtd_device );
       exit( 1 );
   }

   erase.length = meminfo.erasesize;
   for (erase.start = 0; erase.start < meminfo.size;
	erase.start += meminfo.erasesize) {
       
       if( !quiet ) {
	   printf( "\rErasing %ld Kibyte @ %lx -- %2ld %% complete.", 
		   meminfo.erasesize/1024, erase.start,
		   erase.start*100/meminfo.size );
       }
       fflush( stdout );
	
       if(ioctl( fd, MEMERASE, &erase) != 0)
       {
	       fprintf( stderr, "\n%s: %s: MTD Erase failure: %s\n", exe_name, 
			mtd_device, strerror( errno) );
	       //exit( 1 );
       }
   }
   if( !quiet ) {
       printf( "\rErased %ld Kibyte @ %lx -- 100%% complete.       \n",
	       meminfo.size/1024, 0L );
   }
   
   return 0;
}


void process_options( int argc, char *argv[] )
{
    int error=0;

    exe_name=argv[0];

    for(;;) {
        int option_index = 0;
        static const char* short_options="q";
        static const struct option long_options[] = {
            {"help", no_argument, 0, 0},
            {"version", no_argument, 0, 0},
	    {"quiet", no_argument, 0, 'q'},
	    {"silent", no_argument, 0, 'q'},

            {0, 0, 0, 0},
        };

        int c=getopt_long( argc, argv, short_options,
                           long_options, &option_index );
        if( c==EOF ) {
            break;
        }

        switch( c ) {
        case 0 :
            switch( option_index ) {
            case 0 :
                display_help();
                break;
            case 1 :
                display_version();
                break;
            }
            break;
	case 'q' :
	    quiet=1;
	    break;
        case '?' :
            error=1;
            break;
        }
    }
    if( optind==argc ) {
	fprintf( stderr, "%s: no MTD device specified\n", exe_name );
        error=1;
    }
    if( error ) {
        fprintf( stderr, "Try `%s --help' for more information.\n", exe_name );
        exit( 1 );
    }

    mtd_device=argv[optind];
}


void display_help()
{
    printf( "Usage: %s [OPTION] MTD_DEVICE\n"
            "Erases all of the specified MTD device.\n"
            "\n"
            "  -q, --quiet    don't display progress messages\n"
	    "      --silent   same as --quiet\n"
            "      --help     display this help and exit\n"
            "      --version  output version information and exit\n"
            , exe_name);
    exit( 0 );
}
    

void display_version()
{
    printf( PROGRAM " " VERSION "\n"
            "\n"
            "Copyright (C) 2000 Arcom Control Systems Ltd\n"
	    "\n"
            PROGRAM " comes with NO WARRANTY\n"
            "to the extent permitted by law.\n"
            "\n"
            "You may redistribute copies of " PROGRAM "\n"
            "under the terms of the GNU General Public Licence.\n"
            "See the file `COPYING' for more information.\n"                  
	);
    exit( 0 );
}
