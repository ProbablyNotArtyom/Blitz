/* nettel.c -- NETtel specific functions for the DHCP server */
#ifdef CONFIG_NETtel

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <config/autoconf.h>

#include "dhcpd.h"


int commitChanges() {
#ifdef CONFIG_USER_FLATFSD_FLATFSD
        char value[16];
        pid_t pid;
        FILE *in;

        /* get the pid of flatfsd */
        if ((in = fopen("/var/run/flatfsd.pid", "r")) == NULL)
                return -1;

        if (fread(value, 1, sizeof(value), in) <= 0) {
                fclose(in);
                return -1;
        }
        fclose(in);

		pid = atoi(value);

        if (pid == 0 || kill(pid, 10) == -1)
                return -1;
#endif
        return 0;
}


int route_add_host(int type) {
        pid_t pid;
        char *argv[16];
        int s, argc = 0;

        /* route add -host 255.255.255.255 ethX */
        if((pid = vfork()) == 0) { /* child */
                argv[argc++] = "/bin/route";
                if(type == ADD)
                        argv[argc++] = "add";
                else if(type == DEL)
                        argv[argc++] = "del";
                argv[argc++] = "-host";
                argv[argc++] = "255.255.255.255";
                argv[argc++] = interface_name;
                argv[argc] = NULL;
                execvp("/bin/route", argv);
                exit(0);
        } else if (pid > 0) {
                waitpid(pid, &s, 0);
	}
        return 0;
}

#endif
