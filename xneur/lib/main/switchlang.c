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
 *  Copyright (C) 2006-2010 XNeur Team
 *
 */

#include <X11/XKBlib.h>

#include <stdlib.h>
#include <string.h>

#include "xneur.h"

#include "window.h"
#include "keymap.h"

#include "types.h"
#include "utils.h"
#include "log.h"

#include "switchlang.h"

extern struct _window *main_window;

int get_curr_keyboard_group(void)
{
	XkbStateRec xkbState;
	Display *display = XOpenDisplay(NULL);
	XkbGetState(display, XkbUseCoreKbd, &xkbState);
	XCloseDisplay(display);
	int group = xkbState.group;
	//XFree(xkbState);
	return group;
}

void set_next_keyboard_group(struct _xneur_handle *handle)
{
	int new_layout_group = get_curr_keyboard_group() + 1;
	if (new_layout_group == handle->total_languages)
		new_layout_group = 0;
	XkbLockGroup(main_window->display, XkbUseCoreKbd, new_layout_group);
}

void set_prev_keyboard_group(struct _xneur_handle *handle)
{
	int new_layout_group = get_curr_keyboard_group() - 1;
	if (new_layout_group < 0)
		new_layout_group = handle->total_languages - 1;
	XkbLockGroup(main_window->display, XkbUseCoreKbd, new_layout_group);
}
