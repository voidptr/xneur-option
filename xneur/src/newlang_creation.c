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

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "xnconfig_files.h"

#include "xwindow.h"
#include "xkeymap.h"

#include "types.h"
#include "list_char.h"
#include "text.h"

#define NEW_LANG_DIR	"new"
#define NEW_LANG_TEXT	"new.text"

extern struct _xwindow *main_window;

static char* get_file_content(const char *file_name)
{
	struct stat sb;

	if (stat(file_name, &sb) != 0 || sb.st_size < 0)
		return NULL;

	FILE *stream = fopen(file_name, "rb");
	if (stream == NULL)
		return NULL;

	unsigned int file_len = sb.st_size;

	char *content = (char *) malloc((file_len + 2) * sizeof(char)); // + 1 '\0'
	if (fread(content, 1, file_len, stream) != file_len)
	{
		free(content);
		fclose(stream);
		return NULL;
	}

	content[file_len] = '\0';
	fclose(stream);

	return content;
}

void generate_protos(void)
{
	printf("THIS OPTION FOR DEVELOPERS ONLY!\n");
	printf("\nPlease, define new language group and press Enter to continue...\n");
	printf("(see above keyboard layouts groups presented in system): \n");

	int new_lang_group;
	if (!scanf("%d", &new_lang_group))
		exit(EXIT_SUCCESS);

	if (new_lang_group < 0 || new_lang_group > 3)
	{
		printf("New language group is bad! Aborting!\n");
		exit(EXIT_SUCCESS);
	}

	printf("\nSpecified new language group: %d\n", new_lang_group);

	char *text = get_file_content(get_file_path_name(NEW_LANG_DIR, NEW_LANG_TEXT));
	if (text == NULL)
	{
		printf("New language text file not find! Aborting!\n");
		exit(EXIT_FAILURE);
	}

	struct _list_char *proto  = list_char_init();
	struct _list_char *proto3 = list_char_init();

	char *syll = (char *) malloc((256 + 1) * sizeof(char));

	/*for (int i = 1; i < 255; i++)
	{
		char *symb = keycode_to_symbol(i, new_lang_group, 0);
		if (symb != NULL)
		{
			if (isblank(symb[0]) || iscntrl(symb[0]) || isspace(symb[0]) || ispunct(symb[0]) || isdigit(symb[0]))
				continue;

			printf("%s\n", symb);
		}
		free(symb);
		symb = keycode_to_symbol(i, new_lang_group, 1<<7);
		if (symb != NULL)
		{
			if (isblank(symb[0]) || iscntrl(symb[0]) || isspace(symb[0]) || ispunct(symb[0]) || isdigit(symb[0]))
				continue;

			printf("%s\n", symb);
		}
	}
	return;
	*/
	for (int i = 0; i < 100; i++)
	{
		printf("%d\n", i);

		char *sym_i = keycode_to_symbol(i, new_lang_group, 0);
		if (sym_i == NULL)
			continue;
		if (isblank(sym_i[0]) || iscntrl(sym_i[0]) || isspace(sym_i[0]) || ispunct(sym_i[0]) || isdigit(sym_i[0]))
			continue;
		for (int j = 0; j < 100; j++)
		{
			char *sym_j = keycode_to_symbol(j, new_lang_group, 0);
			if (sym_j == NULL)
				continue;
			if (isblank(sym_j[0]) || iscntrl(sym_j[0]) || isspace(sym_j[0]) || ispunct(sym_j[0]) || isdigit(sym_j[0]))
				continue;

			strcpy(syll, sym_i);
			strcat(syll, sym_j);

			if (proto->find(proto, syll, BY_PLAIN))
				continue;

			if (strstr(text, syll) == NULL)
			{
				proto->add(proto, syll);
				continue;
			}

			for (int k = 0; k < 100; k++)
			{
				char *sym_k = keycode_to_symbol(k, new_lang_group, 0);
				if (sym_k == NULL)
					continue;
				if (isblank(sym_k[0]) || iscntrl(sym_k[0]) || isspace(sym_k[0]) || ispunct(sym_k[0]) || isdigit(sym_k[0]))
					continue;

				strcpy(syll, sym_i);
				strcat(syll, sym_j);
				strcat(syll, sym_k);

				if (proto3->find(proto3, syll, BY_PLAIN))
					continue;

				if (strstr(text, syll) != NULL)
					continue;

				proto3->add(proto3, syll);
			}
		}
	}

	for (int i = 0; i < 100; i++)
	{
		char *sym_i = keycode_to_symbol(i, new_lang_group, 1 << 7);
		if (sym_i == NULL)
			continue;
		if (isblank(sym_i[0]) || iscntrl(sym_i[0]) || isspace(sym_i[0]) || ispunct(sym_i[0]) || isdigit(sym_i[0]))
			continue;
		for (int j = 0; j < 100; j++)
		{
			char *sym_j = keycode_to_symbol(j, new_lang_group, 1 << 7);
			if (sym_j == NULL)
				continue;
			if (isblank(sym_j[0]) || iscntrl(sym_j[0]) || isspace(sym_j[0]) || ispunct(sym_j[0]) || isdigit(sym_j[0]))
				continue;

			strcpy(syll, sym_i);
			strcat(syll, sym_j);

			if (proto->find(proto, syll, BY_PLAIN))
				continue;

			if (strstr(text, syll) == NULL)
			{
				proto->add(proto, syll);
				continue;
			}

			for (int k = 0; k < 100; k++)
			{
				char *sym_k = keycode_to_symbol(k, new_lang_group, 1 << 7);
				if (sym_k == NULL)
					continue;
				if (isblank(sym_k[0]) || iscntrl(sym_k[0]) || isspace(sym_k[0]) || ispunct(sym_k[0]) || isdigit(sym_k[0]))
					continue;

				strcpy(syll, sym_i);
				strcat(syll, sym_j);
				strcat(syll, sym_k);

				if (proto3->find(proto3, syll, BY_PLAIN))
					continue;

				if (strstr(text, syll) != NULL)
					continue;

				proto3->add(proto3, syll);
			}
		}
	}

	free(text);

	char *proto_file_path = get_file_path_name(NEW_LANG_DIR, "proto");
	FILE *stream = fopen(proto_file_path, "w");
	proto->save(proto, stream);
	printf("Short proto writed (%d) to %s\n", proto->data_count, proto_file_path);
	fclose(stream);
	free(proto_file_path);

	char *proto3_file_path = get_file_path_name(NEW_LANG_DIR, "proto3");
	stream = fopen(proto3_file_path, "w");
	proto3->save(proto3, stream);
	printf("Big proto writed (%d) to %s\n", proto3->data_count, proto3_file_path);
	fclose(stream);
	free(proto3_file_path);

	proto->uninit(proto);
	proto3->uninit(proto3);

	free(syll);

	printf("End of generation!\n");

	//exit(EXIT_SUCCESS);
/*
	printf("THIS OPTION FOR DEVELOPERS ONLY!\n");
	printf("\n1) Please, define new language group and press Enter to continue...\n");
	printf("(see above keyboard layouts groups presented in system): \n");

	int new_lang_group;
	scanf("%d", &new_lang_group);

	if (new_lang_group < 0 || new_lang_group > 3)
	{
		printf("New language group is bad! Aborting!\n");
		exit(EXIT_SUCCESS);
	}

	printf("\nSpecified new language group: %d\n", new_lang_group);

	char *text = get_file_content_path(NEW_LANG_DIR, NEW_LANG_TEXT);
	if (text == NULL)
	{
		printf("New language text file not find! Aborting!\n");
		exit(EXIT_FAILURE);
	}

	char *low_text = main_window->xkeymap->lower_by_keymaps(main_window->xkeymap, new_lang_group, text);
	free(text);

	printf("Text in low symbols :\n %s\n", low_text);

	struct _list_char *proto  = list_char_init();
	struct _list_char *proto3 = list_char_init();

	char *syll = (char *) malloc((3 + 1) * sizeof(char));

	printf("Check character combination...\n");

	// Check character combination
	for (int i = main_window->xkeymap->min_keycode + 1; i <= main_window->xkeymap->max_keycode; i++)
	{
		char *sym_i = keycode_to_symbol(i, main_window->xkeymap->latin_group, 0);
		if (sym_i == NULLSYM || iscntrl(sym_i[0]) || isspace(sym_i[0]))
			continue;

		for (int j = main_window->xkeymap->min_keycode + 1; j <= main_window->xkeymap->max_keycode; j++)
		{
			char *sym_j = keycode_to_symbol(j, main_window->xkeymap->latin_group, 0);
			if (sym_j == NULLSYM || iscntrl(sym_j[0]) || isspace(sym_j[0]))
				continue;

			strcpy(syll, sym_i);
			strcat(syll, sym_j);

			main_window->xkeymap->convert_text_to_ascii(main_window->xkeymap, syll);

			if (proto->find(proto, syll, BY_PLAIN))
				continue;

			if (strstr(low_text, syll) == NULL)
			{
				proto->add(proto, syll);
				continue;
			}

			for (int k = main_window->xkeymap->min_keycode + 1; k <= main_window->xkeymap->max_keycode; k++)
			{
				char *sym_k = keycode_to_symbol(k, main_window->xkeymap->latin_group, 0);
				if (sym_k == NULL || iscntrl(sym_k[0]) || isspace(sym_k[0]))
					continue;

				strcpy(syll, sym_i);
				strcat(syll, sym_j);
				strcat(syll, sym_k);

				main_window->xkeymap->convert_text_to_ascii(main_window->xkeymap, syll);

				if (proto3->find(proto3, syll, BY_PLAIN))
					continue;

				if (strstr(low_text, syll) != NULL)
					continue;

				proto3->add(proto3, syll);
			}
		}
	}


*/
}
