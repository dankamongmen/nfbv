#!/bin/sh

xdir="/usr/X11R6"

#echo "Testing for libungif presence ... "

cat >cfgtest1.c <<eof
main()
{
}
eof

cc 1>&2 2>/dev/null -o cfgtest1 cfgtest1.c -lungif
    
if test -f cfgtest1; then
    gifflags="-lungif"
else
    cc 1>&2 2>/dev/null -o cfgtest1 cfgtest1.c -lungif -lX11 -L$xdir/lib

    if test -f cfgtest1; then
        gifflags="-lungif -lX11 -L$xdir/lib"
    else
	gifflags="none"
    fi
fi

rm -f cfgtest1
    


#echo "Testing for libjpeg presence ..."

cc 1>&2 2>/dev/null -o cfgtest1 cfgtest1.c -ljpeg

if test -f cfgtest1; then
    jpegflags="-ljpeg"
else
    jpegflags="none"
fi

rm -f cfgtest1
    
#echo "Testing for libpng presence ..."

cc 1>&2 2>/dev/null -o cfgtest1 cfgtest1.c -lpng

if test -f cfgtest1; then
    pngflags="-lpng"
else
    pngflags="none"
fi

rm -f cfgtest1

rm -f cfgtest1.c

deffb="/dev/fb0"
if test -c "/dev/fb/0"; then
    deffb="/dev/fb/0"
fi

echo "fbv Configure script v 0.3"
echo ""

echo -n "What flags for cc are needed on your system in order to compile libungif applications? (usually -lungif) Hit enter to use the default in square brackets or type 'none' to compile without GIF support [$gifflags] ? "
read input_gifflags
echo -n "What flags for cc are needed on your system in order to compile libjpeg applications? (usually -ljpeg) Hit enter to use the default in square brackets or type 'none' to compile without JPEG support [$jpegflags] ? "
read input_jpegflags
echo -n "What flags for cc are needed on your system in order to compile libpng applications? (usually -lpng) Hit enter to use the default in square brackets or type 'none' to compile without PNG support [$pngflags] ? "
read input_pngflags
echo -n "What is your default framebuffer device ? (This could be naturally overriden by usage of FRAMEBUFFER environment variable) [$deffb] ? "
read input_deffb

if test -n "$input_gifflags"; then
     gifflags="$input_gifflags"
fi
if test -n "$input_jpegflags"; then
     jpegflags="$input_jpegflags"
fi

if test -n "$input_pngflags"; then
     pngflags="$input_pngflags"
fi
if test -n "$input_deffb"; then
     deffb="$input_deffb"
fi


clibs=""

echo ""
echo "Generating makefile and config.h..."
echo >config.h "#define DEFAULT_FRAMEBUFFER     \"$deffb\""


if test "$gifflags" != "none"; then 
clibs="$clibs $gifflags"
echo >>config.h "#define FBV_SUPPORT_GIF"
fi
if test "$jpegflags" != "none"; then 
clibs="$clibs $jpegflags"
echo >>config.h "#define FBV_SUPPORT_JPEG"
fi
if test "$pngflags" != "none"; then 
clibs="$clibs $pngflags"
echo >>config.h "#define FBV_SUPPORT_PNG"
fi



cat > Makefile << eof

SRCS = main.c jpeg.c gif.c png.c fb_display.c resize.c
OBJS = main.o jpeg.o gif.o png.o fb_display.o resize.o

OUT = fbv

eof

echo >> Makefile "LIBS=$clibs"

cat >>Makefile <<eof

\$(OUT): \$(OBJS)
	\$(CC)  \$(LDFLAGS) -o \$(OUT) \$(OBJS) \$(LIBS)

clean:
	rm -f \$(OUT) *.o *~ *.bak *core
	rm -f \$(OBJS)
mrproper: clean
	rm -f Makefile config.h
install:
	cp fbv /usr/bin
uninstall:
	rm /usr/bin/fbv

eof

rm -f *.o fbv

echo ""
echo "Done .. Now just type make :)"
