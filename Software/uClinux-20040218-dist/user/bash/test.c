/* GNU test program (ksb and mjb) */

/* Modified to run with the GNU shell Apr 25, 1988 by bfox. */

/* Copyright (C) 1987, 1988, 1989, 1990, 1991 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

/* Define PATTERN_MATCHING to get the csh-like =~ and !~ pattern-matching
   binary operators. */
/* #define PATTERN_MATCHING */

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <stdio.h>

#include "bashtypes.h"

#if defined (HAVE_LIMITS_H)
#  include <limits.h>
#else
#  include <sys/param.h>
#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <errno.h>
#if !defined (errno)
extern int errno;
#endif /* !errno */

#if !defined (_POSIX_VERSION)
#  include <sys/file.h>
#endif /* !_POSIX_VERSION */
#include "posixstat.h"
#include "filecntl.h"

#include "shell.h"
#include "pathexp.h"
#include "test.h"
#include "builtins/common.h"

#include <glob/fnmatch.h>

#if !defined (STRLEN)
#  define STRLEN(s) ((s)[0] ? ((s)[1] ? ((s)[2] ? strlen(s) : 2) : 1) : 0)
#endif

#if !defined (STREQ)
#  define STREQ(a, b) ((a)[0] == (b)[0] && strcmp (a, b) == 0)
#endif /* !STREQ */

#if !defined (R_OK)
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#endif /* R_OK */

#define EQ	0
#define NE	1
#define LT	2
#define GT	3
#define LE	4
#define GE	5

#define NT	0
#define OT	1
#define EF	2

/* The following few defines control the truth and false output of each stage.
   TRUE and FALSE are what we use to compute the final output value.
   SHELL_BOOLEAN is the form which returns truth or falseness in shell terms.
   Default is TRUE = 1, FALSE = 0, SHELL_BOOLEAN = (!value). */
#define TRUE 1
#define FALSE 0
#define SHELL_BOOLEAN(value) (!(value))

#define TEST_ERREXIT_STATUS	2

static procenv_t test_exit_buf;
static int test_error_return;
#define test_exit(val) \
	do { test_error_return = val; longjmp (test_exit_buf, 1); } while (0)

/* We have to use access(2) for machines running AFS, because it's
   not a Unix file system.  This may produce incorrect answers for
   non-AFS files.  I hate AFS. */
#if defined (AFS)
#  define EACCESS(path, mode)	access(path, mode)
#else
#  define EACCESS(path, mode)	test_eaccess(path, mode)
#endif /* AFS */

static int pos;		/* The offset of the current argument in ARGV. */
static int argc;	/* The number of arguments present in ARGV. */
static char **argv;	/* The argument list. */
static int noeval;

static int unary_operator ();
static int binary_operator ();
static int two_arguments ();
static int three_arguments ();
static int posixtest ();

static int expr ();
static int term ();
static int and ();
static int or ();

static void beyond ();

static void
test_syntax_error (format, arg)
     char *format, *arg;
{
  extern int interactive_shell;
  extern char *get_name_for_error ();
  if (interactive_shell == 0)
    fprintf (stderr, "%s: ", get_name_for_error ());
  fprintf (stderr, "%s: ", argv[0]);
  fprintf (stderr, format, arg);
  fprintf (stderr, "\n");
  fflush (stderr);
  test_exit (TEST_ERREXIT_STATUS);
}

/*
 * beyond - call when we're beyond the end of the argument list (an
 *	error condition)
 */
static void
beyond ()
{
  test_syntax_error ("argument expected", (char *)NULL);
}

/* Syntax error for when an integer argument was expected, but
   something else was found. */
static void
integer_expected_error (pch)
     char *pch;
{
  test_syntax_error ("%s: integer expression expected", pch);
}

/* A wrapper for stat () which disallows pathnames that are empty strings
   and handles /dev/fd emulation on systems that don't have it. */
static int
test_stat (path, finfo)
     char *path;
     struct stat *finfo;
{
  if (*path == '\0')
    {
      errno = ENOENT;
      return (-1);
    }
  if (path[0] == '/' && path[1] == 'd' && strncmp (path, "/dev/fd/", 8) == 0)
    {
#if !defined (HAVE_DEV_FD)
      long fd;
      if (legal_number (path + 8, &fd))
	return (fstat ((int)fd, finfo));
      else
	{
	  errno = EBADF;
	  return (-1);
	}
#else
  /* If HAVE_DEV_FD is defined, DEV_FD_PREFIX is defined also, and has a
     trailing slash.  Make sure /dev/fd/xx really uses DEV_FD_PREFIX/xx.
     On most systems, with the notable exception of linux, this is
     effectively a no-op. */
      char pbuf[32];
      strcpy (pbuf, DEV_FD_PREFIX);
      strcat (pbuf, path + 8);
      return (stat (pbuf, finfo));
#endif /* !HAVE_DEV_FD */
    }
#if !defined (HAVE_DEV_STDIN)
  else if (STREQN (path, "/dev/std", 8))
    {
      if (STREQ (path+8, "in"))
	return (fstat (0, finfo));
      else if (STREQ (path+8, "out"))
	return (fstat (1, finfo));
      else if (STREQ (path+8, "err"))
	return (fstat (2, finfo));
      else
	return (stat (path, finfo));
    }
#endif /* !HAVE_DEV_STDIN */
  return (stat (path, finfo));
}

/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
int
test_eaccess (path, mode)
     char *path;
     int mode;
{
  struct stat st;

  if (test_stat (path, &st) < 0)
    return (-1);

  if (current_user.euid == 0)
    {
      /* Root can read or write any file. */
      if (mode != X_OK)
	return (0);

      /* Root can execute any file that has any one of the execute
	 bits set. */
      if (st.st_mode & S_IXUGO)
	return (0);
    }

  if (st.st_uid == current_user.euid)	/* owner */
    mode <<= 6;
  else if (group_member (st.st_gid))
    mode <<= 3;

  if (st.st_mode & mode)
    return (0);

  errno = EACCES;
  return (-1);
}

/* Increment our position in the argument list.  Check that we're not
   past the end of the argument list.  This check is supressed if the
   argument is FALSE.  Made a macro for efficiency. */
#define advance(f) do { ++pos; if (f && pos >= argc) beyond (); } while (0)
#define unary_advance() do { advance (1); ++pos; } while (0)

/*
 * expr:
 *	or
 */
static int
expr ()
{
  if (pos >= argc)
    beyond ();

  return (FALSE ^ or ());		/* Same with this. */
}

/*
 * or:
 *	and
 *	and '-o' or
 */
static int
or ()
{
  int value, v2;

  value = and ();
  while (pos < argc && argv[pos][0] == '-' && argv[pos][1] == 'o' && !argv[pos][2])
    {
      advance (0);
      v2 = or ();
      return (value || v2);
    }

  return (value);
}

/*
 * and:
 *	term
 *	term '-a' and
 */
static int
and ()
{
  int value, v2;

  value = term ();
  while (pos < argc && argv[pos][0] == '-' && argv[pos][1] == 'a' && !argv[pos][2])
    {
      advance (0);
      v2 = and ();
      return (value && v2);
    }
  return (value);
}

/*
 * term - parse a term and return 1 or 0 depending on whether the term
 *	evaluates to true or false, respectively.
 *
 * term ::=
 *	'-'('a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'k'|'p'|'r'|'s'|'u'|'w'|'x') filename
 *	'-'('G'|'L'|'O'|'S'|'N') filename
 * 	'-t' [int]
 *	'-'('z'|'n') string
 *	'-o' option
 *	string
 *	string ('!='|'='|'==') string
 *	<int> '-'(eq|ne|le|lt|ge|gt) <int>
 *	file '-'(nt|ot|ef) file
 *	'(' <expr> ')'
 * int ::=
 *	positive and negative integers
 */
static int
term ()
{
  int value;

  if (pos >= argc)
    beyond ();

  /* Deal with leading `not's. */
  if (argv[pos][0] == '!' && argv[pos][1] == '\0')
    {
      value = 0;
      while (pos < argc && argv[pos][0] == '!' && argv[pos][1] == '\0')
	{
	  advance (1);
	  value = 1 - value;
	}

      return (value ? !term() : term());
    }

  /* A paren-bracketed argument. */
  if (argv[pos][0] == '(' && argv[pos][1] == '\0')
    {
      advance (1);
      value = expr ();
      if (argv[pos] == 0)
	test_syntax_error ("`)' expected", (char *)NULL);
      else if (argv[pos][0] != ')' || argv[pos][1])
	test_syntax_error ("`)' expected, found %s", argv[pos]);
      advance (0);
      return (value);
    }

  /* are there enough arguments left that this could be dyadic? */
  if ((pos + 3 <= argc) && test_binop (argv[pos + 1]))
    value = binary_operator ();

  /* Might be a switch type argument */
  else if (argv[pos][0] == '-' && argv[pos][2] == '\0')
    {
      if (test_unop (argv[pos]))
	value = unary_operator ();
      else
	test_syntax_error ("%s: unary operator expected", argv[pos]);
    }
  else
    {
      value = argv[pos][0] != '\0';
      advance (0);
    }

  return (value);
}

static int
filecomp (s, t, op)
     char *s, *t;
     int op;
{
  struct stat st1, st2;

  if (test_stat (s, &st1) < 0)
    {
      st1.st_mtime = 0;
      if (op == EF)
	return (FALSE);
    }
  if (test_stat (t, &st2) < 0)
    {
      st2.st_mtime = 0;
      if (op == EF)
	return (FALSE);
    }
  
  switch (op)
    {
    case OT: return (st1.st_mtime < st2.st_mtime);
    case NT: return (st1.st_mtime > st2.st_mtime);
    case EF: return ((st1.st_dev == st2.st_dev) && (st1.st_ino == st2.st_ino));
    }
  return (FALSE);
}

static int
arithcomp (s, t, op, flags)
     char *s, *t;
     int op, flags;
{
  long l, r;
  int expok;

  if (flags & TEST_ARITHEXP)
    {
      l = evalexp (s, &expok);
      if (expok == 0)
	return (FALSE);		/* should probably longjmp here */
      r = evalexp (t, &expok);
      if (expok == 0)
	return (FALSE);		/* ditto */
    }
  else
    {
      if (legal_number (s, &l) == 0)
	integer_expected_error (s);
      if (legal_number (t, &r) == 0)
	integer_expected_error (t);
    }

  switch (op)
    {
    case EQ: return (l == r);
    case NE: return (l != r);
    case LT: return (l < r);
    case GT: return (l > r);
    case LE: return (l <= r);
    case GE: return (l >= r);
    }

  return (FALSE);
}

static int
patcomp (string, pat, op)
     char *string, *pat;
     int op;
{
  int m;

  m = fnmatch (pat, string, FNMATCH_EXTFLAG);
  return ((op == EQ) ? (m == 0) : (m != 0));
}

int
binary_test (op, arg1, arg2, flags)
     char *op, *arg1, *arg2;
     int flags;
{
  int patmatch;

  patmatch = (flags & TEST_PATMATCH);

  if (op[0] == '=' && (op[1] == '\0' || (op[1] == '=' && op[2] == '\0')))
    return (patmatch ? patcomp (arg1, arg2, EQ) : STREQ (arg1, arg2));

  else if ((op[0] == '>' || op[0] == '<') && op[1] == '\0')
    return ((op[0] == '>') ? (strcmp (arg1, arg2) > 0) : (strcmp (arg1, arg2) < 0));

  else if (op[0] == '!' && op[1] == '=' && op[2] == '\0')
    return (patmatch ? patcomp (arg1, arg2, NE) : (STREQ (arg1, arg2) == 0));

  else if (op[2] == 't')
    {
      switch (op[1])
	{
	case 'n': return (filecomp (arg1, arg2, NT));		/* -nt */
	case 'o': return (filecomp (arg1, arg2, OT));		/* -ot */
	case 'l': return (arithcomp (arg1, arg2, LT, flags));	/* -lt */
	case 'g': return (arithcomp (arg1, arg2, GT, flags));	/* -gt */
	}
    }
  else if (op[1] == 'e')
    {
      switch (op[2])
	{
	case 'f': return (filecomp (arg1, arg2, EF));		/* -ef */
	case 'q': return (arithcomp (arg1, arg2, EQ, flags));	/* -eq */
	}
    }
  else if (op[2] == 'e')
    {
      switch (op[1])
	{
	case 'n': return (arithcomp (arg1, arg2, NE, flags));	/* -ne */
	case 'g': return (arithcomp (arg1, arg2, GE, flags));	/* -ge */
	case 'l': return (arithcomp (arg1, arg2, LE, flags));	/* -le */
	}
    }

  return (FALSE);	/* should never get here */
}


static int
binary_operator ()
{
  int value;
  char *w;

  w = argv[pos + 1];
  if ((w[0] == '=' && (w[1] == '\0' || (w[1] == '=' && w[2] == '\0'))) || /* =, == */
      ((w[0] == '>' || w[0] == '<') && w[1] == '\0') ||		/* <, > */
      (w[0] == '!' && w[1] == '=' && w[2] == '\0'))		/* != */
    {
      value = binary_test (w, argv[pos], argv[pos + 2], 0);
      pos += 3;
      return (value);
    }

#if defined (PATTERN_MATCHING)
  if ((w[0] == '=' || w[0] == '!') && w[1] == '~' && w[2] == '\0')
    {
      value = patcomp (argv[pos], argv[pos + 2], w[0] == '=' ? EQ : NE);
      pos += 3;
      return (value);
    }
#endif

  if ((w[0] != '-' || w[3] != '\0') || test_binop (w) == 0)
    {
      test_syntax_error ("%s: binary operator expected", w);
      /* NOTREACHED */
      return (FALSE);
    }

  value = binary_test (w, argv[pos], argv[pos + 2], 0);
  pos += 3;
  return value;
}

static int
unary_operator ()
{
  char *op, *arg;
  long r;

  op = argv[pos];
  if (test_unop (op) == 0)
    return (FALSE);

  /* the only tricky case is `-t', which may or may not take an argument. */
  if (op[1] == 't')
    {
      advance (0);
      if (pos < argc)
	{
	  if (legal_number (argv[pos], &r))
	    {
	      advance (0);
	      return (unary_test (op, argv[pos - 1]));
	    }
	  else
	    return (FALSE);
	}
      else
	return (unary_test (op, "1"));
    }

  /* All of the unary operators take an argument, so we first call
     unary_advance (), which checks to make sure that there is an
     argument, and then advances pos right past it.  This means that
     pos - 1 is the location of the argument. */
  unary_advance ();
  return (unary_test (op, argv[pos - 1]));
}

int
unary_test (op, arg)
     char *op, *arg;
{
  long r;
  struct stat stat_buf;
     
  switch (op[1])
    {
    case 'a':			/* file exists in the file system? */
    case 'e':
      return (test_stat (arg, &stat_buf) == 0);

    case 'r':			/* file is readable? */
      return (EACCESS (arg, R_OK) == 0);

    case 'w':			/* File is writeable? */
      return (EACCESS (arg, W_OK) == 0);

    case 'x':			/* File is executable? */
      return (EACCESS (arg, X_OK) == 0);

    case 'O':			/* File is owned by you? */
      return (test_stat (arg, &stat_buf) == 0 &&
	      (uid_t) current_user.euid == (uid_t) stat_buf.st_uid);

    case 'G':			/* File is owned by your group? */
      return (test_stat (arg, &stat_buf) == 0 &&
	      (gid_t) current_user.egid == (gid_t) stat_buf.st_gid);

    case 'N':
      return (test_stat (arg, &stat_buf) == 0 &&
	      stat_buf.st_atime <= stat_buf.st_mtime);

    case 'f':			/* File is a file? */
      if (test_stat (arg, &stat_buf) < 0)
	return (FALSE);

      /* -f is true if the given file exists and is a regular file. */
#if defined (S_IFMT)
      return (S_ISREG (stat_buf.st_mode) || (stat_buf.st_mode & S_IFMT) == 0);
#else
      return (S_ISREG (stat_buf.st_mode));
#endif /* !S_IFMT */

    case 'd':			/* File is a directory? */
      return (test_stat (arg, &stat_buf) == 0 && (S_ISDIR (stat_buf.st_mode)));

    case 's':			/* File has something in it? */
      return (test_stat (arg, &stat_buf) == 0 && stat_buf.st_size > (off_t) 0);

    case 'S':			/* File is a socket? */
#if !defined (S_ISSOCK)
      return (FALSE);
#else
      return (test_stat (arg, &stat_buf) == 0 && S_ISSOCK (stat_buf.st_mode));
#endif /* S_ISSOCK */

    case 'c':			/* File is character special? */
      return (test_stat (arg, &stat_buf) == 0 && S_ISCHR (stat_buf.st_mode));

    case 'b':			/* File is block special? */
      return (test_stat (arg, &stat_buf) == 0 && S_ISBLK (stat_buf.st_mode));

    case 'p':			/* File is a named pipe? */
#ifndef S_ISFIFO
      return (FALSE);
#else
      return (test_stat (arg, &stat_buf) == 0 && S_ISFIFO (stat_buf.st_mode));
#endif /* S_ISFIFO */

    case 'L':			/* Same as -h  */
    case 'h':			/* File is a symbolic link? */
#if !defined (S_ISLNK) || !defined (HAVE_LSTAT)
      return (FALSE);
#else
      return ((arg[0] != '\0') &&
	      (lstat (arg, &stat_buf) == 0) && S_ISLNK (stat_buf.st_mode));
#endif /* S_IFLNK && HAVE_LSTAT */

    case 'u':			/* File is setuid? */
      return (test_stat (arg, &stat_buf) == 0 && (stat_buf.st_mode & S_ISUID) != 0);

    case 'g':			/* File is setgid? */
      return (test_stat (arg, &stat_buf) == 0 && (stat_buf.st_mode & S_ISGID) != 0);

    case 'k':			/* File has sticky bit set? */
#if !defined (S_ISVTX)
      /* This is not Posix, and is not defined on some Posix systems. */
      return (FALSE);
#else
      return (test_stat (arg, &stat_buf) == 0 && (stat_buf.st_mode & S_ISVTX) != 0);
#endif

    case 't':	/* File fd is a terminal? */
      if (legal_number (arg, &r) == 0)
	return (FALSE);
      return (isatty ((int)r));

    case 'n':			/* True if arg has some length. */
      return (arg[0] != '\0');

    case 'z':			/* True if arg has no length. */
      return (arg[0] == '\0');

    case 'o':			/* True if option `arg' is set. */
      return (minus_o_option_value (arg) == 1);
    }
}

/* Return TRUE if OP is one of the test command's binary operators. */
int
test_binop (op)
     char *op;
{
  if (op[0] == '=' && op[1] == '\0')
    return (1);		/* '=' */
  else if ((op[0] == '<' || op[0] == '>') && op[1] == '\0')  /* string <, > */
    return (1);
  else if ((op[0] == '=' || op[0] == '!') && op[1] == '=' && op[2] == '\0')
    return (1);		/* `==' and `!=' */
#if defined (PATTERN_MATCHING)
  else if (op[2] == '\0' && op[1] == '~' && (op[0] == '=' || op[0] == '!'))
    return (1);
#endif
  else if (op[0] != '-' || op[2] == '\0' || op[3] != '\0')
    return (0);
  else
    {
      if (op[2] == 't')
	switch (op[1])
	  {
	  case 'n':		/* -nt */
	  case 'o':		/* -ot */
	  case 'l':		/* -lt */
	  case 'g':		/* -gt */
	    return (1);
	  default:
	    return (0);
	  }
      else if (op[1] == 'e')
	switch (op[2])
	  {
	  case 'q':		/* -eq */
	  case 'f':		/* -ef */
	    return (1);
	  default:
	    return (0);
	  }
      else if (op[2] == 'e')
	switch (op[1])
	  {
	  case 'n':		/* -ne */
	  case 'g':		/* -ge */
	  case 'l':		/* -le */
	    return (1);
	  default:
	    return (0);
	  }
      else
	return (0);
    }
}

/* Return non-zero if OP is one of the test command's unary operators. */
int
test_unop (op)
     char *op;
{
  if (op[0] != '-')
    return (0);

  switch (op[1])
    {
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'k': case 'n':
    case 'o': case 'p': case 'r': case 's': case 't':
    case 'u': case 'w': case 'x': case 'z':
    case 'G': case 'L': case 'O': case 'S': case 'N':
      return (1);
    }

  return (0);
}

static int
two_arguments ()
{
  if (argv[pos][0] == '!' && argv[pos][1] == '\0')
    return (argv[pos + 1][0] == '\0');
  else if (argv[pos][0] == '-' && argv[pos][2] == '\0')
    {
      if (test_unop (argv[pos]))
	return (unary_operator ());
      else
	test_syntax_error ("%s: unary operator expected", argv[pos]);
    }
  else
    test_syntax_error ("%s: unary operator expected", argv[pos]);

  return (0);
}

#define ANDOR(s)  (s[0] == '-' && !s[2] && (s[1] == 'a' || s[1] == 'o'))

#define ONE_ARG_TEST(s)		((s)[0] != '\0')

static int
three_arguments ()
{
  int value;

  if (test_binop (argv[pos+1]))
    {
      value = binary_operator ();
      pos = argc;
    }
  else if (ANDOR (argv[pos+1]))
    {
      if (argv[pos+1][1] == 'a')
	value = ONE_ARG_TEST(argv[pos]) && ONE_ARG_TEST(argv[pos+2]);
      else
	value = ONE_ARG_TEST(argv[pos]) || ONE_ARG_TEST(argv[pos+2]);
      pos = argc;
    }
  else if (argv[pos][0] == '!' && argv[pos][1] == '\0')
    {
      advance (1);
      value = !two_arguments ();
    }
  else if (argv[pos][0] == '(' && argv[pos+2][0] == ')')
    {
      value = ONE_ARG_TEST(argv[pos+1]);
      pos = argc;
    }
  else
    test_syntax_error ("%s: binary operator expected", argv[pos+1]);

  return (value);
}

/* This is an implementation of a Posix.2 proposal by David Korn. */
static int
posixtest ()
{
  int value;

  switch (argc - 1)	/* one extra passed in */
    {
      case 0:
	value = FALSE;
	pos = argc;
	break;

      case 1:
	value = ONE_ARG_TEST(argv[1]);
	pos = argc;
	break;

      case 2:
	value = two_arguments ();
	pos = argc;
	break;

      case 3:
	value = three_arguments ();
	break;

      case 4:
	if (argv[pos][0] == '!' && argv[pos][1] == '\0')
	  {
	    advance (1);
	    value = !three_arguments ();
	    break;
	  }
	/* FALLTHROUGH */
      default:
	value = expr ();
    }

  return (value);
}

/*
 * [:
 *	'[' expr ']'
 * test:
 *	test expr
 */
int
test_command (margc, margv)
     int margc;
     char **margv;
{
  int value;

  int code;

  code = setjmp (test_exit_buf);

  if (code)
    return (test_error_return);

  argv = margv;

  if (margv[0] && margv[0][0] == '[' && margv[0][1] == '\0')
    {
      --margc;

      if (margv[margc] && (margv[margc][0] != ']' || margv[margc][1]))
	test_syntax_error ("missing `]'", (char *)NULL);

      if (margc < 2)
	test_exit (SHELL_BOOLEAN (FALSE));
    }

  argc = margc;
  pos = 1;

  if (pos >= argc)
    test_exit (SHELL_BOOLEAN (FALSE));

  noeval = 0;
  value = posixtest ();

  if (pos != argc)
    test_syntax_error ("too many arguments", (char *)NULL);

  test_exit (SHELL_BOOLEAN (value));
}
