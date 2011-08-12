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

#ifdef HAVE_GCONF
#   include <gconf/gconf-client.h>
#endif
 
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "support.h"

#include "trayicon.h"
#include "misc.h"

#include "configuration.h"


int arg_delay = -1;
const char* arg_keyboard_properties = NULL;
const char* arg_show_in_the_tray = NULL;
const char* arg_rendering_engine = NULL;


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
	
	int value = DEFAULT_DELAY;

#ifdef HAVE_GCONF
	GConfClient* gconfClient = gconf_client_get_default();
	g_assert(GCONF_IS_CLIENT(gconfClient));

	GConfValue* gcValue = NULL;

	// Get keyboard properties command
	/*
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "keyboard_properties", NULL);

	if(gcValue == NULL) 
	{
		if(!gconf_client_set_string(gconfClient, PACKAGE_GCONF_DIR "keyboard_properties", DEFAULT_KEYBOARD_PROPERTIES, NULL)) 
		    g_warning("Failed to set %s (%s)\n", PACKAGE_GCONF_DIR "keyboard_properties", DEFAULT_KEYBOARD_PROPERTIES);
	}
	else
	{
		gconf_value_free(gcValue);
	}
	*/

	// Get what to show in the tray
	/*gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "show_in_the_tray", NULL);

	if(gcValue == NULL) 
	{
		if(!gconf_client_set_string(gconfClient, PACKAGE_GCONF_DIR "show_in_the_tray", "Flag", NULL)) 
		    g_warning("Failed to set %s (%s)\n", PACKAGE_GCONF_DIR "show_in_the_tray", "Flag");
	}
	else
	{
		gconf_value_free(gcValue);
	}

	// Define rendering engine
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "rendering_engine", NULL);

	if(gcValue == NULL) 
	{
		// May be:
		// 1. Built-in
		// 2. StatusIcon
		// 3. AppIndicator
		if(!gconf_client_set_string(gconfClient, PACKAGE_GCONF_DIR "rendering_engine", "Built-in", NULL)) 
		    g_warning("Failed to set %s (%s)\n", PACKAGE_GCONF_DIR "rendering_engine", "Built-in");
	}
	else
	{
		gconf_value_free(gcValue);
	}*/
	
	// Get delay from gconf
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "delay", NULL);

	if(gcValue != NULL) 
	{
		if(gcValue->type == GCONF_VALUE_INT) 
			value = gconf_value_get_int(gcValue);

		gconf_value_free(gcValue);
	}
	
	g_object_unref(gconfClient);
#endif
	static struct option longopts[] =
	{
			{ "help",	no_argument,	NULL,	'h' },
			{ "configure",	no_argument,	NULL,	'c' },
			{ "delay",	required_argument,	NULL,	'D' },
			{ "keyboard-properties",	required_argument,	NULL,	1000 },
			{ "rendering-engine",	required_argument,	NULL,	'E' },
			{ "show",	required_argument,	NULL,	'S' },
			{ NULL,			0,		NULL,	0 }
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "hcS:D:E:", longopts, NULL)) != -1)
	{
		switch (opt)
		{
			case 'D':
			{
				arg_delay = atoi(optarg);
				value = arg_delay;
				break;
			}
			case 'S':
			{
				arg_show_in_the_tray = optarg;
				break;
			}
			case 1000:
			{
				arg_keyboard_properties = optarg;
				break;
			}
			case 'E':
			{
				arg_rendering_engine = optarg;
				break;
			}
			case 'c':
			{
				printf("\nThis option under construction. Sorry.\n");
				return EXIT_SUCCESS;
				break;
			}
			case '?':
			case 'h':
			{
				printf(_("\nGTK2 frontend for XNeur (version %s) \n"), VERSION);
				printf(_("usage: gxneur [options]\n"));
				printf(_("  where options are:\n"));
				printf("\n");
				printf(_("  -D, --delay=<seconds>                Seconds to wait before starting xneur\n"));
				printf(_("  -E, --rendering-engine=<engine>      Rendering engine to use (Built-in, StatusIcon, AppIndicator. Default is %s.)\n"), DEFAULT_RENDERING_ENGINE);
				printf(_("  -S, --show=<mode>                    Icon display mode (Icon, Flag, Text. Default is %s.)\n"), DEFAULT_SHOW_IN_THE_TRAY);
				printf(_("      --keyboard-properties=<command>  Command to run on \"Keyboard Properties\" menu item. Default is %s.\n"), KB_PROP_COMMAND);
				printf(_("  -c, --configure                      Configure xneur and gxneur\n"));
				printf(_("  -h, --help                           Display this help and exit\n"));
				exit(EXIT_SUCCESS);
				break;
			}
		}
	}
	sleep (value);

	xneur_start();

	create_tray_icon();	

	gtk_main();
	
	return EXIT_SUCCESS;
}
