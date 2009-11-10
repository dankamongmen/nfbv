#include "fbv.h"
#include <stdio.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define SHOWDELAY 100000

int clear=1,delay=0,hide=1;

struct formathandler *fh_root=NULL;

struct termios oldtermios;
struct termios ourtermios;

int imm_getchar(int s,int us)
{
    struct timeval tv;
    unsigned char c;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0,&fds);
    tv.tv_sec=s; tv.tv_usec=us;
    if(select(1,&fds,NULL,NULL,&tv))
    {
	read(0,&c,1);
	return((int) c);
    }
    else
	return(EOF);
}

void contoraw(void)
{
    ourtermios.c_iflag=0;
    ourtermios.c_oflag=0;
    ourtermios.c_cflag=0;
    ourtermios.c_lflag=0;
    cfmakeraw(&ourtermios);
    tcgetattr(0,&oldtermios);
    tcsetattr(0,TCSANOW,&ourtermios);
}
void connorm(void)
{
    tcsetattr(0,TCSANOW,&oldtermios);
}


void add_format(int (*picsize)(char *,int *,int*),int (*picread)(char *,unsigned char *,int,int), int (*id)(char*))
{
    struct formathandler *fhn;
    fhn=(struct formathandler*) malloc(sizeof(struct formathandler));
    fhn->get_size=picsize; fhn->get_pic=picread; fhn->id_pic=id;
    fhn->next=fh_root; fh_root=fhn;
}
    extern int fh_gif_getsize(char *,int *,int*);
    extern int fh_gif_load(char *,unsigned char *,int,int);
    extern int fh_jpeg_getsize(char *,int *,int*);
    extern int fh_jpeg_load(char *,unsigned char *,int,int);
    extern int fh_png_getsize(char *,int *,int*);
    extern int fh_png_load(char *,unsigned char *,int,int);

    extern int fh_gif_id(char *);
    extern int fh_jpeg_id(char *);
    extern int fh_png_id(char *);

void init_handlers(void)
{
    add_format(fh_gif_getsize,fh_gif_load,fh_gif_id);
    add_format(fh_jpeg_getsize,fh_jpeg_load,fh_jpeg_id);
    add_format(fh_png_getsize,fh_png_load,fh_png_id);
}

struct formathandler * fh_getsize(char *name,int *x,int *y)
{
    struct formathandler *fh;
    for(fh=fh_root;fh!=NULL;fh=fh->next)
    {
	if(fh->id_pic(name))
	    if(fh->get_size(name,x,y)==FH_ERROR_OK) return(fh);
    }
    return(NULL);
}

void show_image(char *name)
{
    int x,y,xs,ys,xpos,ypos,xdelta,ydelta,c,eol,xstep,ystep,rfrsh;
    unsigned char *buffer;
    struct formathandler *fh;
    if(fh=fh_getsize(name,&x,&y))
    {
	buffer=(unsigned char *) malloc(x*y*3);
	if(fh->get_pic(name,buffer,x,y)==FH_ERROR_OK)
	{
	    if(clear) { printf("\033[H\033[J"); fflush(stdout); usleep(SHOWDELAY); } /* temporary solution */
	    contoraw();
	    getCurrentRes(&xs,&ys);
	    xdelta=0; ydelta=0;
	    if(x<xs)
		xpos=(xs-x)/2;
	    else
		xpos=0;

	    if(y<ys)
		ypos=(ys-y)/2;
	    else
		ypos=0;
	    xdelta=0; ydelta=0;

	    xstep=min(max(x/20,1),xs);
	    ystep=min(max(y/20,1),ys);

	    for(eol=0,rfrsh=1;eol==0;)
	    {
		if(rfrsh) fb_display(buffer,x,y,xdelta,ydelta,xpos,ypos);
		rfrsh=0;
		if(!delay)
		{
		    c=getchar();
		    switch(c)
		    {
			case 'a':
			case 'D':
		    	    xdelta-=xstep;
			    if(xdelta<0) xdelta=0;
			    rfrsh=1;
			    break;
			case 'd':
			case 'C':
			    if(xpos) break;
			    xdelta+=xstep;
			    if(xdelta>(x-xs)) xdelta=x-xs;
			    rfrsh=1;
			    break;
			case 'w':
			case 'A':
			    ydelta-=ystep;
			    if(ydelta<0) ydelta=0;
			    rfrsh=1;
			    break;
			case 'x':
			case 'B':
			    if(ypos) break;
			    ydelta+=ystep;
			    if(ydelta>(y-ys)) ydelta=y-ys;
			    rfrsh=1;
			    break;
			case ' ':
			case 10:
			case 'q':
			    eol=1;
		    }
		}
		else
		{
		    imm_getchar(delay / 10,delay % 10);
		    break;
		}
	    }
	    connorm();
	    if(clear) { printf("\033[0m\033[H\033[J"); fflush(stdout); }
	}
	else
	    printf("Unable to read file !\n");
	free(buffer);
    }
    else
	printf("Unable to read file or format not recognized!\n");
}

extern int optind;
extern char *optarg;

int main(int argc,char **argv)
{
    int x,y,opt,a;
    unsigned char *buffer;
    init_handlers();
    
    if(argc<2)
	printf("Usage: %s [options] image1 image2 image3 ...\n\n"
	       "The options are:\n"
	       " -c : Do not clear the screen before/after displaying image\n"
	       " -h : Do not hide/show cursor before/after displaying image\n"
	       " -s <delay> slideshow, wait 'delay' tenths of a second before displaying each image\n\n"
	       "Use a,d,w and x to scroll the image\n",argv[0]);
    else
    {
	for(;;)
	{
	    opt=getopt_long(argc,argv,"chs:");
	    if(opt==EOF) break;
	    if(opt=='c') clear=0;
	    if(opt=='s') if(optarg) delay=atoi(optarg);
	    if(opt=='h') hide=0;
	}
	while(imm_getchar(0,0)!=EOF);
	if(hide) printf("\033[?25l");
	for(a=optind;argv[a]!=NULL;a++) show_image(argv[a]);
	if(hide) printf("\033[?25h");
    }
    return;
}
