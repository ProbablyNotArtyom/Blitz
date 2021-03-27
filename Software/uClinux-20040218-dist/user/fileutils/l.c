/* l.c - A short version of `ls'
 * Sun Feb  8 16:28:03 EST 1998 - claudio@pos.inf.ufpr.br (Claudio Matsuoka)
 *
 * Based on the ls.c sources Copyright (c) 1993 by David I. Bell
 */

/* This works like `ls -m', displaying files horizontally and separated
 * by commas
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define	LISTSIZE	256
#define PATHLEN		256


#ifndef __P
#define __P(x)
#endif

/* Prototypes */
int		main		__P((int, char **));
void		lsfile		__P((char *));
int		namesort	__P((__const void *, __const void *));
char *		buildname	__P((char *, char *));


#define COLS 80			/* Hardcoded number of columns (Yuck!) */


static	char	**list;
static	int	listsize;
static	int	listused;
static	int	comma = 0, col = 0, len;
static	char	*err_mem = "out of memory\n";


int
main(argc, argv)
	char	**argv;
{
	char		*cp;
	char		*name = NULL;
	int		mult = 0;
	int		isdir = 0;
	int		lf_flag = 0;
	int		i;
	DIR		*dirp;
	int		endslash;
	char		**newlist;
	struct	dirent	*dp;
	char		fullname[PATHLEN];
	struct	stat	statbuf;
	static		char *def[2] = {"-ls", "."};

	if (listsize == 0) {
		list = (char **) malloc(LISTSIZE * sizeof(char *));
		if (list == NULL) {
			fputs (err_mem, stderr);
			return;
		}
		listsize = LISTSIZE;
	}

	listused = 0;

	if (argc <= 1) {
		argc = 2;
		argv = def;
	}

	mult = argc > 2;

	while (argc-- > 1) {
		name = *(++argv);
		endslash = (*name && (name[strlen(name) - 1] == '/'));

		if (lstat (name, &statbuf) < 0) {
			perror (name);
			continue;
		}

		if (!S_ISDIR (statbuf.st_mode)) {
			if (isdir) {
				fputs ("\n\n", stdout);
				comma = 0;
				col = 0;
			}
			lsfile (name);
			isdir = 0;
			lf_flag = 1;
                        continue;
                }

		if (lf_flag)
			fputs ("\n\n", stdout);
		comma = 0;
		col = 0;
		lf_flag = 1;
		isdir = 1;

		/*
		 * Do all the files in a directory.
		 */
		dirp = opendir (name);
		if (dirp == NULL) {
			perror (name);
			continue;
		}

		if (mult) {
			fputs (name, stdout);
			fputs (":\n", stdout);
		}

		while ((dp = readdir(dirp)) != NULL) {
			fullname[0] = '\0';

			if ((*name != '.') || (name[1] != '\0')) {
				strcpy(fullname, name);
				if (!endslash)
					strcat(fullname, "/");
			}

			strcat(fullname, dp->d_name);
			if (listused >= listsize) {
				newlist = realloc(list,
					((sizeof(char **)) * (listsize + LISTSIZE)));
				if (newlist == NULL) {
					fputs (err_mem, stderr);
					break;
				}
				list = newlist;
				listsize += LISTSIZE;
			}

			list[listused] = strdup(fullname);

			if (list[listused] == NULL) {
				fputs (err_mem, stderr);
				break;
			}

			listused++;
		}

		closedir(dirp);

		/*
		 * Sort the files.
		 */
		qsort((char *) list, listused, sizeof(char *), namesort);

		
		/*
		 * Now finally list the filenames.
		 */
		for (i = 0; i < listused; i++) {
			name = list[i];

			if (lstat (name, &statbuf) < 0) {
				perror(name);
				free(name);
				continue;
			}

			cp = strrchr(name, '/');
			if (cp)
				cp++;
			else
				cp = name;

			lsfile (cp);
			free(name);
		}

		listused = 0;
	}
	fputs ("\n", stdout);
}


/*
 * Do an LS of a particular file name
 */
void
lsfile (cp)
	char *cp;
{
	if (*cp != '.') {
		len = strlen (cp);
		if ((col += len) >= (COLS - 3)) {
			fputs (",\n", stdout);
			col = len;
		} else if (comma) {
			fputs (", ", stdout);
			col += 2; 
		} else comma = 1;
		fputs (cp, stdout);
	}
}


/*
 * Build a path name from the specified directory name and file name.
 * If the directory name is NULL, then the original filename is returned.
 * The built path is in a static area, and is overwritten for each call.
 */
char *
buildname(dirname, filename)
	char	*dirname;
	char	*filename;
{
	char		*cp;
	static	char	buf[PATHLEN];

	if ((dirname == NULL) || (*dirname == '\0'))
		return filename;

	cp = strrchr (filename, '/');
	if (cp)
		filename = cp + 1;

	strcpy (buf, dirname);
	strcat (buf, "/");
	strcat (buf, filename);

	return buf;
}


/*
 * Sort routine for list of filenames.
 */
int
namesort(pp1, pp2)
	const void	*pp1;
	const void	*pp2;
{
	char **p1 = (char **)pp1;
	char **p2 = (char **)pp2;

	return strcmp(*p1, *p2);
}
