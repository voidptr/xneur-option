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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include <stdio.h>

#include "xnconfig.h"

extern struct _xneur_config *xconfig;

int on_init(void)
{
	printf("Plugin receive initialisation");
	return (0);
}

int on_xneur_start(void)
{
	printf("Plugin receive xneur start");
	return (0);
}

int on_xneur_reload(void)
{
	printf("Plugin receive xneur stop");
	return (0);
}

int on_xneur_stop(void)
{
	printf("Plugin receive xneur stop");
	return (0);
}

int on_key_press(KeySym key, int modifier_mask)
{
	printf("Plugin receive KeyPress '%s' with mask %d\n", XKeysymToString(key), modifier_mask);
	return (0);
}

int on_key_release(KeySym key, int modifier_mask)
{
	printf("Plugin receive KeyRelease '%s' with mask %d\n", XKeysymToString(key), modifier_mask);
	return (0);
}

int on_hotkey_action(enum _hotkey_action ha)
{
	printf("Plugin receive Hotkey Action '%d'\n", ha);
	return (0);
}

int on_change_action(enum _change_action ca)
{
	printf("Plugin receive Change Action '%d'\n", ca);
	return (0);
}

int on_fini(void)
{
	printf("Plugin receive finalisation");
	return (0);
}