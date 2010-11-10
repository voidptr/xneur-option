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

#ifndef _TRAYICON_H_
#define _TRAYICON_H_

#define MAX_LAYOUTS 4

#include "tray_widget.h"

struct _tray_icon
{
	GtkTrayIcon *tray_icon;
	
	GdkPixbuf  *images[MAX_LAYOUTS];
		
	GtkTooltips *tooltip;
	GtkWidget *image;
	GtkWidget *tray_menu;
	GtkWidget *evbox;
};

void create_tray_icon(void);
void xneur_start_stop(void);
void xneur_auto_manual(void);
void xneur_exit(void);

#endif /* _TRAYICON_H_ */
