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
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "xnconfig.h"

#include "osd.h"
#include "sound.h"
#include "popup.h"

#include "debug.h"

extern struct _xneur_config *xconfig;

void show_notify(int notify, ...)
{
	play_file(notify);
	
	va_list ap;
	va_start(ap, notify);

	if ((xconfig->osds[notify].file != NULL) && (strlen(xconfig->osds[notify].file) != 0))
	{
		char *buffer = (char *) malloc(1024);
		vsprintf(buffer, xconfig->osds[notify].file, ap);

		osd_show(buffer);
	}
	
	if ((xconfig->popups[notify].file != NULL) && (strlen(xconfig->popups[notify].file) != 0))
	{
		char *buffer = (char *) malloc(1024);
		vsprintf(buffer, xconfig->popups[notify].file, ap);

		popup_show(buffer);
	}
	
	va_end(ap);
}
