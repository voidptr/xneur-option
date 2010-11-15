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

#ifndef _CONFIG_MAIN_H_
#define _CONFIG_MAIN_H_

#define CONFIG_NAME			"xneurrc"
#define CONFIG_BCK_NAME			"xneurrc~"
#define LOG_NAME			"xneurlog.html"

#define SOUNDDIR			"sounds"

#define MAX_NOTIFIES		30
#define MAX_HOTKEYS			23

#include "xneur.h"

enum _flag_action
{
	FLAG_LAYOUT_0 = 0,
	FLAG_LAYOUT_1,
	FLAG_LAYOUT_2,
	FLAG_LAYOUT_3,
};

enum _notify_action
{
	NOTIFY_XNEUR_START = 0,
	NOTIFY_XNEUR_RELOAD,
	NOTIFY_XNEUR_STOP,
	NOTIFY_PRESS_KEY_LAYOUT_0,
	NOTIFY_PRESS_KEY_LAYOUT_1,
	NOTIFY_PRESS_KEY_LAYOUT_2,
	NOTIFY_PRESS_KEY_LAYOUT_3,
	NOTIFY_ENABLE_LAYOUT_0,
	NOTIFY_ENABLE_LAYOUT_1,
	NOTIFY_ENABLE_LAYOUT_2,
	NOTIFY_ENABLE_LAYOUT_3,
	NOTIFY_AUTOMATIC_CHANGE_WORD,
	NOTIFY_MANUAL_CHANGE_WORD,
	NOTIFY_MANUAL_TRANSLIT_WORD,
	NOTIFY_MANUAL_CHANGECASE_WORD,
	NOTIFY_MANUAL_PREVIEW_CHANGE_WORD,
	NOTIFY_CHANGE_STRING,
	NOTIFY_CHANGE_SELECTED,
	NOTIFY_CHANGE_SELECTED_AND_ROTATE_LAYOUT,
	NOTIFY_TRANSLIT_SELECTED,
	NOTIFY_CHANGECASE_SELECTED,
	NOTIFY_PREVIEW_CHANGE_SELECTED,
	NOTIFY_CHANGE_CLIPBOARD,
	NOTIFY_TRANSLIT_CLIPBOARD,
	NOTIFY_CHANGECASE_CLIPBOARD,
	NOTIFY_PREVIEW_CHANGE_CLIPBOARD,
	NOTIFY_REPLACE_ABBREVIATION,
	NOTIFY_CORR_INCIDENTAL_CAPS,
	NOTIFY_CORR_TWO_CAPITAL_LETTER,
	NOTIFY_EXEC_USER_ACTION,
	NOTIFY_NONE,
};

enum _hotkey_action
{
	ACTION_CHANGE_WORD = 0,
	ACTION_TRANSLIT_WORD,
	ACTION_CHANGECASE_WORD,
	ACTION_PREVIEW_CHANGE_WORD,
	ACTION_CHANGE_STRING,
	ACTION_CHANGE_MODE,
	ACTION_CHANGE_SELECTED,
	ACTION_CHANGE_SELECTED_AND_ROTATE_LAYOUT,
	ACTION_TRANSLIT_SELECTED,
	ACTION_CHANGECASE_SELECTED,
	ACTION_PREVIEW_CHANGE_SELECTED,
	ACTION_CHANGE_CLIPBOARD,
	ACTION_TRANSLIT_CLIPBOARD,
	ACTION_CHANGECASE_CLIPBOARD,
	ACTION_PREVIEW_CHANGE_CLIPBOARD,
	ACTION_ENABLE_LAYOUT_0,
	ACTION_ENABLE_LAYOUT_1,
	ACTION_ENABLE_LAYOUT_2,
	ACTION_ENABLE_LAYOUT_3,
	ACTION_ROTATE_LAYOUT,
	ACTION_ROTATE_LAYOUT_BACK,
	ACTION_REPLACE_ABBREVIATION,
	ACTION_AUTOCOMPLEMENTATION,
	ACTION_NONE,
};

enum _change_action
{
	CHANGE_INCIDENTAL_CAPS = 0,
	CHANGE_TWO_CAPITAL_LETTER,
	CHANGE_WORD_TO_LAYOUT_0,
	CHANGE_WORD_TO_LAYOUT_1,
	CHANGE_WORD_TO_LAYOUT_2,
	CHANGE_WORD_TO_LAYOUT_3,
	CHANGE_WORD_TRANSLIT,
	CHANGE_WORD_CHANGECASE,
	CHANGE_WORD_PREVIEW_CHANGE,
	CHANGE_SYLL_TO_LAYOUT_0,
	CHANGE_SYLL_TO_LAYOUT_1,
	CHANGE_SYLL_TO_LAYOUT_2,
	CHANGE_SYLL_TO_LAYOUT_3,
	CHANGE_SELECTION,
	CHANGE_STRING_TO_LAYOUT_0,
	CHANGE_STRING_TO_LAYOUT_1,
	CHANGE_STRING_TO_LAYOUT_2,
	CHANGE_STRING_TO_LAYOUT_3,
	CHANGE_ABBREVIATION,
};

struct _xneur_hotkey
{
	int modifiers; // Shift (0x1), Control (0x2), Alt (0x4), Super (0x8)
	char *key;
};

struct _xneur_notify
{
	char *file;
	int enabled;
};

struct _xneur_data
{
	int process_id;
	int manual_mode;
};

struct _xneur_action
{
	struct _xneur_hotkey hotkey;
	char *name;
	char *command;
};

struct _xneur_config
{
	char *version;

	void (*get_library_version) (int *major_version, int *minor_version); // This function MUST be first

	struct _list_char *excluded_apps;
	struct _list_char *auto_apps;
	struct _list_char *manual_apps;
	struct _list_char *layout_remember_apps;
	struct _list_char *window_layouts;
	struct _list_char *abbreviations;
	struct _list_char *plugins;
	
	struct _xneur_data *xneur_data;
	struct _xneur_handle *handle;		// Array of languages used in program
	struct _xneur_hotkey *hotkeys;			// Array of hotkeys used in program
	struct _xneur_notify *sounds;			// Array of sounds for actions
	struct _xneur_notify *osds;			// Array of OSDs for actions
	struct _xneur_notify *popups;			// Array of popups for actions
	struct _xneur_action *actions;			// Array of actions

	int   actions_count;				// Count of actions

	int   manual_mode;				// Enable manual processing mode
	int   log_level;				// Maximum level of log messages to print
	int   send_delay;				// Delay before send event (in milliseconds)
	
	int   default_group;				// Initial keyboard layout for all new applications

	int   play_sounds;				// Play sound samples or not
	int   volume_percent;			// Sound Volume Percent

	int   educate;					// Education xneur
	int   remember_layout;				// Remember layout for each of window
	int   save_selection;				// Save selection after convert

	int   correct_incidental_caps;			// Change iNCIDENTAL CapsLock
	int   correct_two_capital_letter;		// Change two CApital letter
	int   correct_space_with_punctuation;	// Correct spaces before punctuation
	int   flush_buffer_when_press_enter;		// Flush internal buffer when pressed Enter
	int   dont_process_when_press_enter;		// Don't correct word when pressed Enter
	int   check_lang_on_process;			// Check lang on input process
	int   disable_capslock;				// Disable CapsLock use
	
	int   autocomplementation;			// Save pattern and mining
	int   add_space_after_autocomplementation;
	struct _list_char *autocomplementation_excluded_apps;
	
	int   show_osd;					// Show OSD
	char  *osd_font;

	int   show_popup;				// Show popups

	int   abbr_ignore_layout;			// Ignore keyboard layout for abbreviations

	int   save_keyboard_log;			// Save keyboard log
	int   size_keyboard_log;			// Size keyboard log
	char* mail_keyboard_log;
	char* host_keyboard_log;
	int   port_keyboard_log;

	// Troubleshooting
	int troubleshoot_backspace;
	int troubleshoot_left_arrow;
	int troubleshoot_right_arrow;
	int troubleshoot_up_arrow;
	int troubleshoot_down_arrow;
	int troubleshoot_delete;
	int troubleshoot_switch;

	int dont_send_key_release;
	
	char* (*get_home_dict_path) (const char *dir_name, const char *file_name);
	char* (*get_global_dict_path) (const char *dir_name, const char *file_name);
	const char* (*get_bool_name) (int option);

	int   (*load) (struct _xneur_config *p);
	void  (*clear) (struct _xneur_config *p);
	int   (*save) (struct _xneur_config *p);
	int   (*replace) (struct _xneur_config *p);
	void  (*reload) (struct _xneur_config *p);
	int   (*kill) (struct _xneur_config *p);
	void  (*save_dict) (struct _xneur_config *p, int lang);
	void  (*save_pattern) (struct _xneur_config *p, int lang);
	void  (*set_pid) (struct _xneur_config *p, int pid);
	int   (*get_pid) (struct _xneur_config *p);
	void  (*set_manual_mode) (struct _xneur_config *p, int manual_mode);
	int   (*is_manual_mode) (struct _xneur_config *p);
	char* (*get_lang_dir) (struct _xneur_config *p, int lang);
	const char* (*get_log_level_name) (struct _xneur_config *p);
	void  (*uninit) (struct _xneur_config *p);
};

struct _xneur_config* xneur_config_init(void);

#endif /* _CONFIG_MAIN_H_ */
