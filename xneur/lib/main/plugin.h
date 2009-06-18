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

#ifndef _PLUGIN_H_
#define _PLUGIN_H_

#include <X11/XKBlib.h>

#include "xnconfig.h"
//#include <xneur/xnconfig.h>
#include "program.h"
//#include <xneur/program.h>

struct _plugin_functions
{
	void * module;

	void (*on_init)(void);
	void (*on_fini)(void);
	
	void (*on_xneur_start)(void);
	void (*on_xneur_reload)(void);
	void (*on_xneur_stop)(void);
	void (*on_key_press)(struct _program *program, KeySym key, int modifier_mask);
	void (*on_key_release)(struct _program *program, KeySym key, int modifier_mask);
	void (*on_hotkey_action)(enum _hotkey_action ha);
	void (*on_change_action)(enum _change_action ca);
};

struct _plugin
{
	struct _plugin_functions *plugin;
	int plugin_count;
	
	void (*xneur_start) (struct _plugin *p);
	void (*xneur_reload) (struct _plugin *p);
	void (*xneur_stop) (struct _plugin *p);
	void (*key_press) (struct _plugin *p, struct _program *program, KeySym key, int modifier_mask);
	void (*key_release) (struct _plugin *p, struct _program *program, KeySym key, int modifier_mask);
	void (*hotkey_action) (struct _plugin *p, enum _hotkey_action ha);
	void (*change_action) (struct _plugin *p, enum _change_action ca);
	
	void (*add) (struct _plugin *p, char* plugin_name);
	void (*uninit) (struct _plugin *p);
};

struct _plugin* plugin_init(void);

#endif /* _PLUGIN_H_ */
