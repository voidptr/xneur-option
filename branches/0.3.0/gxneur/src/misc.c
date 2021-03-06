/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  (c) XNeur Team 2006
 *
 */

#include <gtk/gtk.h>

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "config_main.h"
#include "list.h"

#include "support.h"
#include "trayicon.h"
#include "interface.h"

#include "misc.h"

GtkWidget *wPreference;
GtkListStore *store;  
  
FILE *stream;

static gboolean xneur_start(gboolean is_sync)
{
	gboolean retval;

	if (is_sync == TRUE)
		retval = g_spawn_command_line_sync("xneur", NULL, NULL, NULL, NULL);
	else
		retval = g_spawn_command_line_async("xneur", NULL);

	if (retval != FALSE)
		return TRUE;

	fprintf(stderr, "ERROR: Couldn't start %s\nVerify that it installed\n", PROGRAM_NAME);
	return FALSE;
}

static gboolean xneur_stop(void)
{
	int xneur_pid = xneur_config_get_pid();
	if (xneur_pid == -1)
		return FALSE;

	if (kill(xneur_pid, SIGTERM) == -1)
		return FALSE;
	return TRUE;
}

void xneur_restart(void)
{
	if (xneur_stop() == TRUE)
		usleep(20000);

	xneur_start(FALSE);
}

void xneur_exit(void)
{
	xneur_stop();
	gtk_main_quit();

	exit(EXIT_SUCCESS);
}

void xneur_start_stop(GtkWidget *widget, struct _tray_icon *tray)
{
	if (widget){};
	if (xneur_stop() == TRUE)
	{
		create_tray_icon(tray, FALSE);
		return;
	}

	xneur_start(FALSE);
	create_tray_icon(tray, TRUE);
}

void xneur_about(void)
{ 
	GtkWidget *wAbout = create_window1();
	gtk_widget_show(wAbout);
}

void xneur_preference(void)
{
	GtkWidget *widgetPtrToBefound;
	wPreference = create_window2();
	gtk_widget_show (wPreference);
	
	// Init configuration
	struct _xneur_config *xconfig = xneur_config_init();
	
	// Load configuration
	error error_code = xconfig->load(xconfig);	
	if (error_code != ERROR_SUCCESS)
	{
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}		

	// Mode set
	widgetPtrToBefound = lookup_widget(wPreference, "combobox1");
	gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), xconfig->current_mode);
	
	// Exclude App set
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	GtkTreeIter	 iter;
	
	widgetPtrToBefound = lookup_widget(wPreference, "treeview1");
	
	store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (widgetPtrToBefound), GTK_TREE_MODEL (store));
	gtk_widget_show (widgetPtrToBefound);	

	int i;
	for (i = 0; i < xconfig->excluded_apps->data_count; i++)
	{
		gtk_list_store_append(GTK_LIST_STORE(store), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store), &iter, 0, xconfig->excluded_apps->data[i].string, -1);
	}				

	cell = gtk_cell_renderer_text_new ();					 
	
	column = gtk_tree_view_column_new_with_attributes (_("Приложение"), cell, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (widgetPtrToBefound), GTK_TREE_VIEW_COLUMN (column));
	
	// Adding/Removing App
	widgetPtrToBefound = lookup_widget(wPreference, "button2");
	
	g_signal_connect_swapped (G_OBJECT (widgetPtrToBefound), "clicked",
															G_CALLBACK (xneur_add_application),
															G_OBJECT (wPreference));
	widgetPtrToBefound = lookup_widget(wPreference, "button3");
	g_signal_connect_swapped (G_OBJECT (widgetPtrToBefound), "clicked",
															G_CALLBACK (xneur_rem_application),
															G_OBJECT (wPreference));
	// Keyboard Bind set
	// ChangeWord
	widgetPtrToBefound = lookup_widget(wPreference, "combobox3");
	if (xconfig->hotkeys[0].key == KEY_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);	
	}
	else if (xconfig->hotkeys[0].key == KEY_BREAK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);	
	}
	else if (xconfig->hotkeys[0].key == KEY_SCROLL_LOCK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);	
	}
	else if (xconfig->hotkeys[0].key == KEY_PRINT_SCREEN)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);	
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);
	}
	
	widgetPtrToBefound = lookup_widget(wPreference, "combobox10");
	if (xconfig->hotkeys[0].modifier == MODIFIER_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);
	}
	else if (xconfig->hotkeys[0].modifier == MODIFIER_SHIFT)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);
	}
	else if (xconfig->hotkeys[0].modifier == MODIFIER_CONTROL)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	
	
	// ChangeString
	widgetPtrToBefound = lookup_widget(wPreference, "combobox5");
	if (xconfig->hotkeys[1].key == KEY_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);	
	}
	else if (xconfig->hotkeys[1].key == KEY_BREAK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);	
	}
	else if (xconfig->hotkeys[1].key == KEY_SCROLL_LOCK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);	
	}
	else if (xconfig->hotkeys[1].key == KEY_PRINT_SCREEN)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);	
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);
	}
	
	widgetPtrToBefound = lookup_widget(wPreference, "combobox11");
	if (xconfig->hotkeys[1].modifier == MODIFIER_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);
	}
	else if (xconfig->hotkeys[1].modifier == MODIFIER_SHIFT)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);
	}
	else if (xconfig->hotkeys[1].modifier == MODIFIER_CONTROL)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	
	// ChangeMode
	widgetPtrToBefound = lookup_widget(wPreference, "combobox7");
	if (xconfig->hotkeys[2].key == KEY_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);	
	}
	else if (xconfig->hotkeys[2].key == KEY_BREAK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);	
	}
	else if (xconfig->hotkeys[2].key == KEY_SCROLL_LOCK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);	
	}
	else if (xconfig->hotkeys[2].key == KEY_PRINT_SCREEN)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);	
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);
	}
	
	widgetPtrToBefound = lookup_widget(wPreference, "combobox6");
	if (xconfig->hotkeys[2].modifier == MODIFIER_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);
	}
	else if (xconfig->hotkeys[2].modifier == MODIFIER_SHIFT)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);
	}
	else if (xconfig->hotkeys[2].modifier == MODIFIER_CONTROL)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	
	// ChangeSelected
	widgetPtrToBefound = lookup_widget(wPreference, "combobox9");
	if (xconfig->hotkeys[3].key == KEY_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);	
	}
	else if (xconfig->hotkeys[3].key == KEY_BREAK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);	
	}
	else if (xconfig->hotkeys[3].key == KEY_SCROLL_LOCK)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);	
	}
	else if (xconfig->hotkeys[3].key == KEY_PRINT_SCREEN)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);	
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 3);
	}
	
	widgetPtrToBefound = lookup_widget(wPreference, "combobox8");
	if (xconfig->hotkeys[3].modifier == MODIFIER_NONE)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 0);
	}
	else if (xconfig->hotkeys[3].modifier == MODIFIER_SHIFT)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 1);
	}
	else if (xconfig->hotkeys[3].modifier == MODIFIER_CONTROL)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	else 
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (widgetPtrToBefound), 2);
	}
	
	// Languages
	#define MAX_LANGUAGES 4
	const char *file_choose_buttons[MAX_LANGUAGES] = {"filechooserbutton1", "filechooserbutton2", "filechooserbutton3", "filechooserbutton4"};
	const char *language_name_entries[MAX_LANGUAGES] = {"entry6", "entry7", "entry8", "entry9"};
	const char *language_combo_boxes[MAX_LANGUAGES] = {"combobox13", "combobox14", "combobox15", "combobox16"};

	int lang;
	for (lang = 0; lang < MAX_LANGUAGES; lang++)
	{
		GtkWidget *dir = lookup_widget(wPreference, file_choose_buttons[lang]);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dir), "/");
	}

	for (lang = 0; lang < xconfig->total_languages && lang < MAX_LANGUAGES; lang++)
	{
		GtkWidget *dir = lookup_widget(wPreference, file_choose_buttons[lang]);
		GtkWidget *name = lookup_widget(wPreference, language_name_entries[lang]);
		widgetPtrToBefound = lookup_widget(wPreference, language_combo_boxes[lang]);

		gtk_combo_box_set_active(GTK_COMBO_BOX(widgetPtrToBefound), xconfig->get_lang_group(xconfig, lang));
		gtk_entry_set_text(GTK_ENTRY(name), xconfig->get_lang_name(xconfig, lang));

		char *lang_dir = xconfig->get_lang_dir_full(xconfig, lang);

		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dir), lang_dir);

		free(lang_dir);
	}
	
	// Button OK
	widgetPtrToBefound = lookup_widget(wPreference, "button5");
	g_signal_connect_swapped((gpointer)widgetPtrToBefound, "clicked", G_CALLBACK(xneur_save_preference), GTK_OBJECT(widgetPtrToBefound));
}

void xneur_add_application(void)
{
	GtkTreeIter	 iter;
	FILE* fp = popen("xprop WM_CLASS", "r");
	char buffer[NAME_MAX];
	const char* cap = "WM_CLASS(STRING)";
	
	if (fp == NULL)
		return;

	if (fgets(buffer, NAME_MAX, fp) == NULL)
		return;

	if (fclose(fp))
		return;

	if (strncmp(buffer, cap, strlen(cap)) != 0)
		return;

	char* p = strstr(buffer, "\", \"");
	if (p == NULL)
		return;
	if (strlen(p += 4) < 2)
		return;
	p[strlen(p) - 2] = '\0';
	
	gtk_list_store_append(GTK_LIST_STORE(store), &iter);
	gtk_list_store_set (GTK_LIST_STORE(store), &iter, 0, p, -1);
}

void xneur_rem_application(void)
{
	GtkTreeIter	 iter;
	GtkTreeSelection *select;	
	GtkTreeModel *model = GTK_TREE_MODEL (store);
	GtkWidget *treeview1;
	treeview1 = lookup_widget(wPreference, "treeview1");
	
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview1));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	if (gtk_tree_selection_get_selected (select, &model, &iter))				
		gtk_list_store_remove(GTK_LIST_STORE(store), &iter);
}
	
gboolean save_app(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	gchar *ptr;
	if (model || path || user_data){};
	gtk_tree_model_get(GTK_TREE_MODEL(store), iter, 0, &ptr, -1);

	fputs("ExcludeApp ", stream); 
	fputs(ptr, stream); 
	fputs("\n", stream);

	g_free(ptr);

	return FALSE;
}

gchar *get_last_file_from_path(gchar *path)
{
	unsigned int i;
	int last = 0;
	for (i=0; i<strlen(path); i++)
	{
		if (path[i] == '/') 
		{
			last = i;
		}
	}
	return path + last + 1;
}

void xneur_save_preference(void)
{
	char *config_file = xneur_config_get_config_file_name();

	char *ptr;
	char *modifier="";
	char *key="";
	GtkWidget *widgetPtrToBefound1;
	GtkWidget *widgetPtrToBefound2;
	GtkWidget *widgetPtrToBefound3;
	if ((stream = fopen (config_file, "w")) != NULL) 
	{
		fputs("# It's a X Neural Switcher configuration file by Gxneur\n# All values writted gXNeur\n\n", stream);
		
		fputs("# Default work mode\n", stream);	
		fputs("DefaultMode ", stream);
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox1");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
			ptr = g_strdup_printf("%s\n\n", "Auto");
		else
			ptr = g_strdup_printf("%s\n\n", "Manual");
		fputs(ptr, stream); 
		
		fputs("# Level of message what program can output\n", stream);
		fputs("# Example:\n", stream);
		fputs("#LogLevel Error\n", stream);
		fputs("#LogLevel Log\n", stream);
		fputs("LogLevel Debug\n\n", stream);	
		
		fputs("# Define used languages\n", stream);
		fputs("# See Settings page on http://www.xneur.ru for details\n", stream);
		
		widgetPtrToBefound1 = lookup_widget(wPreference, "entry6");
		widgetPtrToBefound2 = lookup_widget(wPreference, "filechooserbutton1");
		widgetPtrToBefound3 = lookup_widget(wPreference, "combobox13");
		if ((strlen(gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1))) != 0) && 
			(strlen(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))) > 1))
		{
			ptr = g_strdup_printf("AddLanguage %s %s\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), get_last_file_from_path(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))));
			fputs(ptr, stream);
			ptr = g_strdup_printf("SetXkbGroup %s %d\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound3)));
			fputs(ptr, stream);
		}
		widgetPtrToBefound1 = lookup_widget(wPreference, "entry7");
		widgetPtrToBefound2 = lookup_widget(wPreference, "filechooserbutton2");
		widgetPtrToBefound3 = lookup_widget(wPreference, "combobox14");
		if ((strlen(gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1))) != 0) && 
			(strlen(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))) > 1))

		{
			ptr = g_strdup_printf("AddLanguage %s %s\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), get_last_file_from_path(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))));
			fputs(ptr, stream);
			ptr = g_strdup_printf("SetXkbGroup %s %d\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound3)));
			fputs(ptr, stream);
		}
		widgetPtrToBefound1 = lookup_widget(wPreference, "entry8");
		widgetPtrToBefound2 = lookup_widget(wPreference, "filechooserbutton3");
		widgetPtrToBefound3 = lookup_widget(wPreference, "combobox15");
		if ((strlen(gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1))) != 0) && 
			(strlen(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))) > 1))
		{
			ptr = g_strdup_printf("AddLanguage %s %s\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), get_last_file_from_path(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))));
			fputs(ptr, stream);
			ptr = g_strdup_printf("SetXkbGroup %s %d\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound3)));
			fputs(ptr, stream);
		}
		widgetPtrToBefound1 = lookup_widget(wPreference, "entry9");
		widgetPtrToBefound2 = lookup_widget(wPreference, "filechooserbutton4");
		widgetPtrToBefound3 = lookup_widget(wPreference, "combobox16");
		if ((strlen(gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1))) != 0) &&
			(strlen(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))) > 1))
		{
			ptr = g_strdup_printf("AddLanguage %s %s\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), get_last_file_from_path(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound2))));
			fputs(ptr, stream);
			ptr = g_strdup_printf("SetXkbGroup %s %d\n", gtk_entry_get_text (GTK_ENTRY (widgetPtrToBefound1)), gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound3)));
			fputs(ptr, stream);
		}
		fputs("\n", stream);	
		
		fputs("# Add Applications names to exclude it from procces with xneur\n", stream);
		fputs("# Only for applications, what's window names don't exist in this list xneur will process the input\n", stream);
		fputs("# Example:\n", stream);
		fputs("#ExcludeApp Gaim\n", stream);
		gtk_tree_model_foreach(GTK_TREE_MODEL(store), save_app, NULL);
		fputs("\n", stream);
		
		fputs("# Binds hotkeys for some actions\n", stream);
		fputs("AddBind ", stream); 	
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox3");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
		{
			key = g_strdup_printf("None");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1) 
		{
			key = g_strdup_printf("Break");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 2)
		{
			key = g_strdup_printf("ScrollLock");
		}	
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 3) 
		{
			key = g_strdup_printf("PrtSc");
		}	
		
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox10");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0) 
		{
			modifier = g_strdup_printf("None");
		}	
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1)
		{
			modifier = g_strdup_printf("Shift");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 2)
			modifier = g_strdup_printf("Ctrl");	
		
		ptr = g_strdup_printf("ChangeWord %s %s", modifier, key);								
		fputs(ptr, stream); 
		fputs("\n", stream);
		
		
		fputs("AddBind ", stream); 
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox5");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
		{
			key = g_strdup_printf("None");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1) 
		{
			key = g_strdup_printf("Break");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 2)
		{
			key = g_strdup_printf("ScrollLock");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 3)
		{
			key = g_strdup_printf("PrtSc");
		}
		
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox11");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
		{
			modifier = g_strdup_printf("None");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1)
		{
			modifier = g_strdup_printf("Shift");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 2)
		{
			modifier = g_strdup_printf("Ctrl");	
		}
		
		ptr = g_strdup_printf("ChangeString %s %s", modifier, key);								
		fputs(ptr, stream); 
		fputs("\n", stream);
		
		
		fputs("AddBind ", stream); 
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox7");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
		{
			key = g_strdup_printf("None");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1)
		{
			key = g_strdup_printf("Break");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 2)
		{
			key = g_strdup_printf("ScrollLock");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 3) 
		{
			key = g_strdup_printf("PrtSc");
		}
		
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox6");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
		{
			modifier = g_strdup_printf("None");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1)
		{
			modifier = g_strdup_printf("Shift");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 2)
		{
			modifier = g_strdup_printf("Ctrl");	
		}
		ptr = g_strdup_printf("ChangeMode %s %s", modifier, key);									
		fputs(ptr, stream); 
		fputs("\n", stream);
		
		
		fputs("AddBind ", stream); 
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox9");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
		{
			key = g_strdup_printf("None");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1)
		{
			key = g_strdup_printf("Break");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 2)
		{
			key = g_strdup_printf("ScrollLock");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 3)
		{
			key = g_strdup_printf("PrtSc");
		}
		
		widgetPtrToBefound1 = lookup_widget(wPreference, "combobox8");
		if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 0)
		{
			modifier = g_strdup_printf("None");
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1)
		{
			modifier = g_strdup_printf("Shift");	
		}
		else if (gtk_combo_box_get_active (GTK_COMBO_BOX(widgetPtrToBefound1)) == 1)
		{
			modifier = g_strdup_printf("Ctrl");	
		}
		
		ptr = g_strdup_printf("ChangeSelected %s %s", modifier, key);									
		fputs(ptr, stream); 
		fputs("\n\n", stream);		
		
		fputs("# It's all\n", stream);
		
		fclose (stream);
	}

	xneur_restart();
	gtk_widget_destroy(wPreference);
}

static char* get_file_content(const char *file_name)
{
	struct stat sb;

	if (stat(file_name, &sb) != 0 || sb.st_size < 0)
		return NULL;

	FILE *stream = fopen(file_name, "r");
	if (stream == NULL)
		return NULL;

	unsigned int file_len = sb.st_size;

	char *content = (char *) malloc((file_len + 2) * sizeof(char)); // + 1 '\n' + 1 '\0'
	if (fread(content, 1, file_len, stream) != file_len)
	{
		free(content);
		fclose(stream);
		return NULL;
	}

	content[file_len] = '\n';
	content[file_len + 1] = '\0';
	fclose(stream);

	return content;
}
	
gchar *xneur_get_dict_path(int layout_no)
{
	GtkWidget *widgetPtrToBefound = NULL;
	switch (layout_no)
	{
		case 1: 
		{
			widgetPtrToBefound = lookup_widget(wPreference, "filechooserbutton1");			
			break;
		}
		case 2:
		{
			widgetPtrToBefound = lookup_widget(wPreference, "filechooserbutton2");
			break;
		}
		case 3:
		{
			widgetPtrToBefound = lookup_widget(wPreference, "filechooserbutton3");
			break;
		}
		case 4:
		{
			widgetPtrToBefound = lookup_widget(wPreference, "filechooserbutton4");
			break;
		}
	}
	char *path = malloc ((strlen(gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound))) + 5) * sizeof(char));
	path = g_strdup_printf("%s/%s", gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widgetPtrToBefound)), "dict");
	return path;
}

gchar *xneur_get_dict(int layout_no)
{
	char *path = xneur_get_dict_path(layout_no);

	char *content = get_file_content(path);

	free(path);

	return content;
}
