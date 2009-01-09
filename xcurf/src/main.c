/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Andrew Crew Kuznetsov 2009 <>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include <Imlib2.h>

#define TRUE 1
#define FALSE 0

#define MWM_HINTS_DECORATIONS   (1L << 1) 
#define PROP_MWM_HINTS_ELEMENTS 5

typedef struct {
	int flags;
	int functions;
	int decorations;
	int input_mode;
	int status;
} MWMHints;

Display *display;

Window parent_window;
Window flag_window;

int last_xkb_group = -1;

Visual  *vis;
Colormap cm;
int depth;

Imlib_Image image, buffer;
Imlib_Updates updates, current_update;
int w = 0, h = 0;

char *pixmaps[4];

typedef void (*sg_handler)(int);
void signal_trap(int sig, sg_handler handler)
{
	struct sigaction sa;
	bzero(&sa, sizeof(sa));
	sa.sa_handler = handler;

	if (sigaction(sig, &sa, NULL) == -1)
	{
		printf("Can't trap signal\n");
		exit(1);
	}
}

static void xcf_exit (int status)
{
	if (status) {};
	
	printf("\nCaught SIGTERM/SIGINT, terminating\n");
	exit(0);
}

void set_event_mask(Window window, int event_mask)
{
	if (window == flag_window)
		return;
	
	XSelectInput(display, window, event_mask);
}

static void set_mask_to_window(Window current_window, int mask)
{
	if (current_window == None)
		return;
	
	set_event_mask(current_window, mask);
	
	unsigned int children_count;
	Window root, parent;
	Window *children;
	
	int is_same_screen = XQueryTree(display, current_window, &root, &parent, &children, &children_count);
	if (!is_same_screen)
		return;
	
	unsigned int i;
	for (i = 0; i < children_count; i++)
		set_mask_to_window(children[i], mask);
	
	XFree(children);
}

static void update_focus (void)
{
	Window new_window;
	while (TRUE)
	{
		int revert_to;
		XGetInputFocus(display, &new_window, &revert_to);

		// Catch not empty and not system window 
		if (new_window)
			break;

		printf("New window empty\n");
		usleep(1000);
	}
	
	printf("Focused window %d\n", (int) new_window);
	
	// Up to heighted window
	parent_window = new_window;
	while (TRUE)
	{
		unsigned int children_count;
		Window root, parent;
		Window *children;

		int is_same_screen = XQueryTree(display, parent_window, &root, &parent, &children, &children_count);
		if (!is_same_screen || parent == None || parent == root)
			break;
		
		parent_window = parent;
		XFree(children);
	}	
	
	set_mask_to_window(parent_window, PointerMotionMask | PropertyChangeMask | FocusChangeMask | KeyReleaseMask);
}

static void cursor_hide(XWindowAttributes w_attributes)
{
	if (w_attributes.map_state != IsUnmapped)
		XUnmapWindow(display, flag_window);
	XFlush(display);
}

void cursor_show(void)
{
	XWindowAttributes w_attributes;
	XGetWindowAttributes(display, flag_window, &w_attributes);

	// For set pixmap to window, need map window
	if (w_attributes.map_state == IsUnmapped)
		XMapWindow(display, flag_window);
	
	XkbStateRec xkbState;
	XkbGetState(display, XkbUseCoreKbd, &xkbState);
	
	// if for layout not defined xpm file then unmap window and stop procedure
	if (pixmaps[xkbState.group] == NULL)
	{
		cursor_hide(w_attributes);
		return;
	}

	// if current layout not equal last layout then load new pixmap
	if (last_xkb_group != xkbState.group)
	{
		printf("Group %d\n", xkbState.group);
		last_xkb_group = xkbState.group;

		if (pixmaps[xkbState.group] == NULL)
		{
			cursor_hide(w_attributes);
			return;
		}
		
		image = imlib_load_image(pixmaps[xkbState.group]);
		
		if (image == NULL)
		{
			cursor_hide(w_attributes);
			return;
		}

		imlib_context_set_image(image);
		w = imlib_image_get_width();
		h = imlib_image_get_height();
		XResizeWindow(display, flag_window, w, h);

	}	

	imlib_render_image_on_drawable_at_size(0, 0, w, h);
	XRaiseWindow(display, flag_window);
	
	int root_x, root_y, win_x, win_y;
	Window root, child;
	unsigned int dummyU;
	XQueryPointer(display, parent_window, &root, &child, &root_x, &root_y, &win_x, &win_y, &dummyU);

	XMoveWindow(display, flag_window, root_x + 10, root_y + 10);

	XFlush(display);
}

int main(int argc, char *argv[])
{
	// Set hook to terminate
	signal_trap(SIGTERM, xcf_exit);
	signal_trap(SIGINT, xcf_exit);
	
	// Init pixmaps
	int i;
	for (i=0; i<4; i++)
		pixmaps[i] = NULL;
	for (i=1; i<argc; i++)
	{
		pixmaps[i - 1] = malloc(strlen(argv[i]) * sizeof(char));
		if (pixmaps[i - 1] == NULL)
			exit(1);
		memcpy(pixmaps[i - 1], argv[i], strlen(argv[i]) * sizeof(char));
		printf("%s\n",pixmaps[i - 1]);
	}
	
	// Open display
	display = XOpenDisplay(NULL);
	if (!display)
	{
		printf("Can't connect to XServer\n");
		return (1);
	}
	
	// Init main window
	Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0);
	if (!window)
	{
		printf("Can't create program window\n");
		XCloseDisplay(display);
		return (1);
	}
	
	// Create flag window
	XSetWindowAttributes attrs;
	attrs.override_redirect = True;

	flag_window = XCreateWindow(display, DefaultRootWindow(display), 0, 0, 1, 1,0, CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect, &attrs);
	if (!flag_window)
	{
		printf("Can't create flag window\n");
		XCloseDisplay(display);
		return (1);
	}

	// Set no border mode to flag window
	MWMHints mwmhints;
	bzero(&mwmhints, sizeof(mwmhints));
	mwmhints.flags		= MWM_HINTS_DECORATIONS;
	mwmhints.decorations	= 0;

	Atom motif_prop = XInternAtom(display, "_MOTIF_WM_HINTS", False);
	
	XChangeProperty(display, flag_window, motif_prop, motif_prop, 32, PropModeReplace, (unsigned char *) &mwmhints, PROP_MWM_HINTS_ELEMENTS);
	
	XWMHints wmhints;
	bzero(&wmhints, sizeof(wmhints));
	wmhints.flags = InputHint;
	wmhints.input = 0;

	Atom win_prop = XInternAtom(display, "_WIN_HINTS", False);

	XChangeProperty(display, flag_window, win_prop, win_prop, 32, PropModeReplace, (unsigned char *) &mwmhints, sizeof (XWMHints) / 4);

	printf("Main window with id %d created\n", (int) window);

	// Init image environment
	vis   = DefaultVisual(display, DefaultScreen(display));
	cm    = DefaultColormap(display, DefaultScreen(display));

	imlib_set_cache_size(2048 * 1024);

	imlib_context_set_display(display);
	imlib_context_set_visual(vis);
	imlib_context_set_colormap(cm);
	imlib_context_set_drawable(flag_window);

	imlib_context_set_dither(1);
	
	// 
	XSynchronize(display, TRUE);
	XFlush(display);

	update_focus();
	XEvent e;
	while (TRUE)
	{
		XNextEvent(display, &e);
		switch (e.type)
		{
			case FocusIn:
			case FocusOut:	
			{
				update_focus();
				cursor_show();
				break;
			}
			case MotionNotify:
			{

				cursor_show();
				break;
			}
			case KeyRelease:
			{

				cursor_show();
				break;
			}
		}
	}
	
	return (0);
}
