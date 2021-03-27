/* vi: set sw=4 ts=4: */
/*
 * test implementation for busybox
 *
 * Copyright (c) by a whole pile of folks: 
 *
 * 	test(1); version 7-like  --  author Erik Baalbergen
 * 	modified by Eric Gisin to be used as built-in.
 * 	modified by Arnold Robbins to add SVR3 compatibility
 * 	(-x -c -b -p -u -g -k) plus Korn's -L -nt -ot -ef and new -S (socket).
 * 	modified by J.T. Conklin for NetBSD.
 * 	modified by Herbert Xu to be used as built-in in ash.
 * 	modified by Erik Andersen <andersee@debian.org> to be used 
 * 	in busybox.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Original copyright notice states:
 * 	"This program is in the Public Domain."
 */

#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "busybox.h"

/* test(1) accepts the following grammar:
	oexpr	::= aexpr | aexpr "-o" oexpr ;
	aexpr	::= nexpr | nexpr "-a" aexpr ;
	nexpr	::= primary | "!" primary
	primary	::= unary-operator operand
		| operand binary-operator operand
		| operand
		| "(" oexpr ")"
		;
	unary-operator ::= "-r"|"-w"|"-x"|"-f"|"-d"|"-c"|"-b"|"-p"|
		"-u"|"-g"|"-k"|"-s"|"-t"|"-z"|"-n"|"-o"|"-O"|"-G"|"-L"|"-S";

	binary-operator ::= "="|"!="|"-eq"|"-ne"|"-ge"|"-gt"|"-le"|"-lt"|
			"-nt"|"-ot"|"-ef";
	operand ::= <any legal UNIX file name>
*/

enum token {
	EOI,
	FILRD,
	FILWR,
	FILEX,
	FILEXIST,
	FILREG,
	FILDIR,
	FILCDEV,
	FILBDEV,
	FILFIFO,
	FILSOCK,
	FILSYM,
	FILGZ,
	FILTT,
	FILSUID,
	FILSGID,
	FILSTCK,
	FILNT,
	FILOT,
	FILEQ,
	FILUID,
	FILGID,
	STREZ,
	STRNZ,
	STREQ,
	STRNE,
	STRLT,
	STRGT,
	INTEQ,
	INTNE,
	INTGE,
	INTGT,
	INTLE,
	INTLT,
	UNOT,
	BAND,
	BOR,
	LPAREN,
	RPAREN,
	OPERAND
};

enum token_types {
	UNOP,
	BINOP,
	BUNOP,
	BBINOP,
	PAREN
};

static const struct t_op {
	const char *op_text;
	short op_num, op_type;
} ops [] = {
	{"-r",	FILRD,	UNOP},
	{"-w",	FILWR,	UNOP},
	{"-x",	FILEX,	UNOP},
	{"-e",	FILEXIST,UNOP},
	{"-f",	FILREG,	UNOP},
	{"-d",	FILDIR,	UNOP},
	{"-c",	FILCDEV,UNOP},
	{"-b",	FILBDEV,UNOP},
	{"-p",	FILFIFO,UNOP},
	{"-u",	FILSUID,UNOP},
	{"-g",	FILSGID,UNOP},
	{"-k",	FILSTCK,UNOP},
	{"-s",	FILGZ,	UNOP},
	{"-t",	FILTT,	UNOP},
	{"-z",	STREZ,	UNOP},
	{"-n",	STRNZ,	UNOP},
	{"-h",	FILSYM,	UNOP},		/* for backwards compat */
	{"-O",	FILUID,	UNOP},
	{"-G",	FILGID,	UNOP},
	{"-L",	FILSYM,	UNOP},
	{"-S",	FILSOCK,UNOP},
	{"=",	STREQ,	BINOP},
	{"!=",	STRNE,	BINOP},
	{"<",	STRLT,	BINOP},
	{">",	STRGT,	BINOP},
	{"-eq",	INTEQ,	BINOP},
	{"-ne",	INTNE,	BINOP},
	{"-ge",	INTGE,	BINOP},
	{"-gt",	INTGT,	BINOP},
	{"-le",	INTLE,	BINOP},
	{"-lt",	INTLT,	BINOP},
	{"-nt",	FILNT,	BINOP},
	{"-ot",	FILOT,	BINOP},
	{"-ef",	FILEQ,	BINOP},
	{"!",	UNOT,	BUNOP},
	{"-a",	BAND,	BBINOP},
	{"-o",	BOR,	BBINOP},
	{"(",	LPAREN,	PAREN},
	{")",	RPAREN,	PAREN},
	{0,	0,	0}
};

static char **t_wp;
static struct t_op const *t_wp_op;
static gid_t *group_array = NULL;
static int ngroups;

static enum token t_lex();
static int oexpr();
static int aexpr();
static int nexpr();
static int binop();
static int primary();
static int filstat();
static int getn();
static int newerf();
static int olderf();
static int equalf();
static void syntax();
static int test_eaccess();
static int is_a_group_member();
static void initialize_group_array();

extern int
test_main(int argc, char** argv)
{
	int	res;

	if (strcmp(applet_name, "[") == 0) {
		if (strcmp(argv[--argc], "]"))
			error_msg_and_die("missing ]");
		argv[argc] = NULL;
	}
	/* Implement special cases from POSIX.2, section 4.62.4 */
	switch (argc) {
	case 1:
		exit( 1);
	case 2:
		exit (*argv[1] == '\0');
	case 3:
		if (argv[1][0] == '!' && argv[1][1] == '\0') {
			exit (!(*argv[2] == '\0'));
		}
		break;
	case 4:
		if (argv[1][0] != '!' || argv[1][1] != '\0') {
			if (t_lex(argv[2]), 
			    t_wp_op && t_wp_op->op_type == BINOP) {
				t_wp = &argv[1];
				exit (binop() == 0);
			}
		}
		break;
	case 5:
		if (argv[1][0] == '!' && argv[1][1] == '\0') {
			if (t_lex(argv[3]), 
			    t_wp_op && t_wp_op->op_type == BINOP) {
				t_wp = &argv[2];
				exit (!(binop() == 0));
			}
		}
		break;
	}

	t_wp = &argv[1];
	res = !oexpr(t_lex(*t_wp));

	if (*t_wp != NULL && *++t_wp != NULL)
		syntax(*t_wp, "unknown operand");

	return( res);
}

static void
syntax(op, msg)
	char	*op;
	char	*msg;
{
	if (op && *op)
		error_msg_and_die("%s: %s", op, msg);
	else
		error_msg_and_die("%s", msg);
}

static int
oexpr(n)
	enum token n;
{
	int res;

	res = aexpr(n);
	if (t_lex(*++t_wp) == BOR)
		return oexpr(t_lex(*++t_wp)) || res;
	t_wp--;
	return res;
}

static int
aexpr(n)
	enum token n;
{
	int res;

	res = nexpr(n);
	if (t_lex(*++t_wp) == BAND)
		return aexpr(t_lex(*++t_wp)) && res;
	t_wp--;
	return res;
}

static int
nexpr(n)
	enum token n;			/* token */
{
	if (n == UNOT)
		return !nexpr(t_lex(*++t_wp));
	return primary(n);
}

static int
primary(n)
	enum token n;
{
	int res;

	if (n == EOI)
		syntax(NULL, "argument expected");
	if (n == LPAREN) {
		res = oexpr(t_lex(*++t_wp));
		if (t_lex(*++t_wp) != RPAREN)
			syntax(NULL, "closing paren expected");
		return res;
	}
	if (t_wp_op && t_wp_op->op_type == UNOP) {
		/* unary expression */
		if (*++t_wp == NULL)
			syntax(t_wp_op->op_text, "argument expected");
		switch (n) {
		case STREZ:
			return strlen(*t_wp) == 0;
		case STRNZ:
			return strlen(*t_wp) != 0;
		case FILTT:
			return isatty(getn(*t_wp));
		default:
			return filstat(*t_wp, n);
		}
	}

	if (t_lex(t_wp[1]), t_wp_op && t_wp_op->op_type == BINOP) {
		return binop();
	}	  

	return strlen(*t_wp) > 0;
}

static int
binop()
{
	const char *opnd1, *opnd2;
	struct t_op const *op;

	opnd1 = *t_wp;
	(void) t_lex(*++t_wp);
	op = t_wp_op;

	if ((opnd2 = *++t_wp) == (char *)0)
		syntax(op->op_text, "argument expected");
		
	switch (op->op_num) {
	case STREQ:
		return strcmp(opnd1, opnd2) == 0;
	case STRNE:
		return strcmp(opnd1, opnd2) != 0;
	case STRLT:
		return strcmp(opnd1, opnd2) < 0;
	case STRGT:
		return strcmp(opnd1, opnd2) > 0;
	case INTEQ:
		return getn(opnd1) == getn(opnd2);
	case INTNE:
		return getn(opnd1) != getn(opnd2);
	case INTGE:
		return getn(opnd1) >= getn(opnd2);
	case INTGT:
		return getn(opnd1) > getn(opnd2);
	case INTLE:
		return getn(opnd1) <= getn(opnd2);
	case INTLT:
		return getn(opnd1) < getn(opnd2);
	case FILNT:
		return newerf (opnd1, opnd2);
	case FILOT:
		return olderf (opnd1, opnd2);
	case FILEQ:
		return equalf (opnd1, opnd2);
	}
	/* NOTREACHED */
	return 1;
}

static int
filstat(nm, mode)
	char *nm;
	enum token mode;
{
	struct stat s;
	unsigned int i;

	if (mode == FILSYM) {
#ifdef S_IFLNK
		if (lstat(nm, &s) == 0) {
			i = S_IFLNK;
			goto filetype;
		}
#endif
		return 0;
	}

	if (stat(nm, &s) != 0) 
		return 0;

	switch (mode) {
	case FILRD:
		return test_eaccess(nm, R_OK) == 0;
	case FILWR:
		return test_eaccess(nm, W_OK) == 0;
	case FILEX:
		return test_eaccess(nm, X_OK) == 0;
	case FILEXIST:
		return 1;
	case FILREG:
		i = S_IFREG;
		goto filetype;
	case FILDIR:
		i = S_IFDIR;
		goto filetype;
	case FILCDEV:
		i = S_IFCHR;
		goto filetype;
	case FILBDEV:
		i = S_IFBLK;
		goto filetype;
	case FILFIFO:
#ifdef S_IFIFO
		i = S_IFIFO;
		goto filetype;
#else
		return 0;
#endif
	case FILSOCK:
#ifdef S_IFSOCK
		i = S_IFSOCK;
		goto filetype;
#else
		return 0;
#endif
	case FILSUID:
		i = S_ISUID;
		goto filebit;
	case FILSGID:
		i = S_ISGID;
		goto filebit;
	case FILSTCK:
		i = S_ISVTX;
		goto filebit;
	case FILGZ:
		return s.st_size > 0L;
	case FILUID:
		return s.st_uid == geteuid();
	case FILGID:
		return s.st_gid == getegid();
	default:
		return 1;
	}

filetype:
	return ((s.st_mode & S_IFMT) == i);

filebit:
	return ((s.st_mode & i) != 0);
}

static enum token
t_lex(s)
	char *s;
{
	struct t_op const *op = ops;

	if (s == 0) {
		t_wp_op = (struct t_op *)0;
		return EOI;
	}
	while (op->op_text) {
		if (strcmp(s, op->op_text) == 0) {
			t_wp_op = op;
			return op->op_num;
		}
		op++;
	}
	t_wp_op = (struct t_op *)0;
	return OPERAND;
}

/* atoi with error detection */
static int
getn(s)
	char *s;
{
	char *p;
	long r;

	errno = 0;
	r = strtol(s, &p, 10);

	if (errno != 0)
	  error_msg_and_die("%s: out of range", s);

	while (isspace(*p))
	  p++;
	
	if (*p)
	  error_msg_and_die("%s: bad number", s);

	return (int) r;
}

static int
newerf (f1, f2)
char *f1, *f2;
{
	struct stat b1, b2;

	return (stat (f1, &b1) == 0 &&
		stat (f2, &b2) == 0 &&
		b1.st_mtime > b2.st_mtime);
}

static int
olderf (f1, f2)
char *f1, *f2;
{
	struct stat b1, b2;

	return (stat (f1, &b1) == 0 &&
		stat (f2, &b2) == 0 &&
		b1.st_mtime < b2.st_mtime);
}

static int
equalf (f1, f2)
char *f1, *f2;
{
	struct stat b1, b2;

	return (stat (f1, &b1) == 0 &&
		stat (f2, &b2) == 0 &&
		b1.st_dev == b2.st_dev &&
		b1.st_ino == b2.st_ino);
}

/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
static int
test_eaccess (path, mode)
char *path;
int mode;
{
	struct stat st;
	unsigned int euid = geteuid();

	if (stat (path, &st) < 0)
		return (-1);

	if (euid == 0) {
		/* Root can read or write any file. */
		if (mode != X_OK)
		return (0);

		/* Root can execute any file that has any one of the execute
		   bits set. */
		if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
			return (0);
	}

	if (st.st_uid == euid)		/* owner */
		mode <<= 6;
	else if (is_a_group_member (st.st_gid))
		mode <<= 3;

	if (st.st_mode & mode)
		return (0);

	return (-1);
}

static void
initialize_group_array ()
{
	ngroups = getgroups(0, NULL);
	group_array = xrealloc(group_array, ngroups * sizeof(gid_t));
	getgroups(ngroups, group_array);
}

/* Return non-zero if GID is one that we have in our groups list. */
static int
is_a_group_member (gid)
gid_t gid;
{
	register int i;

	/* Short-circuit if possible, maybe saving a call to getgroups(). */
	if (gid == getgid() || gid == getegid())
		return (1);

	if (ngroups == 0)
		initialize_group_array ();

	/* Search through the list looking for GID. */
	for (i = 0; i < ngroups; i++)
		if (gid == group_array[i])
			return (1);

	return (0);
}
