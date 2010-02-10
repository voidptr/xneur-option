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

#include <X11/XKBlib.h>

#include <stdlib.h>
#include <string.h>

#include "xnconfig.h"

#include "window.h"
#include "keymap.h"

#include "types.h"
#include "utils.h"
#include "log.h"

#include "switchlang.h"

extern struct _xneur_config *xconfig;
extern struct _window *main_window;

int get_active_keyboard_group(void)
{
	XkbStateRec xkbState;
	XkbGetState(main_window->display, XkbUseCoreKbd, &xkbState);
	return xkbState.group;
}

int get_cur_lang(void)
{
	int group = get_active_keyboard_group();

	int lang = xconfig->find_group_lang(xconfig, group);
	if (lang != -1)
		return lang;

	log_message(ERROR, _("Can't find language for this XKB Group"));
	return 0;
}

void switch_lang(int new_lang)
{
	XkbLockGroup(main_window->display, XkbUseCoreKbd, xconfig->get_lang_group(xconfig, new_lang));
}

void switch_group(int new_group)
{
	XkbLockGroup(main_window->display, XkbUseCoreKbd, new_group);
}

void set_next_keyboard_group(void)
{
	int new_layout_group = get_active_keyboard_group() + 1;
	if (new_layout_group == xconfig->total_languages - 1)
		new_layout_group = 0;
	switch_group (new_layout_group);
}
