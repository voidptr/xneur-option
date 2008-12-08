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

#include "log.h"
#include "types.h"

#ifdef WITH_IMAGE

#include <X11/Xutil.h>
#include <X11/XKBlib.h>

#include <unistd.h>
#include <stdio.h>

#include "xnconfig_files.h"

#include "xwindow.h"
extern struct _xneur_config *xconfig;
extern struct _xwindow *main_window;
int last_layuot;

#ifdef WITH_XPM

#include <X11/xpm.h>

Pixmap bitmap;
Pixmap bitmap_mask;
XpmAttributes Attrs;

GC gc;
	
#elif WITH_IMLIB2

#include <Imlib2.h>

Visual  *vis;
Colormap cm;
int depth;

Imlib_Image image, buffer;
Imlib_Updates updates, current_update;
int w = 0, h = 0;

#endif

static void unmap_window(XWindowAttributes w_attributes)
{
	if (w_attributes.map_state != IsUnmapped)
		XUnmapWindow(main_window->display, main_window->flag_window);
	XFlush(main_window->display);
}

#ifdef WITH_XPM

static int ReadPixmapFromFile(char *FileName, Pixmap *phPm, Pixmap *phMask, XpmAttributes *pAttrs, Colormap hColorMap)
{
	FILE *pFile = fopen(FileName, "r");
	if (!pFile)
		return -1;

	fseek(pFile, 0, SEEK_END);
	long sizeFile = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	char *buffer = (char *) malloc(sizeFile * sizeof(char));
	if (!buffer)
	{
		fclose(pFile);
		return -1;
	}

	fread(buffer, 1, sizeFile, pFile);
	fclose(pFile);

	// Check Signature
	if (strncmp(buffer, "/* XPM */", 9))
	{
		free(buffer);
		return -1;
	}

	long N = 0;
	for (long i = 0; i < sizeFile; i++)
	N += (buffer[i] == '"' ? 1 : 0);
	N /= 2;

	char **xpm = (char **) malloc(N * sizeof(char *));
	for (long k = 0, i = 0; i < sizeFile; i++)
	{
		if (buffer[i] != '"')
			continue;
		xpm[k++] = buffer + ++i;
		while (buffer[i] != '"')
			i++;
		buffer[i] = '\0';
	}

	pAttrs->valuemask	= XpmColormap | XpmCloseness;
	pAttrs->colormap	= hColorMap;
	pAttrs->closeness	= 65535;
	XpmCreatePixmapFromData(main_window->display, main_window->flag_window, xpm, phPm, phMask, pAttrs);

	free(xpm);
	free(buffer);
	return 0;
}
static GC create_gc()
{
	XGCValues values;			// Initial values for the GC
	unsigned long valuemask = 0;		// Which values in 'values' to check when creating the GC

	GC gc = XCreateGC(main_window->display, main_window->flag_window, valuemask, &values);
	if (!gc)
	{
		log_message(ERROR, _("Can't create GC"));
		return NULL;
	}
	int screen_num = DefaultScreen(main_window->display);
	// Allocate foreground and background colors for this GC
	XSetForeground(main_window->display, gc, WhitePixel(main_window->display, screen_num));
	XSetBackground(main_window->display, gc, BlackPixel(main_window->display, screen_num));

	int line_width	= 2;			// Line width for the GC
	int line_style	= LineSolid;		// Style for lines drawing
	int cap_style	= CapButt;		// Style of the line's edje
	int join_style	= JoinBevel;		// Joined lines

	// Define the style of lines that will be drawn using this GC
	XSetLineAttributes(main_window->display, gc, (unsigned int) line_width, line_style, cap_style, join_style);

	// Define the fill style for the GC to be 'solid filling'
	XSetFillStyle(main_window->display, gc, FillSolid);

	return gc;
}

void xcursor_show_flag(int x, int y)
{
	XWindowAttributes w_attributes;
	XGetWindowAttributes(main_window->display, main_window->flag_window, &w_attributes);

	XkbStateRec xkbState;
	XkbGetState(main_window->display, XkbUseCoreKbd, &xkbState);
	
	// if for layout not defined xpm file then unmap window and stop procedure
	if (xconfig->flags[xkbState.group].file == NULL)
	{
		unmap_window(w_attributes);
		return;
	}

	// if current layout not equal last layout then load new pixmap
	if (last_layuot != xkbState.group)
	{
		last_layuot = xkbState.group;

		XFreePixmap(main_window->display, bitmap);
		XFreePixmap(main_window->display, bitmap_mask);
		bitmap = 0;
		bitmap_mask = 0;

		char *path = get_file_path_name(PIXMAPDIR, xconfig->flags[xkbState.group].file);
		if (path == NULL)
		{
			unmap_window(w_attributes);
			return;
		}

		ReadPixmapFromFile(path, &bitmap, &bitmap_mask, &Attrs, XDefaultColormap(main_window->display, DefaultScreen(main_window->display)));
		free(path);

		if (bitmap == 0)
		{
			unmap_window(w_attributes);
			return;
		}
	}

	// For set pixmap to window, need map window
	if (w_attributes.map_state == IsUnmapped)
		XMapWindow(main_window->display, main_window->flag_window);

	XSetClipMask(main_window->display, gc, bitmap_mask);
	XSetClipOrigin(main_window->display, gc, 0, 0);
	XCopyArea(main_window->display, bitmap, main_window->flag_window, gc, 0, 0, Attrs.width, Attrs.height, 0, 0);

	XRaiseWindow(main_window->display, main_window->flag_window);
	XMoveResizeWindow(main_window->display, main_window->flag_window, x, y, Attrs.width, Attrs.height);

	XFlush(main_window->display);
}

void xcursor_hide_flag(void)
{
	XWindowAttributes w_attributes;
	XGetWindowAttributes(main_window->display, main_window->flag_window, &w_attributes);

	unmap_window(w_attributes);
}

void xcursor_uninit(struct _xcursor *p)
{
	Display *display = XOpenDisplay(NULL);

	XFreePixmap(display, bitmap);
	XFreePixmap(display, bitmap_mask);
	XFreeGC(display, gc);
	XCloseDisplay(display);

	free(p);

	log_message(DEBUG, _("Current cursor is freed"));
}

struct _xcursor* xcursor_init(void)
{
	struct _xcursor *p = (struct _xcursor *) malloc(sizeof(struct _xcursor));
	bzero(p, sizeof(struct _xcursor));
	gc = create_gc();
	if (gc == NULL)
	{
		free(p);
		return NULL;
	}
	XSync(main_window->display, False);
	last_layuot = -1;

	// Functions mapping
	p->show_flag	= xcursor_show_flag;
	p->hide_flag	= xcursor_hide_flag;
	p->uninit	= xcursor_uninit;

	return p;
}

#elif WITH_IMLIB2 /* WITH_IMLIB2 */

void xcursor_show_flag(int x, int y)
{
	XWindowAttributes w_attributes;
	XGetWindowAttributes(main_window->display, main_window->flag_window, &w_attributes);

	// For set pixmap to window, need map window
	if (w_attributes.map_state == IsUnmapped)
		XMapWindow(main_window->display, main_window->flag_window);
	
	XkbStateRec xkbState;
	XkbGetState(main_window->display, XkbUseCoreKbd, &xkbState);
	
	// if for layout not defined xpm file then unmap window and stop procedure
	if (xconfig->flags[xkbState.group].file == NULL)
	{
		unmap_window(w_attributes);
		return;
	}

	// if current layout not equal last layout then load new pixmap
	if (last_layuot != xkbState.group)
	{
		last_layuot = xkbState.group;

		char *path = get_file_path_name(PIXMAPDIR, xconfig->flags[xkbState.group].file);
		if (path == NULL)
		{
			unmap_window(w_attributes);
			return;
		}

		image = imlib_load_image(path);
		free(path);

		if (image == NULL)
		{
			unmap_window(w_attributes);
			return;
		}

		imlib_context_set_image(image);
		w = imlib_image_get_width();
		h = imlib_image_get_height();
		XResizeWindow(main_window->display, main_window->flag_window, w, h);
	}	
	
	imlib_render_image_on_drawable_at_size(0, 0, w, h);
	XRaiseWindow(main_window->display, main_window->flag_window);
	XMoveWindow(main_window->display, main_window->flag_window, x, y);

	XFlush(main_window->display);
}

void xcursor_hide_flag(void)
{
	XWindowAttributes w_attributes;
	XGetWindowAttributes(main_window->display, main_window->flag_window, &w_attributes);

	unmap_window(w_attributes);
}

void xcursor_uninit(struct _xcursor *p)
{
	if (image)
		imlib_free_image();
	free(p);
	log_message(DEBUG, _("Current cursor is freed"));
}

struct _xcursor* xcursor_init(void)
{
	struct _xcursor *p = (struct _xcursor *) malloc(sizeof(struct _xcursor));
	bzero(p, sizeof(struct _xcursor));
	
	last_layuot = -1;
	
	vis   = DefaultVisual(main_window->display, DefaultScreen(main_window->display));
	cm    = DefaultColormap(main_window->display, DefaultScreen(main_window->display));

	imlib_set_cache_size(2048 * 1024);

	imlib_context_set_display(main_window->display);
	imlib_context_set_visual(vis);
	imlib_context_set_colormap(cm);
	imlib_context_set_drawable(main_window->flag_window);

	imlib_context_set_dither(1);
	
	// Functions mapping
	p->show_flag	= xcursor_show_flag;
	p->hide_flag	= xcursor_hide_flag;
	p->uninit	= xcursor_uninit;

	return p;
}

#endif

#else /* WITH_IMAGE */

void xcursor_show_flag(int x, int y)
{
	if (x || y) {};
	return;
}

void xcursor_hide_flag(void)
{
}

void xcursor_uninit(struct _xcursor *p)
{
	free(p);
	log_message(DEBUG, _("Current cursor is freed"));
}

struct _xcursor* xcursor_init(void)
{
	struct _xcursor *p = (struct _xcursor *) malloc(sizeof(struct _xcursor));
	bzero(p, sizeof(struct _xcursor));
	
	// Functions mapping
	p->show_flag	= xcursor_show_flag;
	p->hide_flag	= xcursor_hide_flag;
	p->uninit	= xcursor_uninit;

	return p;
}

#endif /* WITH_IMAGE */
