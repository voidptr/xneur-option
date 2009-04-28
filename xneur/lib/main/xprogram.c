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
#include <pthread.h>
#include <ctype.h>

#include "xnconfig.h"
#include "xnconfig_files.h"

#include "bind_table.h"

#include "xdefines.h"
#include "xstring.h"
#include "xfocus.h"
#include "xswitchlang.h"
#include "xselection.h"
#include "xevent.h"
#include "xwindow.h"
#include "xkeymap.h"
#include "xutils.h"

#include "types.h"
#include "list_char.h"
#include "log.h"
#include "text.h"
#include "detection.h"
#include "conversion.h"

#include "notify.h"

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

static int prev_mod_key = FALSE;

// Private
static int get_auto_action(KeySym key, int modifier_mask)
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

	// Func, Mod, PF, PrivateKeypad keys
	if (IsFunctionKey(key) || IsModifierKey(key) || IsPFKey(key) || IsPrivateKeypadKey(key))
		return KLB_NO_ACTION;

	// MiscFunc keys
	if (IsMiscFunctionKey(key))
	{
		if (key != XK_Insert)
			return KLB_NO_ACTION;

		if (modifier_mask & ControlMask || modifier_mask & Mod1Mask || modifier_mask & ShiftMask || modifier_mask & Mod4Mask)
			return KLB_CLEAR;

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
		case XK_Tab:
			return KLB_ENTER;
		case XK_space:
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

	int lang = get_cur_lang();
	switch (lang)
	{
		default:
		case 0:
		{
			show_notify(NOTIFY_PRESS_KEY_LAYOUT_0, NULL);
			break;
		}
		case 1:
		{
			show_notify(NOTIFY_PRESS_KEY_LAYOUT_1, NULL);
			break;
		}
		case 2:
		{
			show_notify(NOTIFY_PRESS_KEY_LAYOUT_2, NULL);
			break;
		}
		case 3:
		{
			show_notify(NOTIFY_PRESS_KEY_LAYOUT_3, NULL);
			break;
		}
	}

	return KLB_ADD_SYM;
}

static void xprogram_layout_update(struct _xprogram *p)
{
	if (!xconfig->remember_layout)
		return;

	if ((Window) p->last_window == p->focus->owner_window)
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
		sprintf(window_layouts, "%s %d", text_to_find, xconfig->get_lang_group(xconfig, lang));

		if (!xconfig->window_layouts->exist(xconfig->window_layouts, window_layouts, BY_PLAIN))
			continue;

		free(text_to_find);
		free(window_layouts);

		switch_lang(lang);
		log_message(DEBUG, _("Restore layout group to %d"), xconfig->get_lang_group(xconfig, lang));
		return;
	}

	free(text_to_find);
	free(window_layouts);

	log_message(DEBUG, _("Store default layout group to %d"), xconfig->default_group);
	switch_group(xconfig->default_group);
}

static void xprogram_update(struct _xprogram *p)
{
	p->last_window = p->focus->owner_window;

	int status = p->focus->get_focus_status(p->focus, &p->app_forced_mode, &p->app_focus_mode);
	p->event->set_owner_window(p->event, p->focus->owner_window);

	if (status == FOCUS_UNCHANGED)
		return;

	p->layout_update(p);
	p->string->save_and_clear(p->string, p->last_window);

	if (status == FOCUS_NONE)
		return;

	int listen_mode = LISTEN_GRAB_INPUT;
	if (p->app_focus_mode == FOCUS_EXCLUDED)
		listen_mode = LISTEN_DONTGRAB_INPUT;

	p->focus->update_events(p->focus, listen_mode);
}

static void xprogram_process_input(struct _xprogram *p)
{
	p->update(p);

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
					log_message(LOG, _("Exitting from main cycle"));
					return;
				}
				break;
			}
			case KeyPress:
			{
				log_message(TRACE, _("Received KeyPress (event type %d)"), type);

				// Save received event
				p->event->default_event = p->event->event;

				// Processing received event
				p->on_key_action(p, type);

				// Restore event
				if (p->event->default_event.xkey.keycode != 0)
				{
					p->event->event = p->event->default_event;
					p->event->send_next_event(p->event);
				}
				break;
			}
			case KeyRelease:
			{
				log_message(TRACE, _("Received KeyRelease (event type %d)"), type);

				// Save received event
				p->event->default_event = p->event->event;

				// Processing received event
				p->on_key_action(p, type);

				// Resend special key back to window
				if (p->event->default_event.xkey.keycode != 0)
				{
					p->event->event = p->event->default_event;
					p->event->send_next_event(p->event);
				}
				break;
			}
			case FocusIn:
			case LeaveNotify:
			case EnterNotify:
			{
				if (type == FocusIn)
				{
					log_message(TRACE, _("Received FocusIn (event type %d)"), type);

					p->last_layout = get_active_keyboard_group();

					p->update(p);
				}
				//else if (type == LeaveNotify)
				//	log_message(TRACE, _("Received LeaveNotify (event type %d)"), type);
				//else if (type == EnterNotify)
				//	log_message(TRACE, _("Received EnterNotify (event type %d)"), type);

				break;
			}
			case FocusOut:
			{
				log_message(TRACE, _("Received FocusOut (event type %d)"), type);

				p->last_layout = get_active_keyboard_group();
				p->update(p);
				break;
			}
			case SelectionNotify:
			{
				log_message(TRACE, _("Received SelectionNotify (event type %d)"), type);
				p->process_selection_notify(p);
				break;
			}
			case SelectionRequest:
			{
				log_message(TRACE, _("Received SelectionRequest (event type %d)"), p->event->event.xselectionrequest.requestor, type);
				p->process_selection_notify(p);
				break;
			}
			case ButtonPress:
			{
				p->string->save_and_clear(p->string, p->focus->owner_window);
				log_message(TRACE, _("Received ButtonPress on window %d (event type %d)"), p->event->event.xbutton.window, type);

				// Unfreeze and resend grabbed event
				XAllowEvents(main_window->display, ReplayPointer, CurrentTime);
				break;
			}
			case PropertyNotify:
			{
				if (XInternAtom(main_window->display, "XKLAVIER_STATE", FALSE) == p->event->event.xproperty.atom)
				{
					log_message(TRACE, _("Received Property Notify (layout switch event) (event type %d)"), type);

					// Flush string
					//p->string->clear(p->string);
				}
				break;
			}
			default:
			{
				log_message(DEBUG, _("Uncatched event with type %d)"), type);
				break;
			}
		}
	}
}

static void xprogram_change_lang(struct _xprogram *p, int new_lang)
{
	log_message(DEBUG, _("Changing language from %s to %s"), xconfig->get_lang_name(xconfig, get_cur_lang()), xconfig->get_lang_name(xconfig, new_lang));
	p->string->set_lang_mask(p->string, new_lang);
	switch_lang(new_lang);
}

static void xprogram_change_incidental_caps(struct _xprogram *p)
{
	log_message(DEBUG, _("Correcting iNCIDENTAL CapsLock"));

	// Change modifier mask
	p->string->set_uncaps_mask(p->string);

	// Change CAPS if need
	if (!get_key_state(XK_Caps_Lock))
		return;

	int xkb_opcode, xkb_event, xkb_error;
	int xkb_lmaj = XkbMajorVersion;
	int xkb_lmin = XkbMinorVersion;
	if (XkbLibraryVersion(&xkb_lmaj, &xkb_lmin) && XkbQueryExtension(main_window->display, &xkb_opcode, &xkb_event, &xkb_error, &xkb_lmaj, &xkb_lmin))
		XkbLockModifiers(main_window->display, XkbUseCoreKbd, LockMask, 0);
}

static void xprogram_change_two_capital_letter(struct _xprogram *p)
{
	log_message(DEBUG, _("Correcting two CApital letter"));

	// Change modifier mask
	p->string->keycode_modifiers[1] = p->string->keycode_modifiers[1] & (~ShiftMask);
}

static void xprogram_process_selection_notify(struct _xprogram *p)
{
	char *event_text = NULL;
	event_text = get_selected_text(&p->event->event.xselection);

	if (event_text == NULL)
	{
		p->selected_mode = ACTION_NONE;
		return;
	}

	if (p->selected_mode == ACTION_TRANSLIT_SELECTED)
		convert_text_to_translit(&event_text);

	p->string->set_content(p->string, event_text);
	XFree(event_text);

	switch (p->selected_mode)
	{
		case ACTION_CHANGE_SELECTED:
		{
			p->string->rotate_layout(p->string);

			show_notify(NOTIFY_CHANGE_SELECTED, NULL);
			break;
		}
		case ACTION_CHANGE_CLIPBOARD:
		{
			p->string->rotate_layout(p->string);

			show_notify(NOTIFY_CHANGE_CLIPBOARD, NULL);
			break;
		}
		case ACTION_CHANGECASE_SELECTED:
		{
			p->string->change_case(p->string);

			show_notify(NOTIFY_CHANGECASE_SELECTED, NULL);
			break;
		}
		case ACTION_CHANGECASE_CLIPBOARD:
		{
			p->string->change_case(p->string);

			show_notify(NOTIFY_CHANGECASE_CLIPBOARD, NULL);
			break;
		}
		case ACTION_TRANSLIT_SELECTED:
		{
			int lang = xconfig->find_group_lang(xconfig, main_window->xkeymap->latin_group);
			p->change_lang(p, lang);

			show_notify(NOTIFY_TRANSLIT_SELECTED, NULL);
			break;
		}
		case ACTION_TRANSLIT_CLIPBOARD:
		{
			int lang = xconfig->find_group_lang(xconfig, main_window->xkeymap->latin_group);
			p->change_lang(p, lang);

			show_notify(NOTIFY_TRANSLIT_CLIPBOARD, NULL);
			break;
		}
	}

	// Disable receiving events
	p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);

	// Block events of keyboard
	set_event_mask(p->focus->owner_window, None);
	grab_spec_keys(p->focus->owner_window, FALSE);

	// Selection
	p->change_word(p, CHANGE_SELECTION);

	if (p->selected_mode == ACTION_CHANGE_SELECTED || p->selected_mode == ACTION_CHANGECASE_SELECTED || p->selected_mode == ACTION_TRANSLIT_SELECTED)
	{
		on_selection_converted(SELECTION_PRIMARY);
		if (xconfig->save_selection)
			p->event->send_selection(p->event, p->string->cur_pos);
	}
	if (p->selected_mode == ACTION_CHANGE_CLIPBOARD || p->selected_mode == ACTION_CHANGECASE_CLIPBOARD || p->selected_mode == ACTION_TRANSLIT_CLIPBOARD)
		on_selection_converted(SELECTION_PRIMARY);

	p->string->save_and_clear(p->string, p->focus->owner_window);

	p->update(p);
	p->selected_mode = ACTION_NONE;
}

static void xprogram_on_key_action(struct _xprogram *p, int type)
{
	KeySym key = p->event->get_cur_keysym(p->event);

	// Delete language modifier mask
	int modifier_mask = p->event->get_cur_modifiers(p->event);
	
	if (type == KeyPress)
	{
		prev_mod_key = IsModifierKey(key);
		
		int auto_action = get_auto_action(key, modifier_mask);
		p->perform_auto_action(p, auto_action);
	}

	if (type == KeyRelease)
	{
		if (IsModifierKey(key) && !prev_mod_key)
			return;
		
		int user_action = get_user_action(key, modifier_mask);
		if (user_action >= 0)
		{
			p->perform_user_action(p, user_action);
			p->event->default_event.xkey.keycode = 0;
			prev_mod_key = FALSE;
			return;
		}

		enum _hotkey_action manual_action = get_manual_action(key, modifier_mask);
		if (manual_action != ACTION_NONE)
		{
			p->perform_manual_action(p, manual_action);
			p->event->default_event.xkey.keycode = 0;
			prev_mod_key = FALSE;
			return;
		}
	}
}

static void xprogram_perform_user_action(struct _xprogram *p, int action)
{
	if (p) {};

	log_message(DEBUG, _("Execute user action \"%s\""), xconfig->actions[action].command);

	pthread_attr_t action_thread_attr;
	pthread_attr_init(&action_thread_attr);
	pthread_attr_setdetachstate(&action_thread_attr, PTHREAD_CREATE_DETACHED);

	pthread_t action_thread;
	pthread_create(&action_thread, &action_thread_attr,(void *) &system, (void *) xconfig->actions[action].command);

	pthread_attr_destroy(&action_thread_attr);

	show_notify(NOTIFY_EXEC_USER_ACTION, xconfig->actions[action].command);
}

static void xprogram_perform_auto_action(struct _xprogram *p, int action)
{
	struct _xstring *string = p->string;

	switch (action)
	{
		case KLB_NO_ACTION:
		{
			if (!get_key_state(XK_Caps_Lock))
				return;

			if (!xconfig->disable_capslock)
				return;

			int xkb_opcode, xkb_event, xkb_error;
			int xkb_lmaj = XkbMajorVersion;
			int xkb_lmin = XkbMinorVersion;
			if (XkbLibraryVersion(&xkb_lmaj, &xkb_lmin) && XkbQueryExtension(main_window->display, &xkb_opcode, &xkb_event, &xkb_error, &xkb_lmaj, &xkb_lmin))
				XkbLockModifiers (main_window->display, XkbUseCoreKbd, LockMask, 0);
			return;
		}
		case KLB_CLEAR:
		{
			p->string->save_and_clear(p->string, p->focus->owner_window);
			return;
		}
		case KLB_DEL_SYM:
		{
			string->del_symbol(string);
			return;
		}
		case KLB_ENTER:
		case KLB_SPACE:
		case KLB_ADD_SYM:
		{
			if (action == KLB_ENTER && xconfig->dont_process_when_press_enter)
				action = KLB_ADD_SYM;

			if (p->changed_manual == MANUAL_FLAG_SET)
				p->changed_manual = MANUAL_FLAG_NEED_FLUSH;

			p->last_action = action;

			char sym = main_window->xkeymap->get_cur_ascii_char(main_window->xkeymap, p->event->event);

			if (action == KLB_ADD_SYM)
			{
				if (p->changed_manual == MANUAL_FLAG_NEED_FLUSH)
					p->changed_manual = MANUAL_FLAG_UNSET;

				// Add symbol to internal bufer
				int modifier_mask = groups[get_cur_lang()] | p->event->get_cur_modifiers(p->event);
				p->string->add_symbol(p->string, sym, p->event->event.xkey.keycode, modifier_mask);

				if (!xconfig->check_lang_on_process)
					return;

				// Block events of keyboard (push to event queue)
				set_event_mask(p->focus->owner_window, None);

				// Checking word
				if (p->changed_manual == MANUAL_FLAG_UNSET)
					if (p->check_lang_last_syllable(p))
						p->event->default_event.xkey.keycode = 0;

				// Unblock keyboard
				set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_PRESS_MASK);

				return;
			}

			// Block events of keyboard (push to event queue)
			set_event_mask(p->focus->owner_window, None);

			// Check two capital letter
			if (xconfig->correct_two_capital_letter)
				p->check_tcl_last_word(p);

			// Check incidental caps
			if (xconfig->correct_incidental_caps)
				p->check_caps_last_word(p);

			// Checking word
			if (p->changed_manual == MANUAL_FLAG_UNSET)
				p->check_lang_last_word(p);

			// Add symbol to internal bufer
			p->event->event = p->event->default_event;
			int modifier_mask = groups[get_cur_lang()] | p->event->get_cur_modifiers(p->event);
			p->string->add_symbol(p->string, sym, p->event->event.xkey.keycode, modifier_mask);

			// Send Event
			p->event->send_next_event(p->event);
			p->event->default_event.xkey.keycode = 0;

			// Unblock keyboard
			set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_PRESS_MASK);

			p->changed_manual = MANUAL_FLAG_NEED_FLUSH;

			if (action == KLB_ENTER && xconfig->flush_buffer_when_press_enter)
				p->string->save_and_clear(p->string, p->focus->owner_window);

			return;
		}
	}
}

static int xprogram_perform_manual_action(struct _xprogram *p, enum _hotkey_action action)
{
	switch (action)
	{
		case ACTION_NONE:
			return FALSE;
		case ACTION_CHANGE_MODE:	// User needs to change current work mode
		{
			xconfig->set_manual_mode(xconfig, !xconfig->is_manual_mode(xconfig));

			log_message(DEBUG, _("Manual mode changed to %s"), xconfig->get_bool_name(xconfig->is_manual_mode(xconfig)));
			p->event->default_event.xkey.keycode = 0;
			return TRUE;
		}
		case ACTION_CHANGE_SELECTED:
		case ACTION_TRANSLIT_SELECTED:
		case ACTION_CHANGECASE_SELECTED:
		{
			p->selected_mode = action;
			do_selection_notify(SELECTION_PRIMARY);
			p->event->default_event.xkey.keycode = 0;
			return TRUE;
		}
		case ACTION_CHANGE_CLIPBOARD:
		case ACTION_TRANSLIT_CLIPBOARD:
		case ACTION_CHANGECASE_CLIPBOARD:
		{
			p->selected_mode = action;
			do_selection_notify(SELECTION_CLIPBOARD);
			p->event->default_event.xkey.keycode = 0;
			return TRUE;
		}
		case ACTION_CHANGE_STRING:	// User needs to change current string
		{
			int new_lang = get_next_lang(get_cur_lang());
			int action;
			if (new_lang == 0)
				action = CHANGE_STRING_TO_LAYOUT_0;
			else if (new_lang == 1)
				action = CHANGE_STRING_TO_LAYOUT_1;
			else if (new_lang == 2)
				action = CHANGE_STRING_TO_LAYOUT_2;
			else
				action = CHANGE_STRING_TO_LAYOUT_3;

			p->change_word(p, action);
			p->update(p);

			show_notify(NOTIFY_CHANGE_STRING, NULL);
			break;
		}
		case ACTION_CHANGE_WORD:	// User needs to cancel last change
		{
			int new_lang = get_next_lang(get_cur_lang());

			if (xconfig->educate)
				p->add_word_to_dict(p, new_lang);

			set_event_mask(p->focus->owner_window, None);
			grab_spec_keys(p->focus->owner_window, FALSE);

			int action;
			if (new_lang == 0)
				action = CHANGE_WORD_TO_LAYOUT_0;
			else if (new_lang == 1)
				action = CHANGE_WORD_TO_LAYOUT_1;
			else if (new_lang == 2)
				action = CHANGE_WORD_TO_LAYOUT_2;
			else
				action = CHANGE_WORD_TO_LAYOUT_3;

			p->change_word(p, action);

			set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_PRESS_MASK);
			grab_spec_keys(p->focus->owner_window, TRUE);

			show_notify(NOTIFY_MANUAL_CHANGE_WORD, NULL);
			p->event->default_event.xkey.keycode = 0;
			break;
		}
		case ACTION_ENABLE_LAYOUT_0:
		{
			switch_group(0);
			p->event->default_event.xkey.keycode = 0;
			show_notify(NOTIFY_ENABLE_LAYOUT_0, NULL);
			break;
		}
		case ACTION_ENABLE_LAYOUT_1:
		{
			switch_group(1);
			p->event->default_event.xkey.keycode = 0;
			show_notify(NOTIFY_ENABLE_LAYOUT_1, NULL);
			break;
		}
		case ACTION_ENABLE_LAYOUT_2:
		{
			switch_group(2);
			p->event->default_event.xkey.keycode = 0;
			show_notify(NOTIFY_ENABLE_LAYOUT_2, NULL);
			break;
		}
		case ACTION_ENABLE_LAYOUT_3:
		{
			switch_group(3);
			p->event->default_event.xkey.keycode = 0;
			show_notify(NOTIFY_ENABLE_LAYOUT_3, NULL);
			break;
		}
		case ACTION_ROTATE_LAYOUT:
		{
			set_next_keyboard_group();
			p->event->default_event.xkey.keycode = 0;
			break;
		}	
		case ACTION_REPLACE_ABBREVIATION: // User needs to replace acronym
		{
			// Check last word to acronym list
			char *word = get_last_word(p->string->get_utf_string(p->string));
			if (!word)
				return FALSE;

			for (int words = 0; words < xconfig->abbreviations->data_count; words++)
			{
				char *string		= strdup(xconfig->abbreviations->data[words].string);
				char *replacement	= strsep(&string, " ");

				if (string == NULL)
				{
					free(replacement);
					continue;
				}

				if (xconfig->abbr_ignore_layout)
				{
					KeyCode *dummy_kc = malloc(strlen(replacement) * sizeof(KeyCode));
					int *dummy_kc_mod = malloc(strlen(replacement) * sizeof(int));
					main_window->xkeymap->convert_text_to_ascii(main_window->xkeymap, replacement, dummy_kc, dummy_kc_mod);

					dummy_kc = realloc(dummy_kc, strlen(word) * sizeof(KeyCode));
					dummy_kc_mod = realloc(dummy_kc_mod, strlen(word) * sizeof(int));
					main_window->xkeymap->convert_text_to_ascii(main_window->xkeymap, word, dummy_kc, dummy_kc_mod);

					free(dummy_kc);
					free(dummy_kc_mod);
				}

				if (strcmp(replacement, word) != 0)
				{
					free(replacement);
					continue;
				}

				set_event_mask(p->focus->owner_window, None);
				grab_spec_keys(p->focus->owner_window, FALSE);

				// Replace Abbreviation
				log_message (DEBUG, _("Found Abbreviation '%s' '%s'. Replacing to '%s'."), replacement, word, string);

				p->event->send_backspaces(p->event, strlen(get_last_word(p->string->content)));
				p->string->set_content(p->string, string);

				p->change_word(p, CHANGE_ABBREVIATION);

				set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_PRESS_MASK);
				grab_spec_keys(p->focus->owner_window, TRUE);

				show_notify(NOTIFY_REPLACE_ABBREVIATION, NULL);
				p->string->save_and_clear(p->string, p->focus->owner_window);

				p->event->default_event.xkey.keycode = 0;

				free(replacement);
				return TRUE;
			}
			return FALSE;
		}
	}

	// When CHANGE_STRING or CHANGE_WORD actions occured
	p->changed_manual = MANUAL_FLAG_SET;

	return TRUE;
}

static int xprogram_check_lang_last_word(struct _xprogram *p)
{
	if (p->app_forced_mode == FORCE_MODE_MANUAL)
		return FALSE;

	if (p->app_forced_mode != FORCE_MODE_AUTO && xconfig->is_manual_mode(xconfig))
		return FALSE;

	const char *word = get_last_word(p->string->content);
	if (!word)
		return FALSE;

	int cur_lang = get_cur_lang();
	int new_lang = check_lang(p->string, cur_lang);

	if (new_lang == NO_LANGUAGE)
	{
		log_message(DEBUG, _("No language found to change to"));
		return FALSE;
	}

	if (new_lang == cur_lang)
		return FALSE;

	int change_action = CHANGE_WORD_TO_LAYOUT_0;
	if (new_lang == 0)
		change_action = CHANGE_WORD_TO_LAYOUT_0;
	else if (new_lang == 1)
		change_action = CHANGE_WORD_TO_LAYOUT_1;
	else if (new_lang == 2)
		change_action = CHANGE_WORD_TO_LAYOUT_2;
	else
		change_action = CHANGE_WORD_TO_LAYOUT_3;

	p->change_word(p, change_action);
	show_notify(NOTIFY_AUTOMATIC_CHANGE_WORD, NULL);
	return TRUE;
}

static int xprogram_check_lang_last_syllable(struct _xprogram *p)
{
	if (p->app_forced_mode == FORCE_MODE_MANUAL)
		return FALSE;

	if (p->app_forced_mode != FORCE_MODE_AUTO && xconfig->is_manual_mode(xconfig))
		return FALSE;

	const char *word = get_last_word(p->string->content);
	if (!word)
		return FALSE;

	if (strlen(word) < 3)
		return FALSE;

	int cur_lang = get_cur_lang();
	int new_lang = check_lang(p->string, cur_lang);

	if (new_lang == NO_LANGUAGE)
	{
		log_message(DEBUG, _("No language found to change to"));
		return FALSE;
	}

	if (new_lang == cur_lang)
		return FALSE;

	int change_action = 0;
	if (new_lang == 0)
		change_action = CHANGE_SYLL_TO_LAYOUT_0;
	else if (new_lang == 1)
		change_action = CHANGE_SYLL_TO_LAYOUT_1;
	else if (new_lang == 2)
		change_action = CHANGE_SYLL_TO_LAYOUT_2;
	else
		change_action = CHANGE_SYLL_TO_LAYOUT_3;

	p->change_word(p, change_action);
	show_notify(NOTIFY_AUTOMATIC_CHANGE_WORD, NULL);
	return TRUE;
}

static void xprogram_check_caps_last_word(struct _xprogram *p)
{
	int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

	if (!(p->string->keycode_modifiers[offset] & LockMask) || !(p->string->keycode_modifiers[offset] & ShiftMask))
		return;

	for (int i = 1; i < p->string->cur_pos - offset; i++)
	{
		if ((p->string->keycode_modifiers[offset + i] & LockMask) && (p->string->keycode_modifiers[offset+i] & ShiftMask))
			return;
		if (!(p->string->keycode_modifiers[offset + i] & LockMask))
			return;
	}

	p->change_word(p, CHANGE_INCIDENTAL_CAPS);
	show_notify(NOTIFY_CORR_INCIDENTAL_CAPS, NULL);
}

static void xprogram_check_tcl_last_word(struct _xprogram *p)
{
	int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

	if (!isalpha(p->string->content[offset]))
		return;

	if (p->string->cur_pos - offset <= 2)
		return;

	if (isblank(p->string->content[offset+2]))
		return;

	if (!(p->string->keycode_modifiers[offset] & ShiftMask) || !(p->string->keycode_modifiers[offset + 1] & ShiftMask))
		return;

	for (int i = 2; i < p->string->cur_pos - offset; i++)
	{
		if ((p->string->keycode_modifiers[offset + i] & ShiftMask) && (isalpha(p->string->content[offset + i])))
			return;
	}

	p->change_word(p, CHANGE_TWO_CAPITAL_LETTER);
	show_notify(NOTIFY_CORR_TWO_CAPITAL_LETTER, NULL);
}

static void xprogram_send_string_silent(struct _xprogram *p, int send_backspaces)
{
	if (p->string->cur_pos == 0)
	{
		log_message(DEBUG, _("No string to change"));
		return;
	}

	log_message(DEBUG, _("Processing string '%s'"), p->string->content);

	p->event->send_backspaces(p->event, send_backspaces);		// Delete old string
	p->event->send_string(p->event, p->string);		// Send new string
}

static void xprogram_change_word(struct _xprogram *p, enum _change_action action)
{
	switch (action)
	{
		case CHANGE_INCIDENTAL_CAPS:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_incidental_caps(p);

			p->send_string_silent(p, p->string->cur_pos);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_TWO_CAPITAL_LETTER:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_two_capital_letter(p);

			p->send_string_silent(p, p->string->cur_pos);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_0:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 0);

			p->send_string_silent(p, p->string->cur_pos);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_1:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 1);

			p->send_string_silent(p, p->string->cur_pos);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_2:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 2);

			p->send_string_silent(p, p->string->cur_pos);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_3:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 3);

			p->send_string_silent(p, p->string->cur_pos);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_0:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 0);

			p->send_string_silent(p, p->string->cur_pos - 1);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_1:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 1);

			p->send_string_silent(p, p->string->cur_pos - 1);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_2:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 2);

			p->send_string_silent(p, p->string->cur_pos - 1);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_3:
		{
			int offset = get_last_word_offset(p->string->content, p->string->cur_pos);

			// Shift fields to point to begin of word
			p->string->set_offset(p->string, offset);

			p->change_lang(p, 3);

			p->send_string_silent(p, p->string->cur_pos - 1);

			// Revert fields back
			p->string->unset_offset(p->string, offset);
			break;
		}
		case CHANGE_SELECTION:
		{
			p->send_string_silent(p, 0);
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_0:
		{
			p->change_lang(p, 0);
			p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);	// Disable receiving events

			p->send_string_silent(p, p->string->cur_pos);
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_1:
		{
			p->change_lang(p, 1);
			p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);	// Disable receiving events

			p->send_string_silent(p, p->string->cur_pos);
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_2:
		{
			p->change_lang(p, 2);
			p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);	// Disable receiving events

			p->send_string_silent(p, p->string->cur_pos);
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_3:
		{
			p->change_lang(p, 3);
			p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);	// Disable receiving events

			p->send_string_silent(p, p->string->cur_pos);
			break;
		}
		case CHANGE_ABBREVIATION:
		{
			p->send_string_silent(p, 0);
			break;
		}
	}
}

static void xprogram_add_word_to_dict(struct _xprogram *p, int new_lang)
{
	char *tmp = get_last_word(p->string->content);
	if (tmp == NULL)
		return;

	int curr_lang = get_cur_lang();

	tmp = get_last_word(p->string->xcontent[curr_lang].content);

	char *curr_word = strdup(tmp);

	int len = trim_word(curr_word, strlen(tmp));
	if (len == 0)
	{
		free(curr_word);
		return;
	}

	struct _list_char *curr_temp_dicts = xconfig->languages[curr_lang].temp_dicts;
	if (curr_temp_dicts->exist(curr_temp_dicts, curr_word, BY_PLAIN))
		curr_temp_dicts->rem(curr_temp_dicts, curr_word);

	struct _list_char *new_temp_dicts = xconfig->languages[new_lang].temp_dicts;

	tmp = get_last_word(p->string->xcontent[new_lang].content);

	char *new_word = strdup(tmp);

	len = trim_word(new_word, strlen(tmp));
	if (len == 0)
	{
		free(curr_word);
		free(new_word);
		return;
	}

	if (!new_temp_dicts->exist(new_temp_dicts, new_word, BY_PLAIN))
	{
		new_temp_dicts->add(new_temp_dicts, new_word);
		free(curr_word);
		free(new_word);
		return;
	}

	struct _list_char *curr_dicts = xconfig->languages[curr_lang].dicts;
	if (curr_dicts->exist(curr_dicts, curr_word, BY_PLAIN))
	{
		log_message(DEBUG, _("Remove word '%s' from %s dictionary"), curr_word, xconfig->get_lang_name(xconfig, curr_lang));
		curr_dicts->rem(curr_dicts, curr_word);
		xconfig->save_dicts(xconfig, curr_lang);
	}

	struct _list_char *new_dicts = xconfig->languages[new_lang].dicts;
	if (!new_dicts->exist(new_dicts, new_word, BY_PLAIN))
	{
		log_message(DEBUG, _("Add word '%s' in %s dictionary"), new_word, xconfig->get_lang_name(xconfig, new_lang));
		new_dicts->add(new_dicts, new_word);
		xconfig->save_dicts(xconfig, new_lang);
	}

	free(curr_word);
	free(new_word);
}

static void xprogram_uninit(struct _xprogram *p)
{
	p->focus->uninit(p->focus);
	p->event->uninit(p->event);
	p->string->uninit(p->string);
	main_window->uninit(main_window);

	free(p);

	log_message(DEBUG, _("Program is freed"));
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

	// Function mapping
	p->uninit			= xprogram_uninit;
	p->layout_update		= xprogram_layout_update;
	p->update			= xprogram_update;
	p->on_key_action		= xprogram_on_key_action;
	p->process_input		= xprogram_process_input;
	p->perform_auto_action		= xprogram_perform_auto_action;
	p->perform_manual_action	= xprogram_perform_manual_action;
	p->perform_user_action		= xprogram_perform_user_action;
	p->check_lang_last_word		= xprogram_check_lang_last_word;
	p->check_lang_last_syllable	= xprogram_check_lang_last_syllable;
	p->check_caps_last_word		= xprogram_check_caps_last_word;
	p->check_tcl_last_word		= xprogram_check_tcl_last_word;
	p->change_word			= xprogram_change_word;
	p->add_word_to_dict		= xprogram_add_word_to_dict;
	p->process_selection_notify	= xprogram_process_selection_notify;
	p->change_lang			= xprogram_change_lang;
	p->change_incidental_caps	= xprogram_change_incidental_caps;
	p->change_two_capital_letter	= xprogram_change_two_capital_letter;
	p->send_string_silent		= xprogram_send_string_silent;

	return p;
}
