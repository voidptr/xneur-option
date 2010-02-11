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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <string.h>
#include <stdlib.h>

#include "misc.h"

#include "support.h"

#include "callbacks.h"

#include <glade/glade.h>

// Save dictionary
void on_okbutton1_clicked(GtkButton *button, gpointer user_data)
{
	if (button){};

	GladeXML *gxml = user_data;
	GtkWidget *widgetPtrToBefound = glade_xml_get_widget (gxml, "textview1");
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgetPtrToBefound));

	GtkTextIter begin, end;
	gtk_text_buffer_get_bounds(buffer, &begin, &end);

	char *text = gtk_text_buffer_get_text(buffer, &begin, &end, FALSE);

	widgetPtrToBefound = glade_xml_get_widget (gxml, "entry10");

	char *path = g_strdup_printf("%s", gtk_entry_get_text(GTK_ENTRY(widgetPtrToBefound)));

	FILE *stream = fopen(path, "w");
	if (stream != NULL)
	{
		fputs(text, stream);
		fclose(stream);
	}

	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
	gtk_widget_destroy(window);
}

void on_cancelbutton1_clicked(GtkButton *button, gpointer user_data)
{
	if (button){};
	
	GladeXML *gxml = user_data;
	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
	gtk_widget_destroy(window);
}

// Parse keyboard binds
gboolean on_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (user_data){};
		
	char *string		= XKeysymToString(XKeycodeToKeysym(GDK_DISPLAY(), event->hardware_keycode, 0));
	gchar *modifiers	= modifiers_to_string(event->state);
	gchar *keycode		= g_strdup_printf("%d", event->hardware_keycode);
	gchar *mkey		= g_strdup_printf("%s%s", modifiers, (string != NULL) ? string : keycode);

	gtk_entry_set_text(GTK_ENTRY(widget), mkey);

	g_free(mkey);
	g_free(keycode);
	g_free(modifiers);
		
	return FALSE;
}

gboolean on_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (user_data || event){};

	gchar **key_stat = g_strsplit(gtk_entry_get_text(GTK_ENTRY(widget)), "+", 4);

	if (is_correct_hotkey(key_stat) == -1)
		gtk_entry_set_text(GTK_ENTRY(widget), _("Press any key"));

	g_strfreev(key_stat);
	return FALSE;
}

void on_window2_delete_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	if (widget || event || user_data){};

	gtk_widget_destroy(widget);
	xneur_start();
}
