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

#include <stdlib.h>

#include "log.h"
#include "regexp.h"

#include "typographics.h"

char *check_typographics(char *text)
{
	log_message(ERROR, "'%s'", text);

	if (check_regexp_match(text, SPACE_BEFORE_PUNCTUATION))
		log_message (ERROR, "Find pattern SPACE_BEFORE_PUNCTUATION in '%s'", text);

	if (check_regexp_match(text, NO_SPACE_AFTER_PUNCTUATION))
		log_message (ERROR, "Find pattern SPACE_BEFORE_PUNCTUATION in '%s'", text);

	return NULL;
}