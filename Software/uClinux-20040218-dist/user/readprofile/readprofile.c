/*
 *  readprofile.c - used to read /proc/profile
 *
 *  Copyright (C) 1994,1996 Alessandro Rubini (rubini@ipvvis.unipv.it)
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>

#define RELEASE "2.0, May 1996"

#define S_LEN 128

static char *prgname;

/* These are the defaults */
#ifdef EMBED
static char defaultmap[]="/System.map";
#else
static char defaultmap[]="/usr/src/linux/System.map";
#endif
static char defaultpro[]="/proc/profile";
static char optstring[]="m:p:itvarV";

void usage()
{
  fprintf(stderr,
		  "%s: Usage: \"%s [options]\n"
		  "\t -m <mapfile>  (default = \"%s\")\n"
		  "\t -p <pro-file> (default = \"%s\")\n"
		  "\t -i            print only info about the sampling step\n"
		  "\t -v            print verbose data\n"
		  "\t -a            print all symbols, even if count is 0\n"
		  "\t -r            reset all the counters (root only)\n"
		  "\t -V            print version and exit\n"
		  ,prgname,prgname,defaultmap,defaultpro);
  exit(1);
}

FILE *myopen(char *name, char *mode, int *flag)
{
static char cmdline[S_LEN];

  if (!strcmp(name+strlen(name)-3,".gz"))
    {
    *flag=1;
    sprintf(cmdline,"zcat %s", name);
    return popen(cmdline,mode);
    }
  *flag=0;
  return fopen(name,mode);
}

int main (int argc, char **argv)
{
FILE *pro;
FILE *map;
int proFd;
char *mapFile, *proFile;
unsigned int len, add0=0, step, index, totalticks, percent;
unsigned int *buf, total, fn_len;
unsigned int fn_add, next_add;           /* current and next address */
char fn_name[S_LEN], next_name[S_LEN];   /* current and next name */
char mode[8];
int c;
int optAll=0, optInfo=0, optReset=0, optVerbose=0;
char mapline[S_LEN];
int maplineno=1;
int popenMap;   /* flag to tell if popen() has been used */

#define next (current^1)

  prgname=argv[0];
  proFile=defaultpro;
  mapFile=defaultmap;

  while ((c=getopt(argc,argv,optstring))!=-1)
    {
    switch(c)
      {
      case 'm': mapFile=optarg; break;
      case 'p': proFile=optarg; break;
      case 'a': optAll++;       break;
      case 'i': optInfo++;      break;
      case 'r': optReset++;     break;
      case 'v': optVerbose++;   break;
      case 'V': printf("%s Version %s\n",prgname,RELEASE); exit(0);
      default: usage();
      }
    }

  if (optReset)
    {
    /* try to become root, just in case */
    setuid(0);
    pro=fopen(defaultpro,"a");
    if (!pro)
      {perror(proFile); exit(1);}
    fprintf(pro,"anything\n");
    fclose(pro);
    exit(0);
    }

  /*
   * Use an fd for the profiling buffer, to skip stdio overhead
   */
  if ( ((proFd=open(proFile,O_RDONLY)) < 0)
      || ((len=lseek(proFd,0,SEEK_END)) < 0)
      || (lseek(proFd,0,SEEK_SET)<0) )
    {
    fprintf(stderr,"%s: %s: %s\n",prgname,proFile,strerror(errno));
    exit(1);
    }

  if ( !(buf=malloc(len)) )
    { fprintf(stderr,"%s: malloc(): %s\n",prgname, strerror(errno)); exit(1); }

  if (read(proFd,buf,len) != len)
    {
    fprintf(stderr,"%s: %s: %s\n",prgname,proFile,strerror(errno));
    exit(1);
    }
  close(proFd);

  for (index = 0, totalticks = 0; (index < len/sizeof(int)); index++)
	totalticks += buf[index];

  index = 0;
  step=buf[0];
  if (optInfo)
    {
    printf("Sampling_step: %i\n",step);
    exit(0);
    } 

  total=0;

  if (!(map=myopen(mapFile,"r",&popenMap)))
    {fprintf(stderr,"%s: ",prgname);perror(mapFile);exit(1);}

  while(fgets(mapline,S_LEN,map))
    {
    if (sscanf(mapline,"%x %s %s",&fn_add,mode,fn_name)!=3)
      {
      fprintf(stderr,"%s: %s(%i): wrong map line\n",
	      prgname,mapFile, maplineno);
      exit(1);
      }
    if (strcmp(fn_name,"_stext") == 0) /* only elf works like this */
      {
      add0=fn_add;
      break;
      }
    }

  if (!add0)
    {
    fprintf(stderr,"%s: can't find \"_stext\" in %s\n",prgname, mapFile);
    exit(1);
    }

  /*
   * Main loop.
   */
  while(fgets(mapline,S_LEN,map))
    {
    unsigned int this=0;

    if (sscanf(mapline,"%x %s %s",&next_add,mode,next_name)!=3)
      {
      fprintf(stderr,"%s: %s(%i): wrong map line\n",
	      prgname,mapFile, maplineno);
      exit(1);
      }
    if (*mode!='T' && *mode!='t') break; /* only text is profiled */

    while (index < (next_add-add0)/step)
      this += buf[index++];
    total += this;

    fn_len = next_add-fn_add;
    percent = (this * 100) / totalticks;
    if (percent && (this || optAll))
      {
      if (optVerbose)
	printf("%3d%% %08x %-36s %6i %8d.%04d\n",
	       percent, fn_add,fn_name,this,
	       (this / fn_len), ((this*10000)/fn_len)%10000 );
      else
	printf("%3d%% %6i %-36s %8d.%04d\n",
	       percent, this,fn_name,
	       (this / fn_len), ((this*10000)/fn_len)%10000 );
      }
    fn_add=next_add; strcpy(fn_name,next_name);
    }

  /* trailer */
  printf("---------------------------------------------------------------------------\n");
  if (optVerbose)
    printf("     %08x %-36s %6i %8d.%04d\n",
	   0,"total",total,
	   (total / (fn_add-add0)), ((total*10000)/(fn_add-add0))%10000 );
  else
    printf("     %6i %-36s %8d.%04d\n",
	   total,"total",
	   (total / (fn_add-add0)), ((total*10000)/(fn_add-add0))%10000 );
	
  popenMap ? pclose(map) : fclose(map);
  exit(0);
}



