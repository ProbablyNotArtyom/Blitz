/*****************************************************************************/
/*
 *	flatfsd.c -- Flat file-system daemon.
 *
 *	(C) Copyright 1999-2001, Greg Ungerer <gerg@snapgear.com>
 *	(C) Copyright 2000-2001, Lineo Inc. (www.lineo.com)
 *	(C) Copyright 2001-2002, SnapGear (www.snapgear.com)
 *	(C) Copyright 2002, David McCullough <davidm@snapgear.com>
 */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include <linux/config.h>
#if defined(CONFIG_LEDMAN)
#include <linux/ledman.h>
#endif

#include "flatfs.h"
#include "reboot.h"

/*****************************************************************************/

#define        FILEFS  "/dev/flash/config"

/*
 *	Globals for file and byte count.
 *
 *	This is a kind of ugly way to do it, but we are using LCP (Least Change Principle)
 */

int	numfiles;
int	numbytes;
int	numdropped;

/*****************************************************************************/
/*
 * The code to do Reset/Erase button menus
 */

static int current_cmd = 0;
static void no_action(void) { }
#ifdef CONFIG_LEDMAN
static void reset_config_fs(void);
#endif

#define MAX_LED_PATTERN 4
#define	ACTION_TIMEOUT 5		/* timeout before action in seconds */

static struct {
	void			(*action)(void);
	unsigned long	led;
	unsigned long	timeout;
} cmd_list[] = {
	{ no_action,		0 , 0},
#ifdef CONFIG_LEDMAN
	{ no_action,		0 , 2},
	{ reset_config_fs,		LEDMAN_RESET, 0},
#endif
	{ NULL,				0 , 0}
};

/*****************************************************************************/

static int recv_hup = 0;	/* SIGHUP = reboot device */
static int recv_usr1 = 0;	/* SIGUSR1 = write config to flash */
static int recv_usr2 = 0;	/* SIGUSR2 = erase flash and reboot */
static int exit_flatfsd = 0;  /* SIGINT, SIGTERM, SIGQUIT */

static void block_sig(int blp)
{
	sigset_t sigs;

	sigemptyset(&sigs);
	sigaddset(&sigs, SIGUSR1);
	sigaddset(&sigs, SIGUSR2);
	sigaddset(&sigs, SIGHUP);
	sigaddset(&sigs, SIGTERM);
	sigaddset(&sigs, SIGINT);
	sigaddset(&sigs, SIGQUIT);
	sigprocmask(blp?SIG_BLOCK:SIG_UNBLOCK, &sigs, NULL);
}

static void sigusr1(int signr)
{
	recv_usr1 = 1;
}

static void sigusr2(int signr)
{
	recv_usr2 = 1;
}

static void sighup(int signr)
{
	recv_hup = 1;
}

static void sigexit(int signr)
{
	exit_flatfsd = 1;
}

/*****************************************************************************/
/*
 * Save the filesystem to flash in flat format for retrieval
 * later
 */

static void save_config_to_flash(void)
{
#if !defined(USING_FLASH_FILESYSTEM)
	int	rc;
#endif
	block_sig(1);

#if !defined(USING_FLASH_FILESYSTEM)
	if ((rc = flatwrite(FILEFS)) < 0) {
		syslog(LOG_ERR, "Failed to write flatfs (%d): %m", rc);
	}
#endif
	block_sig(0);
}

/*****************************************************************************/
/*
 *	Default the config filesystem
 */

#ifdef CONFIG_LEDMAN
static void reset_config_fs(void)
{
	int rc;

	block_sig(1);

	flatclean();
	if ((rc = flatnew(DEFAULTDIR)) < 0) {
		syslog(LOG_ERR, "Failed to create new flatfs from %s (%d): %m", DEFAULTDIR, rc);
		exit(1);
	}
#ifdef LOGGING
	system("/bin/logd resetconfig");
#endif
	save_config_to_flash();

	reboot_now();
	block_sig(0);
}
#endif

/*****************************************************************************/

int creatpidfile()
{
	FILE	*f;
	pid_t	pid;
	char	*pidfile = "/var/run/flatfsd.pid";

	pid = getpid();
	if ((f = fopen(pidfile, "w")) == NULL) {
		syslog(LOG_ERR, "Failed to open %s: %m", pidfile);
		return(-1);
	}
	fprintf(f, "%d\n", pid);
	fclose(f);
	return(0);
}

/*****************************************************************************/

/*
 *	Lodge ourselves with the kernel LED manager. If it gets an
 *	interrupt from the reset switch it will send us a SIGUSR2.
 */

int register_resetpid(void)
{
#if defined(CONFIG_LEDMAN) && defined(LEDMAN_CMD_SIGNAL)
	int	fd;

	if ((fd = open("/dev/ledman", O_RDONLY)) < 0) {
		syslog(LOG_ERR, "Failed to open /dev/ledman: %m");
		return(-1);
	}
	if (ioctl(fd, LEDMAN_CMD_SIGNAL, 0) < 0) {
		syslog(LOG_ERR, "Failed to register pid: %m");
		return(-2);
	}
	close(fd);
#endif
	return(0);
}

/*****************************************************************************/

#define CHECK_FOR_SIG(x) \
	do { usleep(x); if (recv_usr1 || recv_usr2 || recv_hup) goto skip_out; } while(0);

static void
led_pause(void)
{
#if defined(CONFIG_LEDMAN) && defined(LEDMAN_CMD_SIGNAL)

	unsigned long start = time(0);

	ledman_cmd(LEDMAN_CMD_ALT_ON, LEDMAN_ALL); /* all leds on */
	ledman_cmd(LEDMAN_CMD_ON | LEDMAN_CMD_ALTBIT, LEDMAN_ALL); /* all leds on */
	CHECK_FOR_SIG(100000);
	ledman_cmd(LEDMAN_CMD_OFF | LEDMAN_CMD_ALTBIT, LEDMAN_ALL); /* all leds off */
	CHECK_FOR_SIG(100000);
	ledman_cmd(LEDMAN_CMD_ON | LEDMAN_CMD_ALTBIT, cmd_list[current_cmd].led);
	CHECK_FOR_SIG(250000);

	while (time(0) - start < cmd_list[current_cmd].timeout) {
		CHECK_FOR_SIG(250000);
	}

	block_sig(1);
	ledman_cmd(LEDMAN_CMD_ON | LEDMAN_CMD_ALTBIT, LEDMAN_ALL); /* all leds on */
	(*cmd_list[current_cmd].action)();
	block_sig(0);

	current_cmd = 0;
skip_out:
	ledman_cmd(LEDMAN_CMD_RESET | LEDMAN_CMD_ALTBIT, LEDMAN_ALL);
	ledman_cmd(LEDMAN_CMD_ALT_OFF, LEDMAN_ALL); /* all leds on */

#else
	pause();
#endif
}

/*****************************************************************************/

void usage(int rc)
{
	printf("usage: flatfsd [-rwh?]\n");
	exit(rc);
}

/*****************************************************************************/

/**
 * Returns the number of seconds since boot.
 */
static long get_uptime(void)
{
	struct sysinfo si;

	sysinfo(&si);
	return(si.uptime);
}

int main(int argc, char *argv[])
{
	struct sigaction	act;
	int			rc, readonly, clobbercfg;

	/* Interval (in seconds) after boot in which a reset button press
	 *   erases the config. After this interval, a button press will reboot.
	 * 0 indicates than there is no time limit.
	 */
	int erase_time = 0;

	clobbercfg = readonly = 0;

	openlog("flatfsd", LOG_PERROR, LOG_DAEMON);

	if ((rc = getopt(argc, argv, "rwh?")) != EOF) {
		switch(rc) {
		case 'w':
			clobbercfg++;
		case 'r':
			readonly++;
			break;
		case 'h':
		case '?':
			usage(0);
			break;
		default:
			usage(1);
			break;
		}
	}

	{
		/* Pick up some settings from /etc/config/config */

		FILE *fh = fopen("/etc/config/config", "r");
		char buf[80];

		if (fh != 0) {
			while ((fgets(buf, sizeof(buf), fh)) != 0) {
				if (strncmp(buf, "flat_erase ", 11) == 0) {
					erase_time = strtoul(buf + 11, 0, 0);
					break;
				}
			}
			fclose(fh);
		}
	}

	if (readonly) {
		if (clobbercfg ||
#if defined(USING_FLASH_FILESYSTEM)
			((rc = flatfilecount()) <= 0)
#else
			((rc = flatread(FILEFS)) < 0)
#endif
		) {
			syslog(LOG_ERR, "Nonexistent or bad flatfs (%d), creating new one...", rc);
			flatclean();
			if ((rc = flatnew(DEFAULTDIR)) < 0) {
				syslog(LOG_ERR, "Failed to create new flatfs, err=%d errno=%d",
					rc, errno);
				exit(1);
			}
#ifdef LOGGING
			system("/bin/logd badconfig");
#endif
			save_config_to_flash();
		}
		syslog(LOG_INFO, "Created %d configuration files (%d bytes)",
			numfiles, numbytes);
		exit(0);
	}

	/*
	 *	Spin forever, waiting for a signal to write...
	 */
	creatpidfile();

	act.sa_handler = sighup;
	memset(&act.sa_mask, 0, sizeof(act.sa_mask));
	act.sa_flags = SA_RESTART;
	act.sa_restorer = 0;
	sigaction(SIGHUP, &act, NULL);

	act.sa_handler = sigusr1;
	memset(&act.sa_mask, 0, sizeof(act.sa_mask));
	act.sa_flags = SA_RESTART;
	act.sa_restorer = 0;
	sigaction(SIGUSR1, &act, NULL);

	act.sa_handler = sigusr2;
	memset(&act.sa_mask, 0, sizeof(act.sa_mask));
	act.sa_flags = SA_RESTART;
	act.sa_restorer = 0;
	sigaction(SIGUSR2, &act, NULL);

	/* Make sure we don't suddenly exit while we are writing */
	act.sa_handler = sigexit;
	memset(&act.sa_mask, 0, sizeof(act.sa_mask));
	act.sa_flags = SA_RESTART;
	act.sa_restorer = 0;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);

	register_resetpid();

	for (;;) {
		if (recv_usr1) {
			recv_usr1 = 0;
#ifdef LOGGING
			system("/bin/logd writeconfig");
#endif
			save_config_to_flash();
			continue;
		}

		if (recv_hup) {
			/* Make sure we do the check above first so that we commit
			 * to flash before rebooting */
			recv_hup = 0;
			reboot_now();
			/*notreached*/
			exit(1);
		}

		if (recv_usr2) {
#ifdef LOGGING
			system("/bin/logd button");
#endif
			recv_usr2 = 0;
			if (erase_time > 0 && current_cmd == 0 && get_uptime() > erase_time) {
				syslog(LOG_ERR, "Erase pressed more than %d seconds after boot, ignoring", erase_time);
			}
			else {
				current_cmd++;
				if (cmd_list[current_cmd].action == NULL) /* wrap */
					current_cmd = 0;
			}
		}

		if (exit_flatfsd) {
			break;
		}
			
		if (current_cmd) {
			led_pause();
		} else if (!recv_usr1 && !recv_usr2)
			pause();
	}
	exit(0);
}

/*****************************************************************************/
