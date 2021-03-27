#include <unistd.h>
#include <string.h>

static void
remove_suffix (name, suffix)
	register char *name, *suffix;
{
	register char *np, *sp;

	np = name + strlen (name);
	sp = suffix + strlen (suffix);

	while (np > name && sp > suffix)
		if (*--np != *--sp)
			return;
	if (np > name)
		*np = '\0';
}
                             

char *
basename (name)
	char *name;
{
	char *base;

	base = rindex (name, '/');
	return base ? base + 1 : name;
}
           


void
strip_trailing_slashes (path)
char *path;
{
	int last;

	last = strlen (path) - 1;
	while (last > 0 && path[last] == '/')
		path[last--] = '\0';
}
               

int
main (argc, argv)
	int argc;
	char **argv;
{
	char *line;
	
	if (argc == 2 || argc == 3) {
		strip_trailing_slashes(argv[1]);
		line = basename(argv[1]);
                if (argc == 3)
			remove_suffix (line, argv[2]);
		write(STDOUT_FILENO,line,strlen(line));
		write(STDOUT_FILENO,"\n",1);
	}
	exit(0);
}
