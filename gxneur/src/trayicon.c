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


//#include <gdk/gdk.h>
//#include <gdk/gdkx.h>
//#include <libxklavier/xklavier.h>

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "support.h"

#include "clock.h"
#include "xkb.h"
#include "misc.h"

#include <xneur/xnconfig.h>

#include "trayicon.h"

extern struct _xneur_config *xconfig;

static int xneur_old_pid = -1;
static int xneur_old_state = -1;
static int xneur_old_group = -1;

static gboolean tray_icon_release(GtkWidget *widget, GdkEventButton *event, struct _tray_icon *tray)
{	
	if (widget){};
		
	if (event->button == 1 || event->button == 2)
		return TRUE;

	gtk_menu_popdown(GTK_MENU(tray->popup_menu));

	return FALSE;
}

gboolean tray_icon_press(GtkWidget *widget, GdkEventButton *event, struct _tray_icon *tray)
{
	if (event->button == 2)
	{
		xneur_start_stop(widget, tray);
		return FALSE;
	}
	
	if (event->button == 1)
	{
		set_next_kbd_group();
		return FALSE;
	}
	
	gtk_menu_popup(GTK_MENU(tray->popup_menu), NULL, NULL, NULL, NULL, event->button, event->time);
	return FALSE;
}

static void exec_user_action(char *cmd)
{
	char *command = malloc ((strlen(cmd) + strlen(" 2> /dev/stdout") + 1) * sizeof(char));
	command[0] = '\0';
	strcat(command, cmd);
	strcat(command, " 2> /dev/stdout");

	FILE *fp = popen(command, "r");
	free(command);
	if (fp == NULL)
		return;

	char buffer[NAME_MAX];
	if (fgets(buffer, NAME_MAX, fp) == NULL)
	{
		pclose(fp);
		return;
	}

	pclose(fp);
	
	error_msg(buffer);
}

GtkWidget* create_menu_icon(struct _tray_icon *tray, gboolean runned, int state)
{
	GtkWidget *menu = gtk_menu_new();
	
	// Start/stop
	gchar *menu_text;
	gchar *menu_icon;
	if (runned == TRUE)
	{
		menu_text = _("Stop daemon");
		menu_icon = "gtk-stop";
	}
	else
	{
		menu_text = _("Start daemon");
		menu_icon = "gtk-execute";
	}

	GtkWidget *menuitem = gtk_image_menu_item_new_with_mnemonic(menu_text);
	GtkWidget *image = gtk_image_new_from_stock(menu_icon, GTK_ICON_SIZE_MENU);

	gtk_widget_set_name(image, "image");
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_start_stop), tray);

	menuitem = gtk_check_menu_item_new_with_mnemonic(_("Auto-correction"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), !state);
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_auto_manual), tray);
	if (runned == FALSE)
		gtk_widget_set_sensitive(menuitem, FALSE);
	
	// Separator
	menuitem = gtk_separator_menu_item_new();
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	gtk_widget_set_sensitive(menuitem, FALSE);

	// User Actions Submenu
	GtkWidget *action_submenu = gtk_menu_new();

	for (int action = 0; action < xconfig->actions_count; action++)
	{
		menuitem = gtk_menu_item_new_with_mnemonic(xconfig->actions[action].name);
		gtk_widget_show(menuitem);
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate", G_CALLBACK(exec_user_action), xconfig->actions[action].command);
		gtk_container_add(GTK_CONTAINER(action_submenu), menuitem);
	}
	menuitem = gtk_image_menu_item_new_with_mnemonic(_("User action"));
	image = gtk_image_new_from_stock("gtk-execute", GTK_ICON_SIZE_MENU);
	gtk_widget_set_name(image, "image");
	gtk_widget_show(image);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), image);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), action_submenu);
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	
	// Separator
	menuitem = gtk_separator_menu_item_new();
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	gtk_widget_set_sensitive(menuitem, FALSE);
	
	// Preference
	menuitem = gtk_image_menu_item_new_from_stock("gtk-preferences", NULL);
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_preference), tray);

	// Keyboard Preference
	menuitem = gtk_menu_item_new_with_mnemonic(_("Keyboard Properties"));
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_kb_preference), tray);
	
	// About
	menuitem = gtk_image_menu_item_new_from_stock("gtk-about", NULL);
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_about), tray);	

	// Exit
	menuitem = gtk_image_menu_item_new_from_stock("gtk-quit", NULL);
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_exit), tray);

 	return menu;
}

void clock_check(Clock *clock)
{
	/*XklEngine *engine = xkl_engine_get_instance (GDK_DISPLAY ());
	const gchar **group_names = xkl_engine_get_groups_names (engine);
	XklConfigRec *config = xkl_config_rec_new ();
	xkl_config_rec_get_from_server(config, engine);
	for (int i=0; i<2; i++)
	{
		printf("%d(total %d) %s (%s)\n", i,
		                                 xkl_engine_get_num_groups(engine), 
		                                 (char*)group_names[i], 
		                                 (char*)config->layouts[i]);
	}*/
	
	struct _tray_icon *tray = clock->tray;
	
	int xneur_pid = xconfig->get_pid(xconfig);
	int xneur_state = xconfig->is_manual_mode(xconfig);
	int xneur_group = get_active_kbd_group();

	if (get_kbd_group_count() != xconfig->handle->total_languages)
	{
		g_spawn_command_line_async(PACKAGE, NULL);
		exit(0);
	}
	
	if (xneur_pid == xneur_old_pid && xneur_state == xneur_old_state && xneur_group == xneur_old_group)
		return;

	xneur_old_pid = xneur_pid;
	xneur_old_state = xneur_state;
	xneur_old_group = xneur_group;

	if (xneur_pid == -1)
		create_tray_icon(tray, FALSE);
	else
		create_tray_icon(tray, TRUE);
}

void create_tray_icon(struct _tray_icon *tray, gboolean runned)
{
	char *layout_name = NULL;
	int lang = get_active_kbd_group();
	if (lang != -1)
		layout_name = strdup(xconfig->handle->languages[lang].dir);

	char *image_file = g_strdup_printf("%s%s", layout_name, ".png");
	
	gchar *hint;
	float saturation;
	if (runned == TRUE)
	{
		saturation = 1.0;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher running ("), layout_name, ")");
	}
	else
	{
		saturation = 0.25;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher stopped ("), layout_name, ")");
	}

	int init_clock = 0;
	if (tray != NULL)
	{
		gtk_object_destroy(GTK_OBJECT(tray->tooltip));
		tray->tooltip = NULL;

		gtk_widget_destroy(GTK_WIDGET(tray->popup_menu));
		tray->popup_menu = NULL;

		gtk_widget_destroy(GTK_WIDGET(tray->image));
		tray->image = NULL;

		gtk_widget_destroy(GTK_WIDGET(tray->evbox));
		tray->evbox = NULL;
	}
	else
	{
		tray = g_new0(struct _tray_icon, 1);	

		tray->widget = _gtk_tray_icon_new(_("X Neural Switcher"));

		g_signal_connect(G_OBJECT(tray->widget), "button_press_event", G_CALLBACK(tray_icon_press), tray);
		g_signal_connect(G_OBJECT(tray->widget), "button_release_event", G_CALLBACK(tray_icon_release), tray);

		init_clock = 1;
	}
		
	tray->tooltip		= gtk_tooltips_new();
	tray->popup_menu	= create_menu_icon(tray, runned, xconfig->is_manual_mode(xconfig));
	tray->evbox		= gtk_event_box_new();

	gtk_event_box_set_visible_window(GTK_EVENT_BOX(tray->evbox), 0);
	gtk_widget_set_size_request(GTK_WIDGET(tray->widget), 27, 18);

	gtk_tooltips_set_tip(tray->tooltip, GTK_WIDGET(tray->widget), hint, NULL);
	
	GdkPixbuf *pb = create_pixbuf(image_file);
	if (pb == NULL)
	{
		for (unsigned int i=0; i < strlen(layout_name); i++)
			layout_name[i] = toupper(layout_name[i]);


		tray->image = gtk_label_new ((const gchar *)layout_name);
		gtk_label_set_justify (GTK_LABEL(tray->image), GTK_JUSTIFY_CENTER);
	}
	else
	{
		GConfClient* gconfClient = gconf_client_get_default();
		g_assert(GCONF_IS_CLIENT(gconfClient));
	
		GConfValue* gcValue = NULL;
		gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "systray_text", NULL);

		// Check text or pixmap
		gboolean value = FALSE;
		if(gcValue != NULL) 
		{
			/* Check if value type is integer */
			if(gcValue->type == GCONF_VALUE_BOOL) 
				value = gconf_value_get_bool(gcValue);

			/* Release resources */
			gconf_value_free(gcValue);
		}

		/* release GConf client */
		g_object_unref(gconfClient);

		if (value) 
		{
			for (unsigned int i=0; i < strlen(layout_name); i++)
				layout_name[i] = toupper(layout_name[i]);
			tray->image = gtk_label_new ((const gchar *)layout_name);
			gtk_label_set_justify (GTK_LABEL(tray->image), GTK_JUSTIFY_CENTER);
		}
		else
		{
			gdk_pixbuf_saturate_and_pixelate(pb, pb, saturation, FALSE);
			tray->image = gtk_image_new_from_pixbuf(pb);
			gdk_pixbuf_unref(pb);
		}
	}	
	
	gtk_container_add(GTK_CONTAINER(tray->evbox), tray->image);
	gtk_container_add(GTK_CONTAINER(tray->widget), tray->evbox);
	gtk_widget_show_all(GTK_WIDGET(tray->widget));

	if (init_clock == 1)
		tray->clock = clock_new(clock_check, tray);

	if (layout_name != NULL)
		free(layout_name);

	if (image_file != NULL)
		free(image_file);
	
	if (hint != NULL)
		free(hint);
}
