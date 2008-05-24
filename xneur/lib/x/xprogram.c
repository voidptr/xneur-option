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
 *  Copyright (C) 2006-2008 XNeur Team
 *
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

#include "xnconfig.h"
#include "xnconfig_files.h"

#include "xstring.h"
#include "xfocus.h"
#include "xswitchlang.h"
#include "xselection.h"
#include "xbtable.h"
#include "xevent.h"
#include "xwindow.h"
#include "xkeymap.h"
#include "xutils.h"
#include "xcursor.h"

#include "types.h"
#include "list_char.h"
#include "log.h"
#include "text.h"
#include "detection.h"
#include "conversion.h"
#include "sound.h"

#include "xprogram.h"

#define KLB_NO_ACTION           0	// Modifier, function etc
#define KLB_ADD_SYM             1	// Alpha
#define KLB_DEL_SYM             2	// Backspace
#define KLB_SPACE               3	// Word end (space etc)
#define KLB_ENTER               4	// Enter
#define KLB_CLEAR               5	// Home, End etc

#define MANUAL_FLAG_UNSET	0
#define MANUAL_FLAG_SET		1
#define MANUAL_FLAG_NEED_FLUSH	2

#define NO_MODIFIER_MASK	0

extern struct _xneur_config *xconfig;

struct _xwindow *main_window;

static int get_auto_action(struct _xprogram *p, KeySym key, int modifier_mask)
{
	// Null symbol
	if (key == 0)
		return KLB_NO_ACTION;
	
	// Cursor keys
	if (IsCursorKey(key))
		return KLB_CLEAR;

	// KeyPad keys
	if (IsKeypadKey(key))
	{
		if (get_key_state(XK_Num_Lock) != 0)
		{
			if (modifier_mask & ControlMask || modifier_mask & Mod1Mask || modifier_mask & ShiftMask || modifier_mask & Mod4Mask)
				return KLB_CLEAR;
			return KLB_ADD_SYM;
		}

		switch (key)
		{
			case XK_KP_Divide:
			case XK_KP_Multiply:
			case XK_KP_Add:
			case XK_KP_Subtract:
				return KLB_ADD_SYM;
		}

		return KLB_CLEAR;
	}

	if (xconfig->events_receive_mode == EVENT_RELEASE && IsModifierKey(key)) 
		p->modifier_mask = p->event->event.xkey.state;
	
	// Func, Mod, PF, PrivateKeypad keys
	if (IsFunctionKey(key) || IsModifierKey(key) || IsPFKey(key) || IsPrivateKeypadKey(key))
		return KLB_NO_ACTION;

	// MiscFunc keys
	if (IsMiscFunctionKey(key))
	{
		if (key == XK_Insert)
		{
			if (modifier_mask & ControlMask || modifier_mask & Mod1Mask || modifier_mask & ShiftMask || modifier_mask & Mod4Mask)
				return KLB_CLEAR;
		}
		return KLB_NO_ACTION;
	}

	// Del, bkspace, tab, return, alpha & num keys
	switch (key)
	{
		case XK_BackSpace:
			return KLB_DEL_SYM;
		case XK_Pause:
		case XK_Escape:
		case XK_Sys_Req:
		case XK_Delete:
			return KLB_NO_ACTION;
		case XK_Return:
			return KLB_ENTER;
		case XK_Tab:
		case XK_space:
		case XK_exclam:
		case XK_at:
		case XK_numbersign:
		case XK_ssharp:
		case XK_dollar:
		case XK_percent:
		case XK_asciicircum:
		case XK_ampersand:
		case XK_asterisk:
		case XK_parenleft:
		case XK_parenright:
		case XK_hyphen:
		case XK_underscore:
		case XK_equal:
		case XK_plus:
		case XK_minus:
		case XK_slash:
		case XK_bar:
		case XK_backslash:
		case XK_question:
			return KLB_SPACE;
	}
	
	if (modifier_mask & ControlMask || modifier_mask & Mod1Mask || modifier_mask & Mod4Mask)
		return KLB_NO_ACTION;

	int sound;
	int lang = get_cur_lang(); 
	switch (lang)
	{
		default:
		case 0:
		{
			sound = SOUND_PRESS_KEY_LAYOUT_0;
			break;
		}
		case 1:
		{
			sound = SOUND_PRESS_KEY_LAYOUT_1;
			break;
		}
		case 2:
		{
			sound = SOUND_PRESS_KEY_LAYOUT_2;
			break;
		}
		case 3:
		{
			sound = SOUND_PRESS_KEY_LAYOUT_0;
			break;
		}
	}

	play_file(sound);

	return KLB_ADD_SYM;
}

void xprogram_cursor_update(struct _xprogram *p)
{
	if (p->focus->draw_flag(p->focus, p->event->event.xmotion.window))
	{		
		int root_x, root_y, win_x, win_y;
		Window root_window, child_window;
		unsigned int dummyU;
					
		XQueryPointer(main_window->display, p->focus->owner_window, &root_window, &child_window, &root_x, &root_y, &win_x, &win_y, &dummyU);			
				
		p->cursor->show_flag(p->cursor, root_x+10, root_y+10);
	}
	else
		p->cursor->hide_flag(p->cursor);
}

void xprogram_layout_update(struct _xprogram *p)
{
	if (xconfig->layout_remember_mode == LAYOUTE_REMEMBER_DISABLE)
		return;

	if (p->last_window == p->focus->owner_window)
		return;

	char *text_to_find	= (char *) malloc(1024 * sizeof(char));
	char *window_layouts	= (char *) malloc(1024 * sizeof(char));

	struct _list_char_data* last_app = NULL;

	char *last_app_name = get_wm_class_name(p->last_window);
	if (last_app_name != NULL)
		last_app = xconfig->layout_remember_apps->find(xconfig->layout_remember_apps, last_app_name, BY_PLAIN);

	if (last_app != NULL)
		sprintf(text_to_find, "%s", last_app_name);
	else
		sprintf(text_to_find, "%d", (int) p->last_window);
	
	if (last_app_name != NULL)
		free(last_app_name);

	// Remove layout for old window
	for (int lang = 0; lang < xconfig->total_languages; lang++)
	{
		sprintf(window_layouts, "%s %d", text_to_find, lang);
		
		if (!xconfig->window_layouts->exist(xconfig->window_layouts, window_layouts, BY_PLAIN))
			continue;

		xconfig->window_layouts->rem(xconfig->window_layouts, window_layouts);
	}

	// Save layout for old window
	sprintf(window_layouts, "%s %d", text_to_find, p->last_layout);
	xconfig->window_layouts->add(xconfig->window_layouts, window_layouts);
	
	struct _list_char_data* curr_app = NULL;

	char *curr_app_name = get_wm_class_name(p->focus->owner_window);
	if (curr_app_name != NULL)
		curr_app = xconfig->layout_remember_apps->find(xconfig->layout_remember_apps, curr_app_name, BY_PLAIN);

	if (curr_app != NULL)
		sprintf(text_to_find, "%s", curr_app_name);
	else
		sprintf(text_to_find, "%d", (int) p->focus->owner_window);

	if (curr_app_name != NULL)
		free(curr_app_name);

	// Restore layout for new window
	for (int lang = 0; lang < xconfig->total_languages; lang++)
	{
		sprintf(window_layouts, "%s %d", text_to_find, lang);

		if (!xconfig->window_layouts->exist(xconfig->window_layouts, window_layouts, BY_PLAIN))
			continue;

		free(text_to_find);
		free(window_layouts);

		switch_lang(lang);
		log_message(DEBUG, "Restore layout group to %d", xconfig->get_lang_group(xconfig, lang));
		return;
	}

	free(text_to_find);
	free(window_layouts);

	log_message(DEBUG, "Store default layout group to %d", xconfig->default_group);
	switch_group(xconfig->default_group);
}

void xprogram_update(struct _xprogram *p, int *do_update)
{
	if (*do_update == FALSE)
		return;

	p->last_window = p->focus->owner_window;

	*do_update = FALSE;

	int status = p->focus->get_focus_status(p->focus, &p->app_forced_mode, &p->app_focus_mode);

	if (status != FOCUS_UNCHANGED)
	{
		p->layout_update(p);
		p->string->clear(p->string);
	}

	if (status == FOCUS_NONE)
		return;

	int listen_mode = LISTEN_GRAB_INPUT;
	if (p->app_focus_mode == FOCUS_EXCLUDED)
		listen_mode = LISTEN_DONTGRAB_INPUT;

	p->event->set_owner_window(p->event, p->focus->owner_window);
	p->focus->update_events(p->focus, listen_mode);
}

void xprogram_process_input(struct _xprogram *p)
{
	int do_update = TRUE;
	p->update(p, &do_update);
				
	while (1)
	{
		int type = p->event->get_next_event(p->event);
		
		if (p->event->event.xany.window == main_window->flag_window)
			continue;
		
		switch (type)
		{
			case ClientMessage:
			{
				// Exit from main cycle by message to main window
				XClientMessageEvent *cme = (XClientMessageEvent *) &(p->event->event);
				if (cme->message_type == main_window->close_atom)
				{
					log_message(LOG, "Exitting from main cycle");
					return;
				}
			}
			case KeyPress:
			{
				log_message(TRACE, "Received KeyPress");

				if (xconfig->events_receive_mode == EVENT_PRESS)
					p->on_key_action(p);
				
				p->update(p, &do_update);				
				break;
			}
			case KeyRelease:
			{
				log_message(TRACE, "Received KeyRelease");

				if (xconfig->events_receive_mode == EVENT_RELEASE)
					p->on_key_action(p);
				
				p->update(p, &do_update);
				break;
			}
			case FocusIn:
			case LeaveNotify:
			case EnterNotify:
			{
				p->cursor_update(p);
				
				p->last_layout = get_active_keyboard_group();

				do_update = TRUE;
				p->update(p, &do_update);
				break;
			}
			case FocusOut:
			{
				p->last_layout = get_active_keyboard_group();

				do_update = TRUE;
				p->update(p, &do_update);
				break;
			}
			case SelectionNotify:
			{
				p->update(p, &do_update);
				p->process_selection(p);
				break;
			}
			case ButtonPress:				// Falling down
				p->string->clear(p->string);
				p->update(p, &do_update);
				log_message(TRACE, "Received ButtonPress");
				p->event->send_next_event(p->event);
				break;
				
			case ButtonRelease:
			{
				p->update(p, &do_update);
				log_message(TRACE, "Received ButtonRelease");
				p->event->send_next_event(p->event);
				break;
			}
			case MotionNotify:
			{
				//log_message(TRACE, "Received Motion Notify");
				p->cursor_update(p);	
				break;				
			}
			case PropertyNotify:
			{
				
				if (XInternAtom(main_window->display, "XKLAVIER_STATE", FALSE) == p->event->event.xproperty.atom)
				{
					log_message(TRACE, "Received Property Notify (layout switch event)");
					
					// Flush string
					p->string->clear(p->string);
					
					// Update flag
					p->cursor_update(p);
				}
				// On all event
				if (xconfig->events_receive_mode == EVENT_RELEASE)
					p->modifier_mask = NO_MODIFIER_MASK;
				
				break;
			}
			default:
			{
				log_message(DEBUG, "Uncatched event with type %d)", type);
				break;
			}
		}
	}
}

void xprogram_change_lang(struct _xprogram *p, int new_lang)
{
	log_message(DEBUG, "Changing language from %s to %s", xconfig->get_lang_name(xconfig, get_cur_lang()), xconfig->get_lang_name(xconfig, new_lang));
	p->string->set_key_code(p->string, new_lang);
	switch_lang(new_lang);
}

void xprogram_process_selection(struct _xprogram *p)
{
	char *event_text = get_selected_text(&p->event->event.xselection);
	if (event_text == NULL)
		return;

	char *selected_text = strdup(event_text);
	XFree(event_text);

	if (p->selected_mode == ACTION_CHANGE_SELECTED || p->selected_mode == ACTION_CHANGECASE_SELECTED)
		main_window->xkeymap->convert_text_to_ascii(main_window->xkeymap, selected_text);

	if (p->selected_mode == ACTION_TRANSLIT_SELECTED)
		convert_text_to_translit(&selected_text);

	p->string->set_content(p->string, selected_text);

	if (p->selected_mode == ACTION_CHANGECASE_SELECTED)
	{
		p->string->changecase_content(p->string);
		p->change_lang(p, get_cur_lang());
	}
	else
		p->change_lang(p, get_next_lang(get_cur_lang()));
	
	if (p->selected_mode == ACTION_CHANGECASE_SELECTED)
		play_file(SOUND_CHANGECASE_SELECTED);
	else if (p->selected_mode == ACTION_TRANSLIT_SELECTED)
		play_file(SOUND_TRANSLIT_SELECTED);
	else if (p->selected_mode == ACTION_CHANGE_SELECTED)
		play_file(SOUND_CHANGE_SELECTED);
	
	p->send_string_silent(p, FALSE);

	on_selection_converted(selected_text);

	if (xconfig->save_selection_mode == SELECTION_SAVE_ENABLED)
		p->event->send_selection(p->event, p->string->cur_pos);

	p->string->clear(p->string);
	free(selected_text);
}

void xprogram_on_key_action(struct _xprogram *p)
{
	KeySym key = p->event->get_cur_keysym(p->event);

	int modifier_mask = p->event->get_cur_modifiers(p->event);
	
	if (p->modifier_mask != NO_MODIFIER_MASK) 
	{
		p->event->event.xkey.state = p->modifier_mask;
		p->modifier_mask = NO_MODIFIER_MASK;
	}
	
	enum _hotkey_action manual_action = get_manual_action(key, modifier_mask);
	if (p->perform_manual_action(p, manual_action))
		return;

	int auto_action = get_auto_action(p, key, modifier_mask);
	p->perform_auto_action(p, auto_action);
}

void xprogram_perform_auto_action(struct _xprogram *p, int action)
{
	struct _xstring *string = p->string;

	switch (action)
	{
		case KLB_CLEAR:
		{
			string->clear(string);
			return;
		}
		case KLB_DEL_SYM:
		{
			string->del_symbol(string);
			return;
		}
		case KLB_SPACE:
		case KLB_ADD_SYM:
		{
			if (p->changed_manual == MANUAL_FLAG_SET)
				p->changed_manual = MANUAL_FLAG_NEED_FLUSH;

			char sym = main_window->xkeymap->get_cur_ascii_char(main_window->xkeymap, p->event->event);
			
			string->add_symbol(string, sym, p->event->event.xkey.keycode, p->event->event.xkey.state);

			p->last_action = action;

			if (action == KLB_ADD_SYM)
			{
				if (p->changed_manual == MANUAL_FLAG_NEED_FLUSH)
					p->changed_manual = MANUAL_FLAG_UNSET;

				return;
			}

			if (p->changed_manual == MANUAL_FLAG_UNSET)
			{
				// Block keyboard
				grab_keyboard(p->focus->owner_window, TRUE);
	
				// Checking word
				p->check_last_word(p);

				// Sending blocked events
				while (XEventsQueued(main_window->display, QueuedAlready))
				{
					int type = p->event->get_next_event(p->event);
					p->event->send_next_event(p->event);

					if (type == KeyPress)
						p->on_key_action(p);	
				}

				// Unblock keyboard 
				grab_keyboard(p->focus->owner_window, FALSE);
			}
			p->changed_manual = MANUAL_FLAG_NEED_FLUSH;

			return;
		}
		case KLB_ENTER:
		{
			char sym = main_window->xkeymap->get_cur_ascii_char(main_window->xkeymap, p->event->event);
			
			string->add_symbol(string, sym, p->event->event.xkey.keycode, p->event->event.xkey.state);
				
			int do_update = TRUE;
			p->update(p, &do_update);
			
			string->clear(string);
			
			p->last_action = action;
			
			p->changed_manual = MANUAL_FLAG_NEED_FLUSH;
			
			return;
		}
	}
}

int xprogram_perform_manual_action(struct _xprogram *p, enum _hotkey_action action)
{
	switch (action)
	{
		case ACTION_NONE:
			return FALSE;
		case ACTION_CHANGE_MODE:	// User needs to change current work mode
		{
			if (xconfig->get_current_mode(xconfig) == AUTO_MODE)
			{
				xconfig->set_current_mode(xconfig, MANUAL_MODE);
				log_message(DEBUG, "Current mode changed to manual");
			}
			else
			{
				xconfig->set_current_mode(xconfig, AUTO_MODE);
				log_message(DEBUG, "Current mode changed to auto");
			}
			return TRUE;
		}
		case ACTION_CHANGE_SELECTED:
		case ACTION_TRANSLIT_SELECTED:
		case ACTION_CHANGECASE_SELECTED:
		{
			p->selected_mode = action;
			do_selection_request();
			return TRUE;
		}
		case ACTION_CHANGE_STRING:	// User needs to change current string
		{
			p->change_lang(p, get_next_lang(get_cur_lang()));
			p->send_string_silent(p, TRUE);
			play_file(SOUND_CHANGE_STRING);
			break;
		}
		case ACTION_CHANGE_WORD:	// User needs to cancel last change
		{
			int next_lang = get_next_lang(get_cur_lang());

			if (xconfig->education_mode == EDUCATION_MODE_ENABLE)
				p->add_word_to_dict(p, next_lang);

			p->change_word(p, next_lang);
			play_file(SOUND_MANUAL_CHANGE_WORD);
			break;
		}
		case ACTION_ENABLE_LAYOUT_0:
		{
			switch_group(0);
			play_file(SOUND_ENABLE_LAYOUT_0);
			break;
		}
		case ACTION_ENABLE_LAYOUT_1:
		{
			switch_group(1);
			play_file(SOUND_ENABLE_LAYOUT_1);
			break;
		}
		case ACTION_ENABLE_LAYOUT_2:
		{
			switch_group(2);
			play_file(SOUND_ENABLE_LAYOUT_2);
			break;
		}
		case ACTION_ENABLE_LAYOUT_3:
		{
			switch_group(3);
			play_file(SOUND_ENABLE_LAYOUT_3);
			break;
		}
	}

	// When CHANGE_STRING or CHANGE_WORD actions occured
	p->changed_manual = MANUAL_FLAG_SET;

	return TRUE;
}

void xprogram_check_last_word(struct _xprogram *p)
{
	if (p->app_forced_mode == FORCE_MODE_MANUAL)
		return;

	if (p->app_forced_mode != FORCE_MODE_AUTO && xconfig->get_current_mode(xconfig) == MANUAL_MODE)
		return;

	const char *word = get_last_word(p->string->content);
	if (!word)
		return;

	int cur_lang = get_cur_lang();

	int new_lang = get_word_lang(word, cur_lang);
	if (new_lang == NO_LANGUAGE)
	{
		log_message(DEBUG, "No language found to change to");
		return;
	}

	if (new_lang == cur_lang)
		return;

	p->change_word(p, new_lang);
	play_file(SOUND_AUTOMATIC_CHANGE_WORD);
}

void xprogram_send_string_silent(struct _xprogram *p, int send_backspaces)
{
	if (p->string->cur_pos == 0)
	{
		log_message(DEBUG, "No string to change");
		return;
	}

	log_message(DEBUG, "Processing string '%s'", p->string->content);

	int bcount = p->string->cur_pos;
	if (send_backspaces == FALSE)
		bcount = 1;

	p->focus->update_events(p->focus, LISTEN_FLUSH);		// Disable receiving events
	p->event->send_backspaces(p->event, bcount);			// Delete old string
	p->event->send_string(p->event, p->string);			// Send new string

	int do_update = TRUE;
	p->update(p, &do_update);
}

void xprogram_change_word(struct _xprogram *p, int new_lang)
{
	int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

	// Shift fields to point to begin of word
	p->string->content		+= offset;
	p->string->keycode		+= offset;
	p->string->keycode_modifiers	+= offset;
	p->string->cur_pos		-= offset;

	p->change_lang(p, new_lang);
	
	p->send_string_silent(p, TRUE);

	// Revert fields back
	p->string->content		-= offset;
	p->string->keycode		-= offset;
	p->string->keycode_modifiers	-= offset;
	p->string->cur_pos		+= offset;
}

void xprogram_add_word_to_dict(struct _xprogram *p, int new_lang)
{
	int len = strlen(get_last_word(p->string->content));
	char *low_word = lower_word(get_last_word(p->string->content), len);

	len = trim_word(low_word, len);
	if (len == 0)
	{
		free(low_word);
		return;
	}

	int curr_lang = get_cur_lang();

	struct _list_char *curr_temp_dicts = xconfig->languages[curr_lang].temp_dicts;
	if (curr_temp_dicts->exist(curr_temp_dicts, low_word, BY_PLAIN))
		curr_temp_dicts->rem(curr_temp_dicts, low_word);

	struct _list_char *new_temp_dicts = xconfig->languages[new_lang].temp_dicts;
	if (!new_temp_dicts->exist(new_temp_dicts, low_word, BY_PLAIN))
	{
		new_temp_dicts->add(new_temp_dicts, low_word);
		free(low_word);
		return;
	}

	struct _list_char *curr_dicts = xconfig->languages[curr_lang].dicts;
	if (curr_dicts->exist(curr_dicts, low_word, BY_PLAIN))
	{
		log_message(DEBUG, "Remove word '%s' from %s dictionary", low_word, xconfig->get_lang_name(xconfig, curr_lang));
		curr_dicts->rem(curr_dicts, low_word);
		xconfig->save_dicts(xconfig, curr_lang); 
	}

	struct _list_char *new_dicts = xconfig->languages[new_lang].dicts;
	if (!new_dicts->exist(new_dicts, low_word, BY_PLAIN))
	{
		log_message(DEBUG, "Add word '%s' in %s dictionary", low_word, xconfig->get_lang_name(xconfig, new_lang));
		new_dicts->add(new_dicts, low_word);
		xconfig->save_dicts(xconfig, new_lang); 
	}

	free(low_word);
}

void xprogram_uninit(struct _xprogram *p)
{
	p->focus->uninit(p->focus);
	log_message(DEBUG, "Current focus is freed");
	p->event->uninit(p->event);
	log_message(DEBUG, "Current event is freed");
	p->string->uninit(p->string);
	log_message(DEBUG, "Current string is freed");
	p->cursor->uninit(p->cursor);
	log_message(DEBUG, "Current cursor is freed");
	
	main_window->uninit(main_window);
	log_message(DEBUG, "Current main_window is freed");
	

	free(p);
}

struct _xprogram* xprogram_init(void)
{
	struct _xprogram *p = (struct _xprogram*) malloc(sizeof(struct _xprogram));
	bzero(p, sizeof(struct _xprogram));

	main_window = xwindow_init();

	if (!main_window->create(main_window) || !main_window->init_keymap(main_window))
	{
		free(p);
		return NULL;
	}

	p->modifier_mask		= NO_MODIFIER_MASK;
	
	p->event			= xevent_init();		// X Event processor
	p->focus			= xfocus_init();		// X Input Focus and Pointer processor
	p->string			= xstring_init();		// Input string buffer
	p->cursor			= xcursor_init();
	
	// Function mapping
	p->uninit			= xprogram_uninit; 
	p->cursor_update		= xprogram_cursor_update;
	p->layout_update		= xprogram_layout_update;
	p->update			= xprogram_update;
	p->on_key_action		= xprogram_on_key_action;
	p->process_input		= xprogram_process_input;
	p->perform_auto_action		= xprogram_perform_auto_action;
	p->perform_manual_action	= xprogram_perform_manual_action;
	p->check_last_word		= xprogram_check_last_word;
	p->change_word			= xprogram_change_word;
	p->add_word_to_dict		= xprogram_add_word_to_dict;
	p->process_selection		= xprogram_process_selection;
	p->change_lang			= xprogram_change_lang;
	p->send_string_silent		= xprogram_send_string_silent;

	return p;
}
