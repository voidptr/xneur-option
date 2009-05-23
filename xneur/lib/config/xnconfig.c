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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "xnconfig_files.h"
#include "xnconfig_memory.h"

#include "types.h"
#include "list_char.h"
#include "log.h"

#include "xnconfig.h"

#define LIBRARY_VERSION_MAJOR		9
#define LIBRARY_VERSION_MINOR		5
#define OPTIONS_DELIMETER		" "

static const char *log_levels[] =	{"Error", "Warning", "Log", "Debug", "Trace"};
static const char *bool_names[] =	{"No", "Yes"};
static const char *fix_names[]  =	{"Fixed"};
static const char *modifier_names[] =	{"Shift", "Control", "Alt", "Super"};

static const char *option_names[] = 	{
						"ManualMode", "ExcludeApp", "AddBind", "LogLevel", "AddLanguage", "Autocomplementation",
						"DisableCapsLock", "CheckOnProcess", "SetAutoApp", "SetManualApp", "GrabMouse",
						"EducationMode", "Version", "LayoutRememberMode", "SaveSelectionMode",
						"DefaultXkbGroup", "AddSound", "PlaySounds", "SendDelay", "LayoutRememberModeForApp",
						"SaveLog", "ReplaceAbbreviation",
						"ReplaceAbbreviationIgnoreLayout", "CorrectIncidentalCaps", "CorrectTwoCapitalLetter",
						"FlushBufferWhenPressEnter", "DontProcessWhenPressEnter", "AddAction",
						"ShowOSD", "AddOSD", "FontOSD", "ShowPopup", "AddPopup", 
	                    "CorrectSpaceWithPunctuation", "AddSpaceAfterAutocomplementation"
					};
static const char *action_names[] =	{
						"ChangeWord", "ChangeString", "ChangeMode",
						"ChangeSelected", "TranslitSelected", "ChangecaseSelected",
						"ChangeClipboard", "TranslitClipboard", "ChangecaseClipboard",
						"EnableLayout1", "EnableLayout2", "EnableLayout3", "EnableLayout4",
						"RotateLayout","ReplaceAbbreviation", "AutocomplementationConfirmation"
					};
static const char *notify_names[] =	{
						"XneurStart", "XneurReload", "XneurStop",
						"PressKeyLayout1", "PressKeyLayout2", "PressKeyLayout3", "PressKeyLayout4",
						"EnableLayout1", "EnableLayout2", "EnableLayout3", "EnableLayout4",
						"AutomaticChangeWord", "ManualChangeWord", "ChangeString",
						"ChangeSelected", "TranslitSelected", "ChangecaseSelected",
						"ChangeClipboard", "TranslitClipboard", "ChangecaseClipboard",
						"ReplaceAbbreviation", "CorrectIncidentalCaps", "CorrectTwoCapitalLetter",
						"ExecuteUserAction"
					};

static int load_lang = -1;

pid_t getsid(pid_t pid);

static char* get_word(char **string)
{
	return strsep(string, OPTIONS_DELIMETER);
}

#define get_option_index(options, option) \
	get_option_index_size(options, option, sizeof(options) / sizeof(options[0]));

static int get_option_index_size(const char *options[], char *option, int options_count)
{
	for (int i = 0; i < options_count; i++)
	{
		if (strcmp(option, options[i]) == 0)
			return i;
	}
	return -1;
}

static void xneur_config_get_library_version(int *major_version, int *minor_version)
{
	*major_version = LIBRARY_VERSION_MAJOR;
	*minor_version = LIBRARY_VERSION_MINOR;
}

static const char* xneur_config_get_bool_name(int option)
{
	return bool_names[option];
}

static void parse_line(struct _xneur_config *p, char *line)
{
	if (line[0] == '#')
		return;

	char *option = get_word(&line);

	int index = get_option_index(option_names, option);
	if (index == -1)
	{
		log_message(WARNING, _("Unrecognized option \"%s\" detected"), option);
		return;
	}

	char *full_string = strdup(line);

	char *param = get_word(&line);
	if (param == NULL)
	{
		free(full_string);
		log_message(WARNING, _("Param mismatch for option %s"), option);
		return;
	}

	switch (index)
	{
		case 0: // Get Default Mode (Auto/Manual)
		{
			int manual = get_option_index(bool_names, param);
			if (manual == -1)
			{
				log_message(WARNING, _("Invalid value for manual mode specified"));
				break;
			}

			p->set_manual_mode(p, manual);
			p->manual_mode = manual;
			break;
		}
		case 1: // Get Applications Names
		{
			p->excluded_apps->add(p->excluded_apps, full_string);
			break;
		}
		case 2: // Get Keyboard Binds
		{
			int action = get_option_index(action_names, param);
			if (action == -1)
			{
				log_message(WARNING, _("Invalid value for action name specified"));
				break;
			}

			p->hotkeys[action].key = NULL;
			while (TRUE)
			{
				char *modifier = get_word(&line);
				if (modifier == NULL)
					break;

				if (modifier[0] == '\0')
					continue;

				int index = get_option_index(modifier_names, modifier);
				if (index == -1)
					p->hotkeys[action].key = strdup(modifier);
				else
					p->hotkeys[action].modifiers |= (1 << index);
			}

			break;
		}
		case 3: // Get Log Level
		{
			int index = get_option_index(log_levels, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for log level specified"));
				break;
			}

			p->log_level = index;
			log_set_level(p->log_level);
			break;
		}
		case 4: // Add Language
		{
			char *dir	= get_word(&line);
			char *group	= get_word(&line);
			char *fixed	= get_word(&line);

			if (dir == NULL || group == NULL)
			{
				log_message(ERROR, _("Argument number mismatch for AddLanguage option"));
				break;
			}

			int fix_index = FALSE;
			if (fixed != NULL)
			{
				int index = get_option_index(fix_names, fixed);
				if (index == 0) // Fixed
					fix_index = TRUE;
			}

			p->add_language(p, param, dir, atoi(group), fix_index);
			break;
		}
		case 5: // Pattern Mining and Recognition
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for pattern minig and recognition mode specified"));
				break;
			}

			p->autocomplementation = index;
			break;
		}
		case 6: // Disable CapsLock use
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for disable CapsLock using mode specified"));
				break;
			}

			p->disable_capslock = index;
			break;
		}
		case 7: // Check lang on input process
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for check language on input process mode specified"));
				break;
			}

			p->check_lang_on_process = index;
			break;
		}
		case 8: // Get Auto Processing Applications
		{
			p->auto_apps->add(p->auto_apps, full_string);
			break;
		}
		case 9: // Get Manual Processing Applications
		{
			p->manual_apps->add(p->manual_apps, full_string);
			break;
		}
		case 10: // Get Mouse Grab Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for mouse grab mode specified"));
				break;
			}

			p->grab_mouse = index;
			break;
		}
		case 11: // Get Education Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for education mode specified"));
				break;
			}

			p->educate = index;
			break;
		}
		case 12: // Get config version
		{
			p->version = strdup(param);
			break;
		}
		case 13: // Get Layout Remember Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for remember layout mode specified"));
				break;
			}

			p->remember_layout = index;
			break;
		}
		case 14: // Get Save Selection Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for save selection mode specified"));
				break;
			}

			p->save_selection = index;
			break;
		}
		case 15: // Get Initial Xkb Group for all new windows
		{
			p->default_group = atoi(get_word(&param));
			break;
		}
		case 16: // Sounds
		{
			int sound = get_option_index(notify_names, param);
			if (sound == -1)
			{
				log_message(WARNING, _("Invalid value for sound action name specified"));
				break;
			}

			if (line == NULL)
				break;

			char *file = strdup(get_word(&line));
			if (strlen(file) != 0)
			{
				p->sounds[sound].file = get_file_path_name(SOUNDDIR, file);
			}
			if (file != NULL)
				free(file);

			break;
		}
		case 17: // Play Sound
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for play sounds mode specified"));
				break;
			}

			p->play_sounds = index;
			break;
		}
		case 18: // Backevent Delay
		{
			p->send_delay = atoi(param);
			if (p->send_delay < 0 || p->send_delay > 50)
			{
				log_message(WARNING, _("Send delay must be between 0 and 50"));
				p->send_delay = 0;
			}
			break;
		}
		case 19: // layout remember for each application
		{
			p->layout_remember_apps->add(p->layout_remember_apps, full_string);
			break;
		}
		case 20: // Save Keyboard Log
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for save keyboard log mode specified"));
				break;
			}

			p->save_keyboard_log = index;
			break;
		}
		case 21: // Get Words for Replacing
		{
			p->abbreviations->add(p->abbreviations, full_string);
			break;
		}
		case 22: // Ignore keyboard layout for abbreviations
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for ignore keyboard layout for abbreviations mode specified"));
				break;
			}

			p->abbr_ignore_layout = index;
			break;
		}
		case 23: // Change iNCIDENTAL CapsLock Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for change iNCIDENTAL CapsLock mode specified"));
				break;
			}

			p->correct_incidental_caps = index;
			break;
		}
		case 24: // Change two CApital letter Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for change two CApital letter mode specified"));
				break;
			}

			p->correct_two_capital_letter = index;
			break;
		}
		case 25: // Flush internal buffer when pressed Enter Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for flush internal buffer when pressed Enter mode specified"));
				break;
			}

			p->flush_buffer_when_press_enter = index;
			break;
		}
		case 26: // Don't process word when pressed Enter Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for don't processing word when pressed Enter mode specified"));
				break;
			}

			p->dont_process_when_press_enter = index;
			break;
		}
		case 27: // User actions
		{
			p->actions = (struct _xneur_action *) realloc(p->actions, (p->actions_count + 1) * sizeof(struct _xneur_action));
			bzero(&p->actions[p->actions_count], sizeof(struct _xneur_action));

			while (TRUE)
			{
				if (param == NULL)
					break;

				if (param[0] == '\0')
					continue;

				int index = get_option_index(modifier_names, param);
				if (index == -1)
				{
					if (param != NULL)
						p->actions[p->actions_count].hotkey.key = strdup(param);
					if (line != NULL)
						p->actions[p->actions_count].command = strdup(line);
					break;
				}

				p->actions[p->actions_count].hotkey.modifiers |= (1 << index);

				param = get_word(&line);
			}

			p->actions_count++;
			break;
		}
		case 28: // Show OSD
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for show OSD mode specified"));
				break;
			}

			p->show_osd = index;
			break;
		}
		case 29: // OSDs
		{
			int osd = get_option_index(notify_names, param);
			if (osd == -1)
			{
				log_message(WARNING, _("Invalid value for OSD action name specified"));
				break;
			}

			if (line == NULL)
				break;

			if (strlen(line) != 0)
				p->osds[osd].file = strdup(line);

			break;
		}
		case 30: // Get Initial Xkb Group for all new windows
		{
			p->osd_font = strdup(param);
			break;
		}
		case 31: // Show popup
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, _("Invalid value for show popup message mode specified"));
				break;
			}

			p->show_popup = index;
			break;
		}
		case 32: // Popups
		{
			int popup = get_option_index(notify_names, param);
			if (popup == -1)
			{
				log_message(WARNING, _("Invalid value for OSD action name specified"));
				break;
			}

			if (line == NULL)
				break;

			if (strlen(line) != 0)
				p->popups[popup].file = strdup(line);

			break;
		}
		case 33:
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
				break;

			p->correct_space_with_punctuation = index;
		}
		case 34:
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
				break;

			p->add_space_after_autocomplementation = index;
		}
	}
	free(full_string);
}

static int parse_config_file(struct _xneur_config *p, const char *dir_name, const char *file_name)
{
	struct _list_char *list = load_list(dir_name, file_name, FALSE);
	if (list == NULL)
	{
		log_message(ERROR, _("Can't find config file %s"), file_name);
		return FALSE;
	}

	for (int i = 0; i < list->data_count; i++)
		parse_line(p, list->data[i].string);

	list->uninit(list);
	return TRUE;
}

static int check_memory_attached(struct _xneur_config *p)
{
	if (p->xneur_data != NULL)
		return TRUE;

	p->xneur_data = (struct _xneur_data *) attach_memory_segment(sizeof(struct _xneur_data));
	if (p->xneur_data == NULL)
		return FALSE;

	return TRUE;
}

static void free_structures(struct _xneur_config *p)
{
	p->window_layouts->uninit(p->window_layouts);
	p->manual_apps->uninit(p->manual_apps);
	p->auto_apps->uninit(p->auto_apps);
	p->layout_remember_apps->uninit(p->layout_remember_apps);
	p->excluded_apps->uninit(p->excluded_apps);

	for (int hotkey = 0; hotkey < MAX_HOTKEYS; hotkey++)
	{
		if (p->hotkeys[hotkey].key != NULL)
			free(p->hotkeys[hotkey].key);
	}

	for (int notify = 0; notify < MAX_NOTIFIES; notify++)
	{
		if (p->sounds[notify].file != NULL)
			free(p->sounds[notify].file);

		if (p->osds[notify].file != NULL)
			free(p->osds[notify].file);

		if (p->popups[notify].file != NULL)
			free(p->popups[notify].file);
	}

	for (int lang = 0; lang < p->total_languages; lang++)
	{
		if (p->languages[lang].temp_dict != NULL)
			p->languages[lang].temp_dict->uninit(p->languages[lang].temp_dict);

		if (p->languages[lang].dict != NULL)
			p->languages[lang].dict->uninit(p->languages[lang].dict);

		if (p->languages[lang].proto != NULL)
			p->languages[lang].proto->uninit(p->languages[lang].proto);

		if (p->languages[lang].big_proto != NULL)
			p->languages[lang].big_proto->uninit(p->languages[lang].big_proto);

		if (p->languages[lang].regexp != NULL)
			p->languages[lang].regexp->uninit(p->languages[lang].regexp);

		if (p->languages[lang].pattern != NULL)
			p->languages[lang].pattern->uninit(p->languages[lang].pattern);
		
		free(p->languages[lang].name);
		free(p->languages[lang].dir);
	}

	for (int action = 0; action < p->actions_count; action++)
	{
		if (p->actions[action].hotkey.key != NULL)
			free(p->actions[action].hotkey.key);
		if (p->actions[action].command != NULL)
			free(p->actions[action].command);
	}

	bzero(p->hotkeys, MAX_HOTKEYS * sizeof(struct _xneur_hotkey));
	bzero(p->sounds, MAX_NOTIFIES * sizeof(struct _xneur_file));
	bzero(p->osds, MAX_NOTIFIES * sizeof(struct _xneur_file));
	bzero(p->popups, MAX_NOTIFIES * sizeof(struct _xneur_file));

	p->total_languages = 0;
	p->actions_count = 0;

	if (p->version != NULL)
		free(p->version);

	if (p->osd_font != NULL)
		free(p->osd_font);

	if (p->languages != NULL)
		free(p->languages);

	if (p->actions != NULL)
		free(p->actions);
}

static void xneur_config_reload(struct _xneur_config *p)
{
	int process_id = p->xneur_data->process_id;
	if (process_id <= 0)
		return;

	kill(process_id, SIGHUP);
}

static void xneur_config_set_pid(struct _xneur_config *p, int process_id)
{
	p->xneur_data->process_id = process_id;
}

static int xneur_config_kill(struct _xneur_config *p)
{
	int process_id = p->xneur_data->process_id;
	if (process_id <= 0)
		return FALSE;

	if (kill(process_id, SIGTERM) == -1)
		return FALSE;

	xneur_config_set_pid(p, 0);

	return TRUE;
}

static int xneur_config_get_pid(struct _xneur_config *p)
{
	int process_id = p->xneur_data->process_id;
	if (process_id <= 0)
		return -1;

	if (getsid(process_id) == -1)
		return -1;

	return p->xneur_data->process_id;
}

static void xneur_config_set_manual_mode(struct _xneur_config *p, int manual_mode)
{
	p->xneur_data->manual_mode = manual_mode;
}

static int xneur_config_is_manual_mode(struct _xneur_config *p)
{
	return (p->xneur_data->manual_mode == TRUE);
}

static int xneur_config_load(struct _xneur_config *p)
{
	if (!parse_config_file(p, NULL, CONFIG_NAME))
		return FALSE;

	if (p->total_languages == 0)
	{
		log_message(ERROR, _("No languages specified in config file"));
		return FALSE;
	}

	for (int lang = 0; lang < p->total_languages; lang++)
	{
		char *lang_dir	= p->get_lang_dir(p, lang);
		char *lang_name	= p->get_lang_name(p, lang);

		p->languages[lang].dict = load_list(lang_dir, DICT_NAME, TRUE);
		if (p->languages[lang].dict == NULL)
		{
			log_message(ERROR, _("Can't find dictionary file for %s language"), lang_name);
			return FALSE;
		}

		p->languages[lang].proto = load_list(lang_dir, PROTO_NAME, TRUE);
		if (p->languages[lang].proto == NULL)
		{
			log_message(ERROR, _("Can't find protos file for %s language"), lang_name);
			return FALSE;
		}

		p->languages[lang].big_proto = load_list(lang_dir, BIG_PROTO_NAME, TRUE);
		if (p->languages[lang].big_proto == NULL)
		{
			log_message(ERROR, _("Can't find big protos file for %s language"), lang_name);
			return FALSE;
		}

		p->languages[lang].regexp = load_list(lang_dir, REGEXP_NAME, TRUE);
		if (p->languages[lang].regexp == NULL)
		{
			log_message(ERROR, _("Can't find regexp file for %s language"), lang_name);
			return FALSE;
		}

		p->languages[lang].pattern = load_list(lang_dir, PATTERN_NAME, TRUE);
		if (p->languages[lang].pattern == NULL)
		{
			log_message(WARNING, _("Can't find pattern file for %s language"), lang_name);
		}
		
		p->languages[lang].temp_dict = p->languages[lang].dict->clone(p->languages[lang].dict);

		load_lang = lang;
	}

	return TRUE;
}

static void xneur_config_clear(struct _xneur_config *p)
{
	free_structures(p);

	p->window_layouts		= list_char_init();
	p->manual_apps			= list_char_init();
	p->auto_apps			= list_char_init();
	p->layout_remember_apps		= list_char_init();
	p->excluded_apps		= list_char_init();
	p->abbreviations		= list_char_init();

	p->version	= NULL;
	p->osd_font	= NULL;
	p->languages	= NULL;
	p->actions	= NULL;
}

static int xneur_config_save(struct _xneur_config *p)
{
 	char *config_file_path_name = get_home_file_path_name(NULL, CONFIG_NAME);

	log_message(LOG, _("Saving main config to %s"), config_file_path_name);

	FILE *stream = fopen(config_file_path_name, "w");
	if (stream == NULL)
	{
		log_message(ERROR, _("Can't create file %s"), config_file_path_name);
		free(config_file_path_name);
		return FALSE;
	}

	free(config_file_path_name);

	fprintf(stream, "# It's a X Neural Switcher configuration file by XNeur\n# All values writted XNeur\n\n");

	fprintf(stream, "# Config version\nVersion %s\n\n", VERSION);
	fprintf(stream, "# Work in manual mode\nManualMode %s\n\n", p->get_bool_name(p->manual_mode));

	fprintf(stream, "# Level of messages program will write to output\n");
	fprintf(stream, "#LogLevel Error\n");
	fprintf(stream, "#LogLevel Warning\n");
	fprintf(stream, "#LogLevel Log\n");
	fprintf(stream, "#LogLevel Debug\n");
	fprintf(stream, "#LogLevel Trace\n");
	fprintf(stream, "LogLevel %s\n\n", p->get_log_level_name(p));

	fprintf(stream, "# Define used languages\n");
	fprintf(stream, "# See Settings page on http://www.xneur.ru for details\n");

	for (int lang = 0; lang < p->total_languages; lang++)
	{
		fprintf(stream, "AddLanguage %s %s %d ", p->languages[lang].name, p->languages[lang].dir, p->languages[lang].group);
		if (p->languages[lang].fixed)
			fprintf(stream, "%s\n", fix_names[0]);
		else
			fprintf(stream, "\n");
	}
	fprintf(stream, "\n");

	fprintf(stream, "# Define initial keyboard layout for all new applications\n");
	fprintf(stream, "DefaultXkbGroup %d\n\n", p->default_group);

	fprintf(stream, "# Add Applications names to exclude it from procces with xneur\n");
	fprintf(stream, "# Xneur will not process the input for this applications\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#ExcludeApp Gaim\n");
	for (int i = 0; i < p->excluded_apps->data_count; i++)
		fprintf(stream, "ExcludeApp %s\n", p->excluded_apps->data[i].string);
	fprintf(stream, "\n");

	fprintf(stream, "# Use this parameter to force set work mode in current application to Auto.\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#SetAutoApp Gedit\n");
	for (int i = 0; i < p->auto_apps->data_count; i++)
		fprintf(stream, "SetAutoApp %s\n", p->auto_apps->data[i].string);
	fprintf(stream, "\n");

	fprintf(stream, "# Use this parameter to force set work mode in current application to Manual.\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#SetManualApp Anjuta\n");
	for (int i = 0; i < p->manual_apps->data_count; i++)
		fprintf(stream, "SetManualApp %s\n", p->manual_apps->data[i].string);
	fprintf(stream, "\n");

	fprintf(stream, "# Binds hotkeys for some actions\n");
	for (int action = 0; action < MAX_HOTKEYS; action++)
	{
		fprintf(stream, "AddBind %s ", action_names[action]);

		const int total_modifiers = sizeof(modifier_names) / sizeof(modifier_names[0]);
		for (int i = 0; i < total_modifiers; i++)
		{
			if (p->hotkeys[action].modifiers & (1 << i))
				fprintf(stream, "%s ", modifier_names[i]);
		}

		if (p->hotkeys[action].key != NULL)
			fprintf(stream, "%s", p->hotkeys[action].key);
		fprintf(stream, "\n");
	}
	fprintf(stream, "\n");

	fprintf(stream, "# This option add user action when pressed key bind\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#AddAction Control Alt f firefox\n");
	for (int action = 0; action < p->actions_count; action++)
	{
		fprintf(stream, "AddAction ");

		const int total_modifiers = sizeof(modifier_names) / sizeof(modifier_names[0]);
		for (int i = 0; i < total_modifiers; i++)
		{
			if (p->actions[action].hotkey.modifiers & (1 << i))
				fprintf(stream, "%s ", modifier_names[i]);
		}

		fprintf(stream, "%s %s\n", p->actions[action].hotkey.key, p->actions[action].command);
	}
	fprintf(stream, "\n");

	fprintf(stream, "# Word Replacing\n# Ignore keyboard layout for abbreviations list\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#ReplaceAbbreviationIgnoreLayout No\n");
	fprintf(stream, "ReplaceAbbreviationIgnoreLayout %s\n\n", p->get_bool_name(p->abbr_ignore_layout));

	fprintf(stream, "# Abbreviations list\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#ReplaceAbbreviation xneur X Neural Switcher\n");
	for (int words = 0; words < p->abbreviations->data_count; words++)
		fprintf(stream, "ReplaceAbbreviation %s\n", p->abbreviations->data[words].string);
	fprintf(stream, "\n");

	fprintf(stream, "# This option enable or disable sound playing\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#PlaySounds No\n");
	fprintf(stream, "PlaySounds %s\n\n", p->get_bool_name(p->play_sounds));

	fprintf(stream, "# Binds sounds for some actions\n");
	for (int sound = 0; sound < MAX_NOTIFIES; sound++)
	{
		if (p->sounds[sound].file == NULL)
			fprintf(stream, "AddSound %s \n", notify_names[sound]);
		else
			fprintf(stream, "AddSound %s %s\n", notify_names[sound], p->sounds[sound].file);
	}
	fprintf(stream, "\n");

	fprintf(stream, "# This option enable or disable mouse processing\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#GrabMouse Yes\n");
	fprintf(stream, "GrabMouse %s\n\n", p->get_bool_name(p->grab_mouse));

	fprintf(stream, "# This option enable or disable self education of xneur\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#EducationMode No\n");
	fprintf(stream, "EducationMode %s\n\n", p->get_bool_name(p->educate));

	fprintf(stream, "# This option enable or disable layout remember for each window\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#LayoutRememberMode No\n");
	fprintf(stream, "LayoutRememberMode %s\n\n", p->get_bool_name(p->remember_layout));

	fprintf(stream, "# Use this parameter to force enable layout remember for each application, not window.\n");
	fprintf(stream, "# Option \"LayoutRememberMode\" must be enabled.\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#LayoutRememberModeForApp Gaim\n");
	for (int i = 0; i < p->layout_remember_apps->data_count; i++)
		fprintf(stream, "LayoutRememberModeForApp %s\n", p->layout_remember_apps->data[i].string);
	fprintf(stream, "\n");

	fprintf(stream, "# This option enable or disable saving selection text\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#SaveSelectionMode No\n");
	fprintf(stream, "SaveSelectionMode %s\n\n", p->get_bool_name(p->save_selection));

	fprintf(stream, "# This option define delay before sendind events to application (in milliseconds between 0 to 50).\n");
	fprintf(stream, "SendDelay %d\n\n", p->send_delay);

	fprintf(stream, "# This option enable or disable logging keyboard\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#SaveLog No\n");
	fprintf(stream, "SaveLog %s\n\n", p->get_bool_name(p->save_keyboard_log));

	fprintf(stream, "# This option enable or disable correction of iNCIDENTAL CapsLock\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#CorrectIncidentalCaps Yes\n");
	fprintf(stream, "CorrectIncidentalCaps %s\n\n", p->get_bool_name(p->correct_incidental_caps));

	fprintf(stream, "# This option enable or disable correction of two CApital letter\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#CorrectTwoCapitalLetter Yes\n");
	fprintf(stream, "CorrectTwoCapitalLetter %s\n\n", p->get_bool_name(p->correct_two_capital_letter));

	fprintf(stream, "# This option enable or disable flushing internal buffer when pressed Enter or Tab\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#FlushBufferWhenPressEnter Yes\n");
	fprintf(stream, "FlushBufferWhenPressEnter %s\n\n", p->get_bool_name(p->flush_buffer_when_press_enter));

	fprintf(stream, "# This option disable or enable processing word when pressed Enter or Tab\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#DontProcessWhenPressEnter Yes\n");
	fprintf(stream, "DontProcessWhenPressEnter %s\n\n", p->get_bool_name(p->dont_process_when_press_enter));

	fprintf(stream, "# This option disable or enable show OSD\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#ShowOSD Yes\n");
	fprintf(stream, "ShowOSD %s\n\n", p->get_bool_name(p->show_osd));

	fprintf(stream, "# This option set font for OSD\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#FontOSD -*-*-*-*-*-*-32-*-*-*-*-*-*-u\n");
	fprintf(stream, "FontOSD %s\n\n", p->osd_font);

	fprintf(stream, "# Binds OSDs for some actions\n");
	for (int notify = 0; notify < MAX_NOTIFIES; notify++)
	{
		if (p->osds[notify].file == NULL)
			fprintf(stream, "AddOSD %s\n", notify_names[notify]);
		else
			fprintf(stream, "AddOSD %s %s\n", notify_names[notify], p->osds[notify].file);
	}

	fprintf(stream, "\n# This option disable or enable show popup messages\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#ShowPopup Yes\n");
	fprintf(stream, "ShowPopup %s\n\n", p->get_bool_name(p->show_popup));

	fprintf(stream, "# Binds popup messages for some actions\n");
	for (int notify = 0; notify < MAX_NOTIFIES; notify++)
	{
		if (p->popups[notify].file == NULL)
			fprintf(stream, "AddPopup %s\n", notify_names[notify]);
		else
			fprintf(stream, "AddPopup %s %s\n", notify_names[notify], p->popups[notify].file);
	}

	fprintf(stream, "\n# This option disable or enable checking language on input process\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#CheckOnProcess Yes\n");
	fprintf(stream, "CheckOnProcess %s\n", p->get_bool_name(p->check_lang_on_process));

	fprintf(stream, "\n# This option disable or enable CapsLock use\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#DisableCapsLock Yes\n");
	fprintf(stream, "DisableCapsLock %s\n", p->get_bool_name(p->disable_capslock));

	fprintf(stream, "\n# This option disable or enable correction spaces befor punctuation\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#CorrectSpaceWithPunctuation No\n");
	fprintf(stream, "CorrectSpaceWithPunctuation %s\n", p->get_bool_name(p->correct_space_with_punctuation));

	fprintf(stream, "\n# This option disable or enable pattern mining and recognition (autocomplementation)\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#Autocomplementation No\n");
	fprintf(stream, "Autocomplementation %s\n", p->get_bool_name(p->autocomplementation));

	fprintf(stream, "\n# This option disable or enable adding space after autocomplementation\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#AddSpaceAfterAutocomplementation No\n");
	fprintf(stream, "AddSpaceAfterAutocomplementation %s\n", p->get_bool_name(p->add_space_after_autocomplementation));

	fprintf(stream, "\n");

	fprintf(stream, "# That's all\n");

	fclose(stream);

	return TRUE;
}

static int xneur_config_replace(struct _xneur_config *p)
{
	char *config_file_path_name		= get_file_path_name(NULL, CONFIG_NAME);
	char *config_backup_file_path_name	= get_file_path_name(NULL, CONFIG_BCK_NAME);

	log_message(LOG, _("Moving config file from %s to %s"), config_file_path_name, config_backup_file_path_name);

	remove(config_backup_file_path_name);

	if (rename(config_file_path_name, config_backup_file_path_name) != 0)
	{
		log_message(ERROR, _("Can't move file!"), config_backup_file_path_name);

		free(config_file_path_name);
		free(config_backup_file_path_name);
		return FALSE;
	}

	free(config_file_path_name);
	free(config_backup_file_path_name);

	return p->load(p);
}

static void xneur_config_save_dict(struct _xneur_config *p, int lang)
{
	if (!p->educate)
		return;

	log_message(LOG, _("Saving %s dictionary"), p->get_lang_name(p, lang));

	save_list(p->languages[lang].dict, p->get_lang_dir(p, lang), DICT_NAME);
}

static void xneur_config_save_pattern(struct _xneur_config *p, int lang)
{
	if (!p->educate)
		return;

	log_message(LOG, _("Saving %s pattern"), p->get_lang_name(p, lang));

	save_list(p->languages[lang].pattern, p->get_lang_dir(p, lang), PATTERN_NAME);
}

static char* xneur_config_get_lang_dir(struct _xneur_config *p, int lang)
{
	if (lang < 0 || lang >= p->total_languages)
		return NULL;

	int path_len = strlen(LANGUAGEDIR) + strlen(p->languages[lang].dir) + 2;
	char *path_file = (char *) malloc(path_len * sizeof(char));
	snprintf(path_file, path_len, "%s/%s", LANGUAGEDIR, p->languages[lang].dir);

	return path_file;
}

static char* xneur_config_get_lang_name(struct _xneur_config *p, int lang)
{
	if (lang < 0 || lang >= p->total_languages)
		return NULL;
	return p->languages[lang].name;
}

static int xneur_config_get_lang_group(struct _xneur_config *p, int lang)
{
	if (lang < 0 || lang >= p->total_languages)
		return -1;
	return p->languages[lang].group;
}

static int xneur_config_find_group_lang(struct _xneur_config *p, int group)
{
	for (int lang = 0; lang < p->total_languages; lang++)
	{
		if (p->languages[lang].group == group)
			return lang;
	}
	return -1;
}

static void xneur_config_add_language(struct _xneur_config *p, const char *name, const char *dir, int group, int fixed)
{
	if (name == NULL || dir == NULL)
	{
		log_message(ERROR, _("Can't add language with empty name or dir"));
		return;
	}

	p->languages = (struct _xneur_language *) realloc(p->languages, (p->total_languages + 1) * sizeof(struct _xneur_language));
	bzero(&(p->languages[p->total_languages]), sizeof(struct _xneur_language));

	p->languages[p->total_languages].name	= strdup(name);
	p->languages[p->total_languages].dir	= strdup(dir);
	p->languages[p->total_languages].group	= group;
	p->languages[p->total_languages].fixed	= fixed;
	p->total_languages++;
}

static const char* xneur_config_get_log_level_name(struct _xneur_config *p)
{
	return log_levels[p->log_level];
}

static void xneur_config_uninit(struct _xneur_config *p)
{
	free_structures(p);

	free(p->hotkeys);
	free(p->sounds);
	free(p->osds);
	free(p->popups);

	free(p);
}

struct _xneur_config* xneur_config_init(void)
{
	struct _xneur_config *p = (struct _xneur_config *) malloc(sizeof(struct _xneur_config));
	bzero(p, sizeof(struct _xneur_config));

	if (!check_memory_attached(p))
	{
		free(p);
		return NULL;
	}

	p->hotkeys = (struct _xneur_hotkey *) malloc(MAX_HOTKEYS * sizeof(struct _xneur_hotkey));
	bzero(p->hotkeys, MAX_HOTKEYS * sizeof(struct _xneur_hotkey));

	p->sounds = (struct _xneur_file *) malloc(MAX_NOTIFIES * sizeof(struct _xneur_file));
	bzero(p->sounds, MAX_NOTIFIES * sizeof(struct _xneur_file));

	p->osds = (struct _xneur_file *) malloc(MAX_NOTIFIES * sizeof(struct _xneur_file));
	bzero(p->osds, MAX_NOTIFIES * sizeof(struct _xneur_file));

	p->popups = (struct _xneur_file *) malloc(MAX_NOTIFIES * sizeof(struct _xneur_file));
	bzero(p->popups, MAX_NOTIFIES * sizeof(struct _xneur_file));

	p->log_level			= LOG;
	p->excluded_apps		= list_char_init();
	p->auto_apps			= list_char_init();
	p->manual_apps			= list_char_init();
	p->layout_remember_apps		= list_char_init();
	p->window_layouts		= list_char_init();
	p->abbreviations		= list_char_init();

	// Function mapping
	p->get_home_dict_path		= get_home_file_path_name;
	p->get_global_dict_path		= get_file_path_name;

	p->get_library_version		= xneur_config_get_library_version;
	p->get_bool_name		= xneur_config_get_bool_name;

	p->load				= xneur_config_load;
	p->clear			= xneur_config_clear;
	p->save				= xneur_config_save;
	p->replace			= xneur_config_replace;
	p->reload			= xneur_config_reload;
	p->kill				= xneur_config_kill;
	p->save_dict			= xneur_config_save_dict;
	p->save_pattern			= xneur_config_save_pattern;
	p->set_manual_mode		= xneur_config_set_manual_mode;
	p->is_manual_mode		= xneur_config_is_manual_mode;
	p->set_pid			= xneur_config_set_pid;
	p->get_pid			= xneur_config_get_pid;
	p->get_lang_dir			= xneur_config_get_lang_dir;
	p->get_lang_name		= xneur_config_get_lang_name;
	p->get_lang_group		= xneur_config_get_lang_group;
	p->find_group_lang		= xneur_config_find_group_lang;
	p->add_language			= xneur_config_add_language;
	p->get_log_level_name		= xneur_config_get_log_level_name;

	p->uninit			= xneur_config_uninit;

	return p;
}
