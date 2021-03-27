/*****************************************************************************/

/*
 *	newfs.c -- create new flat FLASH file-system.
 *
 *	(C) Copyright 1999, Greg Ungerer (gerg@snapgear.com).
 *	(C) Copyright 2000, Lineo Inc. (www.lineo.com)
 *	(C) Copyright 2002, SnapGear (www.snapgear.com)
 */

/*****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "flatfs.h"

/*****************************************************************************/

#define ERROR_CODE()	(-(__LINE__))	/* unique error codes */

/*****************************************************************************/

/*
 *	Count the number of files in the config area.
 *  Updates numfiles and numbytes and returns numfiles or < 0 on error.
 */

int flatfilecount(void)
{
	DIR		*dirp;
	struct dirent	*dp;
	struct stat sb;

	numfiles = 0;
	numbytes = 0;

	if (chdir(SRCDIR) < 0)
		return(ERROR_CODE());

	/* Scan directory */
	if ((dirp = opendir(".")) == NULL)
		return(ERROR_CODE());

	while ((dp = readdir(dirp)) != NULL) {
		/* Only count normal files */
		if (stat(dp->d_name, &sb) == 0) {
			if (S_ISREG(sb.st_mode)) {
				numfiles++;
				numbytes += sb.st_size;
			}
		}
	}

	closedir(dirp);
	return(numfiles);
}

/*****************************************************************************/

/*
 *	Remove all files from the config file-system.
 */

int flatclean(void)
{
	DIR		*dirp;
	struct dirent	*dp;

	if (chdir(SRCDIR) < 0)
		return(ERROR_CODE());

	/* Scan directory */
	if ((dirp = opendir(".")) == NULL)
		return(ERROR_CODE());

	while ((dp = readdir(dirp)) != NULL) {
		if ((strcmp(dp->d_name, ".") == 0) ||
		    (strcmp(dp->d_name, "..") == 0))
			continue;
		unlink(dp->d_name);
	}

	closedir(dirp);
	return(0);
}

/*****************************************************************************/

/*
 *	This is basically just a directory copy. Copy all files from the
 *	given directory to the config directory.
 */

int flatnew(const char *dir)
{
	DIR		*dirp;
	struct stat	st;
	struct dirent	*dp;
	unsigned int	size, n;
	int		fddefault, fdconfig;
	char		filename[512];
	unsigned char	buf[1024];

	if (chdir(dir) < 0)
		return(ERROR_CODE());

	/* Scan directory */
	if ((dirp = opendir(".")) == NULL)
		return(ERROR_CODE());

	numfiles = 0;
	numbytes = 0;
	numdropped = 0;

	while ((dp = readdir(dirp)) != NULL) {

		if ((strcmp(dp->d_name, ".") == 0) ||
		    (strcmp(dp->d_name, "..") == 0))
			continue;

		if (stat(dp->d_name, &st) < 0)
			return(ERROR_CODE());

		strcpy(filename, SRCDIR);
		strcat(filename, "/");
		strcat(filename, dp->d_name);

		/* Write the contents of the file. */
		if ((fddefault = open(dp->d_name, O_RDONLY)) < 0)
			return(ERROR_CODE());
		fdconfig = open(filename, O_WRONLY | O_TRUNC | O_CREAT, st.st_mode);
		if (fdconfig < 0)
			return(ERROR_CODE());

		for (size = st.st_size; (size > 0); size -= n) {
			n = (size > sizeof(buf)) ? sizeof(buf) : size;
			if (read(fddefault, &buf[0], n) != n)
				break;
			if (write(fdconfig, (void *) &buf[0], n) != n)
				break;
		}
		close(fdconfig);
		close(fddefault);

		if (size > 0) {
			numdropped++;
		} else {
			numfiles++;
			numbytes += st.st_size;
		}
	}

	closedir(dirp);
	return(0);
}

/*****************************************************************************/
