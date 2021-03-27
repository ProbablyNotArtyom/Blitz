/* Generate graphic from memory profiling data.
   Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <argp.h>
#include <assert.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <gd.h>
#include <gdfontl.h>
#include <gdfonts.h>


/* Default size of the generated image.  */
#define XSIZE 800
#define YSIZE 600

#ifndef N_
# define N_(Arg) Arg
#endif


/* Definitions of arguments for argp functions.  */
static const struct argp_option options[] =
{
  { "output", 'o', "FILE", 0, N_("Name output file") },
  { "string", 's', "STRING", 0, N_("Title string used in output graphic") },
  { "time", 't', NULL, 0, N_("Generate output linear to time (default is linear to number of function calls)") },
  { "total", 'T', NULL, 0,
    N_("Also draw graph for total memory consumption") },
  { "x-size", 'x', "VALUE", 0, N_("make output graphic VALUE pixel wide") },
  { "y-size", 'y', "VALUE", 0, N_("make output graphic VALUE pixel high") },
  { NULL, 0, NULL, 0, NULL }
};

/* Short description of program.  */
static const char doc[] = N_("Generate graphic from memory profiling data");

/* Strings for arguments in help texts.  */
static const char args_doc[] = N_("DATAFILE [OUTFILE]");

/* Prototype for option handler.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/* Function to print some extra text in the help message.  */
static char *more_help (int key, const char *text, void *input);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt, args_doc, doc, NULL, more_help
};


struct entry
{
  size_t heap;
  size_t stack;
  uint32_t time_low;
  uint32_t time_high;
};


/* Size of the image.  */
static size_t xsize;
static size_t ysize;

/* Name of the output file.  */
static char *outname;

/* Title string for the graphic.  */
static const char *string;

/* Nonzero if graph should be generated linear in time.  */
static int time_based;

/* Nonzero if graph to display total use of memory should be drawn as well.  */
static int also_total = 0;


int
main (int argc, char *argv[])
{
  int remaining;
  const char *inname;
  gdImagePtr im_out;
  int grey, blue, red, green, yellow, black;
  int fd;
  struct stat st;
  size_t maxsize_heap;
  size_t maxsize_stack;
  size_t maxsize_total;
  uint64_t total;
  uint64_t cnt, cnt2;
  FILE *outfile;
  char buf[30];
  size_t last_heap;
  size_t last_stack;
  size_t last_total;
  struct entry headent[2];
  uint64_t start_time;
  uint64_t end_time;
  uint64_t total_time;

  outname = NULL;
  xsize = XSIZE;
  ysize = YSIZE;
  string = NULL;

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if (remaining >= argc || remaining + 2 < argc)
    {
      argp_help (&argp, stdout, ARGP_HELP_SEE | ARGP_HELP_EXIT_ERR,
		 program_invocation_short_name);
      exit (1);
    }

  inname = argv[remaining++];

  if (remaining < argc)
    outname = argv[remaining];
  else if (outname == NULL)
    {
      size_t len = strlen (inname);
      outname = alloca (len + 5);
      stpcpy (stpcpy (outname, inname), ".png");
    }

  /* Open for read/write since we try to repair the file in case the
     application hasn't terminated cleanly.  */
  fd = open (inname, O_RDWR);
  if (fd == -1)
    error (EXIT_FAILURE, errno, "cannot open input file");
  if (fstat (fd, &st) != 0)
    {
      close (fd);
      error (EXIT_FAILURE, errno, "cannot get size of input file");
    }
  /* Test whether the file contains only full records.  */
  if ((st.st_size % sizeof (struct entry)) != 0
      /* The file must at least contain the two administrative records.  */
      || st.st_size < 2 * sizeof (struct entry))
    {
      close (fd);
      error (EXIT_FAILURE, 0, "input file as incorrect size");
    }
  /* Compute number of data entries.  */
  total = st.st_size / sizeof (struct entry) - 2;

  /* Read the administrative information.  */
  read (fd, headent, sizeof (headent));
  maxsize_heap = headent[1].heap;
  maxsize_stack = headent[1].stack;
  maxsize_total = headent[0].stack;
  if (also_total)
    {
      /* We use one scale and since we also draw the total amount of
	 memory used we have to adapt the maximum.  */
      maxsize_heap = maxsize_total;
      maxsize_stack = maxsize_total;
    }

  if (maxsize_heap == 0 && maxsize_stack == 0)
    {
      /* The program aborted before memusage was able to write the
	 information about the maximum heap and stack use.  Repair
	 the file now.  */
      struct entry next;

      while (1)
	{
	  if (read (fd, &next, sizeof (next)) == 0)
	    break;
	  if (next.heap > headent[1].heap)
	    headent[1].heap = next.heap;
	  if (next.stack > headent[1].stack)
	    headent[1].stack = next.stack;
	}

      headent[1].time_low = next.time_low;
      headent[1].time_high = next.time_high;

      /* Write the computed values in the file.  */
      lseek (fd, sizeof (struct entry), SEEK_SET);
      write (fd, &headent[1], sizeof (struct entry));
    }

  start_time = ((uint64_t) headent[0].time_high) << 32 | headent[0].time_low;
  end_time = ((uint64_t) headent[1].time_high) << 32 | headent[1].time_low;
  total_time = end_time - start_time;

  if (xsize < 100)
    xsize = 100;
  if (ysize < 80)
    ysize = 80;

  /* Create output image with the specified size.  */
  im_out = gdImageCreate (xsize, ysize);

  /* First color allocated is background.  */
  grey = gdImageColorAllocate (im_out, 224, 224, 224);

  /* Set transparent color. */
  gdImageColorTransparent (im_out, grey);

  /* These are all the other colors we need (in the moment).  */
  red = gdImageColorAllocate (im_out, 255, 0, 0);
  green = gdImageColorAllocate (im_out, 0, 130, 0);
  blue = gdImageColorAllocate (im_out, 0, 0, 255);
  yellow = gdImageColorAllocate (im_out, 154, 205, 50);
  black = gdImageColorAllocate (im_out, 0, 0, 0);

  gdImageRectangle (im_out, 40, 20, xsize - 40, ysize - 20, blue);

  gdImageString (im_out, gdFontSmall, 38, ysize - 14, (unsigned char *) "0",
		 blue);
  gdImageString (im_out, gdFontSmall, maxsize_heap < 1024 ? 32 : 26,
		 ysize - 26,
		 (unsigned char *) (maxsize_heap < 1024 ? "0" : "0k"), red);
  gdImageString (im_out, gdFontSmall, xsize - 37, ysize - 26,
		 (unsigned char *) (maxsize_stack < 1024 ? "0" : "0k"), green);

  if (string != NULL)
    gdImageString (im_out, gdFontLarge, (xsize - strlen (string) * 8) / 2,
		   2, (char *) string, green);

  gdImageStringUp (im_out, gdFontSmall, 1, ysize / 2 - 10,
		   (unsigned char *) "allocated", red);
  gdImageStringUp (im_out, gdFontSmall, 11, ysize / 2 - 10,
		   (unsigned char *) "memory", red);

  gdImageStringUp (im_out, gdFontSmall, xsize - 39, ysize / 2 - 10,
		   (unsigned char *) "used", green);
  gdImageStringUp (im_out, gdFontSmall, xsize - 27, ysize / 2 - 10,
		   (unsigned char *) "stack", green);

  if (maxsize_heap < 1024)
    {
      snprintf (buf, sizeof (buf), "%Zu", maxsize_heap);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6, 14, buf, red);
    }
  else
    {
      snprintf (buf, sizeof (buf), "%Zuk", maxsize_heap / 1024);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6, 14, buf, red);
    }
  if (maxsize_stack < 1024)
    {
      snprintf (buf, sizeof (buf), "%Zu", maxsize_stack);
      gdImageString (im_out, gdFontSmall, xsize - 37, 14, buf, green);
    }
  else
    {
      snprintf (buf, sizeof (buf), "%Zuk", maxsize_stack / 1024);
      gdImageString (im_out, gdFontSmall, xsize - 37, 14, buf, green);
    }


  if (maxsize_heap < 1024)
    {
      cnt = ((ysize - 40) * (maxsize_heap / 4)) / maxsize_heap;
      gdImageDashedLine (im_out, 40, ysize - 20 - cnt, xsize - 40,
			 ysize - 20 - cnt, red);
      snprintf (buf, sizeof (buf), "%Zu", maxsize_heap / 4);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6,
		     ysize - 26 - cnt, buf, red);
    }
  else
    {
      cnt = ((ysize - 40) * (maxsize_heap / 4096)) / (maxsize_heap / 1024);
      gdImageDashedLine (im_out, 40, ysize - 20 - cnt, xsize - 40,
			 ysize - 20 - cnt, red);
      snprintf (buf, sizeof (buf), "%Zuk", maxsize_heap / 4096);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6,
		     ysize - 26 - cnt, buf, red);
    }
  if (maxsize_stack < 1024)
    {
      cnt2 = ((ysize - 40) * (maxsize_stack / 4)) / maxsize_stack;
      if (cnt != cnt2)
	gdImageDashedLine (im_out, 40, ysize - 20 - cnt2, xsize - 40,
			   ysize - 20 - cnt2, green);
      snprintf (buf, sizeof (buf), "%Zu", maxsize_stack / 4);
      gdImageString (im_out, gdFontSmall, xsize - 37, ysize - 26 - cnt2,
		     buf, green);
    }
  else
    {
      cnt2 = ((ysize - 40) * (maxsize_stack / 4096)) / (maxsize_stack / 1024);
      if (cnt != cnt2)
	gdImageDashedLine (im_out, 40, ysize - 20 - cnt2, xsize - 40,
			   ysize - 20 - cnt2, green);
      snprintf (buf, sizeof (buf), "%Zuk", maxsize_stack / 4096);
      gdImageString (im_out, gdFontSmall, xsize - 37, ysize - 26 - cnt2,
		     buf, green);
    }

  if (maxsize_heap < 1024)
    {
      cnt = ((ysize - 40) * (maxsize_heap / 2)) / maxsize_heap;
      gdImageDashedLine (im_out, 40, ysize - 20 - cnt, xsize - 40,
			 ysize - 20 - cnt, red);
      snprintf (buf, sizeof (buf), "%Zu", maxsize_heap / 2);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6,
		     ysize - 26 - cnt, buf, red);
    }
  else
    {
      cnt = ((ysize - 40) * (maxsize_heap / 2048)) / (maxsize_heap / 1024);
      gdImageDashedLine (im_out, 40, ysize - 20 - cnt, xsize - 40,
			 ysize - 20 - cnt, red);
      snprintf (buf, sizeof (buf), "%Zuk", maxsize_heap / 2048);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6,
		     ysize - 26 - cnt, buf, red);
    }
  if (maxsize_stack < 1024)
    {
      cnt2 = ((ysize - 40) * (maxsize_stack / 2)) / maxsize_stack;
      if (cnt != cnt2)
	gdImageDashedLine (im_out, 40, ysize - 20 - cnt2, xsize - 40,
			   ysize - 20 - cnt2, green);
      snprintf (buf, sizeof (buf), "%Zu", maxsize_stack / 2);
      gdImageString (im_out, gdFontSmall, xsize - 37, ysize - 26 - cnt2,
		     buf, green);
    }
  else
    {
      cnt2 = ((ysize - 40) * (maxsize_stack / 2048)) / (maxsize_stack / 1024);
      if (cnt != cnt2)
	gdImageDashedLine (im_out, 40, ysize - 20 - cnt2, xsize - 40,
			   ysize - 20 - cnt2, green);
      snprintf (buf, sizeof (buf), "%Zuk", maxsize_stack / 2048);
      gdImageString (im_out, gdFontSmall, xsize - 37, ysize - 26 - cnt2,
		     buf, green);
    }

  if (maxsize_heap < 1024)
    {
      cnt = ((ysize - 40) * ((3 * maxsize_heap) / 4)) / maxsize_heap;
      gdImageDashedLine (im_out, 40, ysize - 20 - cnt, xsize - 40,
			 ysize - 20 - cnt, red);
      snprintf (buf, sizeof (buf), "%Zu", (3 * maxsize_heap) / 4);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6,
		     ysize - 26 - cnt, buf, red);
    }
  else
    {
      cnt = ((ysize - 40) * ((3 * maxsize_heap) / 4096)) / (maxsize_heap
							    / 1024);
      gdImageDashedLine (im_out, 40, ysize - 20 - cnt, xsize - 40,
			 ysize - 20 - cnt, red);
      snprintf (buf, sizeof (buf), "%Zuk", (3 * maxsize_heap) / 4096);
      gdImageString (im_out, gdFontSmall, 39 - strlen (buf) * 6,
		     ysize - 26 - cnt, buf, red);
    }
  if (maxsize_stack < 1024)
    {
      cnt2 = ((ysize - 40) * ((3 * maxsize_stack) / 4)) / maxsize_stack;
      if (cnt != cnt2)
	gdImageDashedLine (im_out, 40, ysize - 20 - cnt2, xsize - 40,
			   ysize - 20 - cnt2, green);
      snprintf (buf, sizeof (buf), "%Zu", (3 * maxsize_stack) / 4);
      gdImageString (im_out, gdFontSmall, xsize - 37, ysize - 26 - cnt2,
		     buf, green);
    }
  else
    {
      cnt2 = (((ysize - 40) * ((3 * maxsize_stack) / 4096))
	      / (maxsize_stack / 1024));
      if (cnt != cnt2)
	gdImageDashedLine (im_out, 40, ysize - 20 - cnt2, xsize - 40,
			   ysize - 20 - cnt2, green);
      snprintf (buf, sizeof (buf), "%Zuk", (3 * maxsize_stack) / 4096);
      gdImageString (im_out, gdFontSmall, xsize - 37, ysize - 26 - cnt2,
		     buf, green);
    }


  snprintf (buf, sizeof (buf), "%llu", total);
  gdImageString (im_out, gdFontSmall, xsize - 50, ysize - 14, buf, blue);

  if (!time_based)
    {
      uint64_t previously = start_time;

      gdImageString (im_out, gdFontSmall, 40 + (xsize - 32 * 6 - 80) / 2,
		     ysize - 12,
		     (unsigned char *) "# memory handling function calls",
		     blue);


      last_stack = last_heap = last_total = ysize - 20;
      for (cnt = 1; cnt <= total; ++cnt)
	{
	  struct entry entry;
	  size_t new[2];
	  uint64_t now;

	  read (fd, &entry, sizeof (entry));

	  now = ((uint64_t) entry.time_high) << 32 | entry.time_low;

	  if ((((previously - start_time) * 100) / total_time) % 10 < 5)
	    gdImageFilledRectangle (im_out,
				    40 + ((cnt - 1) * (xsize - 80)) / total,
				    ysize - 19,
				    39 + (cnt * (xsize - 80)) / total,
				    ysize - 14, yellow);
	  previously = now;

	  if (also_total)
	    {
	      size_t new3;

	      new3 = (ysize - 20) - ((((unsigned long long int) (ysize - 40))
				      * (entry.heap + entry.stack))
				     / maxsize_heap);
	      gdImageLine (im_out, 40 + ((xsize - 80) * (cnt - 1)) / total,
			   last_total,
			   40 + ((xsize - 80) * cnt) / total, new3,
			   black);
	      last_total = new3;
	    }

	  // assert (entry.heap <= maxsize_heap);
	  new[0] = (ysize - 20) - ((((unsigned long long int) (ysize - 40))
				    * entry.heap) / maxsize_heap);
	  gdImageLine (im_out, 40 + ((xsize - 80) * (cnt - 1)) / total,
		       last_heap, 40 + ((xsize - 80) * cnt) / total, new[0],
		       red);
	  last_heap = new[0];

	  // assert (entry.stack <= maxsize_stack);
	  new[1] = (ysize - 20) - ((((unsigned long long int) (ysize - 40))
				    * entry.stack) / maxsize_stack);
	  gdImageLine (im_out, 40 + ((xsize - 80) * (cnt - 1)) / total,
		       last_stack, 40 + ((xsize - 80) * cnt) / total, new[1],
		       green);
	  last_stack = new[1];
	}

      cnt = 0;
      while (cnt < total)
	{
	  gdImageLine (im_out, 40 + ((xsize - 80) * cnt) / total, ysize - 20,
		       40 + ((xsize - 80) * cnt) / total, ysize - 15, blue);
	  cnt += MAX (1, total / 20);
	}
      gdImageLine (im_out, xsize - 40, ysize - 20, xsize - 40, ysize - 15,
		   blue);
    }
  else
    {
      uint64_t next_tick = MAX (1, total / 20);
      size_t last_xpos = 40;

      gdImageString (im_out, gdFontSmall, 40 + (xsize - 39 * 6 - 80) / 2,
		     ysize - 12,
		     (unsigned char *) "\
# memory handling function calls / time", blue);

      for (cnt = 0; cnt < 20; cnt += 2)
	gdImageFilledRectangle (im_out,
				40 + (cnt * (xsize - 80)) / 20, ysize - 19,
				39 + ((cnt + 1) * (xsize - 80)) / 20,
				ysize - 14, yellow);

      last_stack = last_heap = last_total = ysize - 20;
      for (cnt = 1; cnt <= total; ++cnt)
	{
	  struct entry entry;
	  size_t new[2];
	  size_t xpos;
	  uint64_t now;

	  read (fd, &entry, sizeof (entry));

	  now = ((uint64_t) entry.time_high) << 32 | entry.time_low;
	  xpos = 40 + ((xsize - 80) * (now - start_time)) / total_time;

	  if (cnt == next_tick)
	    {
	      gdImageLine (im_out, xpos, ysize - 20, xpos, ysize - 15, blue);
	      next_tick += MAX (1, total / 20);
	    }

	  if (also_total)
	    {
	      size_t new3;

	      new3 = (ysize - 20) - ((((unsigned long long int) (ysize - 40))
				      * (entry.heap + entry.stack))
				     / maxsize_heap);
	      gdImageLine (im_out, last_xpos, last_total, xpos, new3, black);
	      last_total = new3;
	    }

	  new[0] = (ysize - 20) - ((((unsigned long long int) (ysize - 40))
				    * entry.heap) / maxsize_heap);
	  gdImageLine (im_out, last_xpos, last_heap, xpos, new[0], red);
	  last_heap = new[0];

	  // assert (entry.stack <= maxsize_stack);
	  new[1] = (ysize - 20) - ((((unsigned long long int) (ysize - 40))
				    * entry.stack) / maxsize_stack);
	  gdImageLine (im_out, last_xpos, last_stack, xpos, new[1], green);
	  last_stack = new[1];

	  last_xpos = xpos;
	}
    }

  /* Write out the result.  */
  outfile = fopen (outname, "w");
  if (outfile == NULL)
    error (EXIT_FAILURE, errno, "cannot open output file");

  gdImagePng (im_out, outfile);

  fclose (outfile);

  gdImageDestroy (im_out);

  return 0;
}


/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'o':
      outname = arg;
      break;
    case 's':
      string = arg;
      break;
    case 't':
      time_based = 1;
      break;
    case 'T':
      also_total = 1;
      break;
    case 'x':
      xsize = atoi (arg);
      if (xsize == 0)
	xsize = XSIZE;
      break;
    case 'y':
      ysize = atoi (arg);
      if (ysize == 0)
	ysize = XSIZE;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}


static char *
more_help (int key, const char *text, void *input)
{
  char *orig;
  char *cp;

  switch (key)
    {
    case ARGP_KEY_HELP_EXTRA:
      /* We print some extra information.  */
      orig = gettext ("\
Report bugs using the `glibcbug' script to <bugs@gnu.org>.\n");
      cp = strdup (orig);
      if (cp == NULL)
	cp = orig;
      return cp;
    default:
      break;
    }
  return (char *) text;
}
