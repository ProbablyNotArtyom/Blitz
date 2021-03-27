/* mailcheck.c -- The check is in the mail... */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

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

#include "config.h"

#include <stdio.h>
#include "bashtypes.h"
#include "posixstat.h"
#ifndef _MINIX
#  include <sys/param.h>
#endif
#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include "bashansi.h"

#include "shell.h"
#include "maxpath.h"
#include "execute_cmd.h"
#include "mailcheck.h"
#include <tilde/tilde.h>

#ifndef NOW
#define NOW ((time_t)time ((time_t *)0))
#endif

typedef struct {
  char *name;
  char *msg;
  time_t access_time;
  time_t mod_time;
  off_t file_size;
} FILEINFO;

/* The list of remembered mail files. */
static FILEINFO **mailfiles = (FILEINFO **)NULL;

/* Number of mail files that we have. */
static int mailfiles_count;

/* The last known time that mail was checked. */
static int last_time_mail_checked;

/* Non-zero means warn if a mail file has been read since last checked. */
int mail_warning;

/* Returns non-zero if it is time to check mail. */
int
time_to_check_mail ()
{
  char *temp;
  time_t now;
  long seconds;

  temp = get_string_value ("MAILCHECK");
  seconds = -1L;

  /* Skip leading whitespace in MAILCHECK. */
  if (temp)
    {
      while (whitespace (*temp))
	temp++;

      seconds = atoi (temp);
    }

  /* Negative number, or non-numbers (such as empty string) cause no
     checking to take place. */
  if (seconds < 0)
    return (0);

  now = NOW;
  /* Time to check if MAILCHECK is explicitly set to zero, or if enough
     time has passed since the last check. */
  return (seconds == 0 || ((now - last_time_mail_checked) >= seconds));
}

/* Okay, we have checked the mail.  Perhaps I should make this function
   go away. */
void
reset_mail_timer ()
{
  last_time_mail_checked = NOW;
}

/* Locate a file in the list.  Return index of
   entry, or -1 if not found. */
static int
find_mail_file (file)
     char *file;
{
  register int i;

  for (i = 0; i < mailfiles_count; i++)
    if (STREQ (mailfiles[i]->name, file))
      return i;

  return -1;
}

#define RESET_MAIL_FILE(i) \
  do \
    { \
      mailfiles[i]->access_time = mailfiles[i]->mod_time = 0; \
      mailfiles[i]->file_size = 0; \
    } \
  while (0)

static void
update_mail_file (i)
     int i;
{
  char *file;
  struct stat finfo;

  file = mailfiles[i]->name;
  if (stat (file, &finfo) == 0)
    {
      mailfiles[i]->access_time = finfo.st_atime;
      mailfiles[i]->mod_time = finfo.st_mtime;
      mailfiles[i]->file_size = finfo.st_size;
    }
  else
    RESET_MAIL_FILE (i);
}

/* Add this file to the list of remembered files and return its index
   in the list of mail files. */
static int
add_mail_file (file, msg)
     char *file, *msg;
{
  struct stat finfo;
  char *filename;
  int i;

  filename = full_pathname (file);
  i = find_mail_file (filename);
  if (i >= 0)
    {
      if (stat (filename, &finfo) == 0)
	{
	  mailfiles[i]->mod_time = finfo.st_mtime;
	  mailfiles[i]->access_time = finfo.st_atime;
	  mailfiles[i]->file_size = finfo.st_size;
	}
      free (filename);
      return i;
    }

  i = mailfiles_count++;
  mailfiles = (FILEINFO **)xrealloc
		(mailfiles, mailfiles_count * sizeof (FILEINFO *));

  mailfiles[i] = (FILEINFO *)xmalloc (sizeof (FILEINFO));
  mailfiles[i]->name = filename;
  mailfiles[i]->msg = msg ? savestring (msg) : (char *)NULL;
  update_mail_file (i);
  return i;
}

/* Reset the existing mail files access and modification times to zero. */
void
reset_mail_files ()
{
  register int i;

  for (i = 0; i < mailfiles_count; i++)
    {
      RESET_MAIL_FILE (i);
    }
}

/* Free the information that we have about the remembered mail files. */
void
free_mail_files ()
{
  register int i;

  for (i = 0; i < mailfiles_count; i++)
    {
      free (mailfiles[i]->name);
      FREE (mailfiles[i]->msg);
      free (mailfiles[i]);
    }

  if (mailfiles)
    free (mailfiles);

  mailfiles_count = 0;
  mailfiles = (FILEINFO **)NULL;
}

/* Return non-zero if FILE's mod date has changed and it has not been
   accessed since modified. */
static int
file_mod_date_changed (i)
     int i;
{
  time_t mtime;
  struct stat finfo;
  char *file;

  file = mailfiles[i]->name;
  mtime = mailfiles[i]->mod_time;

  if ((stat (file, &finfo) == 0) && (finfo.st_size > 0))
    return (mtime != finfo.st_mtime);

  return (0);
}

/* Return non-zero if FILE's access date has changed. */
static int
file_access_date_changed (i)
     int i;
{
  time_t atime;
  struct stat finfo;
  char *file;

  file = mailfiles[i]->name;
  atime = mailfiles[i]->access_time;

  if ((stat (file, &finfo) == 0) && (finfo.st_size > 0))
    return (atime != finfo.st_atime);

  return (0);
}

/* Return non-zero if FILE's size has increased. */
static int
file_has_grown (i)
     int i;
{
  off_t size;
  struct stat finfo;
  char *file;

  file = mailfiles[i]->name;
  size = mailfiles[i]->file_size;

  return ((stat (file, &finfo) == 0) && (finfo.st_size > size));
}

/* Take an element from $MAILPATH and return the portion from
   the first unquoted `?' or `%' to the end of the string.  This is the
   message to be printed when the file contents change. */
static char *
parse_mailpath_spec (str)
     char *str;
{
  char *s;
  int pass_next;

  for (s = str, pass_next = 0; s && *s; s++)
    {
      if (pass_next)
	{
	  pass_next = 0;
	  continue;
	}
      if (*s == '\\')
	{
	  pass_next++;
	  continue;
	}
      if (*s == '?' || *s == '%')
	return s;
    }
  return ((char *)NULL);
}

char *
make_default_mailpath ()
{
  char *mp;

  get_current_user_info ();
  mp = xmalloc (2 + sizeof (DEFAULT_MAIL_DIRECTORY) + strlen (current_user.user_name));
  strcpy (mp, DEFAULT_MAIL_DIRECTORY);
  mp[sizeof(DEFAULT_MAIL_DIRECTORY) - 1] = '/';
  strcpy (mp + sizeof (DEFAULT_MAIL_DIRECTORY), current_user.user_name);
  return (mp);
}

/* Remember the dates of the files specified by MAILPATH, or if there is
   no MAILPATH, by the file specified in MAIL.  If neither exists, use a
   default value, which we randomly concoct from using Unix. */
void
remember_mail_dates ()
{
  char *mailpaths;
  char *mailfile, *mp;
  int i = 0;

  mailpaths = get_string_value ("MAILPATH");

  /* If no $MAILPATH, but $MAIL, use that as a single filename to check. */
  if (mailpaths == 0 && (mailpaths = get_string_value ("MAIL")))
    {
      add_mail_file (mailpaths, (char *)NULL);
      return;
    }

  if (mailpaths == 0)
    {
      mailpaths = make_default_mailpath ();
      add_mail_file (mailpaths, (char *)NULL);
      free (mailpaths);
      return;
    }

  while (mailfile = extract_colon_unit (mailpaths, &i))
    {
      mp = parse_mailpath_spec (mailfile);
      if (mp && *mp)
	*mp++ = '\0';
      add_mail_file (mailfile, mp);
      free (mailfile);
    }
}

/* check_mail () is useful for more than just checking mail.  Since it has
   the paranoids dream ability of telling you when someone has read your
   mail, it can just as easily be used to tell you when someones .profile
   file has been read, thus letting one know when someone else has logged
   in.  Pretty good, huh? */

/* Check for mail in some files.  If the modification date of any
   of the files in MAILPATH has changed since we last did a
   remember_mail_dates () then mention that the user has mail.
   Special hack:  If the variable MAIL_WARNING is non-zero and the
   mail file has been accessed since the last time we remembered, then
   the message "The mail in <mailfile> has been read" is printed. */
void
check_mail ()
{
  char *current_mail_file, *message;
  int i, use_user_notification;
  char *dollar_underscore, *temp;
  WORD_LIST *tlist;

  dollar_underscore = get_string_value ("_");
  if (dollar_underscore)
    dollar_underscore = savestring (dollar_underscore);

  for (i = 0; i < mailfiles_count; i++)
    {
      current_mail_file = mailfiles[i]->name;

      if (*current_mail_file == '\0')
	continue;

      if (file_mod_date_changed (i))
	{
	  int file_is_bigger;

	  use_user_notification = mailfiles[i]->msg != (char *)NULL;
	  message = mailfiles[i]->msg ? mailfiles[i]->msg : "You have mail in $_";

	  bind_variable ("_", current_mail_file);

#define atime mailfiles[i]->access_time
#define mtime mailfiles[i]->mod_time

	  /* Have to compute this before the call to update_mail_file, which
	     resets all the information. */
	  file_is_bigger = file_has_grown (i);

	  update_mail_file (i);

	  /* If the user has just run a program which manipulates the
	     mail file, then don't bother explaining that the mail
	     file has been manipulated.  Since some systems don't change
	     the access time to be equal to the modification time when
	     the mail in the file is manipulated, check the size also.  If
	     the file has not grown, continue. */
	  if ((atime >= mtime) && !file_is_bigger)
	    continue;

	  /* If the mod time is later than the access time and the file
	     has grown, note the fact that this is *new* mail. */
	  if (use_user_notification == 0 && (atime < mtime) && file_is_bigger)
	    message = "You have new mail in $_";
#undef atime
#undef mtime

	  if ((tlist = expand_string (message, Q_DOUBLE_QUOTES)))
	    {
	      temp = string_list (tlist);
	      puts (temp);
	      free (temp);
	      dispose_words (tlist);
	    }
	  else
	    putchar ('\n');
	}

      if (mail_warning && file_access_date_changed (i))
	{
	  update_mail_file (i);
	  printf ("The mail in %s has been read\n", current_mail_file);
	}
    }

  if (dollar_underscore)
    {
      bind_variable ("_", dollar_underscore);
      free (dollar_underscore);
    }
  else
    unbind_variable ("_");
}
