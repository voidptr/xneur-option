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

#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "xnconfig.h"

#include "types.h"
#include "utils.h"
#include "log.h"
#include "list_char.h"

#include "keymap.h"

#include "bind_table.h"

static struct _bind_table *ubtable;

static struct _bind_table btable[MAX_HOTKEYS] =	{
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
							{0, 0, 0},
	                        {0, 0, 0},
							{0, 0, 0}
						};
static const char *normal_action_names[] =	{
							"Change Last Word", "Translit Last Word", "Changecase Last Word", "Preview Change Last Word",
							"Change Last String", "Change Mode",
							"Change Selected", "Translit Selected", "Changecase Selected", "Preview Change Selected",
							"Change Clipboard", "Translit Clipboard", "Changecase Clipboard", "Preview Change Clipboard",
							"Enable Layout 1", "Enable Layout 2", "Enable Layout 3", "Enable Layout 4",
							"Rotate Layouts", "Rotate Layouts Back", "Replace Abbreviation", "Autocomplementation Confirmation",
							"Block Keyboard And Mouse Events"
						};

static const char *modifier_names[] =	{"Shift", "Control", "Alt", "Super"};

extern struct _xneur_config *xconfig;

static char* hotkeys_concat_bind(int action)
{
	char *text = (char *) malloc((24 + 1 + strlen(xconfig->hotkeys[action].key)) * sizeof(char));
	text[0] = '\0';

	int total_modifiers	= sizeof(modifier_names) / sizeof(modifier_names[0]);
	for (int i = 0; i < total_modifiers; i++)
	{
		if ((xconfig->hotkeys[action].modifiers & (0x1 << i)) == 0)
			continue;

		strcat(text, modifier_names[i]);
		strcat(text, "+");
	}

	strcat(text, xconfig->hotkeys[action].key);
	
	return text;
}

static char* user_hotkeys_concat_bind(int action)
{
	char *text = (char *) malloc((24 + 1 + strlen(xconfig->actions[action].hotkey.key)) * sizeof(char));
	text[0] = '\0';

	int total_modifiers	= sizeof(modifier_names) / sizeof(modifier_names[0]);
	for (int i = 0; i < total_modifiers; i++)
	{
		if ((xconfig->actions[action].hotkey.modifiers & (0x1 << i)) == 0)
			continue;

		strcat(text, modifier_names[i]);
		strcat(text, "+");
	}

	strcat(text, xconfig->actions[action].hotkey.key);
	
	return text;
}

static void bind_action(enum _hotkey_action action)
{
	btable[action].modifier_mask	= 0;
	btable[action].key_sym		= 0;
	btable[action].key_sym_shift	= 0;

	if (xconfig->hotkeys[action].key == NULL)
	{
		log_message(DEBUG, _("   No key set for action \"%s\""), normal_action_names[action]);
		return;
	}

	int modifiers = xconfig->hotkeys[action].modifiers;
	if (modifiers & 0x01)
		btable[action].modifier_mask = btable[action].modifier_mask + 1;	// Shift
	if (modifiers & 0x02)
		btable[action].modifier_mask = btable[action].modifier_mask + 4;	// Control
	if (modifiers & 0x04)
		btable[action].modifier_mask = btable[action].modifier_mask + 8;	// Alt
	if (modifiers & 0x08)
		btable[action].modifier_mask = btable[action].modifier_mask + 64;	// Win

	
	KeySym key_sym, key_sym_shift;
	get_keysyms_by_string(xconfig->hotkeys[action].key, &key_sym, &key_sym_shift);
	if (key_sym == NoSymbol)
		key_sym = None;
	if (key_sym_shift == NoSymbol)
		key_sym_shift = key_sym;

	btable[action].key_sym = key_sym;
	btable[action].key_sym_shift = key_sym_shift;

	char *key = hotkeys_concat_bind (action);
	log_message(DEBUG, _("   Action \"%s\" with key \"%s\""), normal_action_names[action], key);
	free(key);
}

static void bind_user_action(int action)
{
	ubtable[action].modifier_mask	= 0;
	ubtable[action].key_sym		= 0;
	ubtable[action].key_sym_shift	= 0;

	if (xconfig->actions[action].hotkey.key == NULL)
	{
		log_message(DEBUG, _("   No key set for action \"%s\""), xconfig->actions[action].name);
		return;
	}

	int action_modifiers = xconfig->actions[action].hotkey.modifiers;

	if (action_modifiers & 0x01)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 1;	// Shift
	if (action_modifiers & 0x02)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 4;	// Control
	if (action_modifiers & 0x04)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 8;	// Alt
	if (action_modifiers & 0x08)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 64;	// Win

	KeySym key_sym, key_sym_shift;
	get_keysyms_by_string(xconfig->actions[action].hotkey.key, &key_sym, &key_sym_shift);
	if (key_sym == NoSymbol)
		key_sym = None;
	if (key_sym_shift == NoSymbol)
		key_sym_shift = key_sym;

	ubtable[action].key_sym = key_sym;
	ubtable[action].key_sym_shift = key_sym_shift;

	char *key = user_hotkeys_concat_bind (action);
	log_message(DEBUG, _("   Action \"%s\" with key \"%s\""), xconfig->actions[action].name, key);
	free(key);
}

enum _hotkey_action get_manual_action(KeySym key_sym, int mask)
{
	// Reset Caps and Num mask
	mask &= ~LockMask;
	mask &= ~Mod2Mask;
	mask &= ~Mod3Mask;

	//log_message (ERROR, "%s %d", XKeysymToString(key_sym), mask);
	for (enum _hotkey_action action = 0; action < MAX_HOTKEYS; action++)
	{
		//log_message (ERROR, "%d---%s %s %d", action, XKeysymToString(btable[action].key_sym), XKeysymToString(btable[action].key_sym_shift), btable[action].modifier_mask);
		if (btable[action].key_sym != key_sym && btable[action].key_sym_shift != key_sym)
			continue;

		if (btable[action].modifier_mask == mask)
			return action;
	}
	return ACTION_NONE;
}

void bind_manual_actions(void)
{
	log_message(DEBUG, _("Binded hotkeys actions:"));
	for (enum _hotkey_action action = 0; action < MAX_HOTKEYS; action++)
		bind_action(action);
}

int get_user_action(KeySym key_sym, int mask)
{
	// Reset Caps and Num mask
	mask &= ~LockMask;
	mask &= ~Mod2Mask;
	mask &= ~Mod3Mask;

	//log_message (ERROR, "%s %d", XKeysymToString(key_sym), mask);
	for (int action = 0; action < xconfig->actions_count; action++)
	{
		//log_message (ERROR, "---%s %s %d", XKeysymToString(ubtable[action].key_sym), XKeysymToString(ubtable[action].key_sym_shift), ubtable[action].modifier_mask);
		if (ubtable[action].key_sym != key_sym && ubtable[action].key_sym_shift != key_sym)
			continue;

		if (ubtable[action].modifier_mask == mask)
			return action;
	}
	return -1;
}

void bind_user_actions(void)
{
	log_message(DEBUG, _("Binded hotkeys user actions:"));

	ubtable = (struct _bind_table *) malloc(xconfig->actions_count * sizeof(struct _bind_table));
	for (int action = 0; action < xconfig->actions_count; action++)
		bind_user_action(action);
}

void unbind_user_actions(void)
{
	free(ubtable);
}
