/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

#include <features.h>
#include <linux/version.h>
#include <linux/autoconf.h>

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
#define GETGROUPS_T gid_t

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if your struct stat has st_blksize.  */
#define HAVE_ST_BLKSIZE 1

/* Define if your struct stat has st_blocks.  */
#define HAVE_ST_BLOCKS 1

/* Define if your struct stat has st_rdev.  */
#define HAVE_ST_RDEV 1

/* Define if major, minor, and makedev are declared in <mkdev.h>.  */
/* #undef MAJOR_IN_MKDEV */

/* Define if major, minor, and makedev are declared in <sysmacros.h>.  */
#define MAJOR_IN_SYSMACROS 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
/* #undef STAT_MACROS_BROKEN */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define on System V Release 4.  */
/* #undef SVR4 */

/* Define if `sys_siglist' is declared by <signal.h>.  */
#define SYS_SIGLIST_DECLARED 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define if this is running the Linux operating system.  */
#define LINUX 1

/* Define if this is running the SunOS 4.x operating system.  */
/* #undef SUNOS4 */

/* Define if this is running the System V release 4 operating system
   or a derivative like Solaris 2.x or Irix 5.x.  */
/* #undef SVR4 */

/* Define if this is running the FreeBSD operating system.  */
/* #undef FREEBSD */

/* Define for UnixWare systems. */
/* #undef UNIXWARE */

/* Define if this is an i386, i486 or pentium architecture.  */
/* #undef I386 */

/* Define if this is an ia64 architecture.  */
/* #undef IA64 */

/* Define if this is an m68k architecture.  */
/* #undef M68K */

/* Define if this is a sparc architecture.  */
/* #undef SPARC */

/* Define if this is a mips architecture.  */
/* #undef MIPS */

/* Define if this is an alpha architecture.  */
/* #undef ALPHA */

/* Define if this is an arm architecture.  */
/* #undef ARM */

/* Define if this is a powerpc architecture.  */
/* #undef POWERPC */

/* Define if you have a SVR4 MP type procfs.  I.E. /dev/xxx/ctl,
   /dev/xxx/status.  Also implies that you have the pr_lwp
   member in prstatus. */
/* #undef HAVE_MP_PROCFS */

/* Define if you have SVR4 and the poll system call works on /proc files.  */
/* #undef HAVE_POLLABLE_PROCFS */

/* Define if you have SVR4_MP and you need to use the poll hack
   to avoid unfinished system calls. */
/* #undef POLL_HACK */

/* Define if the prstatus structure in sys/procfs.h has a pr_syscall member.  */
/* #undef HAVE_PR_SYSCALL */

/* Define if you are have a SPARC with SUNOS4 and your want a version
   of strace that will work on sun4, sun4c and sun4m kernel architectures.
   Only useful if you have a symbolic link from machine to /usr/include/sun4
   in the compilation directory. */
/* #undef SUNOS4_KERNEL_ARCH_KLUDGE */

/* Define if signal.h defines the type sig_atomic_t.  */
#define HAVE_SIG_ATOMIC_T 1

/* Define if the msghdr structure has a msg_control member.  */
#define HAVE_MSG_CONTROL 1

/* Define if stat64 is available in asm/stat.h.  */
/* #undef HAVE_STAT64 */

/* Define if your compiler knows about long long */
#define HAVE_LONG_LONG 1

/* Define if off_t is a long long */
/* #undef HAVE_LONG_LONG_OFF_T */

/* Define if rlim_t is a long long */
/* #undef HAVE_LONG_LONG_RLIM_T */

/* Define if struct sockaddr_in6 contains sin6_scope_id field. */
/* #undef HAVE_SIN6_SCOPE_ID */

/* Define if linux struct sockaddr_in6 contains sin6_scope_id fiels. */
/* #undef HAVE_SIN6_SCOPE_ID_LINUX */

/* Define if have st_flags in struct stat */
/* #undef HAVE_ST_FLAGS */

/* Define if have st_aclcnt in struct stat */
/* #undef HAVE_ST_ACLCNT */

/* Define if have st_level in struct stat */
/* #undef HAVE_ST_LEVEL */

/* Define if have st_fstype in struct stat */
/* #undef HAVE_ST_FSTYPE */

/* Define if have st_gen in struct stat */
/* #undef HAVE_ST_GEN */

/* Define if you have the _sys_siglist function.  */
/* #undef HAVE__SYS_SIGLIST */

/* Define if you have the getdents function.  */
#define HAVE_GETDENTS

/* Define if you have the if_indextoname function.  */
#define HAVE_IF_INDEXTONAME 1

/* Define if you have the inet_ntop function.  */
#define HAVE_INET_NTOP 1

/* Define if you have the mctl function.  */
/* #undef HAVE_MCTL */

/* Define if you have the prctl function.  */
/* #undef HAVE_PRCTL */

/* Define if you have the pread function.  */
#define HAVE_PREAD 1

/* Define if you have the putpmsg function.  */
#define HAVE_PUTPMSG 1

/* Define if you have the sendmsg function.  */
#define HAVE_SENDMSG 1

/* Define if you have the sigaction function.  */
#define HAVE_SIGACTION 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strsignal function.  */
#ifdef __GLIBC__
#define HAVE_STRSIGNAL 1
#endif

/* Define if you have the sys_siglist function.  */
#define HAVE_SYS_SIGLIST 1

/* Define if you have the <asm/reg.h> header file.  */
/* #undef HAVE_ASM_REG_H */

/* Define if you have the <asm/sigcontext.h> header file.  */
#define HAVE_ASM_SIGCONTEXT_H 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <ioctls.h> header file.  */
/* #undef HAVE_IOCTLS_H */

/* Define if you have the <linux/icmp.h> header file.  */
#define HAVE_LINUX_ICMP_H 1

/* Define if you have the <linux/if_packet.h> header file.  */
#define HAVE_LINUX_IF_PACKET_H 1

/* Define if you have the <linux/in6.h> header file.  */
#if LINUX_VERSION_CODE >= 0x020400
#define HAVE_LINUX_IN6_H
#endif

/* Define if you have the <linux/netlink.h> header file.  */
#if LINUX_VERSION_CODE >= 0x020100
#define HAVE_LINUX_NETLINK_H 1
#endif

/* Define if you have the <linux/ptrace.h> header file.  */
#define HAVE_LINUX_PTRACE_H

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <netinet/tcp.h> header file.  */
#define HAVE_NETINET_TCP_H 1

/* Define if you have the <netinet/udp.h> header file.  */
#define HAVE_NETINET_UDP_H 1

/* Define if you have the <sys/acl.h> header file.  */
/* #undef HAVE_SYS_ACL_H */

/* Define if you have the <sys/aio.h> header file.  */
/* #undef HAVE_SYS_AIO_H */

/* Define if you have the <sys/asynch.h> header file.  */
/* #undef HAVE_SYS_ASYNCH_H */

/* Define if you have the <sys/dir.h> header file.  */
#define HAVE_SYS_DIR_H

/* Define if you have the <sys/door.h> header file.  */
/* #undef HAVE_SYS_DOOR_H */

/* Define if you have the <sys/filio.h> header file.  */
/* #undef HAVE_SYS_FILIO_H */

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/poll.h> header file.  */
/* #define HAVE_SYS_POLL_H */

/* Define if you have the <sys/ptrace.h> header file.  */
#ifndef __UC_LIBC__
#define HAVE_SYS_PTRACE_H 1
#endif

/* Define if you have the <sys/reg.h> header file.  */
/* #undef HAVE_SYS_REG_H */

/* Define if you have the <sys/stream.h> header file.  */
/* #undef HAVE_SYS_STREAM_H */

/* Define if you have the <sys/sysconfig.h> header file.  */
/* #undef HAVE_SYS_SYSCONFIG_H */

/* Define if you have the <sys/tiuser.h> header file.  */
/* #undef HAVE_SYS_TIUSER_H */

/* Define if you have the <sys/uio.h> header file.  */
#define HAVE_SYS_UIO_H 1

/* Define if you have the <sys/vfs.h> header file.  */
#define HAVE_SYS_VFS_H 1

/* Define if you have the <termio.h> header file.  */
#define HAVE_TERMIO_H 1

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the <sys/quota.h> header file.  */
#if defined(__GLIBC__) && !defined(__UC_LIBC__) && LINUX_VERSION_CODE >= 0x020100
#define HAVE_SYS_QUOTA_H 1
#endif
