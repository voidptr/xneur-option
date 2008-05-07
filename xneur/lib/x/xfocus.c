/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  (c) XNeur Team 2006
 *
 */

#include <X11/Xutil.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xnconfig.h"

#include "xutils.h"
#include "xwindow.h"
#include "xdefines.h"

#include "types.h"
#include "list_char.h"
#include "log.h"

#include "xfocus.h"

extern struct _xneur_config *xconfig;
extern struct _xwindow *main_window;

const char *verbose_forced_mode[] = {"Default", "Manual", "Automatic"};
const char *verbose_focus_status[] = {"Processed", "Changed Focus", "Unchanged Focus", "Excluded"};

static int get_focus(struct _xfocus *p, int *forced_mode, int *focus_status)
{
	*forced_mode = FORCE_MODE_NORMAL;
	*focus_status = FOCUS_NONE;

	if (p->last_parent_window != None)
	{
		grab_button(p->last_parent_window, FALSE);
		//grab_enter_key(p->last_parent_window, FALSE);
	}
	
	Window new_window;
	while (TRUE)
	{
		int revert_to;
		XGetInputFocus(main_window->display, &new_window, &revert_to);

		if (new_window != None && new_window > 100)
			break;

		log_message(DEBUG, "New window empty");
		usleep(500);
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
	}

	Window old_window = p->owner_window;
	if (new_window == old_window)
	{
		if (new_app_name != NULL)
			free(new_app_name);
		return FOCUS_UNCHANGED;
	}

	p->owner_window = new_window;

	log_message(DEBUG, "Process new window (ID %d) with name '%s' (status %s, mode %s)", new_window, new_app_name, verbose_focus_status[*focus_status], verbose_forced_mode[*forced_mode]);
	
	if (new_app_name != NULL)
		free(new_app_name);
	return FOCUS_CHANGED;
}

int xfocus_get_focus_status(struct _xfocus *p, int *forced_mode, int *focus_status)
{
	int focus = get_focus(p, forced_mode, focus_status);

	if (focus == FOCUS_UNCHANGED)
		return p->last_focus;

	if (focus == FOCUS_CHANGED)
		p->last_focus = FOCUS_UNCHANGED;
	else
		p->last_focus = focus;

	return focus;
}

void xfocus_update_events(struct _xfocus *p, int mode)
{
	Window parent_window = p->owner_window;

	int mask = FOCUS_CHANGE_MASK;
	if (mode == LISTEN_FLUSH)
		mask = None;
	else if (mode == LISTEN_GRAB_INPUT)
	{
		if (xconfig->events_receive_mode == EVENT_PRESS)
			mask |= EVENT_PRESS_MASK;
		else
			mask |= EVENT_RELEASE_MASK;
		
		mask |= INPUT_HANDLE_MASK;
	}

	while (TRUE)
	{
		set_event_mask(parent_window, mask);
		
		int dummy;
		unsigned int dummyU;
		Window root_window, child_window;
		int is_same_screen = XQueryPointer(main_window->display, parent_window, &root_window, &child_window, &dummy, &dummy, &dummy, &dummy, &dummyU);
		if (!is_same_screen || child_window == None)
			break;

		parent_window = child_window;
	}

	p->last_parent_window = parent_window;
}

void xfocus_uninit(struct _xfocus *p)
{
	free(p);
}

struct _xfocus* xfocus_init(void)
{
	struct _xfocus *p = (struct _xfocus *) malloc(sizeof(struct _xfocus));
	bzero(p, sizeof(struct _xfocus));

	// Functions mapping
	p->get_focus_status	= xfocus_get_focus_status;
	p->update_events	= xfocus_update_events;
	p->uninit		= xfocus_uninit;

	return p;
}
