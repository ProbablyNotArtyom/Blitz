/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* #undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
/* #undef HAVE_TZNAME */

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define if on MINIX.  */
/* #undef _MINIX */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
/* #undef TM_IN_SYS_TIME */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define if your <unistd.h> contains bad prototypes for exec*()
   (as it does on SGI IRIX 4.x) */
/* #undef BAD_EXEC_PROTOTYPES */

/* Define if your compiler botches static forward declarations
   (as it does on SCI ODT 3.0) */
/* #undef BAD_STATIC_FORWARD */

/* Define for AIX if your compiler is a genuine IBM xlC/xlC_r
   and you want support for AIX C++ shared extension modules. */
/* #undef AIX_GENUINE_CPLUSPLUS */

/* Define this if you have BeOS threads */
/* #undef BEOS_THREADS */

/* Define if you have the Mach cthreads package */
/* #undef C_THREADS */

/* Define to `long' if <time.h> doesn't define.  */
/* #undef clock_t */

/* Define if getpgrp() must be called as getpgrp(0). */
/* #undef GETPGRP_HAVE_ARG */

/* Define if gettimeofday() does not have second (timezone) argument
   This is the case on Motorola V4 (R40V4.2) */
/* #undef GETTIMEOFDAY_NO_TZ */

/* Define this if your time.h defines altzone */
/* #undef HAVE_ALTZONE */

/* Define this if you have some version of gethostbyname_r() */
#define HAVE_GETHOSTBYNAME_R 1

/* Define this if you have the 3-arg version of gethostbyname_r() */
/* #undef HAVE_GETHOSTBYNAME_R_3_ARG */

/* Define this if you have the 5-arg version of gethostbyname_r() */
/* #undef HAVE_GETHOSTBYNAME_R_5_ARG */

/* Define this if you have the 6-arg version of gethostbyname_r() */
#define HAVE_GETHOSTBYNAME_R_6_ARG 1

/* Define this if you have the type long long */
//#undef HAVE_LONG_LONG

/* Define this if you have the type uintptr_t */
/* #undef HAVE_UINTPTR_T */

/* Define if your compiler supports function prototypes */
#define HAVE_PROTOTYPES 1

/* Define if you have GNU PTH threads */
/* #undef HAVE_PTH */

/* Define if your compiler supports variable length function prototypes
   (e.g. void fprintf(FILE *, char *, ...);) *and* <stdarg.h> */
#define HAVE_STDARG_PROTOTYPES 1

/* Define if malloc(0) returns a NULL pointer */
#define MALLOC_ZERO_RETURNS_NULL 1

/* Define if you have POSIX threads */
/* #undef _POSIX_THREADS */

/* Define to force use of thread-safe errno, h_errno, and other functions */
#define _REENTRANT 1

/* Define if setpgrp() must be called as setpgrp(0, 0). */
/* #undef SETPGRP_HAVE_ARG */

/* Define to empty if the keyword does not work.  */
/* #undef signed */

/* Define to `int' if <sys/types.h> doesn't define.  */
#define socklen_t int

/* Define if  you can safely include both <sys/select.h> and <sys/time.h>
   (which you can't on SCO ODT 3.0). */
#define SYS_SELECT_WITH_SYS_TIME 1

/* Define if a va_list is an array of some kind */
/* #undef VA_LIST_IS_ARRAY */

/* Define to empty if the keyword does not work.  */
/* #undef volatile */

/* Define if you want SIGFPE handled (see Include/pyfpe.h). */
/* #undef WANT_SIGFPE_HANDLER */

/* Define if the compiler provides a wchar.h header file. */
/* #undef HAVE_WCHAR_H */

/* Define if you have a useable wchar_t type defined in wchar.h; useable
   means wchar_t must be 16-bit unsigned type. (see
   Include/unicodeobject.h). */
/* #undef HAVE_USABLE_WCHAR_T */

/* Define if you want wctype.h functions to be used instead of the
   one supplied by Python itself. (see Include/unicodectype.h). */
/* #undef WANT_WCTYPE_FUNCTIONS */

/* Define if you want to use SGI (IRIX 4) dynamic linking.
   This requires the "dl" library by Jack Jansen,
   ftp://ftp.cwi.nl/pub/dynload/dl-1.6.tar.Z.
   Don't bother on IRIX 5, it already has dynamic linking using SunOS
   style shared libraries */ 
/* #undef WITH_SGI_DL */

/* Define if you want to emulate SGI (IRIX 4) dynamic linking.
   This is rumoured to work on VAX (Ultrix), Sun3 (SunOS 3.4),
   Sequent Symmetry (Dynix), and Atari ST.
   This requires the "dl-dld" library,
   ftp://ftp.cwi.nl/pub/dynload/dl-dld-1.1.tar.Z,
   as well as the "GNU dld" library,
   ftp://ftp.cwi.nl/pub/dynload/dld-3.2.3.tar.Z.
   Don't bother on SunOS 4 or 5, they already have dynamic linking using
   shared libraries */ 
/* #undef WITH_DL_DLD */

/* Define if you want to use the new-style (Openstep, Rhapsody, MacOS)
   dynamic linker (dyld) instead of the old-style (NextStep) dynamic
   linker (rld). Dyld is necessary to support frameworks. */
/* #undef WITH_DYLD */

/* Define if you want to compile in rudimentary thread support */
/* #undef WITH_THREAD */

/* Define if you want to compile in cycle garbage collection */
#define WITH_CYCLE_GC 1

/* Define if you want to produce an OpenStep/Rhapsody framework
   (shared library plus accessory files). */
/* #undef WITH_NEXT_FRAMEWORK */

/* Define if you want to use BSD db. */
/* #undef WITH_LIBDB */

/* Define if you want to build an interpreter with many run-time checks  */
/* #undef Py_DEBUG */

/* The number of bytes in an off_t. */
#define SIZEOF_OFF_T 4

/* The number of bytes in a time_t. */
#define SIZEOF_TIME_T 4

/* The number of bytes in a pthread_t. */
#define SIZEOF_PTHREAD_T 4

/* Defined to enable large file support when an off_t is bigger than a long
   and long long is available and at least as big as an off_t. You may need
   to add some flags for configuration and compilation to enable this mode.
   E.g, for Solaris 2.7:
   CFLAGS="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64" OPT="-O2 $CFLAGS" \
 configure
*/
/* #undef HAVE_LARGEFILE_SUPPORT */

/* Defined when any dynamic module loading is enabled */
/* #undef HAVE_DYNAMIC_LOADING */

/* Define if i>>j for signed int i does not extend the sign bit
   when i < 0
*/
/* #undef SIGNED_RIGHT_SHIFT_ZERO_FILLS */

/* The number of bytes in a char.  */
#define SIZEOF_CHAR 1

/* The number of bytes in a double.  */
#define SIZEOF_DOUBLE 8

/* The number of bytes in a float.  */
#define SIZEOF_FLOAT 4

/* The number of bytes in a fpos_t.  */
#define SIZEOF_FPOS_T 4

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in a long long.  */
#define SIZEOF_LONG_LONG 8

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* The number of bytes in a uintptr_t.  */
/* #undef SIZEOF_UINTPTR_T */

/* The number of bytes in a void *.  */
#define SIZEOF_VOID_P 4

/* Define if you have the _getpty function.  */
/* #undef HAVE__GETPTY */

/* Define if you have the alarm function.  */
#define HAVE_ALARM 1

/* Define if you have the chown function.  */
#define HAVE_CHOWN 1

/* Define if you have the clock function.  */
/* #undef HAVE_CLOCK */

/* Define if you have the confstr function.  */
/* #undef HAVE_CONFSTR */

/* Define if you have the ctermid function.  */
/* #undef HAVE_CTERMID */

/* Define if you have the ctermid_r function.  */
/* #undef HAVE_CTERMID_R */

/* Define if you have the dlopen function.  */
/* #undef HAVE_DLOPEN */

/* Define if you have the dup2 function.  */
/* #undef HAVE_DUP2 */

/* Define if you have the execv function.  */
/* #undef HAVE_EXECV */

/* Define if you have the fdatasync function.  */
/* #undef HAVE_FDATASYNC */

/* Define if you have the flock function.  */
#define HAVE_FLOCK 1

/* Define if you have the fork function.  */
/* #undef HAVE_FORK */

/* Define if you have the forkpty function.  */
/* #undef HAVE_FORKPTY */

/* Define if you have the fpathconf function.  */
/* #undef HAVE_FPATHCONF */

/* Define if you have the fseek64 function.  */
/* #undef HAVE_FSEEK64 */

/* Define if you have the fseeko function.  */
/* #undef HAVE_FSEEKO */

/* Define if you have the fstatvfs function.  */
/* #undef HAVE_FSTATVFS */

/* Define if you have the fsync function.  */
#define HAVE_FSYNC 1

/* Define if you have the ftell64 function.  */
/* #undef HAVE_FTELL64 */

/* Define if you have the ftello function.  */
#define HAVE_FTELLO 1

/* Define if you have the ftime function.  */
/* #undef HAVE_FTIME */

/* Define if you have the ftruncate function.  */
/* #undef HAVE_FTRUNCATE */

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getgroups function.  */
#define HAVE_GETGROUPS 1

/* Define if you have the gethostbyname function.  */
/* #undef HAVE_GETHOSTBYNAME */

/* Define if you have the getlogin function.  */
/* #undef HAVE_GETLOGIN */

/* Define if you have the getpeername function.  */
#define HAVE_GETPEERNAME 1

/* Define if you have the getpgrp function.  */
#define HAVE_GETPGRP 1

/* Define if you have the getpid function.  */
/* #undef HAVE_GETPID */

/* Define if you have the getpwent function.  */
#define HAVE_GETPWENT 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the getwd function.  */
#define HAVE_GETWD 1

/* Define if you have the hypot function.  */
#define HAVE_HYPOT 1

/* Define if you have the kill function.  */
#define HAVE_KILL 1

/* Define if you have the link function.  */
#define HAVE_LINK 1

/* Define if you have the lstat function.  */
#define HAVE_LSTAT 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the mkfifo function.  */
#define HAVE_MKFIFO 1

/* Define if you have the mktime function.  */
#define HAVE_MKTIME 1

/* Define if you have the mremap function.  */
#define HAVE_MREMAP 1

/* Define if you have the nice function.  */
/* #undef HAVE_NICE */

/* Define if you have the openpty function.  */
/* #undef HAVE_OPENPTY */

/* Define if you have the pathconf function.  */
/* #undef HAVE_PATHCONF */

/* Define if you have the pause function.  */
#define HAVE_PAUSE 1

/* Define if you have the plock function.  */
/* #undef HAVE_PLOCK */

/* Define if you have the poll function.  */
/* #undef HAVE_POLL */

/* Define if you have the pthread_init function.  */
/* #undef HAVE_PTHREAD_INIT */

/* Define if you have the putenv function.  */
/* #undef HAVE_PUTENV */

/* Define if you have the readlink function.  */
#define HAVE_READLINK 1

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the setegid function.  */
/* #undef HAVE_SETEGID */

/* Define if you have the seteuid function.  */
/* #undef HAVE_SETEUID */

/* Define if you have the setgid function.  */
/* #undef HAVE_SETGID */

/* Define if you have the setlocale function.  */
/* #undef HAVE_SETLOCALE */

/* Define if you have the setpgid function.  */
/* #undef HAVE_SETPGID */

/* Define if you have the setpgrp function.  */
/* #undef HAVE_SETPGRP */

/* Define if you have the setregid function.  */
/* #undef HAVE_SETREGID */

/* Define if you have the setreuid function.  */
/* #undef HAVE_SETREUID */

/* Define if you have the setsid function.  */
/* #undef HAVE_SETSID */

/* Define if you have the setuid function.  */
/* #undef HAVE_SETUID */

/* Define if you have the setvbuf function.  */
/* #undef HAVE_SETVBUF */

/* Define if you have the sigaction function.  */
#define HAVE_SIGACTION 1

/* Define if you have the siginterrupt function.  */
/* #undef HAVE_SIGINTERRUPT */

/* Define if you have the sigrelse function.  */
#define HAVE_SIGRELSE 1

/* Define if you have the statvfs function.  */
/* #undef HAVE_STATVFS */

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
/* #undef HAVE_STRFTIME */

/* Define if you have the strptime function.  */
/* #undef HAVE_STRPTIME */

/* Define if you have the symlink function.  */
#define HAVE_SYMLINK 1

/* Define if you have the sysconf function.  */
#define HAVE_SYSCONF 1

/* Define if you have the tcgetpgrp function.  */
#define HAVE_TCGETPGRP 1

/* Define if you have the tcsetpgrp function.  */
#define HAVE_TCSETPGRP 1

/* Define if you have the tempnam function.  */
/* #undef HAVE_TEMPNAM */

/* Define if you have the timegm function.  */
#define HAVE_TIMEGM 1

/* Define if you have the times function.  */
#define HAVE_TIMES 1

/* Define if you have the tmpfile function.  */
/* #undef HAVE_TMPFILE */

/* Define if you have the tmpnam function.  */
/* #undef HAVE_TMPNAM */

/* Define if you have the tmpnam_r function.  */
/* #undef HAVE_TMPNAM_R */

/* Define if you have the truncate function.  */
#define HAVE_TRUNCATE 1

/* Define if you have the uname function.  */
#define HAVE_UNAME 1

/* Define if you have the waitpid function.  */
#define HAVE_WAITPID 1

/* Define if you have the <db.h> header file.  */
/* #undef HAVE_DB_H */

/* Define if you have the <db1/ndbm.h> header file.  */
/* #undef HAVE_DB1_NDBM_H */

/* Define if you have the <db_185.h> header file.  */
/* #undef HAVE_DB_185_H */

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <dlfcn.h> header file.  */
#define HAVE_DLFCN_H 1

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <gdbm/ndbm.h> header file.  */
/* #undef HAVE_GDBM_NDBM_H */

/* Define if you have the <libutil.h> header file.  */
/* #undef HAVE_LIBUTIL_H */

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
/* #undef HAVE_LOCALE_H */

/* Define if you have the <ncurses.h> header file.  */
/* #undef HAVE_NCURSES_H */

/* Define if you have the <ndbm.h> header file.  */
/* #undef HAVE_NDBM_H */

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <poll.h> header file.  */
/* #undef HAVE_POLL_H */

/* Define if you have the <pthread.h> header file.  */
/* #undef HAVE_PTHREAD_H */

/* Define if you have the <pty.h> header file.  */
/* #undef HAVE_PTY_H */

/* Define if you have the <signal.h> header file.  */
#define HAVE_SIGNAL_H 1

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <sys/audioio.h> header file.  */
/* #undef HAVE_SYS_AUDIOIO_H */

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/lock.h> header file.  */
/* #undef HAVE_SYS_LOCK_H */

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/select.h> header file.  */
/* #undef HAVE_SYS_SELECT_H */

/* Define if you have the <sys/socket.h> header file.  */
#define HAVE_SYS_SOCKET_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/times.h> header file.  */
#define HAVE_SYS_TIMES_H 1

/* Define if you have the <sys/un.h> header file.  */
#define HAVE_SYS_UN_H 1

/* Define if you have the <sys/utsname.h> header file.  */
#define HAVE_SYS_UTSNAME_H 1

/* Define if you have the <sys/wait.h> header file.  */
#define HAVE_SYS_WAIT_H 1

/* Define if you have the <thread.h> header file.  */
/* #undef HAVE_THREAD_H */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <utime.h> header file.  */
#define HAVE_UTIME_H 1

/* Define if you have the dl library (-ldl).  */
/* #undef HAVE_LIBDL */

/* Define if you have the dld library (-ldld).  */
/* #undef HAVE_LIBDLD */

/* Define if you have the ieee library (-lieee).  */
/* #undef HAVE_LIBIEEE */

/* Changes made in porting to the Motorola Coldfire */
#define WITHOUT_COMPLEX 1
/* #undef  HAVE_DYNAMIC_LOADING */
#define WITH_COMPILE 0
#define WITH_GETPATH 0
