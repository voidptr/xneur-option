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
#include "event.h"
#include "focus.h"
#include "selection.h"
#include "switchlang.h"
#include "defines.h"
#include "buffer.h"
#include "window.h"
#include "keymap.h"
#include "utils.h"
#include "plugin.h"

#include "types.h"
#include "list_char.h"
#include "log.h"
#include "text.h"
#include "detection.h"
#include "conversion.h"

#include "notify.h"

#include "program.h"

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

#define MIN_PATTERN_LEN		4

extern struct _xneur_config *xconfig;

struct _window *main_window;

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
		case XK_semicolon:
		case XK_comma:
		case XK_period:
		case XK_1:
		case XK_2:
		case XK_3:
		case XK_4:
		case XK_5:
		case XK_6:
		case XK_7:
		case XK_8:
		case XK_9:
		case XK_0:
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

static void program_layout_update(struct _program *p)
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

static void program_update(struct _program *p)
{
	p->last_window = p->focus->owner_window;

	int status = p->focus->get_focus_status(p->focus, &p->app_forced_mode, &p->app_focus_mode, &p->app_autocomplementation_mode);
	p->event->set_owner_window(p->event, p->focus->owner_window);

	if (status == FOCUS_UNCHANGED)
		return;

	p->layout_update(p);
	p->buffer->save_and_clear(p->buffer, p->last_window);

	if (status == FOCUS_NONE)
		return;

	int listen_mode = LISTEN_GRAB_INPUT;
	if (p->app_focus_mode == FOCUS_EXCLUDED)
		listen_mode = LISTEN_DONTGRAB_INPUT;

	p->modifiers_stack->uninit(p->modifiers_stack);
	p->modifiers_stack	= list_char_init();
	p->update_modifiers_stack(p);
	
	p->focus->update_events(p->focus, listen_mode);
}

static void program_process_input(struct _program *p)
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
				log_message(TRACE, _("Received KeyPress '%s' (event type %d)"), XKeysymToString(p->event->get_cur_keysym(p->event)), type);

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
				log_message(TRACE, _("Received KeyRelease '%s' (event type %d)"), XKeysymToString(p->event->get_cur_keysym(p->event)), type);

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
				p->buffer->save_and_clear(p->buffer, p->focus->owner_window);
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
					//p->buffer->clear(p->buffer);
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

static void program_change_lang(struct _program *p, int new_lang)
{
	log_message(DEBUG, _("Changing language from %s to %s"), xconfig->get_lang_name(xconfig, get_cur_lang()), xconfig->get_lang_name(xconfig, new_lang));
	p->buffer->set_lang_mask(p->buffer, new_lang);
	switch_lang(new_lang);
}

static void program_change_incidental_caps(struct _program *p)
{
	log_message(DEBUG, _("Correcting iNCIDENTAL CapsLock"));

	// Change modifier mask
	p->buffer->set_uncaps_mask(p->buffer);

	// Change CAPS if need
	if (!get_key_state(XK_Caps_Lock))
		return;

	int xkb_opcode, xkb_event, xkb_error;
	int xkb_lmaj = XkbMajorVersion;
	int xkb_lmin = XkbMinorVersion;
	if (XkbLibraryVersion(&xkb_lmaj, &xkb_lmin) && XkbQueryExtension(main_window->display, &xkb_opcode, &xkb_event, &xkb_error, &xkb_lmaj, &xkb_lmin))
		XkbLockModifiers(main_window->display, XkbUseCoreKbd, LockMask, 0);
}

static void program_change_two_capital_letter(struct _program *p)
{
	log_message(DEBUG, _("Correcting two CApital letter"));

	// Change modifier mask
	p->buffer->keycode_modifiers[1] = p->buffer->keycode_modifiers[1] & (~ShiftMask);
}

static void program_process_selection_notify(struct _program *p)
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

	p->buffer->set_content(p->buffer, event_text);
	XFree(event_text);

	switch (p->selected_mode)
	{
		case ACTION_CHANGE_SELECTED:
		{
			p->buffer->rotate_layout(p->buffer);

			show_notify(NOTIFY_CHANGE_SELECTED, NULL);
			break;
		}
		case ACTION_CHANGE_CLIPBOARD:
		{
			p->buffer->rotate_layout(p->buffer);

			show_notify(NOTIFY_CHANGE_CLIPBOARD, NULL);
			break;
		}
		case ACTION_CHANGECASE_SELECTED:
		{
			p->buffer->change_case(p->buffer);

			show_notify(NOTIFY_CHANGECASE_SELECTED, NULL);
			break;
		}
		case ACTION_CHANGECASE_CLIPBOARD:
		{
			p->buffer->change_case(p->buffer);

			show_notify(NOTIFY_CHANGECASE_CLIPBOARD, NULL);
			break;
		}
		case ACTION_TRANSLIT_SELECTED:
		{
			int lang = xconfig->find_group_lang(xconfig, main_window->keymap->latin_group);
			p->change_lang(p, lang);

			show_notify(NOTIFY_TRANSLIT_SELECTED, NULL);
			break;
		}
		case ACTION_TRANSLIT_CLIPBOARD:
		{
			int lang = xconfig->find_group_lang(xconfig, main_window->keymap->latin_group);
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
			p->event->send_selection(p->event, p->buffer->cur_pos);
	}
	if (p->selected_mode == ACTION_CHANGE_CLIPBOARD || p->selected_mode == ACTION_CHANGECASE_CLIPBOARD || p->selected_mode == ACTION_TRANSLIT_CLIPBOARD)
		on_selection_converted(SELECTION_PRIMARY);

	p->buffer->save_and_clear(p->buffer, p->focus->owner_window);

	p->update(p);
	p->selected_mode = ACTION_NONE;
}

static void program_update_modifiers_stack(struct _program *p)
{
	// Update mask
	p->prev_key_mod = 0;
	
	if (p->modifiers_stack->exist(p->modifiers_stack, "Shift_L", BY_PLAIN) || p->modifiers_stack->exist(p->modifiers_stack, "Shift_R", BY_PLAIN))
		p->prev_key_mod += (1 << 0);
	if (p->modifiers_stack->exist(p->modifiers_stack, "Caps_Lock", BY_PLAIN))
		p->prev_key_mod += (1 << 1);
	if (p->modifiers_stack->exist(p->modifiers_stack, "Control_L", BY_PLAIN) || p->modifiers_stack->exist(p->modifiers_stack, "Control_R", BY_PLAIN))
		p->prev_key_mod += (1 << 2);
	if (p->modifiers_stack->exist(p->modifiers_stack, "Alt_L", BY_PLAIN) || p->modifiers_stack->exist(p->modifiers_stack, "Alt_R", BY_PLAIN))
		p->prev_key_mod += (1 << 3);
	if (p->modifiers_stack->exist(p->modifiers_stack, "Meta_L", BY_PLAIN) || p->modifiers_stack->exist(p->modifiers_stack, "Meta_R", BY_PLAIN))
		p->prev_key_mod += (1 << 4);
	if (p->modifiers_stack->exist(p->modifiers_stack, "Num_Lock", BY_PLAIN))
		p->prev_key_mod += (1 << 5);
	if (p->modifiers_stack->exist(p->modifiers_stack, "Hyper_L", BY_PLAIN) || p->modifiers_stack->exist(p->modifiers_stack, "Hyper_R", BY_PLAIN))
		p->prev_key_mod += (1 << 6);
	if (p->modifiers_stack->exist(p->modifiers_stack, "Super_L", BY_PLAIN) || p->modifiers_stack->exist(p->modifiers_stack, "Super_R", BY_PLAIN))
		p->prev_key_mod += (1 << 7);
}

static void program_on_key_action(struct _program *p, int type)
{
	KeySym key = p->event->get_cur_keysym(p->event);

	// Delete language modifier mask
	int modifier_mask = p->event->get_cur_modifiers(p->event);

	if (type == KeyPress)
	{
		p->prev_key = key;
		if (IsModifierKey(key))
		{
			char *keysym_str = XKeysymToString(p->event->get_cur_keysym(p->event));
			p->modifiers_stack->add(p->modifiers_stack, keysym_str);
		}
		p->prev_key_mod |= p->event->get_cur_modifiers_by_keysym(p->event);
		
		int user_action = get_user_action(key, p->prev_key_mod);
		enum _hotkey_action manual_action = get_manual_action(key, p->prev_key_mod);
		if ((user_action >= 0) || (manual_action != ACTION_NONE))
		{
			p->event->default_event.xkey.keycode = 0;
			return;
		}

		p->plugin->key_press(p->plugin, key, p->prev_key_mod);
		
		int auto_action = get_auto_action(key, p->prev_key_mod);
		p->perform_auto_action(p, auto_action);
		
	}

	if (type == KeyRelease)
	{	
		// Del from stack
		if (IsModifierKey(key))
		{
			char *keysym_str = XKeysymToString(p->event->get_cur_keysym(p->event));
			p->modifiers_stack->rem(p->modifiers_stack, keysym_str);
		}

		if (p->prev_key == None)
		{
			p->update_modifiers_stack(p);
			return;
		}
		
		if (p->prev_key != key)
			return;
		
		p->prev_key = None;
		
		modifier_mask = p->prev_key_mod;
		if (IsModifierKey(key))
			modifier_mask &= ~p->event->get_cur_modifiers_by_keysym(p->event);

		p->modifiers_stack->rem(p->modifiers_stack, XKeysymToString(XK_Shift_R));
		p->modifiers_stack->rem(p->modifiers_stack, XKeysymToString(XK_Shift_L));
		if (get_key_state(XK_Shift_R) != 0)
			p->modifiers_stack->add(p->modifiers_stack, XKeysymToString(XK_Shift_R));
		if (get_key_state(XK_Shift_L) != 0)
			p->modifiers_stack->add(p->modifiers_stack, XKeysymToString(XK_Shift_L));

		p->update_modifiers_stack(p);

		p->plugin->key_release(p->plugin, key, p->prev_key_mod);
		
		int user_action = get_user_action(key, modifier_mask);
		if (user_action >= 0)
		{
			p->perform_user_action(p, user_action);
			p->event->default_event.xkey.keycode = 0;
			return;
		}

		enum _hotkey_action manual_action = get_manual_action(key, modifier_mask);
		if (manual_action != ACTION_NONE)
		{
			if (p->perform_manual_action(p, manual_action))
				return;
			p->event->send_xkey(p->event, XKeysymToKeycode(main_window->display, key), modifier_mask);
		}
	}
}

static void program_perform_user_action(struct _program *p, int action)
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

static void program_perform_auto_action(struct _program *p, int action)
{
	struct _buffer *string = p->buffer;

	p->plugin->change_action(p->plugin, action);

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
			p->buffer->save_and_clear(p->buffer, p->focus->owner_window);
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

			char sym = main_window->keymap->get_cur_ascii_char(main_window->keymap, p->event->event);

			if (action == KLB_ADD_SYM)
			{
				if (p->changed_manual == MANUAL_FLAG_NEED_FLUSH)
					p->changed_manual = MANUAL_FLAG_UNSET;

				// Add symbol to internal bufer
				int modifier_mask = groups[get_active_keyboard_group()] | p->event->get_cur_modifiers(p->event);
				p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);

				// Block events of keyboard (push to event queue)
				set_event_mask(p->focus->owner_window, None);

				p->check_brackets_with_symbols(p);
				
				if (!xconfig->check_lang_on_process)
				{
					// Unblock keyboard
					set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);
					return;
				}

				// Checking word
				if (p->changed_manual == MANUAL_FLAG_UNSET)
					if (p->check_lang_last_syllable(p))
						p->event->default_event.xkey.keycode = 0;

				p->check_pattern(p);
				
				// Unblock keyboard
				set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);
				return;
			}

			// Block events of keyboard (push to event queue)
			set_event_mask(p->focus->owner_window, None);

			// Check two capital letter
			p->check_tcl_last_word(p);

			// Check incidental caps
			p->check_caps_last_word(p);

			// Checking word
			if (p->changed_manual == MANUAL_FLAG_UNSET)
				p->check_lang_last_word(p);

			p->add_word_to_pattern(p, get_cur_lang());
			// Add symbol to internal bufer
			p->event->event = p->event->default_event;
			int modifier_mask = groups[get_active_keyboard_group()] | p->event->get_cur_modifiers(p->event);
			p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);

			// Correct space before punctuation
			p->check_space_before_punctuation(p);

			// Correct spaces with brackets
			p->check_space_with_bracket(p);
			
			// Send Event
			p->event->event = p->event->default_event;
			p->event->send_next_event(p->event);
			p->event->default_event.xkey.keycode = 0;

			// Unblock keyboard
			set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);

			p->changed_manual = MANUAL_FLAG_NEED_FLUSH;

			if (action == KLB_ENTER && xconfig->flush_buffer_when_press_enter)
				p->buffer->save_and_clear(p->buffer, p->focus->owner_window);

			p->last_action = ACTION_NONE;
			
			return;
		}
	}
}

static int program_perform_manual_action(struct _program *p, enum _hotkey_action action)
{
	p->plugin->hotkey_action(p->plugin, action);
			
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

			set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);
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
		case ACTION_AUTOCOMPLEMENTATION:
		{
			if (p->last_action == ACTION_AUTOCOMPLEMENTATION)
			{
				// Block events of keyboard (push to event queue)
				set_event_mask(p->focus->owner_window, None);
				
				p->event->send_xkey(p->event, XKeysymToKeycode(main_window->display, XK_Left), p->event->event.xkey.state);
				p->event->send_xkey(p->event, XKeysymToKeycode(main_window->display, XK_Right), p->event->event.xkey.state);
				if (xconfig->add_space_after_autocomplementation)
					p->event->send_xkey(p->event, XKeysymToKeycode(main_window->display, XK_space), p->event->event.xkey.state);
				p->last_action = ACTION_NONE;

				p->buffer->save_and_clear(p->buffer, p->focus->owner_window);
				
				// Unblock keyboard
				set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);
				break;
			}
			p->event->send_xkey(p->event, p->event->event.xkey.keycode, p->event->event.xkey.state);
							
			p->event->event = p->event->default_event;
			char sym = main_window->keymap->get_cur_ascii_char(main_window->keymap, p->event->event);
			int modifier_mask =  p->event->get_cur_modifiers(p->event);
			p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);	

			break;
		}
		case ACTION_REPLACE_ABBREVIATION: // User needs to replace acronym
		{
			//MOVE this code to new function in new module

			char *utf_string = p->buffer->get_utf_string(p->buffer);

			// Check last word to acronym list
			char *word = get_last_word(utf_string);
			if (!word)
			{
				free(utf_string);
				return FALSE;
			}
			
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
					main_window->keymap->convert_text_to_ascii(main_window->keymap, replacement, dummy_kc, dummy_kc_mod);

					dummy_kc = realloc(dummy_kc, strlen(word) * sizeof(KeyCode));
					dummy_kc_mod = realloc(dummy_kc_mod, strlen(word) * sizeof(int));
					main_window->keymap->convert_text_to_ascii(main_window->keymap, word, dummy_kc, dummy_kc_mod);

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
				log_message(DEBUG, _("Found Abbreviation '%s' '%s'. Replacing to '%s'."), replacement, word, string);

				p->event->send_backspaces(p->event, strlen(get_last_word(p->buffer->content)));
				p->buffer->set_content(p->buffer, string);

				p->change_word(p, CHANGE_ABBREVIATION);

				set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);
				grab_spec_keys(p->focus->owner_window, TRUE);

				show_notify(NOTIFY_REPLACE_ABBREVIATION, NULL);
				p->buffer->save_and_clear(p->buffer, p->focus->owner_window);

				//Incapsulate to p->event->clear_code() or smth else
				p->event->default_event.xkey.keycode = 0;

				free(replacement);
				free(utf_string);
				return TRUE;
			}

			free(utf_string);
			return FALSE;
		}
	}

	// When CHANGE_STRING or CHANGE_WORD actions occured
	p->changed_manual = MANUAL_FLAG_SET;

	return TRUE;
}

static int program_check_lang_last_word(struct _program *p)
{
	if (p->app_forced_mode == FORCE_MODE_MANUAL)
		return FALSE;

	if (p->app_forced_mode != FORCE_MODE_AUTO && xconfig->is_manual_mode(xconfig))
		return FALSE;

	const char *word = get_last_word(p->buffer->content);
	if (!word)
		return FALSE;

	int cur_lang = get_cur_lang();
	int new_lang = check_lang(p->buffer, cur_lang);

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

static int program_check_lang_last_syllable(struct _program *p)
{
	if (p->app_forced_mode == FORCE_MODE_MANUAL)
		return FALSE;

	if (p->app_forced_mode != FORCE_MODE_AUTO && xconfig->is_manual_mode(xconfig))
		return FALSE;

	const char *word = get_last_word(p->buffer->content);
	if (!word)
		return FALSE;

	if (strlen(word) < 3)
		return FALSE;

	int cur_lang = get_cur_lang();
	int new_lang = check_lang(p->buffer, cur_lang);

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

static void program_check_caps_last_word(struct _program *p)
{
	if (!xconfig->correct_incidental_caps)
		return;
	
	int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

	if (!(p->buffer->keycode_modifiers[offset] & LockMask) || !(p->buffer->keycode_modifiers[offset] & ShiftMask))
		return;

	for (int i = 1; i < p->buffer->cur_pos - offset; i++)
	{
		if ((p->buffer->keycode_modifiers[offset + i] & LockMask) && (p->buffer->keycode_modifiers[offset+i] & ShiftMask))
			return;
		if (!(p->buffer->keycode_modifiers[offset + i] & LockMask))
			return;
	}

	p->change_word(p, CHANGE_INCIDENTAL_CAPS);
	show_notify(NOTIFY_CORR_INCIDENTAL_CAPS, NULL);
}

static void program_check_tcl_last_word(struct _program *p)
{
	if (!xconfig->correct_two_capital_letter)
		return;

	int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

	if (!isalpha(p->buffer->content[offset]))
		return;

	if (p->buffer->cur_pos - offset <= 2)
		return;

	if (isblank(p->buffer->content[offset+2]))
		return;

	if (!(p->buffer->keycode_modifiers[offset] & ShiftMask) || !(p->buffer->keycode_modifiers[offset + 1] & ShiftMask))
		return;

	for (int i = 2; i < p->buffer->cur_pos - offset; i++)
	{
		if ((p->buffer->keycode_modifiers[offset + i] & ShiftMask) && (isalpha(p->buffer->content[offset + i])))
			return;
	}

	p->change_word(p, CHANGE_TWO_CAPITAL_LETTER);
	show_notify(NOTIFY_CORR_TWO_CAPITAL_LETTER, NULL);
}

static void program_check_space_before_punctuation(struct _program *p)
{
	if (!xconfig->correct_space_with_punctuation)
		return;
	
	char *text = p->buffer->get_utf_string(p->buffer);
	if (text == NULL)
		return;

	if (p->buffer->cur_pos < 3)
	{
		free(text);
		return;
	}
	
	int text_len = strlen(text);
	if (text[text_len - 1] != '.' && text[text_len - 1] != ',' && text[text_len - 1] != '!' && 
	    text[text_len - 1] != '?' && text[text_len - 1] != ';' && text[text_len - 1] != ':')
	{
		free(text);
		return;
	}
	
	if (p->buffer->content[p->buffer->cur_pos-2] != ' ')
	{
		free(text);
		return;
	}
	
	log_message(DEBUG, _("Find spaces before punctuation, correction..."));

	p->event->send_backspaces(p->event, 1);
	p->buffer->del_symbol(p->buffer);
	while (p->buffer->content[p->buffer->cur_pos-2] == ' ')
	{
		p->event->send_backspaces(p->event, 1);
		p->buffer->del_symbol(p->buffer);
	}

	p->event->event = p->event->default_event;
	char sym = main_window->keymap->get_cur_ascii_char(main_window->keymap, p->event->event);
	int modifier_mask = groups[get_active_keyboard_group()] | p->event->get_cur_modifiers(p->event);
	p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);

	free(text);
}

static void program_check_space_with_bracket(struct _program *p)
{
	if (!xconfig->correct_space_with_punctuation)
		return;
	
	char *text = p->buffer->get_utf_string(p->buffer);
	if (text == NULL)
		return;

	if (p->buffer->cur_pos < 3)
	{
		free(text);
		return;
	}
	
	int text_len = strlen(text);
	if (text[text_len - 1] != '(' && text[text_len - 1] != ')')
	{
		free(text);
		return;
	}
	
	if (((text[text_len - 1] == '(') && (text[text_len - 2] == ' ' || text[text_len - 2] == ':' || text[text_len - 2] == ';' || text[text_len - 2] == '-' || isdigit(text[text_len - 2]))) ||
	    ((text[text_len - 1] == ')' && text[text_len - 2] != ' ' )))
	{
		free(text);
		return;
	}
	
	if (text[text_len - 1] == '(')
	{
		log_message(DEBUG, _("Find no space before left bracket, correction..."));
		
		p->buffer->del_symbol(p->buffer);
		p->event->event = p->event->default_event;
		p->event->event.xkey.keycode = XKeysymToKeycode(main_window->display, XK_space);
		p->event->send_next_event(p->event);
		int modifier_mask = groups[get_active_keyboard_group()];
		p->buffer->add_symbol(p->buffer, ' ', p->event->event.xkey.keycode, modifier_mask);

		p->event->event = p->event->default_event;
		char sym = main_window->keymap->get_cur_ascii_char(main_window->keymap, p->event->event);
		modifier_mask |=  p->event->get_cur_modifiers(p->event);
		p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);	
	}

	if (text[text_len - 1] == ')')
	{
		log_message(DEBUG, _("Find spaces before right bracket, correction..."));

		p->buffer->del_symbol(p->buffer);
		while (p->buffer->content[p->buffer->cur_pos - 1] == ' ')
		{
			p->event->send_backspaces(p->event, 1);
			p->buffer->del_symbol(p->buffer);
		}
		p->event->event = p->event->default_event;
		char sym = main_window->keymap->get_cur_ascii_char(main_window->keymap, p->event->event);
		int modifier_mask = groups[get_active_keyboard_group()] | p->event->get_cur_modifiers(p->event);
		p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);	
	}

	free(text);
}

static void program_check_brackets_with_symbols(struct _program *p)
{
	if (!xconfig->correct_space_with_punctuation)
		return;
	
	if (p->buffer->content[p->buffer->cur_pos - 2] == ')')
	{
		log_message(DEBUG, _("Find no spaces after right bracket, correction..."));
		
		p->buffer->del_symbol(p->buffer);
		p->event->event = p->event->default_event;
		p->event->event.xkey.keycode = XKeysymToKeycode(main_window->display, XK_space);
		p->event->send_next_event(p->event);
		int modifier_mask = groups[get_active_keyboard_group()];
		p->buffer->add_symbol(p->buffer, ' ', p->event->event.xkey.keycode, modifier_mask);

		p->event->event = p->event->default_event;
		char sym = main_window->keymap->get_cur_ascii_char(main_window->keymap, p->event->event);
		modifier_mask = groups[get_active_keyboard_group()] | p->event->get_cur_modifiers(p->event);
		p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);	
	}

	if (p->buffer->content[p->buffer->cur_pos - 2] != ' ')
		return;

	int space_count = 0;
	int pos = p->buffer->cur_pos - 2;
	while ((pos >= 0) && (p->buffer->content[pos] == ' '))
	{
		space_count++;
		pos--;
	} 
	
	if (pos < 0 || p->buffer->content[pos] != '(')
	    return;
	    
	log_message(DEBUG, _("Find spaces after left bracket, correction..."));

	p->buffer->del_symbol(p->buffer);
	for (int i = 0; i < space_count; i++)
	{
		p->event->send_backspaces(p->event, 1);
		p->buffer->del_symbol(p->buffer);
	}
	p->event->event = p->event->default_event;
	char sym = main_window->keymap->get_cur_ascii_char(main_window->keymap, p->event->event);
	int modifier_mask = groups[get_active_keyboard_group()] | p->event->get_cur_modifiers(p->event);
	p->buffer->add_symbol(p->buffer, sym, p->event->event.xkey.keycode, modifier_mask);	
}

static void program_check_pattern(struct _program *p)
{
	if (!xconfig->autocomplementation)
		return;

	if (p->app_autocomplementation_mode == AUTOCOMPLEMENTATION_EXCLUDED)
		return;
	
	char *tmp = get_last_word(p->buffer->content);
	if (tmp == NULL)
		return;
	
	if (strlen(tmp) < MIN_PATTERN_LEN - 1)
		return;
	
	int lang = get_cur_lang();
	tmp = get_last_word(p->buffer->i18n_content[lang].content);

	char *word = strdup(tmp);

	int len = trim_word(word, strlen(tmp));
	if (len == 0)
	{
		free (word);
		return;
	}

	struct _list_char_data *pattern_data = xconfig->languages[lang].pattern->find_alike(xconfig->languages[lang].pattern, word);
	if (pattern_data == NULL)
	{
		p->last_action = ACTION_NONE;
		free (word);
		return;
	}
	
	log_message (DEBUG, "Recognition word '%s', autocompletation...", pattern_data->string);

	set_event_mask(p->focus->owner_window, None);
	grab_spec_keys(p->focus->owner_window, FALSE);

	struct _buffer *tmp_buffer = buffer_init();
	
	tmp_buffer->set_content(tmp_buffer, pattern_data->string + strlen(word)*sizeof(char));

	if (tmp_buffer->cur_pos == 0)
	{
		tmp_buffer->uninit(tmp_buffer);
		p->last_action = ACTION_NONE;
		free (word);
		return;
	}

	p->event->event = p->event->default_event;
	p->event->send_next_event(p->event);
	
	p->event->send_string(p->event, tmp_buffer);
	p->event->send_selection(p->event, tmp_buffer->cur_pos);

	p->event->default_event.xkey.keycode = 0;
	
	tmp_buffer->uninit(tmp_buffer);
	
	set_event_mask(p->focus->owner_window, INPUT_HANDLE_MASK | FOCUS_CHANGE_MASK | EVENT_KEY_MASK);
	grab_spec_keys(p->focus->owner_window, TRUE);

	p->last_action = ACTION_AUTOCOMPLEMENTATION;
	free (word);
}

static void program_send_string_silent(struct _program *p, int send_backspaces)
{
	if (p->buffer->cur_pos == 0)
	{
		log_message(DEBUG, _("No string to change"));
		return;
	}

	log_message(DEBUG, _("Processing string '%s'"), p->buffer->content);

	p->event->send_backspaces(p->event, send_backspaces);		// Delete old string
	p->event->send_string(p->event, p->buffer);		// Send new string
}

static void program_change_word(struct _program *p, enum _change_action action)
{
	switch (action)
	{
		case CHANGE_INCIDENTAL_CAPS:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_incidental_caps(p);

			p->send_string_silent(p, p->buffer->cur_pos);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_TWO_CAPITAL_LETTER:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_two_capital_letter(p);

			p->send_string_silent(p, p->buffer->cur_pos);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_0:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 0);

			p->send_string_silent(p, p->buffer->cur_pos);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_1:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 1);

			p->send_string_silent(p, p->buffer->cur_pos);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_2:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 2);

			p->send_string_silent(p, p->buffer->cur_pos);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_WORD_TO_LAYOUT_3:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 3);

			p->send_string_silent(p, p->buffer->cur_pos);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_0:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 0);

			p->send_string_silent(p, p->buffer->cur_pos - 1);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_1:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 1);

			p->send_string_silent(p, p->buffer->cur_pos - 1);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_2:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 2);

			p->send_string_silent(p, p->buffer->cur_pos - 1);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
			break;
		}
		case CHANGE_SYLL_TO_LAYOUT_3:
		{
			int offset = get_last_word_offset(p->buffer->content, p->buffer->cur_pos);

			// Shift fields to point to begin of word
			p->buffer->set_offset(p->buffer, offset);

			p->change_lang(p, 3);

			p->send_string_silent(p, p->buffer->cur_pos - 1);

			// Revert fields back
			p->buffer->unset_offset(p->buffer, offset);
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

			p->send_string_silent(p, p->buffer->cur_pos);
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_1:
		{
			p->change_lang(p, 1);
			p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);	// Disable receiving events

			p->send_string_silent(p, p->buffer->cur_pos);
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_2:
		{
			p->change_lang(p, 2);
			p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);	// Disable receiving events

			p->send_string_silent(p, p->buffer->cur_pos);
			break;
		}
		case CHANGE_STRING_TO_LAYOUT_3:
		{
			p->change_lang(p, 3);
			p->focus->update_events(p->focus, LISTEN_DONTGRAB_INPUT);	// Disable receiving events

			p->send_string_silent(p, p->buffer->cur_pos);
			break;
		}
		case CHANGE_ABBREVIATION:
		{
			p->send_string_silent(p, 0);
			break;
		}
	}
}

static void program_add_word_to_dict(struct _program *p, int new_lang)
{
	char *tmp = get_last_word(p->buffer->content);
	if (tmp == NULL)
		return;
	
	int curr_lang = get_cur_lang();

	tmp = get_last_word(p->buffer->i18n_content[curr_lang].content);

	char *curr_word = strdup(tmp);

	int len = trim_word(curr_word, strlen(tmp));
	if (len == 0)
	{
		free(curr_word);
		return;
	}

	struct _list_char *curr_temp_dict = xconfig->languages[curr_lang].temp_dict;
	if (curr_temp_dict->exist(curr_temp_dict, curr_word, BY_PLAIN))
		curr_temp_dict->rem(curr_temp_dict, curr_word);

	struct _list_char *new_temp_dict = xconfig->languages[new_lang].temp_dict;

	tmp = get_last_word(p->buffer->i18n_content[new_lang].content);

	char *new_word = strdup(tmp);

	len = trim_word(new_word, strlen(tmp));
	if (len == 0)
	{
		free(curr_word);
		free(new_word);
		return;
	}

	if (!new_temp_dict->exist(new_temp_dict, new_word, BY_PLAIN))
	{
		new_temp_dict->add(new_temp_dict, new_word);
		free(curr_word);
		free(new_word);
		return;
	}

	struct _list_char *curr_dict = xconfig->languages[curr_lang].dict;
	if (curr_dict->exist(curr_dict, curr_word, BY_PLAIN))
	{
		log_message(DEBUG, _("Remove word '%s' from %s dictionary"), curr_word, xconfig->get_lang_name(xconfig, curr_lang));
		curr_dict->rem(curr_dict, curr_word);
		xconfig->save_dict(xconfig, curr_lang);
	}

	struct _list_char *new_dict = xconfig->languages[new_lang].dict;
	if (!new_dict->exist(new_dict, new_word, BY_PLAIN))
	{
		log_message(DEBUG, _("Add word '%s' in %s dictionary"), new_word, xconfig->get_lang_name(xconfig, new_lang));
		new_dict->add(new_dict, new_word);
		xconfig->save_dict(xconfig, new_lang);
	}

	p->add_word_to_pattern(p, new_lang);
		
	free(curr_word);
	free(new_word);
}

static void program_add_word_to_pattern(struct _program *p, int new_lang)
{
	if (!xconfig->autocomplementation)
		return;
	
	char *tmp = get_last_word(p->buffer->content);
	if (tmp == NULL)
		return;

	if (strlen(tmp) < MIN_PATTERN_LEN)
		return;
	
	tmp = get_last_word(p->buffer->i18n_content[new_lang].content);

	char *new_word = strdup(tmp);

	int len = trim_word(new_word, strlen(tmp));
	if (len == 0)
	{
		free(new_word);
		return;
	}

	if (isdigit(new_word[len-1]) || ispunct(new_word[len-1]))
	{
		free(new_word);
		return;
	}
	
	for (int i = 0; i < xconfig->total_languages; i++)
	{
		if (i == new_lang)
			continue;
		
		tmp = get_last_word(p->buffer->i18n_content[i].content);
		char *old_word = strdup(tmp);

		len = trim_word(old_word, strlen(tmp));
		if (len == 0)
		{
			free (old_word);
			continue;
		}
		struct _list_char *old_pattern = xconfig->languages[i].pattern;
		if (old_pattern->exist(old_pattern, old_word, BY_PLAIN))
		{
			log_message(DEBUG, _("Remove word '%s' from %s pattern"), old_word, xconfig->get_lang_name(xconfig, i));
			old_pattern->rem(old_pattern, old_word);
			xconfig->save_pattern(xconfig, i);
		}
		free (old_word);
	}
	
	struct _list_char *new_pattern = xconfig->languages[new_lang].pattern;
	if (!new_pattern->exist(new_pattern, new_word, BY_PLAIN))
	{
		log_message(DEBUG, _("Add word '%s' in %s pattern"), new_word, xconfig->get_lang_name(xconfig, new_lang));
		new_pattern->add(new_pattern, new_word);
		xconfig->save_pattern(xconfig, new_lang);
	}

	free(new_word);
}

static void program_uninit(struct _program *p)
{
	p->focus->uninit(p->focus);
	p->event->uninit(p->event);
	p->buffer->uninit(p->buffer);
	p->plugin->uninit(p->plugin);
	
	main_window->uninit(main_window);

	p->modifiers_stack->uninit(p->modifiers_stack);
	
	free(p);

	log_message(DEBUG, _("Program is freed"));
}

struct _program* program_init(void)
{
	struct _program *p = (struct _program*) malloc(sizeof(struct _program));
	bzero(p, sizeof(struct _program));

	main_window = window_init();

	if (!main_window->create(main_window) || !main_window->init_keymap(main_window))
	{
		free(p);
		return NULL;
	}
	
	p->event			= event_init();			// X Event processor
	p->focus			= focus_init();			// X Input Focus and Pointer processor
	p->buffer			= buffer_init();		// Input string buffer
	
	p->plugin			= plugin_init();
	for (int i=0; i<xconfig->plugins->data_count; i++)
	{
		p->plugin->add(p->plugin, xconfig->plugins->data[i].string);
	}
	
	p->modifiers_stack	= list_char_init();
	
	// Function mapping
	p->uninit			= program_uninit;
	p->layout_update		= program_layout_update;
	p->update			= program_update;
	p->update_modifiers_stack	= program_update_modifiers_stack;
	p->on_key_action		= program_on_key_action;
	p->process_input		= program_process_input;
	p->perform_auto_action		= program_perform_auto_action;
	p->perform_manual_action	= program_perform_manual_action;
	p->perform_user_action		= program_perform_user_action;
	p->check_lang_last_word		= program_check_lang_last_word;
	p->check_lang_last_syllable	= program_check_lang_last_syllable;
	p->check_caps_last_word		= program_check_caps_last_word;
	p->check_tcl_last_word		= program_check_tcl_last_word;
	p->check_space_before_punctuation	= program_check_space_before_punctuation;
	p->check_space_with_bracket	= program_check_space_with_bracket;
	p->check_brackets_with_symbols = program_check_brackets_with_symbols;
	p->check_pattern	= program_check_pattern;
	p->change_word			= program_change_word;
	p->add_word_to_dict		= program_add_word_to_dict;
	p->add_word_to_pattern		= program_add_word_to_pattern;
	p->process_selection_notify	= program_process_selection_notify;
	p->change_lang			= program_change_lang;
	p->change_incidental_caps	= program_change_incidental_caps;
	p->change_two_capital_letter	= program_change_two_capital_letter;
	p->send_string_silent		= program_send_string_silent;

	return p;
}
