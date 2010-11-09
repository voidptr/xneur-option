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

#define TIMER_PERIOD		250

static int xneur_old_pid = -1;
static int xneur_old_state = -1;
static int xneur_old_group = -1;

void set_text_to_tray_icon(const gint size, const gchar *markup )
{
    cairo_surface_t *surface;
    cairo_t         *cr;
    //GdkPixbuf       *pixbuf;
    PangoLayout     *layout;
    gint             i_width,
                     i_height,
                     l_width,
                     l_height,
                     dx,
                     dy;
                     
    PangoFontDescription *desc;

	surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, size, size);
    i_width  = cairo_image_surface_get_width( surface );
    i_height = cairo_image_surface_get_height( surface );
    cr = gdk_cairo_create(GDK_DRAWABLE(tray->tray_icon));

    layout = pango_cairo_create_layout( cr );
    pango_layout_set_text( layout, markup, -1 );
    desc = pango_font_description_from_string( "Ubuntu 14" );
    pango_layout_set_font_description( layout, desc );
    pango_font_description_free( desc );

    // Center text 
    pango_layout_get_size( layout, &l_width, &l_height );
    l_width  /= PANGO_SCALE;
    l_height /= PANGO_SCALE;

    dx = ( i_width - l_width ) * .5;
    dy = ( i_height - l_height ) * .5;

    cairo_move_to( cr, dx, dy );
    pango_cairo_show_layout( cr, layout );
    cairo_fill( cr );
	
    g_object_unref( layout );
    cairo_destroy( cr );

    // Convert cairo surface to pixbuf
    /*pixbuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB, TRUE, 8, i_width, i_height );
 
    s_stride = cairo_image_surface_get_stride( surface );
    p_stride = gdk_pixbuf_get_rowstride( pixbuf );
    s_data = cairo_image_surface_get_data( surface );
    p_data = gdk_pixbuf_get_pixels( pixbuf );

    for( i = 0; i < i_height; i++ )
    {
        for( j = 0; j < i_width; j++ )
        {
            gint    s_index = i * s_stride + j * 4,
                    p_index = i * p_stride + j * 4;
            gdouble alpha;

            alpha = s_data[s_index + 3] ?
                        (gdouble)s_data[s_index + 3] / 0xff : 1.0;

            p_data[p_index    ] = s_data[s_index + 2] / alpha;
            p_data[p_index + 1] = s_data[s_index + 1] / alpha;
            p_data[p_index + 2] = s_data[s_index    ] / alpha;
            p_data[p_index + 3] = s_data[s_index + 3];
        }
    }*/

    cairo_surface_destroy( surface );

    return;
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

GtkWidget* create_tray_menu(struct _tray_icon *tray, int state)
{
	GtkWidget *menu = gtk_menu_new();
	
	// Start/stop
	gchar *menu_text;
	gchar *menu_icon;
	int xneur_pid = xconfig->get_pid(xconfig);
	if (xneur_pid != -1)
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
	if (xneur_pid == -1)
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

	gtk_widget_show (menu);
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

	if (GTK_IS_WIDGET(tray->tray_menu))
		gtk_widget_destroy(GTK_WIDGET(tray->tray_menu));
	
	tray->tray_menu	= create_tray_menu(tray, xconfig->is_manual_mode(xconfig));

	gtk_menu_popup(GTK_MENU(tray->tray_menu), NULL, NULL, NULL, NULL, event->button, event->time);
	
	return FALSE;
}

static void tray_icon_handle_notify (GtkWidget *widget, GParamSpec *arg1, struct _tray_icon *tray)
{
	if (widget){};
	
    if ( g_str_equal(arg1->name,"size") )
    {
		int xneur_pid = xconfig->get_pid(xconfig);

		float saturation;
		if (xneur_pid != -1)
		{
			saturation = 1.0;
		}
		else
		{
			saturation = 0.25;
		}
		if (tray->images[get_active_kbd_group()])
		{
			if (gtk_status_icon_is_embedded(tray->tray_icon))
			{
        		gint size = gtk_status_icon_get_size(tray->tray_icon);
				GdkPixbuf *pb = gdk_pixbuf_copy (tray->images[get_active_kbd_group()]);
				gdk_pixbuf_saturate_and_pixelate(tray->images[get_active_kbd_group()], pb, saturation, FALSE);
				pb = gdk_pixbuf_scale_simple (pb, size, size * 15/22, GDK_INTERP_BILINEAR);
				gtk_status_icon_set_from_pixbuf(tray->tray_icon, pb);
				gdk_pixbuf_unref(pb); 
			}
		}
		else
		gtk_status_icon_set_from_icon_name(tray->tray_icon, "keyboard");
	}
	/*if (g_str_equal(arg1->name,"embedded"))
	{
		b_value = gtk_status_icon_is_embedded(pgsi->tray_icon);
	}
	if ( g_str_equal(arg1->name,"visible") )
	{
		b_value = gtk_status_icon_get_vis  ble(pgsi->tray_icon);
	}*/
   
	return;
}

gboolean clock_check(gpointer data)
{
	struct _tray_icon *tray = (struct _tray_icon *)data;
	
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
	
	if (xneur_pid == xneur_old_pid && xneur_state == xneur_old_state && xneur_group == xneur_old_group)
		return TRUE;

	xneur_old_pid = xneur_pid;
	xneur_old_state = xneur_state;
	xneur_old_group = xneur_group;

	int lang = get_active_kbd_group();
	
	gchar *hint;
	float saturation;
	if (xneur_pid != -1)
	{
		saturation = 1.0;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher running ("), xconfig->handle->languages[lang].dir, ")");
	}
	else
	{
		saturation = 0.25;
		hint = g_strdup_printf("%s%s%s", _("X Neural Switcher stopped ("), xconfig->handle->languages[lang].dir, ")");
	}

	if (tray->images[get_active_kbd_group()])
	{
		if (gtk_status_icon_is_embedded(tray->tray_icon))
		{
			gint size = gtk_status_icon_get_size(tray->tray_icon);
			GdkPixbuf *pb = gdk_pixbuf_copy(tray->images[get_active_kbd_group()]);
			gdk_pixbuf_saturate_and_pixelate(tray->images[get_active_kbd_group()], pb, saturation, FALSE);
			pb = gdk_pixbuf_scale_simple(pb, size, size * 15/22, GDK_INTERP_BILINEAR);
			gtk_status_icon_set_from_pixbuf(tray->tray_icon, pb);
			gdk_pixbuf_unref(pb);
		}
	}
	else
		gtk_status_icon_set_from_icon_name(tray->tray_icon, "keyboard");
	
	gtk_status_icon_set_tooltip(tray->tray_icon, hint);
	g_free (hint);

	return TRUE;
}

void xneur_start_stop(void)
{
	if (xconfig->kill(xconfig) == TRUE)
	{
		clock_check(tray);
		return;
	}

	xneur_start();
	clock_check(tray);
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

void create_tray_icon(void)
{
	tray = g_new0(struct _tray_icon, 1);	

	tray->tray_icon = gtk_status_icon_new();

	g_signal_connect(G_OBJECT(tray->tray_icon), "button_press_event", G_CALLBACK(tray_icon_press), tray);
	g_signal_connect(G_OBJECT(tray->tray_icon), "button_release_event", G_CALLBACK(tray_icon_release), tray);
	g_signal_connect(G_OBJECT(tray->tray_icon), "notify::size", G_CALLBACK (tray_icon_handle_notify), tray);

	// Load images to pixbufs
	for (int i = 0; i < xconfig->handle->total_languages; i++)
	{
		char *layout_name = strdup(xconfig->handle->languages[i].dir);
		char *image_file = g_strdup_printf("%s%s", layout_name, ".png");
		if (find_pixmap_file(image_file) == NULL)
		{
			tray->images[i] = NULL;
			continue;
		}
		
		tray->images[i] = create_pixbuf(image_file);
		for (unsigned int i=0; i < strlen(layout_name); i++)
			layout_name[i] = toupper(layout_name[i]); 

		//gint size = gtk_status_icon_get_size(tray->tray_icon);
		//set_text_to_tray_icon(size, layout_name);	
		free(image_file);
		free(layout_name);
	}

	if (tray->images[get_active_kbd_group()])
		gtk_status_icon_set_from_pixbuf(tray->tray_icon, tray->images[get_active_kbd_group()]);
	else
		gtk_status_icon_set_from_icon_name(tray->tray_icon, "keyboard");
	
	g_timeout_add(TIMER_PERIOD, clock_check, (gpointer) tray);
}
