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

#ifdef WITH_XOSD

#include <xosd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "xnconfig.h"

#include "debug.h"
#include "log.h"

#include "osd.h"

extern struct _xneur_config *xconfig;

static void osd_show_thread(void *osd_text)
{
	xosd *osd = xosd_create(1);

	xosd_set_font(osd, xconfig->osd_font);
	xosd_set_colour(osd, "Red");
	xosd_set_timeout(osd, 2);
	xosd_set_shadow_offset(osd, 1);
	xosd_set_align(osd, XOSD_right);

	xosd_display(osd, 0, XOSD_string, (char *) osd_text);

	xosd_wait_until_no_display(osd);
	xosd_destroy(osd);

	free(osd_text);
}

void osd_show(char *osd_text)
{
	if (!xconfig->show_osd)
		return;

	pthread_attr_t osd_thread_attr;
	pthread_attr_init(&osd_thread_attr);
	pthread_attr_setdetachstate(&osd_thread_attr, PTHREAD_CREATE_DETACHED);

	log_message(DEBUG, _("Show OSD \"%s\""), osd_text);

	pthread_t osd_thread;
	pthread_create(&osd_thread, &osd_thread_attr, (void *) &osd_show_thread, osd_text);

	pthread_attr_destroy(&osd_thread_attr);
}

#else /* WITH_XOSD */

void osd_show(char *osd_text)
{
	free(osd_text);
}

#endif /* WITH_XOSD */