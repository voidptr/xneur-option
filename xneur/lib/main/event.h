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
 *  Copyright (C) 2006-2009 XNeur Team
 *
 */

#ifndef _EVENT_H_
#define _EVENT_H_

#include <X11/Xutil.h>

#include "xstring.h"

int get_key_state(int key);
XEvent create_basic_event(void);

struct _event
{
	Window owner_window;		// Window that recieves/sends event
	XEvent event;			// Event to process
	XEvent default_event;
	KeyCode backspace;		// Backspace key code
	KeyCode left;			// Left Arrow key code
	KeyCode right;			// Right Arrow key code

	KeySym (*get_cur_keysym) (struct _event *p);
	int (*get_cur_modifiers) (struct _event *p);

	int  (*get_next_event) (struct _event *p);
	void (*send_next_event) (struct _event *p);
	void (*set_owner_window) (struct _event *p, Window window);
	void (*send_xkey) (struct _event *p, KeyCode kc, int modifiers);
	void (*send_string) (struct _event *p, struct _xstring *str);
	void (*send_backspaces) (struct _event *p, int n);
	void (*send_selection) (struct _event *p, int n);
	void (*uninit) (struct _event *p);
};

struct _event* event_init(void);

#endif /* _EVENT_H_ */
