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
#include <gdk/gdkx.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>

#include <xneur/xnconfig.h>
#include <xneur/list_char.h>

#define GLADE_FILE_ABOUT PACKAGE_GLADE_FILE_DIR"/about.glade"
#define GLADE_FILE_CONFIG PACKAGE_GLADE_FILE_DIR"/config.glade"
#define GLADE_FILE_ABBREVIATION_ADD PACKAGE_GLADE_FILE_DIR"/abbr_add.glade"
#define GLADE_FILE_CHOOSE PACKAGE_GLADE_FILE_DIR"/choose_file.glade"
#define GLADE_FILE_ACTION_ADD PACKAGE_GLADE_FILE_DIR"/action_add.glade"

#define LANGUAGES_DIR "languages"
#define DIR_SEPARATOR		"/"

#include "support.h"
#include "callbacks.h"
#include "trayicon.h"

#include "misc.h"

#define MAX_LANGUAGES			4
#define XNEUR_NEEDED_MAJOR_VERSION	9
#define XNEUR_BUILD_MINOR_VERSION	6
	
struct _xneur_config *xconfig				= NULL;
	
static GtkListStore *store_exclude_app		= NULL;
static GtkListStore *store_auto_app			= NULL;
static GtkListStore *store_manual_app		= NULL;
static GtkListStore *store_layout_app		= NULL;
static GtkListStore *store_draw_flag_app	= NULL;
static GtkListStore *store_abbreviation		= NULL;
static GtkListStore *store_sound			= NULL;
static GtkListStore *store_osd				= NULL;
static GtkListStore *store_popup			= NULL;
static GtkListStore *store_action			= NULL;
static GtkListStore *store_hotkey			= NULL;
static GtkListStore *store_autocomplementation_exclude_app		= NULL;

static GtkTreeView *tmp_treeview	= NULL;

static const char *modifier_names[]			= {"Shift", "Control", "Alt", "Super"};
static const char *all_modifiers[]			= {"Control", "Shift", "Alt", "Super", "Control_R", "Shift_R", "Alt_R", "Super_R", "Control_L", "Shift_L", "Alt_L", "Super_L"};
static const char *language_names[]			= {"Belarusian", 
	                                           "Bulgarian",
	                                           "Czech",
	                                           "German",
                                               "Greek",
                                               "English",
                                               "Spanish",
	                                           "Estonian",
                                               "French",
                                               "Armenian",
                                               "Kazakh",
                                               "Lithuanian",
                                               "Latvian",
                                               "Poland",
                                               "Romanian",
                                               "Russian", 
                                               "Ukrainian",
                                               "Uzbek"};

static char *dirs[]					= {"be", 
	                                   "bg", 
	                                   "cs", 
	                                   "de",
                                       "el",
                                       "en",
                                       "es",
	                                   "et", 
	                                   "fr",
                                       "hy",
	                                   "kk", 
	                                   "lt", 
	                                   "lv", 
	                                   "pl", 
	                                   "ro", 
	                                   "ru", 
	                                   "uk", 
	                                   "uz"};

static const char *notify_names[]			=   {
										"Xneur started", "Xneur reloaded", "Xneur stopped",
										"Keypress on Layout 1", "Keypress on Layout 2", "Keypress on Layout 3",
										"Keypress on Layout 4", "Switch to Layout 1", "Switch to Layout 2",
										"Switch to Layout 3", "Switch to Layout 4", "Correct word automatically",
										"Correct word manually", "Correct last line", "Correct selected text",
										"Transliterate selected text", "Change case of selected text", "Calculate formula on selected text",
										"Correct clipboard text", "Transliterate clipboard text", "Change case of clipboard text", "Calculate formula on clipboard text",
	                                    "Expand abbreviations",
										"Correct aCCIDENTAL caps", "Correct TWo INitial caps", "Execute user action"
										};

static const char *hotkey_names[]			=   {
										"Correct/Undo correction", "Correct last line", "Switch between processing modes", 
										"Correct selected text", "Transliterate selected text", "Change case of selected text", "Calculate formula on selected text",
	                                    "Correct clipboard text", "Transliterate clipboard text", "Change case of clipboard text","Calculate formula on clipboard text",
										"Switch to layout 1", "Switch to layout 2", "Switch to layout 3", "Switch to layout 4",
		                                "Rotate layouts", "Expand abbreviations", "Autocomplementation confirmation"
                                        };

static const int total_notify_names = sizeof(notify_names) / sizeof(notify_names[0]);


static const char *language_name_boxes[MAX_LANGUAGES]	= {"combobox21", "combobox22", "combobox23", "combobox24"};
static const char *language_combo_boxes[MAX_LANGUAGES]	= {"combobox13", "combobox14", "combobox15", "combobox16"};
static const char *language_fix_boxes[MAX_LANGUAGES]	= {"checkbutton14", "checkbutton15", "checkbutton16", "checkbutton17"};

static const int total_modifiers			= sizeof(modifier_names) / sizeof(modifier_names[0]); 
static const int total_all_modifiers			= sizeof(all_modifiers) / sizeof(all_modifiers[0]);
static const int total_languages			= sizeof(language_names) / sizeof(language_names[0]);

static void error_msg(const char *msg, ...)
{
	int len = strlen(msg) + 2;

	va_list ap;
	va_start(ap, msg);
	
	char *buffer = (char *) malloc(1024);
	vsprintf(buffer, msg, ap);
	buffer[len] = 0;

	GtkWidget *dialog = gtk_message_dialog_new (NULL,
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR,
											GTK_BUTTONS_CLOSE,
											"%s", buffer);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);	
	
	free(buffer);
	va_end(ap);
}

static char* concat_bind(int action)
{
	char *text = (char *) malloc((24 + 1 + strlen(xconfig->hotkeys[action].key)) * sizeof(char));
	text[0] = '\0';

	for (int i = 0; i < total_modifiers; i++)
	{
		if ((xconfig->hotkeys[action].modifiers & (0x1 << i)) == 0)
			continue;

		strcat(text, modifier_names[i]);
		strcat(text, "+");
	}

	strcat(text, xconfig->hotkeys[action].key);
	
	return text;
}

static void split_bind(char *text, int action)
{
	char **key_stat = g_strsplit(text, "+", 4);

	int last = is_correct_hotkey(key_stat);
	if (last == -1)
	{
		g_strfreev(key_stat);
		return;
	}
	
	xconfig->hotkeys[action].modifiers = 0;
	
	for (int i = 0; i <= last; i++)
	{
		int assigned = FALSE;
		for (int j = 0; j < total_modifiers; j++) 
 		{ 
			if (g_strcasecmp(key_stat[i], modifier_names[j]) != 0) 
				continue; 

			assigned = TRUE;
			xconfig->hotkeys[action].modifiers |= (0x1 << j); 
			break; 
		} 

		if (assigned == FALSE)
			xconfig->hotkeys[action].key = strdup(key_stat[i]); 
	}

	g_strfreev(key_stat);
}

static void add_item(GtkListStore *store)
{
	FILE *fp = popen("xprop WM_CLASS", "r");
	if (fp == NULL)
		return;

	char buffer[NAME_MAX];
	if (fgets(buffer, NAME_MAX, fp) == NULL)
		return;
	
	if (pclose(fp))
		return;

	const char* cap = "WM_CLASS(STRING)";
	if (strncmp(buffer, cap, strlen(cap)) != 0)
		return;

	char* p = strstr(buffer, "\", \"");
	if (p == NULL)
		return;

	p += 4;

	int len = strlen(p);
	if (len < 2)
		return;

	p[len - 2] = '\0';
	
	GtkTreeIter iter;
	gtk_list_store_append(GTK_LIST_STORE(store), &iter);
	gtk_list_store_set(GTK_LIST_STORE(store), &iter, 0, p, -1);
}

static void remove_item(GtkWidget *treeview, GtkListStore *store)
{
	GtkTreeModel *model = GTK_TREE_MODEL(store);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
		gtk_list_store_remove(GTK_LIST_STORE(store), &iter);
}

static void save_list(GtkListStore *store, struct _list_char *list, GtkTreeIter *iter)
{
	gchar *ptr;
	gtk_tree_model_get(GTK_TREE_MODEL(store), iter, 0, &ptr, -1);
	
	list->add(list, ptr);

	g_free(ptr);
}

static char* get_dir_by_index(int index)
{
	if (index == 0)
		return NULL;
	
	int dir_len = strlen(LANGUAGES_DIR) + strlen(DIR_SEPARATOR) + strlen(dirs[index - 1]) + 1;
	char *dir = (char *) malloc(dir_len * sizeof(char));
	snprintf(dir, dir_len, "%s%s%s", LANGUAGES_DIR, DIR_SEPARATOR, dirs[index - 1]);
	return dir;
}

static const char* get_lang_by_index(int index)
{
	if (index == 0)
		return NULL;

	return language_names[index - 1];
}

static const char* get_lang_code_by_index(int index)
{
	if (index == 0)
		return NULL;
	
	return dirs[index - 1];
}

static void init_libxnconfig(void)
{
	if (xconfig != NULL)
		return;

	// Init configuration
	xconfig = xneur_config_init();

	int major_version, minor_version;
	xconfig->get_library_version(&major_version, &minor_version);

	if ((major_version != XNEUR_NEEDED_MAJOR_VERSION) || (minor_version != XNEUR_BUILD_MINOR_VERSION))
	{
		error_msg(_("Wrong XNeur configuration library api version.\nPlease, install libxnconfig version %d.0.%d\n"), XNEUR_NEEDED_MAJOR_VERSION, XNEUR_BUILD_MINOR_VERSION);
		printf(_("Wrong XNeur configuration library api version.\nPlease, install libxnconfig version %d.0.%d\n"), XNEUR_NEEDED_MAJOR_VERSION, XNEUR_BUILD_MINOR_VERSION);
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}

	//error_msg(_("Using libxnconfig API version %d.0.%d (build with %d.%d)\n"), major_version, minor_version, XNEUR_NEEDED_MAJOR_VERSION, XNEUR_BUILD_MINOR_VERSION);
	printf(_("Using libxnconfig API version %d.0.%d (build with %d.0.%d)\n"), major_version, minor_version, XNEUR_NEEDED_MAJOR_VERSION, XNEUR_BUILD_MINOR_VERSION);

	if (!xconfig->load(xconfig))
	{
		error_msg(_("XNeur config broken!\nPlease, remove ~/.xneur/xneurrc and reinstall XNeur package!\n"));
		printf(_("XNeur config broken!\nPlease, remove ~/.xneur/xneurrc and reinstall XNeur package!\n"));
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}
}

void xneur_start(void)
{
	init_libxnconfig();

	if (!g_spawn_command_line_async("xneur", NULL))
	{
		error_msg(_("Couldn't start xneur\nVerify that it installed\n"));
		fprintf(stderr, _("Couldn't start xneur\nVerify that it installed\n"));
	}
}

void xneur_exit(void)
{
	xconfig->kill(xconfig);
	gtk_main_quit();
}

void xneur_start_stop(GtkWidget *widget, struct _tray_icon *tray)
{
	if (widget){};

	if (xconfig->kill(xconfig) == TRUE)
	{
		create_tray_icon(tray, FALSE);
		return;
	}

	xneur_start();
	create_tray_icon(tray, TRUE);
}

void xneur_auto_manual(GtkWidget *widget, struct _tray_icon *tray)
{
	if (widget || tray){};

	xconfig->set_manual_mode(xconfig, !xconfig->is_manual_mode(xconfig));
}

void xneur_about(void)
{ 
	GladeXML *gxml = glade_xml_new (GLADE_FILE_ABOUT, NULL, NULL);
	
	glade_xml_signal_autoconnect (gxml);
	GtkWidget *window = glade_xml_get_widget (gxml, "window1");

	GdkPixbuf *window_icon_pixbuf = create_pixbuf ("gxneur.png");
	if (window_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
		
		GtkWidget *image = glade_xml_get_widget (gxml, "image1");	
		gtk_image_set_from_pixbuf (GTK_IMAGE (image), window_icon_pixbuf);
		
		gdk_pixbuf_unref (window_icon_pixbuf);
	}
	
	GtkWidget *widget = glade_xml_get_widget(gxml, "label44");
	gchar *text = g_strdup_printf("%s %s", _("Current Version"), VERSION);
	gtk_label_set_text(GTK_LABEL(widget), text);	

	gtk_widget_show(window);
}

static void column_0_edited (GtkCellRendererText *renderer, gchar *path, gchar *new_text, GtkTreeView *treeview)
{
	if (renderer) {};
	GtkTreeIter iter;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model (treeview);
	if (gtk_tree_model_get_iter_from_string (model, &iter, path))
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, new_text, -1);
}

static void column_1_edited (GtkCellRendererText *renderer, gchar *path, gchar *new_text, GtkTreeView *treeview)
{
	if (renderer) {};
	GtkTreeIter iter;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model (treeview);
	if (gtk_tree_model_get_iter_from_string (model, &iter, path))
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 1, new_text, -1);
}

void xneur_preference(void)
{
	GladeXML *gxml = glade_xml_new (GLADE_FILE_CONFIG, NULL, NULL);
	
	glade_xml_signal_autoconnect (gxml);
	GtkWidget *window = glade_xml_get_widget (gxml, "window2");

	GdkPixbuf *window_icon_pixbuf = create_pixbuf ("gxneur.png");
	if (window_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
		gdk_pixbuf_unref (window_icon_pixbuf);
	}
	
	gtk_widget_show(window);

	// Mode set
	GtkWidget *widget = glade_xml_get_widget (gxml, "checkbutton7");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->manual_mode);

	// Exclude App set
	GtkWidget *treeview = glade_xml_get_widget (gxml, "treeview1");

	store_exclude_app = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_exclude_app));
	gtk_widget_show(treeview);

	for (int i = 0; i < xconfig->excluded_apps->data_count; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_exclude_app), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_exclude_app), &iter, 0, xconfig->excluded_apps->data[i].string, -1);
	}

	GtkCellRenderer *cell = gtk_cell_renderer_text_new();

	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(_("Application"), cell, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Adding/Removing Exclude App
	widget = glade_xml_get_widget (gxml, "button2");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_add_exclude_app), G_OBJECT(window));

	widget = glade_xml_get_widget (gxml, "button3");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_rem_exclude_app), G_OBJECT(treeview));

	// Auto App Set
	treeview = glade_xml_get_widget (gxml, "treeview2");

	store_auto_app = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_auto_app));
	gtk_widget_show(treeview);

	for (int i = 0; i < xconfig->auto_apps->data_count; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_auto_app), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_auto_app), &iter, 0, xconfig->auto_apps->data[i].string, -1);
	}

	cell = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new_with_attributes(_("Application"), cell, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Adding/Removing Auto App
	widget = glade_xml_get_widget (gxml, "button19");

	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_add_auto_app), G_OBJECT(window));
	widget = glade_xml_get_widget (gxml, "button20");

	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_rem_auto_app), G_OBJECT(treeview));

	// Manual App Set
	treeview = glade_xml_get_widget (gxml, "treeview3");
	
	store_manual_app = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_manual_app));
	gtk_widget_show(treeview);	

	for (int i = 0; i < xconfig->manual_apps->data_count; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_manual_app), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_manual_app), &iter, 0, xconfig->manual_apps->data[i].string, -1);
	}				

	cell = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new_with_attributes(_("Application"), cell, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Adding/Removing Manual App
	widget = glade_xml_get_widget (gxml, "button21");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_add_manual_app), G_OBJECT(window));

	widget = glade_xml_get_widget (gxml, "button22");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_rem_manual_app), G_OBJECT(treeview));

	// Layout Remember App Set
	treeview = glade_xml_get_widget (gxml, "treeview4");
	
	store_layout_app = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_layout_app));
	gtk_widget_show(treeview);	

	for (int i = 0; i < xconfig->layout_remember_apps->data_count; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_layout_app), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_layout_app), &iter, 0, xconfig->layout_remember_apps->data[i].string, -1);
	}				

	cell = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new_with_attributes(_("Application"), cell, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Adding/Removing Layour Remember App
	widget = glade_xml_get_widget (gxml, "button27");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_add_layout_app), G_OBJECT(window));

	widget = glade_xml_get_widget (gxml, "button28");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_rem_layout_app), G_OBJECT(treeview));
	
	// Languages
	for (int lang = 0; lang < xconfig->total_languages && lang < MAX_LANGUAGES; lang++)
	{
		GtkWidget *name = glade_xml_get_widget (gxml,  language_name_boxes[lang]);
		widget = glade_xml_get_widget (gxml, language_combo_boxes[lang]);

		gtk_combo_box_set_active(GTK_COMBO_BOX(widget), xconfig->get_lang_group(xconfig, lang));

		const char *lang_name = xconfig->get_lang_name(xconfig, lang);
		for (int j = 0; j < total_languages; j++)
		{
			if (strcmp(lang_name, language_names[j]) != 0)
				continue;

			gtk_combo_box_set_active(GTK_COMBO_BOX(name), j + 1);
			break;
		}
		
		widget = glade_xml_get_widget (gxml, language_fix_boxes[lang]);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->languages[lang].fixed);
	}
		
	// Default Layout Group
	widget = glade_xml_get_widget (gxml, "combobox25");
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), xconfig->default_group);
	
	// Education Mode
	widget = glade_xml_get_widget (gxml, "checkbutton2");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->educate);
	
	// Layout Remember Mode
	widget = glade_xml_get_widget (gxml, "checkbutton3");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->remember_layout);
	
	// Saving Selection Mode
	widget = glade_xml_get_widget (gxml, "checkbutton4");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->save_selection);
	
	// Sound Playing Mode
	widget = glade_xml_get_widget (gxml, "checkbutton5");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->play_sounds);
	
	// Logging Keyboard Mode
	widget = glade_xml_get_widget (gxml, "checkbutton6");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->save_keyboard_log);
	
	// Ignore Keyboard Layout Mode
	widget = glade_xml_get_widget (gxml, "checkbutton8");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->abbr_ignore_layout);

	// Check language on input process 
	widget = glade_xml_get_widget (gxml, "checkbutton18");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->check_lang_on_process);

	// Disable CapsLock use 
	widget = glade_xml_get_widget (gxml, "checkbutton19");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->disable_capslock);

	// 
	widget = glade_xml_get_widget (gxml, "checkbutton20");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->correct_space_with_punctuation);

	// Autocomplementation
	widget = glade_xml_get_widget (gxml, "checkbutton21");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->autocomplementation);

	// Space after autocomplementation
	widget = glade_xml_get_widget (gxml, "checkbutton1");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->add_space_after_autocomplementation);

	// Exclude autocomplementation App set
	treeview = glade_xml_get_widget (gxml, "treeview8");

	store_autocomplementation_exclude_app = gtk_list_store_new(1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_autocomplementation_exclude_app));
	gtk_widget_show(treeview);

	for (int i = 0; i < xconfig->autocomplementation_excluded_apps->data_count; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_autocomplementation_exclude_app), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_autocomplementation_exclude_app), &iter, 0, xconfig->autocomplementation_excluded_apps->data[i].string, -1);
	}

	cell = gtk_cell_renderer_text_new();

	column = gtk_tree_view_column_new_with_attributes(_("Application"), cell, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Adding/Removing Exclude App
	widget = glade_xml_get_widget (gxml, "button11");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_add_autocomplementation_exclude_app), G_OBJECT(window));

	widget = glade_xml_get_widget (gxml, "button13");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_rem_autocomplementation_exclude_app), G_OBJECT(treeview));

	// Hotkeys List set
	treeview = glade_xml_get_widget (gxml, "treeview5");

	store_hotkey = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_hotkey));
	gtk_widget_show(treeview);

	hotkey_names[0] = hotkey_names[0];
	for (int i = 0; i < MAX_HOTKEYS; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_hotkey), &iter);

		char *binds = "";
		if (xconfig->hotkeys[i].key != NULL)
			binds = concat_bind(i);

		gtk_list_store_set(GTK_LIST_STORE(store_hotkey), &iter, 
												0, _(hotkey_names[i]),
												1, binds, 
												-1);
	}

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Action"), cell, "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Key bind"), cell, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Button Edit Action
	widget = glade_xml_get_widget (gxml, "button10");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_edit_action), G_OBJECT(treeview));

	// Button Clear Action
	widget = glade_xml_get_widget (gxml, "button1");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_clear_action), G_OBJECT(treeview));
	
	// Abbreviations List set
	treeview = glade_xml_get_widget (gxml, "treeview6");

	store_abbreviation = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_abbreviation));
	gtk_widget_show(treeview);

	for (int i = 0; i < xconfig->abbreviations->data_count; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_abbreviation), &iter);
		char *string		= strdup(xconfig->abbreviations->data[i].string);
		char *replacement	= strsep(&string, " ");
		gtk_list_store_set(GTK_LIST_STORE(store_abbreviation), &iter, 
												0, replacement,
												1, string, 
												-1);
		free(replacement);
	}

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Abbreviation"), cell, "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	g_object_set (cell, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (cell), "edited",
						G_CALLBACK (column_0_edited),
						(gpointer) treeview);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Expansion text"), cell, "text", 1, NULL);
	g_object_set (cell, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (cell), "edited",
						G_CALLBACK (column_1_edited),
						(gpointer) treeview);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Button Add Abbreviation
	widget = glade_xml_get_widget (gxml, "button32");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_add_abbreviation), G_OBJECT(treeview));
	
	// Button Remove Abbreviation
	widget = glade_xml_get_widget (gxml, "button33");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_rem_abbreviation), G_OBJECT(treeview));

	// Sound Paths Preference
	// Sound List set
	treeview = glade_xml_get_widget (gxml, "treeview7");

	store_sound = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_sound));
	gtk_widget_show(treeview);

	for (int i = 0; i < total_notify_names; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_sound), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_sound), &iter, 
												0, _(notify_names[i]),
												1, xconfig->sounds[i].file, 
												-1);
	}
	
	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Action"), cell, "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Sound"), cell, "text", 1, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	g_object_set (cell, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (cell), "edited",
						G_CALLBACK (column_1_edited),
						(gpointer) treeview);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Button Edit Sound
	widget = glade_xml_get_widget (gxml, "button34");
	tmp_treeview = GTK_TREE_VIEW(treeview);
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_edit_sound), G_OBJECT(treeview));

	// Set Callbacks for Dict and Regexp
	widget= glade_xml_get_widget (gxml, "button8");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button8_clicked), gxml);
	widget = glade_xml_get_widget (gxml, "button9");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button9_clicked), gxml);
	widget = glade_xml_get_widget (gxml, "button7");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button7_clicked), gxml);
	widget = glade_xml_get_widget (gxml, "button6");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button6_clicked), gxml);
	widget = glade_xml_get_widget (gxml, "button23");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button23_clicked), gxml);
	widget = glade_xml_get_widget (gxml, "button24");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button24_clicked), gxml);
	widget = glade_xml_get_widget (gxml, "button25");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button25_clicked), gxml);
	widget = glade_xml_get_widget (gxml, "button26");
	g_signal_connect ((gpointer) widget, "clicked", G_CALLBACK (on_button26_clicked), gxml);
	
	// Delay Before Send
	widget = glade_xml_get_widget (gxml, "spinbutton1");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), xconfig->send_delay);
		
	// Log Level
	widget = glade_xml_get_widget (gxml, "combobox1");
	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), xconfig->log_level);
	
	// Correct two capital letter mode
	widget = glade_xml_get_widget (gxml, "checkbutton10");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->correct_two_capital_letter);

	// Correct iNCIDENTAL CapsLock mode
	widget = glade_xml_get_widget (gxml, "checkbutton9");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->correct_incidental_caps);

	// Flush internal buffer when pressed Enter mode
	widget = glade_xml_get_widget (gxml, "checkbutton11");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->flush_buffer_when_press_enter);

	// Don't process word when pressed Enter mode
	widget = glade_xml_get_widget (gxml, "checkbutton12");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->dont_process_when_press_enter);

	// User Actions List set
	treeview = glade_xml_get_widget (gxml, "treeview9");

	store_action = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_action));
	gtk_widget_show(treeview);

	for (int action = 0; action < xconfig->actions_count; action++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_action), &iter);
		
		char *text = (char *) malloc((24 + 1 + strlen(xconfig->actions[action].hotkey.key)) * sizeof(char));
		text[0] = '\0';

		for (int i = 0; i < total_modifiers; i++)
		{
			if ((xconfig->actions[action].hotkey.modifiers & (0x1 << i)) == 0)
				continue;

			strcat(text, modifier_names[i]);
			strcat(text, "+");
		}

		strcat(text, xconfig->actions[action].hotkey.key);
		
		gtk_list_store_set(GTK_LIST_STORE(store_action), &iter, 
												0, xconfig->actions[action].command,
												1, text, 
												-1);
		free(text);
	}

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("User action"), cell, "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Key bind"), cell, "text", 1, NULL);
	g_object_set (cell, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (cell), "edited",
						G_CALLBACK (column_1_edited),
						(gpointer) treeview);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Button Add User Action
	widget = glade_xml_get_widget (gxml, "button36");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_add_user_action), G_OBJECT(treeview));
	
	// Button Remove User Action
	widget = glade_xml_get_widget (gxml, "button37");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_rem_user_action), G_OBJECT(treeview));

	// Button Edit User Action
	widget = glade_xml_get_widget (gxml, "button38");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_edit_user_action), G_OBJECT(treeview));
			
	// Show OSD
	widget = glade_xml_get_widget (gxml, "checkbutton13");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->show_osd);
	
	// OSD Text Preference
	// OSD List set
	treeview = glade_xml_get_widget (gxml, "treeview10");

	store_osd = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_osd));
	gtk_widget_show(treeview);

	for (int i = 0; i < total_notify_names; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_osd), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_osd), &iter, 
												0, _(notify_names[i]),
												1, xconfig->osds[i].file, 
												-1);
	}
	
	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Action"), cell, "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("OSD text"), cell, "text", 1, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	g_object_set (cell, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (cell), "edited",
						G_CALLBACK (column_1_edited),
						(gpointer) treeview);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// OSD Font
	widget = glade_xml_get_widget (gxml, "entry2");
	gtk_entry_set_text(GTK_ENTRY(widget), xconfig->osd_font);

	// Show popup
	widget = glade_xml_get_widget (gxml, "checkbutton22");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), xconfig->show_popup);
	
	// Popup Text Preference
	// Popup List set
	treeview = glade_xml_get_widget (gxml, "treeview12");

	store_popup = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store_popup));
	gtk_widget_show(treeview);

	for (int i = 0; i < total_notify_names; i++)
	{
		GtkTreeIter iter;
		gtk_list_store_append(GTK_LIST_STORE(store_popup), &iter);
		gtk_list_store_set(GTK_LIST_STORE(store_popup), &iter, 
												0, _(notify_names[i]),
												1, xconfig->popups[i].file, 
												-1);
	}
	
	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Action"), cell, "text", 0, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	cell = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes(_("Popup message text"), cell, "text", 1, NULL);
	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(column), True);
	g_object_set (cell, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (cell), "edited",
						G_CALLBACK (column_1_edited),
						(gpointer) treeview);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), GTK_TREE_VIEW_COLUMN(column));

	// Button OK
	widget = glade_xml_get_widget (gxml, "button5");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_save_preference), gxml);

	// Button Cancel
	widget = glade_xml_get_widget (gxml, "button4");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_dontsave_preference), gxml);

}

void xneur_add_exclude_app(void)
{
	add_item(store_exclude_app);
}

void xneur_add_autocomplementation_exclude_app(void)
{
	add_item(store_autocomplementation_exclude_app);
}

void xneur_add_auto_app(void)
{
	add_item(store_auto_app);
}

void xneur_add_manual_app(void)
{
	add_item(store_manual_app);
}

void xneur_add_layout_app(void)
{
	add_item(store_layout_app);
}

void xneur_add_draw_flag_app(void)
{
	add_item(store_draw_flag_app);
}

static void xneur_insert_abbreviation(GladeXML *gxml)
{
	GtkWidget *entry1 = glade_xml_get_widget (gxml, "entry1");
	GtkWidget *entry2 = glade_xml_get_widget (gxml, "entry2");
	const gchar *abbreviation = gtk_entry_get_text(GTK_ENTRY(entry1));
	const gchar *full_text = gtk_entry_get_text(GTK_ENTRY(entry2));
	if (strlen(abbreviation) == 0) 
	{
		error_msg(_("Abbreviation field is empty!"));
		return;
	}
	if (strlen(full_text) == 0) 
	{
		error_msg(_("Expansion text field is empty!"));
		return;
	}
	
	GtkTreeIter iter;
	gtk_list_store_append(GTK_LIST_STORE(store_abbreviation), &iter);
	gtk_list_store_set(GTK_LIST_STORE(store_abbreviation), &iter, 
											0, gtk_entry_get_text(GTK_ENTRY(entry1)),
											1, gtk_entry_get_text(GTK_ENTRY(entry2)), 
										   -1);
	
	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
	gtk_widget_destroy(window);
}

void xneur_add_abbreviation(void)
{
	GladeXML *gxml = glade_xml_new (GLADE_FILE_ABBREVIATION_ADD, NULL, NULL);
	
	glade_xml_signal_autoconnect (gxml);
	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");

	GdkPixbuf *window_icon_pixbuf = create_pixbuf ("gxneur.png");
	if (window_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
		gdk_pixbuf_unref (window_icon_pixbuf);
	}
	
	gtk_widget_show(window);
	
	// Button OK
	GtkWidget *widget = glade_xml_get_widget (gxml, "button1");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_insert_abbreviation), gxml);
}

static void xneur_insert_user_action(GladeXML *gxml)
{
	GtkWidget *entry1 = glade_xml_get_widget (gxml, "entry1");
	GtkWidget *entry2 = glade_xml_get_widget (gxml, "entry2");
	const gchar *action = gtk_entry_get_text(GTK_ENTRY(entry1));
	const gchar *key_bind = gtk_entry_get_text(GTK_ENTRY(entry2));
	if (strlen(key_bind) == 0) 
	{
		error_msg(_("Key bind field is empty!"));
		return;
	}
	if (strlen(action) == 0) 
	{
		error_msg(_("User action field is empty!"));
		return;
	}
	
	GtkTreeIter iter;
	gtk_list_store_append(GTK_LIST_STORE(store_action), &iter);
	gtk_list_store_set(GTK_LIST_STORE(store_action), &iter, 
											0, gtk_entry_get_text(GTK_ENTRY(entry1)),
											1, gtk_entry_get_text(GTK_ENTRY(entry2)), 
										   -1);
	
	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
	gtk_widget_destroy(window);
}

void xneur_add_user_action(void)
{
	GladeXML *gxml = glade_xml_new (GLADE_FILE_ACTION_ADD, NULL, NULL);
	
	glade_xml_signal_autoconnect (gxml);
	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");

	GdkPixbuf *window_icon_pixbuf = create_pixbuf ("gxneur.png");
	if (window_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
		gdk_pixbuf_unref (window_icon_pixbuf);
	}
	
	GtkWidget *widget= glade_xml_get_widget (gxml, "entry2");
	g_signal_connect ((gpointer) widget, "key-press-event", G_CALLBACK (on_key_press_event), gxml);
	
	gtk_widget_show(window);
	
	// Button OK
	widget = glade_xml_get_widget (gxml, "button1");
	g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_insert_user_action), gxml);
}

static void xneur_replace_user_action(GladeXML *gxml)
{
	GtkTreeModel *model = GTK_TREE_MODEL(store_action);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tmp_treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
	{
		GtkWidget *widget1= glade_xml_get_widget (gxml, "entry1");
		GtkWidget *widget2= glade_xml_get_widget (gxml, "entry2");
		gtk_list_store_set(GTK_LIST_STORE(store_action), &iter, 
											0, gtk_entry_get_text(GTK_ENTRY(widget1)),
											1, gtk_entry_get_text(GTK_ENTRY(widget2)), 
										   -1);
	}
	
	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
	gtk_widget_destroy(window);
}

void xneur_edit_user_action(GtkWidget *treeview)
{
	tmp_treeview = GTK_TREE_VIEW(treeview);
	GtkTreeModel *model = GTK_TREE_MODEL(store_action);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
	{
		GladeXML *gxml = glade_xml_new (GLADE_FILE_ACTION_ADD, NULL, NULL);
	
		glade_xml_signal_autoconnect (gxml);
		GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
		
		GdkPixbuf *window_icon_pixbuf = create_pixbuf ("gxneur.png");
		if (window_icon_pixbuf)
		{
			gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
			gdk_pixbuf_unref (window_icon_pixbuf);
		}
	
		char *key_bind;
		char *user_action;
		gtk_tree_model_get(GTK_TREE_MODEL(store_action), &iter, 0, &user_action, 1, &key_bind, -1);

		GtkWidget *widget= glade_xml_get_widget (gxml, "entry1");
		gtk_entry_set_text(GTK_ENTRY(widget), user_action);
		
		widget= glade_xml_get_widget (gxml, "entry2");
		g_signal_connect ((gpointer) widget, "key-press-event", G_CALLBACK (on_key_press_event), gxml);
		g_signal_connect ((gpointer) widget, "key-release-event", G_CALLBACK (on_key_release_event), gxml);
		gtk_entry_set_text(GTK_ENTRY(widget), key_bind);
		
		gtk_widget_show(window);
		
		// Button OK
		widget = glade_xml_get_widget (gxml, "button1");
		g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_replace_user_action), gxml);
	}
}

static void xneur_replace_action(GladeXML *gxml)
{
	GtkTreeModel *model = GTK_TREE_MODEL(store_hotkey);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tmp_treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
	{
		GtkWidget *widget1= glade_xml_get_widget (gxml, "entry1");
		GtkWidget *widget2= glade_xml_get_widget (gxml, "entry2");
		gtk_list_store_set(GTK_LIST_STORE(store_hotkey), &iter, 
											0, gtk_entry_get_text(GTK_ENTRY(widget1)),
											1, gtk_entry_get_text(GTK_ENTRY(widget2)), 
										   -1);
	}
	
	GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
	gtk_widget_destroy(window);
}

void xneur_clear_action(GtkWidget *treeview)
{
	tmp_treeview = GTK_TREE_VIEW(treeview);
	GtkTreeModel *model = GTK_TREE_MODEL(store_hotkey);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tmp_treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
	{
		gtk_list_store_set(GTK_LIST_STORE(store_hotkey), &iter, 
											1, "",
											-1);
	}
}

void xneur_edit_action(GtkWidget *treeview)
{
	tmp_treeview = GTK_TREE_VIEW(treeview);
	GtkTreeModel *model = GTK_TREE_MODEL(store_hotkey);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
	{
		GladeXML *gxml = glade_xml_new (GLADE_FILE_ACTION_ADD, NULL, NULL);
	
		glade_xml_signal_autoconnect (gxml);
		GtkWidget *window = glade_xml_get_widget (gxml, "dialog1");
		
		GdkPixbuf *window_icon_pixbuf = create_pixbuf ("gxneur.png");
		if (window_icon_pixbuf)
		{
			gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
			gdk_pixbuf_unref (window_icon_pixbuf);
		}
	
		char *key_bind;
		char *action;
		gtk_tree_model_get(GTK_TREE_MODEL(store_hotkey), &iter, 0, &action, 1, &key_bind, -1);

		GtkWidget *widget= glade_xml_get_widget (gxml, "entry1");
		gtk_editable_set_editable(GTK_EDITABLE(widget), FALSE);
		gtk_entry_set_text(GTK_ENTRY(widget), action);
		
		widget= glade_xml_get_widget (gxml, "entry2");
		g_signal_connect ((gpointer) widget, "key-press-event", G_CALLBACK (on_key_press_event), gxml);
		g_signal_connect ((gpointer) widget, "key-release-event", G_CALLBACK (on_key_release_event), gxml);
		gtk_entry_set_text(GTK_ENTRY(widget), key_bind);
		
		gtk_widget_show(window);
		
		// Button OK
		widget = glade_xml_get_widget (gxml, "button1");
		g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_replace_action), gxml);
	}
}

static void xneur_replace_sound(GladeXML *gxml)
{
	GtkTreeModel *model = GTK_TREE_MODEL(store_sound);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tmp_treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
	{
		GtkWidget *filechooser = glade_xml_get_widget (gxml, "filechooserdialog1");
	
		gtk_list_store_set(GTK_LIST_STORE(store_sound), &iter, 
											1, gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooser)), 
										   -1);
	}
	
	GtkWidget *window = glade_xml_get_widget (gxml, "filechooserdialog1");
	gtk_widget_destroy(window);
}

void xneur_edit_sound(GtkWidget *treeview)
{
	tmp_treeview = GTK_TREE_VIEW(treeview);
	GtkTreeModel *model = GTK_TREE_MODEL(store_sound);
	GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

	gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);

	GtkTreeIter iter;
	if (gtk_tree_selection_get_selected(select, &model, &iter))
	{
		GladeXML *gxml = glade_xml_new (GLADE_FILE_CHOOSE, NULL, NULL);
	
		glade_xml_signal_autoconnect (gxml);
		GtkWidget *window = glade_xml_get_widget (gxml, "filechooserdialog1");
		
		char *file;
		gtk_tree_model_get(GTK_TREE_MODEL(store_sound), &iter, 1, &file, -1);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(window), file);
		
		GdkPixbuf *window_icon_pixbuf = create_pixbuf ("gxneur.png");
		if (window_icon_pixbuf)
		{
			gtk_window_set_icon (GTK_WINDOW (window), window_icon_pixbuf);
			gdk_pixbuf_unref (window_icon_pixbuf);
		}
	
		gtk_widget_show(window);
		
		// Button OK
		GtkWidget *widget = glade_xml_get_widget (gxml, "button5");
		g_signal_connect_swapped(G_OBJECT(widget), "clicked", G_CALLBACK(xneur_replace_sound), gxml);
	}
}

void xneur_rem_exclude_app(GtkWidget *widget)
{
	remove_item(widget, store_exclude_app);
}

void xneur_rem_autocomplementation_exclude_app(GtkWidget *widget)
{
	remove_item(widget, store_autocomplementation_exclude_app);
}

void xneur_rem_auto_app(GtkWidget *widget)
{
	remove_item(widget, store_auto_app);
}

void xneur_rem_manual_app(GtkWidget *widget)
{
	remove_item(widget, store_manual_app);
}

void xneur_rem_layout_app(GtkWidget *widget)
{
	remove_item(widget, store_layout_app);
}

void xneur_rem_draw_flag_app(GtkWidget *widget)
{
	remove_item(widget, store_draw_flag_app);
}

void xneur_rem_abbreviation(GtkWidget *widget)
{
	remove_item(widget, store_abbreviation);
}

void xneur_rem_user_action(GtkWidget *widget)
{
	remove_item(widget, store_action);
}

gboolean save_exclude_app(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	save_list(store_exclude_app, xconfig->excluded_apps, iter);

	return FALSE;
}

gboolean save_autocomplementation_exclude_app(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	save_list(store_autocomplementation_exclude_app, xconfig->autocomplementation_excluded_apps, iter);

	return FALSE;
}

gboolean save_auto_app(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	save_list(store_auto_app, xconfig->auto_apps, iter);

	return FALSE;
}

gboolean save_manual_app(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	save_list(store_manual_app, xconfig->manual_apps, iter);

	return FALSE;
}

gboolean save_layout_app(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	save_list(store_layout_app, xconfig->layout_remember_apps, iter);

	return FALSE;
}

gboolean save_abbreviation(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	gchar *abbreviation;
	gchar *full_text;
	gtk_tree_model_get(GTK_TREE_MODEL(store_abbreviation), iter, 0, &abbreviation, 1, &full_text, -1);

	int ptr_len = strlen(abbreviation) + strlen(full_text) + 2;
	gchar *ptr = malloc(ptr_len* sizeof(gchar));
	snprintf(ptr, ptr_len, "%s %s", abbreviation, full_text);
	xconfig->abbreviations->add(xconfig->abbreviations, ptr);

	g_free(abbreviation);
	g_free(full_text);
	g_free(ptr);

	return FALSE;
}

gboolean save_user_action(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	gchar *key_bind;
	gchar *action_text;
	gtk_tree_model_get(GTK_TREE_MODEL(store_action), iter, 0, &action_text, 1, &key_bind, -1);

	char **key_stat = g_strsplit(key_bind, "+", 4);

	int last = is_correct_hotkey(key_stat);
	if (last == -1)
	{
		g_strfreev(key_stat);
		return FALSE;
	}
	
	int action = atoi(gtk_tree_path_to_string(path));
	xconfig->actions = (struct _xneur_action *) realloc(xconfig->actions, (action + 1) * sizeof(struct _xneur_action));
	bzero(&xconfig->actions[action], sizeof(struct _xneur_action));		
	xconfig->actions[action].hotkey.modifiers = 0;
	
	for (int i = 0; i <= last; i++)
	{
		int assigned = FALSE;
		for (int j = 0; j < total_modifiers; j++) 
 		{ 
			if (g_strcasecmp(key_stat[i], modifier_names[j]) != 0) 
				continue; 

			assigned = TRUE;
			xconfig->actions[action].hotkey.modifiers |= (0x1 << j); 
			break; 
		} 

		if (assigned == FALSE)
		{
			xconfig->actions[action].hotkey.key = strdup(key_stat[i]); 
			xconfig->actions[action].command = strdup(action_text);
		}
	}

	xconfig->actions_count = action + 1;
	
	g_strfreev(key_stat);
	
	g_free(key_bind);
	g_free(action_text);

	return FALSE;
}

gboolean save_action(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	gchar *key_bind;
	gchar *action_text;
	gtk_tree_model_get(GTK_TREE_MODEL(store_hotkey), iter, 0, &action_text, 1, &key_bind, -1);

	int i = atoi(gtk_tree_path_to_string(path));
	split_bind((char *) key_bind, i);
	
	return FALSE;
}

gboolean save_sound(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	gchar *file_path;
	gtk_tree_model_get(GTK_TREE_MODEL(store_sound), iter, 1, &file_path, -1);
	
	int i = atoi(gtk_tree_path_to_string(path));
	if (xconfig->sounds[i].file != NULL)
		free(xconfig->sounds[i].file);
	
	if (file_path != NULL)
	{
		xconfig->sounds[i].file = strdup(file_path);
		g_free(file_path);
	}
	
	return FALSE;
}

gboolean save_osd(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	gchar *string;
	gtk_tree_model_get(GTK_TREE_MODEL(store_osd), iter, 1, &string, -1);
	
	int i = atoi(gtk_tree_path_to_string(path));
	if (xconfig->osds[i].file != NULL)
		free(xconfig->osds[i].file);
	
	if (string != NULL)
	{
		xconfig->osds[i].file = strdup(string);
		g_free(string);
	}
	
	return FALSE;
}

gboolean save_popup(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data)
{
	if (model || path || user_data){};

	gchar *string;
	gtk_tree_model_get(GTK_TREE_MODEL(store_popup), iter, 1, &string, -1);
	
	int i = atoi(gtk_tree_path_to_string(path));
	if (xconfig->popups[i].file != NULL)
		free(xconfig->popups[i].file);
	
	if (string != NULL)
	{
		xconfig->popups[i].file = strdup(string);
		g_free(string);
	}
	
	return FALSE;
}

void xneur_save_preference(GladeXML *gxml)
{
	xconfig->clear(xconfig);

	for (int i = 0; i < MAX_LANGUAGES; i++)
	{
		GtkWidget *widgetPtrToBefound = glade_xml_get_widget (gxml, language_name_boxes[i]);
		GtkWidget *widgetPtrToBefound2 = glade_xml_get_widget (gxml, language_combo_boxes[i]);

		if (gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound)) == -1)
			continue;

		const char *lang_name = get_lang_by_index(gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound)));
		const char *lang_dir = get_lang_code_by_index(gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound)));
		int lang_group = gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound2));

		widgetPtrToBefound = glade_xml_get_widget (gxml, language_fix_boxes[i]);
		int fixed = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
		xconfig->add_language(xconfig, lang_name, lang_dir, lang_group, fixed);
	}
	
	GtkWidget *widgetPtrToBefound = glade_xml_get_widget (gxml, "combobox25");
	xconfig->default_group = gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound));

	gtk_tree_model_foreach(GTK_TREE_MODEL(store_exclude_app), save_exclude_app, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_auto_app), save_auto_app, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_manual_app), save_manual_app, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_layout_app), save_layout_app, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_abbreviation), save_abbreviation, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_sound), save_sound, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_action), save_user_action, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_osd), save_osd, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_popup), save_popup, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_hotkey), save_action, NULL);
	gtk_tree_model_foreach(GTK_TREE_MODEL(store_autocomplementation_exclude_app), save_autocomplementation_exclude_app, NULL);

	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton7");
	xconfig->manual_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton2");
	xconfig->educate = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));

	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton3");
	xconfig->remember_layout = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton4");
	xconfig->save_selection = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton5");
	xconfig->play_sounds = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton6");
	xconfig->save_keyboard_log = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton8");
	xconfig->abbr_ignore_layout = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	// Delay Before Send
	widgetPtrToBefound = glade_xml_get_widget (gxml, "spinbutton1");
	xconfig->send_delay = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widgetPtrToBefound));
	
	// Log Level
	widgetPtrToBefound = glade_xml_get_widget (gxml, "combobox1");
	xconfig->log_level = gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton10");
	xconfig->correct_two_capital_letter = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton9");
	xconfig->correct_incidental_caps = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton11");
	xconfig->flush_buffer_when_press_enter = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton12");
	xconfig->dont_process_when_press_enter = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton13");
	xconfig->show_osd = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));

	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton18");
	xconfig->check_lang_on_process = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));

	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton19");
	xconfig->disable_capslock = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));

	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton20");
	xconfig->correct_space_with_punctuation = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));

	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton21");
	xconfig->autocomplementation = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));

	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton1");
	xconfig->add_space_after_autocomplementation = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));

	// Show popup
	widgetPtrToBefound = glade_xml_get_widget (gxml, "checkbutton22");
	xconfig->show_popup = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widgetPtrToBefound));
	
	widgetPtrToBefound = glade_xml_get_widget (gxml, "entry2");
	if (xconfig->osd_font != NULL)
		free(xconfig->osd_font);
	xconfig->osd_font = strdup((char *) gtk_entry_get_text(GTK_ENTRY(widgetPtrToBefound)));
	
	GtkWidget *window = glade_xml_get_widget (gxml, "window2");
	gtk_widget_destroy(window);

	xconfig->save(xconfig);
	xconfig->reload(xconfig);
}

void xneur_dontsave_preference(GladeXML *gxml)
{
	GtkWidget *window = glade_xml_get_widget (gxml, "window2");
	gtk_widget_destroy(window);
}

char* xneur_get_dict_path(GladeXML *gxml, int layout_no, const char *file_name)
{
	GtkWidget *widgetPtrToBefound = glade_xml_get_widget (gxml, language_name_boxes[layout_no]);

	char *dir_name = get_dir_by_index(gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound)));

	return xconfig->get_global_dict_path(dir_name, file_name);
}

char* xneur_get_home_dict_path(GladeXML *gxml, int layout_no, const char *file_name)
{
	GtkWidget *widgetPtrToBefound = glade_xml_get_widget (gxml, language_name_boxes[layout_no]);

	char *dir_name = get_dir_by_index(gtk_combo_box_get_active(GTK_COMBO_BOX(widgetPtrToBefound)));

	return xconfig->get_home_dict_path(dir_name, file_name);
}

char* xneur_get_file_content(const char *path)
{
	struct stat sb;

	if (stat(path, &sb) != 0 || sb.st_size < 0)
		return NULL;

	FILE *stream = fopen(path, "r");
	if (stream == NULL)
		return NULL;

	unsigned int file_len = sb.st_size;

	char *content = (char *) malloc((file_len + 2) * sizeof(char)); // + '\n' + '\0'
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

char* modifiers_to_string(unsigned int modifiers)
{
	char *text = (char *) malloc((24 + 1) * sizeof(char));
	text[0] = '\0';

	if (modifiers & ShiftMask)
	{
		strcat(text, modifier_names[0]);
		strcat(text, "+");
	}
  
	if (modifiers & ControlMask)
	{
		strcat(text, modifier_names[1]);
		strcat(text, "+");
	}

	if (modifiers & Mod1Mask)
	{
		strcat(text, modifier_names[2]);
		strcat(text, "+");
	}
	
	if (modifiers & Mod4Mask)
	{
		strcat(text, modifier_names[3]);
		strcat(text, "+");
	}

	return text;
}

int is_correct_hotkey(gchar **key_stat)
{
	int last = 0;
	while (key_stat[last] != NULL)
		last++;

	if (last == 0)
		return -1;

	last--;
	if (g_strcasecmp(key_stat[last], _("Press any key")) == 0)
		return -1;

	return last;
}
