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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef WITH_PCRE
#  include <pcre.h>
#endif

#include <string.h>

#include "types.h"
#include "log.h"

#ifdef WITH_PCRE

int check_regexp_match(const char *str, const char *pattern)
{
	int options = 0;
	const char *error;
	int erroffset;

	const unsigned char *tables = pcre_maketables();

	pcre *re = pcre_compile(pattern, options, &error, &erroffset, tables);
	if (!re)
	{
		log_message(WARNING, "Can't compile pcre pattern '%s'", pattern);
		return FALSE;
	}

	int str_len = strlen(str);

	int count = pcre_exec(re, NULL, str, str_len, 0, 0, NULL, 0);

	pcre_free(re);

	if (count <= 0)
		return FALSE;

	return TRUE;
}

#else

int check_regexp_match(const char *str, const char *pattern)
{
	if (str || pattern){}
	return FALSE;
}

#endif
