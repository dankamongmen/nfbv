/*
    fbv  --  simple image viewer for the linux framebuffer
    Copyright (C) 2000, 2001, 2003  Mateusz Golicz

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include <stdlib.h>

unsigned char * simple_resize(unsigned char * orgin,int ox,int oy,int dx,int dy)
{
    unsigned char *cr,*p,*l;
    int i,j,k,ip;
    cr=(unsigned char*) malloc(dx*dy*3); l=cr;
    
    for(j=0;j<dy;j++,l+=dx*3)
    {
	p=orgin+(j*oy/dy*ox*3);
	for(i=0,k=0;i<dx;i++,k+=3)
	{
	    ip=i*ox/dx*3;
	    l[k]=p[ip];
	    l[k+1]=p[ip+1];
	    l[k+2]=p[ip+2];
	}
    }
    free(orgin);
    return(cr);
}

unsigned char * alpha_resize(unsigned char * alpha,int ox,int oy,int dx,int dy)
{
    unsigned char *cr,*p,*l;
    int i,j,k;
    cr=(unsigned char*) malloc(dx*dy); l=cr;
    
    for(j=0;j<dy;j++,l+=dx)
    {
		p = alpha+(j*oy/dy*ox);
		for(i=0,k=0;i<dx;i++)
		    l[k++]=p[i*ox/dx];
	}
	
    free(alpha);
    return(cr);
}

unsigned char * color_average_resize(unsigned char * orgin,int ox,int oy,int dx,int dy)
{
    unsigned char *cr,*p,*q;
    int i,j,k,l,xa,xb,ya,yb;
    int sq,r,g,b;
    cr=(unsigned char*) malloc(dx*dy*3); p=cr;
    
    for(j=0;j<dy;j++)
    {
	for(i=0;i<dx;i++,p+=3)
	{
	    xa=i*ox/dx;
	    ya=j*oy/dy;
	    xb=(i+1)*ox/dx; if(xb>=ox) xb=ox-1;
	    yb=(j+1)*oy/dy; if(yb>=oy) yb=oy-1;
	    for(l=ya,r=0,g=0,b=0,sq=0;l<=yb;l++)
	    {
		q=orgin+((l*ox+xa)*3);
		for(k=xa;k<=xb;k++,q+=3,sq++)
		{
		    r+=q[0]; g+=q[1]; b+=q[2];
		}
	    }
	    p[0]=r/sq; p[1]=g/sq; p[2]=b/sq;
	}
    }
    free(orgin);
    return(cr);
}
