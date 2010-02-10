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

int get_keyboard_groups_count(void)
{
	XkbDescRec *kbd_desc_ptr = XkbAllocKeyboard();
	if (kbd_desc_ptr == NULL)
	{
		log_message(ERROR, _("Failed to allocate keyboard descriptor"));
		return 0;
	}

	Display *display = XOpenDisplay(NULL);
	XkbGetNames(display, XkbGroupNamesMask, kbd_desc_ptr);
	XCloseDisplay(display);

	if (kbd_desc_ptr->names == NULL)
	{
		log_message(ERROR, _("Failed to get keyboard group names"));
		return 0;
	}

	int groups_count = 0;
	for (; groups_count < XkbNumKbdGroups; groups_count++)
	{
		if (kbd_desc_ptr->names->groups[groups_count] == None)
			break;
	}

	return groups_count;
}

void set_next_keyboard_group(void)
{
	int new_layout_group = get_active_keyboard_group() + 1;
	if (new_layout_group == get_keyboard_groups_count())
		new_layout_group = 0;
	switch_group (new_layout_group);
}

int print_keyboard_groups(void)
{
	XkbDescRec *kbd_desc_ptr = XkbAllocKeyboard();
	if (kbd_desc_ptr == NULL)
	{
		log_message(ERROR, _("Failed to allocate keyboard descriptor"));
		return FALSE;
	}

	Display *display = XOpenDisplay(NULL);
	XkbGetNames(display, XkbAllNamesMask, kbd_desc_ptr);

	if (kbd_desc_ptr->names == NULL)
	{
		XCloseDisplay(display);
		log_message(ERROR, _("Failed to get keyboard group names"));
		return FALSE;
	}

	int groups_count = get_keyboard_groups_count();
	if (groups_count == 0)
	{
		XCloseDisplay(display);
		log_message(ERROR, _("No keyboard layout found"));
		return FALSE;
	}

	log_message(LOG, _("Keyboard layouts present in system:"));

	Atom symbols_atom = kbd_desc_ptr->names->symbols;
	char *symbols	= XGetAtomName(display, symbols_atom);
	char *tmp_symbols = strdup(symbols);
	strsep(&tmp_symbols, "+");
	
	int valid_count = 0;
	for (int group = 0; group < groups_count; group++)
	{
		Atom group_atom = kbd_desc_ptr->names->groups[group];
		
		if (group_atom == None)
			continue;

		char *group_name	= XGetAtomName(display, group_atom);
		char *lang_name		= xconfig->get_lang_name(xconfig, xconfig->find_group_lang(xconfig, group));

		char *short_name = strsep(&tmp_symbols, "+");
		short_name[2] = NULLSYM;
		
		if (lang_name == NULL)
		{
			log_message(ERROR, _("   XKB Group '%s (%s)' not defined in configuration file (group %d)"), group_name, short_name, group);
			continue;
		}

		log_message(LOG, _("   XKB Group '%s (%s)' must be for '%s' language (group %d)"), group_name, short_name, lang_name, group);
		valid_count++;
	}

	free(symbols);
	XCloseDisplay(display);

	log_message(LOG, _("Total %d valid keyboard layouts detected"), valid_count);
	return TRUE;
}

int parse_keyboard_groups(void)
{
	XkbDescRec *kbd_desc_ptr = XkbAllocKeyboard();
	if (kbd_desc_ptr == NULL)
	{
		log_message(ERROR, _("Failed to allocate keyboard descriptor"));
		return FALSE;
	}

	Display *display = XOpenDisplay(NULL);
	XkbGetNames(display, XkbAllNamesMask, kbd_desc_ptr);

	if (kbd_desc_ptr->names == NULL)
	{
		XCloseDisplay(display);
		log_message(ERROR, _("Failed to get keyboard group names"));
		return FALSE;
	}

	int groups_count = get_keyboard_groups_count();
	if (groups_count == 0)
	{
		XCloseDisplay(display);
		log_message(ERROR, _("No keyboard layout found"));
		return FALSE;
	}

	log_message(LOG, _("Keyboard layouts present in system:"));

	Atom symbols_atom = kbd_desc_ptr->names->symbols;
	char *symbols	= XGetAtomName(display, symbols_atom);
	char *tmp_symbols = strdup(symbols);
	strsep(&tmp_symbols, "+");
	
	int valid_count = 0;
	for (int group = 0; group < groups_count; group++)
	{
		Atom group_atom = kbd_desc_ptr->names->groups[group];
		
		if (group_atom == None)
			continue;

		char *group_name	= XGetAtomName(display, group_atom);
		
		char *short_name = strsep(&tmp_symbols, "+");
		short_name[2] = NULLSYM;

		xconfig->languages = (struct _xneur_language *) realloc(xconfig->languages, (xconfig->total_languages + 1) * sizeof(struct _xneur_language));
		bzero(&(xconfig->languages[xconfig->total_languages]), sizeof(struct _xneur_language));

		xconfig->languages[xconfig->total_languages].name	= strdup(group_name);
		xconfig->languages[xconfig->total_languages].dir	= strdup(short_name);
		xconfig->languages[xconfig->total_languages].group	= group;
		xconfig->languages[xconfig->total_languages].fixed	= FALSE;
		xconfig->total_languages++;
		
		log_message(LOG, _("   XKB Group '%s, layout %s, group %d"), group_name, short_name, group);
		valid_count++;
	}

	free(symbols);
	XCloseDisplay(display);

	log_message(LOG, _("Total %d valid keyboard layouts detected"), valid_count);
	return TRUE;
}
