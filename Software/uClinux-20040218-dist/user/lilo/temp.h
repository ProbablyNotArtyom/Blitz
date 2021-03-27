/* temp.h  -  Temporary file registry */

/* Copyright 1992-1995 Werner Almesberger. See file COPYING for details. */


#ifndef TEMP_H
#define TEMP_H


void temp_register(char *name);

/* Registers a file for removal at exit. */

void temp_unregister(char *name);

/* Removes the specified file from the temporary file list. */

void temp_remove(void);

/* Removes all temporary files. */

#endif
