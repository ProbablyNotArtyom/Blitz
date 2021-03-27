/****************************************************************************/

/*
 * netflash.c:  network FLASH loader.
 *
 * Copyright (C) 1999-2001,  Greg Ungerer (gerg@snapgear.com)
 * Copyright (C) 2000-2001,  Lineo (www.lineo.com)
 * Copyright (C) 2000-2002,  SnapGear (www.snapgear.com)
 *
 * Copied and hacked from rootloader.c which was:
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>

#include <linux/config.h>
#include <config/autoconf.h>
#include <linux/major.h>
#ifdef CONFIG_USER_NETFLASH_CRYPTO
#include <openssl/bio.h>
#include "crypto.h"
#endif
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
#include <linux/mtd/mtd.h>
#else
#include <linux/blkmem.h>
#endif
#ifdef CONFIG_LEDMAN
#include <linux/ledman.h>
#endif
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
#include <zlib.h>
#endif
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
#include <linux/jffs2.h>
#endif
#if defined(CONFIG_NFTL_RW) && !defined(NFTL_MAJOR)
 #define NFTL_MAJOR 93
#endif
#ifdef CONFIG_IDE
#include <linux/hdreg.h>
#endif

#include "netflash.h"
#include "exit_codes.h"
#include "versioning.h"

#ifdef CONFIG_COBRA5272
#include <ctype.h>
#endif

/****************************************************************************/

#ifdef CONFIG_USER_NETFLASH_HMACMD5
#include "hmacmd5.h"
#define HMACMD5_OPTIONS "m:"
#else
#define HMACMD5_OPTIONS
#endif

#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
#define DECOMPRESS_OPTIONS "z"
#else
#define DECOMPRESS_OPTIONS
#endif /*CONFIG_USER_NETFLASH_VERSION*/

#ifdef CONFIG_USER_NETFLASH_SETSRC
#define SETSRC_OPTIONS "I:"
#else
#define SETSRC_OPTIONS
#endif

#define CMD_LINE_OPTIONS "bc:Cd:fFhiHjkKlno:pr:stuv?" DECOMPRESS_OPTIONS HMACMD5_OPTIONS SETSRC_OPTIONS

#define DHCPCD_PID_FILE "/var/run/dhcpcd-eth0.pid"

#define CHECKSUM_LENGTH	4

/****************************************************************************/

char *version = "2.1.4";

int exitstatus = 0;

struct fileblock_t *fileblocks = NULL;
int fileblocks_num = 0;

unsigned long file_offset = 0;
unsigned long file_length = 0;
unsigned long image_length = 0;
unsigned int calc_checksum = 0;

#define	BLOCK_OVERHEAD	16
#define	block_len	(_block_len - BLOCK_OVERHEAD)
int _block_len = 8192;

int dothrow = 0;		/* Check version info of image; no program */
int dolock, dounlock;	/* do we lock/unlock segments as we go */
int checkimage;			/* Compare with current flash contents; no program */
int preserveconfig;		/* Preserve special bits of flash such as config fs */
int preserve = 0;		/* Preserve and portions of flash not written to */
int offset = 0;			/* Offset to start writing at */
int stop_early = 0;		/* stop at end of input data, do not write full dev. */
int dojffs2;			/* Write the jffs2 magic to unused segments */
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
int doinflate;			/* Decompress the image */
#endif

extern int tftpverbose;
extern int ftpverbose;

FILE	*nfd;

#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
z_stream z;
struct fileblock_t *zfb;
static int gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */
#endif

struct stat stat_rdev;

#ifdef CONFIG_USER_NETFLASH_SETSRC
char	*srcaddr = NULL;
#endif

static void (* program_segment)(int rd, char *sgdata,
		int sgpos, int sglength, int sgsize);

/****************************************************************************/

void error(const char *, ...) __attribute__ ((noreturn, format (printf, 1, 2)));
void error(const char *msg, ...) {
	va_list ap;

	printf("netflash: ");
	va_start(ap, msg);
	vprintf(msg, ap);
	va_end(ap);
	printf("\n");
	exit(1);
}

/****************************************************************************/

void restartinit(void)
{
	printf("netflash: restarting init process...\n");
	kill(1, SIGCONT);
}


#ifdef CONFIG_USER_NETFLASH_CRYPTO
#define memcpy_withsum(d, s, l, sum)	memcpy(d, s, l)
#else
static void memcpy_withsum(void *dst, void *src, int len, unsigned int *sum)
{
	unsigned char *dp = (unsigned char *)dst;
	unsigned char *sp = (unsigned char *)src;

	while(len > 0){
		*dp = *sp;
		*sum += *dp++;
		sp++;
		len--;
	}
}
#endif


/*
 *	Note: This routine is more general than it currently needs to
 *	be since it handles out of order writes.
 */
void add_data(unsigned long address, unsigned char * data, unsigned long len)
{
	int l;
	struct fileblock_t *fb;
	struct fileblock_t *fbprev = NULL;
	struct fileblock_t *fbnew;

	/*printf("add_data(%lx:%lx)\n", address, len);*/

	/* The fileblocks list is ordered, so initialise this outside
	 * the while loop to save some search time. */
	fb=fileblocks;
	do {
		/* Search for any blocks that overlap with the range we are adding */
		for (; fb!=NULL; fbprev=fb, fb=fb->next) {
			if (address < fb->pos)
				break;
			
			if (address < (fb->pos + fb->maxlength)) {
				l = fb->maxlength - (address - fb->pos);
				if (l > len)
					l = len;
				memcpy_withsum(fb->data + (address - fb->pos),
					data, l, &calc_checksum);
				fb->length = l + (address - fb->pos);
				
				address += l;
				data += l;
				len -= l;
				if (len == 0)
					return;
			}
		}
	
		printf("."); fflush(stdout);

		/* At this point:
		 * fb = block following the range we are adding,
		 *      or NULL if at end
		 * fbprev = block preceding the range, or NULL if at start
		 */
#ifdef CONFIG_USER_NETFLASH_VERSION
#ifndef CONFIG_USER_NETFLASH_CRYPTO
		if (dothrow && fileblocks_num > 2) {
			/* Steal the first block from the list. */
			fbnew = fileblocks;
			fileblocks = fbnew->next;
			if (fbprev == fbnew)
				fbprev = NULL;

			fbnew->pos = address;

			if (fb && ((fb->pos - address) < fbnew->maxlength))
				fbnew->maxlength = fb->pos - address;

		} else {
#endif
#endif /*CONFIG_USER_NETFLASH_VERSION*/

			fbnew = malloc(sizeof(*fbnew));
			if (!fbnew) {
				error("Insufficient memory for image!");
			}
			
			fbnew->pos = address;

			for (;;) {
				if (fb && ((fb->pos - address) < block_len))
					fbnew->maxlength = fb->pos - address;
				else
					fbnew->maxlength = block_len;
			
				fbnew->data = malloc(fbnew->maxlength);
				if (fbnew->data)
					break;

				/* Halve the block size and try again, down to 1 page */
				if (_block_len < 4096) {
					error("Insufficient memory for image!");
				}
				_block_len /= 2;
			}

			fileblocks_num++;

#ifdef CONFIG_USER_NETFLASH_VERSION
#ifndef CONFIG_USER_NETFLASH_CRYPTO
		}
#endif
#endif

		l = fbnew->maxlength;
		if (l > len)
			l = len;
		memcpy_withsum(fbnew->data, data, l, &calc_checksum);
		fbnew->length = l;
		address += l;
		data += l;
		len -= l;
		
		fbnew->next = fb;
		if (fbprev)
			fbprev->next = fbnew;
		else
			fileblocks = fbnew;

		/* Next search starts after the block we just added */
		fbprev = fbnew;
	} while (len > 0);
}


/*
 *	Remove bytes from the end of the data. This is used to remove
 *	checksum/versioning data before writing or decompressing.
 */
void remove_data(int length)
{
	struct fileblock_t *fb;
	struct fileblock_t *fbnext;

	if (fileblocks != NULL && file_length >= length) {
		file_length -= length;
		for (fb = fileblocks; fb != NULL; fb = fb->next) {
			if ((fb->pos + fb->length) >= file_length)
				break;
		}
		fb->length = file_length - fb->pos;
		
		while (fb->next != NULL) {
			fbnext = fb->next;
			fb->next = fbnext->next;
			free(fbnext->data);
			free(fbnext);
		}
	}
}


/*
 *	Generate a checksum over the data.
 */
void chksum()
{
	unsigned char *sp, *ep;
	unsigned int file_checksum;
	int i;
	struct fileblock_t *fb;

	file_checksum = 0;

	if (fileblocks != NULL && file_length >= CHECKSUM_LENGTH) {
#ifdef CONFIG_USER_NETFLASH_CRYPTO
		/* No on the fly check sum so we have to calculate it all here */
		for (fb = fileblocks; fb != NULL; fb = fb->next)
			for (i=0; i<fb->length; i++)
				calc_checksum += fb->data[i];
#endif
		for (fb = fileblocks; fb != NULL; fb = fb->next) {
			if ((fb->pos + fb->length) >= (file_length - CHECKSUM_LENGTH)) {
				sp = fb->data + (file_length - CHECKSUM_LENGTH - fb->pos);
				break;
			}
		}
		
		ep = fb->data + fb->length;
		for (i = 0; i < CHECKSUM_LENGTH; i++) {
			if (sp >= ep) {
				fb = fb->next;
				sp = fb->data;
				ep = sp + fb->length;
			}
			file_checksum = (file_checksum << 8) | *sp;
			calc_checksum -= *sp;
			sp++;
		}

		remove_data(CHECKSUM_LENGTH);

		calc_checksum = (calc_checksum & 0xffff) + (calc_checksum >> 16);
		calc_checksum = (calc_checksum & 0xffff) + (calc_checksum >> 16);

		if (calc_checksum != file_checksum) {
			error("bad image checksum=0x%04x, expected checksum=0x%04x",
					calc_checksum, file_checksum);
		}
	}
	else {
		error("image is too short to contain a checksum");
	}
}


#ifdef CONFIG_USER_NETFLASH_HMACMD5
int check_hmac_md5(char *key)
{
	HMACMD5_CTX ctx;
	int length, total;
	unsigned char hash[16];
	int i;
	struct fileblock_t *fb;

	if (fileblocks != NULL && file_length >= 16) {
		HMACMD5Init(&ctx, key, strlen(key));

		total = 0;
		length = 0;
		for (fb = fileblocks; fb != NULL; fb = fb->next) {
			if (fb->length > (file_length - total - 16))
				length = file_length - total - 16;
			else
				length = fb->length;

			HMACMD5Update(&ctx, fb->data, length);

			total += length;
			if (length != fb->length)
				break;
		}

		HMACMD5Final(hash, &ctx);
		for (i=0; i<16; i++, length++) {
			if (length>=fb->length) {
				length = 0;
				fb = fb->next;
			}
			if (hash[i] != fb->data[length]) {
				error("bad HMAC MD5 signature");
			}
		}
		printf("netflash: HMAC MD5 signature okay\n");

		remove_data(16);
    }
}
#endif


#ifdef CONFIG_USER_NETFLASH_CRYPTO

/* Extract bytes from end of the data.
 * These bytes are removed from the data.
 */
static inline void extract_data(int length, char buf[]) {
	unsigned long i, tpos;
	struct fileblock_t *fb;
	unsigned long target_length;

	if (fileblocks != NULL && file_length >= length) {
		target_length = file_length - length;
		for (fb = fileblocks; fb != NULL; fb = fb->next) {
			if ((fb->pos + fb->length) >= target_length)
				break;
		}
		tpos = target_length - fb->pos;
		for (i=0; i<length;) {
			if (tpos >= fb->length) {
				fb = fb->next;
				tpos = 0;
			}
			buf[i++] = fb->data[tpos++];
		}
		remove_data(length);
	} else
		error("insufficent data at end of image need %d only have %d",
				length, (int)file_length);
}

/* Grab a block at the specified position.  This could span fileblock boundaries etc so
 * we pull it out piecemeal.  This could be written significantly more efficiently
 * I expect.
 */
static inline void get_block(struct fileblock_t *fb,
		unsigned long posn, char buf[], int sz) {
	int i = 0;
	unsigned long offset = posn - fb->pos;

	while (i<sz) {
		while (offset >= fb->length)
			offset = 0, fb = fb->next;
		buf[i++] = fb->data[offset++];
	}
}

/* Write a block of information back into the file at the specified
 * position.  Again this isn't written overly efficiently.
 */
static inline struct fileblock_t *put_block(struct fileblock_t *fb,
		unsigned long posn, const char buf[], int sz) {
	int i = 0;
	unsigned long offset = posn - fb->pos;

	while (i<sz) {
		while (offset >= fb->length)
			offset = 0, fb = fb->next;
		fb->data[offset++] = buf[i++];
	}
	return fb;
}


/* Check the crypto signature on the image...
 * This always includes a public key encrypted header and an MD5
 * checksum.  It optionally includes AES encryption of the image.
 */
void check_crypto_signature(void) {
	struct fileblock_t *fb;
	int i;
  	RSA *pkey;
	struct header hdr;
	struct little_header lhdr;
	
	/* Load public key */
	{
		BIO *in;
		struct stat st;

		if (stat(PUBLIC_KEY_FILE, &st) == -1 && errno == ENOENT)
			return;
		in = BIO_new(BIO_s_file());
		if (in == NULL)
			error("cannot allocate a bio structure");
		if (BIO_read_filename(in, PUBLIC_KEY_FILE) <= 0)
			error("cannot open public key file");
		pkey = PEM_read_bio_RSA_PUBKEY(in, NULL, NULL, NULL);
		if (pkey == NULL)
			error("cannot read public key");
	}
	/* Decode header information */
	extract_data(sizeof(struct little_header), (char *)&lhdr);
	if (lhdr.magic != htons(LITTLE_CRYPTO_MAGIC)) {
#ifdef CONFIG_USER_NETFLASH_CRYPTO_OPTIONAL
		add_data(file_length, (char *)&lhdr,
				sizeof(struct little_header));
		return;
#else
		error("size magic incorrect");
#endif
	}
	{
		unsigned short hlen = ntohs(lhdr.hlen);
		char tmp[hlen];
		char t2[hlen];
		int len;

		extract_data(hlen, tmp);
		len = RSA_public_decrypt(hlen, tmp, t2,
				pkey, RSA_PKCS1_PADDING);
		if (len == -1)
			error("decrypt failed");
		if (len != sizeof(struct header)) {
			printf("Length mismatch %d %d\n", (int)sizeof(struct header), len);
		}
		memcpy(&hdr, t2, sizeof(struct header));
	}
	RSA_free(pkey);
	if (hdr.magic != htonl(CRYPTO_MAGIC))
		error("image not cryptographically enabled");
	/* Decrypt image if needed */
	if (hdr.flags & FLAG_ENCRYPTED) {
		aes_context ac;
		char cin[AES_BLOCK_SIZE];
		char cout[AES_BLOCK_SIZE];
		unsigned long s;

		if ((file_length % AES_BLOCK_SIZE) != 0)
			error("image size not miscable with cryptography");
		aes_set_key(&ac, hdr.aeskey, AESKEYSIZE, 0);
		/* Convert the body of the file */
		for (fb = fileblocks, s = 0; s<file_length; s += AES_BLOCK_SIZE) {
			get_block(fb, s, cin, AES_BLOCK_SIZE);
			aes_decrypt(&ac, cin, cout);
			fb = put_block(fb, s, cout, AES_BLOCK_SIZE);
		}
	}
	/* Remove padding */
	if (hdr.padsize) remove_data(hdr.padsize);
	/* Check MD5 sum if required */
	{
		MD5_CTX ctx;
		unsigned char hash[16];

		if (fileblocks != NULL && file_length >= 16) {
			MD5_Init(&ctx);
			for (fb = fileblocks; fb != NULL; fb = fb->next)
				MD5_Update(&ctx, fb->data, fb->length);
			MD5_Final(hash, &ctx);
			if (memcmp(hdr.md5, hash, MD5_DIGEST_LENGTH) != 0)
				error("bad MD5 signature");
		}
	}
}
#endif


#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
/* Read the decompressed size from the gzip format */
int decompress_size()
{
    unsigned char *sp, *ep;
    int	i, total, size;
    struct fileblock_t *fb;

    /* Get a pointer to the start of the inflated size */
    total = 0;
    size = 0;
    if (fileblocks != NULL && file_length >= 4) {
		for (fb = fileblocks; fb != NULL; fb = fb->next) {
			if ((total + fb->length) >= (file_length - 4)) {
				sp = fb->data + (file_length - total - 4);
				break;
			}
			total += fb->length;
		}
		
		ep = fb->data + fb->length;
		for (i = 0; i < 4; i++) {
			if (sp >= ep) {
				fb = fb->next;
				sp = fb->data;
				ep = sp + fb->length;
			}
			size |= (*sp++) << (8*i);
		}
    }

    return size;
}


/*
 *	Skip bytes, ensuring at least 1 byte remains to be read.
 *	Don't use this to skip past the last byte in the file.
 */
int decompress_skip_bytes(int pos, int num)
{
    while (zfb) {
		if (pos + num < zfb->length)
			return pos + num;
		
		num -= zfb->length - pos;
		pos = 0;
		zfb = zfb->next;
    }
    error("compressed image is too short");
}


int decompress_init()
{
    int pos, flg, xlen, size;

    zfb = fileblocks;
    pos = 0;
	
    /* Skip over gzip header */
    pos = decompress_skip_bytes(pos, 2);
	
    if (zfb->data[pos] != 8) {
		error("image is compressed, unknown compression method");
    }
    pos = decompress_skip_bytes(pos, 1);
	
    flg = zfb->data[pos];
    pos = decompress_skip_bytes(pos, 1);
	
    /* Skip mod time, extended flag, and os */
    pos = decompress_skip_bytes(pos, 6);
	
    /* Skip extra field */
    if (flg & 0x04) {
		xlen = zfb->data[pos];
		pos = decompress_skip_bytes(pos, 1);
		xlen += zfb->data[pos]<<8;
		pos = decompress_skip_bytes(pos, 1+xlen);
    }
	
    /* Skip file name */
    if (flg & 0x08) {
		while (zfb->data[pos])
			pos = decompress_skip_bytes(pos, 1);
		pos = decompress_skip_bytes(pos, 1);
    }
	
    /* Skip comment */
    if (flg & 0x10) {
		while (zfb->data[pos])
			pos = decompress_skip_bytes(pos, 1);
		pos = decompress_skip_bytes(pos, 1);
    }
	
    /* Skip CRC */
    if (flg & 0x02) {
		pos = decompress_skip_bytes(pos, 2);
    }
	
    z.next_in = zfb->data + pos;
    z.avail_in = zfb->length - pos;
    z.zalloc = Z_NULL;
    z.zfree = Z_NULL;
    z.opaque = Z_NULL;
    if (inflateInit2(&z, -MAX_WBITS) != Z_OK) {
		error("image is compressed, decompression failed");
    }
    
    size = decompress_size();
    if (size <= 0) {
		error("image is compressed, decompressed length is invalid");
    }

    return size;
}


int decompress(char* data, int length)
{
    int rc;

    z.next_out = data;
    z.avail_out = length;

    for (;;) {
		while (z.avail_in == 0) {
			zfb = zfb->next;
			if (!zfb) {
				error("unexpected end of file for decompression");
			}
			z.next_in = zfb->data;
			z.avail_in = zfb->length;
		}

		rc = inflate(&z, Z_SYNC_FLUSH);
		
		if (rc == Z_OK) {
			if (z.avail_out == 0)
				return length;
			
			if (z.avail_in != 0) {
				/* Note: This shouldn't happen, but if it does then
				 * need to add code to add another level of buffering
				 * that we append file blocks to...
				 */
				error("decompression deadlock");
			}
		}
		else if (rc == Z_STREAM_END) {
			return length - z.avail_out;
		}
		else {
			error("error during decompression: %x", rc);
		}
    }
}

int check_decompression(int doinflate)
{
	if (doinflate) {
		if (fileblocks->length >= 2
				&& fileblocks->data[0] == gz_magic[0]
				&& fileblocks->data[1] == gz_magic[1]) {
			image_length = decompress_init();
			printf("netflash: image is compressed, decompressed length=%ld\n",
					image_length);
		} else {
			error("image is not compressed");
		}
	}
#ifdef CONFIG_USER_NETFLASH_AUTODECOMPRESS
	else if (fileblocks->length >= 2
			&& fileblocks->data[0] == gz_magic[0]
			&& fileblocks->data[1] == gz_magic[1]) {
		doinflate = 1;
		image_length = decompress_init();
		printf("netflash: image is compressed, decompressed length=%ld\n",
				image_length);
	}
#endif
	else {
		image_length = file_length;
	}

	return doinflate;
}
#endif

/****************************************************************************/

/*
 *	Local copies of the open/close/write used by tftp loader.
 *	The idea is that we get tftp to do all the work of getting
 *	the file over the network. The following code back ends
 *	that process, preparing the read data for FLASH programming.
 */
int local_creat(char *name, int flags)
{
	return(fileno(nfd));
}

FILE *local_fdopen(int fd, char *flags)
{
	return(nfd);
}

FILE *local_fopen(const char *name, const char *flags)
{
	return(nfd);
}

int local_fclose(FILE *fp)
{
	printf("\n");
	fflush(stdout);
	sleep(1);
	return(0);
}

int local_fseek(FILE *fp, int offset, int whence)
{
	/* Shouldn't happen... */
	return(0);
}

int local_putc(int ch, FILE *fp)
{
	/* Shouldn't happen... */
	return(0);
}

int local_write(int fd, char *buf, int count)
{
  add_data(file_offset, buf, count);
  file_offset += count;
  if (file_offset > file_length)
	  file_length = file_offset;
  return(count);
}

/****************************************************************************/
 
/*
 * Call to tftp. This will initialize tftp and do a get operation.
 * This will call the local_write() routine with the data that is
 * fetched, and it will create the ioctl structure.
 */
int tftpfetch(char *srvname, char *filename)
{
  char	*tftpargv[8];
  int	tftpmainargc = 0;

  tftpverbose = 0;	/* Set to 1 for tftp trace info */

  tftpargv[tftpmainargc++] = "tftp";
  tftpargv[tftpmainargc++] = srvname;
#ifdef CONFIG_USER_NETFLASH_SETSRC
  if (srcaddr != NULL)
	tftpargv[tftpmainargc++] = srcaddr;
#endif
  tftpmain(tftpmainargc, tftpargv);
  tftpsetbinary();
  
  printf("netflash: fetching file \"%s\" from %s\n", filename, srvname);
  tftpargv[0] = "get";
  tftpargv[1] = filename;
  tftpget(2, tftpargv);
  return(0);
}

/****************************************************************************/
 
/*
 * Call to ftp. This will initialize ftp and do a get operation.
 * This will call the local_write() routine with the data that is
 * fetched, and it will create the ioctl structure.
 */
int ftpconnect(char *srvname)
{
  char	*ftpargv[4];

#ifdef FTP
  ftpverbose = 0;	/* Set to 1 for ftp trace info */
  printf("netflash: login to remote host %s\n", srvname);

  ftpargv[0] = "ftp";
  ftpargv[1] = srvname;
  ftpmain(2, ftpargv);
  return(0);

#else
  printf("netflash: no ftp support builtin\n");
  return(-1);
#endif /* FTP */
}

int ftpfetch(char *srvname, char *filename)
{
  char	*ftpargv[4];

#ifdef FTP
  ftpverbose = 0;	/* Set to 1 for ftp trace info */
  printf("\nnetflash: ftping file \"%s\" from %s\n", filename, srvname);
  setbinary(); /* make sure we are in binary mode */

  ftpargv[0] = "get";
  ftpargv[1] = filename;
  get(2, ftpargv);

  quit();
  return(0);

#else
  printf("NETFLASH: no ftp support builtin\n");
  return(-1);
#endif /* FTP */
}

/****************************************************************************/

/*
 *	When fetching file we need to even number of bytes in write
 *	buffers. Otherwise FLASH programming will fail. This is mostly
 *	only a problem with http for some reason.
 */

int filefetch(char *filename)
{
  int fd, i, j;
  unsigned char buf[1024];

  if (strncmp(filename, "http://", 7) == 0)
    fd = openhttp(filename);
  else
    fd = open(filename, O_RDONLY);

  if (fd < 0)
    return(-1);

  for (;;) {
    printf(".");
    if ((i = read(fd, buf, sizeof(buf))) <= 0)
      break;
    if (i & 0x1) {
	/* Read more to make even sized buffer */
	if ((j = read(fd, &buf[i], 1)) > 0)
		i += j;
    }
    add_data(file_offset, buf, i);
	file_offset += i;
	file_length = file_offset;
  }

  close(fd);
  printf("\n");
  return(0);
}

/****************************************************************************/

int samedev(struct stat *stat_dev, struct stat *stat_rootfs)
{
	if (S_ISBLK(stat_dev->st_mode)) {
		if (stat_dev->st_rdev == stat_rootfs->st_dev) {
			return 1;
		}
	}
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
	/* Check for matching block/character mtd devices. */
	else if (S_ISCHR(stat_dev->st_mode)) {
		if (major(stat_dev->st_rdev) == MTD_CHAR_MAJOR
				&& major(stat_rootfs->st_dev) == MTD_BLOCK_MAJOR
				&& (minor(stat_dev->st_rdev) >> 1)
					== minor(stat_rootfs->st_dev)) {
			return 1;
		}
	}
#endif
	return 0;
}

/*
 *	Check if we are writing to the root filesystem.
 */
int flashing_rootfs(char *rdev)
{
	static struct stat stat_rootfs, stat_flash;

	/* First a generic check:
	 * is the rootfs device the same as the flash device?
	 */
	if (stat("/", &stat_rootfs) != 0)
		error("stat(\"/\") failed (errno=%d)", errno);
	if (samedev(&stat_rdev, &stat_rootfs))
		return 1;

	/* Secondly, a platform specific check:
	 * /dev/flash/all and /dev/flash/image and /dev/flash/rootfs
	 * can overlap, check if we are writing to any of these, and the
	 * root device is /dev/flash/image or /dev/flash/rootfs.
	 * XXX: checking device numbers would be better than strcmp */
	else if (!strcmp(rdev, "/dev/flash/all")
			|| !strcmp(rdev, "/dev/flash/image")
			|| !strcmp(rdev, "/dev/flash/rootfs")) {
		if (stat("/dev/flash/image", &stat_flash) == 0
				&& samedev(&stat_flash, &stat_rootfs))
			return 1;
		if (stat("/dev/flash/rootfs", &stat_flash) == 0
				&& samedev(&stat_flash, &stat_rootfs))
			return 1;
	}
	return 0;
}

/****************************************************************************/

/*
 *	Search for a process and send a signal to it.
 */
int killprocname(const char *name, int signo)
{
	DIR *dir;
	struct dirent *entry;
	FILE *f;
	char path[32];
	char line[64];
	int ret = 0;

	dir = opendir("/proc");
	if (!dir)
		return 0;

	while ((entry = readdir(dir)) != NULL) {
		if (!isdigit(*entry->d_name))
			continue;

		sprintf(path, "/proc/%s/status", entry->d_name);
		if ((f = fopen(path, "r")) == NULL)
			continue;

		while (fgets(line, sizeof(line), f) != NULL) {
			if (line[strlen(line)-1] == '\n') {
				line[strlen(line)-1] = '\0';
				if (strncmp(line, "Name:\t", 6) == 0
						&& strcmp(line+6, name) == 0) {
					kill(atoi(entry->d_name), signo);
					ret = 1;
				}
			}
		}

		fclose(f);
	}
	closedir(dir);
	return ret;
}

/****************************************************************************/

/*
 *  Read a process pid file and send a signal to it.
 */
void killprocpid(char *file, int signo)
{
    FILE* f;
    pid_t pid;
	char value[16];

    f = fopen(file, "r");
    if (f == NULL)
        return;

    if (fread(value, 1, sizeof(value), f) > 0) {
		pid = atoi(value);
		if (pid)
			kill(pid, signo);
        unlink(file);
    }
    fclose(f);
}

/****************************************************************************/

/*
 *	Find the current console device. We output trace to this device
 *	if it is the controlling tty at process start.
 */
char	*consolelist[] = {
	"/dev/console",
	"/dev/ttyS0",
	"/dev/cua0",
	"/dev/ttyS1",
	"/dev/cua1",
	"/dev/ttyAM0"
};

#define	clistsize	(sizeof(consolelist) / sizeof(char *))
 
char *getconsole(void)
{
	struct stat	myst, st;
	int		i;

	if (fstat(0, &myst) < 0)
		return((char *) NULL);

	for (i = 0; (i < clistsize); i++) {
		if (!stat(consolelist[i], &st) && 
				(myst.st_rdev == st.st_rdev))
			return(consolelist[i]);
	}
	return "/dev/null";
}

/****************************************************************************/

static const char *kill_partial[] = {
#ifdef CONFIG_USER_SNORT_SNORT
	"snort",
#endif
#ifdef CONFIG_USER_SQUID_SQUID
	"squid",
#endif
	NULL
};

/*
 * Kill off processes to reclaim some memory.  Only kills processes
 * that we know are unnecessary for obtaining the image.
 */
void kill_processes_partial(void)
{
	const char **p;
	int count;

	printf("netflash: killing unnecessary tasks...\n");
	fflush(stdout);
	sleep(1);

	kill(1, SIGTSTP);		/* Stop init from reforking tasks */
	atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	/* Ask them nicely. */
	count = 0;
	for (p = kill_partial; *p; p++)
		count += killprocname(*p, SIGTERM);
	if (count)
		sleep(5);		/* give em a moment... */

	/* Time for the no-nonsense approach. */
	count = 0;
	for (p = kill_partial; *p; p++)
		count += killprocname(*p, SIGKILL);
	if (count)
		sleep(2);		/* give em another moment... */
}


/*
 * Kill of processes now to reclaim some memory. Need this now so
 * we can buffer an entire firmware image...
 */
void kill_processes(char *console)
{
	int ttyfd;
	struct termios tio;

	if (console == NULL)
		console = getconsole();

	ttyfd = open(console, O_RDWR|O_NDELAY|O_NOCTTY);
	if (ttyfd >= 0) {
		if (tcgetattr(ttyfd, &tio) >= 0) {
			tio.c_cflag |= CLOCAL;
			tcsetattr(ttyfd, TCSAFLUSH, &tio);
		}
		close(ttyfd);
	}
	freopen(console, "w", stdout);
	freopen(console, "w", stderr);
	
	printf("netflash: killing tasks...\n");
	fflush(stdout);
	sleep(1);
	
	kill(1, SIGTSTP);		/* Stop init from reforking tasks */
	atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	signal(SIGTERM,SIG_IGN);	/* Don't kill ourselves... */
	setpgrp(); 			/* Don't let our parent kill us */
	sleep(1);
	signal(SIGHUP, SIG_IGN);	/* Don't die if our parent dies due to
					 * a closed controlling terminal */
	
	/*Don't take down network interfaces that use dhcpcd*/
	killprocpid(DHCPCD_PID_FILE, SIGKILL);

	kill(-1, SIGTERM);		/* Kill everything that'll die */
	sleep(5);			/* give em a moment... (it may take a while for, e.g., pppd to shutdown cleanly */
	kill(-1, SIGKILL);		/* Really make sure that everything is dead */
	sleep(2);			/* give em another moment... */

	if (console)
		freopen(console, "w", stdout);
}

/****************************************************************************/

#if defined(CONFIG_USER_MOUNT_UMOUNT) || defined(CONFIG_USER_BUSYBOX_UMOUNT)
void umount_all(void)
{
	char *localargv[4];
	int localargc;
	pid_t pid;
	int status;

	localargc = 0;
	localargv[localargc++] = "umount";
	localargv[localargc++] = "-a";
	localargv[localargc++] = "-r";
	localargv[localargc++] = NULL;
	pid = vfork();
	if (pid < 0) {
		error("vfork() failed");
	}
	else if (pid == 0) {
		execvp("/bin/umount", localargv);
		_exit(1);
	}
	waitpid(pid, &status, 0);
}
#endif


/****************************************************************************/

static int get_segment(int rd, char *sgdata, int sgpos, int sgsize)
{
	int sglength;
	int sgoffset;

	if (offset > sgpos)
		sgoffset = offset - sgpos;
	else
		sgoffset = 0;

	/* XXX: preserve case could be optimized to read less */
	if (preserve || sgoffset) {
		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			printf("netflash: lseek(%x) failed\n", sgpos);
			exit(1);
		} else if (read(rd, sgdata, preserve ? sgsize : sgoffset) < 0) {
			printf("netflash: read() failed, pos=%x, errno=%d\n",
					sgpos, errno);
			exit(1);
		}
	}

	sgpos -= offset - sgoffset;
	sgdata += sgoffset;
	sgsize -= sgoffset;

#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	if (doinflate) {
		sglength = decompress(sgdata, sgsize);
	} else {
#endif
		/* Copy the file blocks into the segment buffer */
		sglength = 0;
		while (fileblocks && (fileblocks->pos < sgpos + sgsize)) {

			if (fileblocks->pos + fileblocks->length > sgpos + sgsize)
				sglength = sgsize;
			else
				sglength = fileblocks->pos + fileblocks->length - sgpos;
			  
			if (fileblocks->pos < sgpos) {
				memcpy(sgdata, fileblocks->data + (sgpos - fileblocks->pos),
						sglength);
			} else {
				memcpy(sgdata + (fileblocks->pos - sgpos), fileblocks->data,
						sglength - (fileblocks->pos - sgpos));
			}

			if (fileblocks->pos + fileblocks->length > sgpos + sgsize) {
				/* Need to keep fileblocks pointing to this block
				 * for the start of the next segment. */
				break;
			}

			/* Modify the global: this is an optimization to save searching
			 * through blocks that have been programmed already. */
			fileblocks = fileblocks->next;
		}
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	}
#endif

	if (sglength !=0) {
		if (preserve)
			sglength = sgsize;
		sglength += sgoffset;
	}

	return sglength;
}

static void check_segment(int rd, char *sgdata, int sgpos, int sglength,
		char *check_buf)
{
	if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
		printf("netflash: lseek(%x) failed\n", sgpos);
		exitstatus = 1;
	} else if (read(rd, check_buf, sglength) < 0) {
		printf("netflash: read failed, pos=%x, errno=%d\n",
				sgpos, errno);
		exitstatus = 1;
	} else if (memcmp(sgdata, check_buf, sglength) != 0) {
		int i;
		printf("netflash: check failed, pos=%x\n", sgpos);
		for (i = 0; i < sglength; i++) {
			if (sgdata[i] != check_buf[i])
				printf("%x(%x,%x) ", sgpos + i,
						sgdata[i] & 0xff,
						check_buf[i] & 0xff);
		}
		printf("\n");
		exitstatus = 1;
	}
}

#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
static void program_mtd_segment(int rd, char *sgdata,
		int sgpos, int sglength, int sgsize)
{
	erase_info_t erase_info;
	int len, pos;

	/* Unlock the segment to be reprogrammed.  */
	if (dounlock) {
		erase_info.start = sgpos;
		erase_info.length = sgsize;
		/* Don't bother checking for failure */
		ioctl(rd, MEMUNLOCK, &erase_info);
	}

	erase_info.start = sgpos;
	erase_info.length = sgsize;
	if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
		printf("netflash: lseek(%x) failed\n", sgpos);
		exitstatus = 1;
	} else if (ioctl(rd, MEMERASE, &erase_info) < 0) {
		printf("netflash: ioctl(MEMERASE) failed, errno=%d\n", errno);
		exitstatus = 1;
	} else if (sglength > 0) {
		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			printf("netflash: lseek(%x) failed\n", sgpos);
			exitstatus = 1;
		} else {
			/*
			 * Always write in 512 byte chunks as MTD on
			 * a DoC device can only handle 512 byte writes.
			 *
			 * NOTE: we rely on the fact the sgdata buffer is always
			 *       a multiple of 512 in real size (erase_size for MTD)
			 *       which should always be true, I think.
			 */
			for (pos = sgpos; (sglength >= 512); ) {
				if (write(rd, sgdata, 512) == -1) {
					printf("netflash: write() failed, "
						"pos=%x, errno=%d\n",
						pos, errno);
					exitstatus = 1;
				}
				pos += 512;
				sgdata += 512;
				sglength -= 512;
			}
			/*
			 * If there is a remainder, then still write a 512 byte
			 * chunk, but preserve what is already there.
			 */
			if (sglength > 0) {
				char buf[512];

				if (lseek(rd, pos, SEEK_SET) != pos) {
					printf("netflash: lseek(%x) failed\n",
						pos);
					exitstatus = 1;
				} else if (read(rd, buf, 512) == -1) {
					printf("netflash: read() failed, "
						"pos=%x, errno=%d\n",
						pos, errno);
					exitstatus = 1;
				} else if (lseek(rd, pos, SEEK_SET) != pos) {
					printf("netflash: lseek(%x) failed\n",
						pos);
					exitstatus = 1;
				} else {
					memcpy(buf, sgdata, sglength);
					if (write(rd, buf, 512) == -1) {
						printf("netflash: write() failed, pos=%x, errno=%d\n",
							pos, errno);
						exitstatus = 1;
					}
				}
			}
		}
	} else if (dojffs2) {
		static struct jffs2_unknown_node marker = {
			JFFS2_MAGIC_BITMASK,
			JFFS2_NODETYPE_CLEANMARKER,
			sizeof(struct jffs2_unknown_node),
			0xe41eb0b1
		};

		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			printf("netflash: lseek(%x) failed\n", sgpos);
			exitstatus = 1;
		} else if (write(rd, &marker, sizeof(marker)) < 0) {
			printf("netflash: write() failed, pos=%x, "
				"errno=%d\n", sgpos, errno);
			exitstatus = 1;
		}
	}

	if (dolock) {
		erase_info.start = sgpos;
		erase_info.length = sgsize;
		if (ioctl(rd, MEMLOCK, &erase_info) < 0) {
			printf("netflash: ioctl(MEMLOCK) failed, "
				"errno=%d\n", errno);
			exitstatus = 1;
		}
	}
}
#else
static void program_blkmem_segment(int rd, char *sgdata, int sgpos,
	int sglength, int sgsize)
{
	char buf[128];
	struct blkmem_program_t *prog = (struct blkmem_program_t *)buf;

	prog->magic1 = BMPROGRAM_MAGIC_1;
	prog->magic2 = BMPROGRAM_MAGIC_2;
	prog->reset = 0;
	prog->blocks = 1;
	prog->block[0].data = sgdata;
	prog->block[0].pos = sgpos;
	prog->block[0].length = sglength;
	prog->block[0].magic3 = BMPROGRAM_MAGIC_3;
	if (ioctl(rd, BMPROGRAM, prog) != 0) {
		printf("netflash: ioctl(BMPROGRAM) failed, errno=%d\n", errno);
		exitstatus = 1;
	}
}
#endif

#if defined(CONFIG_NFTL_RW) || defined(CONFIG_IDE)
static void program_generic_segment(int rd, char *sgdata,
		int sgpos, int sglength, int sgsize)
{
	if (sglength > 0) {
		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			printf("netflash: lseek(%x) failed\n", sgpos);
			exitstatus = 1;
		} else if (write(rd, sgdata, sglength) < 0) {
			printf("netflash: write() failed, pos=%x, "
					"errno=%d\n", sgpos, errno);
			exitstatus = 1;
		} else if (fdatasync(rd) < 0) {
			printf("netflash: fdatasync() failed, pos=%x, "
					"errno=%d\n", sgpos, errno);
			exitstatus = 1;
		}
	}
}
#endif

static void program_flash(int rd, int devsize, char *sgdata, int sgsize,
		char *check_buf)
{
	int sgpos, sglength;
#ifdef CONFIG_LEDMAN
	int ledmancount = 0;
#endif

#ifdef CONFIG_LEDMAN
	ledman_cmd(LEDMAN_CMD_ALT_ON, LEDMAN_NVRAM_1);
	ledman_cmd(LEDMAN_CMD_ALT_ON, LEDMAN_NVRAM_2);
#endif

	/* Write the data one segment at a time */
	sgpos = offset - (offset % sgsize);
	for (; sgpos < devsize; sgpos += sgsize) {
		sglength = get_segment(rd, sgdata, sgpos, sgsize);

		if (stop_early && sglength <= 0)
			break;

		if (checkimage) {
			check_segment(rd, sgdata, sgpos, sglength, check_buf);
		}
		else
#if defined(CONFIG_NETtel) && defined(CONFIG_X86)
		if (!preserveconfig || sgpos < 0xe0000 || sgpos >= 0x100000) {
#endif
			program_segment(rd, sgdata, sgpos, sglength, sgsize);

			printf("."); fflush(stdout);
#ifdef CONFIG_LEDMAN
			ledman_cmd(LEDMAN_CMD_OFF | LEDMAN_CMD_ALTBIT,
					ledmancount ? LEDMAN_NVRAM_1 : LEDMAN_NVRAM_2);
			ledman_cmd(LEDMAN_CMD_ON | LEDMAN_CMD_ALTBIT,
					ledmancount ? LEDMAN_NVRAM_2 : LEDMAN_NVRAM_1);
			ledmancount = (ledmancount + 1) & 1;
#endif

#if defined(CONFIG_NETtel) && defined(CONFIG_X86)
		} /* if (!preserveconfig || ...) */
#endif
	}

	printf("\n"); fflush(stdout);
#ifdef CONFIG_LEDMAN
	ledman_cmd(LEDMAN_CMD_ALT_OFF, LEDMAN_NVRAM_1);
	ledman_cmd(LEDMAN_CMD_ALT_OFF, LEDMAN_NVRAM_2);
#endif

	/* Put the flash back in read mode, some old boot loaders don't */
	lseek(rd, 0, SEEK_SET);
	read(rd, sgdata, 1);
}

/****************************************************************************/

int usage(int rc)
{
  printf("usage: netflash [-bCfFhijklntuv"
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	"z"
#endif
	"?] [-c <console-device>] [-d <delay>] "
#ifdef CONFIG_USER_NETFLASH_SETSRC
	"[-I <ip-address>] "
#endif
#ifdef CONFIG_USER_NETFLASH_HMACMD5
	"[-m <hmac-md5-key>] "
#endif
	"[-o <offset>] [-r <flash-device>] "
	"[<net-server>] <file-name>\n\n"
	"\t-b\tdon't reboot hardware when done\n"
	"\t-C\tcheck that image was written correctly\n"
	"\t-f\tuse FTP as load protocol\n"
	"\t-F\tforce overwrite (do not preserve special regions)\n"
  	"\t-h\tprint help\n"
	"\t-i\tignore any version information\n"
	"\t-H\tignore hardware type information\n"
#ifdef CONFIG_USER_NETFLASH_SETSRC
	"\t-I\toriginate TFTP request from this address\n"
#endif
	"\t-j\timage is a JFFS2 filesystem\n"
	"\t-k\tdon't kill other processes (or delays kill until\n"
	"\t\tafter downloading when root filesystem is inside flash)\n"
	"\t-K\tonly kill unnecessary processes (or delays kill until\n"
	"\t\tafter downloading when root filesystem is inside flash)\n"
	"\t-l\tlock flash segments when done\n"
  	"\t-n\tfile with no checksum at end (implies no version information)\n"
	"\t-p\tpreserve portions of flash segments not actually written.\n"
  	"\t-s\tstop erasing/programming at end of input data\n"
	"\t-t\tcheck the image and then throw it away \n"
	"\t-u\tunlock flash segments before programming\n"
  	"\t-v\tdisplay version number\n"
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	"\t-z\tdecompress image before writing\n"
#endif
  );

  exit(rc);
}

/****************************************************************************/

int netflashmain(int argc, char *argv[])
{
	char *srvname, *filename;
	char *rdev, *console;
	char *sgdata, *check_buf = NULL;
	int rd, rc, tmpfd;
	int delay;
	int dochecksum, dokill, dokillpartial, doreboot, doftp;
	int dopreserve, doversion, doremoveversion, dohardwareversion;
	int devsize, sgsize;
	struct fileblock_t *fb;

#ifdef CONFIG_USER_NETFLASH_HMACMD5
	char *hmacmd5key = NULL;
#endif

	rdev = "/dev/flash/image";
	srvname = NULL;
	filename = NULL;
	console = NULL;
	dochecksum = 1;
	dokill = 1;
	dokillpartial = 0;
	doreboot = 1;
	dolock = 0;
	dounlock = 0;
	delay = 0;
	doftp = 0;
	dothrow = 0;
	dopreserve = 1;
	preserveconfig = 0;
	checkimage = 0;
	dojffs2 = 0;
	doremoveversion = 1;

#ifdef CONFIG_USER_NETFLASH_VERSION
	doversion = 1;
	dohardwareversion = 1;
#else
	doversion = 0;
	dohardwareversion = 0;
#endif /*CONFIG_USER_NETFLASH_VERSION*/
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	doinflate = 0;
#endif

	while ((rc = getopt(argc, argv, CMD_LINE_OPTIONS)) > 0) {
		switch (rc) {
		case 'p':
			preserve = 1;
			stop_early = 1;
			break;
		case 's':
			stop_early = 1;
			break;
		case 'b':
			doreboot = 0;
			break;
		case 'c':
			console = optarg;
			break;
		case 'C':
			checkimage = 1;
			break;
		case 'd':
			delay = (atoi(optarg));
			break;
		case 'f':
			doftp = 1;
			break;
		case 'F':
			dopreserve = 0;
			break;
		case 'i': 
			doversion = 0; 
			break;
		case 'H': 
			dohardwareversion = 0; 
			break;
		case 'j':
			dojffs2 = 1;
			break;
		case 'k':
			dokill = 0;
			break;
		case 'K':
			dokill = 1;
			dokillpartial = 1;
			break;
		case 'l':
			dolock++;
			break;
#ifdef CONFIG_USER_NETFLASH_HMACMD5
		case 'm':
			hmacmd5key = optarg;
			break;
#endif
		case 'n':
			/* No checksum implies no version */
			dochecksum = doversion = dohardwareversion = doremoveversion = 0;
			break;
		case 'o':
			offset = strtol(optarg, NULL, 0);
			break;
		case 'r':
			rdev = optarg;
			break;
		case 't':
			dothrow = 1;
			break;
		case 'u':
			dounlock++;
			break;
		case 'v':
			printf("netflash: version %s\n", version);
			exit(0);
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
		case 'z':
			doinflate = 1;
			break;
#endif
#ifdef CONFIG_USER_NETFLASH_SETSRC
		case 'I':
			srcaddr = optarg;
			break;
#endif
		case 'h':
		case '?':
			usage(0);
			break;
		}
	}
  
	if ((nfd = fopen("/dev/null", "rw")) == NULL) {
		error("failed to open(/dev/null)");
	}

	if (optind == (argc - 1)) {
		srvname = NULL;
		filename = argv[optind];
	} else if (optind == (argc - 2)) {
		srvname = argv[optind++];
		filename = argv[optind];
	} else {
		usage(1);
	}

	if (delay > 0) {
		/* sleep the required time */
		printf("netflash: waiting %d seconds before updating "
			"flash...\n",delay);
		sleep(delay);
	}

	/*
	 *	Need to do any real FTP setup early, before killing processes
	 *	(and this losing association with the controlling tty).
	 */
	if (doftp) {
		if (ftpconnect(srvname))
			exit(1);
	}

	if (dokill) {
		if (dokillpartial)
			kill_processes_partial();
		else
			kill_processes(console);
	}

	/*
	 * Open the flash device and allocate a segment sized block.
	 * This is the largest block we need to allocate, so we do
	 * it first to try to avoid fragmentation effects.
	 */
	if (dopreserve && (strcmp(rdev, "/dev/flash/image") == 0))
		preserveconfig = 1;

	rd = open(rdev, O_RDWR);
	if (rd < 0) {
		error("open(%s)=%d failed (errno=%d)",
			rdev, rd, errno);
	}
  
	if (stat(rdev, &stat_rdev) != 0) {
		error("stat(%s) failed (errno=%d)", rdev, errno);
	} else if (S_ISBLK(stat_rdev.st_mode)) {
#ifdef CONFIG_NFTL_RW
		if (major(stat_rdev.st_rdev) == NFTL_MAJOR) {
			unsigned long l;

			program_segment = program_generic_segment;
			preserveconfig = dolock = dounlock = 0;
			if (ioctl(rd, BLKGETSIZE, &l) < 0) {
				error("ioctl(BLKGETSIZE) failed, errno=%d",
						errno);
			}
			devsize = l*512; /* Sectors are always 512 bytes */
			sgsize = 64*1024; /* Just a random power of 2 */
		}
#endif
#ifdef CONFIG_IDE
		if (major(stat_rdev.st_rdev) == IDE0_MAJOR) {
			struct hd_geometry geo;

			program_segment = program_generic_segment;
			preserveconfig = dolock = dounlock = 0;
			if (ioctl(rd, HDIO_GETGEO, &geo) < 0) {
				error("ioctl(HDIO_GETGEO) failed, errno=%d",
						errno);
			}
			devsize = geo.heads*geo.cylinders*geo.sectors*512;
			sgsize = 64*1024; /* Just a random power of 2 */
		}
#endif
	}
	if (!program_segment) {
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
		mtd_info_t mtd_info, rootfs_info;

		program_segment = program_mtd_segment;

		if (ioctl(rd, MEMGETINFO, &mtd_info) < 0) {
			error("ioctl(MEMGETINFO) failed, errno=%d", errno);
		}
		devsize = mtd_info.size;
		sgsize = mtd_info.erasesize;

		/*
		 * NETtel/x86 boards that boot direct from INTEL FLASH also have a
		 * boot sector at the top of the FLASH. When programming complete
		 * images we need to not overwrite this.
		 */
		if (preserveconfig) {
			if ((tmpfd = open("/dev/flash/rootfs", O_RDONLY)) > 0) {
				if (ioctl(tmpfd, MEMGETINFO, &rootfs_info) >= 0) {
					if (rootfs_info.size & 0x000fffff) {
						devsize = devsize - (0x00100000 -
								(rootfs_info.size & 0x000fffff));
					}
				}
				close(tmpfd);
			}
		}
#else
		program_segment = program_blkmem_segment;

		if (ioctl(rd, BMGETSIZEB, &devsize) != 0) {
			error("ioctl(BMGETSIZEB) failed, errno=%d", errno);
		}
		if (ioctl(rd, BMSGSIZE, &sgsize) != 0) {
			error("ioctl(BMSGSIZE) failed, errno=%d", errno);
		}
#endif
	}
  
	if (offset < 0) {
		error("offset is less than zero");
	}
	if (offset >= devsize) {
		error("offset is greater than device size (%d)", devsize);
	}

	sgdata = malloc(sgsize);
	if (!sgdata) {
		error("Insufficient memory for image!");
	}

	if (checkimage) {
		check_buf = malloc(sgsize);
		if (!check_buf) {
			error("Insufficient memory for image!");
		}
	}

	/*
	 * Fetch file into memory buffers. Exactly how depends on the exact
	 * load method. Support for tftp, http and local file currently.
	 */
	fileblocks = NULL;
	file_offset = 0;
	file_length = 0;

	if (srvname) {
		if (doftp)
			ftpfetch(srvname, filename);
		else
			tftpfetch(srvname, filename);
	} else if (filefetch(filename) < 0)
			error("failed to find %s", filename);

	/*
	 * Do some checks on the data received
	 *    - starts at offset 0
	 *    - length > 0
	 *    - no holes
	 *    - checksum
	 */
	if (fileblocks == NULL) {
		error("failed to load new image");
	}

#ifndef CONFIG_USER_NETFLASH_CRYPTO
	if (!dothrow)
#endif
		if (fileblocks->pos != 0)
			error("failed to load new image");

	if (file_length == 0)
		error("failed to load new image");

	for (fb = fileblocks; fb->next != NULL; fb = fb->next)
		if (fb->pos + fb->length != fb->next->pos)
			error("failed to load new image");

	printf("netflash: got \"%s\", length=%ld\n", filename, file_length);

#ifdef CONFIG_USER_NETFLASH_CRYPTO
	check_crypto_signature();
#endif

#ifdef CONFIG_USER_NETFLASH_HMACMD5
	if (hmacmd5key)
		check_hmac_md5(hmacmd5key);
	else
#endif
	if (dochecksum)
		chksum();

	/*
	 * Check the version information.
	 * Side effect: this also checks whether version information is present,
	 * and if so, removes it, since it doesn't need to get written to flash.
	 */
	if (doversion || doremoveversion)
		rc = check_vendor(vendor_name, product_name, image_version);

#ifdef CONFIG_USER_NETFLASH_VERSION
	if (doversion) {
		switch (rc){
#ifndef CONFIG_USER_NETFLASH_VERSION_ALLOW_CURRENT
		case 3:
			printf("netflash: VERSION - you are trying to upgrade "
				"with the same firmware\n"
				"         version that you already have.\n");
			exit(ALREADY_CURRENT);
#endif /* !CONFIG_USER_NETFLASH_VERSION_ALLOW_CURRENT */
#ifndef CONFIG_USER_NETFLASH_VERSION_ALLOW_OLDER
		case 4:
			printf("netflash: VERSION - you are trying to upgrade "
				"with an older version of\n"
				"         the firmware.\n");
			exit(VERSION_OLDER);
#endif /* !CONFIG_USER_NETFLASH_VERSION_ALLOW_OLDER */
		case 5:
			printf("netflash: VERSION - you are trying to load an "
				"image that does not\n         "
				"contain valid version information.\n");
			exit(NO_VERSION);
		case 6:
			printf("netflash: VERSION - you are trying to load an "
				"image for a different language.\n");
			exit(BAD_LANGUAGE);
		case 0:
			default:
			break;
		}
	}

	if (dohardwareversion) {
		switch (rc){
		case 1:
			printf("netflash: VERSION - product name incorrect.\n");
			exit(WRONG_PRODUCT);
		case 2:
			printf("netflash: VERSION - vendor name incorrect.\n");
			exit(WRONG_VENDOR);
		case 0:
			default:
			break;
		}
	}
#endif /*CONFIG_USER_NETFLASH_VERSION*/

#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	doinflate = check_decompression(doinflate);
#else
	image_length = file_length;
#endif

	/* Check image that we fetched will actually fit in the FLASH device. */
	if (image_length > devsize - offset)
		error("image too large for FLASH device (size=%d)",
			devsize - offset);

	if(dothrow) {
		printf("netflash: the image is good.\n");
		exit(0);
	}

	if (flashing_rootfs(rdev)) {
		/*
		 *	Our filesystem is live, so we MUST kill processes if we haven't
		 *  done it already.
		 */
		printf("netflash: flashing root filesystem, kill is forced\n");
		if (!dokill || dokillpartial) {
			kill_processes(console);
		}
		/* We must reboot on this platform */
		doreboot = 1;
	}

#if defined(CONFIG_USER_MOUNT_UMOUNT) || defined(CONFIG_USER_BUSYBOX_UMOUNT)
	if (doreboot)
		umount_all();
#endif

#ifdef CONFIG_JFFS_FS
	/* Stop the JFFS garbage collector */
	killprocname("jffs_gcd", SIGSTOP);
#endif
#ifdef CONFIG_JFFS2_FS
	/* Stop the JFFS2 garbage collector */
	killprocname("jffs2_gcd_mtd1", SIGSTOP);
#endif

#if 0
{
	/* Check how much free memory we have */
	FILE* memfile;
	char buf[128];

	memfile = fopen("/proc/meminfo", "r");
	if (memfile) {
		while (fgets(buf, sizeof(buf), memfile))
			fputs(buf, stdout);
		fclose(memfile);
	}
}
#endif

	/*
	 * Program the FLASH device.
	 */
	fflush(stdout);
	sleep(1);
	printf("netflash: programming FLASH device %s\n", rdev);
	fflush(stdout);
	program_flash(rd, devsize, sgdata, sgsize, check_buf);

	if (doreboot) {
#if __GNU_LIBRARY__ > 5
		reboot(0x01234567);
#else
		reboot(0xfee1dead, 672274793, 0x01234567);
#endif
	}

	return exitstatus;
}

/****************************************************************************/
