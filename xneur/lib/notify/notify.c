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

// Эту функцию надо переписать, т.к. предыдущая её реализация была небезопасна
// И, мало того, работает она с ошибками (при размере текста больше 1024 байт)
void show_notify(int notify, char *command)
{
	if (command == NULL) 
	{
		command = "\0";
	}

	play_file(notify);

	if (xconfig->osds[notify].file != NULL)
	{
		char *buffer = (char *) malloc((strlen(xconfig->osds[notify].file) + strlen(command) + 2) * sizeof(char));
		sprintf(buffer, "%s %s", xconfig->osds[notify].file, command);

		osd_show(buffer);
	}

	if (xconfig->popups[notify].file != NULL)
	{
		char *buffer = (char *) malloc(1024);
		sprintf(buffer, "%s %s", xconfig->popups[notify].file, command);

		popup_show(buffer);
	}
}
