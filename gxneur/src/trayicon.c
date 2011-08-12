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
#    include <gconf/gconf-client.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "support.h"

#include "xkb.h"
#include "misc.h"

#include <xneur/xnconfig.h>

#include "trayicon.h"

#include "configuration.h"

extern const char* arg_show_in_the_tray;
extern const char* arg_rendering_engine;

extern struct _xneur_config *xconfig;

struct _tray_icon *tray;
#ifdef HAVE_GCONF
GConfClient* gconfClient = NULL;
#endif
gchar *show_in_the_tray = NULL;
gchar *rendering_engine = NULL;

Display *dpy = NULL;

static int xneur_old_pid = -1;
static int xneur_old_state = -1;
static int xneur_old_group = -1;
static int force_update = FALSE;

#define ICON_SIZE 24

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

GtkMenu* create_menu(struct _tray_icon *tray, int state)
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
 	return GTK_MENU(menu);
}		

gboolean tray_icon_press(GtkWidget *widget, GdkEventButton *event)
{
	if (widget){};
	
	if (event->button == 2)
	{
		xneur_start_stop();
		return FALSE;
	}
	
	if (event->button == 1)
	{
		set_next_kbd_group(dpy);
		return FALSE;
	}

	gtk_menu_popup(GTK_MENU(tray->menu), NULL, NULL, NULL, NULL, event->button, event->time);
	
	return FALSE;
}

void status_icon_on_click(GtkStatusIcon *status_icon, 
                        gpointer user_data)
{
	if (status_icon || user_data){};
    set_next_kbd_group(dpy);
}

void status_icon_on_menu(GtkStatusIcon *status_icon, guint button, 
                       guint activate_time, gpointer user_data)
{
	if (status_icon || user_data){};
	
    gtk_menu_popup(GTK_MENU(tray->menu), NULL, NULL, NULL, NULL, button, activate_time);
}

GdkPixbuf *text_to_gtk_pixbuf (GdkPixbuf *pb, int w, int h, gchar *text) 
{ 
	GdkPixmap *pm = gdk_pixmap_new (NULL, w, h, 24);
	GdkGC *gc = gdk_gc_new (pm); 
	gdk_draw_pixbuf (pm, gc, pb, 0, 0, 0, 0, w, h, GDK_RGB_DITHER_NONE, 0, 0);

	GtkWidget *scratch = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize (scratch); 
	//GtkStyle *style = gtk_widget_get_style(scratch);
	//gchar *bgcolor = gdk_color_to_string(&style->bg[GTK_STATE_NORMAL]);
	//gchar *textcolor = gdk_color_to_string(&style->text[GTK_STATE_NORMAL]);
	PangoLayout *layout = gtk_widget_create_pango_layout (scratch, NULL); 
	//g_object_unref(style);
	gtk_widget_destroy (scratch);

	//gchar *markup = g_strdup_printf ("<span bgcolor='%s' color='%s'>%s</span>", bgcolor, textcolor, text); 
	//gchar *markup = g_strdup_printf ("<span color='%s'>%s</span>", textcolor, text); 
	gchar *markup = g_strdup_printf ("%s", text);
	pango_layout_set_markup (layout, markup, -1); 
	g_free (markup);
	//g_free (bgcolor);
	//g_free (textcolor);

	int width = 0;
	int heigth = 0;
	pango_layout_get_size (layout, &width, &heigth);
	width = width/PANGO_SCALE;
	heigth = heigth/PANGO_SCALE;

	gdk_draw_layout (pm, gc, (w - width)/2,(h - heigth)/2, layout);

	g_object_unref(layout);
	gdk_gc_unref(gc);
	
	GdkPixbuf *ret = gdk_pixbuf_get_from_drawable (NULL, pm, NULL, 0, 0, 0, 0, w, h);
	return ret; 
} 

static const char *get_tray_icon_name (char *name)
{
    const char * icon_name;

	if (strcasecmp(show_in_the_tray, "Icon") == 0)
	{
		icon_name = PACKAGE;
		return icon_name;
	}
	
    GtkIconTheme * theme = gtk_icon_theme_get_default( );

    // if the tray's icon is a 48x48 file, use it;
    // otherwise, use the fallback builtin icon 
    if (!gtk_icon_theme_has_icon (theme, name))
        icon_name = PACKAGE;
    else 
	{
        GtkIconInfo * icon_info = gtk_icon_theme_lookup_icon (theme, name, 48, GTK_ICON_LOOKUP_USE_BUILTIN);
        const gboolean icon_is_builtin = gtk_icon_info_get_filename (icon_info) == NULL;
        gtk_icon_info_free (icon_info);
        icon_name = icon_is_builtin ? PACKAGE : name;
    }

    return icon_name;
}

gboolean clock_check(gpointer dummy)
{
	if (dummy) {};

	int xneur_pid = xconfig->get_pid(xconfig);
	int xneur_state = xconfig->is_manual_mode(xconfig);
	int xneur_group = get_active_kbd_group(dpy);

	if (get_kbd_group_count(dpy) != xconfig->handle->total_languages)
	{
		for (int i = 0; i < MAX_LAYOUTS; i++)
		{
			if (tray->images[i] != NULL)
				g_free(tray->images[i]);
		}

		gtk_widget_destroy(GTK_WIDGET(tray->menu));
		tray->menu = NULL;
		
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
	
	int lang = get_active_kbd_group(dpy);
	
	gchar *hint;
	gchar *status_text;
	//float saturation = 1.0;
	if (xneur_pid != -1)
	{
		//saturation = 1.0;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher running ("), xconfig->handle->languages[lang].dir, ")");
		status_text = g_strdup_printf("%s", _("Stop daemon"));
	}
	else
	{
		//saturation = 0.25;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher stopped ("), xconfig->handle->languages[lang].dir, ")");
		status_text = g_strdup_printf("%s", _("Start daemon"));
	}

	gtk_menu_item_set_label(GTK_MENU_ITEM(tray->status), status_text);

	gint kbd_gr = get_active_kbd_group(dpy);


	const char *icon_name = get_tray_icon_name(tray->images[kbd_gr]);
	if (strcasecmp(rendering_engine, "Built-in") == 0)
	{
		gtk_widget_destroy (tray->image);
		if (strcasecmp(show_in_the_tray, "Text") == 0)
		{
			char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);
			for (unsigned int i=0; i < strlen(layout_name); i++)
				layout_name[i] = toupper(layout_name[i]); 
			tray->image = gtk_label_new ((const gchar *)layout_name);
			gtk_label_set_justify (GTK_LABEL(tray->image), GTK_JUSTIFY_CENTER);
			free(layout_name);
		}
		else
		{
			tray->image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
		}
		gtk_container_add(GTK_CONTAINER(tray->evbox), tray->image);
		gtk_widget_show_all(GTK_WIDGET(tray->tray_icon));
	}
	else if (strcasecmp(rendering_engine, "StatusIcon") == 0)
	{
		if (gtk_status_icon_is_embedded(tray->status_icon))
		{					
			if (strcasecmp(show_in_the_tray, "Text") == 0)
			{
				char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);
				for (unsigned int i=0; i < strlen(layout_name); i++)
					layout_name[i] = toupper(layout_name[i]);

				GdkPixbuf *trasparent_pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, ICON_SIZE, ICON_SIZE);
				gdk_pixbuf_fill(trasparent_pb, 0xffffffff);
				GdkPixbuf *pb = text_to_gtk_pixbuf (trasparent_pb, gdk_pixbuf_get_width(trasparent_pb), gdk_pixbuf_get_height(trasparent_pb), layout_name);
				free(layout_name);
				pb = gdk_pixbuf_add_alpha(pb, TRUE, 255, 255, 255);
				gtk_status_icon_set_from_pixbuf(tray->status_icon, pb);
				gdk_pixbuf_unref(pb);
				gdk_pixbuf_unref(trasparent_pb);
			}
			else
			{
				gtk_status_icon_set_from_icon_name(tray->status_icon, icon_name);
			}

			gtk_status_icon_set_tooltip(tray->status_icon, hint);
		}	
	}
	else if (strcasecmp(rendering_engine, "AppIndicator") == 0)
	{
#ifdef HAVE_APP_INDICATOR
		char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);
		if (strcasecmp(show_in_the_tray, "Text") == 0)
		{
			for (unsigned int i=0; i < strlen(layout_name); i++)
				layout_name[i] = toupper(layout_name[i]);
#ifdef HAVE_DEPREC_APP_INDICATOR	
			app_indicator_set_icon (tray->app_indicator, icon_name);
#else
			app_indicator_set_label (tray->app_indicator, layout_name, layout_name);
			app_indicator_set_icon (tray->app_indicator, "");
#endif
		}
		else
		{
#ifdef HAVE_DEPREC_APP_INDICATOR
			app_indicator_set_icon (tray->app_indicator, icon_name);
#else
			app_indicator_set_icon (tray->app_indicator, icon_name);
			app_indicator_set_label (tray->app_indicator,"", "");
#endif
		}
		free(layout_name);
#endif
	}
	
	g_free (hint);
	g_free (status_text);

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
			g_free(tray->images[i]);
	}

	gtk_widget_destroy(GTK_WIDGET(tray->menu));
	tray->menu = NULL;
	
	xconfig->kill(xconfig);
	XCloseDisplay(dpy);
	gtk_main_quit();
}

#ifdef HAVE_GCONF

void gconf_key_show_in_the_tray_callback(GConfClient* client,
                            guint cnxn_id,
                            GConfEntry* entry,
                            gpointer user_data)
{
	if (client || cnxn_id || user_data) {};

	if (arg_show_in_the_tray)
		return;
	
	if (gconf_entry_get_value (entry) != NULL && gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
		show_in_the_tray = strdup(gconf_value_get_string (gconf_entry_get_value (entry)));

	force_update = TRUE;
}

void gconf_key_rendering_engine_callback(GConfClient* client,
                            guint cnxn_id,
                            GConfEntry* entry,
                            gpointer user_data)
{
	if (client || cnxn_id || user_data) {};

	if (arg_rendering_engine)
		return;

	const char *new_engine = rendering_engine;
	if (gconf_entry_get_value (entry) != NULL && gconf_entry_get_value (entry)->type == GCONF_VALUE_STRING)
		new_engine = strdup(gconf_value_get_string (gconf_entry_get_value (entry)));

	if (strcasecmp(new_engine, rendering_engine) == 0)
		return;
	
	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		if (tray->images[i] != NULL)
			g_free(tray->images[i]);
	}

	gtk_widget_destroy(GTK_WIDGET(tray->menu));
	tray->menu = NULL;
		
	g_spawn_command_line_async(PACKAGE, NULL);
	
	gtk_main_quit();
}

#endif

void create_tray_icon (void)
{
	dpy = XOpenDisplay(NULL);
#ifdef HAVE_GCONF
	GConfClient* gconfClient = gconf_client_get_default();

	GConfValue* gcValue = NULL;

	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "show_in_the_tray", NULL);
	if(gcValue == NULL) 
	{
		if(!gconf_client_set_string(gconfClient, PACKAGE_GCONF_DIR "show_in_the_tray", DEFAULT_SHOW_IN_THE_TRAY, NULL)) 
		    g_warning("Failed to set %s (%s)\n", PACKAGE_GCONF_DIR "show_in_the_tray", DEFAULT_SHOW_IN_THE_TRAY);
	}
	else
	{
		if(gcValue->type == GCONF_VALUE_STRING) 
			show_in_the_tray = strdup(gconf_value_get_string(gcValue));

		gconf_value_free(gcValue);
	}
	
	gconf_client_notify_add(gconfClient,
                          PACKAGE_GCONF_DIR "show_in_the_tray",
                          gconf_key_show_in_the_tray_callback,
                          NULL,
                          NULL,
                          NULL);
	
	gcValue = gconf_client_get_without_default(gconfClient, PACKAGE_GCONF_DIR "rendering_engine", NULL);
	if(gcValue == NULL) 
	{
		// May be:
		// 1. Built-in
		// 2. StatusIcon
		// 3. AppIndicator
		if(!gconf_client_set_string(gconfClient, PACKAGE_GCONF_DIR "rendering_engine", DEFAULT_RENDERING_ENGINE, NULL)) 
		    g_warning("Failed to set %s (%s)\n", PACKAGE_GCONF_DIR "rendering_engine", DEFAULT_RENDERING_ENGINE);
	}
	else
	{
		if(gcValue->type == GCONF_VALUE_STRING) 
			rendering_engine = strdup(gconf_value_get_string(gcValue));

		gconf_value_free(gcValue);
	}
	
	gconf_client_notify_add(gconfClient,
                          PACKAGE_GCONF_DIR "rendering_engine",
                          gconf_key_rendering_engine_callback,
                          NULL,
                          NULL,
                          NULL);
	//g_object_unref(gconfClient);
#endif
	if (arg_show_in_the_tray)
		show_in_the_tray = strdup(arg_show_in_the_tray);
	if (arg_rendering_engine)
		rendering_engine = strdup(arg_rendering_engine);
	if (!show_in_the_tray)
		show_in_the_tray = strdup(DEFAULT_SHOW_IN_THE_TRAY);
	if (!rendering_engine)
		rendering_engine = strdup(DEFAULT_RENDERING_ENGINE);


	tray = g_new0(struct _tray_icon, 1);
#ifdef HAVE_APP_INDICATOR
	tray->app_indicator = NULL;
#endif
	tray->status_icon = NULL;
	tray->tray_icon = NULL;
	
	// Init pixbuf array
	for (int i = 0; i < MAX_LAYOUTS; i++)
	{
		tray->images[i] = NULL;
	}

	// Load images names
	for (int i = 0; i < xconfig->handle->total_languages; i++)
	{
		char *layout_name = strdup(xconfig->handle->languages[i].dir);
		tray->images[i] = g_strdup_printf("%s-%s", PACKAGE, layout_name);
		free(layout_name);
	}

	
	tray->menu = create_menu(tray, xconfig->is_manual_mode(xconfig));

	if (strcasecmp(rendering_engine, "AppIndicator") == 0)
	{
#ifdef HAVE_APP_INDICATOR
		// App indicator
		tray->app_indicator = app_indicator_new ("X Neural Switcher",
		                           PACKAGE,
		                           APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	
		app_indicator_set_status (tray->app_indicator, APP_INDICATOR_STATUS_ACTIVE);
			                                
		app_indicator_set_menu (tray->app_indicator, tray->menu);
#endif	
	}
	
	gint kbd_gr = get_active_kbd_group(dpy);
	
	// Status Icon
	if (strcasecmp(rendering_engine, "StatusIcon") == 0)
	{
		tray->status_icon = gtk_status_icon_new();

		g_signal_connect(G_OBJECT(tray->status_icon), "activate", G_CALLBACK(status_icon_on_click), NULL);
		g_signal_connect(G_OBJECT(tray->status_icon), "popup-menu", G_CALLBACK(status_icon_on_menu), NULL);

		gtk_status_icon_set_from_icon_name(tray->status_icon,  PACKAGE);
		gtk_status_icon_set_tooltip_text(tray->status_icon, "X Neural Switcher");
		gtk_status_icon_set_visible(tray->status_icon, TRUE);

		if (strcasecmp(show_in_the_tray, "Text") == 0)
		{
			char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);
			for (unsigned int i=0; i < strlen(layout_name); i++)
				layout_name[i] = toupper(layout_name[i]);

			GdkPixbuf *trasparent_pb = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, ICON_SIZE, ICON_SIZE);
			gdk_pixbuf_fill(trasparent_pb, 0xffffffff);
			GdkPixbuf *pb = text_to_gtk_pixbuf (trasparent_pb, gdk_pixbuf_get_width(trasparent_pb), gdk_pixbuf_get_height(trasparent_pb), layout_name);
			free(layout_name);
			pb = gdk_pixbuf_add_alpha(pb, TRUE, 255, 255, 255);
			gtk_status_icon_set_from_pixbuf(tray->status_icon, pb);
			gdk_pixbuf_unref(pb);
			gdk_pixbuf_unref(trasparent_pb);
		}
		else
		{
			gtk_status_icon_set_from_icon_name(tray->status_icon, tray->images[kbd_gr]);
		}
	}
	
	// Tray icon
	if (strcasecmp(rendering_engine, "Built-in") == 0)
	{
		tray->tray_icon = _gtk_tray_icon_new(_("X Neural Switcher"));

		g_signal_connect(G_OBJECT(tray->tray_icon), "button_press_event", G_CALLBACK(tray_icon_press), NULL);
		
		tray->evbox		= gtk_event_box_new();
		gtk_event_box_set_visible_window(GTK_EVENT_BOX(tray->evbox), 0);
		if (strcasecmp(show_in_the_tray, "Text") == 0)
		{
			char *layout_name = strdup(xconfig->handle->languages[kbd_gr].dir);
			for (unsigned int i=0; i < strlen(layout_name); i++)
				layout_name[i] = toupper(layout_name[i]); 
			tray->image = gtk_label_new ((const gchar *)layout_name);
			gtk_label_set_justify (GTK_LABEL(tray->image), GTK_JUSTIFY_CENTER);
			free(layout_name);
		}
		else
		{
			tray->image = gtk_image_new_from_icon_name(tray->images[kbd_gr], GTK_ICON_SIZE_LARGE_TOOLBAR);
		}
		gtk_container_add(GTK_CONTAINER(tray->evbox), tray->image);
		gtk_container_add(GTK_CONTAINER(tray->tray_icon), tray->evbox);
		
		gtk_widget_show_all(GTK_WIDGET(tray->tray_icon));
	}
	force_update = TRUE;

	g_timeout_add(G_PRIORITY_DEFAULT_IDLE, clock_check, 0);
}
