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
#include <stdio.h>

#include "log.h"
#include "regexp.h"

#include "list_char.h"


static void rem_by_id(struct _list_char *list, int id)
{
	free(list->data[id].string);

	if (id != list->data_count - 1)
		memmove(list->data + id, list->data + id + 1, (list->data_count - id - 1) * sizeof(struct _list_char_data));

	list->data_count--;
	if (list->data_count != 0)
	{
		list->data = (struct _list_char_data *) realloc(list->data, list->data_count * sizeof(struct _list_char_data));
		return;
	}

	free(list->data);
	list->data = NULL;
}

static int get_add_id(struct _list_char *list, const char *string)
{
	int first = 0;
	int last = list->data_count - 1;

	while (first <= last)
	{
		int cur = (first + last) / 2;

		struct _list_char_data *data = &list->data[cur];

		int cmp_result = strcmp(data->string, string);
		if (cmp_result == 0)
			return cur;

		if (cmp_result < 0)
			first = cur + 1;
		else
			last = cur - 1;

		if (first > last)
			return (first + last + 1) / 2;
	}
	return 0;
}

static int find_id(struct _list_char *list, const char *string, int mode)
{
	if (mode == BY_PLAIN)
	{
		int first = 0;
		int last = list->data_count - 1;
		list->sort(list);

		while (first <= last)
		{
			int cur = (first + last) / 2;
			struct _list_char_data *data = &list->data[cur];
			//printf("%d %s %s\n", cur, data->string, string);
			int cmp_result = strcmp(data->string, string);
			if (cmp_result == 0)
				return cur;

			if (cmp_result < 0)
				first = cur + 1;
			else
				last = cur - 1;
		}
		return -1;
	}

	if (mode == BY_REGEXP)
	{
		for (int i = 0; i < list->data_count; i++)
		{
			struct _list_char_data *data = &list->data[i];

			if (check_regexp_match(string, data->string))
				return i;
		}
	}

	return -1;
}

struct _list_char_data* list_char_add_last(struct _list_char *list, const char *string)
{
	list->data_count++;
	list->data = (struct _list_char_data *) realloc(list->data, list->data_count * sizeof(struct _list_char_data));

	if (list->data == NULL)
	{
		list->data_count = 0;
		return NULL;
	}

	struct _list_char_data *data = &list->data[list->data_count - 1];

	data->string = strdup(string);

	return data;
}

struct _list_char_data* list_char_add(struct _list_char *list, const char *string)
{
	int id = get_add_id(list, string);

	list->data = (struct _list_char_data *) realloc(list->data, (list->data_count + 1) * sizeof(struct _list_char_data));
	if (list->data == NULL)
		return NULL;

	if (id != list->data_count)
		memmove(list->data + id + 1, list->data + id, (list->data_count - id) * sizeof(struct _list_char_data));

	list->data_count++;

	struct _list_char_data *data = &list->data[id];

	data->string = strdup(string);

	return data;
}

void list_char_rem(struct _list_char *list, const char *string)
{
	int id = find_id(list, string, BY_PLAIN);
	if (id == -1)
		return;

	rem_by_id(list, id);
}

struct _list_char_data* list_char_find(struct _list_char *list, const char *string, int mode)
{
	int id = find_id(list, string, mode);
	if (id == -1)
		return NULL;

	return &list->data[id];
}

int list_char_exist(struct _list_char *list, const char *string, int mode)
{
	struct _list_char_data *data = list->find(list, string, mode);
	return (data != NULL);
}

struct _list_char* list_char_clone(struct _list_char *list)
{
	struct _list_char *list_copy = list_char_init();

	for (int i = 0; i < list->data_count; i++)
		list_copy->add(list_copy, list->data[i].string);

	return list_copy;
}

void list_char_sort(struct _list_char *list)
{
	if (list->data_count <= 1)
		return;

	// Bubble sort??? Change to quicksort!
	for (int i = 1; i < list->data_count; i++)
	{
		struct _list_char_data *temp = &list->data[i];

		struct _list_char_data *data1, data2;
		for (int j = i - 1; j >= 0; j--)
		{
			data1 = &list->data[j];
			data2 = &list->data[j + 1];

			if (strcmp(data1->string, temp->string) < 0)
				break;

			*data2 = *data1;
		}

		if (data2 == temp)
			continue;

		*data2 = *temp;
	}
}

void list_char_load(struct _list_char *list, char *content)
{
	while (1)
	{
		char *line = strsep(&content, "\n");
		if (line == NULL)
			break;

		if (line[0] == '\0')
			continue;

		list->add_last(list, line);
	}
}

void list_char_save(struct _list_char *list, FILE *stream)
{
	for (int i = 0; i < list->data_count; i++)
	{
		fputs(list->data[i].string, stream);
		fputs("\n", stream);
	}
}

void list_char_uninit(struct _list_char *list)
{
	for (int i = 0; i < list->data_count; i++)
		free(list->data[i].string);

	free(list->data);
	free(list);
}

struct _list_char* list_char_init(void)
{
	struct _list_char *list = (struct _list_char *) malloc(sizeof(struct _list_char));
	bzero(list, sizeof(struct _list_char));

	list->uninit		= list_char_uninit;
	list->add		= list_char_add;
	list->add_last		= list_char_add_last;
	list->rem		= list_char_rem;
	list->find		= list_char_find;
	list->load		= list_char_load;
	list->save		= list_char_save;
	list->clone		= list_char_clone;
	list->sort		= list_char_sort;
	list->exist		= list_char_exist;
	return list;
}
