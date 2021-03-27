/*
 * configfile.c
 *
 * Methods for accessing the PPTPD config file and searching for
 * PPTPD keywords.
 *
 * $Id: configfile.c,v 1.2 2001/01/09 00:00:34 davidm Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>

#include "defaults.h"
#include "configfile.h"
#include "our_syslog.h"

/* Local function prototypes */
static FILE *open_config_file(char *filename);
static void close_config_file(FILE * file);

/*
 * read_config_file
 *
 * This method opens up the file specified by 'filename' and searches
 * through the file for 'keyword'. If 'keyword' is found any string
 * following it is stored in 'value'.
 *
 * args: filename (IN) - config filename
 *       keyword (IN) - word to search for in config file
 *       value (OUT) - value of keyword
 *
 * retn: -1 on error, 0 if keyword not found, 1 on value success
 */
int read_config_file(char *filename, char *keyword, char *value)
{
	FILE *in;
	int len;
	char buffer[MAX_CONFIG_STRING_SIZE], w[MAX_CONFIG_STRING_SIZE],
		v[MAX_CONFIG_STRING_SIZE];

	in = open_config_file(filename);
	if (in == NULL) {
		/* Couldn't find config file, or permission denied */
		return -1;
	}
	while ((fgets(buffer, MAX_CONFIG_STRING_SIZE - 1, in)) != NULL) {
		/* ignore long lines */
		if (buffer[(len = strlen(buffer)) - 1] != '\n') {
			syslog(LOG_ERR, "Long config file line ignored.");
			do
				fgets(buffer, MAX_CONFIG_STRING_SIZE - 1, in);
			while (buffer[strlen(buffer) - 1] != '\n');
			continue;
		}
		buffer[len - 1] = '\0';

		/* short-circuit comments */
		if (buffer[0] == '#')
			continue;

		/* check if it's what we want */
		if (sscanf(buffer, "%s %s", w, v) > 0 && !strcmp(w, keyword)) {
			/* found it :-) */
			strcpy(value, v);
			close_config_file(in);
			/* tell them we got it */
			return 1;
		}
	}
	close_config_file(in);
	/* didn't find it - better luck next time */
	return 0;
}

/*
 * open_config_file
 *
 * Opens up the PPTPD config file for reading.
 *
 * args: filename - the config filename (eg. '/etc/pptpd.conf')
 *
 * retn: NULL on error, file descriptor on success
 *
 */
static FILE *open_config_file(char *filename)
{
	FILE *in;
	static int first = 1;

	if ((in = fopen(filename, "r")) == NULL) {
		/* Couldn't open config file */
		if (first) {
			perror(filename);
			first = 0;
		}
		return NULL;
	}
	return in;
}

/*
 * close_config_file
 *
 * Closes the PPTPD config file descriptor
 *
 */
static void close_config_file(FILE * in)
{
	fclose(in);
}
