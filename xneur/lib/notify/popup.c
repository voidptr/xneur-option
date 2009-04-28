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

#ifdef WITH_LIBNOTIFY

#include <libnotify/notify.h>

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include "xnconfig.h"

#include "debug.h"
#include "log.h"

#include "popup.h"

extern struct _xneur_config *xconfig;

static const char *icon = "distributor-logo";
static const char *type = NULL;
static const NotifyUrgency urgency = NOTIFY_URGENCY_NORMAL;
static const long expire_timeout = NOTIFY_EXPIRES_DEFAULT;

time_t timestamp = 0;

static void popup_show_thread(void *popup_text)
{
	if (!notify_init("xneur"))
		return;

	NotifyNotification *notify = notify_notification_new(popup_text, NULL, icon, NULL);

	notify_notification_set_category(notify, type);
	notify_notification_set_urgency(notify, urgency);
	notify_notification_set_timeout(notify, expire_timeout);

	notify_notification_show(notify, NULL);

	notify_uninit();

	free(popup_text);
}

void popup_show(char *popup_text)
{
	if (!xconfig->show_popup)
		return;

	time_t curtime = time(NULL);
	if ((curtime - timestamp) < 1)
		return;
	timestamp = curtime;
	
	pthread_attr_t popup_thread_attr;
	pthread_attr_init(&popup_thread_attr);
	pthread_attr_setdetachstate(&popup_thread_attr, PTHREAD_CREATE_DETACHED);

	log_message(DEBUG, _("Show popup message \"%s\""), popup_text);

	pthread_t popup_thread;
	pthread_create(&popup_thread, &popup_thread_attr, (void*) &popup_show_thread, popup_text);

	pthread_attr_destroy(&popup_thread_attr);
}

#else /* WITH_LIBNOTIFY */

void popup_show(char *popup_text)
{
	free(popup_text);
}

#endif /* WITH_LIBNOTIFY */
