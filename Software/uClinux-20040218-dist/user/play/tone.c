/*****************************************************************************/

/*
 *	tone.c  --  play a tone.
 *
 *	(C) Copyright 1999-2001, Greg Ungerer (gerg@snapgear.com)
 *	(C) Copyright 2000-2001, Lineo Inc. (www.lineo.com) 
 */

/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <mathf.h>

#include <linux/soundcard.h>

/*****************************************************************************/

#define	DACDEVICE	"/dev/dsp"
#define	MAXFREQ		50000
#define	BUFSIZE		(2*MAXFREQ)

short	buf[BUFSIZE];

#define	PI	((float) 3.141592654)

/*****************************************************************************/

/*
 *	These routines generate a single wave in the buffer. There is a
 *	reasonable error component here, since for the most part the wave
 *	at the specified frequency won't be a whole number of samples.
 *	Its pretty close for lower frequencies, good enough to do audio
 *	quality testing anyways.
 */

int mksinebuf(short *bp, int rf, int wf)
{
	float	radspersample;
	int	samplespercycle, i;
	short	val;

	samplespercycle = rf / wf;
	radspersample = (2 * PI) / samplespercycle;

	for (i = 0; (i < samplespercycle); i++) {
		val = (short) (32767 * sin(i * radspersample));
		*bp++ = val;
		*bp++ = val;
	}

	return(samplespercycle);
}


/*****************************************************************************/

int mksquarebuf(short *bp, int rf, int wf)
{
	int	samplespercycle, samplesperhalfcycle, i;
	short	val;

	samplespercycle = rf / wf;
	samplesperhalfcycle = samplespercycle / 2;

	for (i = 0; (i < samplespercycle); i++) {
		val = (i >= samplesperhalfcycle) ? -32767 : 32767;
		*bp++ = val;
		*bp++ = val;
	}

	return(samplespercycle);
}

/*****************************************************************************/

int mkrampbuf(short *bp, int rf, int wf)
{
	int	samplespercycle, incpersample, i;
	short	val;

	samplespercycle = rf / wf;
	incpersample = 65535 / samplespercycle;

	for (i = 0, val = -32767; (i < samplespercycle); i++) {
		*bp++ = val;
		*bp++ = val;
		val += incpersample;
	}

	return(samplespercycle);
}

/*****************************************************************************/

int mksawtoothbuf(short *bp, int rf, int wf)
{
	int	samplespercycle, samplesperhalfcycle;
	int	incpersample, i;
	short	val;

	samplespercycle = rf / wf;
	samplesperhalfcycle = samplespercycle / 2;
	incpersample = 65535 / samplesperhalfcycle;

	for (i = 0, val = -32767; (i < samplespercycle); i++) {
		*bp++ = val;
		*bp++ = val;
		val += (incpersample * ((i >= samplesperhalfcycle) ? -1 : 1));
	}

	return(samplespercycle);
}

/*****************************************************************************/

#define	SINE		0
#define	SQUARE		1
#define	RAMP		2
#define	SAWTOOTH	3

#define	NUMTYPES	4

int (*mkwavbuf[])(short *, int, int) = {
	mksinebuf,
	mksquarebuf,
	mkrampbuf,
	mksawtoothbuf,
};

char *wavnames[] = {
	"sine",
	"square",
	"ramp",
	"sawtooth",
};

/*****************************************************************************/

void usage(int rc)
{
	printf("usage: tone [-sqrwh?] [-f replay-freq] [wave-freq]\n\n"
		"\t-h?\tthis help\n"
		"\t-s\tsine wave output\n"
		"\t-q\tsquare wave output\n"
		"\t-r\tramp wave output\n"
		"\t-w\tsawtooth wave output\n"
		"\t-f\tfrequency of replay engine\n");
	exit(rc);
}

/*****************************************************************************/

int main(int argc, char *argv[])
{
	int	ofd, i, c, size;
	int	replayfreq, wavefreq, wavetyp;

	replayfreq = 44100;
	wavetyp = SINE;
	wavefreq = 1000;

	while ((c = getopt(argc, argv, "?hsqrwf:")) >= 0) {
		switch (c) {
		case 'f':
			replayfreq = atoi(optarg);
			if ((replayfreq < 1) || (replayfreq > MAXFREQ)) {
				printf("ERROR: invalid frequency %d, range "
					"1-%d\n", replayfreq, MAXFREQ);
				exit(1);
			}
			break;
		case 's':
			wavetyp = SINE;
			break;
		case 'q':
			wavetyp = SQUARE;
			break;
		case 'r':
			wavetyp = RAMP;
			break;
		case 'w':
			wavetyp = SAWTOOTH;
			break;
		case 'h':
		case '?':
			usage(0);
			break;
		default:
			usage(1);
			break;
		}
	}

	if (optind == (argc - 1)) {
		wavefreq = atoi(argv[optind]);
		if ((wavefreq < 1) || (wavefreq > replayfreq)) {
			printf("ERROR: invalid wave frequency %d, range "
				"1-%d\n", wavefreq, replayfreq);
			exit(1);
		}
	}

	if ((ofd = open(DACDEVICE, O_RDWR)) < 1) {
		printf("ERROR: failed to open DAC device %s\n", DACDEVICE);
		exit(1);
	}

	if (ioctl(ofd, SNDCTL_DSP_SPEED, &replayfreq) < 0) {
		printf("ERROR: ioctl(SNDCTL_DSP_SPEED,%d) failed, "
			"errno=%d\n", replayfreq, errno);
		exit(1);
	}

	i = 1;
	if (ioctl(ofd, SNDCTL_DSP_STEREO, &i) < 0) {
		printf("ERROR: ioctl(SNDCTL_DSP_STEREO,%d) failed, "
			"errno=%d\n", errno, i);
		exit(1);
	}

	i = AFMT_S16_BE;
	if (ioctl(ofd, SNDCTL_DSP_SAMPLESIZE, &i) < 0) {
		printf("ERROR: ioctl(SNDCTL_DSP_SAMPLESIZE,0x%x) failed, "
			"errno=%d\n", errno, i);
		exit(1);
	}

	printf("TONE: generating %s wave at %d Hz...\n", 
		wavnames[wavetyp], wavefreq);
	size = (mkwavbuf[wavetyp])(buf, replayfreq, wavefreq);
	size *= 2 * sizeof(short);

	for (;;) {
		if ((i = write(ofd, buf, size)) != size) {
			printf("ERROR: write(%s) failed, i=%d, errno=%d\n",
				DACDEVICE, i, errno);
		}
	}

	close(ofd);
	exit(0);
}

/*****************************************************************************/
