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

#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "xnconfig.h"

extern struct _xneur_config *xconfig;

struct _xneur_statistic
{
	char *start_time;
	char *end_time;

	int key_count;
	
	int change_incidental_caps;
	int change_two_capital_letter;
	int change_word_to_0;
	int change_word_to_1;
	int change_word_to_2;
	int change_word_to_3;
	int change_syll_to_0;
	int change_syll_to_1;
	int change_syll_to_2;
	int change_syll_to_3;
	int change_selection;
	int change_string_to_0;
	int change_string_to_1;
	int change_string_to_2;
	int change_string_to_3;
	int change_abreviation;

	int action_change_word;
	int action_change_string;
	int action_change_mode;
	int action_change_selected;
	int action_translit_selected;
	int action_changecase_selected;
	int action_change_clipboard;
	int action_translit_clipboard;
	int action_changecase_clipboard;
	int action_enable_layout_0;
	int action_enable_layout_1;
	int action_enable_layout_2;
	int action_enable_layout_3;
	int action_rotate_layout;
	int action_replace_abbreviation;
	int action_autocomplementation;
} statistic;

int on_init(void)
{
	statistic.start_time = malloc(256 * sizeof(char));
	statistic.end_time = malloc(256 * sizeof(char));

	statistic.key_count = 0;
	
	statistic.change_incidental_caps = 0;
	statistic.change_two_capital_letter = 0;
	statistic.change_word_to_0 = 0;
	statistic.change_word_to_1 = 0;
	statistic.change_word_to_2 = 0;
	statistic.change_word_to_3 = 0;
	statistic.change_syll_to_0 = 0;
	statistic.change_syll_to_1 = 0;
	statistic.change_syll_to_2 = 0;
	statistic.change_syll_to_3 = 0;
	statistic.change_selection = 0;
	statistic.change_string_to_0 = 0;
	statistic.change_string_to_1 = 0;
	statistic.change_string_to_2 = 0;
	statistic.change_string_to_3 = 0;
	statistic.change_abreviation = 0;

	statistic.action_change_word = 0;
	statistic.action_change_string = 0;
	statistic.action_change_mode = 0;
	statistic.action_change_selected = 0;
	statistic.action_translit_selected = 0;
	statistic.action_changecase_selected = 0;
	statistic.action_change_clipboard = 0;
	statistic.action_translit_clipboard = 0;
	statistic.action_changecase_clipboard = 0;
	statistic.action_enable_layout_0 = 0;
	statistic.action_enable_layout_1 = 0;
	statistic.action_enable_layout_2 = 0;
	statistic.action_enable_layout_3 = 0;
	statistic.action_rotate_layout = 0;
	statistic.action_replace_abbreviation = 0;
	statistic.action_autocomplementation = 0;
	
	printf("[PLG] Plugin for keyboard statistic initialized\n");
	return (0);
}

int on_xneur_start(void)
{
	time_t curtime = time(NULL);
	struct tm *loctime = localtime(&curtime);
	if (loctime == NULL)
		return 0;

	strftime(statistic.start_time, 256, "%c", loctime);
	
	printf("[PLG] Plugin for keyboard statistic receive xneur start\n");
	return (0);
}

int on_xneur_reload(void)
{
	printf("[PLG] Plugin receive xneur stop\n");
	return (0);
}

int on_xneur_stop(void)
{
	time_t curtime = time(NULL);
	struct tm *loctime = localtime(&curtime);
	if (loctime == NULL)
		return 0;

	strftime(statistic.end_time, 256, "%c", loctime);

	printf("XNeur statistic (from %s to %s)\n", statistic.start_time, statistic.end_time);
	printf("Count:\n");
	if (statistic.key_count != 0)
	{
		printf("	Key press count = %d\n", statistic.key_count);
	}
	if (statistic.change_incidental_caps != 0)
	{
		printf("	Change incidental caps count = %d\n", statistic.change_incidental_caps);
	}
	if (statistic.change_two_capital_letter != 0)
	{
		printf("	Change two capital letter count = %d\n", statistic.change_two_capital_letter);
	}
	if (statistic.change_word_to_0 != 0)
	{	
		printf("	Change word to layout 0 count = %d\n", statistic.change_word_to_0);
	}
	if (statistic.change_word_to_1 != 0)
	{
		printf("	Change word to layout 1 count = %d\n", statistic.change_word_to_1);
	}
	if (statistic.change_word_to_2 != 0)
	{
		printf("	Change word to layout 2 count = %d\n", statistic.change_word_to_2);
	}
	if (statistic.change_word_to_3 != 0)
	{
		printf("	Change word to layout 3 count = %d\n", statistic.change_word_to_3);
	}
	if (statistic.change_syll_to_0 != 0)
	{
		printf("	Change syllable to layout 0 count = %d\n", statistic.change_syll_to_0);
	}
	if (statistic.change_syll_to_1 != 0)
	{
		printf("	Change syllable to layout 1 count = %d\n", statistic.change_syll_to_1);
	}
	if (statistic.change_syll_to_2 != 0)
	{
		printf("	Change syllable to layout 2 count = %d\n", statistic.change_syll_to_2);
	}
	if (statistic.change_syll_to_3 != 0)
	{
		printf("	Change syllable to layout 3 count = %d\n", statistic.change_syll_to_3);
	}
	if (statistic.change_selection != 0)
	{
		printf("	Change selection count = %d\n", statistic.change_selection);
	}
	if (statistic.change_string_to_0 != 0)
	{
		printf("	Change string to layout 0 count = %d\n", statistic.change_string_to_0);
	}
	if (statistic.change_string_to_1 != 0)
	{
		printf("	Change string to layout 1 count = %d\n", statistic.change_string_to_1);
	}
	if (statistic.change_string_to_2 != 0)
	{
		printf("	Change string to layout 2 count = %d\n", statistic.change_string_to_2);
	}
	if (statistic.change_string_to_3 != 0)
	{
		printf("	Change string to layout 3 count = %d\n", statistic.change_string_to_3);
	}
	if (statistic.change_abreviation != 0)
	{
		printf("	Change abbreviations count = %d\n", statistic.change_abreviation);
	}

	if (statistic.action_change_word != 0)
	{
		printf("	Manual change word count = %d\n", statistic.action_change_word);
	}
	if (statistic.action_change_string != 0)
	{
		printf("	Manual change string count = %d\n", statistic.action_change_string);
	}
	if (statistic.action_change_mode != 0)
	{
		printf("	Manual change mode count = %d\n", statistic.action_change_mode);
	}
	if (statistic.action_change_selected != 0)
	{
		printf("	Manual change selected count = %d\n", statistic.action_change_selected);
	}
	if (statistic.action_translit_selected != 0)
	{
		printf("	Manual transit selected count = %d\n", statistic.action_translit_selected);
	}
	if (statistic.action_changecase_selected != 0)
	{
		printf("	Manual changecase selected count = %d\n", statistic.action_changecase_selected);
	}
	if (statistic.action_change_clipboard != 0)
	{
		printf("	Manual change clipboard count = %d\n", statistic.action_change_clipboard);
	}
	if (statistic.action_translit_clipboard != 0)
	{
		printf("	Manual translit clipboard count = %d\n", statistic.action_translit_clipboard);
	}
	if (statistic.action_changecase_clipboard != 0)
	{
		printf("	Manual changecase clipboard count = %d\n", statistic.action_changecase_clipboard);
	}
	if (statistic.action_enable_layout_0 != 0)
	{
		printf("	Manual enable layout 0 count = %d\n", statistic.action_enable_layout_0);
	}
	if (statistic.action_enable_layout_1 != 0)
	{
		printf("	Manual enable layout 1 count = %d\n", statistic.action_enable_layout_1);
	}
	if (statistic.action_enable_layout_2 != 0)
	{
		printf("	Manual enable layout 2 count = %d\n", statistic.action_enable_layout_2);
	}
	if (statistic.action_enable_layout_3 != 0)
	{
		printf("	Manual enable layout 3 count = %d\n", statistic.action_enable_layout_3);
	}
	if (statistic.action_rotate_layout != 0)
	{
		printf("	Manual rotate layout count = %d\n", statistic.action_rotate_layout);
	}
	if (statistic.action_replace_abbreviation != 0)
	{
		printf("	Manual replace abbreviation count = %d\n", statistic.action_replace_abbreviation);
	}
	if (statistic.action_autocomplementation != 0)
	{
		printf("	Autocomplementation count = %d\n", statistic.action_autocomplementation);
	}

	printf("[PLG] Plugin for keyboard statistic receive xneur stop\n");
	return (0);
}

int on_key_press(KeySym key, int modifier_mask)
{
	statistic.key_count++;
	
	printf("[PLG] Plugin for keyboard statistic receive KeyPress '%s' with mask %d\n", XKeysymToString(key), modifier_mask);
	return (0);
}

int on_hotkey_action(enum _hotkey_action ha)
{
	switch (ha)
	{
		case ACTION_NONE:
		{
			break;
		}
		case ACTION_CHANGE_MODE:	// User needs to change current work mode
		{
			statistic.action_change_mode++;
			break;
		}
		case ACTION_CHANGE_SELECTED:
		{
			statistic.action_change_selected++;
			break;
		}
		case ACTION_TRANSLIT_SELECTED:
		{
			statistic.action_translit_selected++;
			break;
		}
		case ACTION_CHANGECASE_SELECTED:
		{
			statistic.action_changecase_selected++;
			break;
		}
		case ACTION_CHANGE_CLIPBOARD:
		{
			statistic.action_change_clipboard++;
			break;
		}
		case ACTION_TRANSLIT_CLIPBOARD:
		{
			statistic.action_translit_clipboard++;
			break;
		}
		case ACTION_CHANGECASE_CLIPBOARD:
		{
			statistic.action_changecase_clipboard++;
			break;
		}
		case ACTION_CHANGE_STRING:	// User needs to change current string
		{
			statistic.action_change_string++;
			break;
		}
		case ACTION_CHANGE_WORD:	// User needs to cancel last change
		{
			statistic.action_change_word++;
			break;
		}
		case ACTION_ENABLE_LAYOUT_0:
		{
			statistic.action_enable_layout_0++;
			break;
		}
		case ACTION_ENABLE_LAYOUT_1:
		{
			statistic.action_enable_layout_1++;
			break;
		}
		case ACTION_ENABLE_LAYOUT_2:
		{
			statistic.action_enable_layout_2++;
			break;
		}
		case ACTION_ENABLE_LAYOUT_3:
		{
			statistic.action_enable_layout_3++;
			break;
		}
		case ACTION_ROTATE_LAYOUT:
		{
			statistic.action_rotate_layout++;
			break;
		}
		case ACTION_AUTOCOMPLEMENTATION:
		{
			statistic.action_autocomplementation++;
			break;
		}
		case ACTION_REPLACE_ABBREVIATION: // User needs to replace acronym
		{
			statistic.action_replace_abbreviation++;
			break;
		}
	}
	
	return (0);
}

int on_change_action(enum _change_action ca)
{
	switch (ca)
	{
		case CHANGE_INCIDENTAL_CAPS:
		{
			statistic.change_incidental_caps++;
			break;
		}
		case CHANGE_TWO_CAPITAL_LETTER:
		{
			statistic.change_two_capital_letter++;
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_0:
		{
			statistic.change_word_to_0++;
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_1:
		{
			statistic.change_word_to_1++;
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_2:
		{
			statistic.change_word_to_2++;
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_3:
		{
			statistic.change_word_to_3++;
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_0:
		{
			statistic.change_syll_to_0++;
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_1:
		{
			statistic.change_syll_to_1++;
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_2:
		{
			statistic.change_syll_to_2++;
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_3:
		{
			statistic.change_syll_to_3++;
			break;
		}
		case CHANGE_SELECTION:
		{
			statistic.change_selection++;
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_0:
		{
			statistic.change_string_to_0++;
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_1:
		{
			statistic.change_string_to_1++;
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_2:
		{
			statistic.change_string_to_2++;
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_3:
		{
			statistic.change_string_to_3++;
			break;
		}
		case CHANGE_ABBREVIATION:
		{
			statistic.change_abreviation++;
			break;
		}
	}
	
	return (0);
}

int on_fini(void)
{
	free(statistic.start_time);
	free(statistic.end_time);
	
	printf("[PLG] Plugin receive finalisation\n");
	return (0);
}
