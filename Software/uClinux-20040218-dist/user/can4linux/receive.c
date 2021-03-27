#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#include <can4linux.h>

#define STDDEV "can0"

int main(int argc,char **argv)
{
int fd;
int got;
canmsg_t rx;
char device[40];

    if(argc == 2) {
	sprintf(device, "/dev/%s", argv[1]);
    }
    else {
	sprintf(device, "/dev/%s", STDDEV);
    }
    printf("using CAN device %s\n", device);
    
    if(( fd = open(device, O_RDWR )) < 0 ) {
	fprintf(stderr,"Error opening CAN device %s\n", device);
        exit(1);
    }

    printf("waiting for msg at 0x%p:\n", &rx);

    while(1) {
      got=read(fd, &rx, 1);
      if( got > 0) {
	printf("Received with ret=%d: %12lu.%06lu id=%ld len=%d msg='%s' flags=0x%x \n",
		    got, 
		    rx.timestamp.tv_sec,
		    rx.timestamp.tv_usec,
		    rx.id, rx.length, rx.data ,rx.flags );
	fflush(stdout);
      } else {
	printf("Received with ret=%d\n", got);
	fflush(stdout);
      }
      sleep(1);
    }


    close(fd);
    return 0;
}
