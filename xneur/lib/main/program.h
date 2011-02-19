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

#ifndef _PROGRAM_H_
#define _PROGRAM_H_

struct _program
{
	struct _switchlang *switchlang;
	struct _selection *selection;
	struct _event *event;
	struct _focus *focus;
	struct _buffer *buffer;
	struct _plugin *plugin;
	
	int  last_action;
	int  changed_manual;
	int  app_forced_mode;
	int  app_focus_mode;
	int  app_autocompletion_mode;
	
	int  action_mode;

	int  last_layout;
	int  last_window;
	
	KeySym prev_key;
	int prev_key_mod;
	struct _list_char *modifiers_stack;
	
	void (*layout_update) (struct _program *p);
	void (*update) (struct _program *p);
	void (*update_modifiers_stack) (struct _program *p);
	void (*on_key_action) (struct _program *p, int type);
	void (*process_input) (struct _program *p);
	int  (*perform_manual_action) (struct _program *p, enum _hotkey_action action);
	void (*perform_auto_action) (struct _program *p, int action);
	void (*perform_user_action) (struct _program *p, int action);
	int  (*check_lang_last_word) (struct _program *p);
	int  (*check_lang_last_syllable) (struct _program *p);
	void (*check_caps_last_word) (struct _program *p);
	void (*check_tcl_last_word) (struct _program *p);
	void (*check_space_before_punctuation) (struct _program *p);
	void (*check_space_with_bracket) (struct _program *p);
	void (*check_brackets_with_symbols) (struct _program *p);
	void (*check_capital_letter_after_dot) (struct _program *p);
	void (*check_pattern) (struct _program *p, int selection);
	void (*change_word) (struct _program *p, enum _change_action action);
	void (*add_word_to_dict) (struct _program *p, int new_lang);
	void (*add_word_to_pattern) (struct _program *p, int new_lang);
	void (*process_selection_notify) (struct _program *p);
	void (*change_lang) (struct _program *p, int new_lang);
	void (*change_incidental_caps) (struct _program *p);
	void (*change_two_capital_letter) (struct _program *p);
	void (*send_string_silent) (struct _program *p, int send_backspaces);
	void (*plugin_process) ();
	void (*uninit) (struct _program *p);
};

struct _program* program_init(void);

#endif /* _PROGRAM_H_ */
