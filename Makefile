
SRCS = main.c jpeg.c gif.c png.c fb_display.c
OBJS = main.o jpeg.o gif.o png.o fb_display.o

OUT = fbv

LIBS = -ljpeg -lvga -lungif -lX11 -L/usr/X11R6/lib -lpng
CFLAGS= --warn-all -I.

$(OUT): $(OBJS) fb_display.h
	$(CC)  $(LDFLAGS) -o $(OUT) $(OBJS) $(LIBS)

clean:
	rm -f $(OUT) *.o *~ *.bak *core
	rm -f $(OBJS)

