#define HAVE_PROTOTYPES
#define HAVE_STDDEF_H
#define HAVE_STDLIB_H
#define NEED_BSD_STRINGS
#define NEED_SYS_TYPES_H

#ifdef JPEG_INTERNALS
#define INLINE inline
#endif /* JPEG_INTERNALS */

#ifdef JPEG_CJPEG_DJPEG
#define BMP_SUPPORTED		/* BMP image file format */
#define GIF_SUPPORTED		/* GIF image file format */
#define PPM_SUPPORTED		/* PBMPLUS PPM/PGM image file format */
#undef RLE_SUPPORTED		/* Utah RLE image file format */
#define TARGA_SUPPORTED		/* Targa image file format */
#endif /* JPEG_CJPEG_DJPEG */

