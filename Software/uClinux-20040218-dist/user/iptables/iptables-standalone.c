/*
 * Author: Paul.Russell@rustcorp.com.au and mneuling@radlogic.com.au
 *
 * Based on the ipchains code by Paul Russell and Michael Neuling
 *
 * (C) 2000-2002 by the netfilter coreteam <coreteam@netfilter.org>:
 * 		    Paul 'Rusty' Russell <rusty@rustcorp.com.au>
 * 		    Marc Boucher <marc+nf@mbsi.ca>
 * 		    James Morris <jmorris@intercode.com.au>
 * 		    Harald Welte <laforge@gnumonks.org>
 * 		    Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 *	iptables -- IP firewall administration for kernels with
 *	firewall table (aimed for the 2.3 kernels)
 *
 *	See the accompanying manual page iptables(8) for information
 *	about proper usage of this program.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <iptables.h>

#define MAX_IPTC_HANDLES  6

static int iptables_batch()
{
	static char buf[1024]; /* must hold longest iptables command line */
#define MAX_IPT_ARGS 128
	static char *argv[MAX_IPT_ARGS + 1];
	char *cp;
	int argc, ret = 0;
	char *table[MAX_IPTC_HANDLES];
	iptc_handle_t handle[MAX_IPTC_HANDLES];
	int i;

	for (i = 0; i < MAX_IPTC_HANDLES; i++) {
		table[i] = 0;
		handle[i] = 0;
	}

	while (fgets(buf, sizeof(buf), stdin)) {

		if ((cp = strchr(buf, '\n'))) {
			*cp = '\0';
			if (cp != buf && *(cp - 1) == 0x01)
				*(cp - 1) = 0;
		}

		for (cp = buf, argc = 0; cp && argc < MAX_IPT_ARGS; argc++) {
			argv[argc] = cp;
			if ((cp = strchr(cp, 0x01)))
				*cp++ = '\0';
		}
		if (cp && argc >= MAX_IPT_ARGS) {
			fprintf(stderr, "iptables: too many args for batch processor\n");
			exit(1);
		}
		argv[argc] = NULL;

		ret = do_command(argc, argv, "filter", table, handle, MAX_IPTC_HANDLES) || ret;
	}
	if (ret) {
		for (i = 0; i < MAX_IPTC_HANDLES; i++) {
			if (handle[i]) {
				iptc_commit(&handle[i]);
			}
		}
	}
	return(0);
}


int
main(int argc, char *argv[])
{
	int ret;
	char *table = NULL;
	iptc_handle_t handle = NULL;

	program_name = "iptables";
	program_version = IPTABLES_VERSION;

#ifdef NO_SHARED_LIBS
	init_extensions();
#endif

	if (strstr(argv[0], "iptables-batch"))
		exit(iptables_batch());
	
	ret = do_command(argc, argv, "filter", &table, &handle, 1);
	if (ret)
		ret = iptc_commit(&handle);

	if (!ret)
		fprintf(stderr, "iptables: %s\n",
			iptc_strerror(errno));

	exit(!ret);
}
