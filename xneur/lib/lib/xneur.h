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

#ifndef _XNEUR_H_
#define _XNEUR_H_

struct _xneur_language
{
	char *dir;
	char *name;
	int  group;
	int  excluded;

	struct _list_char *temp_dict;
	struct _list_char *dict;
	struct _list_char *proto;
	struct _list_char *big_proto;
	struct _list_char *regexp;
	struct _list_char *pattern;
};

struct _xneur_handle
{
	struct _xneur_language *languages;
	int total_languages;
};

// Initialyze structure (must be installed proto, proto3, dict and regexp)
// One application - one create.
struct _xneur_handle *xneur_handle_create (void);

// Destroy structure
void xneur_handle_destroy (struct _xneur_handle *handle);

// Check WORD. Library return alleged new word and it's layout
void xneur_check (struct _xneur_handle *handle, char *word, int layout, char *new_word);

#endif /* _XNEUR_H_ */
