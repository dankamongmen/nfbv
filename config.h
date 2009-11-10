#define DEFAULT_FRAMEBUFFER     "/dev/fb0"

#if HAVE_LIBUNGIF==1
#	define FBV_SUPPORT_GIF
#endif

#if HAVE_LIBJPEG==1
#	define FBV_SUPPORT_JPEG
#endif

#if HAVE_LIBPNG==1
#	define FBV_SUPPORT_PNG
#endif
