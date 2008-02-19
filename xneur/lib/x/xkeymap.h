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
 *  (c) XNeur Team 2007
 *
 */

#ifndef _XKEYMAP_H_
#define _XKEYMAP_H_

#include <X11/Xutil.h>

int   get_keycode_mod(int keyboard_group);
int   get_languages_mask(void);
void  get_keysyms_by_string(char *keyname, KeySym *Lower, KeySym *Upper);
char* keycode_to_symbol(KeyCode kc, int group, int state);

struct _xkeymap
{
	KeySym *keymap;

	int latin_group;
	int latin_group_mask;	// Define latin group mask
	int min_keycode;
	int max_keycode;
	int keysyms_per_keycode;
	int total_key_arrays;

	char  (*get_ascii)(struct _xkeymap *p, const char *sym);
	char  (*get_cur_ascii_char) (struct _xkeymap *p, XEvent e);
	void  (*convert_text_to_ascii)(struct _xkeymap *p, char *text);
	void  (*char_to_keycode)(struct _xkeymap *p, char ch, KeyCode *kc, int *modifier);
	void  (*store_keymaps)(struct _xkeymap *p);
	void  (*print_keymaps)(struct _xkeymap *p);
	char* (*lower_by_keymaps)(struct _xkeymap *p, int gr, char *text);
	void  (*uninit) (struct _xkeymap *p);
};

struct _xkeymap *xkeymap_init(void);

#endif /* _XKEYMAP_H_ */
