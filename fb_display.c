#include "fb_display.h"

char *fbdev = DEFAULT_FRAMEBUFFER;
unsigned short red[256], green[256], blue[256];
struct fb_cmap map332 = {0, 256, red, green, blue, NULL};


int openFB(const char *name);
void closeFB(int fh);
void getVarScreenInfo(int fh, struct fb_var_screeninfo *var);
void setVarScreenInfo(int fh, struct fb_var_screeninfo *var);
void getFixScreenInfo(int fh, struct fb_fix_screeninfo *fix);
void set332map(int fh);
unsigned char* convertRGB2FB(unsigned char *rgbbuff, unsigned long count);
void blit2FB(int fh, unsigned char *fbbuff,
	unsigned int pic_xs, unsigned int pic_ys,
	unsigned int scr_xs, unsigned int scr_ys,
	unsigned int xp, unsigned int yp);

void fb_display(char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan)
{
    struct fb_var_screeninfo var;
    unsigned char *fbbuff = NULL;
    int fh = -1;
    
    /* first we get the framebuffer device handle */
    fh = openFB(fbdev);
    
    /* read current video mode */
    getVarScreenInfo(fh, &var);
    
    /* check if we have correct mode */
    if(var.bits_per_pixel != 8){
        fprintf(stderr, "Only 8bpp modes supported now. You've got: %d\n", var.bits_per_pixel);
	exit(1);
    }
    
    /* let's make a pseudo-truecolor map */
    set332map(fh);
    
    /* correct panning */
    if(x_pan > x_size - var.xres) x_pan = 0;
    if(y_pan > y_size - var.yres) y_pan = 0;
    
    /* blit buffer 2 fb */
    fbbuff = convertRGB2FB(rgbbuff, x_size * y_size);
    blit2FB(fh, fbbuff, x_size, y_size, var.xres, var.yres, x_pan, y_pan);
    free(fbbuff);
    
    /* close device */
    closeFB(fh);
}

int openFB(const char *name)
{
    int fh;
		
    if ((fh = open(name, O_WRONLY)) == -1){
        fprintf(stderr, "open %s: %s\n", name, strerror(errno));
	exit(1);
    }
    return fh;
}

void closeFB(int fh)
{
    close(fh);
}

void getVarScreenInfo(int fh, struct fb_var_screeninfo *var)
{
    if (ioctl(fh, FBIOGET_VSCREENINFO, var)){
        fprintf(stderr, "ioctl FBIOGET_VSCREENINFO: %s\n", strerror(errno));
	exit(1);
    }
}

void setVarScreenInfo(int fh, struct fb_var_screeninfo *var)
{
    if (ioctl(fh, FBIOPUT_VSCREENINFO, var)){
        fprintf(stderr, "ioctl FBIOPUT_VSCREENINFO: %s\n", strerror(errno));
	exit(1);
    }
}

void getFixScreenInfo(int fh, struct fb_fix_screeninfo *fix)
{
    if (ioctl(fh, FBIOGET_FSCREENINFO, fix)){
        fprintf(stderr, "ioctl FBIOGET_FSCREENINFO: %s\n", strerror(errno));
	exit(1);
    }
}

void blit2FB(int fh, unsigned char *fbbuff,
	unsigned int pic_xs, unsigned int pic_ys,
	unsigned int scr_xs, unsigned int scr_ys,
	unsigned int xp, unsigned int yp)
{
    int i, xc, yc;

    xc = (pic_xs > scr_xs) ? scr_xs : pic_xs;
    yc = (pic_ys > scr_ys) ? scr_ys : pic_ys;
    
    for(i = 0; i < yc; i++){
	lseek(fh, i*scr_xs, SEEK_SET);
	write(fh, fbbuff + (i+yp)*pic_xs+xp, xc);
    }
}

inline unsigned int make8color(register int r, register int g, register int b)
{
	return (
	    ((r >> 5) & 7) << 5 |
	    ((g >> 5) & 7) << 2 |
	    ((b >> 6) & 3)      );
}

void make332map(struct fb_cmap *map)
{
	int rs, gs, bs, i;
	int r = 8, g = 8, b = 4;

	map->red = red;
	map->green = green;
	map->blue = blue;

	rs = 256 / (r - 1);
	gs = 256 / (g - 1);
	bs = 256 / (b - 1);
	
	for (i = 0; i < 256; i++) {
		map->red[i]   = (rs * ((i / (g * b)) % r)) * 255;
		map->green[i] = (gs * ((i / b) % g)) * 255;
		map->blue[i]  = (bs * ((i) % b)) * 255;
	}
}

void set332map(int fh)
{
    make332map(&map332);
    
    if (ioctl(fh, FBIOPUTCMAP, &map332) < 0) {
        fprintf(stderr, "Error putting pseudo-truecolor colormap");
        exit(1);
    }
}
unsigned char* convertRGB2FB(unsigned char *rgbbuff, unsigned long count)
{
    unsigned long i;
    unsigned char *buffp, *rgbp, *fbbuff;
    
    fbbuff = buffp = (unsigned char *) malloc(count * sizeof(unsigned char));
    rgbp = rgbbuff;
    
    for(i = 0; i < count; i++)
	fbbuff[i] = make8color(rgbbuff[i*3], rgbbuff[i*3+1], rgbbuff[i*3+2]);
	
    return fbbuff;
}
