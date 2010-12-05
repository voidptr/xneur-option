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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <X11/extensions/XTest.h>

#include "log.h"

FILE *stream = NULL;
time_t event_time = 0;

void rec_init (char *path)
{
	if (path == NULL)
		return;

	log_message(LOG, _("Recording keyboard and mouse events to %s"), path);
	
	stream = fopen(path, "w");
	if (stream == NULL)
	{
		log_message(ERROR, _("Can't create recording file %s"), path);
		return;
	}
	
	time_t curtime = time(NULL);
	struct tm *loctime = localtime(&curtime);
	if (loctime == NULL)
	{
		fclose (stream);
		stream = NULL;
		return;
	}

	event_time = curtime;
	
	char *date = malloc(256 * sizeof(char));
	char *time = malloc(256 * sizeof(char));
	strftime(date, 256, "%x", loctime);
	strftime(time, 256, "%X", loctime);

	fprintf(stream, "# Xneur record this file on %s %s.\n", date, time);
	fprintf(stream, "# Format:\n");
	fprintf(stream, "# EventType Delay KeyCode X_Coordinate Y_Coordinate\n");
	
	free(time);
	free(date);
}

void rec (XEvent *event)
{
	if (stream == NULL)
		return;

	time_t curtime = time(NULL);
	struct tm *loctime = localtime(&curtime);
	if (loctime == NULL)
		return;

	int delay = curtime - event_time;
	switch (event->type)
	{
		case KeyPress:
		{
			fprintf(stream, "KeyPress %d %d %d %d\n", delay, event->xkey.keycode, event->xkey.x_root, event->xkey.y_root);
			break;
		}
		case KeyRelease:
		{
			fprintf(stream, "KeyRelease %d %d %d %d\n", delay, event->xkey.keycode, event->xkey.x_root, event->xkey.y_root);
			break;
		}
		case ButtonPress:
		{
			fprintf(stream, "ButtonPress %d %d %d %d\n", delay, event->xbutton.button, event->xbutton.x_root, event->xbutton.y_root);
			break;
		}
		case ButtonRelease:
		{
			fprintf(stream, "ButtonRelease %d %d %d %d\n", delay, event->xbutton.button, event->xbutton.x_root, event->xbutton.y_root);
			break;
		}
	}
	
	event_time = curtime;
}

void rec_uninit (void)
{
	if (stream != NULL)
	{
		fclose (stream);
		stream = NULL;
	}
}
