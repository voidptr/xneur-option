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

#include <X11/Xlocale.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "xnconfig.h"

#include "event.h"
#include "switchlang.h"
#include "window.h"

#include "types.h"
#include "text.h"
#include "log.h"

#include "keymap.h"

extern struct _xneur_config *xconfig;
extern struct _window *main_window;

static const int keyboard_groups[]	= {0x00000000, 0x00002000, 0x00004000, 0x00006000};
static const int state_masks[]		= {0x00, 0x01, 0x80, 0x10}; // None, NumLock, Alt, Shift

static const int max_groups_count	= sizeof(keyboard_groups) / sizeof(keyboard_groups[0]);

static int locale_create(void)
{
	if (setlocale(LC_ALL, "") == NULL)
	{
		log_message(ERROR, _("Couldn't set default locale"));
		return FALSE;
	}

	char *locale = setlocale(LC_CTYPE, "");
	if (locale == NULL || (strstr(locale, "UTF") == NULL && strstr(locale, "utf") == NULL) )
		log_message(WARNING, _("Your default locale is not UTF-8"));

	log_message(LOG, _("Using locale %s"), locale);
	return TRUE;
}

int get_languages_mask(void)
{
	int languages_mask = 0;
	for (int group = 0; group < max_groups_count; group++)
		languages_mask = languages_mask | keyboard_groups[group];
	return languages_mask;
}

char* keycode_to_symbol(KeyCode kc, int group, int state)
{
	XEvent event = create_basic_event();
	event.xkey.keycode = kc;
	event.xkey.state = 0;
	if (group >= 0)
		event.xkey.state = keyboard_groups[group];
	event.xkey.state |= state;

	char *symbol = (char *) malloc((256 + 1) * sizeof(char));

	int nbytes = XLookupString((XKeyEvent *) &event, symbol, 256, NULL, NULL);
	if (nbytes <= 0)
		return NULL;
	symbol[nbytes] = NULLSYM;

	return symbol;
}

int get_keycode_mod(int group)
{
	return keyboard_groups[group];
}

void get_keysyms_by_string(char *keyname, KeySym *lower, KeySym *upper)
{
	Display *display = XOpenDisplay(NULL);

	if (keyname == NULL)
	{
		*lower = NoSymbol;
		*upper = NoSymbol;
		return;
	}

	KeySym inbound_key = XStringToKeysym(keyname);

	int min_keycode, max_keycode;
	XDisplayKeycodes(display, &min_keycode, &max_keycode);

	int keysyms_per_keycode;
	KeySym *keymap = XGetKeyboardMapping(display, min_keycode, max_keycode - min_keycode + 1, &keysyms_per_keycode);

	XCloseDisplay(display);

	for (int i = min_keycode; i <= max_keycode; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (keymap[j] == NoSymbol)
				continue;

			if (keymap[j] != inbound_key)
				continue;

			*lower = keymap[0];
			*upper = keymap[1];
			return;
		}

		keymap += keysyms_per_keycode;
	}
}

// Private
static int init_keymaps(struct _keymap *p)
{
	p->keyboard_groups_count = get_keyboard_groups_count();
	if (p->keyboard_groups_count > max_groups_count)
	{
		log_message(ERROR, _("Too many keyboard layouts (max %d)"), max_groups_count);
		return FALSE;
	}

	// Define all key codes and key symbols
	XDisplayKeycodes(main_window->display, &(p->min_keycode), &(p->max_keycode));

	p->keymap = XGetKeyboardMapping(main_window->display, p->min_keycode, p->max_keycode - p->min_keycode + 1, &(p->keysyms_per_keycode));
	if (!p->keymap)
	{
		log_message(ERROR, _("Unable to get keyboard mapping table"));
		return FALSE;
	}
	return TRUE;
}

static void keymap_char_to_keycode(struct _keymap *p, char ch, KeyCode *kc, int *modifier)
{
	if (ch == 10 || ch == 13)
	{
		*kc		= XKeysymToKeycode(main_window->display, XK_Return);
		*modifier	= 0;
		return;
	}

	XEvent event = create_basic_event();
	char *symbol = (char *) malloc((256 + 1) * sizeof(char));

	for (int i = p->min_keycode + 1; i <= p->max_keycode; i++)
	{
		event.xkey.keycode	= i;
		event.xkey.state	= 0;

		int nbytes = XLookupString((XKeyEvent *) &event, symbol, 256, NULL, NULL);
		if ((nbytes > 0) && (symbol[0] == ch))
			break;

		event.xkey.state	= ShiftMask;

		nbytes = XLookupString((XKeyEvent *) &event, symbol, 256, NULL, NULL);
		if ((nbytes > 0) && (symbol[0] == ch))
			break;
	}

	*kc		= event.xkey.keycode;
	*modifier	= (event.xkey.state == ShiftMask) ? 1 : 0;
	free(symbol);
}

static char keymap_get_ascii(struct _keymap *p, const char *sym, KeyCode *kc, int *modifier)
{
	XEvent event		= create_basic_event();

	char *symbol		= (char *) malloc((256 + 1) * sizeof(char));
	char *prev_symbols	= (char *) malloc((256 + 1) * sizeof(char));

	for (int lang = 0; lang < p->keyboard_groups_count; lang++)
	{
		if (lang == p->latin_group)
			continue;

		KeySym *keymap = p->keymap;
		for (int i = p->min_keycode; i <= p->max_keycode; i++)
		{
			int max = p->keysyms_per_keycode - 1;
			while (max >= 0 && keymap[max] == NoSymbol)
				max--;

			prev_symbols[0] = NULLSYM;

			for (int j = 0; j <= max; j++)
			{
				if (keymap[j] == NoSymbol)
					continue;

				for (int n = 0; n < 3; n++)
				{
					for (int m = 0; m < 3; m++) // Modifiers
					{
						event.xkey.keycode	= i;

						event.xkey.state	= get_keycode_mod(lang);
						event.xkey.state	|= state_masks[m];
						event.xkey.state	|= state_masks[n];

						int nbytes = XLookupString((XKeyEvent *) &event, symbol, 256, NULL, NULL);
						if (nbytes <= 0)
							continue;

						symbol[nbytes] = NULLSYM;

						if (strstr(prev_symbols, symbol) != NULL)
							continue;
						strcat(prev_symbols, symbol);

						if (strncmp(sym, symbol, strlen(symbol)) != 0)
							continue;

						event.xkey.state = 0;
						event.xkey.state |= state_masks[m];
						event.xkey.state |= state_masks[n];

						nbytes = XLookupString((XKeyEvent *) &event, symbol, 256, NULL, NULL);
						if (nbytes <= 0)
							continue;

						char sym = symbol[0];

						free(prev_symbols);
						free(symbol);
						*kc		= event.xkey.keycode;
						*modifier	= get_keycode_mod(lang) | event.xkey.state;
						return sym;
					}
				}
			}
			keymap += p->keysyms_per_keycode;
		}
	}

	free(prev_symbols);
	free(symbol);
	return NULLSYM;
}

static char keymap_get_cur_ascii_char(struct _keymap *p, XEvent e)
{
	XKeyEvent *ke = (XKeyEvent *) &e;

	int mod = 0;
	if (ke->state & ShiftMask)
		mod = ShiftMask;

	char *symbol = (char *) malloc((256 + 1) * sizeof(char));

	ke->state = get_keycode_mod(p->latin_group);
	ke->state |= mod;

	int nbytes = XLookupString(ke, symbol, 256, NULL, NULL);
	if (nbytes > 0)
	{
		char sym = symbol[0];
		free(symbol);
		return sym;
	}

	free(symbol);
	return ' ';
}

static void keymap_convert_text_to_ascii(struct _keymap *p, char *text, KeyCode *kc, int *kc_mod)
{
	int text_len = strlen(text);

	int j = 0;
	for (int i = 0; i < text_len; i++)
	{
		if (isascii(text[i]) || isspace(text[i]))
		{
			p->char_to_keycode(p, text[i], &kc[j], &kc_mod[j]);
			text[j++] = text[i];
			continue;
		}

		char new_symbol = p->get_ascii(p, &text[i], &kc[j], &kc_mod[j]);

		for(; i < text_len - 1; i++)
		{
			if (isascii(text[i + 1]) || isspace(text[i + 1]))
				break;

			if (p->get_ascii(p, &text[i + 1], &kc[i + 1], &kc_mod[i + 1]) != NULLSYM)
				break;
		}

		text[j++] = new_symbol;
	}

	text[j] = NULLSYM;
}

static char* keymap_lower_by_keymaps(struct _keymap *p, int gr, char *text)
{
	if (text == NULL)
		return NULL;

	char *symbol_old	= (char *) malloc((256 + 1) * sizeof(char));
	char *symbol_new	= (char *) malloc((256 + 1) * sizeof(char));
	char *prev_symbols	= (char *) malloc((256 + 1) * sizeof(char));

	char *newtext = strdup(text);

	KeySym *keymap = p->keymap;
	for (int i = p->min_keycode; i <= p->max_keycode; i++)
	{
		int max = p->keysyms_per_keycode - 1;
		while (max >= 0 && keymap[max] == NoSymbol)
			max--;

		prev_symbols[0] = NULLSYM;

		for (int j = 1; j <= max; j += 2)
		{
			if (keymap[j] == NoSymbol)
				continue;

			for (int m = 0; m < 4; m++) // Modifiers
			{
				for (int n = 0; n < 4; n++)
				{
					if (n == m)
						continue;

					// Get BIG symbol from keymap
					XEvent event		= create_basic_event();
					event.xkey.keycode	= i;
					event.xkey.state	= get_keycode_mod(gr);
					event.xkey.state	|= state_masks[m];
					event.xkey.state	|= state_masks[n];

					int nbytes = XLookupString((XKeyEvent *) &event, symbol_old, 256, NULL, NULL);
					if (nbytes <= 0)
						continue;

					symbol_old[nbytes] = NULLSYM;

					if (strstr(prev_symbols, symbol_old) != NULL)
						continue;

					// Get small symbol
					event.xkey.state	= 0;
					event.xkey.state	|= state_masks[m];
					event.xkey.state	|= state_masks[n];
					event.xkey.state	&= ~ShiftMask;

					nbytes = XLookupString((XKeyEvent *) &event, symbol_new, 256, NULL, NULL);
					if (nbytes <= 0)
						continue;

					symbol_new[nbytes] = NULLSYM;

					char *replaced = str_replace(newtext, symbol_old, symbol_new);

					free(newtext);
					newtext = replaced;
				}
			}
		}
		keymap += p->keysyms_per_keycode;
	}

	free(prev_symbols);
	free(symbol_new);
	free(symbol_old);

	return newtext;
}

static void keymap_uninit(struct _keymap *p)
{
	if (p->keymap != NULL)
		XFree(p->keymap);
	free(p);

	log_message(DEBUG, _("Keymap is freed"));
}

struct _keymap* keymap_init(void)
{
	struct _keymap *p = (struct _keymap *) malloc(sizeof(struct _keymap));
	bzero(p, sizeof(struct _keymap));

	if (!locale_create() || !init_keymaps(p))
	{
		free(p);
		return NULL;
	}

	p->get_ascii			= keymap_get_ascii;
	p->get_cur_ascii_char		= keymap_get_cur_ascii_char;
	p->convert_text_to_ascii	= keymap_convert_text_to_ascii;
	p->char_to_keycode		= keymap_char_to_keycode;
	p->lower_by_keymaps		= keymap_lower_by_keymaps;
	p->uninit			= keymap_uninit;

	return p;
}
