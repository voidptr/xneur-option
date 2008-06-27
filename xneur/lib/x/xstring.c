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

#include <X11/Xlocale.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#include "xnconfig.h"
#include "xnconfig_files.h"

#include "xwindow.h"
#include "xevent.h"
#include "xkeymap.h"
#include "xutils.h"

#include "types.h"
#include "text.h"
#include "conversion.h"
#include "log.h"

#include "xstring.h"

#define INIT_STRING_LENGTH 64

extern struct _xneur_config *xconfig;
extern struct _xwindow *main_window;

Window last_log_window = 0;

// Private
static void set_new_size(struct _xstring *p, int new_size)
{
	p->cur_size		= new_size;
	p->content		= (char *) realloc(p->content, p->cur_size * sizeof(char));
	p->keycode		= (KeyCode *) realloc(p->keycode, p->cur_size * sizeof(KeyCode));
	p->keycode_modifiers	= (int *) realloc(p->keycode_modifiers, p->cur_size * sizeof(int));
}

static void xstring_set_key_code(struct _xstring *p, int lang)
{
	int keycode_mod		= get_keycode_mod(xconfig->get_lang_group(xconfig, lang));
	int languages_mask	= get_languages_mask();

	// Set new language bit
	for (int i = 0; i < p->cur_pos; i++)
	{
		p->keycode_modifiers[i] = p->keycode_modifiers[i] & (~languages_mask);
		p->keycode_modifiers[i] = p->keycode_modifiers[i] | keycode_mod;
	}
}

static void xstring_save_log(struct _xstring *p, char *file_name, Window window)
{
	if (!xconfig->save_keyboard_log || p->cur_pos == 0 || file_name == NULL)
		return;

	char *file_path_name = get_home_file_path_name(NULL, file_name);
	FILE *stream = fopen(file_path_name, "a");
	free(file_path_name);
	if (stream == NULL)
		return;
	
	if (window != last_log_window)
	{
		last_log_window = window;
		char *app_name = get_wm_class_name(window);
		fprintf(stream, "[%s]\n", app_name);
		free(app_name);
	}
	
	time_t curtime = time(NULL);
	struct tm *loctime = localtime(&curtime);
	if (loctime == NULL)
		return;

	char *buffer = malloc(256 * sizeof(char));
	strftime(buffer, 256, "%c", loctime);
	fprintf(stream, "  (%s): ", buffer);
	free(buffer);
	
	for (int i = 0; i < p->cur_pos; i++)
	{
		if (p->keycode[i] == 36)			// Return
		{
			fprintf(stream, "\n");
			continue;
		}
		if (p->keycode[i] == 23)			// Tab
		{
			fprintf(stream, "\t");
			continue;
		}

		char *symbol = keycode_to_symbol(p->keycode[i], -1, p->keycode_modifiers[i]);
		if (symbol == NULL)
		{
			fprintf(stream, "<?>");
			continue;
		}

		fprintf(stream, "%s", symbol);
		free(symbol);
	}

	fprintf(stream, "\n");
	fclose(stream);
}

static void xstring_clear(struct _xstring *p)
{
	for (int i = 0; i < p->cur_pos; i++)
	{
		p->keycode[i] = 0;
		p->keycode_modifiers[i] = 0;
	}

	p->cur_pos = 0;
	p->content[0] = NULLSYM;
}

static int xstring_is_space_last(struct _xstring *p)
{
	if (p->cur_pos <= 0)
		return FALSE;

	if (isspace(p->content[p->cur_pos - 1]))
		return TRUE;

	return FALSE;
}

static void xstring_set_content(struct _xstring *p, const char *new_content)
{
	p->cur_pos = strlen(new_content);
	if (p->cur_pos >= p->cur_size)
		set_new_size(p, p->cur_pos + 1);

	if (p->content == NULL || p->keycode == NULL || p->keycode_modifiers == NULL)
		return;
	
	p->content[p->cur_pos] = NULLSYM;
	if (!p->cur_pos)
		return;

	memcpy(p->content, new_content, p->cur_pos);

	for (int i = 0; i < p->cur_pos; i++)
		main_window->xkeymap->char_to_keycode(main_window->xkeymap, p->content[i], &p->keycode[i], &p->keycode_modifiers[i]);
}

static void xstring_change_case(struct _xstring *p)
{
	for (int i = 0; i < p->cur_pos; i++)
	{
		if (p->keycode_modifiers[i] & ShiftMask)
			p->keycode_modifiers[i] = (p->keycode_modifiers[i] & ~ShiftMask);
		else
			p->keycode_modifiers[i] = (p->keycode_modifiers[i] | ShiftMask);
	}
}

static void xstring_add_symbol(struct _xstring *p, char sym, KeyCode keycode, int modifier)
{
	if (p->cur_pos == p->cur_size - 1)
		set_new_size(p, p->cur_size * 2);

	if (p->content == NULL || p->keycode == NULL || p->keycode_modifiers == NULL)
		return;
	
	p->content[p->cur_pos] = sym;
	p->keycode[p->cur_pos] = keycode;
	p->keycode_modifiers[p->cur_pos] = modifier;

	p->cur_pos++;
	p->content[p->cur_pos] = NULLSYM;
}

static void xstring_del_symbol(struct _xstring *p)
{
	if (p->cur_pos == 0)
		return;

	p->cur_pos--;
	p->content[p->cur_pos] = NULLSYM;
}

static char *xstring_get_utf_string(struct _xstring *p)
{
	char *symbol		= (char *) malloc((256 + 1) * sizeof(char));
	char *utf_string	= (char *) malloc((p->cur_pos + 1) * sizeof(char));

	utf_string[0] = NULLSYM;

	int free_left		= p->cur_pos;
	int utf_string_size	= free_left;

	XEvent event = create_basic_event();
	for (int i = 0; i < p->cur_pos; i++)
	{
		event.xkey.keycode	= p->keycode[i];
		event.xkey.state	= p->keycode_modifiers[i];
		
		int nbytes = XLookupString((XKeyEvent *) &event, symbol, 256, NULL, NULL);
		if (nbytes <= 0)
			continue;
		
		symbol[nbytes] = NULLSYM;
		
		free_left -= nbytes;
		if (free_left == 0)
		{
			free_left	+= p->cur_pos;
			utf_string_size	+= free_left;
			utf_string = (char *) realloc(utf_string, utf_string_size * sizeof(char));
		}

		strcat(utf_string, symbol);
	}

	free(symbol);
	return utf_string;
}

static void xstring_save_and_clear(struct _xstring *p, Window window)
{
	p->save_log(p, LOG_NAME, window);
	p->clear(p);
}

static void xstring_uninit(struct _xstring *p)
{
	free(p->keycode_modifiers);
	free(p->keycode);
	free(p->content);
	free(p);

	log_message(DEBUG, "String is freed");
}

struct _xstring* xstring_init(void)
{
	struct _xstring *p = (struct _xstring *) malloc(sizeof(struct _xstring));
	bzero(p, sizeof(struct _xstring));

	p->cur_size		= INIT_STRING_LENGTH;

	p->content		= (char *) malloc(p->cur_size * sizeof(char));
	p->keycode		= (KeyCode *) malloc(p->cur_size * sizeof(KeyCode));
	p->keycode_modifiers	= (int *) malloc(p->cur_size * sizeof(int));

	bzero(p->content, p->cur_size * sizeof(char));
	bzero(p->keycode, p->cur_size * sizeof(KeyCode));
	bzero(p->keycode_modifiers, p->cur_size * sizeof(int));

	// Functions mapping
	p->clear		= xstring_clear;
	p->save_log		= xstring_save_log;
	p->save_and_clear	= xstring_save_and_clear;
	p->is_space_last	= xstring_is_space_last;
	p->set_key_code		= xstring_set_key_code;
	p->set_content		= xstring_set_content;
	p->change_case		= xstring_change_case;
	p->add_symbol		= xstring_add_symbol;
	p->del_symbol		= xstring_del_symbol;
	p->get_utf_string	= xstring_get_utf_string;
	p->uninit		= xstring_uninit;

	return p;
}
