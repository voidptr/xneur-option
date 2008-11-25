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

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>

#include "xnconfig.h"

#include "xprogram.h"
#include "xbtable.h"
#include "xswitchlang.h"
#include "xOSD.h"

#include "types.h"
#include "utils.h"
#include "list_char.h"
#include "log.h"
#include "colors.h"
#include "sound.h"

struct _xneur_config *xconfig = NULL;
static struct _xprogram *xprogram = NULL;

static int xneur_check_lock = TRUE;

static void xneur_reload(int status);

static void xneur_check_config_version(int final)
{
	log_message(LOG, "Checking configuration file version...");

	if (xconfig->version != NULL && strcmp(xconfig->version, VERSION) == 0)
	{
		log_message(LOG, "User configuration file version is OK!");
		return;
	}

	log_message(ERROR, "Configuration file version is out of date!");

	if (final)
	{
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}

	if (!xconfig->replace(xconfig))
	{
		log_message(ERROR, "Default configuration file not founded in system! Please, reinstall XNeur!");
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}

	log_message(LOG, "Configuration file replaced to default one");

	xneur_reload(0);
}

static void xneur_init(void)
{
	if (!print_keyboard_groups())
	{
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}

	bind_manual_actions();
	bind_user_actions();
}

static void xneur_load_config(int final)
{
	log_message(LOG, "Loading configuration");

	if (!xconfig->load(xconfig))
	{
		log_message(ERROR, "Configuration file damaged! Please, remove old file before starting xneur!");
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}

	xneur_check_config_version(final);

	log_message(LOG, "Log level is set to %s", xconfig->get_log_level_name(xconfig));
	log_message(LOG, "Total detected %d languages", xconfig->total_languages);

	for (int lang = 0; lang < xconfig->total_languages; lang++)
	{
		char *lang_name = xconfig->get_lang_name(xconfig, lang);

		log_message(DEBUG, "%s dictionary has %d records", lang_name, xconfig->languages[lang].dicts->data_count);
		log_message(DEBUG, "%s proto has %d records", lang_name, xconfig->languages[lang].protos->data_count);
		log_message(DEBUG, "%s big proto has %d records", lang_name, xconfig->languages[lang].big_protos->data_count);
		log_message(DEBUG, "%s regexp has %d records", lang_name, xconfig->languages[lang].regexp->data_count);
	}

	log_message(DEBUG, "Configuration load complete");

	log_message(LOG, "Default group for all new windows set to %d", xconfig->default_group);
	log_message(LOG, "Manual mode set to %s", xconfig->get_bool_name(xconfig->is_manual_mode(xconfig)));
	log_message(LOG, "Mouse processing mode set to %s", xconfig->get_bool_name(xconfig->grab_mouse));
	log_message(LOG, "Education mode set to %s", xconfig->get_bool_name(xconfig->educate));
	log_message(LOG, "Layout remember mode set to %s", xconfig->get_bool_name(xconfig->remember_layout));
	log_message(LOG, "Save selection mode set to %s", xconfig->get_bool_name(xconfig->save_selection));
	log_message(LOG, "Sound playing mode set to %s", xconfig->get_bool_name(xconfig->play_sounds));
	log_message(LOG, "Logging keyboard mode set to %s", xconfig->get_bool_name(xconfig->save_keyboard_log));
	log_message(LOG, "Ignore keyboard layout for abbreviations mode set to %s", xconfig->get_bool_name(xconfig->abbr_ignore_layout));
	log_message(LOG, "Correct of iNCIDENTAL CapsLock mode set to %s", xconfig->get_bool_name(xconfig->correct_incidental_caps));
	log_message(LOG, "Correct of two CApital letter mode set to %s", xconfig->get_bool_name(xconfig->correct_two_capital_letter));
	log_message(LOG, "Flush internal buffer when pressed Enter or Tab mode set to %s", xconfig->get_bool_name(xconfig->flush_buffer_when_press_enter));
	log_message(LOG, "Don't process word when pressed Enter or Tab mode set to %s", xconfig->get_bool_name(xconfig->dont_process_when_press_enter));
}

static void xneur_set_lock(void)
{
	if (xneur_check_lock == TRUE)
	{
		int locked_pid = xconfig->get_pid(xconfig);
		if (locked_pid != -1)
		{
			log_message(ERROR, PACKAGE " already running with pid %d", locked_pid);
			exit(EXIT_FAILURE);
		}
	}

	int process_id = getpid();

	xconfig->set_pid(xconfig, process_id);

	log_message(DEBUG, PACKAGE " pid is %d", process_id);
}

static void xneur_cleanup(void)
{
	sound_uninit();
	log_message(DEBUG, "Current sound data is freed");

	if (xprogram != NULL)
		xprogram->uninit(xprogram);

	log_message(DEBUG, "Current program info is freed");

	if (xconfig != NULL)
	{
		xconfig->set_pid(xconfig, 0);
		xconfig->uninit(xconfig);
	}
	log_message(DEBUG, "Current configuration data is freed");

#ifdef WITH_DEBUG
	xndebug_uninit();
#endif
}

static void xneur_terminate(int status)
{
	if (status){}

	log_message(DEBUG, "Caught SIGTERM/SIGINT, terminating");
	osd_show("X Neural Switcher stopped");
	sleep(1);
	
	xneur_cleanup();

	exit(EXIT_SUCCESS);
}

static void xneur_reload(int status)
{
	status = status; // To prevent warnings
	log_message(LOG, "Caught SIGHUP, reloading configuration file");
	osd_show("X Neural Switcher reloaded");
	
	sound_uninit();

	if (xconfig != NULL)
		xconfig->uninit(xconfig);

	xconfig = xneur_config_init();
	if (xconfig == NULL)
	{
		log_message(ERROR, "Can't init libxnconfig");
		exit(EXIT_FAILURE);
	}

	xneur_load_config(TRUE);
	xneur_init();
	sound_init();
}

static void xneur_usage(void)
{
	printf("\nXneur - automatic keyboard switcher (version %s) \n", VERSION);
	printf("usage: xneur [options]\n");
	printf("  where options are:\n");
	printf("\n");
	printf("  -v, --version           Print version and exit\n");
	printf("  -h, --help              This help!\n");
	printf("  -a, --about             About for " PACKAGE "\n");
	printf("  -f, --force             Skip check for other instances of " PACKAGE " runned\n");
}

static void xneur_version(void)
{
	printf("\nXneur - automatic keyboard switcher (version %s) \n", VERSION);
	printf("mailto: andrewcrew@rambler.ru\n\n");
	printf("web: http://www.xneur.ru/\n");
}

static void xneur_about(void)
{
	printf("\nXneur\n");
	printf("Automatic keyboard switcher (version %s) \n", VERSION);
	printf("It's program like Punto Switcher. \n");
	printf("This utility is made for X Window System.\n\n");
	printf("mailto: andrewcrew@rambler.ru\n");
	printf("web: http://www.xneur.ru/\n");
}

static void xneur_get_options(int argc, char *argv[])
{
	static struct option longopts[] =
	{
			{ "version",		no_argument,	NULL,	'v' },
			{ "help",		no_argument,	NULL,	'h' },
			{ "about",		no_argument,	NULL,	'a' },
			{ "force",		no_argument,	NULL,	'f' },
			{ NULL,			0,		NULL,	0 }
	};

	int opted = FALSE;
	int opt;
	while ((opt = getopt_long(argc, argv, "vhafg", longopts, NULL)) != -1)
	{
		opted = TRUE;
		switch (opt)
		{
			case 'v':
			{
				xneur_version();
				break;
			}
			case 'a':
			{
				xneur_about();
				break;
			}
			case 'f':
			{
				xneur_check_lock = FALSE;
				opted = FALSE;
				break;
			}
			case '?':
			case 'h':
			{
				xneur_usage();
				break;
			}
		}
	}

	if (opted)
		exit(EXIT_SUCCESS);
}

static void xneur_reklama(void)
{
	printf("\n");
	printf(LIGHT_PURPLE_COLOR "====================================================" NORMAL_COLOR "\n");
	printf(LIGHT_PURPLE_COLOR ">>> " LIGHT_PURPLE_COLOR "Please visit " RED_COLOR "http://www.xneur.ru" LIGHT_BLUE_COLOR " for support" LIGHT_PURPLE_COLOR " <<<" NORMAL_COLOR "\n");
	printf(LIGHT_PURPLE_COLOR "====================================================" NORMAL_COLOR "\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	xneur_reklama();

	xneur_get_options(argc, argv);

	xconfig = xneur_config_init();
	if (xconfig == NULL)
	{
		log_message(ERROR, "Can't init libxnconfig");
		exit(EXIT_FAILURE);
	}

	xneur_set_lock();
	xneur_load_config(FALSE);

	xprogram = xprogram_init();
	if (xprogram == NULL)
	{
		log_message(ERROR, "Failed to init program structure");
		xconfig->set_pid(xconfig, 0);
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}

	sound_init();
	xneur_init();

	log_message(DEBUG, "Init program structure complete");
	osd_show("X Neural Switcher started");
	
	xntrap(SIGTERM, xneur_terminate);
	xntrap(SIGINT, xneur_terminate);
	xntrap(SIGHUP, xneur_reload);

	xprogram->process_input(xprogram);

	xneur_cleanup();

	exit(EXIT_SUCCESS);
}
