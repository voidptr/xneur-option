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

#include <stdlib.h>
#include <strings.h>

#include "xutils.h"
#include "xdefines.h"
#include "xkeymap.h"

#include "types.h"
#include "log.h"

#include "xwindow.h"

#define MWM_HINTS_DECORATIONS   (1L << 1) 
#define PROP_MWM_HINTS_ELEMENTS 5
typedef struct {
  int  flags;
  int  functions;
  int  decorations;
  int   input_mode;
  int  status;
} MWMHints;

static int error_handler(Display *d, XErrorEvent *e)
{
	if (d || e) {}
        return FALSE;
}

int xwindow_create(struct _xwindow *p)
{
	XSetErrorHandler(error_handler);

	Display *display = XOpenDisplay(NULL);
	if (!display)
	{
		log_message(ERROR, "Can't connect to XServer");
		return FALSE;
	}

	// Create Main Window
	Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0);
	if (!window)
	{
		log_message(ERROR, "Can't create program window");
		XCloseDisplay(display);
		return FALSE;
	}

	// Create flag window
	Window flag_window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 27, 18, 0, 0, 4095);
	if (!flag_window)
	{
		log_message(ERROR, "Can't create flag window");
		XCloseDisplay(display);
		return FALSE;
	}
	// Set no border mode to flag window
	MWMHints mwmhints;
	Atom prop;
	bzero(&mwmhints, sizeof(mwmhints));
	prop = XInternAtom(display, "_MOTIF_WM_HINTS", False);
	mwmhints.flags = MWM_HINTS_DECORATIONS;
	mwmhints.decorations = 0;
	XChangeProperty(display, flag_window, prop, prop, 32, PropModeReplace, (unsigned char *) &mwmhints, PROP_MWM_HINTS_ELEMENTS);
	
	//
	p->display = display;
	p->window  = window;
	p->flag_window  = flag_window;
	
	log_message(LOG, "Main window with id %d created", window);

	XSynchronize(display, TRUE);
	XFlush(display);

	return TRUE;
}

void xwindow_destroy(struct _xwindow *p)
{
	if (p->window == None)
		return;
	
	p->window = None;
	p->flag_window = None;
}

int xwindow_init_keymap(struct _xwindow *p)
{
	p->xkeymap = xkeymap_init();
	if (!p->xkeymap)
		return FALSE;

	return TRUE;
}

void xwindow_uninit(struct _xwindow *p)
{
	if (p->xkeymap != NULL)
		p->xkeymap->uninit(p->xkeymap);

	p->destroy(p);

	free(p);
}

struct _xwindow* xwindow_init(void)
{
	struct _xwindow *p = (struct _xwindow *) malloc(sizeof(struct _xwindow));
	bzero(p, sizeof(struct _xwindow));

	// Function mapping
	p->create		= xwindow_create;
	p->destroy		= xwindow_destroy;
	p->init_keymap		= xwindow_init_keymap;
	p->uninit		= xwindow_uninit;

	return p;
}
