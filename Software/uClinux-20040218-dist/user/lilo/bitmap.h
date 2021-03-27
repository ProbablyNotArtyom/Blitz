/* bitmap.h */


#ifndef BITMAP_H
#define BITMAP_H

typedef unsigned long  bm_uint32;
typedef signed long    bm_sint32;
typedef unsigned short bm_uint16;
typedef signed short   bm_sint16;
typedef unsigned char  bm_byte;


/* Windows/OS2 bitmap header */
typedef struct BitMapHeader {
   bm_uint32   size;
   bm_sint32   width;
   bm_sint32   height;
   bm_uint16   numBitPlanes;
   bm_uint16   numBitsPerPlane;
   bm_uint32   compressionScheme;
   bm_uint32   sizeImageData;
   bm_uint32   xResolution, yResolution;
   bm_uint32   numColorsUsed, numImportantColors;
} BITMAPHEADER;


/* OS2 bitmap header */
typedef struct BitMapHeader2 {
   bm_uint32   size;
   bm_sint16   width;
   bm_sint16   height;
   bm_uint16   numBitPlanes;
   bm_uint16   numBitsPerPlane;
} BITMAPHEADER2;


typedef struct Rgb {
   bm_byte  blue, green, red, null;
} RGB;

/* common BM file header */
typedef struct BitMapFileHeader {
   bm_uint16   magic;      /* must be "BM" */
   bm_uint16   size[2];				/* actually bm_uint32 */
   bm_sint16   xHotspot, yHotspot;
   bm_uint16   offsetToBits[2];			/* actually bm_uint32 */
} BITMAPFILEHEADER;	/* needed to compensate for GCC's alignment rules */

/* LILO scheme */
typedef struct Scheme {
   short int fg, bg, sh;
   } SCHEME;

#endif
/* end bitmap.h */
