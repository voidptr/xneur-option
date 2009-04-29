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

#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xnconfig.h"

#include "xstring.h"
#include "xswitchlang.h"
#include "xwindow.h"
#include "xutils.h"
#include "xdefines.h"

#include "types.h"
#include "log.h"
#include "conversion.h"

#include "event.h"

extern struct _xwindow *main_window;
extern struct _xneur_config *xconfig;

int get_key_state(int key)
{
	KeyCode key_code = XKeysymToKeycode(main_window->display, key);
	if (key_code == NoSymbol)
		return 0;

	XModifierKeymap *map = XGetModifierMapping(main_window->display);

	int key_mask = 0;
	for (int i = 0; i < 8; i++)
	{
		if (map->modifiermap[map->max_keypermod * i] == key_code)
			key_mask = (1 << i);
	}

	XFreeModifiermap(map);

	if (key_mask == 0)
		return 0;

	Window wDummy;
	int iDummy;
	unsigned int mask;
	XQueryPointer(main_window->display, DefaultRootWindow(main_window->display), &wDummy, &wDummy, &iDummy, &iDummy, &iDummy, &iDummy, &mask);

	return ((mask & key_mask) != 0);
}

XEvent create_basic_event(void)
{
	XEvent event;

	event.type		= KeyPress;
	event.xkey.type		= KeyPress;
	event.xkey.root		= RootWindow(main_window->display, DefaultScreen(main_window->display));
	event.xkey.subwindow	= None;
	event.xkey.same_screen	= True;
	event.xkey.display	= main_window->display;
	event.xkey.state	= 0;
	event.xkey.keycode	= XKeysymToKeycode(main_window->display, XK_space);
	event.xkey.time		= CurrentTime;

	return event;
}

void event_send_xkey(struct _event *p, KeyCode kc, int modifiers)
{
	p->event.type			= KeyPress;
	p->event.xkey.type		= KeyPress;
	p->event.xkey.window		= p->owner_window;
	p->event.xkey.root		= RootWindow(main_window->display, DefaultScreen(main_window->display));
	p->event.xkey.subwindow		= None;
	p->event.xkey.same_screen	= True;
	p->event.xkey.display		= main_window->display;
	p->event.xkey.state		= modifiers;
	p->event.xkey.keycode		= kc;
	p->event.xkey.time		= CurrentTime;

	XSendEvent(main_window->display, p->owner_window, TRUE, KeyPressMask, &p->event);

	/*
	p->event.type			= KeyRelease;
	p->event.xkey.type		= KeyRelease;
	p->event.xkey.time		= CurrentTime;

	if (xconfig->send_delay > 0)
		usleep(xconfig->send_delay);

	XSendEvent(main_window->display, p->owner_window, TRUE, KeyReleaseMask, &p->event);
	*/
}

static void event_send_backspaces(struct _event *p, int count)
{
	for (int i = 0; i < count; i++)
		p->send_xkey(p, p->backspace, None);
}

static void event_send_selection(struct _event *p, int count)
{
	for (int i = 0; i < count; i++)
		p->send_xkey(p, p->left, None);
	for (int i = 0; i < count; i++)
		p->send_xkey(p, p->right, ShiftMask);
}

static void event_send_string(struct _event *p, struct _xstring *str)
{
	for (int i = 0; i < str->cur_pos; i++)
		p->send_xkey(p, str->keycode[i], str->keycode_modifiers[i]);
}

static void event_set_owner_window(struct _event *p, Window window)
{
	p->owner_window = window;
}

static KeySym event_get_cur_keysym(struct _event *p)
{
	return XLookupKeysym(&p->event.xkey, 0);
}

static int event_get_cur_modifiers(struct _event *p)
{
	int mask = 0;
	int key_sym = p->get_cur_keysym(p);

	if ((p->event.xkey.state & ShiftMask) && (key_sym != XK_Shift_L) && (key_sym != XK_Shift_R))
		mask += (1 << 0);
	if ((p->event.xkey.state & LockMask) && (key_sym != XK_Caps_Lock))
		mask += (1 << 1);
	if ((p->event.xkey.state & ControlMask) && (key_sym != XK_Control_L) && (key_sym != XK_Control_R))
		mask += (1 << 2);
	if ((p->event.xkey.state & Mod1Mask) && (key_sym != XK_Alt_L) && (key_sym != XK_Alt_R))
		mask += (1 << 3);
	if ((p->event.xkey.state & Mod2Mask) && (key_sym != XK_Meta_L) && (key_sym != XK_Meta_R))
		mask += (1 << 4);
	if ((p->event.xkey.state & Mod3Mask) && (key_sym != XK_Num_Lock))
		mask += (1 << 5);
	if ((p->event.xkey.state & Mod4Mask) && (key_sym != XK_Hyper_L) && (key_sym != XK_Hyper_R))
		mask += (1 << 6);
	if ((p->event.xkey.state & Mod5Mask) && (key_sym != XK_Super_L) && (key_sym != XK_Super_R))
		mask += (1 << 7);

	/*if (p->event.xkey.state & ShiftMask)
		mask += (1 << 0);
	if (p->event.xkey.state & LockMask)
		mask += (1 << 1);
	if (p->event.xkey.state & ControlMask)
		mask += (1 << 2);
	if (p->event.xkey.state & Mod1Mask)
		mask += (1 << 3);
	if (p->event.xkey.state & Mod2Mask)
		mask += (1 << 4);
	if (p->event.xkey.state & Mod3Mask)
		mask += (1 << 5);
	if (p->event.xkey.state & Mod4Mask)
		mask += (1 << 6);
	if (p->event.xkey.state & Mod5Mask)
		mask += (1 << 7);*/
	return mask;
}

static int event_get_next_event(struct _event *p)
{
	XNextEvent(main_window->display, &(p->event));
	return p->event.type;
}

static void event_send_next_event(struct _event *p)
{
	p->event.xkey.state = p->get_cur_modifiers(p) | groups[get_active_keyboard_group()];
	XSendEvent(main_window->display, p->event.xany.window, TRUE, NoEventMask, &p->event);
}

static void event_uninit(struct _event *p)
{
	free(p);

	log_message(DEBUG, _("Event is freed"));
}

struct _event* event_init(void)
{
	struct _event *p = (struct _event *) malloc(sizeof(struct _event));
	bzero(p, sizeof(struct _event));

	p->backspace		= XKeysymToKeycode(main_window->display, XK_BackSpace);
	p->left			= XKeysymToKeycode(main_window->display, XK_Left);
	p->right		= XKeysymToKeycode(main_window->display, XK_Right);

	// Functions mapping
	p->get_next_event	= event_get_next_event;
	p->send_next_event	= event_send_next_event;
	p->set_owner_window	= event_set_owner_window;
	p->send_xkey		= event_send_xkey;
	p->send_string		= event_send_string;
	p->get_cur_keysym	= event_get_cur_keysym;
	p->get_cur_modifiers	= event_get_cur_modifiers;
	p->send_backspaces	= event_send_backspaces;
	p->send_selection	= event_send_selection;
	p->uninit		= event_uninit;

	return p;
}
