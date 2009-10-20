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
#  include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#ifdef WITH_ASPELL
#  include <aspell.h>
#endif

#include "xnconfig.h"

#include "switchlang.h"

#include "keymap.h"
#include "window.h"

#include "types.h"
#include "utils.h"
#include "list_char.h"
#include "log.h"
#include "text.h"

#include "detection.h"

#define PROTO_LEN	2
#define BIG_PROTO_LEN	3

extern struct _xneur_config *xconfig;
extern struct _window *main_window;

static int is_fixed_layout(int cur_lang)
{
	return xconfig->languages[cur_lang].fixed;
}

static int get_dict_lang(char **word)
{
	for (int lang = 0; lang < xconfig->total_languages; lang++)
	{
		if (is_fixed_layout(lang))
			continue;

		if (xconfig->languages[lang].dict->exist(xconfig->languages[lang].dict, word[lang], BY_PLAIN))
		{
			log_message(DEBUG, _("   [+] Found this word in %s language dictionary"), xconfig->get_lang_name(xconfig, lang));
			return lang;
		}
	}

	log_message(DEBUG, _("   [-] This word not found in any dictionaries"));
	return NO_LANGUAGE;
}

static int get_regexp_lang(char **word)
{
	for (int lang = 0; lang < xconfig->total_languages; lang++)
	{
		if (is_fixed_layout(lang))
			continue;

		if (xconfig->languages[lang].regexp->exist(xconfig->languages[lang].regexp, word[lang], BY_REGEXP))
		{
			log_message(DEBUG, _("   [+] Found this word in %s language regular expressions file"), xconfig->get_lang_name(xconfig, lang));
			return lang;
		}
	}

	log_message(DEBUG, _("   [-] This word not found in any regular expressions files"));
	return NO_LANGUAGE;
}

#ifdef WITH_ASPELL
static int get_aspell_hits(char **word, int len)
{
	AspellConfig *spell_config = new_aspell_config();

	for (int lang = 0; lang < xconfig->total_languages; lang++)
	{
		if (is_fixed_layout(lang))
			continue;

		if (len < 2)
			continue;

		aspell_config_replace(spell_config, "lang", xconfig->languages[lang].dir);
		AspellCanHaveError *possible_err = new_aspell_speller(spell_config);

		if (aspell_error_number(possible_err) != 0)
		{
			log_message(DEBUG, _("   [!] Error aspell checking for %s aspell dictionary"), xconfig->get_lang_name(xconfig, lang));
			continue;
		}

		AspellSpeller *spell_checker = to_aspell_speller(possible_err);
		int correct = aspell_speller_check(spell_checker, word[lang], strlen(word[lang]));
		delete_aspell_speller(spell_checker);
		if (correct)
		{
			log_message(DEBUG, _("   [+] Found this word in %s aspell dictionary"), xconfig->get_lang_name(xconfig, lang));
			return lang;
		}
	}

	log_message(DEBUG, _("   [-] This word has no hits for all aspell dictionaries"));
	return NO_LANGUAGE;
}
#endif

static int get_proto_hits(char *word, int *sym_len, int len, int offset, int lang)
{
	int n_bytes = 0;
	for (int i = 0; i < PROTO_LEN; i++)
		n_bytes += sym_len[i];

	char *proto = (char *) malloc((n_bytes + 1) * sizeof(char));

	int local_offset = 0;
	for (int i = 0; i <= len - offset - PROTO_LEN; i++)
	{
		strncpy(proto, word + local_offset, n_bytes);
		proto[n_bytes] = NULLSYM;

		if (xconfig->languages[lang].proto->exist(xconfig->languages[lang].proto, proto, BY_PLAIN))
		{
			free(proto);
			return TRUE;
		}

		local_offset += sym_len[i];
	}

	free(proto);
	return FALSE;
}

static int get_big_proto_hits(char *word, int *sym_len, int len, int offset, int lang)
{
	int n_bytes = 0;
	for (int i = 0; i < BIG_PROTO_LEN; i++)
		n_bytes += sym_len[i];

	char *proto = (char *) malloc((n_bytes + 1) * sizeof(char));

	int local_offset = 0;
	for (int i = 0; i <= len - offset - BIG_PROTO_LEN; i++)
	{
		strncpy(proto, word+local_offset, n_bytes);
		proto[n_bytes] = NULLSYM;

		if (xconfig->languages[lang].proto->exist(xconfig->languages[lang].big_proto, proto, BY_PLAIN))
		{
			free(proto);
			return TRUE;
		}

		local_offset += sym_len[i];
	}

	free(proto);
	return FALSE;
}

static int get_proto_lang(char **word, int **sym_len, int len, int offset, int cur_lang, int proto_len)
{
	int (*get_proto_hits_function) (char *word, int *sym_len, int len, int offset, int lang);

	if (proto_len == PROTO_LEN)
		get_proto_hits_function = get_proto_hits;
	else
		get_proto_hits_function = get_big_proto_hits;

	if (len < proto_len)
	{
		log_message(DEBUG, _("   [-] Skip checking by language proto of size %d (word is very short)"), proto_len);
		return NO_LANGUAGE;
	}

	int hits = get_proto_hits_function(word[cur_lang], sym_len[cur_lang], len, offset, cur_lang);
	if (hits == 0)
	{
		log_message(DEBUG, _("   [-] This word is ok for %s proto of size %d"), xconfig->get_lang_name(xconfig, cur_lang), proto_len);
		return NO_LANGUAGE;
	}

	log_message(DEBUG, _("   [*] This word has hits for %s proto of size %d"), xconfig->get_lang_name(xconfig, cur_lang), proto_len);

	for (int lang = 0; lang < xconfig->total_languages; lang++)
	{
		if ((lang == cur_lang) || (is_fixed_layout(lang)))
			continue;

		int hits = get_proto_hits_function(word[lang], sym_len[lang], len, offset, lang);
		if (hits != 0)
		{
			log_message(DEBUG, _("   [*] This word has hits for %s language proto of size %d"), xconfig->get_lang_name(xconfig, lang), proto_len);
			continue;
		}

		log_message(DEBUG, _("   [+] This word has no hits for %s language proto of size %d"), xconfig->get_lang_name(xconfig, lang), proto_len);
		return lang;
	}

	log_message(DEBUG, _("   [-] This word has hits in all languages proto of size %d"), proto_len);
	return NO_LANGUAGE;
}

int check_lang(struct _buffer *p, int cur_lang)
{
	if (is_fixed_layout(cur_lang))
		return NO_LANGUAGE;

	int group = get_active_keyboard_group();
	if (xconfig->find_group_lang(xconfig, group) == -1)
		return NO_LANGUAGE;

	char **word = (char **) malloc((xconfig->total_languages + 1) * sizeof(char *));
	int **sym_len = (int **) malloc((xconfig->total_languages + 1) * sizeof(int *));

	for (int i = 0; i < xconfig->total_languages; i++)
	{
		word[i] = get_last_word(p->i18n_content[i].content);
		del_final_numeric_char(word[i]);
		
		log_message(DEBUG, _("Processing word '%s'"), word[i]);

		sym_len[i] = p->i18n_content[i].symbol_len + get_last_word_offset(p->content, strlen(p->content));
	}

	// Check by regexp
	int lang = get_regexp_lang(word);

	// Check by dictionary
	if (lang == NO_LANGUAGE)
		lang = get_dict_lang(word);

	int len = strlen(get_last_word(p->content));
#ifdef WITH_ASPELL
	// Check by aspell
	if (lang == NO_LANGUAGE)
		lang = get_aspell_hits(word, len);
#endif

	// If not found in dictionary, try to find in proto
	len = strlen(p->content);
	int offset = get_last_word_offset(p->content, len);
	if (lang == NO_LANGUAGE)
		lang = get_proto_lang(word, sym_len, len, offset, cur_lang, PROTO_LEN);

	if (lang == NO_LANGUAGE)
		lang = get_proto_lang(word, sym_len, len, offset, cur_lang, BIG_PROTO_LEN);

	log_message(DEBUG, _("End word processing"));

	free(word);
	free(sym_len);
	return lang;
}

int get_next_lang(int cur_lang)
{
	int next_lang = cur_lang + 1;
	if (next_lang >= xconfig->total_languages)
		next_lang = 0;

	return next_lang;
}
