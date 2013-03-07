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
 *  Copyright (C) 2006-2012 XNeur Team
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "types.h"

#include "text.h"

static const char ch_up[] =
{
	'"', '{', '}', ':', '<', '>', '!', '@', '#', '$', '%', '^',
	'&', '*', '(', ')', '_', '+', '|', '?', '~'
};

static const char ch_down[] =
{
	'\'', '[', ']', ';', ',', '.', '1', '2', '3', '4', '5', '6',
	'7', '8', '9', '0', '-', '=', '\\', '/', '`'
};

static const int ch_up_len = sizeof(ch_up) / sizeof(ch_up[0]);

int is_upper_non_alpha_cyr(char symbol)
{
	for (int i = 0; i < ch_up_len; i++)
	{
		if (ch_up[i] == symbol)
			return TRUE;
	}
	return FALSE;
}

int get_last_word_offset(const char *string, int string_len)
{
	int len = string_len;
	while (len != 0 && (isspace(string[len - 1]) || (string[len - 1] == '-')))
		len--;

	if (len == 0)
		return string_len;

	while (len != 0 && (!isspace(string[len - 1]) && !(string[len - 1] == '-')))
		len--;

	return len;
}

char* get_last_word(char *string)
{
	int len = strlen(string);

	int offset = get_last_word_offset(string, len);
	if (offset == -1)
		return NULL;

	return string + offset;
}

int trim_word(char *word, int len)
{
	while (len != 0)
	{
		if (!isspace(word[len - 1]))
			break;

		word[--len] = NULLSYM;
	}
	return len;
}

char full_tolower(char sym)
{
	if (!isalpha(sym))
	{
		for (int i = 0; i < ch_up_len; i++)
		{
			if (ch_up[i] == sym)
				return ch_down[i];
		}
	}
	return tolower(sym);
}

void lower_word_inplace(char *word)
{
	int len = strlen(word);

	for (int i = 0; i < len; i++)
		word[i] = full_tolower(word[i]);
}

char* lower_word(const char *word, int len)
{
	char *ret = (char *) malloc(len + 1);

	for (int i = 0; i < len; i++)
		ret[i] = full_tolower(word[i]);
	ret[len] = NULLSYM;

	return ret;
}

char* str_replace(const char *source, const char *search, const char *replace)
{
	if (source == NULL)
		return NULL;

	int source_len = strlen(source);
	int search_len = strlen(search);
	int replace_len = strlen(replace);

	int max_multiplier = replace_len / search_len + 1;

	char *result = (char *) malloc((source_len * max_multiplier + 1) * sizeof(char));
	result[0] = NULLSYM;

	char *result_orig = result;
	while (TRUE)
	{
		char *found = strstr(source, search);
		if (found == NULL)
		{
			strcat(result, source);
			break;
		}

		if (found != source)
			strncat(result, source, (found - source)/sizeof(char));

		strcat(result, replace);
		source = found + search_len;
	}
	
	return result_orig;
}

char* real_sym_to_escaped_sym(const char *source)
{
	char *dummy = str_replace(source, "\\", "\\\\");
	source = strdup(dummy);
	free(dummy);

	dummy = str_replace(source, "\t", "\\t");
	free((void*)source);
	source = strdup(dummy);
	free(dummy);

	dummy = str_replace(source, "\n", "\\n");
	free((void*)source);

	return dummy;
}

char* escaped_sym_to_real_sym(const char *source)
{
	// Replace escaped-symbols
	char escape[] = {'\n', NULLSYM};
	char *dummy = str_replace(source, "\\n", escape);
	source = strdup(dummy);
	free(dummy);

	escape[0] = '\t';
	dummy = str_replace(source, "\\t", escape);
	free((void*)source);
	source = strdup(dummy);
	free(dummy);

	escape[0] = '\\';
	dummy = str_replace(source, "\\\\", escape);
	free((void*)source);

	return dummy;
}

void del_final_numeric_char(char *word)
{
	int offset = 0;
	int len = strlen(word);
	for (int i = len; i>0; i--)
	{
		switch (word[i-1])
		{
			case '=':
			case '+':
			case '-':
			case '/':
			case '|':
			case '\\':
			case '?':
			case ';':
			case ',':
			case '.':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0':
			case ' ':
			{
				offset++;
				break;
			}
			default:
			{
				i = 0;
				break;
			}
		}
	}
	if (offset == len)
		return;
	word[len - offset] = NULLSYM;
}

int levenshtein(const char *s, const char *t)
{
	int ls = strlen(s), lt = strlen(t);
	int d[ls + 1][lt + 1];
 
	for (int i = 0; i <= ls; i++)
		for (int j = 0; j <= lt; j++)
			d[i][j] = -1;
 
	int dist(int i, int j) {
		if (d[i][j] >= 0) return d[i][j];
 
		int x;
		if (i == ls)
			x = lt - j;
		else if (j == lt)
			x = ls - i;
		else if (s[i] == t[j])
			x = dist(i + 1, j + 1);
		else {
			x = dist(i + 1, j + 1);
 
			int y;
			if ((y = dist(i, j + 1)) < x) x = y;
			if ((y = dist(i + 1, j)) < x) x = y;
			x++;
		}
		return d[i][j] = x;
	}
	return dist(0, 0);
}

int damerau_levenshtein(const char *string1, const char *string2, int w, int s, int a, int d)
{
	int len1 = strlen(string1), len2 = strlen(string2);
	int *row0 = malloc(sizeof(int) * (len2 + 1));
	int *row1 = malloc(sizeof(int) * (len2 + 1));
	int *row2 = malloc(sizeof(int) * (len2 + 1));
	int i, j;

	for (j = 0; j <= len2; j++)
		row1[j] = j * a;
	for (i = 0; i < len1; i++) 
	{
		int *dummy;

		row2[0] = (i + 1) * d;
		for (j = 0; j < len2; j++) 
		{
			/* substitution */
			row2[j + 1] = row1[j] + s * (string1[i] != string2[j]);
			/* swap */
			if (i > 0 && j > 0 && string1[i - 1] == string2[j] && string1[i] == string2[j - 1] && row2[j + 1] > row0[j - 1] + w)
				row2[j + 1] = row0[j - 1] + w;
			/* deletion */
			if (row2[j + 1] > row1[j + 1] + d)
				row2[j + 1] = row1[j + 1] + d;
			/* insertion */
			if (row2[j + 1] > row2[j] + a)
				row2[j + 1] = row2[j] + a;
		}

		dummy = row0;
		row0 = row1;
		row1 = row2;
		row2 = dummy;
	}

	i = row1[len2];
	free(row0);
	free(row1);
	free(row2);

	return i;
}
