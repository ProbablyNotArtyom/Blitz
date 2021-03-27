/*
 * Mini xargs implementation for busybox
 *
 * Copyright (C) 1999,2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 1999,2000,2001 by Erik Andersen <andersee@debian.org>
 * Remixed by Mark Whitley <markw@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "busybox.h"

int xargs_main(int argc, char **argv)
{
	char *cmd_to_be_executed;
	char *file_to_act_on;
	int i;
	int len;

	/*
	 * No options are supported in this version of xargs; no getopt.
	 *
	 * Re: The missing -t flag: Most programs that produce output also print
	 * the filename, so xargs doesn't really need to do it again. Supporting
	 * the -t flag =greatly= bloats up the size of this app and the memory it
	 * uses because you have to buffer all the input file strings in memory. If
	 * you really want to see the filenames that xargs will act on, just run it
	 * once with no args and xargs will echo the filename. Simple.
	 */

	argv++;
	len = argc;     /* arg = count for ' ' + trailing '\0' */
	/* Store the command to be executed (taken from the command line) */
	if (argc == 1) {
		/* default behavior is to echo all the filenames */
		argv[0] = "/bin/echo";
		len++;  /* space for trailing '\0' */
	} else {
		argc--;
		}
	/* concatenate all the arguments passed to xargs together */
	for (i = 0; i < argc; i++)
		len += strlen(argv[i]);
	cmd_to_be_executed = xmalloc (len);
	for (i = len = 0; i < argc; i++) {
		len = sprintf(cmd_to_be_executed + len, "%s ", argv[i]);
	}

	/* Now, read in one line at a time from stdin, and store this 
	 * line to be used later as an argument to the command */
	while ((file_to_act_on = get_line_from_file(stdin)) !=NULL) {

		FILE *cmd_output;
		char *output_line;
		char *execstr;

		/* eat the newline off the filename. */
		chomp(file_to_act_on);

		/* eat blank lines */
		if (file_to_act_on[0] == 0)
			continue;

		/* assemble the command and execute it */
		execstr = xcalloc(strlen(cmd_to_be_executed) +
				strlen(file_to_act_on) + 1, sizeof(char));
		strcat(execstr, cmd_to_be_executed);
		strcat(execstr, file_to_act_on);
		
		cmd_output = popen(execstr, "r");
		if (cmd_output == NULL)
			perror_msg_and_die("popen");

		/* harvest the output */
		while ((output_line = get_line_from_file(cmd_output)) != NULL) {
			fputs(output_line, stdout);
			free(output_line);
		}

		/* clean up */
		pclose(cmd_output);
		free(execstr);
		free(file_to_act_on);
	}

#ifdef BB_FEATURE_CLEAN_UP
	free(cmd_to_be_executed);
#endif

	return 0;
}

/* vi: set sw=4 ts=4: */
