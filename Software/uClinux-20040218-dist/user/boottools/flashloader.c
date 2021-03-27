#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <bootstd.h>
#include <flash.h>

_bsc2(int,program,void *,a1,int,a2)

#define MAX_CHAIN_LEN 512

void *chain[MAX_CHAIN_LEN];

char spinner[] = { 8, '|' , 8, '\\' , 8, '-', 8, '/'};

main(int argc, char *argv[])
{
	int fd;

	int count;
	int n;
	int b;
	mnode_t m;

	if (argc != 2) {
		printf("Usage: %s imagefile.bin\n",argv[0]);
		exit(1);
	}

	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		printf("%s: file [%s] not found\n",argv[0], argv[1]);
		exit(2);
	}
	printf("Loading file [%s]\n", argv[1]);

	for (count = b = 0; n == 4096 || !b ; count += n) {
		chain[b] = malloc(4096);
		n = read(fd, chain[b++], 4096);
		write(1, &spinner[(b & 3) << 1], 2);
		if (b >= MAX_CHAIN_LEN) {
			printf("%s: file [%s] too large to load\n",argv[0], argv[1]);
			exit(3);
		}
	}
	printf("Loaded %d bytes\n",count);

	m.len = count;
	m.offset = (void *)chain;

	program(&m, PGM_ERASE_FIRST | PGM_EXEC_AFTER);
	/* not reached, PGM_EXEC_AFTER starts the new kernel */
}
