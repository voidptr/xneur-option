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
#   include "config.h"
#endif

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
 
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <unistd.h>

#include "support.h"

#include "trayicon.h"
#include "misc.h"

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

	GConfClient* gconfClient = gconf_client_get_default();
	g_assert(GCONF_IS_CLIENT(gconfClient));

	GConfValue* gcValue = NULL;

	// Get keyboard properties command
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "keyboard_properties", NULL);

	if(gcValue == NULL) 
	{
		if(!gconf_client_set_string(gconfClient, PACKAGE_GCONF_DIR "keyboard_properties", KB_PROP_COMMAND, NULL)) 
		    g_warning("Failed to set %s (%s)\n", PACKAGE_GCONF_DIR "keyboard_properties", KB_PROP_COMMAND);
	}
	else
	{
		gconf_value_free(gcValue);
	}

	// Get what to show in the tray
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "show_in_the_tray", NULL);

	if(gcValue == NULL) 
	{
		if(!gconf_client_set_string(gconfClient, PACKAGE_GCONF_DIR "show_in_the_tray", "Flag", NULL)) 
		    g_warning("Failed to set %s (%s)\n", PACKAGE_GCONF_DIR "show_in_the_tray", "Flag");
	}
	else
	{
		gconf_value_free(gcValue);
	}
	
	// Get show on panel status
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "show_icon_on_panel_indicators", NULL);

	if(gcValue == NULL) 
	{
		if(!gconf_client_set_bool(gconfClient, PACKAGE_GCONF_DIR "show_icon_on_panel_indicators", TRUE, NULL)) 
		    g_warning("Failed to set %s\n", PACKAGE_GCONF_DIR "show_icon_on_panel_indicators");
	}
	else
	{
		gconf_value_free(gcValue);
	}
	
	// Get delay from gconf
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "delay", NULL);

	int value = 0;
	if(gcValue != NULL) 
	{
		if(gcValue->type == GCONF_VALUE_INT) 
			value = gconf_value_get_int(gcValue);

		gconf_value_free(gcValue);
	}
	
	g_object_unref(gconfClient);
	
	sleep (value);

	xneur_start();

	create_tray_icon();	

	gtk_main();
	
	return EXIT_SUCCESS;
}
