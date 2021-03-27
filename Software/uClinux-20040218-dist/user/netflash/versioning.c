/* netflash.c:
 *
 * Copyright (C) 2000,  Lineo (www.lineo.com)
 * Copyright (C) 1999-2000,  Greg Ungerer (gerg@snapgear.com)
 *
 * Copied and hacked from rootloader.c which was:
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <sys/mount.h>
#include <string.h>
#include <linux/autoconf.h>
#include <config/autoconf.h>
#include <ctype.h>
#include "netflash.h"
#include "versioning.h"

#define MAX_VENDOR_SIZE			64
#define MAX_PRODUCT_SIZE		64
#define MAX_VERSION_SIZE		12
#define MAX_LANG_SIZE			8


/****************************************************************************/
extern struct blkmem_program_t * prog;

char vendor_name[] = CONFIG_VENDOR;
char product_name[] = CONFIG_PRODUCT;
char image_version[] = CONFIG_VERSION;


/****************************************************************************/
static char *get_string(struct fileblock_t **block, char *cp, char *str, int len);
static int check_version_info(char *version, char *new_version);
static char *decrement_blk(char *cp, struct fileblock_t **block);
static int get_version_bits(char *version, char *ver_long, char *letter,
		int *num, char *lang);
static int minor_to_int(char letter, int num);


/****************************************************************************/

/*
 * Code to check that we are putting the correct type of flash into this
 * unit.
 * This code also removes the versioning information from the end
 * of the memory buffer.
 *
 * ret:
 *		0 - everything is correct.
 *		1 - the product name is incorrect.
 *		2 - the vendor name is incorrect.
 *		3 - the version is the same.
 *		4 - the version is older.
 *		5 - the version is invalid.
 *		6 - the version language is different.
 */

/*
 * The last few bytes of the image look like the following:
 *
 *  \0version\0vendore_name\0product_namechksum
 *	the chksum is 16bits wide, and the version is no more than 20bytes.
 *
 * version is w.x.y[nz], where n is ubpi, and w, x, y and z are 1 or 2 digit
 * numbers.
 *
 */
int check_vendor(char *vendorName, char *productName, char *version)
{
	struct fileblock_t *currBlock;
	int versionInfo;
	char *cp;
	char imageVendorName[MAX_VENDOR_SIZE];
	char imageProductName[MAX_PRODUCT_SIZE];
	char imageVersion[MAX_VERSION_SIZE];

	/*
	 * Point to what should be the last byte in the product name string.
	 */
	if (fileblocks == NULL)
		return 5;
	for (currBlock = fileblocks; currBlock->next; currBlock = currBlock->next);
	cp = currBlock->data + currBlock->length - 1;

	/*
	 * Now try to get the vendor/product/version strings, from the end
	 * of the image
	 */
	cp = get_string(&currBlock, cp, imageProductName, MAX_PRODUCT_SIZE);
	if (cp == NULL)
		return 5;

	cp = get_string(&currBlock, cp, imageVendorName, MAX_VENDOR_SIZE);
	if (cp == NULL)
		return 5;

	cp = get_string(&currBlock, cp, imageVersion, MAX_VERSION_SIZE);
	if (cp == NULL)
		return 5;

	/* Looks like there was versioning information there, strip it off
	 * now so that we don't write it to flash, or try to decompress it, etc */
	remove_data(strlen(imageProductName) + strlen(imageVendorName) + strlen(imageVersion) + 3);

	/*
	 * Check the product name.
	 */
	if (strcmp(productName, imageProductName) != 0)
		return 1;

	/*
	 * Check the vendor name.
	 */
	if (strcmp(vendorName, imageVendorName) != 0)
		return 2;

	/*
	 * Check the version number.
	 */
	versionInfo = check_version_info(version, imageVersion);

	return versionInfo;
}


/* get_string
 *
 * This gets a printable string from the memory buffer.
 * It searchs backwards for a non-printable character or a NULL terminator.
 * Success is defined as the two strings
 * being excactly the same.
 *
 * inputs:
 *
 * block - the block in which we should begin comparing.
 * cp - a pointer to the char from which we should begin comparing. it
 *		must point into the specified block.
 * str/len - the buffer to store the string in.
 *
 * ret:
 *
 * NULL - we couldn't find the string.
 * anything else - a pointer to the char before the NULL terminator.
 */
char *get_string(struct fileblock_t **block, char *cp, char *str, int len)
{
	int i, j;
	char c;

	i = 0;
	while (cp && *cp && (i < len)) {
		if (!isprint(*cp))
			return NULL;
		str[i++] = *cp;
		cp = decrement_blk(cp, block);
	}
	if (cp == NULL || i == 0 || i >= len)
		return NULL;

	/* Store the null terminator */
	str[i] = 0;
	cp = decrement_blk(cp, block);
	if (cp == NULL)
		return NULL;

	/* We read string in reverse order, so reverse it again */
	for (j=0; j<i/2; j++) {
		c = str[j];
		str[j] = str[i-j-1];
		str[i-j-1] = c;
	}

	return cp;
}


#define NUM_VERSION_ELEMS 9
/* check_version_info
 *
 * Check with the version number in imageVersion is a valid
 * upgrade to the current version.
 * The version is ALWAYS of the form major.minor.minor or it is invalid.
 * We determine whether something is older (less than) or newer,
 * by simply using a strcmp.  This functionality will change over
 * time to reflect intuitive notions of what constitutes reasonable versioing.
 *
 * inputs:
 *
 * version - the version of the current flash image.
 * new_version - the version of the new flash image.
 *
 * ret:
 * 		0 - it all worked perfectly and the version looks okay.
 *		3 - the new version is the same.
 *		4 - the new version is older.
 *		5 - the new version is invalid.
 *		6 - the version language is different.
 */
int check_version_info(char *version, char *new_version)
{
	char new_ver[NUM_VERSION_ELEMS];
	char old_ver[NUM_VERSION_ELEMS];
	char old_version[MAX_VERSION_SIZE];
	char new_lang[MAX_LANG_SIZE];
	char old_lang[MAX_LANG_SIZE];
	char new_letter, old_letter;
	int new_minor, old_minor;
	int res;
	int old, new;
	
	strncpy(old_version, version, sizeof(old_version));
	old_version[sizeof(old_version)-1] = '\0';
	
	if(!get_version_bits(new_version, new_ver, &new_letter, &new_minor,
			new_lang))
		return 5;

	if(!get_version_bits(old_version, old_ver, &old_letter, &old_minor,
			old_lang))
		return 5;
	
	if (strcmp(old_lang, new_lang) != 0)
		return 6;
	res = strcmp(old_ver, new_ver);
	if(res < 0)
		return 0;
	else if(res > 0)
		return 4;
	else{			/*we have to look at the minor numbers and the char*/
		if((new = minor_to_int(new_letter, new_minor)) > \
			(old = minor_to_int(old_letter, old_minor)))
			return 0;
		else if(new == old)
			return 3;
		else
			return 4;
	}
} 
#undef NUM_VERSION_ELEMS
#undef MAX_VERSION_SIZE

/*
 * Decrement the pointer and block number appropriately.
 * we return NULL when asked to decrement before the beginning.
 */
char *decrement_blk(char *cp, struct fileblock_t **block)
{
	struct fileblock_t *p;
	if(cp==NULL || (*block)==NULL)
		return NULL;

	if(cp == (char *)(*block)->data){ /*move to previous block*/
		if (fileblocks == *block)
			return NULL;
		for (p=fileblocks; p && p->next!=(*block); p=p->next);
		if (p==NULL)
			return NULL;
		*block = p;
		cp = (*block)->data + (*block)->length - 1;
	}else{
		cp--;
	}
	return cp;
}


/*
 * This is not currently used.
 */
#if 0
char *increment_blk(char *cp, struct fileblock_t **block)
{
	if(cp == NULL || (*block) == NULL){
		return NULL;
	}

	if(cp == ((char *)(*block)->data) + (*block)->length - 1){
		if (*block->next == NULL) {
			*block = fileblocks;
			cp = NULL;
		}else{
			*block = (*block)->next;
			cp = (*block)->data;
		}
	}else{
		cp++;
	}
	return cp;
}
#endif /*0*/


static int get_version_bits(char *version, char *ver_long, char *letter,
		int *num, char *lang)
{
	int i;
	char *tmp;
	int len;
	char *eptr;
	char ver_tmp[10] = {'\0'};
	ver_long[0] = '\0';

	/* Extrat the language suffix */
	eptr = strchr(version, '\0');
	while (--eptr > version && isupper(*eptr));
	if (eptr == version)
		return 0;
	eptr++;
	for (i=0; (lang[i] = eptr[i]) != '\0'; i++);
	*eptr-- = '\0';

       	/* Versions with unnumbered trailing letters will be treated as [u|b|p|i]0 */
        if (strchr("bupi", *eptr) != NULL) {
		eptr[1] = '0';
		eptr[2] = '\0';
	}

	tmp = strtok(version, ".");

	while(tmp != NULL){
		if((len = strlen(tmp)) == 1){
			if(!isdigit(tmp[0]))
				return 0;
			strncat(ver_tmp, "0", sizeof(ver_tmp) - strlen(ver_tmp));
		}else if(len == 2){
			if((!(isdigit(tmp[0]))) && (!(isdigit(tmp[1]))))
				return 0;
		}else if(len == 3){
			if((!(isdigit(tmp[0]))) || ((isdigit(tmp[1]))))
				return 0;
			strncat(ver_tmp, "0", sizeof(ver_tmp) - strlen(ver_tmp));
		}
		strncat(ver_tmp, tmp, sizeof(ver_tmp) - strlen(ver_tmp));
		tmp = strtok(NULL, ".");
	}
	
	if(((len = strlen(ver_tmp)) == 7) || (len > 9))
		return 0;
	if(strlen(ver_tmp) > 6){
		tmp = &(ver_tmp[6]);

		/*
		 * We only support an (u)pdate > (b)eta > (p)re-release
		 * '>' denotes more recent (greater version number) to the left.
		 */
		if((*tmp != 'p') && (*tmp != 'b') && (*tmp != 'u'))
			return 0;

		*letter = *tmp;
		*tmp = '\0';
		tmp++;

		if(*tmp == '\0')	/*if we have a letter, we MUST have a number*/
			return 0;
		*num = strtol(tmp, &eptr, 10);

		if((*eptr) != '\0') /*we should have used up the entier string*/
			return 0;		
		strcpy(ver_long, ver_tmp);
		return 1;
	}else if(len == 6){
		*letter = '\0';
		*num = 0;
		strcpy(ver_long, ver_tmp);
		return 1;
	}else
		return 0;
}


int minor_to_int(char letter, int num)
{
	int res=0;
	if(letter == 'u')
		res+=300;
	if(letter == '\0'){
		res+=200;
		return res;
	}
	if(letter == 'b')
		res+=100;

	/* Otherwise it is 'p' or something unknown.
         * Just leave it as-is.
         */

	return res + num;
}

/****************************************************************************/
