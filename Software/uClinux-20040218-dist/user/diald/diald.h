/*
 * diald.h - Main header file.
 *
 * Copyright (c) 1994, 1995, 1996 Eric Schenk.
 * All rights reserved. Please see the file LICENSE which should be
 * distributed with this software for terms of use.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/termios.h>
#include <sys/bitypes.h>
#include <net/if.h>
#include <netdb.h>

#ifndef __USE_MISC
#define __USE_MISC 1
#endif
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if_slip.h>

#include <linux/version.h>
#include <config/autoconf.h>
/* This only exists in kernels >= 1.3.75 */
#if LINUX_VERSION_CODE >= 66379
#define HAS_SOCKADDR_PKT
#include <net/if_packet.h>
#define SOCKADDR sockaddr_pkt
#else
#include <sys/socket.h>
#define SOCKADDR sockaddr
#endif

#include "config.h"
#include "fsm.h"
#include "timer.h"
#include "firewall.h"
#include "bufio.h"

#define LOG_DDIAL	LOG_LOCAL2

/* SLIP special character codes */
#define END             0300    /* indicates end of packet */
#define ESC             0333    /* indicates byte stuffing */
#define ESC_END         0334    /* ESC ESC_END means END data byte */
#define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */

/* Operation modes */
#define MODE_SLIP 0
#define MODE_PPP 1
#define MODE_DEV 2


/* Dynamic slip interpretation modes */
#define DMODE_REMOTE 0
#define DMODE_LOCAL 1
#define DMODE_REMOTE_LOCAL 2
#define DMODE_LOCAL_REMOTE 3
#define DMODE_BOOTP 4

/* Define DEBUG flags */
#define DEBUG_FILTER_MATCH	0x0001
#define DEBUG_PROXYARP		0x0004
#define DEBUG_VERBOSE		0x0008
#define DEBUG_STATE_CONTROL	0x0010
#define DEBUG_TICK		0x0020
#define DEBUG_CONNECTION_QUEUE	0x0040

/* Define MONITOR flags */
#define MONITOR_STATE		0x0001
#define MONITOR_INTERFACE	0x0002
#define MONITOR_STATUS		0x0004
#define MONITOR_LOAD		0x0008
#define MONITOR_MESSAGE		0x0010
#define MONITOR_QUEUE		0x0020

/*
 * If you choose UNSAFE_ROUTING=0, then by default diald will route all
 * outgoing packets to the proxy device and forward them to the real
 * device by itself. This has the advantage that it gets around a bug
 * in the current production release (1.2.X) linux kernels
 * that causes TCP sessions to lock up if the route is changed while
 * a packet is being retransmitted. However, this introduces quite
 * a bit of overhead on outgoing packets (10-20%). (Note that incoming packets
 * don't go through this process!)
 * If you choose UNSAFE_ROUTING=1, then diald will change the routes anyway.
 * If you are using diald on a machine were there is a single outgoing
 * link this is perfectly safe (I think!), but if diald is being used
 * in an environment where more than one ppp or slip link can be active
 * at a time, then there is a small chance that you can lock up TCP
 * sessions. In particular if the link is terminated (either by diald
 * or by the other end hanging up) when a TCP session is in the middle
 * of retransmitting a packet, then that TCP session can become locked
 * if when the link comes back up it is brought back up on a different
 * ppp or slip device.
 * [NOTE: As of linux 1.3.13, the Linux kernel has been
 *  fixed to allow routing changes under an active TCP retransmit,
 *  so with 1.3.13 and later UNSAFE_ROUTING as the default is perfectly safe.]
 * [NOTE 2: unlike previous versions of diald, this option can now be
 * controlled from the command line or configuration file. See the new
 * options "reroute" and "-reroute".
 */

#define UNSAFE_ROUTING 1	/* do rerouting by default */

/*
 * Originally diald just threw away any packets it received when
 * the link was down. This is OK because IP is an unreliable protocol,
 * so applications will resend packets when the link comes back up.
 * On the other hand the kernel doubles the timeout for TCP packets
 * every time a send fails. If you define BUFFER_PACKETS diald
 * will store packets that come along when the link is down and
 * send them as soon as the link comes up. This should speed up
 * the initial connections a bit.
 */

#define BUFFER_PACKETS 1	/* turn on packet buffering code. */
#ifdef EMBED
#define BUFFER_SIZE 65500	/* smaller to allow for malloc overhead */
#else
#define BUFFER_SIZE 65536	/* size of buffer to store packets */
#endif
#define BUFFER_FIFO_DISPOSE 1	/* dispose of old packets to make room
				 * for new packets if the buffer becomes
				 * full. Without this option new packets
				 * are discarded if there is no room.
				 */
#define BUFFER_TIMEOUT 600	/* Maximum number of seconds to keep a
				 * packet in the buffer. Don't make this
				 * too large or you will break IP.
				 * (Something on the order of 1 hour
				 * probably the maximum safe value.
				 * I expect that the 10 minutes defined
				 * by default should be plenty.
				 */

/*
 * Various timeouts and times used in diald.
 */

#define PAUSETIME 1	/* how many seconds should diald sleep each time
			   it checks to see what's happening. Note that
			   this is a maximum time and that a packet
			   arriving will cut the nap short. */
#define DEFAULT_FIRST_PACKET_TIMEOUT 120
#define DEFAULT_DIAL_DELAY 30
#define DEFAULT_MTU 1500
#define DEFAULT_SPEED 38400

typedef struct monitors {
    struct monitors *next;
    int fd;			/* monitor output fp. */
    int level;			/* Information level requested */
    char *name;
} MONITORS;

/* Configuration variables */

char **devices;
int device_count;
char device[10];
char device_node[9];
int device_iface;
int inspeed;
int window;
int mtu;
int mru;
char *connector;
char *disconnector;
char *orig_local_ip;
char *orig_remote_ip;
char *local_ip;
unsigned long local_addr;
char *remote_ip;
char *netmask;
char *addroute;
char *delroute;
char *ip_up;
char *ip_down;
char *acctlog;
char *pidlog;
char *fifoname;
char *lock_prefix;
int pidstring;
char *run_prefix;
char *diald_config_file;
char *diald_defs_file;
char *path_route;
char *path_ifconfig;
char *path_bootpc;
char *path_pppd;
int buffer_packets;
int buffer_size;
int buffer_fifo_dispose;
int buffer_timeout;
FILE *acctfp;
int call_start_time;
int mode;
int debug;
int modem;
int rotate_devices;
int crtscts;
int dodaemon;
int dynamic_addrs;
int dynamic_mode;
int slip_encap;
int lock_dev;
int default_route;
int pppd_argc;
char **pppd_argv;
int connect_timeout;
int disconnect_timeout;
int redial_timeout;
int nodev_retry_timeout;
int stop_dial_timeout;
int kill_timeout;
int start_pppd_timeout;
int stop_pppd_timeout;
int first_packet_timeout;
int retry_count;
int died_retry_count;
int redial_backoff_start;
int redial_backoff_limit;
int redial_backoff;
int dial_fail_limit;
int two_way;
int give_way;
int do_reroute;
int proxyarp;
int route_wait;
int metric;
int drmetric;

#ifdef SIOCSKEEPALIVE
extern int keepalive;
#endif

#ifdef SIOCSOUTFILL
extern int outfill;
#endif

/* Global variables */

int fifo_fd;			/* FIFO command pipe. */
MONITORS *monitors;		/* List of monitor pipes. */
int proxy_mfd;			/* master pty fd */
FILE *proxy_mfp;		/* also have an fp. Hackery for recv_packet. */
int proxy_sfd;			/* slave pty fd */
int modem_fd;			/* modem device fp (for slip links) */
char packet[4096];		/* slip packet buffer */
int modem_hup;			/* have we seen a modem HUP? */
int request_down;		/* has the user requested link down? */
int request_up;			/* has the user requested link up? */
int forced;			/* has the user requested the link forced up? */
int link_pid;			/* current pppd command pid */
int dial_pid;			/* current dial command pid */
int running_pid;		/* current system command pid */
int dial_status;		/* status from last dial command */
int state_timeout;		/* state machine timeout counter */
int blocked;			/* user has blocked the link */
int state;			/* DFA state */
int current_retry_count;	/* current retry count */
int proxy_iface;		/* Interface number for proxy pty */
int link_iface;			/* Interface number for ppp line */
int orig_disc;			/* original PTY line disciple */
int fwdfd;			/* control socket for packet forwarding */
int snoopfd;			/* snooping socket fd */
int fwunit;			/* firewall unit for firewall control */
int req_pid;			/* pid of process that made "request" */
char *req_dev;			/* name of the device file requested to open */
int use_req;			/* are we actually using the FIFO link-up request device? */
char snoop_dev[10];		/* The interface name we are listening on */
int txtotal,rxtotal;		/* transfer stats for the link */
int itxtotal, irxtotal;		/* instantaneous transfer stats */
int delayed_quit;		/* has the user requested delayed termination?*/
int terminate;			/* has the user requested termination? */
int impulse_time;		/* time for current impulses */
int impulse_init_time;		/* initial time for current impulses */
int impulse_fuzz;		/* fuzz for current impulses */
char *pidfile;			/* full path filename of pid file */
int force_dynamic;		/* 1 if the current connect passed back addrs */
int redial_rtimeout;		/* current real redial timeout */
int dial_failures;		/* number of dial failures since last success */
int ppp_half_dead;		/* is the ppp link half dead? */

/* function prototypes */
void init_vars(void);
void parse_init(void);
void parse_options_file(char *);
void parse_args(int, char *[]);
void check_setup(void);
void signal_setup(void);
void default_sigacts(void);
void block_signals(void);
void unblock_signals(void);
void filter_setup(void);
void get_pty(int *, int *);
void proxy_up(void);
void proxy_down(void);
void proxy_config(char *, char *);
void dynamic_slip(void);
void idle_filter_proxy(void);
void open_fifo(void);
void filter_read(void);
void fifo_read(void);
void proxy_read(void);
void modem_read(void);
void advance_filter_queue(void);
void alrm_timer(int);
int recv_packet(unsigned char *, int);
void sig_hup(int);
void sig_intr(int);
void sig_term(int);
void sig_io(int);
void sig_chld(int);
void sig_pipe(int);
void linkup(int);
void die(int);
void print_filter_queue(int);
void monitor_queue(void);
void become_daemon(void);
void change_state(void);
void output_state(void);
void add_device(void *, char **);
void set_str(char **, char **);
void set_int(int *, char **);
void set_flag(int *, char **);
void clear_flag(int *, char **);
void set_mode(char **, char **);
void set_dslip_mode(char **, char **);
void read_config_file(int *, char **);
void add_filter(void *var, char **);
int insert_packet(unsigned char *, int);
int lock(char *dev);
void unlock(void);
void fork_dialer(char *, int);
void flush_timeout_queue(void);
void set_up_tty(int, int, int);
void flush_prules(void);
void flush_filters(void);
void flush_vars(void);
void parse_impulse(void *var, char **argv);
void parse_restrict(void *var, char **argv);
void parse_or_restrict(void *var, char **argv);
void parse_bringup(void *var, char **argv);
void parse_keepup(void *var, char **argv);
void parse_accept(void *var, char **argv);
void parse_ignore(void *var, char **argv);
void parse_wait(void *var, char **argv);
void parse_up(void *var, char **argv);
void parse_down(void *var, char **argv);
void parse_prule(void *var, char **argv);
void parse_var(void *var, char **argv);
void close_modem(void);
int open_modem (void);
void reopen_modem (void);
void finish_dial(void);
void ppp_start(void);
int ppp_set_addrs(void);
int ppp_dead(void);
int ppp_route_exists(void);
void ppp_stop(void);
void ppp_reroute(void);
void ppp_kill(void);
void ppp_zombie(void);
int ppp_rx_count(void);
void slip_start(void);
int slip_set_addrs(void);
int slip_dead(void);
void slip_stop(void);
void slip_reroute(void);
void slip_kill(void);
void slip_zombie(void);
int slip_rx_count(void);
void dev_start(void);
int dev_set_addrs(void);
int dev_dead(void);
void dev_stop(void);
void dev_reroute(void);
void dev_kill(void);
void dev_zombie(void);
int dev_rx_count(void);
void idle_filter_init(void);
void interface_up(void);
void interface_down(void);
void buffer_init(int *, char **);
int queue_empty(void);
int fw_wait(void);
int fw_reset_wait(void);
int next_alarm(void);
void buffer_packet(unsigned int,unsigned char *);
void forward_buffer(void);
void run_ip_up(void);
void run_ip_down(void);
void set_ptp(char *, int, char *, int);
void add_routes(char *, int, char *, char *, int);
void del_routes(char *, int, char *, char *, int);
void pipe_init(int, PIPE *);
int pipe_read(PIPE *);
void pipe_flush(PIPE *, int);
int set_proxyarp (unsigned int);
int clear_proxyarp (unsigned int);
int report_system_result(int,char *);
void mon_write(int,char *,int);
void background_system(const char *);
void block_timer();
void unblock_timer();

