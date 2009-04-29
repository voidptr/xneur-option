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

#ifndef _XPROGRAM_H_
#define _XPROGRAM_H_

struct _xprogram
{
	struct _switchlang *switchlang;
	struct _xselection *selection;
	struct _event *event;
	struct _focus *focus;
	struct _xstring *string;

	int  last_action;
	int  changed_manual;
	int  app_forced_mode;
	int  app_focus_mode;

	int  selected_mode;

	int  modifier_mask;

	int  last_layout;
	int  last_window;

	int prev_mod_key;

	void (*layout_update) (struct _xprogram *p);
	void (*update) (struct _xprogram *p);
	void (*on_key_action) (struct _xprogram *p, int type);
	void (*process_input) (struct _xprogram *p);
	int  (*perform_manual_action) (struct _xprogram *p, enum _hotkey_action action);
	void (*perform_auto_action) (struct _xprogram *p, int action);
	void (*perform_user_action) (struct _xprogram *p, int action);
	int  (*check_lang_last_word) (struct _xprogram *p);
	int  (*check_lang_last_syllable) (struct _xprogram *p);
	void (*check_caps_last_word) (struct _xprogram *p);
	void (*check_tcl_last_word) (struct _xprogram *p);
	void (*change_word) (struct _xprogram *p, enum _change_action action);
	void (*add_word_to_dict) (struct _xprogram *p, int new_lang);
	void (*process_selection_notify) (struct _xprogram *p);
	void (*change_lang) (struct _xprogram *p, int new_lang);
	void (*change_incidental_caps) (struct _xprogram *p);
	void (*change_two_capital_letter) (struct _xprogram *p);
	void (*send_string_silent) (struct _xprogram *p, int send_backspaces);
	void (*uninit) (struct _xprogram *p);
};

struct _xprogram* xprogram_init(void);

#endif /* _XPROGRAM_H_ */
