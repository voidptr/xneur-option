/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  Copyright (C) 2006-2008 XNeur Team
 *
 */
 
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "xcursor.h"

#ifdef WITH_XPM

#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/XKBlib.h>

#include <unistd.h>
#include <stdio.h>

#include "xnconfig_files.h"

#include "xwindow.h"

#include "log.h"
#include "types.h"

extern struct _xneur_config *xconfig;
extern struct _xwindow *main_window;

static int ReadPixmapFromFile (char *FileName, Pixmap *phPm, Pixmap *phMask, XpmAttributes *pAttrs, Display *pDisplay, Window hWnd, Colormap hColorMap)
{
	FILE *pFile;
	long sizeFile, i, k, N;
	char *buffer;
	char **xpm;

	// Open file
	pFile = fopen (FileName, "r");
	if (!pFile) {
		//perror (FileName);
		return -1;
	}
	fseek (pFile, 0, SEEK_END);
	sizeFile = ftell (pFile);
	fseek (pFile, 0, SEEK_SET);
	
	buffer = (char*)malloc (sizeFile);
	if (!buffer) {
		//perror (FileName);
		fclose (pFile);	
		return -1;
	}
	fread (buffer, 1, sizeFile, pFile);
	fclose (pFile);
	
	// Check Signature
	if (strncmp (buffer, "/* XPM */", 9)) {
		//printf ("%s: not XPM format file\n", FileName);
		free(buffer);
		return -1;
	}

	for (N=0,i=0; i<sizeFile; i++)
		N += (buffer[i]=='"' ? 1 : 0);
	N /= 2;
	xpm = (char**)malloc (N * sizeof (char*));
	for (k=0,i=0; i<sizeFile; i++)
		if (buffer[i]=='"') {
			xpm[k++] = buffer + ++i;
			while (buffer[i]!='"')
				i++;
			buffer[i] = '\0';
		}

	pAttrs->valuemask = XpmColormap | XpmCloseness;
	pAttrs->colormap = hColorMap;
	pAttrs->closeness = 65535;
	XpmCreatePixmapFromData (pDisplay, hWnd, xpm, phPm, phMask, pAttrs);

	free (xpm);
	free (buffer);
	return 0;
}

static GC create_gc(Display* display, Window win, int reverse_video)
{
	GC gc;				/* handle of newly created GC.  */
	unsigned long valuemask = 0;		/* which values in 'values' to  */
					/* check when creating the GC.  */
	XGCValues values;			/* initial values for the GC.   */
	unsigned int line_width = 2;		/* line width for the GC.       */
	int line_style = LineSolid;		/* style for lines drawing and  */
	int cap_style = CapButt;		/* style of the line's edje and */
	int join_style = JoinBevel;		/*  joined lines.		*/
	int screen_num = DefaultScreen(display);

	gc = XCreateGC(display, win, valuemask, &values);
	if (gc < 0) 
	{
		fprintf(stderr, "XCreateGC: \n");
		return NULL;
	}

	/* allocate foreground and background colors for this GC. */
	if (reverse_video) 
	{
		XSetForeground(display, gc, WhitePixel(display, screen_num));
		XSetBackground(display, gc, BlackPixel(display, screen_num));
	}
	else 
	{
		XSetForeground(display, gc, BlackPixel(display, screen_num));
		XSetBackground(display, gc, WhitePixel(display, screen_num));
	}

	/* define the style of lines that will be drawn using this GC. */
	XSetLineAttributes(display, gc, line_width, line_style, cap_style, join_style);

	/* define the fill style for the GC. to be 'solid filling'. */
	XSetFillStyle(display, gc, FillSolid);

	return gc;
} 

void xcursor_show_flag(struct _xcursor *p, int x, int y)
{
	XWindowAttributes w_attributes;
	XGetWindowAttributes(main_window->display, main_window->flag_window, &w_attributes); 
				
	XkbStateRec xkbState;
	XkbGetState(main_window->display, XkbUseCoreKbd, &xkbState);
	
	// if for layout not defined xpm file then unmap window and stop procedure
	if (xconfig->flags[xkbState.group].file[0] == NULLSYM)
	{
		if (w_attributes.map_state != IsUnmapped)
			XUnmapWindow(main_window->display, main_window->flag_window);
		XFlush (main_window->display);
		return;
	}
	
	// if current layout not equal last layout then load new pixmap
	if (p->last_layuot != xkbState.group)
	{
		p->last_layuot = xkbState.group;
		
		XFreePixmap(main_window->display, p->bitmap);
		XFreePixmap(main_window->display, p->bitmap_mask);
		p->bitmap = 0;
		p->bitmap_mask = 0;
		char *path = get_file_path_name(PIXMAPDIR, xconfig->flags[xkbState.group].file);
		if (path == NULL)
		{
			if  (w_attributes.map_state != IsUnmapped)
				XUnmapWindow(main_window->display, main_window->flag_window);
			XFlush (main_window->display);
			return;
		}
		ReadPixmapFromFile(path, &p->bitmap, &p->bitmap_mask, &p->Attrs,
						   main_window->display, main_window->flag_window,
						   XDefaultColormap (main_window->display, DefaultScreen(main_window->display)));
		if (path != NULL)
			free(path);
		// if pixmap not loaded then unmap window (if need) and return
		if (p->bitmap == 0)
		{
			if  (w_attributes.map_state != IsUnmapped)
				XUnmapWindow(main_window->display, main_window->flag_window);
			XFlush (main_window->display);
			return;
		}
		
	}
	
		// for set pixmap to window, need map window
	if (w_attributes.map_state == IsUnmapped)
		XMapWindow(main_window->display, main_window->flag_window);
	
	XSetClipMask (main_window->display, p->gc, p->bitmap_mask);
	XSetClipOrigin (main_window->display, p->gc, 0, 0);
	XCopyArea (main_window->display, p->bitmap, main_window->flag_window, p->gc, 0, 0, p->Attrs.width, p->Attrs.height, 0, 0);
	
	XRaiseWindow(main_window->display, main_window->flag_window);
	XMoveResizeWindow(main_window->display, main_window->flag_window, x, y, p->Attrs.width, p->Attrs.height);	
		
	XFlush (main_window->display);
}

void xcursor_hide_flag(struct _xcursor *p)
{
	XWindowAttributes w_attributes;
	XGetWindowAttributes(main_window->display, main_window->flag_window, &w_attributes);
	
	if (w_attributes.map_state != IsUnmapped)
		XUnmapWindow(main_window->display, main_window->flag_window);
}

void xcursor_uninit(struct _xcursor *p)
{
	Display *dpy = XOpenDisplay(NULL);

	XFreePixmap(dpy, p->bitmap);
	XFreePixmap(dpy, p->bitmap_mask);
	
	XFreeGC(dpy, p->gc);
	XCloseDisplay(dpy);
	
	free(p);
}

struct _xcursor* xcursor_init(void)
{
	struct _xcursor *p = (struct _xcursor *) malloc(sizeof(struct _xcursor));
	bzero(p, sizeof(struct _xcursor));

	int dummy;
	p->gc = create_gc(main_window->display, main_window->flag_window, dummy);
	if (p->gc == NULL)
	{
		free(p);
		return NULL;
	}
	XSync(main_window->display, False);
	
	p->last_layuot = -1;
	
	// Functions mapping
	p->show_flag = xcursor_show_flag;
	p->hide_flag = xcursor_hide_flag;
	p->uninit		= xcursor_uninit;

	return p;
}


#else /* WITH_XPM */

#include "xcursor.h"

void xcursor_load_pixmaps(struct _xcursor *p)
{
	return;
}

void xcursor_show_flag(struct _xcursor *p, int x, int y)
{
	return;
}

void xcursor_hide_flag(struct _xcursor *p)
{
	return;
}

void xcursor_uninit(struct _xcursor *p)
{
	free(p);
}

struct _xcursor* xcursor_init(void)
{
	struct _xcursor *p = (struct _xcursor *) malloc(sizeof(struct _xcursor));
	bzero(p, sizeof(struct _xcursor));
	
	// Functions mapping
	p->show_flag = xcursor_show_flag;
	p->hide_flag = xcursor_hide_flag;
	p->uninit		= xcursor_uninit;

	return p;
}

#endif /* WITH_XPM */
