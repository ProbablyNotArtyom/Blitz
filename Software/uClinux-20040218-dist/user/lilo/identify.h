/* identify.h  -  Translate label names to kernel paths */

/* Copyright 1992-1995 Werner Almesberger. See file COPYING for details. */


#ifndef IDENTIFY_H
#define IDENTIFY_H

void identify_image(char *label,char *options);

/* Identifies the image which is referenced by the label. Prints the path name
   to standard output. If options is non-NULL, the following characters are
   used to filter the selection: i = traditional image, c = compound image,
   v = verify that the file exists. An error message is printed to standard
   error if no appropriate image can be found. */

#endif
