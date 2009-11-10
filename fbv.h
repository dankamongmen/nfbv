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

struct formathandler 
{
    struct formathandler *next;
    int (*get_size)(char *,int *,int*);
    int (*get_pic)(char *,unsigned char *,unsigned char **, int,int);
    int (*id_pic)(char *);
};

#define FH_ERROR_OK 0
#define FH_ERROR_FILE 1		/* read/access error */
#define FH_ERROR_FORMAT 2	/* file format error */

void fb_display(unsigned char *rgbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
void getCurrentRes(int *x, int *y);
