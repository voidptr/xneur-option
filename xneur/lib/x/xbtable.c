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

#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "xnconfig.h"

#include "types.h"
#include "utils.h"
#include "log.h"
#include "list_char.h"

#include "xkeymap.h"

#include "xbtable.h"

static struct _xbtable *ubtable;

static struct _xbtable btable[MAX_HOTKEYS] =	{
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
							"Change Last Word", "Change Last String", "Change Mode", 
							"Change Selected", "Translit Selected", "Changecase Selected",
							"Enable Layout 1", "Enable Layout 2", "Enable Layout 3", "Enable Layout 4",
							"Replace Abbreviation"
						};
extern struct _xneur_config *xconfig;
	
static void bind_action(enum _hotkey_action action)
{
	btable[action].modifier_mask	= 0;
	btable[action].key_sym		= 0;
	btable[action].key_sym_shift	= 0;
	
	if (xconfig->hotkeys[action].key == NULL)
	{
		log_message(DEBUG, "   No key set for action \"%s\"", normal_action_names[action]);
		return;
	}

	if (xconfig->hotkeys[action].modifiers & 0x1)
		btable[action].modifier_mask = btable[action].modifier_mask + 1;	// Shift
	if (xconfig->hotkeys[action].modifiers & 0x2)
		btable[action].modifier_mask = btable[action].modifier_mask + 4;	// Control
	if (xconfig->hotkeys[action].modifiers & 0x4)
		btable[action].modifier_mask = btable[action].modifier_mask + 8;	// Alt
	if (xconfig->hotkeys[action].modifiers & 0x8)
		btable[action].modifier_mask = btable[action].modifier_mask + 64;	// Win
	
	KeySym key_sym, key_sym_shift;
	get_keysyms_by_string(xconfig->hotkeys[action].key, &key_sym, &key_sym_shift);
	if (key_sym == NoSymbol)
		key_sym = None;
	if (key_sym_shift == NoSymbol)
		key_sym_shift = None;

	btable[action].key_sym = key_sym;
	btable[action].key_sym_shift = key_sym_shift;
	
	if (btable[action].key_sym == 0)
		return;

	log_message(DEBUG, "   Action \"%s\" with mod_mask %d and key \"%s (%s)\"", normal_action_names[action], btable[action].modifier_mask, XKeysymToString(btable[action].key_sym), XKeysymToString(btable[action].key_sym_shift));
}

enum _hotkey_action get_manual_action(KeySym key_sym, int mask)
{
	// Reset Caps and Num mask
	mask &= ~LockMask;
	mask &= ~Mod2Mask;
	mask &= ~Mod3Mask;
	
	for (enum _hotkey_action action = 0; action < MAX_HOTKEYS; action++)
	{
		if (btable[action].key_sym != key_sym && btable[action].key_sym_shift != key_sym)
			continue;

		if (btable[action].modifier_mask == mask)
			return action;
	}
	return ACTION_NONE;
}

void bind_manual_actions(void)
{
	log_message(DEBUG, "Binded hotkeys actions (mod_mask = Shift(1) + Ctrl(4) + Alt(8) + Win(64)):");
	for (enum _hotkey_action action = 0; action < MAX_HOTKEYS; action++)
		bind_action(action);
}

static void bind_user_action(int action)
{
	ubtable[action].modifier_mask	= 0;
	ubtable[action].key_sym		= 0;
	ubtable[action].key_sym_shift	= 0;
	
	if (xconfig->actions->action_hotkey[action].key == NULL)
	{
		//log_message(DEBUG, "   No key set for action \"%s\"", normal_action_names[action]);
		return;
	}

	if (xconfig->actions->action_hotkey[action].modifiers & 0x1)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 1;	// Shift
	if (xconfig->actions->action_hotkey[action].modifiers & 0x2)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 4;	// Control
	if (xconfig->actions->action_hotkey[action].modifiers & 0x4)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 8;	// Alt
	if (xconfig->actions->action_hotkey[action].modifiers & 0x8)
		ubtable[action].modifier_mask = ubtable[action].modifier_mask + 64;	// Win
	
	KeySym key_sym, key_sym_shift;
	get_keysyms_by_string(xconfig->actions->action_hotkey[action].key, &key_sym, &key_sym_shift);
	if (key_sym == NoSymbol)
		key_sym = None;
	if (key_sym_shift == NoSymbol)
		key_sym_shift = None;

	ubtable[action].key_sym = key_sym;
	ubtable[action].key_sym_shift = key_sym_shift;
	
	if (ubtable[action].key_sym == 0)
		return;

	log_message(DEBUG, "   Action \"%s\" with mod_mask %d and key \"%s (%s)\"", xconfig->actions->action_command->data[action].string, ubtable[action].modifier_mask, XKeysymToString(ubtable[action].key_sym), XKeysymToString(ubtable[action].key_sym_shift));
}

int get_user_action(KeySym key_sym, int mask)
{
	// Reset Caps and Num mask
	mask &= ~LockMask;
	mask &= ~Mod2Mask;
	mask &= ~Mod3Mask;
	for (int action = 0; action < xconfig->actions->action_command->data_count; action++)
	{
		if (ubtable[action].key_sym != key_sym && ubtable[action].key_sym_shift != key_sym)
			continue;

		if (ubtable[action].modifier_mask == mask)
			return action;
	}
	return -1;
}

void bind_user_actions(void)
{
	int total_actions = xconfig->actions->action_command->data_count;
	
	log_message(DEBUG, "Binded hotkeys user actions (mod_mask = Shift(1) + Ctrl(4) + Alt(8) + Win(64)):");
	ubtable = (struct _xbtable *) malloc(total_actions * sizeof(struct _xbtable));
	for (int action = 0; action < total_actions; action++)
		bind_user_action(action);
}