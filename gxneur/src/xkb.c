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

#include "xkb.h"
#include <string.h>

int get_active_kbd_group(void)
{
	Display *dpy = XOpenDisplay(NULL);

	XkbStateRec xkbState;
	XkbGetState(dpy, XkbUseCoreKbd, &xkbState);
	XCloseDisplay(dpy);

	int group = xkbState.group;
	return group;
}

int get_kbd_group_count(void) 
{
	Display *dpy = XOpenDisplay(NULL);
    XkbDescRec desc[1];
    int gc;
    memset(desc, 0, sizeof(desc));
    desc->device_spec = XkbUseCoreKbd;
    XkbGetControls(dpy, XkbGroupsWrapMask, desc);
    XkbGetNames(dpy, XkbGroupNamesMask, desc);
	gc = desc->ctrls->num_groups;
    XkbFreeControls(desc, XkbGroupsWrapMask, True);
    XkbFreeNames(desc, XkbGroupNamesMask, True);
	XCloseDisplay(dpy);
    return gc;
}

int set_next_kbd_group(void)
{
	Display *dpy = XOpenDisplay(NULL);

	int active_layout_group = get_active_kbd_group();
	
	int new_layout_group = active_layout_group + 1;
	if (new_layout_group == get_kbd_group_count())
		new_layout_group = 0;

	XkbLockGroup(dpy, XkbUseCoreKbd, new_layout_group);
	XCloseDisplay(dpy);
	return 1;
}
