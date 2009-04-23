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

#ifndef _CONFIG_MAIN_H_
#define _CONFIG_MAIN_H_

#define CONFIG_NAME			"xneurrc"
#define CONFIG_BCK_NAME			"xneurrc~"
#define LOG_NAME			"xneurlog"

#define DICT_NAME			"dict"
#define PROTO_NAME			"proto"
#define BIG_PROTO_NAME			"proto3"
#define REGEXP_NAME			"regexp"

#define SOUNDDIR			"sounds"
#define PIXMAPDIR			"pixmaps"

#define MAX_FLAGS			4
#define MAX_SOUNDS			25
#define MAX_OSDS			25
#define MAX_HOTKEYS			14

enum _flag_action
{
	FLAG_LAYOUT_0 = 0,
	FLAG_LAYOUT_1,
	FLAG_LAYOUT_2,
	FLAG_LAYOUT_3,
};

enum _sound_action
{
	SOUND_XNEUR_START = 0,
	SOUND_XNEUR_RELOAD,
	SOUND_XNEUR_STOP,
	SOUND_PRESS_KEY_LAYOUT_0,
	SOUND_PRESS_KEY_LAYOUT_1,
	SOUND_PRESS_KEY_LAYOUT_2,
	SOUND_PRESS_KEY_LAYOUT_3,
	SOUND_ENABLE_LAYOUT_0,
	SOUND_ENABLE_LAYOUT_1,
	SOUND_ENABLE_LAYOUT_2,
	SOUND_ENABLE_LAYOUT_3,
	SOUND_AUTOMATIC_CHANGE_WORD,
	SOUND_MANUAL_CHANGE_WORD,
	SOUND_CHANGE_STRING,
	SOUND_CHANGE_SELECTED,
	SOUND_TRANSLIT_SELECTED,
	SOUND_CHANGECASE_SELECTED,
	SOUND_CHANGE_CLIPBOARD,
	SOUND_TRANSLIT_CLIPBOARD,
	SOUND_CHANGECASE_CLIPBOARD,
	SOUND_REPLACE_ABBREVIATION,
	SOUND_CORR_INCIDENTAL_CAPS,
	SOUND_CORR_TWO_CAPITAL_LETTER,
	SOUND_EXEC_USER_ACTION,
	SOUND_NONE,
};

enum _osd_action
{
	OSD_XNEUR_START = 0,
	OSD_XNEUR_RELOAD,
	OSD_XNEUR_STOP,
	OSD_PRESS_KEY_LAYOUT_0,
	OSD_PRESS_KEY_LAYOUT_1,
	OSD_PRESS_KEY_LAYOUT_2,
	OSD_PRESS_KEY_LAYOUT_3,
	OSD_ENABLE_LAYOUT_0,
	OSD_ENABLE_LAYOUT_1,
	OSD_ENABLE_LAYOUT_2,
	OSD_ENABLE_LAYOUT_3,
	OSD_AUTOMATIC_CHANGE_WORD,
	OSD_MANUAL_CHANGE_WORD,
	OSD_CHANGE_STRING,
	OSD_CHANGE_SELECTED,
	OSD_TRANSLIT_SELECTED,
	OSD_CHANGECASE_SELECTED,
	OSD_CHANGE_CLIPBOARD,
	OSD_TRANSLIT_CLIPBOARD,
	OSD_CHANGECASE_CLIPBOARD,
	OSD_REPLACE_ABBREVIATION,
	OSD_CORR_INCIDENTAL_CAPS,
	OSD_CORR_TWO_CAPITAL_LETTER,
	OSD_EXEC_USER_ACTION,
	OSD_NONE,
};

enum _hotkey_action
{
	ACTION_CHANGE_WORD = 0,
	ACTION_CHANGE_STRING,
	ACTION_CHANGE_MODE,
	ACTION_CHANGE_SELECTED,
	ACTION_TRANSLIT_SELECTED,
	ACTION_CHANGECASE_SELECTED,
	ACTION_CHANGE_CLIPBOARD,
	ACTION_TRANSLIT_CLIPBOARD,
	ACTION_CHANGECASE_CLIPBOARD,
	ACTION_ENABLE_LAYOUT_0,
	ACTION_ENABLE_LAYOUT_1,
	ACTION_ENABLE_LAYOUT_2,
	ACTION_ENABLE_LAYOUT_3,
	ACTION_REPLACE_ABBREVIATION,
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
	CHANGE_SYLL_TO_LAYOUT_0,
	CHANGE_SYLL_TO_LAYOUT_1,
	CHANGE_SYLL_TO_LAYOUT_2,
	CHANGE_SYLL_TO_LAYOUT_3,
	CHANGE_SELECTION,
	CHANGE_CLIPBOARD,
	CHANGE_STRING_TO_LAYOUT_0,
	CHANGE_STRING_TO_LAYOUT_1,
	CHANGE_STRING_TO_LAYOUT_2,
	CHANGE_STRING_TO_LAYOUT_3,
	CHANGE_ABBREVIATION,
};

struct _xneur_language
{
	char *dir;
	char *name;
	int  group;
	int  fixed;

	struct _list_char *temp_dicts;
	struct _list_char *dicts;
	struct _list_char *protos;
	struct _list_char *big_protos;
	struct _list_char *regexp;
};

struct _xneur_hotkey
{
	int modifiers; // Shift (0x1), Control (0x2), Alt (0x4), Super (0x8)
	char *key;
};

struct _xneur_file
{
	char *file;
};

struct _xneur_data
{
	int process_id;
	int manual_mode;
};

struct _xneur_action
{
	struct _xneur_hotkey hotkey;
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

	struct _xneur_data *xneur_data;
	struct _xneur_language *languages;		// Array of languages used in program
	struct _xneur_hotkey *hotkeys;			// Array of hotkeys used in program
	struct _xneur_file *sounds;			// Array of sounds for actions
	struct _xneur_file *osds;			// Array of OSDs for actions
	struct _xneur_action *actions;			// Array of actions

	int   actions_count;				// Count of actions

	int   manual_mode;				// Enable manual processing mode
	int   log_level;				// Maximum level of log messages to print
	int   send_delay;				// Delay before send event (in milliseconds)
	int   total_languages;				// Total languages to work with

	int   default_group;				// Initial keyboard layout for all new applications

	int   play_sounds;				// Play sound samples or not
	int   grab_mouse;				// Grab mouse or not
	int   educate;					// Education xneur
	int   remember_layout;				// Remember layout for each of window
	int   save_selection;				// Save selection after convert
	int   save_keyboard_log;			// Save keyboard log
	int   correct_incidental_caps;			// Change iNCIDENTAL CapsLock
	int   correct_two_capital_letter;		// Change two CApital letter
	int   flush_buffer_when_press_enter;		// Flush internal buffer when pressed Enter
	int   dont_process_when_press_enter;		// Don't correct word when pressed Enter
	int   check_lang_on_process;			// Check lang on input process
	int   disable_capslock;				// Disable CapsLock use

	int   show_osd;					// Show OSD
	char  *osd_font;

	int   abbr_ignore_layout;			// Ignore keyboard layout for abbreviations

	char* (*get_home_dict_path) (const char *dir_name, const char *file_name);
	char* (*get_global_dict_path) (const char *dir_name, const char *file_name);
	const char* (*get_bool_name) (int option);

	int   (*load) (struct _xneur_config *p);
	void  (*clear) (struct _xneur_config *p);
	int   (*save) (struct _xneur_config *p);
	int   (*replace) (struct _xneur_config *p);
	void  (*reload) (struct _xneur_config *p);
	int   (*kill) (struct _xneur_config *p);
	void  (*save_dicts) (struct _xneur_config *p, int lang);
	void  (*set_pid) (struct _xneur_config *p, int pid);
	int   (*get_pid) (struct _xneur_config *p);
	void  (*set_manual_mode) (struct _xneur_config *p, int manual_mode);
	int   (*is_manual_mode) (struct _xneur_config *p);
	char* (*get_lang_dir) (struct _xneur_config *p, int lang);
	char* (*get_lang_name) (struct _xneur_config *p, int lang);
	int   (*get_lang_group) (struct _xneur_config *p, int lang);
	int   (*find_group_lang) (struct _xneur_config *p, int group);
	void  (*add_language) (struct _xneur_config *p, const char *name, const char *dir, int group, int fixed);
	const char* (*get_log_level_name) (struct _xneur_config *p);
	void  (*uninit) (struct _xneur_config *p);
};

struct _xneur_config* xneur_config_init(void);

#endif /* _CONFIG_MAIN_H_ */
