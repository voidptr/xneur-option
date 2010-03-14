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

#ifndef _TRAYICON_H_
#define _TRAYICON_H_

#include "tray_widget.h"

struct _tray_icon
{
	GtkTrayIcon   *widget;

	GtkTooltips *tooltip;
	GtkWidget *image;
	GtkWidget *popup_menu;
	GtkWidget *evbox;
	GtkWidget *clock;
};

void create_tray_icon(struct _tray_icon *tray, gboolean runned);

#endif /* _TRAYICON_H_ */
