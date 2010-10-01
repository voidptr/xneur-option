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

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
 
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>

#include "support.h"

#include "trayicon.h"
#include "misc.h"

#define GCONF_DIR "/apps/" PACKAGE "/"

int main(int argc, char *argv[])
{
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);
#endif

	gtk_set_locale();
	gtk_init(&argc, &argv);

	add_pixmap_directory(PACKAGE_PIXMAPS_DIR);

	
	GConfClient* gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));
 
    if(!gconf_client_set_int(gconfClient, GCONF_DIR "mykey", 13, NULL)) {
      g_warning(" failed to set %smykey (%d)\n", GCONF_DIR, 13);
    }
 
    /* release GConf client */
    g_object_unref(gconfClient);
	
	xneur_start();

	create_tray_icon(NULL, TRUE);

	gtk_main();

	return EXIT_SUCCESS;
}
