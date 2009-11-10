#include "config.h"
#include "fbv.h"
#include <stdio.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define SHOWDELAY 100000
#define IDSTRING "fbv 0.92b, s-tech"

extern unsigned char * simple_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);
extern unsigned char * color_average_resize(unsigned char * orgin,int ox,int oy,int dx,int dy);


int clear=1,delay=0,hide=1,dispinfo=1,allowstrech=0;

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
    tcgetattr(0,&oldtermios);
    memcpy(&ourtermios,&oldtermios,sizeof(struct termios));
    ourtermios.c_lflag&=!(ECHO|ICANON);
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
#ifdef FBV_SUPPORT_GIF
    extern int fh_gif_getsize(char *,int *,int*);
    extern int fh_gif_load(char *,unsigned char *,int,int);
    extern int fh_gif_id(char *);
#endif
#ifdef FBV_SUPPORT_JPEG
    extern int fh_jpeg_getsize(char *,int *,int*);
    extern int fh_jpeg_load(char *,unsigned char *,int,int);
    extern int fh_jpeg_id(char *);
#endif
#ifdef FBV_SUPPORT_PNG
    extern int fh_png_getsize(char *,int *,int*);
    extern int fh_png_load(char *,unsigned char *,int,int);
    extern int fh_png_id(char *);
#endif

void init_handlers(void)
{
#ifdef FBV_SUPPORT_GIF
    add_format(fh_gif_getsize,fh_gif_load,fh_gif_id);
#endif
#ifdef FBV_SUPPORT_JPEG
    add_format(fh_jpeg_getsize,fh_jpeg_load,fh_jpeg_id);
#endif
#ifdef FBV_SUPPORT_PNG
    add_format(fh_png_getsize,fh_png_load,fh_png_id);
#endif
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

int show_image(char *name)
{
    int x,y,xs,ys,xpos,ypos,xdelta,ydelta,c,eol,xstep,ystep,rfrsh,imx,imy;
    unsigned char *buffer;
    struct formathandler *fh;
    eol=1;
    if(fh=fh_getsize(name,&x,&y))
    {
	buffer=(unsigned char *) malloc(x*y*3);
	if(fh->get_pic(name,buffer,x,y)==FH_ERROR_OK)
	{
	    if(clear) { printf("\033[H\033[J"); fflush(stdout); usleep(SHOWDELAY); } /* temporary solution */
	    if(dispinfo) printf("%s\n%s\n%d x %d\n",IDSTRING,name,x,y); 
	    contoraw();
	    getCurrentRes(&xs,&ys);
	    if((x>xs || y>ys) && allowstrech)
	    {
		if( (y*xs/x) <= ys)
		{
		    imx=xs;
		    imy=y*xs/x;
		}
		else
		{
		    imx=x*ys/y;
		    imy=ys;
		}
		if(allowstrech==1)
		    buffer=simple_resize(buffer,x,y,imx,imy);
		else
		    buffer=color_average_resize(buffer,x,y,imx,imy);
	        x=imx; y=imy;
	    }
	    
	    if(x<xs) xpos=(xs-x)/2; else xpos=0;
	    if(y<ys) ypos=(ys-y)/2; else ypos=0;
	    xdelta=0; ydelta=0;

	    xstep=min(max(x/20,1),xs);
	    ystep=min(max(y/20,1),ys);

	    for(eol=-1,rfrsh=1;eol==-1;)
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
			    eol=1;
			    break;
			case 'q':
			    eol=0;
			    break;
		    }
		}
		else
		{
		    if(imm_getchar(delay / 10,delay % 10)=='q') eol=0; else eol=1;
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
	
    return(eol);
}

void help(char *name)
{
	printf("Usage: %s [options] image1 image2 image3 ...\n\n"
	       "The options are:\n"
	       " -h : Show this help\n"
	       " -c : Do not clear the screen before/after displaying image\n"
	       " -u : Do not hide/show cursor before/after displaying image\n"
	       " -i : Do not show image information\n"
	       " -f : Strech (using simple resize) the image to fit onto screen if necessary\n"
	       " -k : Strech (using color average resize) the image to fit onto screen if necessary\n"
               " -s <delay> slideshow, wait 'delay' tenths of a second before displaying each image\n\n"
	       "Use a,d,w and x to scroll the image\n\n"
	       "%s/2000, http://s-tech.linux-pl.com\n",name,IDSTRING);
}

extern int optind;
extern char *optarg;

int main(int argc,char **argv)
{
    int x,y,opt,a;
    unsigned char *buffer;
    init_handlers();
    
    if(argc<2)
	help(argv[0]);
    else
    {
	for(;;)
	{
	    opt=getopt_long(argc,argv,"chukfis:");
	    if(opt==EOF) break;
	    switch(opt)
	    {
		case 'c': clear=0; break;
		case 's': if(optarg) delay=atoi(optarg); break;
		case 'u': hide=0; break;
		case 'h': help(argv[0]); break;
		case 'i': dispinfo=0; break;
		case 'f': allowstrech=1; break;
		case 'k': allowstrech=2; break;
	    }
	}
	while(imm_getchar(0,0)!=EOF);
	if(hide) printf("\033[?25l");
	for(a=optind;argv[a]!=NULL;a++) if(!show_image(argv[a])) break;
	if(hide) printf("\033[?25h");
    }
    return;
}
