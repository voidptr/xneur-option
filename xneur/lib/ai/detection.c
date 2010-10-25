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
#  include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

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

static int get_dict_lang(struct _xneur_handle *handle, char **word)
{
	for (int lang = 0; lang < handle->total_languages; lang++)
	{
		if (handle->languages[lang].excluded)
			continue;

		if (handle->languages[lang].dict->exist(handle->languages[lang].dict, word[lang], BY_PLAIN))
		{
			log_message(DEBUG, _("   [+] Found this word in %s language dictionary"), handle->languages[lang].name);
			return lang;
		}
	}

	log_message(DEBUG, _("   [-] This word not found in any dictionaries"));
	return NO_LANGUAGE;
}

static int get_regexp_lang(struct _xneur_handle *handle, char **word)
{
	for (int lang = 0; lang < handle->total_languages; lang++)
	{
		if (handle->languages[lang].excluded)
			continue;

		if (handle->languages[lang].regexp->exist(handle->languages[lang].regexp, word[lang], BY_REGEXP))
		{
			log_message(DEBUG, _("   [+] Found this word in %s language regular expressions file"), handle->languages[lang].name);
			return lang;
		}
	}

	log_message(DEBUG, _("   [-] This word not found in any regular expressions files"));
	return NO_LANGUAGE;
}

#ifdef WITH_ASPELL
static int get_aspell_hits(struct _xneur_handle *handle, char **word, int len, int cur_lang)
{
	if (len >= 2)
	{
		// check for current language first
		if (handle->has_spell_checker[cur_lang])
		{
			if (!handle->languages[cur_lang].excluded && 
				aspell_speller_check(handle->spell_checkers[cur_lang], word[cur_lang], strlen(word[cur_lang])))
			{
				log_message(DEBUG, _("   [+] Found this word in %s aspell dictionary"), handle->languages[cur_lang].name);
				return cur_lang;
			}
		}
		else
		{
			log_message(DEBUG, _("   [!] Now we don't support aspell dictionary for %s layout"), handle->languages[cur_lang].name);
		}
		

		// check for another languages
		for (int lang = 0; lang < handle->total_languages; lang++)
		{
			if (handle->languages[lang].excluded || lang == cur_lang)
				continue;
			
			if (!handle->has_spell_checker[lang])
			{
				log_message(DEBUG, _("   [!] Now we don't support aspell dictionary for %s layout"), handle->languages[lang].name);
				continue;
			}
			
			if (aspell_speller_check(handle->spell_checkers[lang], word[lang], strlen(word[lang])))
			{
				log_message(DEBUG, _("   [+] Found this word in %s aspell dictionary"), handle->languages[lang].name);
				return lang;
			}
		}
	}

	log_message(DEBUG, _("   [-] This word has no hits for all aspell dictionaries"));
	return NO_LANGUAGE;
}
#endif

#ifdef WITH_ENCHANT
static int get_enchant_hits(struct _xneur_handle *handle, char **word, int len, int cur_lang)
{
	if (len >= 2)
	{
		// check for current language first
		if (handle->has_enchant_checker[cur_lang])
		{
			if (!handle->languages[cur_lang].excluded && 
				!enchant_dict_check(handle->enchant_dicts[cur_lang], word[cur_lang], strlen(word[cur_lang])))
			{
				log_message(DEBUG, _("   [+] Found this word in %s enchant wrapper dictionary"), handle->languages[cur_lang].name);
				return cur_lang;
			}
		}
		else
		{
			log_message(DEBUG, _("   [!] Now we don't support enchant wrapper dictionary for %s layout"), handle->languages[cur_lang].name);
		}
		

		// check for another languages
		for (int lang = 0; lang < handle->total_languages; lang++)
		{
			if (handle->languages[lang].excluded || lang == cur_lang)
				continue;
			
			if (!handle->has_enchant_checker[lang])
			{
				log_message(DEBUG, _("   [!] Now we don't support enchant wrapper dictionary for %s layout"), handle->languages[lang].name);
				continue;
			}
			
			if (!enchant_dict_check(handle->enchant_dicts[lang], word[lang], strlen(word[lang])))
			{
				log_message(DEBUG, _("   [+] Found this word in %s enchant wrapper dictionary"), handle->languages[lang].name);
				return lang;
			}
		}
	}

	log_message(DEBUG, _("   [-] This word has no hits for all enchant wrapper dictionaries"));
	return NO_LANGUAGE;
}
#endif				

static int get_proto_hits(struct _xneur_handle *handle, char *word, int *sym_len, int len, int offset, int lang)
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

		if (handle->languages[lang].proto->exist(handle->languages[lang].proto, proto, BY_PLAIN))
		{
			free(proto);
			return TRUE;
		}

		local_offset += sym_len[i];
	}

	free(proto);
	return FALSE;
}

static int get_big_proto_hits(struct _xneur_handle *handle, char *word, int *sym_len, int len, int offset, int lang)
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

		if (handle->languages[lang].proto->exist(handle->languages[lang].big_proto, proto, BY_PLAIN))
		{
			free(proto);
			return TRUE;
		}

		local_offset += sym_len[i];
	}

	free(proto);
	return FALSE;
}

static int get_proto_lang(struct _xneur_handle *handle, char **word, int **sym_len, int len, int offset, int cur_lang, int proto_len)
{
	int (*get_proto_hits_function) (struct _xneur_handle *handle, char *word, int *sym_len, int len, int offset, int lang);

	if (proto_len == PROTO_LEN)
		get_proto_hits_function = get_proto_hits;
	else
		get_proto_hits_function = get_big_proto_hits;

	if (len < proto_len)
	{
		log_message(DEBUG, _("   [-] Skip checking by language proto of size %d (word is very short)"), proto_len);
		return NO_LANGUAGE;
	}

	int hits = get_proto_hits_function(handle, word[cur_lang], sym_len[cur_lang], len, offset, cur_lang);
	if (hits == 0)
	{
		log_message(DEBUG, _("   [-] This word is ok for %s proto of size %d"), handle->languages[cur_lang].name, proto_len);
		return NO_LANGUAGE;
	}

	log_message(DEBUG, _("   [*] This word has hits for %s proto of size %d"), handle->languages[cur_lang].name, proto_len);

	for (int lang = 0; lang < handle->total_languages; lang++)
	{
		if ((lang == cur_lang) || (handle->languages[lang].excluded))
			continue;

		int hits = get_proto_hits_function(handle, word[lang], sym_len[lang], len, offset, lang);
		if (hits != 0)
		{
			log_message(DEBUG, _("   [*] This word has hits for %s language proto of size %d"), handle->languages[lang].name, proto_len);
			continue;
		}

		log_message(DEBUG, _("   [+] This word has no hits for %s language proto of size %d"), handle->languages[lang].name, proto_len);
		return lang;
	}

	log_message(DEBUG, _("   [-] This word has hits in all languages proto of size %d"), proto_len);
	return NO_LANGUAGE;
}

int check_lang(struct _xneur_handle *handle, struct _buffer *p, int cur_lang)
{
	char **word = (char **) malloc((handle->total_languages + 1) * sizeof(char *));
	int **sym_len = (int **) malloc((handle->total_languages + 1) * sizeof(int *));

	for (int i = 0; i < handle->total_languages; i++)
	{
		word[i] = strdup(get_last_word(p->i18n_content[i].content));
		del_final_numeric_char(word[i]);

		log_message(DEBUG, _("Processing word '%s' on layout '%s'"), word[i], handle->languages[i].dir);

		sym_len[i] = p->i18n_content[i].symbol_len + get_last_word_offset(p->content, strlen(p->content));
	}

	log_message(DEBUG, _("Start word processing..."));

	// Check by regexp
	int lang = get_regexp_lang(handle, word);

	// Check by dictionary
	if (lang == NO_LANGUAGE)
		lang = get_dict_lang(handle, word);

	int len = strlen(get_last_word(p->content));

#ifdef WITH_ENCHANT
	// Check by enchant wrapper
	if (lang == NO_LANGUAGE)
		lang = get_enchant_hits(handle, word, len, cur_lang);
#endif

#ifdef WITH_ASPELL
	// Check by aspell
	if (lang == NO_LANGUAGE)
		lang = get_aspell_hits(handle, word, len, cur_lang);
#endif
	
	// If not found in dictionary, try to find in proto
	len = strlen(p->content);
	int offset = get_last_word_offset(p->content, len);
	if (lang == NO_LANGUAGE)
		lang = get_proto_lang(handle, word, sym_len, len, offset, cur_lang, PROTO_LEN);

	if (lang == NO_LANGUAGE)
		lang = get_proto_lang(handle, word, sym_len, len, offset, cur_lang, BIG_PROTO_LEN);

	log_message(DEBUG, _("End word processing."));

	for (int i = 0; i < handle->total_languages; i++)
		free(word[i]);
	free(word);
	free(sym_len);
	return lang;
}

