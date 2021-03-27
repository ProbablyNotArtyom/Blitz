/* vi: set sw=8 ts=8: */
/*
 * tiny vi.c: A small 'vi' clone
 * Copyright (C) 2000, 2001 Sterling Huxley <sterling@europa.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

static const char vi_Version[] =
	"$Id: vi.c,v 1.19 2002/10/26 10:19:07 andersen Exp $";

/*
 * To compile for standalone use:
 *	gcc -Wall -Os -s -DSTANDALONE -o vi vi.c
 *	  or
 *	gcc -Wall -Os -s -DSTANDALONE -DBB_FEATURE_VI_CRASHME -o vi vi.c		# include testing features
 *	strip vi
 */

/*
 * Things To Do:
 *	EXINIT
 *	$HOME/.exrc  and  ./.exrc
 *	add magic to search	/foo.*bar
 *	add :help command
 *	:map macros
 *	how about mode lines:   vi: set sw=8 ts=8:
 *	if mark[] values were line numbers rather than pointers
 *	   it would be easier to change the mark when add/delete lines
 *	More intelligence in refresh()
 *	":r !cmd"  and  "!cmd"  to filter text through an external command
 *	A true "undo" facility
 *	An "ex" line oriented mode- maybe using "cmdedit"
 */

//----  Feature --------------  Bytes to immplement
#ifdef STANDALONE
#define vi_main			main
#define BB_FEATURE_VI_COLON	// 4288
#define BB_FEATURE_VI_YANKMARK	// 1408
#define BB_FEATURE_VI_SEARCH	// 1088
#define BB_FEATURE_VI_USE_SIGNALS	// 1056
#define BB_FEATURE_VI_DOT_CMD	//  576
#define BB_FEATURE_VI_READONLY	//  128
#define BB_FEATURE_VI_SETOPTS	//  576
#define BB_FEATURE_VI_SET	//  224
#define BB_FEATURE_VI_WIN_RESIZE	//  256  WIN_RESIZE
// To test editor using CRASHME:
//    vi -C filename
// To stop testing, wait until all to text[] is deleted, or
//    Ctrl-Z and kill -9 %1
// while in the editor Ctrl-T will toggle the crashme function on and off.
//#define BB_FEATURE_VI_CRASHME		// randomly pick commands to execute
#endif							/* STANDALONE */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <regex.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#ifndef STANDALONE
#include "busybox.h"
#endif							/* STANDALONE */

#ifndef TRUE
#define TRUE			((int)1)
#define FALSE			((int)0)
#endif							/* TRUE */
#define MAX_SCR_COLS		BUFSIZ

// Misc. non-Ascii keys that report an escape sequence
#define VI_K_UP			128	// cursor key Up
#define VI_K_DOWN		129	// cursor key Down
#define VI_K_RIGHT		130	// Cursor Key Right
#define VI_K_LEFT		131	// cursor key Left
#define VI_K_HOME		132	// Cursor Key Home
#define VI_K_END		133	// Cursor Key End
#define VI_K_INSERT		134	// Cursor Key Insert
#define VI_K_PAGEUP		135	// Cursor Key Page Up
#define VI_K_PAGEDOWN		136	// Cursor Key Page Down
#define VI_K_FUN1		137	// Function Key F1
#define VI_K_FUN2		138	// Function Key F2
#define VI_K_FUN3		139	// Function Key F3
#define VI_K_FUN4		140	// Function Key F4
#define VI_K_FUN5		141	// Function Key F5
#define VI_K_FUN6		142	// Function Key F6
#define VI_K_FUN7		143	// Function Key F7
#define VI_K_FUN8		144	// Function Key F8
#define VI_K_FUN9		145	// Function Key F9
#define VI_K_FUN10		146	// Function Key F10
#define VI_K_FUN11		147	// Function Key F11
#define VI_K_FUN12		148	// Function Key F12

static const int YANKONLY = FALSE;
static const int YANKDEL = TRUE;
static const int FORWARD = 1;	// code depends on "1"  for array index
static const int BACK = -1;	// code depends on "-1" for array index
static const int LIMITED = 0;	// how much of text[] in char_search
static const int FULL = 1;	// how much of text[] in char_search

static const int S_BEFORE_WS = 1;	// used in skip_thing() for moving "dot"
static const int S_TO_WS = 2;		// used in skip_thing() for moving "dot"
static const int S_OVER_WS = 3;		// used in skip_thing() for moving "dot"
static const int S_END_PUNCT = 4;	// used in skip_thing() for moving "dot"
static const int S_END_ALNUM = 5;	// used in skip_thing() for moving "dot"

typedef unsigned char Byte;


static int editing;		// >0 while we are editing a file
static int cmd_mode;		// 0=command  1=insert
static int file_modified;	// buffer contents changed
static int err_method;		// indicate error with beep or flash
static int fn_start;		// index of first cmd line file name
static int save_argc;		// how many file names on cmd line
static int cmdcnt;		// repetition count
static fd_set rfds;		// use select() for small sleeps
static struct timeval tv;	// use select() for small sleeps
static char erase_char;		// the users erase character
static int rows, columns;	// the terminal screen is this size
static int crow, ccol, offset;	// cursor is on Crow x Ccol with Horz Ofset
static char *SOs, *SOn;		// terminal standout start/normal ESC sequence
static char *bell;		// terminal bell sequence
static char *Ceol, *Ceos;	// Clear-end-of-line and Clear-end-of-screen ESC sequence
static char *CMrc;		// Cursor motion arbitrary destination ESC sequence
static char *CMup, *CMdown;	// Cursor motion up and down ESC sequence
static Byte *status_buffer;	// mesages to the user
static Byte last_input_char;	// last char read from user
static Byte last_forward_char;	// last char searched for with 'f'
static Byte *cfn;		// previous, current, and next file name
static Byte *text, *end, *textend;	// pointers to the user data in memory
static Byte *screen;		// pointer to the virtual screen buffer
static int screensize;		//            and its size
static Byte *screenbegin;	// index into text[], of top line on the screen
static Byte *dot;		// where all the action takes place
static int tabstop;
static struct termios term_orig, term_vi;	// remember what the cooked mode was

#ifdef BB_FEATURE_VI_OPTIMIZE_CURSOR
static int last_row;		// where the cursor was last moved to
#endif							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */
#ifdef BB_FEATURE_VI_USE_SIGNALS
static jmp_buf restart;		// catch_sig()
#endif							/* BB_FEATURE_VI_USE_SIGNALS */
#ifdef BB_FEATURE_VI_WIN_RESIZE
static struct winsize winsize;	// remember the window size
#endif							/* BB_FEATURE_VI_WIN_RESIZE */
#ifdef BB_FEATURE_VI_DOT_CMD
static int adding2q;		// are we currently adding user input to q
static Byte *last_modifying_cmd;	// last modifying cmd for "."
static Byte *ioq, *ioq_start;	// pointer to string for get_one_char to "read"
#endif							/* BB_FEATURE_VI_DOT_CMD */
#if	defined(BB_FEATURE_VI_DOT_CMD) || defined(BB_FEATURE_VI_YANKMARK)
static Byte *modifying_cmds;	// cmds that modify text[]
#endif							/* BB_FEATURE_VI_DOT_CMD || BB_FEATURE_VI_YANKMARK */
#ifdef BB_FEATURE_VI_READONLY
static int vi_readonly, readonly;
#endif							/* BB_FEATURE_VI_READONLY */
#ifdef BB_FEATURE_VI_SETOPTS
static int autoindent;
static int showmatch;
static int ignorecase;
#endif							/* BB_FEATURE_VI_SETOPTS */
#ifdef BB_FEATURE_VI_YANKMARK
static Byte *reg[28];		// named register a-z, "D", and "U" 0-25,26,27
static int YDreg, Ureg;		// default delete register and orig line for "U"
static Byte *mark[28];		// user marks points somewhere in text[]-  a-z and previous context ''
static Byte *context_start, *context_end;
#endif							/* BB_FEATURE_VI_YANKMARK */
#ifdef BB_FEATURE_VI_SEARCH
static Byte *last_search_pattern;	// last pattern from a '/' or '?' search
#endif							/* BB_FEATURE_VI_SEARCH */


static void edit_file(Byte *);	// edit one file
static void do_cmd(Byte);	// execute a command
static void sync_cursor(Byte *, int *, int *);	// synchronize the screen cursor to dot
static Byte *begin_line(Byte *);	// return pointer to cur line B-o-l
static Byte *end_line(Byte *);	// return pointer to cur line E-o-l
static Byte *dollar_line(Byte *);	// return pointer to just before NL
static Byte *prev_line(Byte *);	// return pointer to prev line B-o-l
static Byte *next_line(Byte *);	// return pointer to next line B-o-l
static Byte *end_screen(void);	// get pointer to last char on screen
static int count_lines(Byte *, Byte *);	// count line from start to stop
static Byte *find_line(int);	// find begining of line #li
static Byte *move_to_col(Byte *, int);	// move "p" to column l
static int isblnk(Byte);	// is the char a blank or tab
static void dot_left(void);	// move dot left- dont leave line
static void dot_right(void);	// move dot right- dont leave line
static void dot_begin(void);	// move dot to B-o-l
static void dot_end(void);	// move dot to E-o-l
static void dot_next(void);	// move dot to next line B-o-l
static void dot_prev(void);	// move dot to prev line B-o-l
static void dot_scroll(int, int);	// move the screen up or down
static void dot_skip_over_ws(void);	// move dot pat WS
static void dot_delete(void);	// delete the char at 'dot'
static Byte *bound_dot(Byte *);	// make sure  text[0] <= P < "end"
static Byte *new_screen(int, int);	// malloc virtual screen memory
static Byte *new_text(int);	// malloc memory for text[] buffer
static Byte *char_insert(Byte *, Byte);	// insert the char c at 'p'
static Byte *stupid_insert(Byte *, Byte);	// stupidly insert the char c at 'p'
static Byte find_range(Byte **, Byte **, Byte);	// return pointers for an object
static int st_test(Byte *, int, int, Byte *);	// helper for skip_thing()
static Byte *skip_thing(Byte *, int, int, int);	// skip some object
static Byte *find_pair(Byte *, Byte);	// find matching pair ()  []  {}
static Byte *text_hole_delete(Byte *, Byte *);	// at "p", delete a 'size' byte hole
static Byte *text_hole_make(Byte *, int);	// at "p", make a 'size' byte hole
static Byte *yank_delete(Byte *, Byte *, int, int);	// yank text[] into register then delete
static void show_help(void);	// display some help info
static void print_literal(Byte *, Byte *);	// copy s to buf, convert unprintable
static void rawmode(void);	// set "raw" mode on tty
static void cookmode(void);	// return to "cooked" mode on tty
static int mysleep(int);	// sleep for 'h' 1/100 seconds
static Byte readit(void);	// read (maybe cursor) key from stdin
static Byte get_one_char(void);	// read 1 char from stdin
static int file_size(Byte *);	// what is the byte size of "fn"
static int file_insert(Byte *, Byte *, int);
static int file_write(Byte *, Byte *, Byte *);
static void place_cursor(int, int, int);
static void screen_erase();
static void clear_to_eol(void);
static void clear_to_eos(void);
static void standout_start(void);	// send "start reverse video" sequence
static void standout_end(void);	// send "end reverse video" sequence
static void flash(int);		// flash the terminal screen
static void beep(void);		// beep the terminal
static void indicate_error(char);	// use flash or beep to indicate error
static void show_status_line(void);	// put a message on the bottom line
static void psb(char *, ...);	// Print Status Buf
static void psbs(char *, ...);	// Print Status Buf in standout mode
static void ni(Byte *);		// display messages
static void edit_status(void);	// show file status on status line
static void redraw(int);	// force a full screen refresh
static void format_line(Byte*, Byte*, int);
static void refresh(int);	// update the terminal from screen[]

#ifdef BB_FEATURE_VI_SEARCH
static Byte *char_search(Byte *, Byte *, int, int);	// search for pattern starting at p
static int mycmp(Byte *, Byte *, int);	// string cmp based in "ignorecase"
#endif							/* BB_FEATURE_VI_SEARCH */
#ifdef BB_FEATURE_VI_COLON
static void Hit_Return(void);
static Byte *get_one_address(Byte *, int *);	// get colon addr, if present
static Byte *get_address(Byte *, int *, int *);	// get two colon addrs, if present
static void colon(Byte *);	// execute the "colon" mode cmds
#endif							/* BB_FEATURE_VI_COLON */
static Byte *get_input_line(Byte *);	// get input line- use "status line"
#ifdef BB_FEATURE_VI_USE_SIGNALS
static void winch_sig(int);	// catch window size changes
static void suspend_sig(int);	// catch ctrl-Z
static void alarm_sig(int);	// catch alarm time-outs
static void catch_sig(int);	// catch ctrl-C
static void core_sig(int);	// catch a core dump signal
#endif							/* BB_FEATURE_VI_USE_SIGNALS */
#ifdef BB_FEATURE_VI_DOT_CMD
static void start_new_cmd_q(Byte);	// new queue for command
static void end_cmd_q();	// stop saving input chars
#else							/* BB_FEATURE_VI_DOT_CMD */
#define end_cmd_q()
#endif							/* BB_FEATURE_VI_DOT_CMD */
#ifdef BB_FEATURE_VI_WIN_RESIZE
static void window_size_get(int);	// find out what size the window is
#endif							/* BB_FEATURE_VI_WIN_RESIZE */
#ifdef BB_FEATURE_VI_SETOPTS
static void showmatching(Byte *);	// show the matching pair ()  []  {}
#endif							/* BB_FEATURE_VI_SETOPTS */
#if defined(BB_FEATURE_VI_YANKMARK) || defined(BB_FEATURE_VI_COLON) || defined(BB_FEATURE_VI_CRASHME)
static Byte *string_insert(Byte *, Byte *);	// insert the string at 'p'
#endif							/* BB_FEATURE_VI_YANKMARK || BB_FEATURE_VI_COLON || BB_FEATURE_VI_CRASHME */
#ifdef BB_FEATURE_VI_YANKMARK
static Byte *text_yank(Byte *, Byte *, int);	// save copy of "p" into a register
static Byte what_reg(void);		// what is letter of current YDreg
static void check_context(Byte);	// remember context for '' command
static Byte *swap_context(Byte *);	// goto new context for '' command
#endif							/* BB_FEATURE_VI_YANKMARK */
#ifdef BB_FEATURE_VI_CRASHME
static void crash_dummy();
static void crash_test();
static int crashme = 0;
#endif							/* BB_FEATURE_VI_CRASHME */


extern int vi_main(int argc, char **argv)
{
	int c;

#ifdef BB_FEATURE_VI_YANKMARK
	int i;
#endif							/* BB_FEATURE_VI_YANKMARK */

	CMrc= "\033[%d;%dH";	// Terminal Crusor motion ESC sequence
	CMup= "\033[A";		// move cursor up one line, same col
	CMdown="\n";		// move cursor down one line, same col
	Ceol= "\033[0K";	// Clear from cursor to end of line
	Ceos= "\033[0J";	// Clear from cursor to end of screen
	SOs = "\033[7m";	// Terminal standout mode on
	SOn = "\033[0m";	// Terminal standout mode off
	bell= "\007";		// Terminal bell sequence
#ifdef BB_FEATURE_VI_CRASHME
	(void) srand((long) getpid());
#endif							/* BB_FEATURE_VI_CRASHME */
	status_buffer = (Byte *) xmalloc(200);	// hold messages to user
#ifdef BB_FEATURE_VI_READONLY
	vi_readonly = readonly = FALSE;
	if (strncmp(argv[0], "view", 4) == 0) {
		readonly = TRUE;
		vi_readonly = TRUE;
	}
#endif							/* BB_FEATURE_VI_READONLY */
#ifdef BB_FEATURE_VI_SETOPTS
	autoindent = 1;
	ignorecase = 1;
	showmatch = 1;
#endif							/* BB_FEATURE_VI_SETOPTS */
#ifdef BB_FEATURE_VI_YANKMARK
	for (i = 0; i < 28; i++) {
		reg[i] = 0;
	}					// init the yank regs
#endif							/* BB_FEATURE_VI_YANKMARK */
#if defined(BB_FEATURE_VI_DOT_CMD) || defined(BB_FEATURE_VI_YANKMARK)
	modifying_cmds = (Byte *) "aAcCdDiIJoOpPrRsxX<>~";	// cmds modifying text[]
#endif							/* BB_FEATURE_VI_DOT_CMD */

	//  1-  process $HOME/.exrc file
	//  2-  process EXINIT variable from environment
	//  3-  process command line args
	while ((c = getopt(argc, argv, "hCR")) != -1) {
		switch (c) {
#ifdef BB_FEATURE_VI_CRASHME
		case 'C':
			crashme = 1;
			break;
#endif							/* BB_FEATURE_VI_CRASHME */
#ifdef BB_FEATURE_VI_READONLY
		case 'R':		// Read-only flag
			readonly = TRUE;
			break;
#endif							/* BB_FEATURE_VI_READONLY */
			//case 'r':	// recover flag-  ignore- we don't use tmp file
			//case 'x':	// encryption flag- ignore
			//case 'c':	// execute command first
			//case 'h':	// help -- just use default
		default:
			show_help();
			return 1;
		}
	}

	// The argv array can be used by the ":next"  and ":rewind" commands
	// save optind.
	fn_start = optind;	// remember first file name for :next and :rew
	save_argc = argc;

	//----- This is the main file handling loop --------------
	if (optind >= argc) {
		editing = 1;	// 0= exit,  1= one file,  2= multiple files
		edit_file(0);
	} else {
		for (; optind < argc; optind++) {
			editing = 1;	// 0=exit, 1=one file, 2+ =many files
			if (cfn != 0)
				free(cfn);
			cfn = (Byte *) strdup(argv[optind]);
			edit_file(cfn);
		}
	}
	//-----------------------------------------------------------

	return (0);
}

static void edit_file(Byte * fn)
{
	char c;
	int cnt, size, ch;

#ifdef BB_FEATURE_VI_USE_SIGNALS
	char *msg;
	int sig;
#endif							/* BB_FEATURE_VI_USE_SIGNALS */
#ifdef BB_FEATURE_VI_YANKMARK
	static Byte *cur_line;
#endif							/* BB_FEATURE_VI_YANKMARK */

	rawmode();
	rows = 24;
	columns = 80;
	ch= -1;
#ifdef BB_FEATURE_VI_WIN_RESIZE
	window_size_get(0);
#endif							/* BB_FEATURE_VI_WIN_RESIZE */
	new_screen(rows, columns);	// get memory for virtual screen

	cnt = file_size(fn);	// file size
	size = 2 * cnt;		// 200% of file size
	new_text(size);		// get a text[] buffer
	screenbegin = dot = end = text;
	if (fn != 0) {
		ch= file_insert(fn, text, cnt);
	}
	if (ch < 1) {
		(void) char_insert(text, '\n');	// start empty buf with dummy line
	}
	file_modified = FALSE;
#ifdef BB_FEATURE_VI_YANKMARK
	YDreg = 26;			// default Yank/Delete reg
	Ureg = 27;			// hold orig line for "U" cmd
	for (cnt = 0; cnt < 28; cnt++) {
		mark[cnt] = 0;
	}					// init the marks
	mark[26] = mark[27] = text;	// init "previous context"
#endif							/* BB_FEATURE_VI_YANKMARK */

	err_method = 1;		// flash
	last_forward_char = last_input_char = '\0';
	crow = 0;
	ccol = 0;
	edit_status();

#ifdef BB_FEATURE_VI_USE_SIGNALS
	signal(SIGHUP, catch_sig);
	signal(SIGINT, catch_sig);
	signal(SIGALRM, alarm_sig);
	signal(SIGTERM, catch_sig);
	signal(SIGQUIT, core_sig);
	signal(SIGILL, core_sig);
	signal(SIGTRAP, core_sig);
	signal(SIGIOT, core_sig);
	signal(SIGABRT, core_sig);
	signal(SIGFPE, core_sig);
	signal(SIGBUS, core_sig);
	signal(SIGSEGV, core_sig);
#ifdef SIGSYS
	signal(SIGSYS, core_sig);
#endif	
	signal(SIGWINCH, winch_sig);
	signal(SIGTSTP, suspend_sig);
	sig = setjmp(restart);
	if (sig != 0) {
		msg = "";
		if (sig == SIGWINCH)
			msg = "(window resize)";
		if (sig == SIGHUP)
			msg = "(hangup)";
		if (sig == SIGINT)
			msg = "(interrupt)";
		if (sig == SIGTERM)
			msg = "(terminate)";
		if (sig == SIGBUS)
			msg = "(bus error)";
		if (sig == SIGSEGV)
			msg = "(I tried to touch invalid memory)";
		if (sig == SIGALRM)
			msg = "(alarm)";

		psbs("-- caught signal %d %s--", sig, msg);
		screenbegin = dot = text;
	}
#endif							/* BB_FEATURE_VI_USE_SIGNALS */

	editing = 1;
	cmd_mode = 0;		// 0=command  1=insert  2='R'eplace
	cmdcnt = 0;
	tabstop = 8;
	offset = 0;			// no horizontal offset
	c = '\0';
#ifdef BB_FEATURE_VI_DOT_CMD
	if (last_modifying_cmd != 0)
		free(last_modifying_cmd);
	if (ioq_start != NULL)
		free(ioq_start);
	ioq = ioq_start = last_modifying_cmd = 0;
	adding2q = 0;
#endif							/* BB_FEATURE_VI_DOT_CMD */
	redraw(FALSE);			// dont force every col re-draw
	show_status_line();

	//------This is the main Vi cmd handling loop -----------------------
	while (editing > 0) {
#ifdef BB_FEATURE_VI_CRASHME
		if (crashme > 0) {
			if ((end - text) > 1) {
				crash_dummy();	// generate a random command
			} else {
				crashme = 0;
				dot =
					string_insert(text, (Byte *) "\n\n#####  Ran out of text to work on.  #####\n\n");	// insert the string
				refresh(FALSE);
			}
		}
#endif							/* BB_FEATURE_VI_CRASHME */
		last_input_char = c = get_one_char();	// get a cmd from user
#ifdef BB_FEATURE_VI_YANKMARK
		// save a copy of the current line- for the 'U" command
		if (begin_line(dot) != cur_line) {
			cur_line = begin_line(dot);
			text_yank(begin_line(dot), end_line(dot), Ureg);
		}
#endif							/* BB_FEATURE_VI_YANKMARK */
#ifdef BB_FEATURE_VI_DOT_CMD
		// These are commands that change text[].
		// Remember the input for the "." command
		if (!adding2q && ioq_start == 0
			&& strchr((char *) modifying_cmds, c) != NULL) {
			start_new_cmd_q(c);
		}
#endif							/* BB_FEATURE_VI_DOT_CMD */
		do_cmd(c);		// execute the user command
		//
		// poll to see if there is input already waiting. if we are
		// not able to display output fast enough to keep up, skip
		// the display update until we catch up with input.
		if (mysleep(0) == 0) {
			// no input pending- so update output
			refresh(FALSE);
			show_status_line();
		}
#ifdef BB_FEATURE_VI_CRASHME
		if (crashme > 0)
			crash_test();	// test editor variables
#endif							/* BB_FEATURE_VI_CRASHME */
	}
	//-------------------------------------------------------------------

	place_cursor(rows, 0, FALSE);	// go to bottom of screen
	clear_to_eol();		// Erase to end of line
	cookmode();
}

static Byte readbuffer[BUFSIZ];

#ifdef BB_FEATURE_VI_CRASHME
static int totalcmds = 0;
static int Mp = 85;		// Movement command Probability
static int Np = 90;		// Non-movement command Probability
static int Dp = 96;		// Delete command Probability
static int Ip = 97;		// Insert command Probability
static int Yp = 98;		// Yank command Probability
static int Pp = 99;		// Put command Probability
static int M = 0, N = 0, I = 0, D = 0, Y = 0, P = 0, U = 0;
char chars[20] = "\t012345 abcdABCD-=.$";
char *words[20] = { "this", "is", "a", "test",
	"broadcast", "the", "emergency", "of",
	"system", "quick", "brown", "fox",
	"jumped", "over", "lazy", "dogs",
	"back", "January", "Febuary", "March"
};
char *lines[20] = {
	"You should have received a copy of the GNU General Public License\n",
	"char c, cm, *cmd, *cmd1;\n",
	"generate a command by percentages\n",
	"Numbers may be typed as a prefix to some commands.\n",
	"Quit, discarding changes!\n",
	"Forced write, if permission originally not valid.\n",
	"In general, any ex or ed command (such as substitute or delete).\n",
	"I have tickets available for the Blazers vs LA Clippers for Monday, Janurary 1 at 1:00pm.\n",
	"Please get w/ me and I will go over it with you.\n",
	"The following is a list of scheduled, committed changes.\n",
	"1.   Launch Norton Antivirus (Start, Programs, Norton Antivirus)\n",
	"Reminder....Town Meeting in Central Perk cafe today at 3:00pm.\n",
	"Any question about transactions please contact Sterling Huxley.\n",
	"I will try to get back to you by Friday, December 31.\n",
	"This Change will be implemented on Friday.\n",
	"Let me know if you have problems accessing this;\n",
	"Sterling Huxley recently added you to the access list.\n",
	"Would you like to go to lunch?\n",
	"The last command will be automatically run.\n",
	"This is too much english for a computer geek.\n",
};
char *multilines[20] = {
	"You should have received a copy of the GNU General Public License\n",
	"char c, cm, *cmd, *cmd1;\n",
	"generate a command by percentages\n",
	"Numbers may be typed as a prefix to some commands.\n",
	"Quit, discarding changes!\n",
	"Forced write, if permission originally not valid.\n",
	"In general, any ex or ed command (such as substitute or delete).\n",
	"I have tickets available for the Blazers vs LA Clippers for Monday, Janurary 1 at 1:00pm.\n",
	"Please get w/ me and I will go over it with you.\n",
	"The following is a list of scheduled, committed changes.\n",
	"1.   Launch Norton Antivirus (Start, Programs, Norton Antivirus)\n",
	"Reminder....Town Meeting in Central Perk cafe today at 3:00pm.\n",
	"Any question about transactions please contact Sterling Huxley.\n",
	"I will try to get back to you by Friday, December 31.\n",
	"This Change will be implemented on Friday.\n",
	"Let me know if you have problems accessing this;\n",
	"Sterling Huxley recently added you to the access list.\n",
	"Would you like to go to lunch?\n",
	"The last command will be automatically run.\n",
	"This is too much english for a computer geek.\n",
};

// create a random command to execute
static void crash_dummy()
{
	static int sleeptime;	// how long to pause between commands
	char c, cm, *cmd, *cmd1;
	int i, cnt, thing, rbi, startrbi, percent;

	// "dot" movement commands
	cmd1 = " \n\r\002\004\005\006\025\0310^$-+wWeEbBhjklHL";

	// is there already a command running?
	if (strlen((char *) readbuffer) > 0)
		goto cd1;
  cd0:
	startrbi = rbi = 0;
	sleeptime = 0;		// how long to pause between commands
	memset(readbuffer, '\0', BUFSIZ - 1);	// clear the read buffer
	// generate a command by percentages
	percent = (int) lrand48() % 100;	// get a number from 0-99
	if (percent < Mp) {	//  Movement commands
		// available commands
		cmd = cmd1;
		M++;
	} else if (percent < Np) {	//  non-movement commands
		cmd = "mz<>\'\"";	// available commands
		N++;
	} else if (percent < Dp) {	//  Delete commands
		cmd = "dx";		// available commands
		D++;
	} else if (percent < Ip) {	//  Inset commands
		cmd = "iIaAsrJ";	// available commands
		I++;
	} else if (percent < Yp) {	//  Yank commands
		cmd = "yY";		// available commands
		Y++;
	} else if (percent < Pp) {	//  Put commands
		cmd = "pP";		// available commands
		P++;
	} else {
		// We do not know how to handle this command, try again
		U++;
		goto cd0;
	}
	// randomly pick one of the available cmds from "cmd[]"
	i = (int) lrand48() % strlen(cmd);
	cm = cmd[i];
	if (strchr(":\024", cm))
		goto cd0;		// dont allow colon or ctrl-T commands
	readbuffer[rbi++] = cm;	// put cmd into input buffer

	// now we have the command-
	// there are 1, 2, and multi char commands
	// find out which and generate the rest of command as necessary
	if (strchr("dmryz<>\'\"", cm)) {	// 2-char commands
		cmd1 = " \n\r0$^-+wWeEbBhjklHL";
		if (cm == 'm' || cm == '\'' || cm == '\"') {	// pick a reg[]
			cmd1 = "abcdefghijklmnopqrstuvwxyz";
		}
		thing = (int) lrand48() % strlen(cmd1);	// pick a movement command
		c = cmd1[thing];
		readbuffer[rbi++] = c;	// add movement to input buffer
	}
	if (strchr("iIaAsc", cm)) {	// multi-char commands
		if (cm == 'c') {
			// change some thing
			thing = (int) lrand48() % strlen(cmd1);	// pick a movement command
			c = cmd1[thing];
			readbuffer[rbi++] = c;	// add movement to input buffer
		}
		thing = (int) lrand48() % 4;	// what thing to insert
		cnt = (int) lrand48() % 10;	// how many to insert
		for (i = 0; i < cnt; i++) {
			if (thing == 0) {	// insert chars
				readbuffer[rbi++] = chars[((int) lrand48() % strlen(chars))];
			} else if (thing == 1) {	// insert words
				strcat((char *) readbuffer, words[(int) lrand48() % 20]);
				strcat((char *) readbuffer, " ");
				sleeptime = 0;	// how fast to type
			} else if (thing == 2) {	// insert lines
				strcat((char *) readbuffer, lines[(int) lrand48() % 20]);
				sleeptime = 0;	// how fast to type
			} else {	// insert multi-lines
				strcat((char *) readbuffer, multilines[(int) lrand48() % 20]);
				sleeptime = 0;	// how fast to type
			}
		}
		strcat((char *) readbuffer, "\033");
	}
  cd1:
	totalcmds++;
	if (sleeptime > 0)
		(void) mysleep(sleeptime);	// sleep 1/100 sec
}

// test to see if there are any errors
static void crash_test()
{
	static time_t oldtim;
	time_t tim;
	char d[2], buf[BUFSIZ], msg[BUFSIZ];

	msg[0] = '\0';
	if (end < text) {
		strcat((char *) msg, "end<text ");
	}
	if (end > textend) {
		strcat((char *) msg, "end>textend ");
	}
	if (dot < text) {
		strcat((char *) msg, "dot<text ");
	}
	if (dot > end) {
		strcat((char *) msg, "dot>end ");
	}
	if (screenbegin < text) {
		strcat((char *) msg, "screenbegin<text ");
	}
	if (screenbegin > end - 1) {
		strcat((char *) msg, "screenbegin>end-1 ");
	}

	if (strlen(msg) > 0) {
		alarm(0);
		sprintf(buf, "\n\n%d: \'%c\' %s\n\n\n%s[Hit return to continue]%s",
			totalcmds, last_input_char, msg, SOs, SOn);
		write(1, buf, strlen(buf));
		while (read(0, d, 1) > 0) {
			if (d[0] == '\n' || d[0] == '\r')
				break;
		}
		alarm(3);
	}
	tim = (time_t) time((time_t *) 0);
	if (tim >= (oldtim + 3)) {
		sprintf((char *) status_buffer,
				"Tot=%d: M=%d N=%d I=%d D=%d Y=%d P=%d U=%d size=%d",
				totalcmds, M, N, I, D, Y, P, U, end - text + 1);
		oldtim = tim;
	}
	return;
}
#endif							/* BB_FEATURE_VI_CRASHME */

//---------------------------------------------------------------------
//----- the Ascii Chart -----------------------------------------------
//
//  00 nul   01 soh   02 stx   03 etx   04 eot   05 enq   06 ack   07 bel
//  08 bs    09 ht    0a nl    0b vt    0c np    0d cr    0e so    0f si
//  10 dle   11 dc1   12 dc2   13 dc3   14 dc4   15 nak   16 syn   17 etb
//  18 can   19 em    1a sub   1b esc   1c fs    1d gs    1e rs    1f us
//  20 sp    21 !     22 "     23 #     24 $     25 %     26 &     27 '
//  28 (     29 )     2a *     2b +     2c ,     2d -     2e .     2f /
//  30 0     31 1     32 2     33 3     34 4     35 5     36 6     37 7
//  38 8     39 9     3a :     3b ;     3c <     3d =     3e >     3f ?
//  40 @     41 A     42 B     43 C     44 D     45 E     46 F     47 G
//  48 H     49 I     4a J     4b K     4c L     4d M     4e N     4f O
//  50 P     51 Q     52 R     53 S     54 T     55 U     56 V     57 W
//  58 X     59 Y     5a Z     5b [     5c \     5d ]     5e ^     5f _
//  60 `     61 a     62 b     63 c     64 d     65 e     66 f     67 g
//  68 h     69 i     6a j     6b k     6c l     6d m     6e n     6f o
//  70 p     71 q     72 r     73 s     74 t     75 u     76 v     77 w
//  78 x     79 y     7a z     7b {     7c |     7d }     7e ~     7f del
//---------------------------------------------------------------------

//----- Execute a Vi Command -----------------------------------
static void do_cmd(Byte c)
{
	Byte c1, *p, *q, *msg, buf[9], *save_dot;
	int cnt, i, j, dir, yf;

	c1 = c;				// quiet the compiler
	cnt = yf = dir = 0;	// quiet the compiler
	p = q = save_dot = msg = buf;	// quiet the compiler
	memset(buf, '\0', 9);	// clear buf
	
	/* if this is a cursor key, skip these checks */
	switch (c) {
		case VI_K_UP:
		case VI_K_DOWN:
		case VI_K_LEFT:
		case VI_K_RIGHT:
		case VI_K_HOME:
		case VI_K_END:
		case VI_K_PAGEUP:
		case VI_K_PAGEDOWN:
			goto key_cmd_mode;
	}

	if (cmd_mode == 2) {
		// we are 'R'eplacing the current *dot with new char
		if (*dot == '\n') {
			// don't Replace past E-o-l
			cmd_mode = 1;	// convert to insert
		} else {
			if (1 <= c && c <= 127) {	// only ASCII chars
				if (c != 27)
					dot = yank_delete(dot, dot, 0, YANKDEL);	// delete char
				dot = char_insert(dot, c);	// insert new char
			}
			goto dc1;
		}
	}
	if (cmd_mode == 1) {
		//  hitting "Insert" twice means "R" replace mode
		if (c == VI_K_INSERT) goto dc5;
		// insert the char c at "dot"
		if (1 <= c && c <= 127) {
			dot = char_insert(dot, c);	// only ASCII chars
		}
		goto dc1;
	}

key_cmd_mode:
	switch (c) {
		//case 0x01:	// soh
		//case 0x09:	// ht
		//case 0x0b:	// vt
		//case 0x0e:	// so
		//case 0x0f:	// si
		//case 0x10:	// dle
		//case 0x11:	// dc1
		//case 0x13:	// dc3
#ifdef BB_FEATURE_VI_CRASHME
	case 0x14:			// dc4  ctrl-T
		crashme = (crashme == 0) ? 1 : 0;
		break;
#endif							/* BB_FEATURE_VI_CRASHME */
		//case 0x16:	// syn
		//case 0x17:	// etb
		//case 0x18:	// can
		//case 0x1c:	// fs
		//case 0x1d:	// gs
		//case 0x1e:	// rs
		//case 0x1f:	// us
		//case '!':	// !- 
		//case '#':	// #- 
		//case '&':	// &- 
		//case '(':	// (- 
		//case ')':	// )- 
		//case '*':	// *- 
		//case ',':	// ,- 
		//case '=':	// =- 
		//case '@':	// @- 
		//case 'F':	// F- 
		//case 'K':	// K- 
		//case 'Q':	// Q- 
		//case 'S':	// S- 
		//case 'T':	// T- 
		//case 'V':	// V- 
		//case '[':	// [- 
		//case '\\':	// \- 
		//case ']':	// ]- 
		//case '_':	// _- 
		//case '`':	// `- 
		//case 'g':	// g- 
		//case 'u':	// u- FIXME- there is no undo
		//case 'v':	// v- 
	default:			// unrecognised command
		buf[0] = c;
		buf[1] = '\0';
		if (c <= ' ') {
			buf[0] = '^';
			buf[1] = c + '@';
			buf[2] = '\0';
		}
		ni((Byte *) buf);
		end_cmd_q();	// stop adding to q
	case 0x00:			// nul- ignore
		break;
	case 2:			// ctrl-B  scroll up   full screen
	case VI_K_PAGEUP:	// Cursor Key Page Up
		dot_scroll(rows - 2, -1);
		break;
#ifdef BB_FEATURE_VI_USE_SIGNALS
	case 0x03:			// ctrl-C   interrupt
		longjmp(restart, 1);
		break;
	case 26:			// ctrl-Z suspend
		suspend_sig(SIGTSTP);
		break;
#endif							/* BB_FEATURE_VI_USE_SIGNALS */
	case 4:			// ctrl-D  scroll down half screen
		dot_scroll((rows - 2) / 2, 1);
		break;
	case 5:			// ctrl-E  scroll down one line
		dot_scroll(1, 1);
		break;
	case 6:			// ctrl-F  scroll down full screen
	case VI_K_PAGEDOWN:	// Cursor Key Page Down
		dot_scroll(rows - 2, 1);
		break;
	case 7:			// ctrl-G  show current status
		edit_status();
		break;
	case 'h':			// h- move left
	case VI_K_LEFT:	// cursor key Left
	case 8:			// ctrl-H- move left    (This may be ERASE char)
	case 127:			// DEL- move left   (This may be ERASE char)
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dot_left();
		break;
	case 10:			// Newline ^J
	case 'j':			// j- goto next line, same col
	case VI_K_DOWN:	// cursor key Down
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dot_next();		// go to next B-o-l
		dot = move_to_col(dot, ccol + offset);	// try stay in same col
		break;
	case 12:			// ctrl-L  force redraw whole screen
	case 18:			// ctrl-R  force redraw
		place_cursor(0, 0, FALSE);	// put cursor in correct place
		clear_to_eos();	// tel terminal to erase display
		(void) mysleep(10);
		screen_erase();	// erase the internal screen buffer
		refresh(TRUE);	// this will redraw the entire display
		break;
	case 13:			// Carriage Return ^M
	case '+':			// +- goto next line
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dot_next();
		dot_skip_over_ws();
		break;
	case 21:			// ctrl-U  scroll up   half screen
		dot_scroll((rows - 2) / 2, -1);
		break;
	case 25:			// ctrl-Y  scroll up one line
		dot_scroll(1, -1);
		break;
	case 27:			// esc
		if (cmd_mode == 0)
			indicate_error(c);
		cmd_mode = 0;	// stop insrting
		end_cmd_q();
		*status_buffer = '\0';	// clear status buffer
		break;
	case ' ':			// move right
	case 'l':			// move right
	case VI_K_RIGHT:	// Cursor Key Right
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dot_right();
		break;
#ifdef BB_FEATURE_VI_YANKMARK
	case '"':			// "- name a register to use for Delete/Yank
		c1 = get_one_char();
		c1 = tolower(c1);
		if (islower(c1)) {
			YDreg = c1 - 'a';
		} else {
			indicate_error(c);
		}
		break;
	case '\'':			// '- goto a specific mark
		c1 = get_one_char();
		c1 = tolower(c1);
		if (islower(c1)) {
			c1 = c1 - 'a';
			// get the b-o-l
			q = mark[(int) c1];
			if (text <= q && q < end) {
				dot = q;
				dot_begin();	// go to B-o-l
				dot_skip_over_ws();
			}
		} else if (c1 == '\'') {	// goto previous context
			dot = swap_context(dot);	// swap current and previous context
			dot_begin();	// go to B-o-l
			dot_skip_over_ws();
		} else {
			indicate_error(c);
		}
		break;
	case 'm':			// m- Mark a line
		// this is really stupid.  If there are any inserts or deletes
		// between text[0] and dot then this mark will not point to the
		// correct location! It could be off by many lines!
		// Well..., at least its quick and dirty.
		c1 = get_one_char();
		c1 = tolower(c1);
		if (islower(c1)) {
			c1 = c1 - 'a';
			// remember the line
			mark[(int) c1] = dot;
		} else {
			indicate_error(c);
		}
		break;
	case 'P':			// P- Put register before
	case 'p':			// p- put register after
		p = reg[YDreg];
		if (p == 0) {
			psbs("Nothing in register %c", what_reg());
			break;
		}
		// are we putting whole lines or strings
		if (strchr((char *) p, '\n') != NULL) {
			if (c == 'P') {
				dot_begin();	// putting lines- Put above
			}
			if (c == 'p') {
				// are we putting after very last line?
				if (end_line(dot) == (end - 1)) {
					dot = end;	// force dot to end of text[]
				} else {
					dot_next();	// next line, then put before
				}
			}
		} else {
			if (c == 'p')
				dot_right();	// move to right, can move to NL
		}
		dot = string_insert(dot, p);	// insert the string
		end_cmd_q();	// stop adding to q
		break;
	case 'U':			// U- Undo; replace current line with original version
		if (reg[Ureg] != 0) {
			p = begin_line(dot);
			q = end_line(dot);
			p = text_hole_delete(p, q);	// delete cur line
			p = string_insert(p, reg[Ureg]);	// insert orig line
			dot = p;
			dot_skip_over_ws();
		}
		break;
#endif							/* BB_FEATURE_VI_YANKMARK */
	case '$':			// $- goto end of line
	case VI_K_END:		// Cursor Key End
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dot = end_line(dot + 1);
		break;
	case '%':			// %- find matching char of pair () [] {}
		for (q = dot; q < end && *q != '\n'; q++) {
			if (strchr("()[]{}", *q) != NULL) {
				// we found half of a pair
				p = find_pair(q, *q);
				if (p == NULL) {
					indicate_error(c);
				} else {
					dot = p;
				}
				break;
			}
		}
		if (*q == '\n')
			indicate_error(c);
		break;
	case 'f':			// f- forward to a user specified char
		last_forward_char = get_one_char();	// get the search char
		//
		// dont seperate these two commands. 'f' depends on ';'
		//
		//**** fall thru to ... 'i'
	case ';':			// ;- look at rest of line for last forward char
		if (cmdcnt-- > 1) {
			do_cmd(';');
		}				// repeat cnt
		if (last_forward_char == 0) break;
		q = dot + 1;
		while (q < end - 1 && *q != '\n' && *q != last_forward_char) {
			q++;
		}
		if (*q == last_forward_char)
			dot = q;
		break;
	case '-':			// -- goto prev line
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dot_prev();
		dot_skip_over_ws();
		break;
#ifdef BB_FEATURE_VI_DOT_CMD
	case '.':			// .- repeat the last modifying command
		// Stuff the last_modifying_cmd back into stdin
		// and let it be re-executed.
		if (last_modifying_cmd != 0) {
			ioq = ioq_start = (Byte *) strdup((char *) last_modifying_cmd);
		}
		break;
#endif							/* BB_FEATURE_VI_DOT_CMD */
#ifdef BB_FEATURE_VI_SEARCH
	case '?':			// /- search for a pattern
	case '/':			// /- search for a pattern
		buf[0] = c;
		buf[1] = '\0';
		q = get_input_line(buf);	// get input line- use "status line"
		if (strlen((char *) q) == 1)
			goto dc3;	// if no pat re-use old pat
		if (strlen((char *) q) > 1) {	// new pat- save it and find
			// there is a new pat
			if (last_search_pattern != 0) {
				free(last_search_pattern);
			}
			last_search_pattern = (Byte *) strdup((char *) q);
			goto dc3;	// now find the pattern
		}
		// user changed mind and erased the "/"-  do nothing
		break;
	case 'N':			// N- backward search for last pattern
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dir = BACK;		// assume BACKWARD search
		p = dot - 1;
		if (last_search_pattern[0] == '?') {
			dir = FORWARD;
			p = dot + 1;
		}
		goto dc4;		// now search for pattern
		break;
	case 'n':			// n- repeat search for last pattern
		// search rest of text[] starting at next char
		// if search fails return orignal "p" not the "p+1" address
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
	  dc3:
		if (last_search_pattern == 0) {
			msg = (Byte *) "No previous regular expression";
			goto dc2;
		}
		if (last_search_pattern[0] == '/') {
			dir = FORWARD;	// assume FORWARD search
			p = dot + 1;
		}
		if (last_search_pattern[0] == '?') {
			dir = BACK;
			p = dot - 1;
		}
	  dc4:
		q = char_search(p, last_search_pattern + 1, dir, FULL);
		if (q != NULL) {
			dot = q;	// good search, update "dot"
			msg = (Byte *) "";
			goto dc2;
		}
		// no pattern found between "dot" and "end"- continue at top
		p = text;
		if (dir == BACK) {
			p = end - 1;
		}
		q = char_search(p, last_search_pattern + 1, dir, FULL);
		if (q != NULL) {	// found something
			dot = q;	// found new pattern- goto it
			msg = (Byte *) "search hit BOTTOM, continuing at TOP";
			if (dir == BACK) {
				msg = (Byte *) "search hit TOP, continuing at BOTTOM";
			}
		} else {
			msg = (Byte *) "Pattern not found";
		}
	  dc2:
		psbs("%s", msg);
		break;
	case '{':			// {- move backward paragraph
		q = char_search(dot, (Byte *) "\n\n", BACK, FULL);
		if (q != NULL) {	// found blank line
			dot = next_line(q);	// move to next blank line
		}
		break;
	case '}':			// }- move forward paragraph
		q = char_search(dot, (Byte *) "\n\n", FORWARD, FULL);
		if (q != NULL) {	// found blank line
			dot = next_line(q);	// move to next blank line
		}
		break;
#endif							/* BB_FEATURE_VI_SEARCH */
	case '0':			// 0- goto begining of line
	case '1':			// 1- 
	case '2':			// 2- 
	case '3':			// 3- 
	case '4':			// 4- 
	case '5':			// 5- 
	case '6':			// 6- 
	case '7':			// 7- 
	case '8':			// 8- 
	case '9':			// 9- 
		if (c == '0' && cmdcnt < 1) {
			dot_begin();	// this was a standalone zero
		} else {
			cmdcnt = cmdcnt * 10 + (c - '0');	// this 0 is part of a number
		}
		break;
	case ':':			// :- the colon mode commands
		p = get_input_line((Byte *) ":");	// get input line- use "status line"
#ifdef BB_FEATURE_VI_COLON
		colon(p);		// execute the command
#else							/* BB_FEATURE_VI_COLON */
		if (*p == ':')
			p++;				// move past the ':'
		cnt = strlen((char *) p);
		if (cnt <= 0)
			break;
		if (strncasecmp((char *) p, "quit", cnt) == 0 ||
			strncasecmp((char *) p, "q!", cnt) == 0) {	// delete lines
			if (file_modified == TRUE && p[1] != '!') {
				psbs("No write since last change (:quit! overrides)");
			} else {
				editing = 0;
			}
		} else if (strncasecmp((char *) p, "write", cnt) == 0 ||
				   strncasecmp((char *) p, "wq", cnt) == 0) {
			cnt = file_write(cfn, text, end - 1);
			file_modified = FALSE;
			psb("\"%s\" %dL, %dC", cfn, count_lines(text, end - 1), cnt);
			if (p[1] == 'q') {
				editing = 0;
			}
		} else if (strncasecmp((char *) p, "file", cnt) == 0 ) {
			edit_status();			// show current file status
		} else if (sscanf((char *) p, "%d", &j) > 0) {
			dot = find_line(j);		// go to line # j
			dot_skip_over_ws();
		} else {		// unrecognised cmd
			ni((Byte *) p);
		}
#endif							/* BB_FEATURE_VI_COLON */
		break;
	case '<':			// <- Left  shift something
	case '>':			// >- Right shift something
		cnt = count_lines(text, dot);	// remember what line we are on
		c1 = get_one_char();	// get the type of thing to delete
		find_range(&p, &q, c1);
		(void) yank_delete(p, q, 1, YANKONLY);	// save copy before change
		p = begin_line(p);
		q = end_line(q);
		i = count_lines(p, q);	// # of lines we are shifting
		for ( ; i > 0; i--, p = next_line(p)) {
			if (c == '<') {
				// shift left- remove tab or 8 spaces
				if (*p == '\t') {
					// shrink buffer 1 char
					(void) text_hole_delete(p, p);
				} else if (*p == ' ') {
					// we should be calculating columns, not just SPACE
					for (j = 0; *p == ' ' && j < tabstop; j++) {
						(void) text_hole_delete(p, p);
					}
				}
			} else if (c == '>') {
				// shift right -- add tab or 8 spaces
				(void) char_insert(p, '\t');
			}
		}
		dot = find_line(cnt);	// what line were we on
		dot_skip_over_ws();
		end_cmd_q();	// stop adding to q
		break;
	case 'A':			// A- append at e-o-l
		dot_end();		// go to e-o-l
		//**** fall thru to ... 'a'
	case 'a':			// a- append after current char
		if (*dot != '\n')
			dot++;
		goto dc_i;
		break;
	case 'B':			// B- back a blank-delimited Word
	case 'E':			// E- end of a blank-delimited word
	case 'W':			// W- forward a blank-delimited word
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dir = FORWARD;
		if (c == 'B')
			dir = BACK;
		if (c == 'W' || isspace(dot[dir])) {
			dot = skip_thing(dot, 1, dir, S_TO_WS);
			dot = skip_thing(dot, 2, dir, S_OVER_WS);
		}
		if (c != 'W')
			dot = skip_thing(dot, 1, dir, S_BEFORE_WS);
		break;
	case 'C':			// C- Change to e-o-l
	case 'D':			// D- delete to e-o-l
		save_dot = dot;
		dot = dollar_line(dot);	// move to before NL
		// copy text into a register and delete
		dot = yank_delete(save_dot, dot, 0, YANKDEL);	// delete to e-o-l
		if (c == 'C')
			goto dc_i;	// start inserting
#ifdef BB_FEATURE_VI_DOT_CMD
		if (c == 'D')
			end_cmd_q();	// stop adding to q
#endif							/* BB_FEATURE_VI_DOT_CMD */
		break;
	case 'G':		// G- goto to a line number (default= E-O-F)
		dot = end - 1;				// assume E-O-F
		if (cmdcnt > 0) {
			dot = find_line(cmdcnt);	// what line is #cmdcnt
		}
		dot_skip_over_ws();
		break;
	case 'H':			// H- goto top line on screen
		dot = screenbegin;
		if (cmdcnt > (rows - 1)) {
			cmdcnt = (rows - 1);
		}
		if (cmdcnt-- > 1) {
			do_cmd('+');
		}				// repeat cnt
		dot_skip_over_ws();
		break;
	case 'I':			// I- insert before first non-blank
		dot_begin();	// 0
		dot_skip_over_ws();
		//**** fall thru to ... 'i'
	case 'i':			// i- insert before current char
	case VI_K_INSERT:	// Cursor Key Insert
	  dc_i:
		cmd_mode = 1;	// start insrting
		psb("-- Insert --");
		break;
	case 'J':			// J- join current and next lines together
		if (cmdcnt-- > 2) {
			do_cmd(c);
		}				// repeat cnt
		dot_end();		// move to NL
		if (dot < end - 1) {	// make sure not last char in text[]
			*dot++ = ' ';	// replace NL with space
			while (isblnk(*dot)) {	// delete leading WS
				dot_delete();
			}
		}
		end_cmd_q();	// stop adding to q
		break;
	case 'L':			// L- goto bottom line on screen
		dot = end_screen();
		if (cmdcnt > (rows - 1)) {
			cmdcnt = (rows - 1);
		}
		if (cmdcnt-- > 1) {
			do_cmd('-');
		}				// repeat cnt
		dot_begin();
		dot_skip_over_ws();
		break;
	case 'M':			// M- goto middle line on screen
		dot = screenbegin;
		for (cnt = 0; cnt < (rows-1) / 2; cnt++)
			dot = next_line(dot);
		break;
	case 'O':			// O- open a empty line above
		//    0i\n ESC -i
		p = begin_line(dot);
		if (p[-1] == '\n') {
			dot_prev();
	case 'o':			// o- open a empty line below; Yes, I know it is in the middle of the "if (..."
			dot_end();
			dot = char_insert(dot, '\n');
		} else {
			dot_begin();	// 0
			dot = char_insert(dot, '\n');	// i\n ESC
			dot_prev();	// -
		}
		goto dc_i;
		break;
	case 'R':			// R- continuous Replace char
	  dc5:
		cmd_mode = 2;
		psb("-- Replace --");
		break;
	case 'X':			// X- delete char before dot
	case 'x':			// x- delete the current char
	case 's':			// s- substitute the current char
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dir = 0;
		if (c == 'X')
			dir = -1;
		if (dot[dir] != '\n') {
			if (c == 'X')
				dot--;	// delete prev char
			dot = yank_delete(dot, dot, 0, YANKDEL);	// delete char
		}
		if (c == 's')
			goto dc_i;	// start insrting
		end_cmd_q();	// stop adding to q
		break;
	case 'Z':			// Z- if modified, {write}; exit
		// ZZ means to save file (if necessary), then exit
		c1 = get_one_char();
		if (c1 != 'Z') {
			indicate_error(c);
			break;
		}
		if (file_modified == TRUE
#ifdef BB_FEATURE_VI_READONLY
			&& vi_readonly == FALSE
			&& readonly == FALSE
#endif							/* BB_FEATURE_VI_READONLY */
			) {
			cnt = file_write(cfn, text, end - 1);
			if (cnt == (end - 1 - text + 1)) {
				editing = 0;
			}
		} else {
			editing = 0;
		}
		break;
	case '^':			// ^- move to first non-blank on line
		dot_begin();
		dot_skip_over_ws();
		break;
	case 'b':			// b- back a word
	case 'e':			// e- end of word
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dir = FORWARD;
		if (c == 'b')
			dir = BACK;
		if ((dot + dir) < text || (dot + dir) > end - 1)
			break;
		dot += dir;
		if (isspace(*dot)) {
			dot = skip_thing(dot, (c == 'e') ? 2 : 1, dir, S_OVER_WS);
		}
		if (isalnum(*dot) || *dot == '_') {
			dot = skip_thing(dot, 1, dir, S_END_ALNUM);
		} else if (ispunct(*dot)) {
			dot = skip_thing(dot, 1, dir, S_END_PUNCT);
		}
		break;
	case 'c':			// c- change something
	case 'd':			// d- delete something
#ifdef BB_FEATURE_VI_YANKMARK
	case 'y':			// y- yank   something
	case 'Y':			// Y- Yank a line
#endif							/* BB_FEATURE_VI_YANKMARK */
		yf = YANKDEL;	// assume either "c" or "d"
#ifdef BB_FEATURE_VI_YANKMARK
		if (c == 'y' || c == 'Y')
			yf = YANKONLY;
#endif							/* BB_FEATURE_VI_YANKMARK */
		c1 = 'y';
		if (c != 'Y')
			c1 = get_one_char();	// get the type of thing to delete
		find_range(&p, &q, c1);
		if (c1 == 27) {	// ESC- user changed mind and wants out
			c = c1 = 27;	// Escape- do nothing
		} else if (strchr("wW", c1)) {
			if (c == 'c') {
				// don't include trailing WS as part of word
				while (isblnk(*q)) {
					if (q <= text || q[-1] == '\n')
						break;
					q--;
				}
			}
			dot = yank_delete(p, q, 0, yf);	// delete word
		} else if (strchr("^0bBeEft$", c1)) {
			// single line copy text into a register and delete
			dot = yank_delete(p, q, 0, yf);	// delete word
		} else if (strchr("cdykjHL%+-{}\r\n", c1)) {
			// multiple line copy text into a register and delete
			dot = yank_delete(p, q, 1, yf);	// delete lines
			if (c == 'c') {
				dot = char_insert(dot, '\n');
				// on the last line of file don't move to prev line
				if (dot != (end-1)) {
					dot_prev();
				}
			} else if (c == 'd') {
				dot_begin();
				dot_skip_over_ws();
			}
		} else {
			// could not recognize object
			c = c1 = 27;	// error-
			indicate_error(c);
		}
		if (c1 != 27) {
			// if CHANGING, not deleting, start inserting after the delete
			if (c == 'c') {
				strcpy((char *) buf, "Change");
				goto dc_i;	// start inserting
			}
			if (c == 'd') {
				strcpy((char *) buf, "Delete");
			}
#ifdef BB_FEATURE_VI_YANKMARK
			if (c == 'y' || c == 'Y') {
				strcpy((char *) buf, "Yank");
			}
			p = reg[YDreg];
			q = p + strlen((char *) p);
			for (cnt = 0; p <= q; p++) {
				if (*p == '\n')
					cnt++;
			}
			psb("%s %d lines (%d chars) using [%c]",
				buf, cnt, strlen((char *) reg[YDreg]), what_reg());
#endif							/* BB_FEATURE_VI_YANKMARK */
			end_cmd_q();	// stop adding to q
		}
		break;
	case 'k':			// k- goto prev line, same col
	case VI_K_UP:		// cursor key Up
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		dot_prev();
		dot = move_to_col(dot, ccol + offset);	// try stay in same col
		break;
	case 'r':			// r- replace the current char with user input
		c1 = get_one_char();	// get the replacement char
		if (*dot != '\n') {
			*dot = c1;
			file_modified = TRUE;	// has the file been modified
		}
		end_cmd_q();	// stop adding to q
		break;
	case 't':			// t- move to char prior to next x
                last_forward_char = get_one_char();
                do_cmd(';');
                if (*dot == last_forward_char)
                        dot_left();
                last_forward_char= 0;
		break;
	case 'w':			// w- forward a word
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		if (isalnum(*dot) || *dot == '_') {	// we are on ALNUM
			dot = skip_thing(dot, 1, FORWARD, S_END_ALNUM);
		} else if (ispunct(*dot)) {	// we are on PUNCT
			dot = skip_thing(dot, 1, FORWARD, S_END_PUNCT);
		}
		if (dot < end - 1)
			dot++;		// move over word
		if (isspace(*dot)) {
			dot = skip_thing(dot, 2, FORWARD, S_OVER_WS);
		}
		break;
	case 'z':			// z-
		c1 = get_one_char();	// get the replacement char
		cnt = 0;
		if (c1 == '.')
			cnt = (rows - 2) / 2;	// put dot at center
		if (c1 == '-')
			cnt = rows - 2;	// put dot at bottom
		screenbegin = begin_line(dot);	// start dot at top
		dot_scroll(cnt, -1);
		break;
	case '|':			// |- move to column "cmdcnt"
		dot = move_to_col(dot, cmdcnt - 1);	// try to move to column
		break;
	case '~':			// ~- flip the case of letters   a-z -> A-Z
		if (cmdcnt-- > 1) {
			do_cmd(c);
		}				// repeat cnt
		if (islower(*dot)) {
			*dot = toupper(*dot);
			file_modified = TRUE;	// has the file been modified
		} else if (isupper(*dot)) {
			*dot = tolower(*dot);
			file_modified = TRUE;	// has the file been modified
		}
		dot_right();
		end_cmd_q();	// stop adding to q
		break;
		//----- The Cursor and Function Keys -----------------------------
	case VI_K_HOME:	// Cursor Key Home
		dot_begin();
		break;
		// The Fn keys could point to do_macro which could translate them
	case VI_K_FUN1:	// Function Key F1
	case VI_K_FUN2:	// Function Key F2
	case VI_K_FUN3:	// Function Key F3
	case VI_K_FUN4:	// Function Key F4
	case VI_K_FUN5:	// Function Key F5
	case VI_K_FUN6:	// Function Key F6
	case VI_K_FUN7:	// Function Key F7
	case VI_K_FUN8:	// Function Key F8
	case VI_K_FUN9:	// Function Key F9
	case VI_K_FUN10:	// Function Key F10
	case VI_K_FUN11:	// Function Key F11
	case VI_K_FUN12:	// Function Key F12
		break;
	}

  dc1:
	// if text[] just became empty, add back an empty line
	if (end == text) {
		(void) char_insert(text, '\n');	// start empty buf with dummy line
		dot = text;
	}
	// it is OK for dot to exactly equal to end, otherwise check dot validity
	if (dot != end) {
		dot = bound_dot(dot);	// make sure "dot" is valid
	}
#ifdef BB_FEATURE_VI_YANKMARK
	check_context(c);	// update the current context
#endif							/* BB_FEATURE_VI_YANKMARK */

	if (!isdigit(c))
		cmdcnt = 0;		// cmd was not a number, reset cmdcnt
	cnt = dot - begin_line(dot);
	// Try to stay off of the Newline
	if (*dot == '\n' && cnt > 0 && cmd_mode == 0)
		dot--;
}

//----- The Colon commands -------------------------------------
#ifdef BB_FEATURE_VI_COLON
static Byte *get_one_address(Byte * p, int *addr)	// get colon addr, if present
{
	int st;
	Byte *q;

#ifdef BB_FEATURE_VI_YANKMARK
	Byte c;
#endif							/* BB_FEATURE_VI_YANKMARK */
#ifdef BB_FEATURE_VI_SEARCH
	Byte *pat, buf[BUFSIZ];
#endif							/* BB_FEATURE_VI_SEARCH */

	*addr = -1;			// assume no addr
	if (*p == '.') {	// the current line
		p++;
		q = begin_line(dot);
		*addr = count_lines(text, q);
#ifdef BB_FEATURE_VI_YANKMARK
	} else if (*p == '\'') {	// is this a mark addr
		p++;
		c = tolower(*p);
		p++;
		if (c >= 'a' && c <= 'z') {
			// we have a mark
			c = c - 'a';
			q = mark[(int) c];
			if (q != NULL) {	// is mark valid
				*addr = count_lines(text, q);	// count lines
			}
		}
#endif							/* BB_FEATURE_VI_YANKMARK */
#ifdef BB_FEATURE_VI_SEARCH
	} else if (*p == '/') {	// a search pattern
		q = buf;
		for (p++; *p; p++) {
			if (*p == '/')
				break;
			*q++ = *p;
			*q = '\0';
		}
		pat = (Byte *) strdup((char *) buf);	// save copy of pattern
		if (*p == '/')
			p++;
		q = char_search(dot, pat, FORWARD, FULL);
		if (q != NULL) {
			*addr = count_lines(text, q);
		}
		free(pat);
#endif							/* BB_FEATURE_VI_SEARCH */
	} else if (*p == '$') {	// the last line in file
		p++;
		q = begin_line(end - 1);
		*addr = count_lines(text, q);
	} else if (isdigit(*p)) {	// specific line number
		sscanf((char *) p, "%d%n", addr, &st);
		p += st;
	} else {			// I don't reconise this
		// unrecognised address- assume -1
		*addr = -1;
	}
	return (p);
}

static Byte *get_address(Byte *p, int *b, int *e)	// get two colon addrs, if present
{
	//----- get the address' i.e., 1,3   'a,'b  -----
	// get FIRST addr, if present
	while (isblnk(*p))
		p++;				// skip over leading spaces
	if (*p == '%') {			// alias for 1,$
		p++;
		*b = 1;
		*e = count_lines(text, end-1);
		goto ga0;
	}
	p = get_one_address(p, b);
	while (isblnk(*p))
		p++;
	if (*p == ',') {			// is there a address seperator
		p++;
		while (isblnk(*p))
			p++;
		// get SECOND addr, if present
		p = get_one_address(p, e);
	}
ga0:
	while (isblnk(*p))
		p++;				// skip over trailing spaces
	return (p);
}

static void colon(Byte * buf)
{
	Byte c, *orig_buf, *buf1, *q, *r;
	Byte *fn, cmd[BUFSIZ], args[BUFSIZ];
	int i, l, li, ch, st, b, e;
	int useforce, forced;
	struct stat st_buf;

	// :3154	// if (-e line 3154) goto it  else stay put
	// :4,33w! foo	// write a portion of buffer to file "foo"
	// :w		// write all of buffer to current file
	// :q		// quit
	// :q!		// quit- dont care about modified file
	// :'a,'z!sort -u   // filter block through sort
	// :'f		// goto mark "f"
	// :'fl		// list literal the mark "f" line
	// :.r bar	// read file "bar" into buffer before dot
	// :/123/,/abc/d    // delete lines from "123" line to "abc" line
	// :/xyz/	// goto the "xyz" line
	// :s/find/replace/ // substitute pattern "find" with "replace"
	// :!<cmd>	// run <cmd> then return
	//
	if (strlen((char *) buf) <= 0)
		goto vc1;
	if (*buf == ':')
		buf++;			// move past the ':'

	forced = useforce = FALSE;
	li = st = ch = i = 0;
	b = e = -1;
	q = text;			// assume 1,$ for the range
	r = end - 1;
	li = count_lines(text, end - 1);
	fn = cfn;			// default to current file
	memset(cmd, '\0', BUFSIZ);	// clear cmd[]
	memset(args, '\0', BUFSIZ);	// clear args[]

	// look for optional address(es)  :.  :1  :1,9   :'q,'a   :%
	buf = get_address(buf, &b, &e);

	// remember orig command line
	orig_buf = buf;

	// get the COMMAND into cmd[]
	buf1 = cmd;
	while (*buf != '\0') {
		if (isspace(*buf))
			break;
		*buf1++ = *buf++;
	}
	// get any ARGuments
	while (isblnk(*buf))
		buf++;
	strcpy((char *) args, (char *) buf);
	buf1 = last_char_is((char *)cmd, '!');
	if (buf1) {
		useforce = TRUE;
		*buf1 = '\0';   // get rid of !
	}
	if (b >= 0) {
		// if there is only one addr, then the addr
		// is the line number of the single line the
		// user wants. So, reset the end
		// pointer to point at end of the "b" line
		q = find_line(b);	// what line is #b
		r = end_line(q);
		li = 1;
	}
	if (e >= 0) {
		// we were given two addrs.  change the
		// end pointer to the addr given by user.
		r = find_line(e);	// what line is #e
		r = end_line(r);
		li = e - b + 1;
	}
	// ------------ now look for the command ------------
	i = strlen((char *) cmd);
	if (i == 0) {		// :123CR goto line #123
		if (b >= 0) {
			dot = find_line(b);	// what line is #b
			dot_skip_over_ws();
		}
	} else if (strncmp((char *) cmd, "!", 1) == 0) {	// run a cmd
		// :!ls   run the <cmd>
		(void) alarm(0);		// wait for input- no alarms
		place_cursor(rows - 1, 0, FALSE);	// go to Status line
		clear_to_eol();			// clear the line
		cookmode();
		system(orig_buf+1);		// run the cmd
		rawmode();
		Hit_Return();			// let user see results
		(void) alarm(3);		// done waiting for input
	} else if (strncmp((char *) cmd, "=", i) == 0) {	// where is the address
		if (b < 0) {	// no addr given- use defaults
			b = e = count_lines(text, dot);
		}
		psb("%d", b);
	} else if (strncasecmp((char *) cmd, "delete", i) == 0) {	// delete lines
		if (b < 0) {	// no addr given- use defaults
			q = begin_line(dot);	// assume .,. for the range
			r = end_line(dot);
		}
		dot = yank_delete(q, r, 1, YANKDEL);	// save, then delete lines
		dot_skip_over_ws();
	} else if (strncasecmp((char *) cmd, "edit", i) == 0) {	// Edit a file
		int sr;
		sr= 0;
		// don't edit, if the current file has been modified
		if (file_modified == TRUE && useforce != TRUE) {
			psbs("No write since last change (:edit! overrides)");
			goto vc1;
		}
		if (strlen(args) > 0) {
			// the user supplied a file name
			fn= args;
		} else if (cfn != 0 && strlen(cfn) > 0) {
			// no user supplied name- use the current filename
			fn= cfn;
			goto vc5;
		} else {
			// no user file name, no current name- punt
			psbs("No current filename");
			goto vc1;
		}

		// see if file exists- if not, its just a new file request
		if ((sr=stat((char*)fn, &st_buf)) < 0) {
			// This is just a request for a new file creation.
			// The file_insert below will fail but we get
			// an empty buffer with a file name.  Then the "write"
			// command can do the create.
		} else {
			if ((st_buf.st_mode & (S_IFREG)) == 0) {
				// This is not a regular file
				psbs("\"%s\" is not a regular file", fn);
				goto vc1;
			}
			if ((st_buf.st_mode & (S_IRUSR | S_IRGRP | S_IROTH)) == 0) {
				// dont have any read permissions
				psbs("\"%s\" is not readable", fn);
				goto vc1;
			}
		}

		// There is a read-able regular file
		// make this the current file
		q = (Byte *) strdup((char *) fn);	// save the cfn
		if (cfn != 0)
			free(cfn);		// free the old name
		cfn = q;			// remember new cfn

	  vc5:
		// delete all the contents of text[]
		new_text(2 * file_size(fn));
		screenbegin = dot = end = text;

		// insert new file
		ch = file_insert(fn, text, file_size(fn));

		if (ch < 1) {
			// start empty buf with dummy line
			(void) char_insert(text, '\n');
			ch= 1;
		}
		file_modified = FALSE;
#ifdef BB_FEATURE_VI_YANKMARK
		if (Ureg >= 0 && Ureg < 28 && reg[Ureg] != 0) {
			free(reg[Ureg]);	//   free orig line reg- for 'U'
			reg[Ureg]= 0;
		}
		if (YDreg >= 0 && YDreg < 28 && reg[YDreg] != 0) {
			free(reg[YDreg]);	//   free default yank/delete register
			reg[YDreg]= 0;
		}
		for (li = 0; li < 28; li++) {
			mark[li] = 0;
		}				// init the marks
#endif							/* BB_FEATURE_VI_YANKMARK */
		// how many lines in text[]?
		li = count_lines(text, end - 1);
		psb("\"%s\"%s"
#ifdef BB_FEATURE_VI_READONLY
			"%s"
#endif							/* BB_FEATURE_VI_READONLY */
			" %dL, %dC", cfn,
			(sr < 0 ? " [New file]" : ""),
#ifdef BB_FEATURE_VI_READONLY
			((vi_readonly == TRUE || readonly == TRUE) ? " [Read only]" : ""),
#endif							/* BB_FEATURE_VI_READONLY */
			li, ch);
	} else if (strncasecmp((char *) cmd, "file", i) == 0) {	// what File is this
		if (b != -1 || e != -1) {
			ni((Byte *) "No address allowed on this command");
			goto vc1;
		}
		if (strlen((char *) args) > 0) {
			// user wants a new filename
			if (cfn != NULL)
				free(cfn);
			cfn = (Byte *) strdup((char *) args);
		} else {
			// user wants file status info
			edit_status();
		}
	} else if (strncasecmp((char *) cmd, "features", i) == 0) {	// what features are available
		// print out values of all features
		place_cursor(rows - 1, 0, FALSE);	// go to Status line, bottom of screen
		clear_to_eol();	// clear the line
		cookmode();
		show_help();
		rawmode();
		Hit_Return();
	} else if (strncasecmp((char *) cmd, "list", i) == 0) {	// literal print line
		if (b < 0) {	// no addr given- use defaults
			q = begin_line(dot);	// assume .,. for the range
			r = end_line(dot);
		}
		place_cursor(rows - 1, 0, FALSE);	// go to Status line, bottom of screen
		clear_to_eol();	// clear the line
		write(1, "\r\n", 2);
		for (; q <= r; q++) {
			c = *q;
			if (c > '~')
				standout_start();
			if (c == '\n') {
				write(1, "$\r", 2);
			} else if (*q < ' ') {
				write(1, "^", 1);
				c += '@';
			}
			write(1, &c, 1);
			if (c > '~')
				standout_end();
		}
#ifdef BB_FEATURE_VI_SET
	  vc2:
#endif							/* BB_FEATURE_VI_SET */
		Hit_Return();
	} else if ((strncasecmp((char *) cmd, "quit", i) == 0) ||	// Quit
			   (strncasecmp((char *) cmd, "next", i) == 0)) {	// edit next file
		if (useforce == TRUE) {
			// force end of argv list
			if (*cmd == 'q') {
				optind = save_argc;
			}
			editing = 0;
			goto vc1;
		}
		// don't exit if the file been modified
		if (file_modified == TRUE) {
			psbs("No write since last change (:%s! overrides)",
				 (*cmd == 'q' ? "quit" : "next"));
			goto vc1;
		}
		// are there other file to edit
		if (*cmd == 'q' && optind < save_argc - 1) {
			psbs("%d more file to edit", (save_argc - optind - 1));
			goto vc1;
		}
		if (*cmd == 'n' && optind >= save_argc - 1) {
			psbs("No more files to edit");
			goto vc1;
		}
		editing = 0;
	} else if (strncasecmp((char *) cmd, "read", i) == 0) {	// read file into text[]
		fn = args;
		if (strlen((char *) fn) <= 0) {
			psbs("No filename given");
			goto vc1;
		}
		if (b < 0) {	// no addr given- use defaults
			q = begin_line(dot);	// assume "dot"
		}
		// read after current line- unless user said ":0r foo"
		if (b != 0)
			q = next_line(q);
#ifdef BB_FEATURE_VI_READONLY
		l= readonly;			// remember current files' status
#endif
		ch = file_insert(fn, q, file_size(fn));
#ifdef BB_FEATURE_VI_READONLY
		readonly= l;
#endif
		if (ch < 0)
			goto vc1;	// nothing was inserted
		// how many lines in text[]?
		li = count_lines(q, q + ch - 1);
		psb("\"%s\""
#ifdef BB_FEATURE_VI_READONLY
			"%s"
#endif							/* BB_FEATURE_VI_READONLY */
			" %dL, %dC", fn,
#ifdef BB_FEATURE_VI_READONLY
			((vi_readonly == TRUE || readonly == TRUE) ? " [Read only]" : ""),
#endif							/* BB_FEATURE_VI_READONLY */
			li, ch);
		if (ch > 0) {
			// if the insert is before "dot" then we need to update
			if (q <= dot)
				dot += ch;
			file_modified = TRUE;
		}
	} else if (strncasecmp((char *) cmd, "rewind", i) == 0) {	// rewind cmd line args
		if (file_modified == TRUE && useforce != TRUE) {
			psbs("No write since last change (:rewind! overrides)");
		} else {
			// reset the filenames to edit
			optind = fn_start - 1;
			editing = 0;
		}
#ifdef BB_FEATURE_VI_SET
	} else if (strncasecmp((char *) cmd, "set", i) == 0) {	// set or clear features
		i = 0;			// offset into args
		if (strlen((char *) args) == 0) {
			// print out values of all options
			place_cursor(rows - 1, 0, FALSE);	// go to Status line, bottom of screen
			clear_to_eol();	// clear the line
			printf("----------------------------------------\r\n");
#ifdef BB_FEATURE_VI_SETOPTS
			if (!autoindent)
				printf("no");
			printf("autoindent ");
			if (!err_method)
				printf("no");
			printf("flash ");
			if (!ignorecase)
				printf("no");
			printf("ignorecase ");
			if (!showmatch)
				printf("no");
			printf("showmatch ");
			printf("tabstop=%d ", tabstop);
#endif							/* BB_FEATURE_VI_SETOPTS */
			printf("\r\n");
			goto vc2;
		}
		if (strncasecmp((char *) args, "no", 2) == 0)
			i = 2;		// ":set noautoindent"
#ifdef BB_FEATURE_VI_SETOPTS
		if (strncasecmp((char *) args + i, "autoindent", 10) == 0 ||
			strncasecmp((char *) args + i, "ai", 2) == 0) {
			autoindent = (i == 2) ? 0 : 1;
		}
		if (strncasecmp((char *) args + i, "flash", 5) == 0 ||
			strncasecmp((char *) args + i, "fl", 2) == 0) {
			err_method = (i == 2) ? 0 : 1;
		}
		if (strncasecmp((char *) args + i, "ignorecase", 10) == 0 ||
			strncasecmp((char *) args + i, "ic", 2) == 0) {
			ignorecase = (i == 2) ? 0 : 1;
		}
		if (strncasecmp((char *) args + i, "showmatch", 9) == 0 ||
			strncasecmp((char *) args + i, "sm", 2) == 0) {
			showmatch = (i == 2) ? 0 : 1;
		}
		if (strncasecmp((char *) args + i, "tabstop", 7) == 0) {
			sscanf(strchr((char *) args + i, '='), "=%d", &ch);
			if (ch > 0 && ch < columns - 1)
				tabstop = ch;
		}
#endif							/* BB_FEATURE_VI_SETOPTS */
#endif							/* BB_FEATURE_VI_SET */
#ifdef BB_FEATURE_VI_SEARCH
	} else if (strncasecmp((char *) cmd, "s", 1) == 0) {	// substitute a pattern with a replacement pattern
		Byte *ls, *F, *R;
		int gflag;

		// F points to the "find" pattern
		// R points to the "replace" pattern
		// replace the cmd line delimiters "/" with NULLs
		gflag = 0;		// global replace flag
		c = orig_buf[1];	// what is the delimiter
		F = orig_buf + 2;	// start of "find"
		R = (Byte *) strchr((char *) F, c);	// middle delimiter
		if (!R) goto colon_s_fail;
		*R++ = '\0';	// terminate "find"
		buf1 = (Byte *) strchr((char *) R, c);
		if (!buf1) goto colon_s_fail;
		*buf1++ = '\0';	// terminate "replace"
		if (*buf1 == 'g') {	// :s/foo/bar/g
			buf1++;
			gflag++;	// turn on gflag
		}
		q = begin_line(q);
		if (b < 0) {	// maybe :s/foo/bar/
			q = begin_line(dot);	// start with cur line
			b = count_lines(text, q);	// cur line number
		}
		if (e < 0)
			e = b;		// maybe :.s/foo/bar/
		for (i = b; i <= e; i++) {	// so, :20,23 s \0 find \0 replace \0
			ls = q;		// orig line start
		  vc4:
			buf1 = char_search(q, F, FORWARD, LIMITED);	// search cur line only for "find"
			if (buf1 != NULL) {
				// we found the "find" pattern- delete it
				(void) text_hole_delete(buf1, buf1 + strlen((char *) F) - 1);
				// inset the "replace" patern
				(void) string_insert(buf1, R);	// insert the string
				// check for "global"  :s/foo/bar/g
				if (gflag == 1) {
					if ((buf1 + strlen((char *) R)) < end_line(ls)) {
						q = buf1 + strlen((char *) R);
						goto vc4;	// don't let q move past cur line
					}
				}
			}
			q = next_line(ls);
		}
#endif							/* BB_FEATURE_VI_SEARCH */
	} else if (strncasecmp((char *) cmd, "version", i) == 0) {	// show software version
		psb("%s", vi_Version);
	} else if ((strncasecmp((char *) cmd, "write", i) == 0) ||	// write text to file
			   (strncasecmp((char *) cmd, "wq", i) == 0)) {	// write text to file
		// is there a file name to write to?
		if (strlen((char *) args) > 0) {
			fn = args;
		}
#ifdef BB_FEATURE_VI_READONLY
		if ((vi_readonly == TRUE || readonly == TRUE) && useforce == FALSE) {
			psbs("\"%s\" File is read only", fn);
			goto vc3;
		}
#endif							/* BB_FEATURE_VI_READONLY */
		// how many lines in text[]?
		li = count_lines(q, r);
		ch = r - q + 1;
		// see if file exists- if not, its just a new file request
		if (useforce == TRUE) {
			// if "fn" is not write-able, chmod u+w
			// sprintf(syscmd, "chmod u+w %s", fn);
			// system(syscmd);
			forced = TRUE;
		}
		l = file_write(fn, q, r);
		if (useforce == TRUE && forced == TRUE) {
			// chmod u-w
			// sprintf(syscmd, "chmod u-w %s", fn);
			// system(syscmd);
			forced = FALSE;
		}
		psb("\"%s\" %dL, %dC", fn, li, l);
		if (q == text && r == end - 1 && l == ch)
			file_modified = FALSE;
		if (cmd[1] == 'q' && l == ch) {
			editing = 0;
		}
#ifdef BB_FEATURE_VI_READONLY
	  vc3:;
#endif							/* BB_FEATURE_VI_READONLY */
#ifdef BB_FEATURE_VI_YANKMARK
	} else if (strncasecmp((char *) cmd, "yank", i) == 0) {	// yank lines
		if (b < 0) {	// no addr given- use defaults
			q = begin_line(dot);	// assume .,. for the range
			r = end_line(dot);
		}
		text_yank(q, r, YDreg);
		li = count_lines(q, r);
		psb("Yank %d lines (%d chars) into [%c]",
			li, strlen((char *) reg[YDreg]), what_reg());
#endif							/* BB_FEATURE_VI_YANKMARK */
	} else {
		// cmd unknown
		ni((Byte *) cmd);
	}
  vc1:
	dot = bound_dot(dot);	// make sure "dot" is valid
	return;
#ifdef BB_FEATURE_VI_SEARCH
colon_s_fail:
	psb(":s expression missing delimiters");
	return;
#endif

}

static void Hit_Return(void)
{
	char c;

	standout_start();	// start reverse video
	write(1, "[Hit return to continue]", 24);
	standout_end();		// end reverse video
	while ((c = get_one_char()) != '\n' && c != '\r')	/*do nothing */
		;
	redraw(TRUE);		// force redraw all
}
#endif							/* BB_FEATURE_VI_COLON */

//----- Synchronize the cursor to Dot --------------------------
static void sync_cursor(Byte * d, int *row, int *col)
{
	Byte *beg_cur, *end_cur;	// begin and end of "d" line
	Byte *beg_scr, *end_scr;	// begin and end of screen
	Byte *tp;
	int cnt, ro, co;

	beg_cur = begin_line(d);	// first char of cur line
	end_cur = end_line(d);	// last char of cur line

	beg_scr = end_scr = screenbegin;	// first char of screen
	end_scr = end_screen();	// last char of screen

	if (beg_cur < screenbegin) {
		// "d" is before  top line on screen
		// how many lines do we have to move
		cnt = count_lines(beg_cur, screenbegin);
	  sc1:
		screenbegin = beg_cur;
		if (cnt > (rows - 1) / 2) {
			// we moved too many lines. put "dot" in middle of screen
			for (cnt = 0; cnt < (rows - 1) / 2; cnt++) {
				screenbegin = prev_line(screenbegin);
			}
		}
	} else if (beg_cur > end_scr) {
		// "d" is after bottom line on screen
		// how many lines do we have to move
		cnt = count_lines(end_scr, beg_cur);
		if (cnt > (rows - 1) / 2)
			goto sc1;	// too many lines
		for (ro = 0; ro < cnt - 1; ro++) {
			// move screen begin the same amount
			screenbegin = next_line(screenbegin);
			// now, move the end of screen
			end_scr = next_line(end_scr);
			end_scr = end_line(end_scr);
		}
	}
	// "d" is on screen- find out which row
	tp = screenbegin;
	for (ro = 0; ro < rows - 1; ro++) {	// drive "ro" to correct row
		if (tp == beg_cur)
			break;
		tp = next_line(tp);
	}

	// find out what col "d" is on
	co = 0;
	do {				// drive "co" to correct column
		if (*tp == '\n' || *tp == '\0')
			break;
		if (*tp == '\t') {
			//         7       - (co %    8  )
			co += ((tabstop - 1) - (co % tabstop));
		} else if (*tp < ' ') {
			co++;		// display as ^X, use 2 columns
		}
	} while (tp++ < d && ++co);

	// "co" is the column where "dot" is.
	// The screen has "columns" columns.
	// The currently displayed columns are  0+offset -- columns+ofset
	// |-------------------------------------------------------------|
	//               ^ ^                                ^
	//        offset | |------- columns ----------------|
	//
	// If "co" is already in this range then we do not have to adjust offset
	//      but, we do have to subtract the "offset" bias from "co".
	// If "co" is outside this range then we have to change "offset".
	// If the first char of a line is a tab the cursor will try to stay
	//  in column 7, but we have to set offset to 0.

	if (co < 0 + offset) {
		offset = co;
	}
	if (co >= columns + offset) {
		offset = co - columns + 1;
	}
	// if the first char of the line is a tab, and "dot" is sitting on it
	//  force offset to 0.
	if (d == beg_cur && *d == '\t') {
		offset = 0;
	}
	co -= offset;

	*row = ro;
	*col = co;
}

//----- Text Movement Routines ---------------------------------
static Byte *begin_line(Byte * p) // return pointer to first char cur line
{
	while (p > text && p[-1] != '\n')
		p--;			// go to cur line B-o-l
	return (p);
}

static Byte *end_line(Byte * p) // return pointer to NL of cur line line
{
	while (p < end - 1 && *p != '\n')
		p++;			// go to cur line E-o-l
	return (p);
}

static Byte *dollar_line(Byte * p) // return pointer to just before NL line
{
	while (p < end - 1 && *p != '\n')
		p++;			// go to cur line E-o-l
	// Try to stay off of the Newline
	if (*p == '\n' && (p - begin_line(p)) > 0)
		p--;
	return (p);
}

static Byte *prev_line(Byte * p) // return pointer first char prev line
{
	p = begin_line(p);	// goto begining of cur line
	if (p[-1] == '\n' && p > text)
		p--;			// step to prev line
	p = begin_line(p);	// goto begining of prev line
	return (p);
}

static Byte *next_line(Byte * p) // return pointer first char next line
{
	p = end_line(p);
	if (*p == '\n' && p < end - 1)
		p++;			// step to next line
	return (p);
}

//----- Text Information Routines ------------------------------
static Byte *end_screen(void)
{
	Byte *q;
	int cnt;

	// find new bottom line
	q = screenbegin;
	for (cnt = 0; cnt < rows - 2; cnt++)
		q = next_line(q);
	q = end_line(q);
	return (q);
}

static int count_lines(Byte * start, Byte * stop) // count line from start to stop
{
	Byte *q;
	int cnt;

	if (stop < start) {	// start and stop are backwards- reverse them
		q = start;
		start = stop;
		stop = q;
	}
	cnt = 0;
	stop = end_line(stop);	// get to end of this line
	for (q = start; q <= stop && q <= end - 1; q++) {
		if (*q == '\n')
			cnt++;
	}
	return (cnt);
}

static Byte *find_line(int li)	// find begining of line #li
{
	Byte *q;

	for (q = text; li > 1; li--) {
		q = next_line(q);
	}
	return (q);
}

//----- Dot Movement Routines ----------------------------------
static void dot_left(void)
{
	if (dot > text && dot[-1] != '\n')
		dot--;
}

static void dot_right(void)
{
	if (dot < end - 1 && *dot != '\n')
		dot++;
}

static void dot_begin(void)
{
	dot = begin_line(dot);	// return pointer to first char cur line
}

static void dot_end(void)
{
	dot = end_line(dot);	// return pointer to last char cur line
}

static Byte *move_to_col(Byte * p, int l)
{
	int co;

	p = begin_line(p);
	co = 0;
	do {
		if (*p == '\n' || *p == '\0')
			break;
		if (*p == '\t') {
			//         7       - (co %    8  )
			co += ((tabstop - 1) - (co % tabstop));
		} else if (*p < ' ') {
			co++;		// display as ^X, use 2 columns
		}
	} while (++co <= l && p++ < end);
	return (p);
}

static void dot_next(void)
{
	dot = next_line(dot);
}

static void dot_prev(void)
{
	dot = prev_line(dot);
}

static void dot_scroll(int cnt, int dir)
{
	Byte *q;

	for (; cnt > 0; cnt--) {
		if (dir < 0) {
			// scroll Backwards
			// ctrl-Y  scroll up one line
			screenbegin = prev_line(screenbegin);
		} else {
			// scroll Forwards
			// ctrl-E  scroll down one line
			screenbegin = next_line(screenbegin);
		}
	}
	// make sure "dot" stays on the screen so we dont scroll off
	if (dot < screenbegin)
		dot = screenbegin;
	q = end_screen();	// find new bottom line
	if (dot > q)
		dot = begin_line(q);	// is dot is below bottom line?
	dot_skip_over_ws();
}

static void dot_skip_over_ws(void)
{
	// skip WS
	while (isspace(*dot) && *dot != '\n' && dot < end - 1)
		dot++;
}

static void dot_delete(void)	// delete the char at 'dot'
{
	(void) text_hole_delete(dot, dot);
}

static Byte *bound_dot(Byte * p) // make sure  text[0] <= P < "end"
{
	if (p >= end && end > text) {
		p = end - 1;
		indicate_error('1');
	}
	if (p < text) {
		p = text;
		indicate_error('2');
	}
	return (p);
}

//----- Helper Utility Routines --------------------------------

//----------------------------------------------------------------
//----- Char Routines --------------------------------------------
/* Chars that are part of a word-
 *    0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz
 * Chars that are Not part of a word (stoppers)
 *    !"#$%&'()*+,-./:;<=>?@[\]^`{|}~
 * Chars that are WhiteSpace
 *    TAB NEWLINE VT FF RETURN SPACE
 * DO NOT COUNT NEWLINE AS WHITESPACE
 */

static Byte *new_screen(int ro, int co)
{
	int li;

	if (screen != 0)
		free(screen);
	screensize = ro * co + 8;
	screen = (Byte *) xmalloc(screensize);
	// initialize the new screen. assume this will be a empty file.
	screen_erase();
	//   non-existant text[] lines start with a tilde (~).
	for (li = 1; li < ro - 1; li++) {
		screen[(li * co) + 0] = '~';
	}
	return (screen);
}

static Byte *new_text(int size)
{
	if (size < 10240)
		size = 10240;	// have a minimum size for new files
	if (text != 0) {
		//text -= 4;
		free(text);
	}
	text = (Byte *) xmalloc(size + 8);
	memset(text, '\0', size);	// clear new text[]
	//text += 4;		// leave some room for "oops"
	textend = text + size - 1;
	//textend -= 4;		// leave some root for "oops"
	return (text);
}

#ifdef BB_FEATURE_VI_SEARCH
static int mycmp(Byte * s1, Byte * s2, int len)
{
	int i;

	i = strncmp((char *) s1, (char *) s2, len);
#ifdef BB_FEATURE_VI_SETOPTS
	if (ignorecase) {
		i = strncasecmp((char *) s1, (char *) s2, len);
	}
#endif							/* BB_FEATURE_VI_SETOPTS */
	return (i);
}

static Byte *char_search(Byte * p, Byte * pat, int dir, int range)	// search for pattern starting at p
{
#ifndef REGEX_SEARCH
	Byte *start, *stop;
	int len;

	len = strlen((char *) pat);
	if (dir == FORWARD) {
		stop = end - 1;	// assume range is p - end-1
		if (range == LIMITED)
			stop = next_line(p);	// range is to next line
		for (start = p; start < stop; start++) {
			if (mycmp(start, pat, len) == 0) {
				return (start);
			}
		}
	} else if (dir == BACK) {
		stop = text;	// assume range is text - p
		if (range == LIMITED)
			stop = prev_line(p);	// range is to prev line
		for (start = p - len; start >= stop; start--) {
			if (mycmp(start, pat, len) == 0) {
				return (start);
			}
		}
	}
	// pattern not found
	return (NULL);
#else							/*REGEX_SEARCH */
	char *q;
	struct re_pattern_buffer preg;
	int i;
	int size, range;

	re_syntax_options = RE_SYNTAX_POSIX_EXTENDED;
	preg.translate = 0;
	preg.fastmap = 0;
	preg.buffer = 0;
	preg.allocated = 0;

	// assume a LIMITED forward search
	q = next_line(p);
	q = end_line(q);
	q = end - 1;
	if (dir == BACK) {
		q = prev_line(p);
		q = text;
	}
	// count the number of chars to search over, forward or backward
	size = q - p;
	if (size < 0)
		size = p - q;
	// RANGE could be negative if we are searching backwards
	range = q - p;

	q = (char *) re_compile_pattern(pat, strlen((char *) pat), &preg);
	if (q != 0) {
		// The pattern was not compiled
		psbs("bad search pattern: \"%s\": %s", pat, q);
		i = 0;			// return p if pattern not compiled
		goto cs1;
	}

	q = p;
	if (range < 0) {
		q = p - size;
		if (q < text)
			q = text;
	}
	// search for the compiled pattern, preg, in p[]
	// range < 0-  search backward
	// range > 0-  search forward
	// 0 < start < size
	// re_search() < 0  not found or error
	// re_search() > 0  index of found pattern
	//            struct pattern    char     int    int    int     struct reg
	// re_search (*pattern_buffer,  *string, size,  start, range,  *regs)
	i = re_search(&preg, q, size, 0, range, 0);
	if (i == -1) {
		p = 0;
		i = 0;			// return NULL if pattern not found
	}
  cs1:
	if (dir == FORWARD) {
		p = p + i;
	} else {
		p = p - i;
	}
	return (p);
#endif							/*REGEX_SEARCH */
}
#endif							/* BB_FEATURE_VI_SEARCH */

static Byte *char_insert(Byte * p, Byte c) // insert the char c at 'p'
{
	if (c == 22) {		// Is this an ctrl-V?
		p = stupid_insert(p, '^');	// use ^ to indicate literal next
		p--;			// backup onto ^
		refresh(FALSE);	// show the ^
		c = get_one_char();
		*p = c;
		p++;
		file_modified = TRUE;	// has the file been modified
	} else if (c == 27) {	// Is this an ESC?
		cmd_mode = 0;
		cmdcnt = 0;
		end_cmd_q();	// stop adding to q
		strcpy((char *) status_buffer, " ");	// clear the status buffer
		if ((p[-1] != '\n') && (dot>text)) {
			p--;
		}
	} else if (c == erase_char) {	// Is this a BS
		//     123456789
		if ((p[-1] != '\n') && (dot>text)) {
			p--;
			p = text_hole_delete(p, p);	// shrink buffer 1 char
#ifdef BB_FEATURE_VI_DOT_CMD
			// also rmove char from last_modifying_cmd
			if (strlen((char *) last_modifying_cmd) > 0) {
				Byte *q;

				q = last_modifying_cmd;
				q[strlen((char *) q) - 1] = '\0';	// erase BS
				q[strlen((char *) q) - 1] = '\0';	// erase prev char
			}
#endif							/* BB_FEATURE_VI_DOT_CMD */
		}
	} else {
		// insert a char into text[]
		Byte *sp;		// "save p"

		if (c == 13)
			c = '\n';	// translate \r to \n
		sp = p;			// remember addr of insert
		p = stupid_insert(p, c);	// insert the char
#ifdef BB_FEATURE_VI_SETOPTS
		if (showmatch && strchr(")]}", *sp) != NULL) {
			showmatching(sp);
		}
		if (autoindent && c == '\n') {	// auto indent the new line
			Byte *q;

			q = prev_line(p);	// use prev line as templet
			for (; isblnk(*q); q++) {
				p = stupid_insert(p, *q);	// insert the char
			}
		}
#endif							/* BB_FEATURE_VI_SETOPTS */
	}
	return (p);
}

static Byte *stupid_insert(Byte * p, Byte c) // stupidly insert the char c at 'p'
{
	p = text_hole_make(p, 1);
	if (p != 0) {
		*p = c;
		file_modified = TRUE;	// has the file been modified
		p++;
	}
	return (p);
}

static Byte find_range(Byte ** start, Byte ** stop, Byte c)
{
	Byte *save_dot, *p, *q;
	int cnt;

	save_dot = dot;
	p = q = dot;

	if (strchr("cdy><", c)) {
		// these cmds operate on whole lines
		p = q = begin_line(p);
		for (cnt = 1; cnt < cmdcnt; cnt++) {
			q = next_line(q);
		}
		q = end_line(q);
	} else if (strchr("^%$0bBeEft", c)) {
		// These cmds operate on char positions
		do_cmd(c);		// execute movement cmd
		q = dot;
	} else if (strchr("wW", c)) {
		do_cmd(c);		// execute movement cmd
		if (dot > text)
			dot--;		// move back off of next word
		if (dot > text && *dot == '\n')
			dot--;		// stay off NL
		q = dot;
	} else if (strchr("H-k{", c)) {
		// these operate on multi-lines backwards
		q = end_line(dot);	// find NL
		do_cmd(c);		// execute movement cmd
		dot_begin();
		p = dot;
	} else if (strchr("L+j}\r\n", c)) {
		// these operate on multi-lines forwards
		p = begin_line(dot);
		do_cmd(c);		// execute movement cmd
		dot_end();		// find NL
		q = dot;
	} else {
		c = 27;			// error- return an ESC char
		//break;
	}
	*start = p;
	*stop = q;
	if (q < p) {
		*start = q;
		*stop = p;
	}
	dot = save_dot;
	return (c);
}

static int st_test(Byte * p, int type, int dir, Byte * tested)
{
	Byte c, c0, ci;
	int test, inc;

	inc = dir;
	c = c0 = p[0];
	ci = p[inc];
	test = 0;

	if (type == S_BEFORE_WS) {
		c = ci;
		test = ((!isspace(c)) || c == '\n');
	}
	if (type == S_TO_WS) {
		c = c0;
		test = ((!isspace(c)) || c == '\n');
	}
	if (type == S_OVER_WS) {
		c = c0;
		test = ((isspace(c)));
	}
	if (type == S_END_PUNCT) {
		c = ci;
		test = ((ispunct(c)));
	}
	if (type == S_END_ALNUM) {
		c = ci;
		test = ((isalnum(c)) || c == '_');
	}
	*tested = c;
	return (test);
}

static Byte *skip_thing(Byte * p, int linecnt, int dir, int type)
{
	Byte c;

	while (st_test(p, type, dir, &c)) {
		// make sure we limit search to correct number of lines
		if (c == '\n' && --linecnt < 1)
			break;
		if (dir >= 0 && p >= end - 1)
			break;
		if (dir < 0 && p <= text)
			break;
		p += dir;		// move to next char
	}
	return (p);
}

// find matching char of pair  ()  []  {}
static Byte *find_pair(Byte * p, Byte c)
{
	Byte match, *q;
	int dir, level;

	match = ')';
	level = 1;
	dir = 1;			// assume forward
	switch (c) {
	case '(':
		match = ')';
		break;
	case '[':
		match = ']';
		break;
	case '{':
		match = '}';
		break;
	case ')':
		match = '(';
		dir = -1;
		break;
	case ']':
		match = '[';
		dir = -1;
		break;
	case '}':
		match = '{';
		dir = -1;
		break;
	}
	for (q = p + dir; text <= q && q < end; q += dir) {
		// look for match, count levels of pairs  (( ))
		if (*q == c)
			level++;	// increase pair levels
		if (*q == match)
			level--;	// reduce pair level
		if (level == 0)
			break;		// found matching pair
	}
	if (level != 0)
		q = NULL;		// indicate no match
	return (q);
}

#ifdef BB_FEATURE_VI_SETOPTS
// show the matching char of a pair,  ()  []  {}
static void showmatching(Byte * p)
{
	Byte *q, *save_dot;

	// we found half of a pair
	q = find_pair(p, *p);	// get loc of matching char
	if (q == NULL) {
		indicate_error('3');	// no matching char
	} else {
		// "q" now points to matching pair
		save_dot = dot;	// remember where we are
		dot = q;		// go to new loc
		refresh(FALSE);	// let the user see it
		(void) mysleep(40);	// give user some time
		dot = save_dot;	// go back to old loc
		refresh(FALSE);
	}
}
#endif							/* BB_FEATURE_VI_SETOPTS */

//  open a hole in text[]
static Byte *text_hole_make(Byte * p, int size)	// at "p", make a 'size' byte hole
{
	Byte *src, *dest;
	int cnt;

	if (size <= 0)
		goto thm0;
	src = p;
	dest = p + size;
	cnt = end - src;	// the rest of buffer
	if (memmove(dest, src, cnt) != dest) {
		psbs("can't create room for new characters");
	}
	memset(p, ' ', size);	// clear new hole
	end = end + size;	// adjust the new END
	file_modified = TRUE;	// has the file been modified
  thm0:
	return (p);
}

//  close a hole in text[]
static Byte *text_hole_delete(Byte * p, Byte * q) // delete "p" thru "q", inclusive
{
	Byte *src, *dest;
	int cnt, hole_size;

	// move forwards, from beginning
	// assume p <= q
	src = q + 1;
	dest = p;
	if (q < p) {		// they are backward- swap them
		src = p + 1;
		dest = q;
	}
	hole_size = q - p + 1;
	cnt = end - src;
	if (src < text || src > end)
		goto thd0;
	if (dest < text || dest >= end)
		goto thd0;
	if (src >= end)
		goto thd_atend;	// just delete the end of the buffer
	if (memmove(dest, src, cnt) != dest) {
		psbs("can't delete the character");
	}
  thd_atend:
	end = end - hole_size;	// adjust the new END
	if (dest >= end)
		dest = end - 1;	// make sure dest in below end-1
	if (end <= text)
		dest = end = text;	// keep pointers valid
	file_modified = TRUE;	// has the file been modified
  thd0:
	return (dest);
}

// copy text into register, then delete text.
// if dist <= 0, do not include, or go past, a NewLine
//
static Byte *yank_delete(Byte * start, Byte * stop, int dist, int yf)
{
	Byte *p;

	// make sure start <= stop
	if (start > stop) {
		// they are backwards, reverse them
		p = start;
		start = stop;
		stop = p;
	}
	if (dist <= 0) {
		// we can not cross NL boundaries
		p = start;
		if (*p == '\n')
			return (p);
		// dont go past a NewLine
		for (; p + 1 <= stop; p++) {
			if (p[1] == '\n') {
				stop = p;	// "stop" just before NewLine
				break;
			}
		}
	}
	p = start;
#ifdef BB_FEATURE_VI_YANKMARK
	text_yank(start, stop, YDreg);
#endif							/* BB_FEATURE_VI_YANKMARK */
	if (yf == YANKDEL) {
		p = text_hole_delete(start, stop);
	}					// delete lines
	return (p);
}

static void show_help(void)
{
	puts("These features are available:"
#ifdef BB_FEATURE_VI_SEARCH
	"\n\tPattern searches with / and ?"
#endif							/* BB_FEATURE_VI_SEARCH */
#ifdef BB_FEATURE_VI_DOT_CMD
	"\n\tLast command repeat with \'.\'"
#endif							/* BB_FEATURE_VI_DOT_CMD */
#ifdef BB_FEATURE_VI_YANKMARK
	"\n\tLine marking with  'x"
	"\n\tNamed buffers with  \"x"
#endif							/* BB_FEATURE_VI_YANKMARK */
#ifdef BB_FEATURE_VI_READONLY
	"\n\tReadonly if vi is called as \"view\""
	"\n\tReadonly with -R command line arg"
#endif							/* BB_FEATURE_VI_READONLY */
#ifdef BB_FEATURE_VI_SET
	"\n\tSome colon mode commands with \':\'"
#endif							/* BB_FEATURE_VI_SET */
#ifdef BB_FEATURE_VI_SETOPTS
	"\n\tSettable options with \":set\""
#endif							/* BB_FEATURE_VI_SETOPTS */
#ifdef BB_FEATURE_VI_USE_SIGNALS
	"\n\tSignal catching- ^C"
	"\n\tJob suspend and resume with ^Z"
#endif							/* BB_FEATURE_VI_USE_SIGNALS */
#ifdef BB_FEATURE_VI_WIN_RESIZE
	"\n\tAdapt to window re-sizes"
#endif							/* BB_FEATURE_VI_WIN_RESIZE */
	);
}

static void print_literal(Byte * buf, Byte * s) // copy s to buf, convert unprintable
{
	Byte c, b[2];

	b[1] = '\0';
	strcpy((char *) buf, "");	// init buf
	if (strlen((char *) s) <= 0)
		s = (Byte *) "(NULL)";
	for (; *s > '\0'; s++) {
		c = *s;
		if (*s > '~') {
			strcat((char *) buf, SOs);
			c = *s - 128;
		}
		if (*s < ' ') {
			strcat((char *) buf, "^");
			c += '@';
		}
		b[0] = c;
		strcat((char *) buf, (char *) b);
		if (*s > '~')
			strcat((char *) buf, SOn);
		if (*s == '\n') {
			strcat((char *) buf, "$");
		}
	}
}

#ifdef BB_FEATURE_VI_DOT_CMD
static void start_new_cmd_q(Byte c)
{
	// release old cmd
	if (last_modifying_cmd != 0)
		free(last_modifying_cmd);
	// get buffer for new cmd
	last_modifying_cmd = (Byte *) xmalloc(BUFSIZ);
	memset(last_modifying_cmd, '\0', BUFSIZ);	// clear new cmd queue
	// if there is a current cmd count put it in the buffer first
	if (cmdcnt > 0)
		sprintf((char *) last_modifying_cmd, "%d", cmdcnt);
	// save char c onto queue
	last_modifying_cmd[strlen((char *) last_modifying_cmd)] = c;
	adding2q = 1;
	return;
}

static void end_cmd_q()
{
#ifdef BB_FEATURE_VI_YANKMARK
	YDreg = 26;			// go back to default Yank/Delete reg
#endif							/* BB_FEATURE_VI_YANKMARK */
	adding2q = 0;
	return;
}
#endif							/* BB_FEATURE_VI_DOT_CMD */

#if defined(BB_FEATURE_VI_YANKMARK) || defined(BB_FEATURE_VI_COLON) || defined(BB_FEATURE_VI_CRASHME)
static Byte *string_insert(Byte * p, Byte * s) // insert the string at 'p'
{
	int cnt, i;

	i = strlen((char *) s);
	p = text_hole_make(p, i);
	strncpy((char *) p, (char *) s, i);
	for (cnt = 0; *s != '\0'; s++) {
		if (*s == '\n')
			cnt++;
	}
#ifdef BB_FEATURE_VI_YANKMARK
	psb("Put %d lines (%d chars) from [%c]", cnt, i, what_reg());
#endif							/* BB_FEATURE_VI_YANKMARK */
	return (p);
}
#endif							/* BB_FEATURE_VI_YANKMARK || BB_FEATURE_VI_COLON || BB_FEATURE_VI_CRASHME */

#ifdef BB_FEATURE_VI_YANKMARK
static Byte *text_yank(Byte * p, Byte * q, int dest)	// copy text into a register
{
	Byte *t;
	int cnt;

	if (q < p) {		// they are backwards- reverse them
		t = q;
		q = p;
		p = t;
	}
	cnt = q - p + 1;
	t = reg[dest];
	if (t != 0) {		// if already a yank register
		free(t);		//   free it
	}
	t = (Byte *) xmalloc(cnt + 1);	// get a new register
	memset(t, '\0', cnt + 1);	// clear new text[]
	strncpy((char *) t, (char *) p, cnt);	// copy text[] into bufer
	reg[dest] = t;
	return (p);
}

static Byte what_reg(void)
{
	Byte c;
	int i;

	i = 0;
	c = 'D';			// default to D-reg
	if (0 <= YDreg && YDreg <= 25)
		c = 'a' + (Byte) YDreg;
	if (YDreg == 26)
		c = 'D';
	if (YDreg == 27)
		c = 'U';
	return (c);
}

static void check_context(Byte cmd)
{
	// A context is defined to be "modifying text"
	// Any modifying command establishes a new context.

	if (dot < context_start || dot > context_end) {
		if (strchr((char *) modifying_cmds, cmd) != NULL) {
			// we are trying to modify text[]- make this the current context
			mark[27] = mark[26];	// move cur to prev
			mark[26] = dot;	// move local to cur
			context_start = prev_line(prev_line(dot));
			context_end = next_line(next_line(dot));
			//loiter= start_loiter= now;
		}
	}
	return;
}

static Byte *swap_context(Byte * p) // goto new context for '' command make this the current context
{
	Byte *tmp;

	// the current context is in mark[26]
	// the previous context is in mark[27]
	// only swap context if other context is valid
	if (text <= mark[27] && mark[27] <= end - 1) {
		tmp = mark[27];
		mark[27] = mark[26];
		mark[26] = tmp;
		p = mark[26];	// where we are going- previous context
		context_start = prev_line(prev_line(prev_line(p)));
		context_end = next_line(next_line(next_line(p)));
	}
	return (p);
}
#endif							/* BB_FEATURE_VI_YANKMARK */

static int isblnk(Byte c) // is the char a blank or tab
{
	return (c == ' ' || c == '\t');
}

//----- Set terminal attributes --------------------------------
static void rawmode(void)
{
	tcgetattr(0, &term_orig);
	term_vi = term_orig;
	term_vi.c_lflag &= (~ICANON & ~ECHO);	// leave ISIG ON- allow intr's
	term_vi.c_iflag &= (~IXON & ~ICRNL);
	term_vi.c_oflag &= (~ONLCR);
#ifndef linux
	term_vi.c_cc[VMIN] = 1;
	term_vi.c_cc[VTIME] = 0;
#endif
	erase_char = term_vi.c_cc[VERASE];
	tcsetattr(0, TCSANOW, &term_vi);
}

static void cookmode(void)
{
	tcsetattr(0, TCSANOW, &term_orig);
}

#ifdef BB_FEATURE_VI_WIN_RESIZE
//----- See what the window size currently is --------------------
static void window_size_get(int sig)
{
	int i;

	i = ioctl(0, TIOCGWINSZ, &winsize);
	if (i != 0) {
		// force 24x80
		winsize.ws_row = 24;
		winsize.ws_col = 80;
	}
	if (winsize.ws_row <= 1) {
		winsize.ws_row = 24;
	}
	if (winsize.ws_col <= 1) {
		winsize.ws_col = 80;
	}
	rows = (int) winsize.ws_row;
	columns = (int) winsize.ws_col;
}
#endif							/* BB_FEATURE_VI_WIN_RESIZE */

//----- Come here when we get a window resize signal ---------
#ifdef BB_FEATURE_VI_USE_SIGNALS
static void winch_sig(int sig)
{
	signal(SIGWINCH, winch_sig);
#ifdef BB_FEATURE_VI_WIN_RESIZE
	window_size_get(0);
#endif							/* BB_FEATURE_VI_WIN_RESIZE */
	new_screen(rows, columns);	// get memory for virtual screen
	redraw(TRUE);		// re-draw the screen
}

//----- Come here when we get a continue signal -------------------
static void cont_sig(int sig)
{
	rawmode();			// terminal to "raw"
	*status_buffer = '\0';	// clear the status buffer
	redraw(TRUE);		// re-draw the screen

	signal(SIGTSTP, suspend_sig);
	signal(SIGCONT, SIG_DFL);
	kill(getpid(), SIGCONT);
}

//----- Come here when we get a Suspend signal -------------------
static void suspend_sig(int sig)
{
	place_cursor(rows - 1, 0, FALSE);	// go to bottom of screen
	clear_to_eol();		// Erase to end of line
	cookmode();			// terminal to "cooked"

	signal(SIGCONT, cont_sig);
	signal(SIGTSTP, SIG_DFL);
	kill(getpid(), SIGTSTP);
}

//----- Come here when we get a signal ---------------------------
static void catch_sig(int sig)
{
	signal(SIGHUP, catch_sig);
	signal(SIGINT, catch_sig);
	signal(SIGTERM, catch_sig);
	longjmp(restart, sig);
}

static void alarm_sig(int sig)
{
	signal(SIGALRM, catch_sig);
	longjmp(restart, sig);
}

//----- Come here when we get a core dump signal -----------------
static void core_sig(int sig)
{
	signal(SIGQUIT, core_sig);
	signal(SIGILL, core_sig);
	signal(SIGTRAP, core_sig);
	signal(SIGIOT, core_sig);
	signal(SIGABRT, core_sig);
	signal(SIGFPE, core_sig);
	signal(SIGBUS, core_sig);
	signal(SIGSEGV, core_sig);
#ifdef SIGSYS
	signal(SIGSYS, core_sig);
#endif

	dot = bound_dot(dot);	// make sure "dot" is valid

	longjmp(restart, sig);
}
#endif							/* BB_FEATURE_VI_USE_SIGNALS */

static int mysleep(int hund)	// sleep for 'h' 1/100 seconds
{
	// Don't hang- Wait 5/100 seconds-  1 Sec= 1000000
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = hund * 10000;
	select(1, &rfds, NULL, NULL, &tv);
	return (FD_ISSET(0, &rfds));
}

//----- IO Routines --------------------------------------------
static Byte readit(void)	// read (maybe cursor) key from stdin
{
	Byte c;
	int i, bufsiz, cnt, cmdindex;
	struct esc_cmds {
		Byte *seq;
		Byte val;
	};

	static struct esc_cmds esccmds[] = {
		{(Byte *) "OA", (Byte) VI_K_UP},	// cursor key Up
		{(Byte *) "OB", (Byte) VI_K_DOWN},	// cursor key Down
		{(Byte *) "OC", (Byte) VI_K_RIGHT},	// Cursor Key Right
		{(Byte *) "OD", (Byte) VI_K_LEFT},	// cursor key Left
		{(Byte *) "OH", (Byte) VI_K_HOME},	// Cursor Key Home
		{(Byte *) "OF", (Byte) VI_K_END},	// Cursor Key End
		{(Byte *) "[A", (Byte) VI_K_UP},	// cursor key Up
		{(Byte *) "[B", (Byte) VI_K_DOWN},	// cursor key Down
		{(Byte *) "[C", (Byte) VI_K_RIGHT},	// Cursor Key Right
		{(Byte *) "[D", (Byte) VI_K_LEFT},	// cursor key Left
		{(Byte *) "[H", (Byte) VI_K_HOME},	// Cursor Key Home
		{(Byte *) "[F", (Byte) VI_K_END},	// Cursor Key End
		{(Byte *) "[2~", (Byte) VI_K_INSERT},	// Cursor Key Insert
		{(Byte *) "[5~", (Byte) VI_K_PAGEUP},	// Cursor Key Page Up
		{(Byte *) "[6~", (Byte) VI_K_PAGEDOWN},	// Cursor Key Page Down
		{(Byte *) "OP", (Byte) VI_K_FUN1},	// Function Key F1
		{(Byte *) "OQ", (Byte) VI_K_FUN2},	// Function Key F2
		{(Byte *) "OR", (Byte) VI_K_FUN3},	// Function Key F3
		{(Byte *) "OS", (Byte) VI_K_FUN4},	// Function Key F4
		{(Byte *) "[15~", (Byte) VI_K_FUN5},	// Function Key F5
		{(Byte *) "[17~", (Byte) VI_K_FUN6},	// Function Key F6
		{(Byte *) "[18~", (Byte) VI_K_FUN7},	// Function Key F7
		{(Byte *) "[19~", (Byte) VI_K_FUN8},	// Function Key F8
		{(Byte *) "[20~", (Byte) VI_K_FUN9},	// Function Key F9
		{(Byte *) "[21~", (Byte) VI_K_FUN10},	// Function Key F10
		{(Byte *) "[23~", (Byte) VI_K_FUN11},	// Function Key F11
		{(Byte *) "[24~", (Byte) VI_K_FUN12},	// Function Key F12
		{(Byte *) "[11~", (Byte) VI_K_FUN1},	// Function Key F1
		{(Byte *) "[12~", (Byte) VI_K_FUN2},	// Function Key F2
		{(Byte *) "[13~", (Byte) VI_K_FUN3},	// Function Key F3
		{(Byte *) "[14~", (Byte) VI_K_FUN4},	// Function Key F4
	};

#define ESCCMDS_COUNT (sizeof(esccmds)/sizeof(struct esc_cmds))

	(void) alarm(0);	// turn alarm OFF while we wait for input
	// get input from User- are there already input chars in Q?
	bufsiz = strlen((char *) readbuffer);
	if (bufsiz <= 0) {
	  ri0:
		// the Q is empty, wait for a typed char
		bufsiz = read(0, readbuffer, BUFSIZ - 1);
		if (bufsiz < 0) {
			if (errno == EINTR)
				goto ri0;	// interrupted sys call
			if (errno == EBADF)
				editing = 0;
			if (errno == EFAULT)
				editing = 0;
			if (errno == EINVAL)
				editing = 0;
			if (errno == EIO)
				editing = 0;
			errno = 0;
			bufsiz = 0;
		}
		readbuffer[bufsiz] = '\0';
	}
	// return char if it is not part of ESC sequence
	if (readbuffer[0] != 27)
		goto ri1;

	// This is an ESC char. Is this Esc sequence?
	// Could be bare Esc key. See if there are any
	// more chars to read after the ESC. This would
	// be a Function or Cursor Key sequence.
	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 50000;	// Wait 5/100 seconds- 1 Sec=1000000

	// keep reading while there are input chars and room in buffer
	while (select(1, &rfds, NULL, NULL, &tv) > 0 && bufsiz <= (BUFSIZ - 5)) {
		// read the rest of the ESC string
		i = read(0, (void *) (readbuffer + bufsiz), BUFSIZ - bufsiz);
		if (i > 0) {
			bufsiz += i;
			readbuffer[bufsiz] = '\0';	// Terminate the string
		}
	}
	// Maybe cursor or function key?
	for (cmdindex = 0; cmdindex < ESCCMDS_COUNT; cmdindex++) {
		cnt = strlen((char *) esccmds[cmdindex].seq);
		i = strncmp((char *) esccmds[cmdindex].seq, (char *) readbuffer, cnt);
		if (i == 0) {
			// is a Cursor key- put derived value back into Q
			readbuffer[0] = esccmds[cmdindex].val;
			// squeeze out the ESC sequence
			for (i = 1; i < cnt; i++) {
				memmove(readbuffer + 1, readbuffer + 2, BUFSIZ - 2);
				readbuffer[BUFSIZ - 1] = '\0';
			}
			break;
		}
	}
  ri1:
	c = readbuffer[0];
	// remove one char from Q
	memmove(readbuffer, readbuffer + 1, BUFSIZ - 1);
	readbuffer[BUFSIZ - 1] = '\0';
	(void) alarm(3);	// we are done waiting for input, turn alarm ON
	return (c);
}

//----- IO Routines --------------------------------------------
static Byte get_one_char()
{
	static Byte c;

#ifdef BB_FEATURE_VI_DOT_CMD
	// ! adding2q  && ioq == 0  read()
	// ! adding2q  && ioq != 0  *ioq
	// adding2q         *last_modifying_cmd= read()
	if (!adding2q) {
		// we are not adding to the q.
		// but, we may be reading from a q
		if (ioq == 0) {
			// there is no current q, read from STDIN
			c = readit();	// get the users input
		} else {
			// there is a queue to get chars from first
			c = *ioq++;
			if (c == '\0') {
				// the end of the q, read from STDIN
				free(ioq_start);
				ioq_start = ioq = 0;
				c = readit();	// get the users input
			}
		}
	} else {
		// adding STDIN chars to q
		c = readit();	// get the users input
		if (last_modifying_cmd != 0) {
			int len = strlen((char *) last_modifying_cmd);
			if (len + 1 >= BUFSIZ) {
				psbs("last_modifying_cmd overrun");
			} else {
				// add new char to q
				last_modifying_cmd[len] = c;
			}

		}
	}
#else							/* BB_FEATURE_VI_DOT_CMD */
	c = readit();		// get the users input
#endif							/* BB_FEATURE_VI_DOT_CMD */
	return (c);			// return the char, where ever it came from
}

static Byte *get_input_line(Byte * prompt) // get input line- use "status line"
{
	Byte buf[BUFSIZ];
	Byte c;
	int i;
	static Byte *obufp = NULL;

	strcpy((char *) buf, (char *) prompt);
	*status_buffer = '\0';	// clear the status buffer
	place_cursor(rows - 1, 0, FALSE);	// go to Status line, bottom of screen
	clear_to_eol();		// clear the line
	write(1, prompt, strlen((char *) prompt));	// write out the :, /, or ? prompt

	for (i = strlen((char *) buf); i < BUFSIZ;) {
		c = get_one_char();	// read user input
		if (c == '\n' || c == '\r' || c == 27)
			break;		// is this end of input
		if (c == erase_char) {	// user wants to erase prev char
			i--;		// backup to prev char
			buf[i] = '\0';	// erase the char
			buf[i + 1] = '\0';	// null terminate buffer
			write(1, " ", 3);	// erase char on screen
			if (i <= 0) {	// user backs up before b-o-l, exit
				break;
			}
		} else {
			buf[i] = c;	// save char in buffer
			buf[i + 1] = '\0';	// make sure buffer is null terminated
			write(1, buf + i, 1);	// echo the char back to user
			i++;
		}
	}
	refresh(FALSE);
	if (obufp != NULL)
		free(obufp);
	obufp = (Byte *) strdup((char *) buf);
	return (obufp);
}

static int file_size(Byte * fn) // what is the byte size of "fn"
{
	struct stat st_buf;
	int cnt, sr;

	if (fn == 0 || strlen(fn) <= 0)
		return (-1);
	cnt = -1;
	sr = stat((char *) fn, &st_buf);	// see if file exists
	if (sr >= 0) {
		cnt = (int) st_buf.st_size;
	}
	return (cnt);
}

static int file_insert(Byte * fn, Byte * p, int size)
{
	int fd, cnt;

	cnt = -1;
#ifdef BB_FEATURE_VI_READONLY
	readonly = FALSE;
#endif							/* BB_FEATURE_VI_READONLY */
	if (fn == 0 || strlen((char*) fn) <= 0) {
		psbs("No filename given");
		goto fi0;
	}
	if (size == 0) {
		// OK- this is just a no-op
		cnt = 0;
		goto fi0;
	}
	if (size < 0) {
		psbs("Trying to insert a negative number (%d) of characters", size);
		goto fi0;
	}
	if (p < text || p > end) {
		psbs("Trying to insert file outside of memory");
		goto fi0;
	}

	// see if we can open the file
#ifdef BB_FEATURE_VI_READONLY
	if (vi_readonly == TRUE) goto fi1;		// do not try write-mode
#endif
	fd = open((char *) fn, O_RDWR);			// assume read & write
	if (fd < 0) {
		// could not open for writing- maybe file is read only
#ifdef BB_FEATURE_VI_READONLY
  fi1:
#endif
		fd = open((char *) fn, O_RDONLY);	// try read-only
		if (fd < 0) {
			psbs("\"%s\" %s", fn, "could not open file");
			goto fi0;
		}
#ifdef BB_FEATURE_VI_READONLY
		// got the file- read-only
		readonly = TRUE;
#endif							/* BB_FEATURE_VI_READONLY */
	}
	p = text_hole_make(p, size);
	cnt = read(fd, p, size);
	close(fd);
	if (cnt < 0) {
		cnt = -1;
		p = text_hole_delete(p, p + size - 1);	// un-do buffer insert
		psbs("could not read file \"%s\"", fn);
	} else if (cnt < size) {
		// There was a partial read, shrink unused space text[]
		p = text_hole_delete(p + cnt, p + (size - cnt) - 1);	// un-do buffer insert
		psbs("could not read all of file \"%s\"", fn);
	}
	if (cnt >= size)
		file_modified = TRUE;
  fi0:
	return (cnt);
}

static int file_write(Byte * fn, Byte * first, Byte * last)
{
	int fd, cnt, charcnt;

	if (fn == 0) {
		psbs("No current filename");
		return (-1);
	}
	charcnt = 0;
	// FIXIT- use the correct umask()
	fd = open((char *) fn, (O_WRONLY | O_CREAT | O_TRUNC), 0664);
	if (fd < 0)
		return (-1);
	cnt = last - first + 1;
	charcnt = write(fd, first, cnt);
	if (charcnt == cnt) {
		// good write
		//file_modified= FALSE; // the file has not been modified
	} else {
		charcnt = 0;
	}
	close(fd);
	return (charcnt);
}

//----- Terminal Drawing ---------------------------------------
// The terminal is made up of 'rows' line of 'columns' columns.
// classicly this would be 24 x 80.
//  screen coordinates
//  0,0     ...     0,79
//  1,0     ...     1,79
//  .       ...     .
//  .       ...     .
//  22,0    ...     22,79
//  23,0    ...     23,79   status line
//

//----- Move the cursor to row x col (count from 0, not 1) -------
static void place_cursor(int row, int col, int opti)
{
	char cm1[BUFSIZ];
	char *cm;
	int l;
#ifdef BB_FEATURE_VI_OPTIMIZE_CURSOR
	char cm2[BUFSIZ];
	Byte *screenp;
	// char cm3[BUFSIZ];
	int Rrow= last_row;
#endif							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */
	
	memset(cm1, '\0', BUFSIZ - 1);  // clear the buffer

	if (row < 0) row = 0;
	if (row >= rows) row = rows - 1;
	if (col < 0) col = 0;
	if (col >= columns) col = columns - 1;
	
	//----- 1.  Try the standard terminal ESC sequence
	sprintf((char *) cm1, CMrc, row + 1, col + 1);
	cm= cm1;
	if (opti == FALSE) goto pc0;

#ifdef BB_FEATURE_VI_OPTIMIZE_CURSOR
	//----- find the minimum # of chars to move cursor -------------
	//----- 2.  Try moving with discreet chars (Newline, [back]space, ...)
	memset(cm2, '\0', BUFSIZ - 1);  // clear the buffer
	
	// move to the correct row
	while (row < Rrow) {
		// the cursor has to move up
		strcat(cm2, CMup);
		Rrow--;
	}
	while (row > Rrow) {
		// the cursor has to move down
		strcat(cm2, CMdown);
		Rrow++;
	}
	
	// now move to the correct column
	strcat(cm2, "\r");			// start at col 0
	// just send out orignal source char to get to correct place
	screenp = &screen[row * columns];	// start of screen line
	strncat(cm2, screenp, col);

	//----- 3.  Try some other way of moving cursor
	//---------------------------------------------

	// pick the shortest cursor motion to send out
	cm= cm1;
	if (strlen(cm2) < strlen(cm)) {
		cm= cm2;
	}  /* else if (strlen(cm3) < strlen(cm)) {
		cm= cm3;
	} */
#endif							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */
  pc0:
	l= strlen(cm);
	if (l) write(1, cm, l);			// move the cursor
}

//----- Erase from cursor to end of line -----------------------
static void clear_to_eol()
{
	write(1, Ceol, strlen(Ceol));	// Erase from cursor to end of line
}

//----- Erase from cursor to end of screen -----------------------
static void clear_to_eos()
{
	write(1, Ceos, strlen(Ceos));	// Erase from cursor to end of screen
}

//----- Start standout mode ------------------------------------
static void standout_start() // send "start reverse video" sequence
{
	write(1, SOs, strlen(SOs));	// Start reverse video mode
}

//----- End standout mode --------------------------------------
static void standout_end() // send "end reverse video" sequence
{
	write(1, SOn, strlen(SOn));	// End reverse video mode
}

//----- Flash the screen  --------------------------------------
static void flash(int h)
{
	standout_start();	// send "start reverse video" sequence
	redraw(TRUE);
	(void) mysleep(h);
	standout_end();		// send "end reverse video" sequence
	redraw(TRUE);
}

static void beep()
{
	write(1, bell, strlen(bell));	// send out a bell character
}

static void indicate_error(char c)
{
#ifdef BB_FEATURE_VI_CRASHME
	if (crashme > 0)
		return;			// generate a random command
#endif							/* BB_FEATURE_VI_CRASHME */
	if (err_method == 0) {
		beep();
	} else {
		flash(10);
	}
}

//----- Screen[] Routines --------------------------------------
//----- Erase the Screen[] memory ------------------------------
static void screen_erase()
{
	memset(screen, ' ', screensize);	// clear new screen
}

//----- Draw the status line at bottom of the screen -------------
static void show_status_line(void)
{
	static int last_cksum;
	int l, cnt, cksum;

	cnt = strlen((char *) status_buffer);
	for (cksum= l= 0; l < cnt; l++) { cksum += (int)(status_buffer[l]); }
	// don't write the status line unless it changes
	if (cnt > 0 && last_cksum != cksum) {
		last_cksum= cksum;		// remember if we have seen this line
		place_cursor(rows - 1, 0, FALSE);	// put cursor on status line
		write(1, status_buffer, cnt);
		clear_to_eol();
		place_cursor(crow, ccol, FALSE);	// put cursor back in correct place
	}
}

//----- format the status buffer, the bottom line of screen ------
// print status buffer, with STANDOUT mode
static void psbs(char *format, ...)
{
	va_list args;

	va_start(args, format);
	strcpy((char *) status_buffer, SOs);	// Terminal standout mode on
	vsprintf((char *) status_buffer + strlen((char *) status_buffer), format,
			 args);
	strcat((char *) status_buffer, SOn);	// Terminal standout mode off
	va_end(args);

	return;
}

// print status buffer
static void psb(char *format, ...)
{
	va_list args;

	va_start(args, format);
	vsprintf((char *) status_buffer, format, args);
	va_end(args);
	return;
}

static void ni(Byte * s) // display messages
{
	Byte buf[BUFSIZ];

	print_literal(buf, s);
	psbs("\'%s\' is not implemented", buf);
}

static void edit_status(void)	// show file status on status line
{
	int cur, tot, percent;

	cur = count_lines(text, dot);
	tot = count_lines(text, end - 1);
	//    current line         percent
	//   -------------    ~~ ----------
	//    total lines            100
	if (tot > 0) {
		percent = (100 * cur) / tot;
	} else {
		cur = tot = 0;
		percent = 100;
	}
	psb("\"%s\""
#ifdef BB_FEATURE_VI_READONLY
		"%s"
#endif							/* BB_FEATURE_VI_READONLY */
		"%s line %d of %d --%d%%--",
		(cfn != 0 ? (char *) cfn : "No file"),
#ifdef BB_FEATURE_VI_READONLY
		((vi_readonly == TRUE || readonly == TRUE) ? " [Read only]" : ""),
#endif							/* BB_FEATURE_VI_READONLY */
		(file_modified == TRUE ? " [modified]" : ""),
		cur, tot, percent);
}

//----- Force refresh of all Lines -----------------------------
static void redraw(int full_screen)
{
	place_cursor(0, 0, FALSE);	// put cursor in correct place
	clear_to_eos();		// tel terminal to erase display
	screen_erase();		// erase the internal screen buffer
	refresh(full_screen);	// this will redraw the entire display
}

//----- Format a text[] line into a buffer ---------------------
static void format_line(Byte *dest, Byte *src, int li)
{
	int co;
	Byte c;
	
	for (co= 0; co < MAX_SCR_COLS; co++) {
		c= ' ';		// assume blank
		if (li > 0 && co == 0) {
			c = '~';        // not first line, assume Tilde
		}
		// are there chars in text[] and have we gone past the end
		if (text < end && src < end) {
			c = *src++;
		}
		if (c == '\n')
			break;
		if (c < ' ' || c > '~') {
			if (c == '\t') {
				c = ' ';
				//       co %    8     !=     7
				for (; (co % tabstop) != (tabstop - 1); co++) {
					dest[co] = c;
				}
			} else {
				dest[co++] = '^';
				c |= '@';       // make it visible
				c &= 0x7f;      // get rid of hi bit
			}
		}
		// the co++ is done here so that the column will
		// not be overwritten when we blank-out the rest of line
		dest[co] = c;
		if (src >= end)
			break;
	}
}

//----- Refresh the changed screen lines -----------------------
// Copy the source line from text[] into the buffer and note
// if the current screenline is different from the new buffer.
// If they differ then that line needs redrawing on the terminal.
//
static void refresh(int full_screen)
{
	static int old_offset;
	int li, changed;
	Byte buf[MAX_SCR_COLS];
	Byte *tp, *sp;		// pointer into text[] and screen[]
#ifdef BB_FEATURE_VI_OPTIMIZE_CURSOR
	int last_li= -2;				// last line that changed- for optimizing cursor movement
#endif							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */

#ifdef BB_FEATURE_VI_WIN_RESIZE
	window_size_get(0);
#endif							/* BB_FEATURE_VI_WIN_RESIZE */
	sync_cursor(dot, &crow, &ccol);	// where cursor will be (on "dot")
	tp = screenbegin;	// index into text[] of top line

	// compare text[] to screen[] and mark screen[] lines that need updating
	for (li = 0; li < rows - 1; li++) {
		int cs, ce;				// column start & end
		memset(buf, ' ', MAX_SCR_COLS);		// blank-out the buffer
		buf[MAX_SCR_COLS-1] = 0;		// NULL terminate the buffer
		// format current text line into buf
		format_line(buf, tp, li);

		// skip to the end of the current text[] line
		while (tp < end && *tp++ != '\n') /*no-op*/ ;

		// see if there are any changes between vitual screen and buf
		changed = FALSE;	// assume no change
		cs= 0;
		ce= columns-1;
		sp = &screen[li * columns];	// start of screen line
		if (full_screen == TRUE) {
			// force re-draw of every single column from 0 - columns-1
			goto re0;
		}
		// compare newly formatted buffer with virtual screen
		// look forward for first difference between buf and screen
		for ( ; cs <= ce; cs++) {
			if (buf[cs + offset] != sp[cs]) {
				changed = TRUE;	// mark for redraw
				break;
			}
		}

		// look backward for last difference between buf and screen
		for ( ; ce >= cs; ce--) {
			if (buf[ce + offset] != sp[ce]) {
				changed = TRUE;	// mark for redraw
				break;
			}
		}
		// now, cs is index of first diff, and ce is index of last diff

		// if horz offset has changed, force a redraw
		if (offset != old_offset) {
  re0:
			changed = TRUE;
		}

		// make a sanity check of columns indexes
		if (cs < 0) cs= 0;
		if (ce > columns-1) ce= columns-1;
		if (cs > ce) {  cs= 0;  ce= columns-1;  }
		// is there a change between vitual screen and buf
		if (changed == TRUE) {
			//  copy changed part of buffer to virtual screen
			memmove(sp+cs, buf+(cs+offset), ce-cs+1);

			// move cursor to column of first change
			if (offset != old_offset) {
				// opti_cur_move is still too stupid
				// to handle offsets correctly
				place_cursor(li, cs, FALSE);
			} else {
#ifdef BB_FEATURE_VI_OPTIMIZE_CURSOR
				// if this just the next line
				//  try to optimize cursor movement
				//  otherwise, use standard ESC sequence
				place_cursor(li, cs, li == (last_li+1) ? TRUE : FALSE);
				last_li= li;
#else							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */
				place_cursor(li, cs, FALSE);	// use standard ESC sequence
#endif							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */
			}

			// write line out to terminal
			write(1, sp+cs, ce-cs+1);
#ifdef BB_FEATURE_VI_OPTIMIZE_CURSOR
			last_row = li;
#endif							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */
		}
	}

#ifdef BB_FEATURE_VI_OPTIMIZE_CURSOR
	place_cursor(crow, ccol, (crow == last_row) ? TRUE : FALSE);
	last_row = crow;
#else
	place_cursor(crow, ccol, FALSE);
#endif							/* BB_FEATURE_VI_OPTIMIZE_CURSOR */
	
	if (offset != old_offset)
		old_offset = offset;
}
