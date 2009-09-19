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

#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "window.h"

#include "types.h"
#include "utils.h"
#include "log.h"

#include "selection.h"

extern struct _window *main_window;

/*char* get_selected_text(XSelectionEvent *event)
{
	if (event->property == None)
	{
		log_message(DEBUG, _("Convert to selection target return None answer"));
		return NULL;
	}

	unsigned long len, bytes_left, dummy;
	unsigned char *data = NULL;
	int format;
	Atom type;

	XGetWindowProperty(main_window->display, main_window->window, event->property, 0, 0, 0, AnyPropertyType, &type, &format, &len, &bytes_left, &data);
	if (bytes_left == 0)
	{
		log_message(DEBUG, _("Selected text length is 0"));
		return NULL;
	}

	if (XGetWindowProperty(main_window->display, main_window->window, event->property, 0, bytes_left, 0, AnyPropertyType, &type,&format, &len, &dummy, &data) != Success)
	{
		log_message(ERROR, _("Failed to get selected text data"));
		return NULL;
	}

	return (char *) data;
}

void on_selection_converted(enum _selection_type sel_type)
{
	char *sel_name = "NONE";
	switch (sel_type)
	{
		case SELECTION_PRIMARY:
		{
			sel_name = "PRIMARY";
			break;
		}
		case SELECTION_SECONDARY:
		{
			sel_name = "SECONDARY";
			break;
		}
		case SELECTION_CLIPBOARD:
		{
			sel_name = "CLIPBOARD";
			break;
		}
	}
	Atom selection = XInternAtom(main_window->display, sel_name, FALSE);
	XSetSelectionOwner(main_window->display, selection, None, CurrentTime);
}

void do_selection_notify(enum _selection_type sel_type)
{
	char *sel_name = "NONE";
	switch (sel_type)
	{
		case SELECTION_PRIMARY:
		{
			sel_name = "PRIMARY";
			break;
		}
		case SELECTION_SECONDARY:
		{
			sel_name = "SECONDARY";
			break;
		}
		case SELECTION_CLIPBOARD:
		{
			sel_name = "CLIPBOARD";
			break;
		}
	}
	Atom target = XInternAtom(main_window->display, "UTF8_STRING", FALSE);
	Atom selection = XInternAtom(main_window->display, sel_name, FALSE);

	int status = XConvertSelection(main_window->display, selection, target, None, main_window->window, CurrentTime);
	if (status == BadAtom)
		log_message(ERROR, _("Failed to convert selection with error BadAtom"));
	else if (status == BadWindow)
		log_message(ERROR, _("Failed to convert selection with error BadWindow"));
}*/

static Display * display;
static Window window;

static Atom utf8_atom;
static Atom compound_text_atom;

static unsigned char *wait_selection (Atom selection)
{
	XEvent event;
	Atom target;

	int format;
	unsigned long bytesafter, length;
	unsigned char * value, * retval = NULL;
	int keep_waiting = True;

	while (keep_waiting) 
	{
		XNextEvent (display, &event);

		switch (event.type) 
		{
			case SelectionNotify:
				if (event.xselection.selection != selection) 
					break;

				if (event.xselection.property == None) 
				{
					log_message(WARNING, _("Conversion refused"));
					value = NULL;
					keep_waiting = False;
				} 
				else 
				{
					XGetWindowProperty (event.xselection.display,
						event.xselection.requestor,
						event.xselection.property, 0L, 1000000,
						False, (Atom)AnyPropertyType, &target, &format, &length, &bytesafter, &value);

					if (target != utf8_atom && target != XA_STRING && target != compound_text_atom) 
					{
						/* Report non-TEXT atoms */
						log_message(WARNING, _("Selection is not a string."));
						free (retval);
						retval = NULL;
						keep_waiting = False;
					} 
					else 
					{
						retval = (unsigned char *)strdup ((char *)value);
						XFree (value);
						keep_waiting = False;
					}

					XDeleteProperty (event.xselection.display, event.xselection.requestor, event.xselection.property);
		    	}
				break;
    		default:
     			 break;
    	}
	}
	return retval;
}

static Time get_timestamp (void)
{
  XEvent event;

  XChangeProperty (display, window, XA_WM_NAME, XA_STRING, 8,
                   PropModeAppend, NULL, 0);

  while (1) {
    XNextEvent (display, &event);

    if (event.type == PropertyNotify)
      return event.xproperty.time;
  }
}

static unsigned char *get_selection (Atom selection, Atom request_target)
{
	unsigned char * retval;

	// Get a timestamp 
	XSelectInput (display, window, PropertyChangeMask);
	
	Atom prop = XInternAtom (display, "XSEL_DATA", False);
	Time timestamp = get_timestamp ();
	
	XConvertSelection (display, selection, request_target, prop, window, timestamp);
	XSync (display, False);

	retval = wait_selection (selection);
	
	return retval;
}

unsigned char *get_selection_text (enum _selection_type sel_type)
{
	Atom selection = 0;
	switch (sel_type)
	{
		case SELECTION_PRIMARY:
		{
			selection = XInternAtom(main_window->display, "PRIMARY", FALSE);
			break;
		}
		case SELECTION_SECONDARY:
		{
			selection = XInternAtom(main_window->display, "SECONDARY", FALSE);
			break;
		}
		case SELECTION_CLIPBOARD:
		{
			selection = XInternAtom(main_window->display, "CLIPBOARD", FALSE);
			break;
		}
	}

	Window root;
	int black;
	display = XOpenDisplay (NULL);
	if (display == NULL)
		return NULL;
	root = XDefaultRootWindow (display);
	
	//Create an unmapped window for receiving events
	black = BlackPixel (display, DefaultScreen (display));
	window = XCreateSimpleWindow (display, root, 0, 0, 1, 1, 0, black, black);

	utf8_atom = XInternAtom (display, "UTF8_STRING", FALSE);
	compound_text_atom = XInternAtom (display, "COMPOUND_TEXT", False);

	unsigned char * retval;
	if ((retval = get_selection (selection, utf8_atom)) == NULL)
		retval = get_selection (selection, XA_STRING);

	XDestroyWindow(display, window);
	XCloseDisplay(display);

	return retval;
}