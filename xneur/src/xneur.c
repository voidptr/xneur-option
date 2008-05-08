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

static void xneur_load_config(int final)
{
	log_message(LOG, "Loading configuration");

	// Load configuration
	if (!xconfig->load(xconfig))
	{
		log_message(ERROR, "Configuration file damaged! Please, remove old file before starting xneur!");
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}
	
	// Checking configuration file version
	xneur_check_config_version(final);

	const char *log_levels[] = {"Error", "Warning", "Log", "Debug", "Trace"};
	log_message(LOG, "Log level is set to %s", log_levels[xconfig->log_level]);

	log_message(LOG, "Total detected %d languages", xconfig->total_languages);
	
	if (!check_xkb_extension() || !check_keyboard_groups())
	{
		xconfig->uninit(xconfig);
		exit(EXIT_FAILURE);
	}

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
	
	char *current_mode = "auto";
	if (xconfig->get_current_mode(xconfig) == MANUAL_MODE)
		current_mode = "manual";
	log_message(LOG, "Current mode set to %s", current_mode);

	current_mode = "Yes";
	if (xconfig->mouse_processing_mode == MOUSE_GRAB_DISABLE)
		current_mode = "No";
	log_message(LOG, "Mouse processing mode set to %s", current_mode);

	current_mode = "Yes";
	if (xconfig->education_mode == EDUCATION_MODE_DISABLE)
		current_mode = "No";
	log_message(LOG, "Education mode set to %s", current_mode);

	current_mode = "Yes";
	if (xconfig->layout_remember_mode == LAYOUTE_REMEMBER_DISABLE)
		current_mode = "No";
	log_message(LOG, "Layout remember mode set to %s", current_mode);

	current_mode = "Yes";
	if (xconfig->save_selection_mode == SELECTION_SAVE_DISABLED)
		current_mode = "No";
	log_message(LOG, "Save selection after convertion mode set to %s", current_mode);
	
	current_mode = "Yes";
	if (xconfig->sound_mode == SOUND_DISABLED)
		current_mode = "No";
	log_message(LOG, "Sound playing mode set to %s", current_mode);
	
	current_mode = "KeyPress";
	if (xconfig->events_receive_mode == EVENT_RELEASE)
		current_mode = "KeyRelease";
	log_message(LOG, "Current events receive mode set to %s", current_mode);
	
	bind_manual_actions();	
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

	int xneur_pid = getpid();

	xconfig->set_pid(xconfig, xneur_pid);

	log_message(DEBUG, PACKAGE " pid is %d", xneur_pid);
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

	xneur_cleanup();

	exit(EXIT_SUCCESS);
}

static void xneur_reload(int status)
{
	status = status; // To prevent warnings
	log_message(LOG, "Caught SIGHUP, reloading configuration file");

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

	// Parse command line options
	xneur_get_options(argc, argv);

	// Init configuration
	xconfig = xneur_config_init();
	if (xconfig == NULL)
	{
		log_message(ERROR, "Can't init libxnconfig");
		return EXIT_FAILURE;
	}

	// Init program structures
	xprogram = xprogram_init();
	if (xprogram == NULL)
	{
		log_message(ERROR, "Failed to init program structure");
		xconfig->set_pid(xconfig, 0);
		xconfig->uninit(xconfig);
		return EXIT_FAILURE;
	}
	
	// Set lock pid
	xneur_set_lock();

	// Load configuration file
	xneur_load_config(FALSE);

	sound_init();

	log_message(DEBUG, "Init program structure complete");

	xntrap(SIGTERM, xneur_terminate);
	xntrap(SIGINT, xneur_terminate);
	xntrap(SIGHUP, xneur_reload);

	xprogram->process_input(xprogram);

	xneur_cleanup();

	return EXIT_SUCCESS;
}
