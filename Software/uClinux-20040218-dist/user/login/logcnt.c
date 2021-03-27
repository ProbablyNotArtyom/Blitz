#include <stdio.h>
#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <config/autoconf.h>
#include <signal.h>

#define AA_COUNTER_FILE	"/etc/config/access_counts"
#define AA_SINGLE_CHARS	"*-+"

#ifdef AA_EXTERN_ONLY
extern void access__attempted(const int denied, const char *const user);
#else
void access__attempted(const int denied, const char *const user) {
#ifdef CONFIG_USER_FLATFSD_FLATFSD
	void killProcess(const char *const pidfile, const int signo) {
		pid_t pid = 0;
		FILE *in;

		if((in = fopen(pidfile, "r")) == NULL)
			return;
		if (fscanf(in, "%d", &pid) != 1) {
			fclose(in);
			return;
		}
		fclose(in);
		if (pid)
			kill(pid, signo);
	}
#endif
	void set_count(const char *const user, const int count) {
		FILE *f, *fnew;
		char buf[50];

		if ((f = fopen(AA_COUNTER_FILE, "r")) == NULL) {
			if (count && (f=fopen(AA_COUNTER_FILE, "w")) != NULL) {
				fprintf(f, "%s %d\n", user, count);
				fclose(f);
			}
			return;
		}
		if ((fnew = fopen(AA_COUNTER_FILE ".tmp", "w")) == NULL) {
			fclose(f);
			return;
		}
		while (fgets(buf, sizeof(buf), f) != NULL) {
			char *const p = strchr(buf, ' ');
			if (p != NULL) {
				*p = '\0';
				if (strcmp(buf, user) == 0)
					continue;
				*p = ' ';
			}
			fputs(buf, fnew);
		}
		if (count)
			fprintf(fnew, "%s %d\n", user, count);
		fclose(f);
		fclose(fnew);
		rename(AA_COUNTER_FILE ".tmp", AA_COUNTER_FILE);
	}
	int get_count(const char *const user) {
		FILE *f = fopen(AA_COUNTER_FILE, "r");
		char buf[50];

		if (f == NULL)
			return 0;
		while (fgets(buf, sizeof(buf), f) != NULL) {
			char *const p = strchr(buf, ' ');
			if (p == NULL) continue;
			*p = '\0';
			if (strcmp(buf, user) == 0) {
				fclose(f);
				return atoi(p+1);
			}
		}
		fclose(f);
		return 0;
	}
	const int max = get_count("-")?:5;
	void bump_count(const char *const user) {
		const int n = get_count(user);
		if (n >= max) {
			system("/bin/logd message access attempt overrun!");
			syslog(LOG_EMERG, "access attempt overrun!");
#if CONFIG_USER_FLASHW_FLASHW
			system("/bin/flashw GARBAGE /dev/flash/config");
#endif
			system("/bin/reboot");
		} else
			set_count(user, n+1);
	}
	void trim_lines(void) {
		FILE *f, *fnew;
		char buf[50];
		int c=get_count("+")?:100;

		if ((f = fopen(AA_COUNTER_FILE, "r")) == NULL)
			return;
		while (fgets(buf, sizeof(buf), f) != NULL)
			if (buf[1] != ' ' || strchr(AA_SINGLE_CHARS, buf[0]) == NULL)
				c--;
		if (c >= 0) {
			fclose(f);
			return;
		}
		if ((fnew = fopen(AA_COUNTER_FILE ".tmp", "w")) == NULL) {
			fclose(f);
			return;
		}
		rewind(f);
		while (fgets(buf, sizeof(buf), f) != NULL)
			if ((buf[1] == ' ' && strchr(AA_SINGLE_CHARS, buf[0]) != NULL) ||
					c++>=0)
				fputs(buf, fnew);
		fclose(f);
		fclose(fnew);
		rename(AA_COUNTER_FILE ".tmp", AA_COUNTER_FILE);
	}

	if (strcmp(user, "-") == 0)
		goto bcom;
	else if (denied) {
		bump_count(user);
bcom:		bump_count("*");
		trim_lines();
	} else {
		if (get_count(user) == 0 && get_count("*") == 0)
			return;
		set_count(user, 0);
		set_count("*", 0);
	}
#ifdef CONFIG_USER_FLATFSD_FLATFSD
	killProcess("/var/run/flatfsd.pid", SIGUSR1);
#endif
}
#endif
