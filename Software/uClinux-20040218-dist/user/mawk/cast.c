
/********************************************
cast.c
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/


/*   $Log: cast.c,v $
 *   Revision 1.6  1996/08/11 22:07:50  mike
 *   Fix small bozo in rt_error("overflow converting ...")
 *
 * Revision 1.5  1995/06/18  19:17:45  mike
 * Create a type Int which on most machines is an int, but on machines
 * with 16bit ints, i.e., the PC is a long.  This fixes implicit assumption
 * that int==long.
 *
 * Revision 1.4  1995/06/06  00:02:02  mike
 * fix cast in d_to_l()
 *
 * Revision 1.3  1993/07/17  13:22:45  mike
 * indent and general code cleanup
 *
 * Revision 1.2	 1993/07/04  12:51:41  mike
 * start on autoconfig changes
 *
 * Revision 5.5	 1993/03/06  18:49:45  mike
 * rm rt_overflow from check_strnum
 *
 * Revision 5.4	 1993/02/13  21:57:20  mike
 * merge patch3
 *
 * Revision 5.3.1.4  1993/01/22	 15:05:19  mike
 * pow2->mpow2 for linux
 *
 * Revision 5.3.1.3  1993/01/22	 14:18:33  mike
 * const for strtod and ansi picky compilers
 *
 * Revision 5.3.1.2  1993/01/20	 12:53:06  mike
 * d_to_l()
 *
 * Revision 5.3.1.1  1993/01/15	 03:33:37  mike
 * patch3: safer double to int conversion
 *
 * Revision 5.3	 1992/11/28  23:48:42  mike
 * For internal conversion numeric->string, when testing
 * if integer, use longs instead of ints so 16 and 32 bit
 * systems behave the same
 *
 * Revision 5.2	 1992/08/17  14:19:45  brennan
 * patch2: After parsing, only bi_sprintf() uses string_buff.
 *
 * Revision 5.1	 1991/12/05  07:55:41  brennan
 * 1.1 pre-release
 *
*/


/*  cast.c  */

#include "mawk.h"
#include "field.h"
#include "memory.h"
#include "scan.h"
#include "repl.h"

int mpow2[NUM_CELL_TYPES] =
{1, 2, 4, 8, 16, 32, 64, 128, 256, 512} ;

void
cast1_to_d(cp)
   register CELL *cp ;
{
   switch (cp->type)
   {
      case C_NOINIT:
	 cp->dval = 0.0 ;
	 break ;

      case C_DOUBLE:
	 return ;

      case C_MBSTRN:
      case C_STRING:
	 {
	    register STRING *s = (STRING *) cp->ptr ;

#if FPE_TRAPS_ON		/* look for overflow error */
	    errno = 0 ;
	    cp->dval = strtod(s->str, (char **) 0) ;
	    if (errno && cp->dval != 0.0)	/* ignore underflow */
	       rt_error("overflow converting %s to double", s->str) ;
#else
	    cp->dval = strtod(s->str, (char **) 0) ;
#endif
	    free_STRING(s) ;
	 }
	 break ;

      case C_STRNUM:
	 /* don't need to convert, but do need to free the STRING part */
	 free_STRING(string(cp)) ;
	 break ;


      default:
	 bozo("cast on bad type") ;
   }
   cp->type = C_DOUBLE ;
}

void
cast2_to_d(cp)
   register CELL *cp ;
{
   register STRING *s ;

   switch (cp->type)
   {
      case C_NOINIT:
	 cp->dval = 0.0 ;
	 break ;

      case C_DOUBLE:
	 goto two ;
      case C_STRNUM:
	 free_STRING(string(cp)) ;
	 break ;

      case C_MBSTRN:
      case C_STRING:
	 s = (STRING *) cp->ptr ;

#if FPE_TRAPS_ON		/* look for overflow error */
	 errno = 0 ;
	 cp->dval = strtod(s->str, (char **) 0) ;
	 if (errno && cp->dval != 0.0)	/* ignore underflow */
	    rt_error("overflow converting %s to double", s->str) ;
#else
	 cp->dval = strtod(s->str, (char **) 0) ;
#endif
	 free_STRING(s) ;
	 break ;

      default:
	 bozo("cast on bad type") ;
   }
   cp->type = C_DOUBLE ;

 two:cp++ ;

   switch (cp->type)
   {
      case C_NOINIT:
	 cp->dval = 0.0 ;
	 break ;

      case C_DOUBLE:
	 return ;
      case C_STRNUM:
	 free_STRING(string(cp)) ;
	 break ;

      case C_MBSTRN:
      case C_STRING:
	 s = (STRING *) cp->ptr ;

#if FPE_TRAPS_ON		/* look for overflow error */
	 errno = 0 ;
	 cp->dval = strtod(s->str, (char **) 0) ;
	 if (errno && cp->dval != 0.0)	/* ignore underflow */
	    rt_error("overflow converting %s to double", s->str) ;
#else
	 cp->dval = strtod(s->str, (char **) 0) ;
#endif
	 free_STRING(s) ;
	 break ;

      default:
	 bozo("cast on bad type") ;
   }
   cp->type = C_DOUBLE ;
}

void
cast1_to_s(cp)
   register CELL *cp ;
{
   register Int lval ;
   char xbuff[260] ;

   switch (cp->type)
   {
      case C_NOINIT:
	 null_str.ref_cnt++ ;
	 cp->ptr = (PTR) & null_str ;
	 break ;

      case C_DOUBLE:

	 lval = d_to_I(cp->dval) ;
	 if (lval == cp->dval)	sprintf(xbuff, INT_FMT, lval) ;
	 else  sprintf(xbuff, string(CONVFMT)->str, cp->dval) ;

	 cp->ptr = (PTR) new_STRING(xbuff) ;
	 break ;

      case C_STRING:
	 return ;

      case C_MBSTRN:
      case C_STRNUM:
	 break ;

      default:
	 bozo("bad type on cast") ;
   }
   cp->type = C_STRING ;
}

void
cast2_to_s(cp)
   register CELL *cp ;
{
   register Int lval ;
   char xbuff[260] ;

   switch (cp->type)
   {
      case C_NOINIT:
	 null_str.ref_cnt++ ;
	 cp->ptr = (PTR) & null_str ;
	 break ;

      case C_DOUBLE:

	 lval = d_to_I(cp->dval) ;
	 if (lval == cp->dval)	sprintf(xbuff, INT_FMT, lval) ;
	 else  sprintf(xbuff, string(CONVFMT)->str, cp->dval) ;

	 cp->ptr = (PTR) new_STRING(xbuff) ;
	 break ;

      case C_STRING:
	 goto two ;

      case C_MBSTRN:
      case C_STRNUM:
	 break ;

      default:
	 bozo("bad type on cast") ;
   }
   cp->type = C_STRING ;

two:
   cp++ ;

   switch (cp->type)
   {
      case C_NOINIT:
	 null_str.ref_cnt++ ;
	 cp->ptr = (PTR) & null_str ;
	 break ;

      case C_DOUBLE:

	 lval = d_to_I(cp->dval) ;
	 if (lval == cp->dval)	sprintf(xbuff, INT_FMT, lval) ;
	 else  sprintf(xbuff, string(CONVFMT)->str, cp->dval) ;

	 cp->ptr = (PTR) new_STRING(xbuff) ;
	 break ;

      case C_STRING:
	 return ;

      case C_MBSTRN:
      case C_STRNUM:
	 break ;

      default:
	 bozo("bad type on cast") ;
   }
   cp->type = C_STRING ;
}

void
cast_to_RE(cp)
   register CELL *cp ;
{
   register PTR p ;

   if (cp->type < C_STRING)  cast1_to_s(cp) ;

   p = re_compile(string(cp)) ;
   free_STRING(string(cp)) ;
   cp->type = C_RE ;
   cp->ptr = p ;

}

void
cast_for_split(cp)
   register CELL *cp ;
{
   static char meta[] = "^$.*+?|[]()" ;
   static char xbuff[] = "\\X" ;
   int c ;
   unsigned len ;

   if (cp->type < C_STRING)  cast1_to_s(cp) ;

   if ((len = string(cp)->len) == 1)
   {
      if ((c = string(cp)->str[0]) == ' ')
      {
	 free_STRING(string(cp)) ;
	 cp->type = C_SPACE ;
	 return ;
      }
      else if (strchr(meta, c))
      {
	 xbuff[1] = c ;
	 free_STRING(string(cp)) ;
	 cp->ptr = (PTR) new_STRING(xbuff) ;
      }
   }
   else if (len == 0)
   {
      free_STRING(string(cp)) ;
      cp->type = C_SNULL ;
      return ;
   }

   cast_to_RE(cp) ;
}

/* input: cp-> a CELL of type C_MBSTRN (maybe strnum)
   test it -- casting it to the appropriate type
   which is C_STRING or C_STRNUM
*/

void
check_strnum(cp)
   CELL *cp ;
{
   char *test ;
   register unsigned char *s, *q ;

   cp->type = C_STRING ;	 /* assume not C_STRNUM */
   s = (unsigned char *) string(cp)->str ;
   q = s + string(cp)->len ;
   while (scan_code[*s] == SC_SPACE)  s++ ;
   if (s == q)	return ;

   while (scan_code[q[-1]] == SC_SPACE)	 q-- ;
   if (scan_code[q[-1]] != SC_DIGIT &&
       q[-1] != '.')
      return ;

   switch (scan_code[*s])
   {
      case SC_DIGIT:
      case SC_PLUS:
      case SC_MINUS:
      case SC_DOT:

#if FPE_TRAPS_ON
	 errno = 0 ;
	 cp->dval = strtod((char *) s, &test) ;
	 /* make overflow pure string */
	 if (errno && cp->dval != 0.0)	return ;
#else
	 cp->dval = strtod((char *) s, &test) ;
#endif

	 if ((char *) q <= test)  cp->type = C_STRNUM ;
	 /*  <= instead of == , for some buggy strtod
		 e.g. Apple Unix */
   }
}

/* cast a CELL to a replacement cell */

void
cast_to_REPL(cp)
   register CELL *cp ;
{
   register STRING *sval ;

   if (cp->type < C_STRING)  cast1_to_s(cp) ;
   sval = (STRING *) cp->ptr ;

   cellcpy(cp, repl_compile(sval)) ;
   free_STRING(sval) ;
}


/* convert a double to Int (this is not as simple as a
   cast because the results are undefined if it won't fit).
   Truncate large values to +Max_Int or -Max_Int
   Send nans to -Max_Int
*/

Int
d_to_I(d)
   double d;
{
   if (d >= Max_Int)	return Max_Int ;
   if (d > -Max_Int)	return (Int) d ;
   return -Max_Int ;
}
