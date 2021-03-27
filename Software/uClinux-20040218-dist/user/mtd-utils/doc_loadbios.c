#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

/* $Id$ */
#include <linux/mtd/mtd.h>

unsigned char databuf[512];

int main(int argc,char **argv)
{
   mtd_info_t meminfo;
   int ifd,ofd;
   struct stat statbuf;
   erase_info_t erase;
   unsigned long retlen, ofs, iplsize, ipltailsize, savebuflen;
   unsigned char *iplbuf, *savebuf;

   iplbuf = NULL;
   savebuf = NULL;
   savebuflen = 0;

   if (argc < 3) {
	   fprintf(stderr,"You must specify a device,"
			" the source firmware file and the offset\n");
	   return 1;
   }
   
   // Open and size the device
   if ((ofd = open(argv[1],O_RDWR)) < 0) {
	   perror("Open flash device");
	   return 1;
   }
   
   if ((ifd = open(argv[2], O_RDONLY)) < 0) {
	   perror("Open firmware file\n");
	   close(ofd);
	   return 1;
   }
   
   if (fstat(ifd, &statbuf) != 0) {
	   perror("Stat firmware file");
	   goto error;
   }
   
#if 0
   if (statbuf.st_size > 65536) {
	   printf("Firmware too large (%ld bytes)\n",statbuf.st_size);
	   goto error;
   }
#endif   
     
   if (ioctl(ofd,MEMGETINFO,&meminfo) != 0) {
	   perror("ioctl(MEMGETINFO)");
	   goto error;
   }

   iplsize = (ipltailsize = 0);
   if (argc >= 4) {	
	   /* DoC Millennium has IPL in the first 1K of flash memory */
	   /* You may want to specify the offset 1024 to store
	      the firmware next to IPL. */
	   iplsize = strtoul(argv[3], NULL, 0);
	   ipltailsize = iplsize % meminfo.erasesize;
   }

   if ((iplsize + statbuf.st_size) % meminfo.erasesize) {
	   /*
		* preserve any partial eraseblocks - davidm@snapgear.com
		*/
	   savebuflen = meminfo.erasesize -
		   ((iplsize + statbuf.st_size) % meminfo.erasesize);
	   savebuf = (unsigned char *) malloc(savebuflen);
	   if (!savebuf) {
		   fprintf(stderr, "Not enough memory for preserve buffer of"
				  " %lu bytes\n", savebuflen);
		   goto error;
	   }
	   if (lseek(ofd, iplsize + statbuf.st_size, SEEK_SET) < 0) {
		   perror("lseek");
		   goto error;
	   }
	   if (read(ofd, savebuf, savebuflen) != savebuflen) {
		   perror("read");
		   goto error;
	   }
	   printf("Saving %lu bytes at end of last erase block\n", savebuflen);
   }

   if (lseek(ofd, iplsize - ipltailsize, SEEK_SET) < 0) {
	   perror("lseek");
	   goto error;
   }

   if (ipltailsize) {
	   iplbuf = malloc(ipltailsize);
	   if (iplbuf == NULL) {
		   fprintf(stderr, "Not enough memory for IPL tail buffer of"
			" %lu bytes\n", (unsigned long) ipltailsize);
		   goto error;
	   }
	   printf("Reading IPL%s area of length %lu at offset %lu\n",
		(iplsize - ipltailsize) ? " tail" : "",
		(long unsigned) ipltailsize,
		(long unsigned) (iplsize - ipltailsize));
	   if (read(ofd, iplbuf, ipltailsize) != ipltailsize) {
		   perror("read");
		   goto error;
	   }
   }

   erase.length = meminfo.erasesize;

   for (ofs = iplsize - ipltailsize ; 
	ofs < iplsize + statbuf.st_size ; 
	ofs += meminfo.erasesize) {
	   erase.start = ofs;
	   printf("Performing Flash Erase of length %lu at offset %lu\n",
		  (long unsigned) erase.length, (long unsigned) erase.start);
	   
	   if (ioctl(ofd,MEMERASE,&erase) != 0) {      
		   perror("ioctl(MEMERASE)");
		   goto error;
	   }
   }

   if (lseek(ofd, iplsize - ipltailsize, SEEK_SET) < 0) {
	   perror("lseek");
	   goto error;
   }

   if (ipltailsize) {
	   printf("Writing IPL%s area of length %lu at offset %lu\n",
		(iplsize - ipltailsize) ? " tail" : "",
		(long unsigned) ipltailsize,
		(long unsigned) (iplsize - ipltailsize));
	   if (write(ofd, iplbuf, ipltailsize) != ipltailsize) {
		   perror("write");
		   goto error;
	   }
   }

   ofs = 0; /* for save buffer processing */

   printf("Writing the firmware of length %lu at %lu... ", 
		(unsigned long) statbuf.st_size,
		(unsigned long) iplsize);

   do {
	   retlen = read(ifd, databuf, 512);
	   if (retlen == -1) {
		   perror("read");
		   goto error;
	   }
	   /*
		* don't want to erase the next sector, esp if we are 512 byte
		* aligned, who knows what is there ? - davidm@snapgear.com
		*/
	   if (retlen == 0)
		   break;
	   /*
		* take care of partial 512 byte sectors
		*/
	   if (retlen < 512) {
	     memset(databuf+retlen, 0xff, 512-retlen);
		 if (savebuflen) {
			 if (savebuflen < 512-retlen)
				 ofs = savebuflen;
			 else
				 ofs = 512-retlen;
			 memcpy(databuf+retlen, savebuf, ofs);
		 }
	   }
	   if (write(ofd, databuf, 512) != 512) {
		   perror("write");
		   goto error;
	   }
   } while (retlen == 512);

   while (ofs < savebuflen) {
	   retlen = 512;
	   if (savebuflen - ofs < 512)
		   retlen = savebuflen - ofs;
	   memcpy(databuf, &savebuf[ofs], retlen);
	   if (retlen < 512)
	     memset(databuf+retlen, 0xff, 512-retlen);
	   if (write(ofd, databuf, 512) != 512) {
		   perror("write");
		   goto error;
	   }
	   ofs += retlen;
   }
   // printf("end=%lu, ofs=0x%x\n", iplsize+statbuf.st_size+ofs, ofs);

   printf("Done.\n");

   if (savebuf != NULL)
	   free(savebuf);
   if (iplbuf != NULL)
	   free(iplbuf);
   close(ifd);
   close(ofd);
   return 0;

error:
   if (iplbuf != NULL)
	   free(iplbuf);
   close(ifd);
   close(ofd);
   return 1;
}
