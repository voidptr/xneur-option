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


#include <X11/XKBlib.h>

#include <string.h>
#include <stdlib.h>

#include "xneur.h"

#include "xnconfig_files.h"

#include "list_char.h"
#include "types.h"

#include "switchlang.h"

#include "detection.h"

struct _xneur_handle *xneur_handle_create (void)
{
	struct _xneur_handle *handle = (struct _xneur_handle *) malloc(sizeof(struct _xneur_handle));;

	XkbDescRec *kbd_desc_ptr = XkbAllocKeyboard();
	if (kbd_desc_ptr == NULL)
		return NULL;

	Display *display = XOpenDisplay(NULL);
	XkbGetNames(display, XkbAllNamesMask, kbd_desc_ptr);

	if (kbd_desc_ptr->names == NULL)
	{
		XCloseDisplay(display);
		return NULL;
	}

	int groups_count = 0;
	for (; groups_count < XkbNumKbdGroups; groups_count++)
	{
		if (kbd_desc_ptr->names->groups[groups_count] == None)
			break;
	}
	
	if (groups_count == 0)
	{
		XCloseDisplay(display);
		return NULL;
	}

	Atom symbols_atom = kbd_desc_ptr->names->symbols;
	char *symbols	= XGetAtomName(display, symbols_atom);
	char *tmp_symbols = strdup(symbols);
	strsep(&tmp_symbols, "+");

	handle->languages = (struct _xneur_language *) malloc(sizeof(struct _xneur_language));
	handle->total_languages = 0;	
	for (int group = 0; group < groups_count; group++)
	{
		Atom group_atom = kbd_desc_ptr->names->groups[group];
			
		if (group_atom == None)
			continue;

		char *group_name	= XGetAtomName(display, group_atom);
		
		char *short_name = strsep(&tmp_symbols, "+");
		short_name[2] = NULLSYM;

		handle->languages = (struct _xneur_language *) realloc(handle->languages, (handle->total_languages + 1) * sizeof(struct _xneur_language));
		bzero(&(handle->languages[handle->total_languages]), sizeof(struct _xneur_language));

		handle->languages[handle->total_languages].name	= strdup(group_name);
		handle->languages[handle->total_languages].dir	= strdup(short_name);
		handle->languages[handle->total_languages].group	= group;
		handle->languages[handle->total_languages].excluded	= FALSE;
		handle->total_languages++;
	}

	free(symbols);
	XCloseDisplay(display);

	if (handle->total_languages == 0)
		return NULL;

	for (int lang = 0; lang < handle->total_languages; lang++)
	{
		int path_len = strlen(LANGUAGEDIR) + strlen(handle->languages[lang].dir) + 2;
		char *lang_dir = (char *) malloc(path_len * sizeof(char));
		snprintf(lang_dir, path_len, "%s/%s", LANGUAGEDIR, handle->languages[lang].dir);

		handle->languages[lang].dict = load_list(lang_dir, DICT_NAME, TRUE);		
		if (handle->languages[lang].dict == NULL)
			handle->languages[lang].dict->data_count = 0;

		handle->languages[lang].proto = load_list(lang_dir, PROTO_NAME, TRUE);
		if (handle->languages[lang].proto == NULL)
			handle->languages[lang].proto->data_count = 0;

		handle->languages[lang].big_proto = load_list(lang_dir, BIG_PROTO_NAME, TRUE);
		if (handle->languages[lang].big_proto == NULL)
			handle->languages[lang].big_proto->data_count = 0;

		handle->languages[lang].regexp = load_list(lang_dir, REGEXP_NAME, TRUE);
		if (handle->languages[lang].regexp == NULL)
			handle->languages[lang].regexp->data_count = 0;

		handle->languages[lang].pattern = load_list(lang_dir, PATTERN_NAME, TRUE);
		if (handle->languages[lang].pattern == NULL)
			handle->languages[lang].pattern->data_count = 0;
		
		handle->languages[lang].temp_dict = handle->languages[lang].dict->clone(handle->languages[lang].dict);

		if (lang_dir != NULL)
			free(lang_dir);

		if ((handle->languages[lang].dict->data_count == 0 &&
		    handle->languages[lang].proto->data_count == 0 &&
		    handle->languages[lang].big_proto->data_count == 0 &&
		    handle->languages[lang].regexp->data_count == 0))
		{
			handle->languages[lang].excluded	= TRUE;
		}
	}
	return handle;
}

void xneur_handle_destroy (struct _xneur_handle *handle)
{
	if (handle == NULL) 
		return;
	
	for (int lang = 0; lang < handle->total_languages; lang++)
	{
		if (handle->languages[lang].temp_dict != NULL)
			handle->languages[lang].temp_dict->uninit(handle->languages[lang].temp_dict);

		if (handle->languages[lang].dict != NULL)
			handle->languages[lang].dict->uninit(handle->languages[lang].dict);

		if (handle->languages[lang].proto != NULL)
			handle->languages[lang].proto->uninit(handle->languages[lang].proto);

		if (handle->languages[lang].big_proto != NULL)
			handle->languages[lang].big_proto->uninit(handle->languages[lang].big_proto);

		if (handle->languages[lang].regexp != NULL)
			handle->languages[lang].regexp->uninit(handle->languages[lang].regexp);

		if (handle->languages[lang].pattern != NULL)
			handle->languages[lang].pattern->uninit(handle->languages[lang].pattern);
		
		free(handle->languages[lang].name);
		free(handle->languages[lang].dir);
	}
	handle->total_languages = 0;
	free(handle);
}

int xneur_get_layout (struct _xneur_handle *handle, char *word)
{
	if (!word || handle == NULL)
		return -1;
	
	struct _buffer *buffer = buffer_init(handle);
	buffer->set_content(buffer, word);
	int cur_lang = get_active_keyboard_group();
	int new_lang = check_lang(handle, buffer, cur_lang);
	buffer->uninit(buffer);
	if (new_lang == NO_LANGUAGE)
		new_lang = cur_lang;

	return new_lang;
}

char *xneur_get_word (struct _xneur_handle *handle, char *word)
{
	if (!word || handle == NULL)
		return NULL;
	
	struct _buffer *buffer = buffer_init(handle);
	buffer->set_content(buffer, word);
	int cur_lang = get_active_keyboard_group();
	int new_lang = check_lang(handle, buffer, cur_lang);
	if (new_lang == NO_LANGUAGE)
		return strdup(word);

	buffer->set_lang_mask(buffer, new_lang);
	char *new_word = strdup(buffer->get_utf_string(buffer));
	buffer->uninit(buffer);

	return new_word;
}

