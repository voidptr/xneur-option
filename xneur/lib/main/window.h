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

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <X11/Xutil.h>

struct _window
{
	struct _keymap *keymap;

	Display *display;
	Window window;

	Window flag_window;

	Atom close_atom;
	Atom protocols_atom;

	int  (*create) (struct _window *p);
	void (*destroy) (struct _window *p);
	void (*move_window) (struct _window *p, int x, int y);
	int  (*init_keymap) (struct _window *p);
	void (*uninit) (struct _window *p);
};

struct _window* window_init(void);

#endif /* _WINDOW_H_ */
