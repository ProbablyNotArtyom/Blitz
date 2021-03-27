/* expand.c: expand a file with holes into another
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *                     D. Jeff Dionne <jeff@lineo.ca>,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ntohl(x) (x)

int
expand(char *from, char *to)
{
  int fdi;
  int fdo;
  unsigned int pos;
  unsigned int len;
  unsigned int count;
  char *buf;

  count = 0;

  if ((fdi = open(from,O_RDONLY)) < 0) {
    fprintf(stderr,"Can't open compressed file %s\n",from);
    return 0;
  }

  if ((fdo = open(to,O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
    fprintf(stderr,"Can't open expanded file %s\n",to);
    close(fdi);
    return 0;
  }
  
  
  
  if (!(buf = malloc(2048))) {
    fprintf(stderr,"can't allocate memory\n");
    close(fdi);
    close(fdo);
    return 0;
  }

  /* Prefill */
  read(fdi,(char *)&len,4);

  memset(buf, 0, 2048);
  while(len>0) {
  	write(fdo, buf, (len > 2048) ? 2048 : len);
  	len -= 2048;
  }
  lseek(fdo, 0, SEEK_SET);
  
  /* ZRLE */
  while (read(fdi,&pos,4) == 4) {
    if (read(fdi,&len,4) != 4) break;
    pos = ntohl(pos);
    len = ntohl(len);
    
    lseek(fdo,pos,SEEK_SET);
    read(fdi,buf,len);
    write(fdo,buf,len);
  }

  close(fdi);
  close(fdo);
  free(buf);
}
