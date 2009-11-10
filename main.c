#include "fbv.h"
#include <stdio.h>

struct formathandler *fh_root=NULL;

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
    int x,y;
    unsigned char *buffer;
    struct formathandler *fh;
    if(fh=fh_getsize(name,&x,&y))
    {
	buffer=(unsigned char *) malloc(x*y*3);
	if(fh->get_pic(name,buffer,x,y)==FH_ERROR_OK)
	{
	    printf("\033[H\033[J"); fflush(stdout); /* temporary solution */
	    fb_display(buffer,x,y,0,0);
	    getchar(); /* temporary.. dreamer_ !!!!! */
	}
	else
	    printf("Unable to read file !\n");
	free(buffer);
    }
    else
	printf("Unable to read file or format not recognized!\n");
}

int main(int argc,char **argv)
{
    int x,y;
    unsigned char *buffer;
    init_handlers();
    if(argc>=2) show_image(argv[1]);
    return;
}
