#include "fb_display.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
	      
    char *dev = DEFAULT_FRAMEBUFFER;
    char *modedbf = DEFAULT_MODEDBFILE;

int openFB(const char *name);
void closeFB(int fh);
void getVarScreenInfo(int fh, struct fb_var_screeninfo *var);
void setVarScreenInfo(int fh, struct fb_var_screeninfo *var);
void getFixScreenInfo(int fh, struct fb_fix_screeninfo *fix);
static void convertFromVideoMode(const struct VideoMode *vmode, struct fb_var_screeninfo *var);
static void convertToVideoMode(const struct fb_var_screeninfo *var, struct VideoMode *vmode);
static struct VideoMode *findBestVideoMode(int sx, int sy);
void blitRGB2FB(int fh, char *rgbbuff, int pxs, int pys, int sxs, int sys, int xp, int yp, int xs, int ys, int dxs, int dys);
char getKey(void);

void fb_display(char *rgbbuff, int x_size, int y_size, float scale_factor, int options)
{
    struct VideoMode *vmode = NULL;
    struct fb_var_screeninfo var, var_back;
//    struct fb_fix_screeninfo fix;
    int fh = -1,
	disp_x, disp_y, x_pan = 0, y_pan = 0;

    /* compute the sizes */
    disp_x = (int) ((float) x_size * scale_factor);
    disp_y = (int) ((float) y_size * scale_factor);

printf("x%d, y%d\n", disp_x, disp_y);
    /* first we get the framebuffer device handle */
    fh = openFB(dev);
    /* backup current video mode */
    getVarScreenInfo(fh, &var_back);
    if(options & FBDISPLAY_AUTO_RES){
	/* read modes database */
	readModeDB();
	/* search for best fitting videomode */
	if (!(vmode = findBestVideoMode(disp_x, disp_y))){
	    fprintf(stderr, "Couldn't find video mode for: %d x %d\n", disp_x, disp_y);
	    exit(1);
	}
	convertFromVideoMode(vmode, &var);
	setVarScreenInfo(fh, &var);
    }else
	convertToVideoMode(&var, vmode);

printf("xres:%d, yres:%d\n", vmode->xres, vmode->yres);
    
    do{
	if((disp_x > vmode->xres) || (disp_y > vmode->yres)){
	    if(options & FBDISPLAY_AUTO_SIZE){
		scale_factor = min(((float) vmode->xres / (float) x_size),((float) vmode->yres / (float) y_size));
		blitRGB2FB(fh, rgbbuff, x_size, y_size, vmode->xres, vmode->yres, 0, 0, x_size, y_size,
		    (int) ((float) x_size * scale_factor), disp_y = (int) ((float) y_size * scale_factor));
	    }else{
		blitRGB2FB(fh, rgbbuff, x_size, y_size, vmode->xres, vmode->yres, x_pan, y_pan,
		    vmode->xres, vmode->yres, vmode->xres, vmode->yres);
	    }
	}else{
	    blitRGB2FB(fh, rgbbuff, x_size, y_size, vmode->xres, vmode->yres, 0, 0,
		x_size, y_size, x_size, y_size);
	}
	
    
    }while(!getKey());
    
    /* restore previous mode */
    setVarScreenInfo(fh, &var_back);
    closeFB(fh);
}

int openFB(const char *name)
{
    int fh;
		
    if ((fh = open(name, O_RDWR)) == -1){
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

static void convertFromVideoMode(const struct VideoMode *vmode,
				 struct fb_var_screeninfo *var)
{
    memset(var, 0, sizeof(struct fb_var_screeninfo));
    var->xres = vmode->xres;
    var->yres = vmode->yres;
    var->xres_virtual = vmode->vxres;
    var->yres_virtual = vmode->vyres;
    var->bits_per_pixel = vmode->depth;
    var->nonstd = vmode->nonstd;
    var->activate = FB_ACTIVATE_NOW;
    var->accel_flags = vmode->accel_flags;
    var->pixclock = vmode->pixclock;
    var->left_margin = vmode->left;
    var->right_margin = vmode->right;
    var->upper_margin = vmode->upper;
    var->lower_margin = vmode->lower;
    var->hsync_len = vmode->hslen;
    var->vsync_len = vmode->vslen;
    if (vmode->hsync == HIGH)
	var->sync |= FB_SYNC_HOR_HIGH_ACT;
    if (vmode->vsync == HIGH)
	var->sync |= FB_SYNC_VERT_HIGH_ACT;
    if (vmode->csync == HIGH)
	var->sync |= FB_SYNC_COMP_HIGH_ACT;
    if (vmode->gsync == HIGH)
	var->sync |= FB_SYNC_ON_GREEN;
    if (vmode->extsync == TRUE)
	var->sync |= FB_SYNC_EXT;
    if (vmode->bcast == TRUE)
	var->sync |= FB_SYNC_BROADCAST;
    if (vmode->laced == TRUE)
	var->vmode = FB_VMODE_INTERLACED;
    else if (vmode->dblscan == TRUE)
	var->vmode = FB_VMODE_DOUBLE;
    else
	var->vmode = FB_VMODE_NONINTERLACED;
    var->vmode |= FB_VMODE_CONUPDATE;
    var->red.length = vmode->red.length;
    var->red.offset = vmode->red.offset;
    var->green.length = vmode->green.length;
    var->green.offset = vmode->green.offset;
    var->blue.length = vmode->blue.length;
    var->blue.offset = vmode->blue.offset;
    var->transp.length = vmode->transp.length;
    var->transp.offset = vmode->transp.offset;
    var->grayscale = vmode->grayscale;
}

static void convertToVideoMode(const struct fb_var_screeninfo *var,
			       struct VideoMode *vmode)
{
    vmode->name = NULL;
    vmode->xres = var->xres;
    vmode->yres = var->yres;
    vmode->vxres = var->xres_virtual;
    vmode->vyres = var->yres_virtual;
    vmode->depth = var->bits_per_pixel;
    vmode->nonstd = var->nonstd;
    vmode->accel_flags = var->accel_flags;
    vmode->pixclock = var->pixclock;
    vmode->left = var->left_margin;
    vmode->right = var->right_margin;
    vmode->upper = var->upper_margin;
    vmode->lower = var->lower_margin;
    vmode->hslen = var->hsync_len;
    vmode->vslen = var->vsync_len;
    vmode->hsync = var->sync & FB_SYNC_HOR_HIGH_ACT ? HIGH : LOW;
    vmode->vsync = var->sync & FB_SYNC_VERT_HIGH_ACT ? HIGH : LOW;
    vmode->csync = var->sync & FB_SYNC_COMP_HIGH_ACT ? HIGH : LOW;
    vmode->gsync = var->sync & FB_SYNC_ON_GREEN ? TRUE : FALSE;
    vmode->extsync = var->sync & FB_SYNC_EXT ? TRUE : FALSE;
    vmode->bcast = var->sync & FB_SYNC_BROADCAST ? TRUE : FALSE;
    vmode->grayscale = var->grayscale;
    vmode->laced = FALSE;
    vmode->dblscan = FALSE;
    switch (var->vmode & FB_VMODE_MASK) {
	case FB_VMODE_INTERLACED:
	    vmode->laced = TRUE;
	    break;
	case FB_VMODE_DOUBLE:
	    vmode->dblscan = TRUE;
	    break;
    }
    vmode->red.length = var->red.length;
    vmode->red.offset = var->red.offset;
    vmode->green.length = var->green.length;
    vmode->green.offset = var->green.offset;
    vmode->blue.length = var->blue.length;
    vmode->blue.offset = var->blue.offset;
    vmode->transp.length = var->transp.length;
    vmode->transp.offset = var->transp.offset;
    fillScanRates(vmode);
}

static struct VideoMode *findBestVideoMode(int sx, int sy)
{
    struct VideoMode *vmode, *ivm;
    int prev_delta;

    vmode = NULL;
    prev_delta = 65535;
    for (ivm = VideoModes; ivm; ivm = ivm->next){
	if ((ivm->xres >= sx) && (ivm->yres >= sx) && (ivm->xres - sx + ivm->yres - sy < prev_delta))
	    vmode = ivm;
    }
    return vmode;
}

void blitRGB2FB(int fh, char *rgbbuff, int pxs, int pys, int sxs, int sys, int xp, int yp, int xs, int ys, int dxs, int dys)
{
    int i,j;
    char *bufptr;
    float dx, dy, xoffs, yoffs;

    dx = (float) pxs / (float) dxs;
    dy = (float) pys / (float) dys;


    for(j = 0, yoffs = 0; j < dys; j++){
	bufptr = rgbbuff + j * pxs * 3;
	if (fseek((FILE*)fh, (int)yoffs * sxs, SEEK_SET)){
	    fprintf(stderr, "fseek on FB failed: %s\n", strerror(errno));
	    exit(1);
	}
	for(i = 0, xoffs = 0; i < dxs; i++){
	    if (fputc(bufptr[(int)xoffs * 3], (FILE*)fh)){
		fprintf(stderr, "write to FB failed: %s\n", strerror(errno));
		exit(1);
	    }
	    if (fputc(bufptr[(int)xoffs * 3 + 1], (FILE*)fh)){
		fprintf(stderr, "write to FB failed: %s\n", strerror(errno));
		exit(1);
	    }
	    if (fputc(bufptr[(int)xoffs * 3 + 2], (FILE*)fh)){
		fprintf(stderr, "write to FB failed: %s\n", strerror(errno));
		exit(1);
	    }
	    xoffs += dx;
	}
	yoffs += dy;
    }
}

char getKey(void)
{
    char c;
    c=fgetc(stdin);
    return c;
}
