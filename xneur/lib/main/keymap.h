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

#ifndef _KEYMAP_H_
#define _KEYMAP_H_

#include <X11/Xutil.h>

int   get_keycode_mod(int keyboard_group);
int   get_languages_mask(void);
void  get_keysyms_by_string(char *keyname, KeySym *Lower, KeySym *Upper);
char* keycode_to_symbol(KeyCode kc, int group, int state);

struct _keymap
{
	KeySym *keymap;

	int latin_group;
	int latin_group_mask;
	int min_keycode;
	int max_keycode;
	int keysyms_per_keycode;
	int keyboard_groups_count;

	char  (*get_ascii)(struct _keymap *p, const char *sym, KeyCode *kc, int *modifier);
	char  (*get_cur_ascii_char) (struct _keymap *p, XEvent e);
	void  (*convert_text_to_ascii)(struct _keymap *p, char *text, KeyCode *kc, int *kc_mod);
	void  (*char_to_keycode)(struct _keymap *p, char ch, KeyCode *kc, int *modifier);
	void  (*print_keymaps)(struct _keymap *p);
	char* (*lower_by_keymaps)(struct _keymap *p, int gr, char *text);
	void  (*uninit) (struct _keymap *p);
};

struct _keymap *keymap_init(void);

#endif /* _KEYMAP_H_ */
