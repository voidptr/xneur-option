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
#   include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <xneur/xnconfig.h>
#include <xneur/list_char.h>
#include <xneur/xneur.h>

struct _xneur_config *xconfig				= NULL;

void xneur_start(void)
{
	struct _xneur_handle *xnh;
	xnh = xneur_handle_create();

	int layout = xneur_get_layout(xnh, "ghbdtn");
	char *nw = xneur_get_word(xnh, "цщкл");

	printf("%d %s\n", layout, nw);
	free(nw);

	xneur_handle_destroy(xnh);
}
