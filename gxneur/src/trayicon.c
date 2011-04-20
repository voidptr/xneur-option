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
#include <ctype.h>
#include <string.h>

#include "support.h"

#include "xkb.h"
#include "misc.h"

#include <xneur/xnconfig.h>

#include "trayicon.h"

extern struct _xneur_config *xconfig;

struct _tray_icon *tray;

GConfClient* gconfClient = NULL;
gboolean text_on_tray = FALSE;
gboolean resize_tray_icon = FALSE;
gboolean show_icon_on_panel_indicators = FALSE;

gint expose_handler = 0;

#define TIMER_PERIOD		250

static int xneur_old_pid = -1;
static int xneur_old_state = -1;
static int xneur_old_group = -1;
static int force_update = FALSE;

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

GtkMenu* create_tray_menu(struct _tray_icon *tray, int state)
{
	if (state) {};
	GtkMenu *menu = GTK_MENU(gtk_menu_new());
	GtkWidget *menuitem;
	GtkWidget *image;
	
	// Start/stop
	gchar *status_text;
	int xneur_pid = xconfig->get_pid(xconfig);
	if (xneur_pid != -1)
	{
		status_text = g_strdup_printf("%s", _("Stop daemon"));
	}
	else
	{
		status_text = g_strdup_printf("%s", _("Start daemon"));
	}

	if (tray->status == NULL)
	{
		tray->status = gtk_menu_item_new_with_mnemonic(status_text);
		gtk_widget_show(tray->status);
		gtk_container_add(GTK_CONTAINER(menu), tray->status);
		g_signal_connect(G_OBJECT(tray->status), "activate", G_CALLBACK(xneur_start_stop), tray);
	}
	else
	{
		gtk_menu_item_set_label(GTK_MENU_ITEM(tray->status), status_text);
	}
	g_free(status_text);
	
	//gtk_widget_show(tray->app_indicator_status);
	//gtk_container_add(GTK_CONTAINER(menu), tray->status);
	//g_signal_connect(G_OBJECT(tray->status), "activate", G_CALLBACK(xneur_start_stop), tray);
	
	/*menuitem = gtk_check_menu_item_new_with_mnemonic(_("Auto-correction"));
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menuitem), !state);
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_auto_manual), tray);
	if (xneur_pid == -1)
		gtk_widget_set_sensitive(menuitem, FALSE);*/
	
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
	if (xconfig->actions_count < 1)
		gtk_widget_set_sensitive(menuitem, FALSE);
	
	// Separator
	/*menuitem = gtk_separator_menu_item_new();
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	gtk_widget_set_sensitive(menuitem, FALSE);*/

	// View log
	menuitem = gtk_menu_item_new_with_mnemonic(_("View log..."));
	gtk_widget_show(menuitem);
	gtk_container_add(GTK_CONTAINER(menu), menuitem);
	g_signal_connect(G_OBJECT(menuitem), "activate", G_CALLBACK(xneur_get_logfile), tray);
	
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

	gtk_widget_show (GTK_WIDGET(menu));
 	return menu;
}	

static gboolean tray_icon_release(GtkWidget *widget, GdkEventButton *event, struct _tray_icon *tray)
{	
	if (widget){};
		
	if (event->button == 1 || event->button == 2)
		return TRUE;

	gtk_menu_popdown(GTK_MENU(tray->tray_menu));

	return FALSE;
}

gboolean tray_icon_press(GtkWidget *widget, GdkEventButton *event, struct _tray_icon *tray)
{
	if (widget){};
	
	if (event->button == 2)
	{
		xneur_start_stop();
		return FALSE;
	}
	
	if (event->button == 1)
	{
		set_next_kbd_group();
		return FALSE;
	}

	//if (GTK_IS_WIDGET(tray->tray_menu))
		//gtk_widget_destroy(GTK_WIDGET(tray->tray_menu));
	
	//tray->tray_menu	= create_tray_menu(tray, xconfig->is_manual_mode(xconfig));

	gtk_menu_popup(GTK_MENU(tray->tray_menu), NULL, NULL, NULL, NULL, event->button, event->time);
	
	return FALSE;
}

gboolean tray_expose_event (GtkWidget      *widget,
                            GdkEventExpose *event,
			  struct _tray_icon *tray)
{
	if (widget || tray) {};

	if (tray->height != event->area.height)
	{
		tray->height = event->area.height;
		force_update = TRUE;
	}
	return FALSE;
}

gboolean clock_check(gpointer dummy)
{
	if (dummy) {};
	
	int xneur_pid = xconfig->get_pid(xconfig);
	int xneur_state = xconfig->is_manual_mode(xconfig);
	int xneur_group = get_active_kbd_group();

	if (get_kbd_group_count() != xconfig->handle->total_languages)
	{
		for (int i = 0; i < MAX_LAYOUTS; i++)
		{
			if (tray->images[i] != NULL)
				gdk_pixbuf_unref(tray->images[i]);
		}

		gtk_widget_destroy(GTK_WIDGET(tray->tray_menu));
		tray->tray_menu = NULL;

		gtk_widget_destroy(GTK_WIDGET(tray->evbox));
		tray->evbox = NULL;
		
		g_spawn_command_line_async(PACKAGE, NULL);
		
		gtk_main_quit();
	}
	
	if (xneur_pid == xneur_old_pid && 
	    xneur_state == xneur_old_state && 
	    xneur_group == xneur_old_group &&
	    force_update == FALSE)
		return TRUE;

	force_update = FALSE;

	xneur_old_pid = xneur_pid;
	xneur_old_state = xneur_state;
	xneur_old_group = xneur_group;

	int lang = get_active_kbd_group();
	
	gchar *hint;
	gchar *status_text;
	float saturation;
	if (xneur_pid != -1)
	{
		saturation = 1.0;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher running ("), xconfig->handle->languages[lang].dir, ")");
		status_text = g_strdup_printf("%s", _("Stop daemon"));
	}
	else
	{
		saturation = 0.25;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher stopped ("), xconfig->handle->languages[lang].dir, ")");
		status_text = g_strdup_printf("%s", _("Start daemon"));
	}

	gtk_menu_item_set_label(GTK_MENU_ITEM(tray->status), status_text);
	
	gint kbd_gr = get_active_kbd_group();
	if ((!tray->images[kbd_gr]) || (text_on_tray))
	{
		gtk_widget_destroy(GTK_WIDGET(tray->image));
		char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);
		for (unsigned int i=0; i < strlen(layout_name); i++)
			layout_name[i] = toupper(layout_name[i]); 
		tray->image = gtk_label_new ((const gchar *)layout_name);
		gtk_label_set_justify (GTK_LABEL(tray->image), GTK_JUSTIFY_CENTER);
		
		free(layout_name);
		gtk_container_add(GTK_CONTAINER(tray->evbox), tray->image);
		gtk_widget_show_all(GTK_WIDGET(tray->tray_icon));

	}
	else
	{
		GdkPixbuf *pb = gdk_pixbuf_copy(tray->images[kbd_gr]);
		gdk_pixbuf_saturate_and_pixelate(pb, pb, saturation, FALSE);

		if (resize_tray_icon)
		{
			float proportion = 1;
			int width = gdk_pixbuf_get_width(pb) ;
			int height = gdk_pixbuf_get_height(pb) ;
			proportion = (width > height) ? (float)tray->height / (float)width : (float)tray->height / (float)height;
			width *= proportion;
			height *= proportion;
			if ((width > 0) && (height > 0)) 
				pb = gdk_pixbuf_scale_simple (pb, width, height, GDK_INTERP_BILINEAR);
		}
		
		gtk_widget_destroy(GTK_WIDGET(tray->image));
		tray->image = gtk_image_new_from_pixbuf(pb);
		gtk_container_add(GTK_CONTAINER(tray->evbox), tray->image);
		gtk_widget_show_all(GTK_WIDGET(tray->tray_icon));
		
		gdk_pixbuf_unref(pb);
		
	}

	if (tray->tooltip != NULL)
	{
		gtk_object_destroy(GTK_OBJECT(tray->tooltip));
		tray->tooltip = NULL;
	}
	tray->tooltip = gtk_tooltips_new();
	gtk_tooltips_set_tip(tray->tooltip, GTK_WIDGET(tray->tray_icon), hint, NULL);

	g_free (hint);
	g_free (status_text);
	
#ifdef HAVE_APP_INDICATOR
	// App indicator part
	if (xneur_pid != -1)
	{
		app_indicator_set_status (tray->app_indicator, APP_INDICATOR_STATUS_ACTIVE);
	}
	else
	{
		app_indicator_set_status (tray->app_indicator, APP_INDICATOR_STATUS_ATTENTION);
	}

	if (!show_icon_on_panel_indicators)
		app_indicator_set_status (tray->app_indicator, APP_INDICATOR_STATUS_PASSIVE);

	char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);

	char *image_file = NULL;
	GConfClient* gconfClient = gconf_client_get_default();
	GConfValue* gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "pixmap_dir", NULL);
	if(gcValue != NULL) 
	{
		const char *string_value = NULL;
		if(gcValue->type == GCONF_VALUE_STRING) 
		{
			string_value = gconf_value_get_string(gcValue);
			image_file = g_strdup_printf("%s/%s%s", string_value, layout_name, ".png");
		}
		gconf_value_free(gcValue);
	}
	g_object_unref(gconfClient);
	
	if (!g_file_test (image_file, G_FILE_TEST_EXISTS) || (text_on_tray))
	{
		for (unsigned int i=0; i < strlen(layout_name); i++)
			layout_name[i] = toupper(layout_name[i]);
#ifdef HAVE_DEPREC_APP_INDICATOR	
		app_indicator_set_icon (tray->app_indicator, image_file);
#else
		app_indicator_set_label (tray->app_indicator, layout_name, layout_name);
		app_indicator_set_icon (tray->app_indicator, "");
#endif
	}
	else
	{
#ifdef HAVE_DEPREC_APP_INDICATOR
		app_indicator_set_icon (tray->app_indicator, image_file);
#else
		app_indicator_set_icon (tray->app_indicator, image_file);
		app_indicator_set_label (tray->app_indicator,"", "");
#endif
	}
		
	free(image_file);
	free(layout_name);
#endif
	
	return TRUE;
}

void xneur_start_stop(void)
{
	if (xconfig->kill(xconfig) == TRUE)
	{
		clock_check(0);
		return;
	}

	xneur_start();
	clock_check(0);
}

void xneur_auto_manual(void)
{
	xconfig->set_manual_mode(xconfig, !xconfig->is_manual_mode(xconfig));
}

void xneur_exit(void)
{
	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		if (tray->images[i] != NULL)
			gdk_pixbuf_unref(tray->images[i]);
	}

	gtk_widget_destroy(GTK_WIDGET(tray->tray_menu));
	tray->tray_menu = NULL;
	
	xconfig->kill(xconfig);
	gtk_main_quit();
}

void gconf_key_resize_tray_icon_callback(GConfClient* client,
                            guint cnxn_id,
                            GConfEntry* entry,
                            gpointer user_data)
{
	if (client || cnxn_id || user_data) {};
	
	if (gconf_entry_get_value (entry) != NULL && gconf_entry_get_value (entry)->type == GCONF_VALUE_BOOL)
		resize_tray_icon = gconf_value_get_bool (gconf_entry_get_value (entry));

	if (resize_tray_icon)
	{
		g_signal_handler_unblock(tray->tray_icon, expose_handler);
	}
	else
	{
		g_signal_handler_block(tray->tray_icon, expose_handler);
	}
	
	force_update = TRUE;
}

void gconf_key_show_icon_on_panel_indicator_callback(GConfClient* client,
                            guint cnxn_id,
                            GConfEntry* entry,
                            gpointer user_data)
{
	if (client || cnxn_id || user_data) {};
	
	if (gconf_entry_get_value (entry) != NULL && gconf_entry_get_value (entry)->type == GCONF_VALUE_BOOL)
		show_icon_on_panel_indicators = gconf_value_get_bool (gconf_entry_get_value (entry));

	force_update = TRUE;
}

void gconf_key_text_on_tray_callback(GConfClient* client,
                            guint cnxn_id,
                            GConfEntry* entry,
                            gpointer user_data)
{
	if (client || cnxn_id || user_data) {};
	
	if (gconf_entry_get_value (entry) != NULL && gconf_entry_get_value (entry)->type == GCONF_VALUE_BOOL)
		text_on_tray = gconf_value_get_bool (gconf_entry_get_value (entry));

	force_update = TRUE;
}

void gconf_key_pixmap_dir_callback(GConfClient* client,
                            guint cnxn_id,
                            GConfEntry* entry,
                            gpointer user_data)
{
	if (client || cnxn_id || user_data) {};
	
	if (gconf_entry_get_value (entry) != NULL && gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
		add_pixmap_directory(gconf_value_get_string (gconf_entry_get_value (entry)));
	for (int i = 0; i < xconfig->handle->total_languages; i++)
	{
		char *layout_name = strdup(xconfig->handle->languages[i].dir);
		char *image_file = g_strdup_printf("%s%s", layout_name, ".png");
		tray->images[i] = create_pixbuf(image_file);
		free(image_file);
		free(layout_name);
	}
	
	force_update = TRUE;
}

void create_tray_icon(void)
{
	tray = g_new0(struct _tray_icon, 1);	

	tray->tray_icon = _gtk_tray_icon_new(_("X Neural Switcher"));

	g_signal_connect(G_OBJECT(tray->tray_icon), "button_press_event", G_CALLBACK(tray_icon_press), tray);
	g_signal_connect(G_OBJECT(tray->tray_icon), "button_release_event", G_CALLBACK(tray_icon_release), tray);
	
	// Init pixbuf array
	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		tray->images[i] = NULL;
	}

	gint kbd_gr = get_active_kbd_group();
	// Load images to pixbufs
	for (int i = 0; i < xconfig->handle->total_languages; i++)
	{
		char *layout_name = strdup(xconfig->handle->languages[i].dir);
		char *image_file = g_strdup_printf("%s%s", layout_name, ".png");
		tray->images[i] = create_pixbuf(image_file);
		free(image_file);
		free(layout_name);
	}

	tray->evbox		= gtk_event_box_new();
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(tray->evbox), 0);

	if (tray->images[kbd_gr])
	{
		GdkPixbuf *pb = gdk_pixbuf_copy(tray->images[kbd_gr]);
		gdk_pixbuf_saturate_and_pixelate(pb, pb, .25, FALSE);
		tray->image = gtk_image_new_from_pixbuf(pb);
		gdk_pixbuf_unref(pb);
	}
	else
	{
		char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);
		for (unsigned int i=0; i < strlen(layout_name); i++)
			layout_name[i] = toupper(layout_name[i]); 
		tray->image = gtk_label_new ((const gchar *)layout_name);
		gtk_label_set_justify (GTK_LABEL(tray->image), GTK_JUSTIFY_CENTER);
		free(layout_name);
	}

	gtk_container_add(GTK_CONTAINER(tray->evbox), tray->image);
	gtk_container_add(GTK_CONTAINER(tray->tray_icon), tray->evbox);
	gtk_widget_show_all(GTK_WIDGET(tray->tray_icon));

	tray->tray_menu	= create_tray_menu(tray, xconfig->is_manual_mode(xconfig));
	
#ifdef HAVE_APP_INDICATOR
	// App indicator
	tray->app_indicator = app_indicator_new ("X Neural Switcher",
                               "gxneur",
                               APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	
	app_indicator_set_status (tray->app_indicator, APP_INDICATOR_STATUS_ACTIVE);
	                                    
	tray->app_indicator_menu = create_tray_menu(tray, xconfig->is_manual_mode(xconfig));
	app_indicator_set_menu (tray->app_indicator, tray->app_indicator_menu);
#endif
	
	GConfClient* gconfClient = gconf_client_get_default();

	GConfValue* gcValue = NULL;
	
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "text_on_tray", NULL);
	if(gcValue != NULL) 
	{
		if(gcValue->type == GCONF_VALUE_BOOL) 
			text_on_tray = gconf_value_get_bool(gcValue);

		gconf_value_free(gcValue);
	}

	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "show_icon_on_panel_indicators", NULL);
	if(gcValue != NULL) 
	{
		if(gcValue->type == GCONF_VALUE_BOOL) 
			show_icon_on_panel_indicators = gconf_value_get_bool(gcValue);

		gconf_value_free(gcValue);
	}
	
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "resize_tray_icon", NULL);
	if(gcValue != NULL) 
	{
		if(gcValue->type == GCONF_VALUE_BOOL) 
			resize_tray_icon = gconf_value_get_bool(gcValue);

		gconf_value_free(gcValue);
	}
	expose_handler = g_signal_connect (tray->tray_icon, "expose-event", G_CALLBACK (tray_expose_event), tray);
	
	if (resize_tray_icon)
	{
		if (!g_signal_handler_is_connected(tray->tray_icon, expose_handler))
			g_signal_handler_unblock(tray->tray_icon, expose_handler);
	}
	else
	{
		if (g_signal_handler_is_connected(tray->tray_icon, expose_handler))
			g_signal_handler_block(tray->tray_icon, expose_handler);
	}
	
	gconf_client_notify_add(gconfClient,
                          PACKAGE_GCONF_DIR "pixmap_dir",
                          gconf_key_pixmap_dir_callback,
                          NULL,
                          NULL,
                          NULL);		                        
	gconf_client_notify_add(gconfClient,
                          PACKAGE_GCONF_DIR "resize_tray_icon",
                          gconf_key_resize_tray_icon_callback,
                          NULL,
                          NULL,
                          NULL);
	gconf_client_notify_add(gconfClient,
                          PACKAGE_GCONF_DIR "show_icon_on_panel_indicators",
                          gconf_key_show_icon_on_panel_indicator_callback,
                          NULL,
                          NULL,
                          NULL);
	gconf_client_notify_add(gconfClient,
                          PACKAGE_GCONF_DIR "text_on_tray",
                          gconf_key_text_on_tray_callback,
                          NULL,
                          NULL,
                          NULL);
	g_object_unref(gconfClient);
	
	g_timeout_add(TIMER_PERIOD, clock_check, 0);
}
