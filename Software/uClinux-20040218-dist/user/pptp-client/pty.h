/* pty.h ....... find a free pty/tty pair.  
 *               Inspired/stolen from the xterm source.
 *               NOTE: This is very likely to be highly non-portable.
 *               C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: pty.h,v 1.2 2000/07/24 23:18:27 matthewn Exp $
 */

/* Hmm.  PTYs can be anywhere.... */

#ifdef __linux__
#define PTYDEV	"/dev/ptyxx"
#define TTYDEV	"/dev/ttyxx"

#define PTYMAX  (strlen(PTYDEV)+1)
#define TTYMAX  (strlen(TTYDEV)+1)

#define PTYCHAR1	"abcdepqrstuvwxyz"
#define PTYCHAR2	"0123456789abcdef"
#endif

/* Get pty/tty pair, put filename in ttydev, ptydev (which must be
 * at least PTYMAX characters long), and return file descriptor of
 * open pty.
 * Return value < 0 indicates failure.
 */
int getpseudotty(char *ttydev, char *ptydev);
