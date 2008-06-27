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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "xnconfig_files.h"
#include "xnconfig_memory.h"

#include "types.h"
#include "utils.h"
#include "list_char.h"
#include "log.h"

#include "xnconfig.h"

#define LIBRARY_API_VERSION_MAJOR	3
#define LIBRARY_API_VERSION_MINOR	1
#define OPTIONS_DELIMETER		" "

static const char *log_levels[] =	{"Error", "Warning", "Log", "Debug", "Trace"};
static const char *bool_names[] =	{"No", "Yes"};
static const char *modifier_names[] =	{"Shift", "Control", "Alt", "Super"};
static const char *mode_names[] =	{"Manual", "Auto"};
static const char *flag_names[] =	{"Layout1Flag", "Layout2Flag", "Layout3Flag", "Layout4Flag"};

static const char *option_names[] = 	{
						"DefaultMode", "ExcludeApp", "AddBind", "LogLevel", "AddLanguage", "VowelLetter",
						"ConsonantLetter", "NoFirstLetter", "SetAutoApp", "SetManualApp", "GrabMouse",
						"EducationMode", "Version", "LayoutRememberMode", "SaveSelectionMode",
						"DefaultXkbGroup", "AddSound", "PlaySound", "SendDelay", "LayoutRememberModeForApp",
						"DrawFlagApp", "AddFlagPixmap", "SaveLog", "Locale"
					};
static const char *action_names[] =	{
						"ChangeWord", "ChangeString", "ChangeMode", 
						"ChangeSelected", "TranslitSelected", "ChangecaseSelected",
						"EnableLayout1", "EnableLayout2", "EnableLayout3", "EnableLayout4"
					};
static const char *sound_names[] =	{
						"PressKeyLayout1", "PressKeyLayout2", "PressKeyLayout3", "PressKeyLayout4",
						"EnableLayout1", "EnableLayout2", "EnableLayout3", "EnableLayout4",
						"AutomaticChangeWord", "ManualChangeWord", "ChangeString", 
						"ChangeSelected", "TranslitSelected", "ChangecaseSelected"
					};

static int load_lang = -1;

pid_t getsid(pid_t pid);
char* strsep(char **stringp, const char *delim);

static char* get_word(char **string)
{
	return strsep(string, OPTIONS_DELIMETER);
}

static int get_option_index(const char **options, char *option)
{
	const int options_count = sizeof(options) / sizeof(options[0]);

	for (int i = 0; i < options_count; i++)
	{
		if (strcmp(option, options[i]) == 0)
			return i;
	}
	return -1;
}

static void parse_line(struct _xneur_config *p, char *line)
{
	if (line[0] == '#')
		return;

	char *option = get_word(&line);

	int index = get_option_index(option);
	if (index == -1)
	{
		log_message(WARNING, "Unrecognized option \"%s\" detected", option);
		return;
	}

	char *param = get_word(&line);
	if (param == NULL)
	{
		log_message(ERROR, "Param mismatch for option %s", option);
		return;
	}

	switch (index)
	{
		case 0: // Get Default Mode (Manual/Auto)
		{
			int index = get_option_index(mode_names, param);
			if (index == -1)
			{
				log_message(WARNING, "Invalid value for default mode specified");
				break;
			}

			p->set_current_mode(p, index);
			break;
		}
		case 1: // Get Applications Names
		{
			p->excluded_apps->add(p->excluded_apps, param);
			break;
		}
		case 2: // Get Keyboard Binds
		{
			int action = get_option_index(action_names, param);
			if (action == -1)
			{
				log_message(WARNING, "Invalid value for action name specified");
				break;
			}

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
				log_message(WARNING, "Invalid value for log level specified");
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

			if (dir == NULL || group == NULL)
			{
				log_message(ERROR, "Argument number mismatch for AddLanguage option");
				break;
			}

			p->add_language(p, param, dir, atoi(group));
			break;
		}
		case 5: // Get Vowel Letter
		{
			p->languages[load_lang].vowel_letter = strdup(param);
			break;
		}
		case 6: // Get Consonant Letter
		{
			p->languages[load_lang].consonant_letter = strdup(param);
			break;
		}
		case 7: // Get No First Letter
		{
			p->languages[load_lang].nofirst_letter = strdup(param);
			break;
		}
		case 8: // Get Auto Processing Applications
		{
			p->auto_apps->add(p->auto_apps, param);
			break;
		}
		case 9: // Get Manual Processing Applications
		{
			p->manual_apps->add(p->manual_apps, param);
			break;
		}
		case 10: // Get Mouse Grab Mode
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, "Invalid value for mouse grab mode specified");
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
				log_message(WARNING, "Invalid value for education mode specified");
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
				log_message(WARNING, "Invalid value for remember layout mode specified");
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
				log_message(WARNING, "Invalid value for save selection mode specified");
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
			int sound = get_option_index(sound_names, param);
			if (sound == -1)
			{
				log_message(WARNING, "Invalid value for sound action name specified");
				break;
			}

			p->sounds[sound].file = strdup(get_word(&line));
			break;
		}
		case 17: // Play Sound
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, "Invalid value for play sounds mode specified");
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
				log_message(WARNING, "Send dealy must be between 0 and 50");
				p->send_delay = 0;
			}
			break;
		}
		case 19: // layout remember for each application
		{
			p->layout_remember_apps->add(p->layout_remember_apps, param);
			break;
		}
		case 20: // Get Draw Flag Applications
		{
			p->draw_flag_apps->add(p->draw_flag_apps, param);
			break;
		}
		case 21: // Flags
		{
			int flag = get_option_index(flag_names, param);
			if (flag == -1)
			{
				log_message(WARNING, "Invalid value for flag layout name specified");
				break;
			}

			p->flags[flag].file = strdup(get_word(&line));
			break;
		}
		case 22: // Save Keyboard Log
		{
			int index = get_option_index(bool_names, param);
			if (index == -1)
			{
				log_message(WARNING, "Invalid value for save keyboard log mode specified");
				break;
			}

			p->save_keyboard_log = index;
			break;
		}
		case 23: // Working locale
		{
			p->locale = strdup(param);
			break;
		}
	}
}

static int parse_config_file(struct _xneur_config *p, const char *dir_name, const char *file_name)
{	
	struct _list_char *list = load_list(dir_name, file_name, FALSE);
	if (list == NULL)
	{
		log_message(ERROR, "Can't find config file %s", file_name);
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
	p->draw_flag_apps->uninit(p->draw_flag_apps);
	
	if (p->version != NULL)
	{
		free(p->version);
		p->version = NULL;
	}
	
	for (int lang = 0; lang < p->total_languages; lang++)
	{
		if (p->languages[lang].temp_dicts != NULL)
			p->languages[lang].temp_dicts->uninit(p->languages[lang].temp_dicts);

		if (p->languages[lang].dicts != NULL)
			p->languages[lang].dicts->uninit(p->languages[lang].dicts);

		if (p->languages[lang].protos != NULL)
			p->languages[lang].protos->uninit(p->languages[lang].protos);

		if (p->languages[lang].big_protos != NULL)
			p->languages[lang].big_protos->uninit(p->languages[lang].big_protos);

		if (p->languages[lang].regexp != NULL)
			p->languages[lang].regexp->uninit(p->languages[lang].regexp);

		if (p->languages[lang].vowel_letter != NULL)
			free(p->languages[lang].vowel_letter);

		if (p->languages[lang].consonant_letter != NULL)
			free(p->languages[lang].consonant_letter);

		if (p->languages[lang].nofirst_letter != NULL)
			free(p->languages[lang].nofirst_letter);

		free(p->languages[lang].name);
		free(p->languages[lang].dir);
	}

	p->total_languages = 0;

	if (p->languages != NULL)
	{
		free(p->languages);
		p->languages = NULL;
	}

	for (int action = 0; action < MAX_HOTKEYS; action++)
	{
		if (p->hotkeys[action].key != NULL)
			free(p->hotkeys[action].key);
	}
	
	for (int sound = 0; sound < MAX_SOUNDS; sound++)
	{
		if (p->sounds[sound].file != NULL)
			free(p->sounds[sound].file);
	}

	for (int flag = 0; flag < MAX_FLAGS; flag++)
	{
		if (p->flags[flag].file != NULL)
			free(p->flags[flag].file);
	}
	
	bzero(p->hotkeys, MAX_HOTKEYS * sizeof(struct _xneur_hotkey));
	bzero(p->sounds, MAX_SOUNDS * sizeof(struct _xneur_file));
	bzero(p->flags, MAX_FLAGS * sizeof(struct _xneur_file));
}

char *xneur_config_get_bool_name(int option)
{
	return bool_names[option];
}

char *xneur_config_get_mode_name(struct _xneur_config *p)
{
	return mode_names[p->get_current_mode(p)];
}

char *xneur_config_get_log_level_name(struct _xneur_config *p)
{
	return log_levels[xconfig->log_level];
}

void xneur_config_reload(struct _xneur_config *p)
{
	int xneur_pid = p->xneur_data->xneur_pid;
	if (xneur_pid > 0)
		kill(xneur_pid, SIGHUP);
}

void xneur_config_set_pid(struct _xneur_config *p, int pid)
{
	p->xneur_data->xneur_pid = pid;
}

int xneur_config_kill(struct _xneur_config *p)
{
	int xneur_pid = p->xneur_data->xneur_pid;
	if (xneur_pid <= 0)
		return FALSE;

	if (kill(xneur_pid, SIGTERM) == -1)
		return FALSE;

	xneur_config_set_pid(p, 0);

	return TRUE;
}

int xneur_config_get_pid(struct _xneur_config *p)
{
	int xneur_pid = p->xneur_data->xneur_pid;
	if (xneur_pid <= 0)
		return -1;

	if (getsid(xneur_pid) == -1)
		return -1;

	return p->xneur_data->xneur_pid;
}

void xneur_config_set_current_mode(struct _xneur_config *p, int mode)
{
	p->xneur_data->xneur_mode = mode;
}

int xneur_config_get_current_mode(struct _xneur_config *p)
{
	return p->xneur_data->xneur_mode;
}

int xneur_config_load(struct _xneur_config *p)
{
	if (!parse_config_file(p, NULL, CONFIG_NAME))
		return FALSE;

	if (p->total_languages == 0)
	{
		log_message(ERROR, "No languages specified in config file");
		return FALSE;
	}

	for (int lang = 0; lang < p->total_languages; lang++)
	{
		char *lang_dir	= p->get_lang_dir(p, lang);
		char *lang_name	= p->get_lang_name(p, lang);

		p->languages[lang].dicts = load_list(lang_dir, DICT_NAME, TRUE);
		if (p->languages[lang].dicts == NULL)
		{
			log_message(ERROR, "Can't find dictionary file for %s language", lang_name);
			return FALSE;
		}

		p->languages[lang].protos = load_list(lang_dir, PROTO_NAME, TRUE);
		if (p->languages[lang].protos == NULL)
		{
			log_message(ERROR, "Can't find protos file for %s language", lang_name);
			return FALSE;
		}

		p->languages[lang].big_protos = load_list(lang_dir, BIG_PROTO_NAME, TRUE);
		if (p->languages[lang].big_protos == NULL)
		{
			log_message(ERROR, "Can't find big protos file for %s language", lang_name);
			return FALSE;
		}

		p->languages[lang].regexp = load_list(lang_dir, REGEXP_NAME, TRUE);
		if (p->languages[lang].regexp == NULL)
		{
			log_message(ERROR, "Can't find regexp file for %s language", lang_name);
			return FALSE;
		}

		p->languages[lang].temp_dicts = p->languages[lang].dicts->clone(p->languages[lang].dicts);

		load_lang = lang;
		if (!parse_config_file(p, lang_dir, LANGDEF_NAME))
			return FALSE;
	}
	
	return TRUE;
}

void xneur_config_clear(struct _xneur_config *p)
{
	free_structures(p);

	p->window_layouts	= list_char_init();
	p->manual_apps		= list_char_init();
	p->auto_apps		= list_char_init();
	p->layout_remember_apps	= list_char_init();
	p->excluded_apps	= list_char_init();
	p->draw_flag_apps	= list_char_init();
}

int xneur_config_save(struct _xneur_config *p)
{
 	char *config_file_path_name = get_home_file_path_name(NULL, CONFIG_NAME);

	log_message(LOG, "Saving main config to %s", config_file_path_name);

	FILE *stream = fopen(config_file_path_name, "w");
	if (stream == NULL)
	{
		log_message(ERROR, "Can't create file %s", config_file_path_name);
		free(config_file_path_name);
		return FALSE;
	}

	free(config_file_path_name);

	fprintf(stream, "# It's a X Neural Switcher configuration file by XNeur\n# All values writted XNeur\n\n");

	fprintf(stream, "# Config version\nVersion %s\n\n", VERSION);
	fprintf(stream, "# Working locale\Locale %s\n\n", p->locale);
	fprintf(stream, "# Default work mode\nDefaultMode %s\n\n", p->get_mode_name(p));

	fprintf(stream, "# Level of messages program will write to output\n");
	fprintf(stream, "#LogLevel Error\n");
	fprintf(stream, "#LogLevel Warning\n");
	fprintf(stream, "#LogLevel Log\n");
	fprintf(stream, "#LogLevel Debug\n");
	fprintf(stream, "LogLevel %s\n\n", p->get_log_level_name());

	fprintf(stream, "# Define used languages\n");
	fprintf(stream, "# See Settings page on http://www.xneur.ru for details\n");

	for (int lang = 0; lang < p->total_languages; lang++)
		fprintf(stream, "AddLanguage %s %s %d\n", p->languages[lang].name, p->languages[lang].dir, p->languages[lang].group);
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
		for (int i = 0; i < total_modifiers; i++)
		{
			if (p->hotkeys[action].modifiers & (0x1 << i))
				fprintf(stream, "%s ", modifier_names[i]);
		}
		if (p->hotkeys[action].key == NULL)
			fprintf(stream, "\n");
		else
			fprintf(stream, "%s\n", p->hotkeys[action].key);
	}
	fprintf(stream, "\n");

	fprintf(stream, "# This option enable or disable sound playing\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#PlaySound No\n");
	if (p->sound_mode)
		fprintf(stream, "PlaySound Yes\n\n");
	else
		fprintf(stream, "PlaySound No\n\n");
	
	fprintf(stream, "# Binds sounds for some actions\n");
	for (int sound = 0; sound < MAX_SOUNDS; sound++)
	{
		if (p->sounds[sound].file == NULL)
			fprintf(stream, "AddSound %s\n", sound_names[sound]);
		else
			fprintf(stream, "AddSound %s %s\n", sound_names[sound], p->sounds[sound].file);
	}
	fprintf(stream, "\n");
	
	fprintf(stream, "# This option enable or disable mouse processing\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#GrabMouse Yes\n");
	if (p->mouse_processing_mode)
		fprintf(stream, "GrabMouse Yes\n\n");
	else
		fprintf(stream, "GrabMouse No\n\n");

	fprintf(stream, "# This option enable or disable self education of xneur\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#EducationMode No\n");
	if (p->education_mode)
		fprintf(stream, "EducationMode Yes\n\n");
	else
		fprintf(stream, "EducationMode No\n\n");

	fprintf(stream, "# This option enable or disable layout remember for each window\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#LayoutRememberMode No\n");
	if (p->layout_remember_mode)
		fprintf(stream, "LayoutRememberMode Yes\n\n");
	else
		fprintf(stream, "LayoutRememberMode No\n\n");

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
	if (p->save_selection_mode)
		fprintf(stream, "SaveSelectionMode Yes\n\n");
	else
		fprintf(stream, "SaveSelectionMode No\n\n");

	fprintf(stream, "# This option define delay before sendind events to application (in milliseconds between 0 to 50).\n");
	fprintf(stream, "SendDelay %d\n\n", p->send_delay);
	
	fprintf(stream, "# Binds pixmaps for some layouts (pixmap only in xpm format)\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#AddFlagPixmap <Layout1Flag|Layout2Flag|Layout3Flag|Layout4Flag> English.xpm\n");			
	for (int flag = 0; flag < MAX_FLAGS; flag++)
	{
		if (p->flags[flag].file == NULL)
			continue;

		fprintf(stream, "AddFlagPixmap %s %s\n", flag_names[flag], p->flags[flag].file);
	}
	fprintf(stream, "\n");
			
	fprintf(stream, "# Add Applications names to draw flag in window\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#DrawFlagApp Gedit\n");
	for (int i = 0; i < p->draw_flag_apps->data_count; i++)
		fprintf(stream, "DrawFlagApp %s\n", p->draw_flag_apps->data[i].string);
	fprintf(stream, "\n");
			
	fprintf(stream, "# This option enable or disable logging keyboard\n");
	fprintf(stream, "# Example:\n");
	fprintf(stream, "#SaveLog No\n");
	if (p->save_log_mode)
		fprintf(stream, "SaveLog Yes\n\n");
	else
		fprintf(stream, "SaveLog No\n\n");
			
	fprintf(stream, "# That's all\n");

	fclose(stream);

	return TRUE;
}

int xneur_config_replace(struct _xneur_config *p)
{
	char *config_file_path_name = get_file_path_name(NULL, CONFIG_NAME);
	char *config_backup_file_path_name = get_file_path_name(NULL, CONFIG_BCK_NAME);

	log_message(LOG, "Moving config file from %s to %s", config_file_path_name, config_backup_file_path_name);
	
	remove(config_backup_file_path_name);

	if (rename(config_file_path_name, config_backup_file_path_name) != 0)
	{
		log_message(ERROR, "Can't move file!", config_backup_file_path_name);

		free(config_file_path_name);
		free(config_backup_file_path_name);
		return FALSE;
	}

	free(config_file_path_name);
	free(config_backup_file_path_name);

	return p->load(p);
}

void xneur_config_save_dicts(struct _xneur_config *p, int lang)
{
	if (!p->educate)
		return;
	
	log_message(LOG, "Saving %s dictionary", p->get_lang_name(p, lang));

	save_list(p->languages[lang].dicts, p->get_lang_dir(p, lang), DICT_NAME);
}

char* xneur_config_get_lang_dir(struct _xneur_config *p, int lang)
{
	if (lang < 0 || lang >= p->total_languages)
		return NULL;
	return p->languages[lang].dir;
}

char* xneur_config_get_lang_name(struct _xneur_config *p, int lang)
{
	if (lang < 0 || lang >= p->total_languages)
		return NULL;
	return p->languages[lang].name;
}

int xneur_config_find_group_lang(struct _xneur_config *p, int group)
{
	for (int lang = 0; lang < p->total_languages; lang++)
	{
		if (p->languages[lang].group == group)
			return lang;
	}
	return NO_LANGUAGE;
}

int xneur_config_get_lang_group(struct _xneur_config *p, int lang)
{
	if (lang < 0 || lang >= p->total_languages)
		return -1;
	return p->languages[lang].group;
}

void xneur_config_get_library_api_version(int *major_version, int *minor_version)
{
	*major_version = LIBRARY_API_VERSION_MAJOR;
	*minor_version = LIBRARY_API_VERSION_MINOR;
}

void xneur_xonfig_add_language(struct _xneur_config *p, const char *name, const char *dir, int group)
{
	if (name == NULL || dir == NULL)
	{
		log_message(ERROR, "Can't add language with empty name or dir");
		return;
	}

	p->languages = (struct _xneur_language *) realloc(p->languages, (p->total_languages + 1) * sizeof(struct _xneur_language));
	if (p->languages == NULL)
		return;
	bzero(&(p->languages[p->total_languages]), sizeof(struct _xneur_language));

	p->languages[p->total_languages].name	= strdup(name);
	p->languages[p->total_languages].dir	= strdup(dir);
	p->languages[p->total_languages].group	= group;
	
	p->total_languages++;
}

void xneur_config_uninit(struct _xneur_config *p)
{
	free_structures(p);

	free(p->hotkeys);
	free(p->sounds);
	free(p->flags);
	
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

	p->sounds = (struct _xneur_file *) malloc(MAX_SOUNDS * sizeof(struct _xneur_file));
	bzero(p->sounds, MAX_SOUNDS * sizeof(struct _xneur_file));

	p->flags = (struct _xneur_file *) malloc(MAX_FLAGS * sizeof(struct _xneur_file));
	bzero(p->flags, MAX_FLAGS * sizeof(struct _xneur_file));
	
	p->log_level			= LOG;
	p->excluded_apps		= list_char_init();
	p->auto_apps			= list_char_init();
	p->manual_apps			= list_char_init();
	p->layout_remember_apps		= list_char_init();
	p->window_layouts		= list_char_init();
	p->draw_flag_apps		= list_char_init();
		
	// Function mapping
	p->get_dict_path		= get_file_path_name;
	p->get_home_dict_path		= get_home_file_path_name;

	p->get_library_api_version	= xneur_config_get_library_api_version;

	p->load				= xneur_config_load;
	p->clear			= xneur_config_clear;
	p->save				= xneur_config_save;
	p->replace			= xneur_config_replace;
	p->reload			= xneur_config_reload;
	p->kill				= xneur_config_kill;
	p->save_dicts			= xneur_config_save_dicts;
	p->set_current_mode		= xneur_config_set_current_mode;
	p->get_current_mode		= xneur_config_get_current_mode;
	p->set_pid			= xneur_config_set_pid;
	p->get_pid			= xneur_config_get_pid;
	p->get_lang_dir			= xneur_config_get_lang_dir;
	p->get_lang_name		= xneur_config_get_lang_name;
	p->get_lang_group		= xneur_config_get_lang_group;
	p->find_group_lang		= xneur_config_find_group_lang;
	p->add_language			= xneur_xonfig_add_language;

	p->get_bool_name		= xneur_config_get_bool_name;
	p->get_log_level_name		= xneur_config_get_log_level_name;
	p->get_mode_name		= xneur_config_get_mode_name;

	p->uninit			= xneur_config_uninit;

	p->set_current_mode(p, AUTO_MODE);
	
	return p;
}
