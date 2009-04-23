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

#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>

#include "xwindow.h"

#include "types.h"
#include "utils.h"
#include "log.h"

#include "xselection.h"

extern struct _xwindow *main_window;

char* get_selected_text(XSelectionEvent *event)
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
}

