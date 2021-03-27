/* temp.c  -  Temporary file registry */

/* Copyright 1992-1995 Werner Almesberger. See file COPYING for details. */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "temp.h"


typedef struct _temp {
    char *name;
    struct _temp *next;
} TEMP;


static TEMP *list = NULL;


void temp_register(char *name)
{
    TEMP *new;

    new = alloc_t(TEMP);
    new->name = stralloc(name);
    new->next = list;
    list = new;
}


void temp_unregister(char *name)
{
    TEMP **walk,*this;

    for (walk = &list; *walk; walk = &(*walk)->next)
	if (!strcmp(name,(*walk)->name)) {
	    this = *walk;
	    *walk = this->next;
	    free(this->name);
	    free(this);
	    return;
	}
    die("Internal error: temp_unregister %s",name);
}


void temp_remove(void)
{
    TEMP *next;

    while (list) {
	next = list->next;
	if (remove(list->name) < 0)
	    fprintf(errstd,"(temp) %s: %s",list->name,strerror(errno));
	else if (verbose > 1) printf("Removed temporary file %s\n",list->name);
	free(list->name);
	free(list);
	list = next;
    }
}
