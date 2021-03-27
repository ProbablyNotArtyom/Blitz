/* identify.c  -  Translate label names to kernel paths */

/* Copyright 1992-1995 Werner Almesberger. See file COPYING for details. */


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "cfg.h"


static char *identify,*opt;


static void do_identify(char *var,char type)
{
    char *label,*path,*alias,*initrd;
    int image, ramdisk;

#if 1
    image = !!strchr(opt,'i');
    ramdisk = !!strchr(opt,'r');
    if (opt && !image && !ramdisk) exit(1);
    if (!opt) image = 1;
#else
    image = ramdisk = 1;
    printf("do_identify:  opt=\"%s\"\n", opt);
#endif
    
    label = strrchr(path = cfg_get_strg(cf_identify,var),'/');
    if (label) label++;
    if (cfg_get_strg(cf_all,"label")) label = cfg_get_strg(cf_all,"label");
    else if (!label) label = path;
    alias = cfg_get_strg(cf_all,"alias");
    initrd = cfg_get_strg(cf_kernel,"initrd");
#ifdef LCF_IGNORECASE
    if (!strcasecmp(label,identify) || (alias && !strcasecmp(alias,identify))) {
#else
    if (!strcmp(label,identify) || (alias && !strcmp(alias,identify))) {
#endif
	if (image) printf("%s\n",path);
	if (ramdisk) printf("%s\n",initrd?initrd:"No initial ramdisk specified");
	exit(0);
    }
}


void id_image(void)
{
    cfg_init(cf_image);
    (void) cfg_parse(cf_image);
    do_identify("image",'i');
    cfg_init(cf_identify);
}


void id_other(void)
{
    cfg_init(cf_other);
    cfg_init(cf_kernel);
    (void) cfg_parse(cf_other);
    cfg_init(cf_identify);
}


void identify_image(char *label,char *options)
{
    identify = label;
    opt = options;
    cfg_init(cf_identify);
    if (cfg_parse(cf_identify)) cfg_error("Syntax error");
    fprintf(errstd,"No image found for \"%s\"\n",label);
    exit(1);
}
