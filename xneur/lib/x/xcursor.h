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

#ifndef _XIMAGE_H_
#define _XIMAGE_H_
 
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <X11/Xutil.h>
#include <X11/xpm.h>

#include "xnconfig.h"

struct _xcursor
{
	void (*show_flag) (int x, int y);
	void (*hide_flag) (void);
	void (*uninit) (struct _xcursor *p);
};

struct _xcursor* xcursor_init(void);

#endif /* _XIMAGE_H_ */
