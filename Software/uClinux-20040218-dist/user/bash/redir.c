/* redir.c -- Functions to perform input and output redirection. */

/* Copyright (C) 1997 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */
#include "config.h"

#if !defined (__GNUC__) && !defined (HAVE_ALLOCA_H) && defined (_AIX)
  #pragma alloca
#endif /* _AIX && RISC6000 && !__GNUC__ */

#include <stdio.h>
#include "bashtypes.h"
#ifndef _MINIX
#  include <sys/file.h>
#endif
#include "filecntl.h"
#include "posixstat.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <errno.h>

#if !defined (errno)
extern int errno;
#endif

#include "bashansi.h"

#include "memalloc.h"
#include "shell.h"
#include "flags.h"
#include "execute_cmd.h"
#include "redir.h"

#if defined (BUFFERED_INPUT)
#  include "input.h"
#endif

extern int posixly_correct;
extern int interactive, interactive_shell;
extern REDIRECT *redirection_undo_list;
extern REDIRECT *exec_redirection_undo_list;

/* Static functions defined and used in this file. */
static void add_undo_close_redirect ();
static void add_exec_redirect ();
static int add_undo_redirect ();
static int do_redirection_internal ();
static int expandable_redirection_filename ();
static int stdin_redirection ();

/* Spare redirector used when translating [N]>&WORD or [N]<&WORD to a new
   redirection and when creating the redirection undo list. */
static REDIRECTEE rd;

/* Set to errno when a here document cannot be created for some reason.
   Used to print a reasonable error message. */
static int heredoc_errno;

void
redirection_error (temp, error)
     REDIRECT *temp;
     int error;
{
  char *filename;
  int oflags;

  if (expandable_redirection_filename (temp))
    {
      if (posixly_correct && interactive_shell == 0)
	{
	  oflags = temp->redirectee.filename->flags;
	  temp->redirectee.filename->flags |= W_NOGLOB;
	}
      filename = redirection_expand (temp->redirectee.filename);
      if (posixly_correct && interactive_shell == 0)
	temp->redirectee.filename->flags = oflags;
      if (filename == 0)
	filename = savestring (temp->redirectee.filename->word);
      if (filename == 0)
	{
	  filename = xmalloc (1);
	  filename[0] = '\0';
	}
    }
  else
    filename = itos (temp->redirectee.dest);

  switch (error)
    {
    case AMBIGUOUS_REDIRECT:
      internal_error ("%s: ambiguous redirect", filename);
      break;

    case NOCLOBBER_REDIRECT:
      internal_error ("%s: cannot overwrite existing file", filename);
      break;

#if defined (RESTRICTED_SHELL)
    case RESTRICTED_REDIRECT:
      internal_error ("%s: restricted: cannot redirect output", filename);
      break;
#endif /* RESTRICTED_SHELL */

    case HEREDOC_REDIRECT:
      internal_error ("cannot create temp file for here document: %s", strerror (heredoc_errno));
      break;

    default:
      internal_error ("%s: %s", filename, strerror (error));
      break;
    }

  FREE (filename);
}

/* Perform the redirections on LIST.  If FOR_REAL, then actually make
   input and output file descriptors, otherwise just do whatever is
   neccessary for side effecting.  INTERNAL says to remember how to
   undo the redirections later, if non-zero.  If SET_CLEXEC is non-zero,
   file descriptors opened in do_redirection () have their close-on-exec
   flag set. */
int
do_redirections (list, for_real, internal, set_clexec)
     REDIRECT *list;
     int for_real, internal, set_clexec;
{
  int error;
  REDIRECT *temp;

  if (internal)
    {
      if (redirection_undo_list)
	{
	  dispose_redirects (redirection_undo_list);
	  redirection_undo_list = (REDIRECT *)NULL;
	}
      if (exec_redirection_undo_list)
	dispose_exec_redirects ();
    }

  for (temp = list; temp; temp = temp->next)
    {
      error = do_redirection_internal (temp, for_real, internal, set_clexec);
      if (error)
	{
	  redirection_error (temp, error);
	  return (error);
	}
    }
  return (0);
}

/* Return non-zero if the redirection pointed to by REDIRECT has a
   redirectee.filename that can be expanded. */
static int
expandable_redirection_filename (redirect)
     REDIRECT *redirect;
{
  switch (redirect->instruction)
    {
    case r_output_direction:
    case r_appending_to:
    case r_input_direction:
    case r_inputa_direction:
    case r_err_and_out:
    case r_input_output:
    case r_output_force:
    case r_duplicating_input_word:
    case r_duplicating_output_word:
      return 1;

    default:
      return 0;
    }
}

/* Expand the word in WORD returning a string.  If WORD expands to
   multiple words (or no words), then return NULL. */
char *
redirection_expand (word)
     WORD_DESC *word;
{
  char *result;
  WORD_LIST *tlist1, *tlist2;
  WORD_DESC *w;

  w = copy_word (word);
  if (posixly_correct)
    w->flags |= W_NOSPLIT;

  tlist1 = make_word_list (w, (WORD_LIST *)NULL);
  tlist2 = expand_words_no_vars (tlist1);
  dispose_words (tlist1);

  if (!tlist2 || tlist2->next)
    {
      /* We expanded to no words, or to more than a single word.
	 Dispose of the word list and return NULL. */
      if (tlist2)
	dispose_words (tlist2);
      return ((char *)NULL);
    }
  result = string_list (tlist2);  /* XXX savestring (tlist2->word->word)? */
  dispose_words (tlist2);
  return (result);
}

/* Write the text of the here document pointed to by REDIRECTEE to the file
   descriptor FD, which is already open to a temp file.  Return 0 if the
   write is successful, otherwise return errno. */
static int
write_here_document (fd, redirectee)
     int fd;
     WORD_DESC *redirectee;
{
  char *document;
  int document_len, fd2;
  FILE *fp;
  register WORD_LIST *t, *tlist;

  /* Expand the text if the word that was specified had
     no quoting.  The text that we expand is treated
     exactly as if it were surrounded by double quotes. */

  if (redirectee->flags & W_QUOTED)
    {
      document = redirectee->word;
      document_len = strlen (document);
      /* Set errno to something reasonable if the write fails. */
      if (write (fd, document, document_len) < document_len)
	{
	  if (errno == 0)
	    errno = ENOSPC;
	  return (errno);
	}
      else
	return 0;
    }

  tlist = expand_string (redirectee->word, Q_HERE_DOCUMENT);
  if (tlist)
    {
      /* Try using buffered I/O (stdio) and writing a word
	 at a time, letting stdio do the work of buffering
	 for us rather than managing our own strings.  Most
	 stdios are not particularly fast, however -- this
	 may need to be reconsidered later. */
      if ((fd2 = dup (fd)) < 0 || (fp = fdopen (fd2, "w")) == NULL)
	{
	  if (fd2 >= 0)
	    close (fd2);
	  return (errno);
	}
      errno = 0;
      for (t = tlist; t; t = t->next)
	{
	  /* This is essentially the body of
	     string_list_internal expanded inline. */
	  document = t->word->word;
	  document_len = strlen (document);
	  if (t != tlist)
	    putc (' ', fp);	/* separator */
	  fwrite (document, document_len, 1, fp);
	  if (ferror (fp))
	    {
	      if (errno == 0)
		errno = ENOSPC;
	      fd2 = errno;
	      fclose(fp);
	      dispose_words (tlist);
	      return (fd2);
	    }
	}
      fclose (fp);
      dispose_words (tlist);
    }
  return 0;
}

/* Create a temporary file holding the text of the here document pointed to
   by REDIRECTEE, and return a file descriptor open for reading to the temp
   file.  Return -1 on any error, and make sure errno is set appropriately. */
static int
here_document_to_fd (redirectee)
     WORD_DESC *redirectee;
{
  char *filename;
  int r, fd, fd2;
  static int fnum = 0;

  fd = sh_mktmpfd ("sh-thd", MT_USERANDOM, &filename);

  /* If we failed for some reason other than the file existing, abort */
  if (fd < 0)
    {
      FREE (filename);
      return (fd);
    }

  errno = r = 0;		/* XXX */
  /* write_here_document returns 0 on success, errno on failure. */
  if (redirectee->word)
    r = write_here_document (fd, redirectee);

  if (r)
    {
      close (fd);
      unlink (filename);
      free (filename);
      errno = r;
      return (-1);
    }

  /* In an attempt to avoid races, we close the first fd only after opening
     the second. */
  /* Make the document really temporary.  Also make it the input. */
  fd2 = open (filename, O_RDONLY, 0600);

  if (fd2 < 0)
    {
      r = errno;
      unlink (filename);
      free (filename);
      close (fd);
      errno = r;
      return -1;
    }

  close (fd);
  if (unlink (filename) < 0)
    {
      r = errno;
#if defined (__CYGWIN__)
      /* Under CygWin 1.1.0, the unlink will fail if the file is
	 open. This hack will allow the previous action of silently
	 ignoring the error, but will still leave the file there. This
	 needs some kind of magic. */
      if (r == EACCES)
	return (fd2);
#endif /* __CYGWIN__ */
      close (fd2);
      free (filename);
      errno = r;
      return (-1);
    }

  free (filename);
  return (fd2);
}

#define RF_DEVFD	1
#define RF_DEVSTDERR	2
#define RF_DEVSTDIN	3
#define RF_DEVSTDOUT	4
#define RF_DEVTCP	5
#define RF_DEVUDP	6

/* A list of pattern/value pairs for filenames that the redirection
   code handles specially. */
static STRING_INT_ALIST _redir_special_filenames[] = {
#if !defined (HAVE_DEV_FD)
  { "/dev/fd/[0-9]*", RF_DEVFD },
#endif
#if !defined (HAVE_DEV_STDIN)
  { "/dev/stderr", RF_DEVSTDERR },
  { "/dev/stdin", RF_DEVSTDIN },
  { "/dev/stdout", RF_DEVSTDOUT },
#endif
#if defined (NETWORK_REDIRECTIONS)
  { "/dev/tcp/*/*", RF_DEVTCP },
  { "/dev/udp/*/*", RF_DEVUDP },
#endif
  { (char *)NULL, -1 }
};

static int
redir_special_open (spec, filename, flags, mode, ri)
     int spec;
     char *filename;
     int flags, mode;
     enum r_instruction ri;
{
  int fd;
  long lfd;

  fd = -1;
  switch (spec)
    {
#if !defined (HAVE_DEV_FD)
    case RF_DEVFD:
      if (legal_number, filename+8, &lfd)
	fd = fcntl ((int)lfd, F_DUPFD, 10);
      else
	fd = AMBIGUOUS_REDIRECT;
      break;
#endif

#if !defined (HAVE_DEV_STDIN)
    case RF_DEVSTDIN:
      fd = fcntl (0, F_DUPFD, 10);
      break;
    case RF_DEVSTDOUT:
      fd = fcntl (1, F_DUPFD, 10);
      break;
    case RF_DEVSTDERR:
      fd = fcntl (2, F_DUPFD, 10);
      break;
#endif

#if defined (NETWORK_REDIRECTIONS)
    case RF_DEVTCP:
    case RF_DEVUDP:
#if defined (HAVE_NETWORK)
      fd = netopen (filename);
#else
      internal_warning ("/dev/(tcp|udp)/host/port not supported without networking");
      fd = open (filename, flags, mode);
#endif
      break;
#endif /* NETWORK_REDIRECTIONS */
    }

  return fd;
}
      
/* Open FILENAME with FLAGS in noclobber mode, hopefully avoiding most
   race conditions and avoiding the problem where the file is replaced
   between the stat(2) and open(2). */
static int
noclobber_open (filename, flags, mode, ri)
     char *filename;
     int flags, mode;
     enum r_instruction ri;
{
  int r, fd;
  struct stat finfo, finfo2;

  /* If the file exists and is a regular file, return an error
     immediately. */
  r = stat (filename, &finfo);
  if (r == 0 && (S_ISREG (finfo.st_mode)))
    return (NOCLOBBER_REDIRECT);

  /* If the file was not present (r != 0), make sure we open it
     exclusively so that if it is created before we open it, our open
     will fail.  Make sure that we do not truncate an existing file.
     Note that we don't turn on O_EXCL unless the stat failed -- if
     the file was not a regular file, we leave O_EXCL off. */
  flags &= ~O_TRUNC;
  if (r != 0)
    {
      fd = open (filename, flags|O_EXCL, mode);
      return ((fd < 0 && errno == EEXIST) ? NOCLOBBER_REDIRECT : fd);
    }
  fd = open (filename, flags, mode);

  /* If the open failed, return the file descriptor right away. */
  if (fd < 0)
    return (errno == EEXIST ? NOCLOBBER_REDIRECT : fd);

  /* OK, the open succeeded, but the file may have been changed from a
     non-regular file to a regular file between the stat and the open.
     We are assuming that the O_EXCL open handles the case where FILENAME
     did not exist and is symlinked to an existing file between the stat
     and open. */

  /* If we can open it and fstat the file descriptor, and neither check
     revealed that it was a regular file, and the file has not been replaced,
     return the file descriptor. */
  if ((fstat (fd, &finfo2) == 0) && (S_ISREG (finfo2.st_mode) == 0) &&
      r == 0 && (S_ISREG (finfo.st_mode) == 0) &&
      same_file (filename, filename, &finfo, &finfo2))
    return fd;

  /* The file has been replaced.  badness. */
  close (fd);  
  errno = EEXIST;
  return (NOCLOBBER_REDIRECT);
}

static int
redir_open (filename, flags, mode, ri)
     char *filename;
     int flags, mode;
     enum r_instruction ri;
{
  int fd, r;

  r = find_string_in_alist (filename, _redir_special_filenames, 1);
  if (r >= 0)
    return (redir_special_open (r, filename, flags, mode, ri));

  /* If we are in noclobber mode, you are not allowed to overwrite
     existing files.  Check before opening. */
  if (noclobber && CLOBBERING_REDIRECT (ri))
    {
      fd = noclobber_open (filename, flags, mode, ri);
      if (fd == NOCLOBBER_REDIRECT)
	return (NOCLOBBER_REDIRECT);
    }
  else
    {
      fd = open (filename, flags, mode);
#if defined (AFS)
      if ((fd < 0) && (errno == EACCES))
	fd = open (filename, flags & ~O_CREAT, mode);
#endif /* AFS */
    }

  return fd;
}

/* Do the specific redirection requested.  Returns errno or one of the
   special redirection errors (*_REDIRECT) in case of error, 0 on success.
   If FOR_REAL is zero, then just do whatever is neccessary to produce the
   appropriate side effects.   REMEMBERING, if non-zero, says to remember
   how to undo each redirection.  If SET_CLEXEC is non-zero, then
   we set all file descriptors > 2 that we open to be close-on-exec.  */
static int
do_redirection_internal (redirect, for_real, remembering, set_clexec)
     REDIRECT *redirect;
     int for_real, remembering, set_clexec;
{
  WORD_DESC *redirectee;
  int redir_fd, fd, redirector, r, oflags;
  char *redirectee_word;
  enum r_instruction ri;
  REDIRECT *new_redirect;

  redirectee = redirect->redirectee.filename;
  redir_fd = redirect->redirectee.dest;
  redirector = redirect->redirector;
  ri = redirect->instruction;

  if (ri == r_duplicating_input_word || ri == r_duplicating_output_word)
    {
      /* We have [N]>&WORD or [N]<&WORD.  Expand WORD, then translate
	 the redirection into a new one and continue. */
      redirectee_word = redirection_expand (redirectee);

      if (redirectee_word == 0)
	return (AMBIGUOUS_REDIRECT);
      else if (redirectee_word[0] == '-' && redirectee_word[1] == '\0')
	{
	  rd.dest = 0L;
	  new_redirect = make_redirection (redirector, r_close_this, rd);
	}
      else if (all_digits (redirectee_word))
	{
	  if (ri == r_duplicating_input_word)
	    {
	      rd.dest = atol (redirectee_word);
	      new_redirect = make_redirection (redirector, r_duplicating_input, rd);
	    }
	  else
	    {
	      rd.dest = atol (redirectee_word);
	      new_redirect = make_redirection (redirector, r_duplicating_output, rd);
	    }
	}
      else if (ri == r_duplicating_output_word && redirector == 1)
	{
	  rd.filename = make_bare_word (redirectee_word);
	  new_redirect = make_redirection (1, r_err_and_out, rd);
	}
      else
	{
	  free (redirectee_word);
	  return (AMBIGUOUS_REDIRECT);
	}

      free (redirectee_word);

      /* Set up the variables needed by the rest of the function from the
	 new redirection. */
      if (new_redirect->instruction == r_err_and_out)
	{
	  char *alloca_hack;

	  /* Copy the word without allocating any memory that must be
	     explicitly freed. */
	  redirectee = (WORD_DESC *)alloca (sizeof (WORD_DESC));
	  xbcopy ((char *)new_redirect->redirectee.filename,
		 (char *)redirectee, sizeof (WORD_DESC));

	  alloca_hack = (char *)
	    alloca (1 + strlen (new_redirect->redirectee.filename->word));
	  redirectee->word = alloca_hack;
	  strcpy (redirectee->word, new_redirect->redirectee.filename->word);
	}
      else
	/* It's guaranteed to be an integer, and shouldn't be freed. */
	redirectee = new_redirect->redirectee.filename;

      redir_fd = new_redirect->redirectee.dest;
      redirector = new_redirect->redirector;
      ri = new_redirect->instruction;

      /* Overwrite the flags element of the old redirect with the new value. */
      redirect->flags = new_redirect->flags;
      dispose_redirects (new_redirect);
    }

  switch (ri)
    {
    case r_output_direction:
    case r_appending_to:
    case r_input_direction:
    case r_inputa_direction:
    case r_err_and_out:		/* command &>filename */
    case r_input_output:
    case r_output_force:
      if (posixly_correct && interactive_shell == 0)
	{
	  oflags = redirectee->flags;
	  redirectee->flags |= W_NOGLOB;
	}
      redirectee_word = redirection_expand (redirectee);
      if (posixly_correct && interactive_shell == 0)
	redirectee->flags = oflags;

      if (redirectee_word == 0)
	return (AMBIGUOUS_REDIRECT);

#if defined (RESTRICTED_SHELL)
      if (restricted && (WRITE_REDIRECT (ri)))
	{
	  free (redirectee_word);
	  return (RESTRICTED_REDIRECT);
	}
#endif /* RESTRICTED_SHELL */

      fd = redir_open (redirectee_word, redirect->flags, 0666, ri);
      free (redirectee_word);

      if (fd == NOCLOBBER_REDIRECT)
	return (fd);

      if (fd < 0)
	return (errno);

      if (for_real)
	{
	  if (remembering)
	    /* Only setup to undo it if the thing to undo is active. */
	    if ((fd != redirector) && (fcntl (redirector, F_GETFD, 0) != -1))
	      add_undo_redirect (redirector);
	    else
	      add_undo_close_redirect (redirector);

#if defined (BUFFERED_INPUT)
	  check_bash_input (redirector);
#endif

	  if ((fd != redirector) && (dup2 (fd, redirector) < 0))
	    return (errno);

#if defined (BUFFERED_INPUT)
	  /* Do not change the buffered stream for an implicit redirection
	     of /dev/null to fd 0 for asynchronous commands without job
	     control (r_inputa_direction). */
	  if (ri == r_input_direction || ri == r_input_output)
	    duplicate_buffered_stream (fd, redirector);
#endif /* BUFFERED_INPUT */

	  /*
	   * If we're remembering, then this is the result of a while, for
	   * or until loop with a loop redirection, or a function/builtin
	   * executing in the parent shell with a redirection.  In the
	   * function/builtin case, we want to set all file descriptors > 2
	   * to be close-on-exec to duplicate the effect of the old
	   * for i = 3 to NOFILE close(i) loop.  In the case of the loops,
	   * both sh and ksh leave the file descriptors open across execs.
	   * The Posix standard mentions only the exec builtin.
	   */
	  if (set_clexec && (redirector > 2))
	    SET_CLOSE_ON_EXEC (redirector);
	}

      if (fd != redirector)
	{
#if defined (BUFFERED_INPUT)
	  if (INPUT_REDIRECT (ri))
	    close_buffered_fd (fd);
	  else
#endif /* !BUFFERED_INPUT */
	    close (fd);		/* Don't close what we just opened! */
	}

      /* If we are hacking both stdout and stderr, do the stderr
	 redirection here. */
      if (ri == r_err_and_out)
	{
	  if (for_real)
	    {
	      if (remembering)
		add_undo_redirect (2);
	      if (dup2 (1, 2) < 0)
		return (errno);
	    }
	}
      break;

    case r_reading_until:
    case r_deblank_reading_until:
      /* REDIRECTEE is a pointer to a WORD_DESC containing the text of
	 the new input.  Place it in a temporary file. */
      if (redirectee)
	{
	  fd = here_document_to_fd (redirectee);

	  if (fd < 0)
	    {
	      heredoc_errno = errno;
	      return (HEREDOC_REDIRECT);
	    }

	  if (for_real)
	    {
	      if (remembering)
		/* Only setup to undo it if the thing to undo is active. */
		if ((fd != redirector) && (fcntl (redirector, F_GETFD, 0) != -1))
		  add_undo_redirect (redirector);
		else
		  add_undo_close_redirect (redirector);

#if defined (BUFFERED_INPUT)
	      check_bash_input (redirector);
#endif
	      if (fd != redirector && dup2 (fd, redirector) < 0)
		{
		  r = errno;
		  close (fd);
		  return (r);
		}

#if defined (BUFFERED_INPUT)
	      duplicate_buffered_stream (fd, redirector);
#endif

	      if (set_clexec && (redirector > 2))
		SET_CLOSE_ON_EXEC (redirector);
	    }

	  if (fd != redirector)
#if defined (BUFFERED_INPUT)
	    close_buffered_fd (fd);
#else
	    close (fd);
#endif
	}
      break;

    case r_duplicating_input:
    case r_duplicating_output:
      if (for_real && (redir_fd != redirector))
	{
	  if (remembering)
	    /* Only setup to undo it if the thing to undo is active. */
	    if (fcntl (redirector, F_GETFD, 0) != -1)
	      add_undo_redirect (redirector);
	    else
	      add_undo_close_redirect (redirector);

#if defined (BUFFERED_INPUT)
	  check_bash_input (redirector);
#endif
	  /* This is correct.  2>&1 means dup2 (1, 2); */
	  if (dup2 (redir_fd, redirector) < 0)
	    return (errno);

#if defined (BUFFERED_INPUT)
	  if (ri == r_duplicating_input)
	    duplicate_buffered_stream (redir_fd, redirector);
#endif /* BUFFERED_INPUT */

	  /* First duplicate the close-on-exec state of redirectee.  dup2
	     leaves the flag unset on the new descriptor, which means it
	     stays open.  Only set the close-on-exec bit for file descriptors
	     greater than 2 in any case, since 0-2 should always be open
	     unless closed by something like `exec 2<&-'. */
	  /* if ((already_set || set_unconditionally) && (ok_to_set))
		set_it () */
	  if (((fcntl (redir_fd, F_GETFD, 0) == 1) || set_clexec) &&
	       (redirector > 2))
	    SET_CLOSE_ON_EXEC (redirector);
	}
      break;

    case r_close_this:
      if (for_real)
	{
	  if (remembering && (fcntl (redirector, F_GETFD, 0) != -1))
	    add_undo_redirect (redirector);

#if defined (BUFFERED_INPUT)
	  check_bash_input (redirector);
	  close_buffered_fd (redirector);
#else /* !BUFFERED_INPUT */
	  close (redirector);
#endif /* !BUFFERED_INPUT */
	}
      break;

    case r_duplicating_input_word:
    case r_duplicating_output_word:
      break;
    }
  return (0);
}

#define SHELL_FD_BASE	10

/* Remember the file descriptor associated with the slot FD,
   on REDIRECTION_UNDO_LIST.  Note that the list will be reversed
   before it is executed.  Any redirections that need to be undone
   even if REDIRECTION_UNDO_LIST is discarded by the exec builtin
   are also saved on EXEC_REDIRECTION_UNDO_LIST. */
static int
add_undo_redirect (fd)
     int fd;
{
  int new_fd, clexec_flag;
  REDIRECT *new_redirect, *closer, *dummy_redirect;

  new_fd = fcntl (fd, F_DUPFD, SHELL_FD_BASE);

  if (new_fd < 0)
    {
      sys_error ("redirection error: cannot duplicate fd");
      return (-1);
    }

  clexec_flag = fcntl (fd, F_GETFD, 0);

  rd.dest = 0L;
  closer = make_redirection (new_fd, r_close_this, rd);
  dummy_redirect = copy_redirects (closer);

  rd.dest = (long)new_fd;
  if (fd == 0)
    new_redirect = make_redirection (fd, r_duplicating_input, rd);
  else
    new_redirect = make_redirection (fd, r_duplicating_output, rd);
  new_redirect->next = closer;

  closer->next = redirection_undo_list;
  redirection_undo_list = new_redirect;

  /* Save redirections that need to be undone even if the undo list
     is thrown away by the `exec' builtin. */
  add_exec_redirect (dummy_redirect);

  /* File descriptors used only for saving others should always be
     marked close-on-exec.  Unfortunately, we have to preserve the
     close-on-exec state of the file descriptor we are saving, since
     fcntl (F_DUPFD) sets the new file descriptor to remain open
     across execs.  If, however, the file descriptor whose state we
     are saving is <= 2, we can just set the close-on-exec flag,
     because file descriptors 0-2 should always be open-on-exec,
     and the restore above in do_redirection() will take care of it. */
  if (clexec_flag || fd < 3)
    SET_CLOSE_ON_EXEC (new_fd);

  return (0);
}

/* Set up to close FD when we are finished with the current command
   and its redirections. */
static void
add_undo_close_redirect (fd)
     int fd;
{
  REDIRECT *closer;

  rd.dest = 0L;
  closer = make_redirection (fd, r_close_this, rd);
  closer->next = redirection_undo_list;
  redirection_undo_list = closer;
}

static void
add_exec_redirect (dummy_redirect)
     REDIRECT *dummy_redirect;
{
  dummy_redirect->next = exec_redirection_undo_list;
  exec_redirection_undo_list = dummy_redirect;
}

/* Return 1 if the redirection specified by RI and REDIRECTOR alters the
   standard input. */
static int
stdin_redirection (ri, redirector)
     enum r_instruction ri;
     int redirector;
{
  switch (ri)
    {
    case r_input_direction:
    case r_inputa_direction:
    case r_input_output:
    case r_reading_until:
    case r_deblank_reading_until:
      return (1);
    case r_duplicating_input:
    case r_duplicating_input_word:
    case r_close_this:
      return (redirector == 0);
    case r_output_direction:
    case r_appending_to:
    case r_duplicating_output:
    case r_err_and_out:
    case r_output_force:
    case r_duplicating_output_word:
      return (0);
    }
  return (0);
}

/* Return non-zero if any of the redirections in REDIRS alter the standard
   input. */
int
stdin_redirects (redirs)
     REDIRECT *redirs;
{
  REDIRECT *rp;
  int n;

  for (n = 0, rp = redirs; rp; rp = rp->next)
    n += stdin_redirection (rp->instruction, rp->redirector);
  return n;
}
