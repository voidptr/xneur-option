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

#include <X11/Xutil.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xnconfig.h"

#include "utils.h"
#include "window.h"
#include "defines.h"

#include "types.h"
#include "list_char.h"
#include "log.h"

#include "focus.h"

extern struct _xneur_config *xconfig;
extern struct _window *main_window;

const char *verbose_forced_mode[]	= {"Default", "Manual", "Automatic"};
const char *verbose_focus_status[]	= {"Processed", "Changed Focus", "Unchanged Focus", "Excluded"};

static void grab_mouse_button (Window current_window, int grab)
{
	if (current_window == None)
		return;

	grab_button(current_window, grab);

	unsigned int children_count;
	Window root_window, parent_window;
	Window *children_return;

	int is_same_screen = XQueryTree(main_window->display, current_window, &root_window, &parent_window, &children_return, &children_count);
	if (!is_same_screen)
		return;

	for (unsigned int i = 0; i < children_count; i++)
		grab_mouse_button (children_return[i], grab);

	XFree(children_return);
}

// Private
static int get_focus(struct _focus *p, int *forced_mode, int *focus_status, int *autocomplementation_mode)
{
	*forced_mode	= FORCE_MODE_NORMAL;
	*focus_status	= FOCUS_NONE;
	*autocomplementation_mode	= AUTOCOMPLEMENTATION_INCLUDED;
	
	Window new_window;
	int show_message = TRUE;
	while (TRUE)
	{
		int revert_to;
		XGetInputFocus(main_window->display, &new_window, &revert_to);

		// Catch not empty and not system window
		if (new_window != None && new_window > 1000)
			break;

		if (show_message)
		{
			log_message(DEBUG, _("New window empty"));
			show_message = FALSE;
		}
		usleep(10000);
	}

	char *new_app_name = get_wm_class_name(new_window);
	if (new_app_name != NULL)
	{
		if (xconfig->excluded_apps->exist(xconfig->excluded_apps, new_app_name, BY_PLAIN))
			*focus_status = FOCUS_EXCLUDED;

		if (xconfig->auto_apps->exist(xconfig->auto_apps, new_app_name, BY_PLAIN))
			*forced_mode = FORCE_MODE_AUTO;
		else if (xconfig->manual_apps->exist(xconfig->manual_apps, new_app_name, BY_PLAIN))
			*forced_mode = FORCE_MODE_MANUAL;

		if (xconfig->autocomplementation_excluded_apps->exist(xconfig->autocomplementation_excluded_apps, new_app_name, BY_PLAIN))
			*autocomplementation_mode	= AUTOCOMPLEMENTATION_EXCLUDED;
	}

	Window old_window = p->owner_window;
	if (new_window == old_window)
	{
		if (new_app_name != NULL)
			free(new_app_name);
		return FOCUS_UNCHANGED;
	}

	log_message(DEBUG, _("Focused window %d"), new_window);
	// Up to heighted window
	p->parent_window = new_window;
	while (TRUE)
	{
		unsigned int children_count;
		Window root_window, parent_window;
		Window *children_return;

		int is_same_screen = XQueryTree(main_window->display, p->parent_window, &root_window, &parent_window, &children_return, &children_count);
		if (!is_same_screen || parent_window == None || parent_window == root_window)
			break;

		p->parent_window = parent_window;
		XFree(children_return);
	}

	// Clear masking on unfocused window
	p->update_events(p, LISTEN_DONTGRAB_INPUT);
	// Replace unfocused window to focused window
	p->owner_window = new_window;

	log_message(DEBUG, _("Process new window (ID %d) with name '%s' (status %s, mode %s)"), new_window, new_app_name, verbose_focus_status[*focus_status], verbose_forced_mode[*forced_mode]);

	if (new_app_name != NULL)
		free(new_app_name);
	return FOCUS_CHANGED;
}

static int focus_get_focus_status(struct _focus *p, int *forced_mode, int *focus_status, int *autocomplementation_mode)
{
	int focus = get_focus(p, forced_mode, focus_status, autocomplementation_mode);

	if (focus == FOCUS_UNCHANGED)
		return p->last_focus;

	if (focus == FOCUS_CHANGED)
		p->last_focus = FOCUS_UNCHANGED;
	else
		p->last_focus = focus;

	return focus;
}

static void focus_update_events(struct _focus *p, int mode)
{
	if (mode == LISTEN_DONTGRAB_INPUT)
	{
		// Event unmasking
		grab_mouse_button(p->parent_window, FALSE);
		set_event_mask(p->owner_window, FOCUS_CHANGE_MASK);

		// Ungrabbing special key (Enter, Tab and other)
		grab_spec_keys(p->owner_window, FALSE);
	}
	else
	{
		// Event masking
		grab_mouse_button(p->parent_window, TRUE);
		set_event_mask(p->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);

		// Grabbing special key (Enter, Tab and other)
		grab_spec_keys(p->owner_window, TRUE);
	}

	p->last_parent_window = p->parent_window;
}

static void focus_uninit(struct _focus *p)
{
	free(p);

	log_message(DEBUG, _("Focus is freed"));
}

struct _focus* focus_init(void)
{
	struct _focus *p = (struct _focus *) malloc(sizeof(struct _focus));
	bzero(p, sizeof(struct _focus));

	// Functions mapping
	p->get_focus_status	= focus_get_focus_status;
	p->update_events	= focus_update_events;
	p->uninit		= focus_uninit;

	return p;
}
